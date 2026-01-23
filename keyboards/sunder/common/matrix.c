// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "matrix.h"
#include "quantum.h"

#include "he_adc.h"
#include "he_debug.h"
#include "he_keys.h"
#include "he_matrix.h"

#ifdef SPLIT_KEYBOARD
#    include "split_common/split_util.h"
#    include "split_common/transactions.h"

#    define ROWS_PER_HAND (MATRIX_ROWS / 2)
uint8_t thisHand, thatHand;
#else
#    define ROWS_PER_HAND (MATRIX_ROWS)
#endif

matrix_row_t matrix[MATRIX_ROWS];

matrix_row_t press_states[ROWS_PER_HAND];
key_state_t key_matrix[ROWS_PER_HAND][MATRIX_COLS];

input_priority_state_t input_priority_states[NUM_INPUT_PRIORITY_PAIRS];
uint8_t input_priority_indices[DYNAMIC_KEYMAP_LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS];

#if defined(DEBUG_MATRIX_SCAN_RATE)
static uint32_t matrix_timer = 0;
#endif

#ifdef SPLIT_KEYBOARD
bool matrix_post_scan(void) {
    bool changed = false;
    if (is_keyboard_master()) {
        static bool  last_connected              = false;
        matrix_row_t slave_matrix[ROWS_PER_HAND] = {0};
        if (transport_master_if_connected(matrix + thisHand, slave_matrix)) {
            changed = memcmp(matrix + thatHand, slave_matrix, sizeof(slave_matrix)) != 0;

            last_connected = true;
        } else if (last_connected) {
            // reset other half when disconnected
            memset(slave_matrix, 0, sizeof(slave_matrix));
            changed = true;

            last_connected = false;
        }

        if (changed) memcpy(matrix + thatHand, slave_matrix, sizeof(slave_matrix));

        matrix_scan_kb();
    } else {
        transport_slave(matrix + thatHand, matrix + thisHand);

        matrix_slave_scan_kb();
    }

    return changed;
}

__attribute__((weak)) void matrix_slave_scan_kb(void) {
    matrix_slave_scan_user();
}
__attribute__((weak)) void matrix_slave_scan_user(void) {}
#endif

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

inline bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[row] & ((matrix_row_t)1 << col));
}

void matrix_print(void) {
    //
}

void bootmagic_scan(void) {
    uint8_t row = BOOTMAGIC_ROW;
    uint8_t col = BOOTMAGIC_COLUMN;

#if defined(SPLIT_KEYBOARD) && defined(BOOTMAGIC_ROW_RIGHT) && defined(BOOTMAGIC_COLUMN_RIGHT)
    if (!is_keyboard_left()) {
        row = BOOTMAGIC_ROW_RIGHT - thisHand;
        col = BOOTMAGIC_COLUMN_RIGHT;
    }
#endif
    set_mux_pins(col);
    adcConvert(&ADCD1, &adcgrpcfg, adc_buf, ADC_BUFFER_DEPTH);

    uint16_t analog_value = adc_buf[row];

    if (analog_value > 2450) {
        // Jump to bootloader.
        bootloader_jump();
    }
}

void matrix_init(void) {

#ifdef SPLIT_KEYBOARD
    thisHand = isLeftHand ? 0 : (ROWS_PER_HAND);
    thatHand = ROWS_PER_HAND - thisHand;
#endif

    memset(matrix, 0, sizeof(matrix));
    adc_dma_init();
    mux_pin_init();
    he_sensor_init();

    // This *must* be called for correct keyboard behavior
    matrix_init_kb();
}

uint8_t matrix_scan(void) {

    matrix_row_t curr_matrix[ROWS_PER_HAND] = {0};

    #ifdef SPLIT_KEYBOARD
    memcpy(curr_matrix, matrix + thisHand, sizeof(matrix_row_t) * ROWS_PER_HAND);
    #else
    memcpy(curr_matrix, matrix, sizeof(matrix));
    #endif

    uint8_t profile = LAYER_UNIVERSAL;
    uint8_t current_layer = get_highest_layer(layer_state);
    
    if (he_config.special_layer != 0 && current_layer == he_config.special_layer - 1)
        profile = LAYER_SPECIAL;

    const uint8_t act_buf = he_config.switch_option == 0 ? 8 : 7;
    
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        set_mux_pins(col);
        adcConvert(&ADCD1, &adcgrpcfg, adc_buf, ADC_BUFFER_DEPTH);
        
        for (uint8_t row = 0; row < ROWS_PER_HAND; row++) {
            key_state_t *key_state = &key_matrix[row][col];

            if (key_state->adc_max == 0) continue;

            actuation_t *actuation_cfg = &he_config.actuation_matrix[profile][row + thisHand][col];
            key_state->adc_val = adc_buf[row];

            if (key_state->adc_val > key_state->adc_max + MIN_MAX_BUFFER) {
                key_state->adc_max = key_state->adc_val;

                uint32_t range = key_state->adc_max - key_state->adc_min;
                const uint32_t target_resolution_shifted = 67108864;
                key_state->scale = (target_resolution_shifted + (range >> 1)) / range;
            } else {
                key_state->adc_val = MIN(MAX(key_state->adc_min, key_state->adc_val), key_state->adc_max);
            }

            uint32_t curr_offset = key_state->adc_val - key_state->adc_min;
            uint16_t norm_value = ((curr_offset * key_state->scale) + 0x8000) >> 16;
            key_state->pos_curr = lut[norm_value];

            // Update key states
            if (actuation_cfg->rt_mode == 0) {
                uint8_t reset_point = (actuation_cfg->reset_point == 255) ? (actuation_cfg->actuation_point - act_buf) : actuation_cfg->reset_point;
                key_state->key_dir = KEY_DIR_INACTIVE;
                
                if (key_state->pos_curr > actuation_cfg->actuation_point) {
                    key_state->is_pressed = true;
                } else if (key_state->pos_curr <= (reset_point)) {
                    key_state->is_pressed = false;
                }
            } else {
                // Rapid Trigger
                uint8_t rt_release = (actuation_cfg->rt_release == 0) ? actuation_cfg->rt_press : actuation_cfg->rt_release;
                uint8_t reset_point = (actuation_cfg->rt_mode == 2) ? 0 : (actuation_cfg->actuation_point - act_buf);
                
                if (key_state->key_dir == KEY_DIR_INACTIVE) {
                    if (key_state->pos_curr > actuation_cfg->actuation_point) {
                        key_state->pos_curr = key_state->pos_curr;
                        key_state->key_dir = KEY_DIR_DOWN;
                        key_state->is_pressed = true;
                    }
                } 
                else {
                    // Active State (DOWN or UP)
                    if (key_state->pos_curr <= reset_point) {
                        key_state->pos_prev = key_state->pos_curr;
                        key_state->key_dir = KEY_DIR_INACTIVE;
                        key_state->is_pressed = false;
                    }
                    else if (key_state->key_dir == KEY_DIR_DOWN) {
                        if (key_state->pos_curr > key_state->pos_prev) {
                            key_state->pos_prev = key_state->pos_curr; 
                        } else if ((key_state->pos_prev > key_state->pos_curr) && ((key_state->pos_prev - key_state->pos_curr) > rt_release)) {
                            key_state->pos_prev = key_state->pos_curr;
                            key_state->key_dir = KEY_DIR_UP;
                            key_state->is_pressed = false;
                        }
                    } 
                    else { // KEY_DIR_UP
                        if (key_state->pos_curr < key_state->pos_prev) {
                            key_state->pos_prev = key_state->pos_curr;
                        } else if ((key_state->pos_curr > key_state->pos_prev) && ((key_state->pos_curr - key_state->pos_prev) > actuation_cfg->rt_press)) {
                            key_state->pos_prev = key_state->pos_curr;
                            key_state->key_dir = KEY_DIR_DOWN;
                            key_state->is_pressed = true;
                        }
                    }
                }
            }
        }
    }

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {

        for (uint8_t row = 0; row < ROWS_PER_HAND; row++) {

            bool prev_pressed = (press_states[row] & (1 << col));
            key_state_t *key_state = &key_matrix[row][col];
            

            if (!prev_pressed && key_state->is_pressed) {
                uint8_t pair_index = input_priority_indices[current_layer][row + thisHand][col];
                if (pair_index) {
                    const input_priority_t *pair_cfg = &he_config.input_priority_pairs[pair_index - 1];
                    input_priority_state_t *pair_state = &input_priority_states[pair_index - 1];

                    uint8_t row_pairs[] = {
                        pair_cfg->primary_row - thisHand,
                        pair_cfg->secondary_row - thisHand,
                    };

                    uint8_t col_pairs[] = {
                        pair_cfg->primary_col,
                        pair_cfg->secondary_col,
                    };

                    const uint8_t index = (row == row_pairs[0] && col == col_pairs[0]) ? 0 : 1;
                    
                    bool is_pressed[] = {
                        key_matrix[row_pairs[0]][col_pairs[0]].is_pressed,
                        key_matrix[row_pairs[1]][col_pairs[1]].is_pressed,
                    };
                    
                    if (is_pressed[0] & is_pressed[1]) {
                        if (pair_cfg->resolution == INPUT_PRIORITY_RESOLUTION_DEPTH) {
                            is_pressed[index] = key_matrix[row_pairs[index]][col_pairs[index]].pos_curr >=
                                                key_matrix[row_pairs[index ^ 1]][col_pairs[index ^ 1]].pos_curr + act_buf;
                            is_pressed[index ^ 1] = !is_pressed[index];
                        } else {
                            is_pressed[index] =
                                (pair_cfg->resolution != INPUT_PRIORITY_RESOLUTION_NEUTRAL) &
                                ((pair_cfg->resolution == INPUT_PRIORITY_RESOLUTION_LAST) |
                                ((pair_cfg->resolution == INPUT_PRIORITY_RESOLUTION_PRIMARY) & (index == 0)) |
                                ((pair_cfg->resolution == INPUT_PRIORITY_RESOLUTION_SECONDARY) & (index == 1)));

                            is_pressed[index ^ 1] =
                                (pair_cfg->resolution != INPUT_PRIORITY_RESOLUTION_NEUTRAL) & !is_pressed[index];
                        }
                    }

                    for (uint32_t i = 0; i < 2; i++) {
                        if (is_pressed[i] & !pair_state->is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] |= (1 << col_pairs[i]);
                        } else if (!is_pressed[i] & pair_state->is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] &= ~(1 << col_pairs[i]);
                        }
                    }

                } else {
                    curr_matrix[row] |= (1 << col);
                }
                press_states[row] |= (1 << col);

            } else if (prev_pressed && !key_state->is_pressed) {
                uint8_t pair_index = input_priority_indices[current_layer][row + thisHand][col];
                if (pair_index) {
                    const input_priority_t *pair_cfg = &he_config.input_priority_pairs[pair_index - 1];
                    input_priority_state_t *pair_state = &input_priority_states[pair_index - 1];

                    uint8_t row_pairs[] = {
                        pair_cfg->primary_row - thisHand,
                        pair_cfg->secondary_row - thisHand,
                    };

                    uint8_t col_pairs[] = {
                        pair_cfg->primary_col,
                        pair_cfg->secondary_col,
                    };

                    bool is_pressed[] = {
                        key_matrix[row_pairs[0]][col_pairs[0]].is_pressed,
                        key_matrix[row_pairs[1]][col_pairs[1]].is_pressed,
                    };

                    for (uint32_t i = 0; i < 2; i++) {
                        if (is_pressed[i] & !pair_state->is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] |= (1 << col_pairs[i]);
                        } else if (!is_pressed[i] & pair_state->is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] &= ~(1 << col_pairs[i]);
                        }
                    }

                } else {
                    curr_matrix[row] &= ~(1 << col);
                }
                press_states[row] &= ~(1 << col);

            } else if (key_state->is_pressed) {
                uint8_t pair_index = input_priority_indices[current_layer][row + thisHand][col];
                if (pair_index) {
                    const input_priority_t *pair_cfg = &he_config.input_priority_pairs[pair_index - 1];
                    input_priority_state_t *pair_state = &input_priority_states[pair_index - 1];

                    uint8_t row_pairs[] = {
                        pair_cfg->primary_row - thisHand,
                        pair_cfg->secondary_row - thisHand,
                    };

                    uint8_t col_pairs[] = {
                        pair_cfg->primary_col,
                        pair_cfg->secondary_col,
                    };

                    const uint8_t index = (row == row_pairs[0] && col == col_pairs[0]) ? 0 : 1;
                    
                    bool is_pressed[] = {
                        key_matrix[row_pairs[0]][col_pairs[0]].is_pressed,
                        key_matrix[row_pairs[1]][col_pairs[1]].is_pressed,
                    };
                    
                    if (pair_cfg->resolution == INPUT_PRIORITY_RESOLUTION_DEPTH && (is_pressed[0] & is_pressed[1])) {
                        if ((key_matrix[row_pairs[index]][col_pairs[index]].pos_curr >= 255 - act_buf) &&
                            (key_matrix[row_pairs[index ^ 1]][col_pairs[index ^ 1]].pos_curr >= 255 - act_buf)) {
                            
                            is_pressed[0] = is_pressed[1] = true;

                        } else if (!pair_state->is_pressed[index]) {
                            is_pressed[index] = key_matrix[row_pairs[index]][col_pairs[index]].pos_curr >=
                                                key_matrix[row_pairs[index ^ 1]][col_pairs[index ^ 1]].pos_curr + act_buf;
                            is_pressed[index ^ 1] = !is_pressed[index];
                        } else if (pair_state->is_pressed[index ^ 1]) {
                            is_pressed[index] = key_matrix[row_pairs[index]][col_pairs[index]].pos_curr >= 255 - act_buf;
                        } else {
                            is_pressed[0] = pair_state->is_pressed[0];
                            is_pressed[1] = pair_state->is_pressed[1];
                        }
                    } else if (is_pressed[0] & is_pressed[1]) {
                        is_pressed[0] = pair_state->is_pressed[0];
                        is_pressed[1] = pair_state->is_pressed[1];
                    }

                    for (uint32_t i = 0; i < 2; i++) {
                        if (is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] |= (1 << col_pairs[i]);
                        } else if (!is_pressed[i]) {
                            pair_state->is_pressed[i] = is_pressed[i];
                            curr_matrix[row_pairs[i]] &= ~(1 << col_pairs[i]);
                        }
                    }
                }
            }
        }
    }

    #if defined(DEBUG_MATRIX_SCAN_RATE)
    uint32_t timer_now = timer_read32();
    
    if (TIMER_DIFF_32(timer_now, matrix_timer) >= 500) {
        uprintf("matrix scan rate: %lu\n", get_matrix_scan_rate());

        if (is_keyboard_left()) {
            print_key_matrix(left_print_index, left_print_index_len);
        } else {
            print_key_matrix(right_print_index, right_print_index_len);
        }
        matrix_timer = timer_now;
    }
    #endif

    bool changed;

#ifdef SPLIT_KEYBOARD
    changed = memcmp(matrix + thisHand, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(matrix + thisHand, curr_matrix, sizeof(curr_matrix));
    matrix_post_scan();
#else
    changed = memcmp(matrix, curr_matrix, sizeof(curr_matrix)) != 0;
    if (changed) memcpy(matrix, curr_matrix, sizeof(curr_matrix));
    // This *must* be called for correct keyboard behavior
    matrix_scan_kb();
#endif

    return changed;
}
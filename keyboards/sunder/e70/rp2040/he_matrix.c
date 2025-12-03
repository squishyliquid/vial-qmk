// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_matrix.h"
#include "he_sync.h"
#include "he_keys.h"
#include "he_adc.h"
#include "lut.h"
#include "split_util.h"

static const pin_t mux_pins[MUX_BITS] = ALL_MUX_PINS;

void mux_pin_init(void) {
    for (int i = 0; i < MUX_BITS; i++) {
        gpio_set_pin_output(mux_pins[i]);
        gpio_write_pin_low(mux_pins[i]);
    }
}

void he_sensor_init(void) {
    wait_ms(100);
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        set_mux_pins(c);

        adcConvert(&ADCD1, &adcgrpcfg, init_adc_buf, INIT_ADC_BUFFER_DEPTH);
        uint16_t adc_channel_avg[ADC_NUM_CHANNELS];
        average_adc_buffer(adc_channel_avg, init_adc_buf, INIT_ADC_BUFFER_DEPTH);
        
        for (uint8_t r = 0; r < ROWS_PER_HAND; r++) {
            if (is_keyboard_left()) {
                if ((r == 0 && (c == 0 || c == 7 || c == 12)) ||
                    (r == 1 && (c == 11 || c == 12)) ||
                    (r == 2 && c == 0)) {
                    continue;
                }
            } else if (c == 12 && (r == 0 || r == 1)) {
                continue;
            }

            key_state_t *key_state = &key_matrix[r][c];

            uint16_t analog_value = adc_channel_avg[r];

            key_state->pos_curr = 0;
            key_state->pos_prev = 0;
            
            uint16_t offset = (analog_value + 100) / 200; 

            key_state->adc_max = analog_value - offset;

            const he_mul_t *mul = &he_mul[10];

            for (uint8_t i = 0; i < 11; i++) {
                if (analog_value >= he_mul[i].threshold) {
                    mul = &he_mul[i];
                    break;
                }
            }

            key_state->adc_bp[IDX_05] = ((uint32_t)(analog_value) * (mul->m05) + 50) / 100;
            key_state->adc_bp[IDX_10] = ((uint32_t)(analog_value) * (mul->m10) + 50) / 100;
            key_state->adc_bp[IDX_15] = ((uint32_t)(analog_value) * (mul->m15) + 50) / 100;
            key_state->adc_bp[IDX_20] = ((uint32_t)(analog_value) * (mul->m20) + 50) / 100;
            key_state->adc_bp[IDX_25] = ((uint32_t)(analog_value) * (mul->m25) + 50) / 100;
            key_state->adc_bp[IDX_30] = ((uint32_t)(analog_value) * (mul->m30) + 50) / 100;
            key_state->adc_min  = ((uint32_t)(analog_value) * (mul->mmin) + 50) / 100 - offset;

        }
    }
}

static bool synced = false;
static bool full_sync = false;
static bool he_sensor_reinit = false;

bool get_synced_status(void) {
    return synced;
}

void enable_full_sync(void) {
    if (!full_sync) {
        full_sync = true;
    }
}

void enable_he_sensor_reinit(void) {
    if (!he_sensor_reinit) {
        he_sensor_reinit = true;
    }
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        if (full_sync && !synced) {
            if (is_transport_connected()) {
                he_full_sync();
                synced = true;
            }
        }
    }
    // if (he_sensor_reinit) {
    //     he_sensor_init();
    //     he_sensor_reinit = false;
    // }
}
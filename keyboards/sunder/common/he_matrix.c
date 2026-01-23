// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_matrix.h"
#include "he_sync.h"
#include "he_keys.h"
#include "he_adc.h"
#include "lut.h"
#include "split_util.h"

static const pin_t mux_pins[MUX_BITS] = ALL_MUX_PINS;

uint8_t *lut;

void mux_pin_init(void) {
    for (int i = 0; i < MUX_BITS; i++) {
        gpio_set_pin_output(mux_pins[i]);
        gpio_write_pin_low(mux_pins[i]);
    }
}

void he_sensor_init(void) {
    wait_ms(100);
    uint8_t offset_multiplier = 15;
    lut = lut_340;

    uint8_t switch_option = he_config.switch_option;

    switch (switch_option) {
    case SWITCH_320: {
        lut = lut_320;
        offset_multiplier = 12;
        break;
    }
    case SWITCH_340:
    case SWITCH_350: {
        break;
    }
    case SWITCH_380: {
        lut = lut_380;
        break;
    }
    case SWITCH_390: {
        lut = lut_390;
        offset_multiplier = 18;
        break;
    }
    default:
        break;
    }

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        set_mux_pins(c);

        adcConvert(&ADCD1, &adcgrpcfg, init_adc_buf, INIT_ADC_BUFFER_DEPTH);
        uint16_t adc_channel_avg[ADC_NUM_CHANNELS];
        average_adc_buffer(adc_channel_avg, init_adc_buf, INIT_ADC_BUFFER_DEPTH);
        
        for (uint8_t r = 0; r < ROWS_PER_HAND; r++) {

            key_state_t *key_state = &key_matrix[r][c];

            uint16_t analog_value = adc_channel_avg[r];

            if (analog_value < 100) {
                continue;
            }

            key_state->pos_curr = 0;
            key_state->pos_prev = 0;
            
            uint16_t offset = (analog_value + 50) / 100 * offset_multiplier; 

            key_state->adc_max = analog_value + offset;
            key_state->adc_min = analog_value + 1;

            uint32_t range = key_state->adc_max - key_state->adc_min;
            const uint32_t target_resolution_shifted = 67108864; //(1024 * 65536)
            key_state->scale = (target_resolution_shifted + (range >> 1)) / range;
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
    if (he_sensor_reinit) {
        he_sensor_init();
        he_sensor_reinit = false;
    }
}
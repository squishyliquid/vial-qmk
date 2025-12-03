// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_sync.h"
#include "quantum.h"
#include "transactions.h"
#include "split_util.h"

void he_full_sync(void) {
    for (int profile = 0; profile < ACTUATION_PROFILE_COUNT; profile ++) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                actuation_t *actuation_cfg = &he_config.actuation_matrix[profile][row][col];
                uint8_t cfg[] = {
                    ACTUATION_SYNC,
                    profile,
                    row,
                    col,
                    actuation_cfg->actuation_point,
                    actuation_cfg->rt_mode,
                    actuation_cfg->rt_press,
                    actuation_cfg->rt_release
                };
                transaction_rpc_send(HE_CONFIG_SYNC, sizeof(cfg), cfg);
            }
        }
    }

    for (int pair_index = 0; pair_index < NUM_INPUT_PRIORITY_PAIRS; pair_index ++) {
        input_priority_t *pair_cfg = &he_config.input_priority_pairs[pair_index];
        uint8_t cfg[] = {
            INPUT_PRIORITY_SYNC,
            pair_index,
            pair_cfg->layer,
            pair_cfg->primary_row,
            pair_cfg->primary_col,
            pair_cfg->secondary_row,
            pair_cfg->secondary_col,
            pair_cfg->resolution
        };
        transaction_rpc_send(HE_CONFIG_SYNC, sizeof(cfg), cfg);
    }

    uint8_t switch_option = he_config.switch_option;
    uint8_t cfg_switch[] = {
        SWITCH_OPTION_SYNC,
        switch_option
    };
    transaction_rpc_send(HE_CONFIG_SYNC, sizeof(cfg_switch), cfg_switch);

    uint8_t layer_index = he_config.special_layer;
    uint8_t cfg_layer[] = {
        SPECIAL_LAYER_SYNC,
        layer_index
    };
    transaction_rpc_send(HE_CONFIG_SYNC, sizeof(cfg_layer), cfg_layer);  
}

void he_sync_slave(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    const uint8_t* data = (const uint8_t*)in_data;
    
    switch (data[0]) {
        case ACTUATION_SYNC: {
            uint8_t profile = data[1];
            uint8_t row = data[2];
            uint8_t col = data[3];

            actuation_t actuation_cfg = {
                .actuation_point = data[4],
                .rt_mode = data[5],
                .rt_press = data[6],
                .rt_release = data[7]
            };

            dynamic_keymap_set_he_actuation_config(profile, row, col, &actuation_cfg);
            break;
        }
        case INPUT_PRIORITY_SYNC: {
            uint8_t index = data[1];

            input_priority_t pair_cfg = {
                .layer = data[2],
                .primary_row = data[3],
                .primary_col = data[4],
                .secondary_row = data[5],
                .secondary_col = data[6],
                .resolution = data[7]
            };

            dynamic_keymap_set_he_input_priority_pair(index, &pair_cfg);
            break;
        }
        case SWITCH_OPTION_SYNC: {
            uint8_t switch_option = data[1];

            dynamic_keymap_set_he_switch(&switch_option);
            break;
        }
        case SPECIAL_LAYER_SYNC: {
            uint8_t layer_index = data[1];
            dynamic_keymap_set_he_special_layer(&layer_index);
            break;
        }
        default:
            break;
    }
}

void keyboard_post_init_kb(void) {
    transaction_register_rpc(HE_CONFIG_SYNC, he_sync_slave);
    keyboard_post_init_user();
}
// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_keys.h"
#include "quantum.h"
#include "transactions.h"
#include "split_util.h"

he_config_t he_config = {
    .actuation_matrix = { 
        [0 ... ACTUATION_PROFILE_COUNT - 1] = { 
            [0 ... MATRIX_ROWS - 1][0 ... MATRIX_COLS - 1] = {
                DEFAULT_ACTUATION, 
                DEFAULT_RT_MODE, 
                DEFAULT_RT_PRESS,
                DEFAULT_RT_RELEASE
            } 
        }
    },
    .input_priority_pairs = { 
        [0 ... NUM_INPUT_PRIORITY_PAIRS - 1] = {
            .layer = INPUT_PRIORITY_PAIR_DISABLED,
            .primary_row = INPUT_PRIORITY_PAIR_DISABLED,
            .primary_col = INPUT_PRIORITY_PAIR_DISABLED,
            .secondary_row = INPUT_PRIORITY_PAIR_DISABLED,
            .secondary_col = INPUT_PRIORITY_PAIR_DISABLED,
            .resolution = INPUT_PRIORITY_PAIR_DISABLED
        }
    },
    .switch_option = DEFAULT_SWITCH,
    .special_layer = DEFAULT_SPECIAL_LAYER
};

uint8_t input_priority_indices[DYNAMIC_KEYMAP_LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS] = {0};
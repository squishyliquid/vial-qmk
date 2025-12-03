// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_debug.h"

const uint8_t left_print_index[][3] = {
    {0,0,3}, {0,0,2}, {0,0,1}, {0,0,11}, {0,0,10}, {0,0,9}, {0,0,8},
    {1,0,4}, {1,0,5}, {1,0,6}, {1,1,7}, {1,1,6}, {1,1,5}, {1,1,4},
    {2,1,8}, {2,1,9}, {2,1,10}, {2,1,0}, {2,1,1}, {2,1,2}, {2,1,3},
    {3,2,10}, {3,2,9}, {3,2,8}, {3,2,7}, {3,2,6}, {3,2,5}, {3,2,4},
    {4,2,11}, {4,2,12}, {4,2,1}, {4,2,2}, {4,2,3}
};

const uint8_t right_print_index[][3] = {
    {0,0,3}, {0,0,2}, {0,0,1}, {0,0,0}, {0,0,11}, {0,0,10}, {0,0,9}, {0,0,8},
    {1,0,4}, {1,0,5}, {1,0,6}, {1,0,7}, {1,1,7}, {1,1,6}, {1,1,5}, {1,1,4},
    {2,1,8}, {2,1,10}, {2,1,11}, {2,1,0}, {2,1,1}, {2,1,2}, {2,1,3},
    {3,1,9}, {3,2,3}, {3,2,2}, {3,2,1}, {3,2,0}, {3,2,12}, {3,2,11},
    {4,2,4}, {4,2,5}, {4,2,6}, {4,2,7}, {4,2,8}, {4,2,9}, {4,2,10}
};

const size_t left_print_index_len = sizeof(left_print_index)/sizeof(left_print_index[0]);
const size_t right_print_index_len = sizeof(right_print_index)/sizeof(right_print_index[0]);

void print_key_matrix(const uint8_t keys_arr[][3], size_t len) {
    uint8_t current_row = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t print_row = keys_arr[i][0];
        uint8_t k_row     = keys_arr[i][1];
        uint8_t k_col     = keys_arr[i][2];

        if (current_row != print_row) {
            current_row = print_row;
            uprintf("\n");
        }

        uprintf("(%u, %u, %u) ",
                key_matrix[k_row][k_col].pos_curr,
                key_matrix[k_row][k_col].adc_max,
                key_matrix[k_row][k_col].adc_val);

    }
    uprintf("\n\n");
}
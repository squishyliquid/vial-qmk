// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "he_debug.h"

const uint8_t left_print_index[][3] = {
    {0,0,4}, {0,0,6}, {0,0,7}, {0,1,4}, {0,1,6}, {0,1,5}, {0,1,7},
    {1,0,2}, {1,0,3}, {1,0,0}, {1,1,1}, {1,1,2}, {1,1,3}, {1,1,0},
    {2,0,1}, {2,2,4}, {2,2,6}, {2,2,5}, {2,2,7}, {2,3,7}, {2,3,3},
    {3,2,2}, {3,2,1}, {3,2,0}, {3,2,3}, {3,3,4}, {3,3,6},
    {4,3,2}, {4,3,1}, {4,3,0}
};

const uint8_t right_print_index[][3] = {
    {0,0,2}, {0,0,6}, {0,0,7}, {0,1,4}, {0,1,6}, {0,1,5}, {0,1,7},
    {1,0,1}, {1,0,3}, {1,0,0}, {1,1,1}, {1,1,2}, {1,1,3}, {1,1,0},
    {2,0,4}, {2,2,4}, {2,2,6}, {2,2,5}, {2,2,7}, {2,3,4}, {2,3,6},
    {3,2,2}, {3,2,1}, {3,2,0}, {3,2,3}, {3,3,7}, {3,3,3},
    {4,3,0}, {4,3,1}, {4,3,2}
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

        uprintf("(%u, %u) ",
                key_matrix[k_row][k_col].pos_curr,
                // key_matrix[k_row][k_col].adc_min,
                key_matrix[k_row][k_col].adc_val);
    }
    uprintf("\n\n");
}
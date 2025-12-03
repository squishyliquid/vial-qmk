// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

typedef enum {
    IDX_05 = 0,
    IDX_10,
    IDX_15,
    IDX_20,
    IDX_25,
    IDX_30,
    IDX_COUNT
} adc_idx_t;

typedef struct {
    uint16_t threshold;
    uint8_t m05, m10, m15, m20, m25, m30, mmin;
} he_mul_t;

typedef struct {
    uint16_t adc_threshold;   // adc_max threshold for this tier
    uint8_t  offset[7];       // travel offset per segment
    uint8_t  range[7];        // travel range per segment
} he_lut_t;

extern const he_mul_t he_mul[];

extern const he_lut_t he_lut[];
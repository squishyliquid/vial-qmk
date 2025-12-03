// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <inttypes.h>
#include <stdbool.h>

#ifdef SPLIT_KEYBOARD
#    define ROWS_PER_HAND (MATRIX_ROWS / 2)
#else
#    define ROWS_PER_HAND (MATRIX_ROWS)
#endif

#define MIN_MAX_BUFFER 5
typedef enum {
    SWITCH_320 = 0,
    SWITCH_340,
    SWITCH_350,
    SWITCH_380,
    SWITCH_390
} switch_t;

#define DEFAULT_ACTUATION 128
#define DEFAULT_RT_MODE 0
#define DEFAULT_RT_PRESS 0
#define DEFAULT_RT_RELEASE 0
#define DEFAULT_SWITCH SWITCH_340
#define DEFAULT_SPECIAL_LAYER 0
#define NUM_INPUT_PRIORITY_PAIRS 8
#define INPUT_PRIORITY_PAIR_DISABLED 255

typedef struct {
    uint8_t actuation_point;
    uint8_t rt_mode;
    uint8_t rt_press;
    uint8_t rt_release;
} actuation_t;

typedef enum {
    KEY_DIR_INACTIVE = 0,
    KEY_DIR_DOWN,
    KEY_DIR_UP,
} key_dir_t;

typedef struct {
    uint16_t adc_val;
    uint16_t adc_min;
    uint16_t adc_max;
    uint32_t scale;

    uint8_t pos_curr;
    uint8_t pos_prev;
    uint8_t key_dir;
    bool is_pressed;
} key_state_t;

extern key_state_t key_matrix[ROWS_PER_HAND][MATRIX_COLS];

typedef enum {
    LAYER_UNIVERSAL = 0,
    LAYER_SPECIAL,
    ACTUATION_PROFILE_COUNT
} actuation_layer_t;
typedef enum {
    INPUT_PRIORITY_RESOLUTION_LAST = 0,
    INPUT_PRIORITY_RESOLUTION_PRIMARY,
    INPUT_PRIORITY_RESOLUTION_SECONDARY,
    INPUT_PRIORITY_RESOLUTION_NEUTRAL,
    INPUT_PRIORITY_RESOLUTION_DEPTH,
} input_priority_resolution_t;

typedef struct {
    uint8_t layer;
    uint8_t primary_row;
    uint8_t primary_col;
    uint8_t secondary_row;
    uint8_t secondary_col;
    uint8_t resolution;
} input_priority_t;

typedef struct {
    bool is_pressed[2];
} input_priority_state_t;

extern input_priority_state_t input_priority_states[NUM_INPUT_PRIORITY_PAIRS];

extern uint8_t input_priority_indices[DYNAMIC_KEYMAP_LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS];

typedef struct {
    actuation_t actuation_matrix[ACTUATION_PROFILE_COUNT][MATRIX_ROWS][MATRIX_COLS];
    input_priority_t input_priority_pairs[NUM_INPUT_PRIORITY_PAIRS];
    uint8_t switch_option;
    uint8_t special_layer;
} he_config_t;

extern he_config_t he_config;
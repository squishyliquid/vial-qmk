// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

#ifdef SPLIT_KEYBOARD
#    define ROWS_PER_HAND (MATRIX_ROWS / 2)
#else
#    define ROWS_PER_HAND (MATRIX_ROWS)
#endif

void mux_pin_init(void);

void he_sensor_init(void);

bool get_synced_status(void);

void enable_full_sync(void);

void enable_he_sensor_reinit(void);
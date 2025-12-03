// Copyright 2023 squishyliquid (@squishyliquid)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef enum {
    ACTUATION_SYNC = 0,
    INPUT_PRIORITY_SYNC,
    SWITCH_OPTION_SYNC,
    SPECIAL_LAYER_SYNC
} he_sync_t;

void he_full_sync(void);
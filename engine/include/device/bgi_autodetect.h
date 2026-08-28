// FILENAME: bgi_autodetect.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c, screen.c)
// NEEDED BY: libkernel (bgi_autodetect.c)
// NEEDS: libengine (bgi.h, bgi.c)
// Implements virtual device and graphics rendering logic for bgi_autodetect.
//
// ---- Includes ----

#ifndef DEVICE_BGI_AUTODETECT_H
#define DEVICE_BGI_AUTODETECT_H

#include "device/bgi.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BGI_MODE_ID_AUTODETECT_START 500

// @brief Auto-detect host display modes and register them into BGI context.
// @param ctx Pointer to initialized BGI context.
// @return Number of autodetected modes registered.
int bgi_autodetect_modes(BGI_Context *ctx);

// @brief Get total count of autodetected host display modes.
int bgi_get_detected_mode_count(void);

// @brief Format human-readable info string for a registered mode (for CATALOG SCREEN).
// @param mode_idx Index in BGI modes table.
// @param buf Output buffer.
// @param buf_size Maximum buffer size.
void bgi_format_mode_info(int mode_idx, char *buf, size_t buf_size);

#endif // DEVICE_BGI_AUTODETECT_H

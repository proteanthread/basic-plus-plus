// FILENAME: dev_tandy.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (vdev.h)
// Header definitions for TRS-80 and Tandy family peripherals (MDMn:, TELn:, LCDn:, BARn:, WNDn:, LPn:, FDDn:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_TANDY_H
#define DEVICE_DEV_TANDY_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a concrete VDev instance for a Tandy/TRS-80 peripheral.
VDev dev_tandy_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_TANDY_H

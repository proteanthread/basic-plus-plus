// FILENAME: dev_loopback.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (vdev.h)
// Header definitions for full-duplex FIFO loopback testing device (LOOP1..8:, LOOP0:, LOOP9:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_LOOPBACK_H
#define DEVICE_DEV_LOOPBACK_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a concrete VDev instance for a loopback channel (LOOP1..8:, LOOP0:, LOOP9:).
VDev dev_loopback_create(const char *name, int channel);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_LOOPBACK_H

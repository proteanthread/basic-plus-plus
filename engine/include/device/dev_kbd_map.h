// FILENAME: dev_kbd_map.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (vdev.h)
// Header definitions for programmable keyboard & controller mapping profiles (KBD1..8:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_KBD_MAP_H
#define DEVICE_DEV_KBD_MAP_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a concrete VDev instance for a keyboard/controller profile (KBD1..8:).
VDev dev_kbd_map_create(const char *name, int profile);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_KBD_MAP_H

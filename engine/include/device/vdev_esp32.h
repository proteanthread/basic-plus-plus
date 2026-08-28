// FILENAME: vdev_esp32.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c)
// NEEDED BY: libkernel (vdev_esp32.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Declares the ESP32: virtual device driver for microcontroller stream I/O.
//
// ---- Includes ----

#ifndef VDEV_ESP32_H
#define VDEV_ESP32_H

#include "device/vdev.h"

// ---- Public Constructor ----
VDev vdev_esp32_create(const char *name);

#endif // VDEV_ESP32_H

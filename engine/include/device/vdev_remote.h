// FILENAME: vdev_remote.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (vdev_remote.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides REMOTE: virtual device interface.
//
// ---- Includes ----

#ifndef DEVICE_VDEV_REMOTE_H
#define DEVICE_VDEV_REMOTE_H

#include "device/vdev.h"

VDev vdev_remote_create(const char *name);

#endif // DEVICE_VDEV_REMOTE_H

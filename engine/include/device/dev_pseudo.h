// FILENAME: dev_pseudo.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libkernel
// NEEDS: libkernel (vdev.h)
// Unix/POSIX and hardware pseudo-device drivers (NULL, ZERO, RANDOM, URANDOM, PORT).
//
// ---- Includes ----

#ifndef DEVICE_DEV_PSEUDO_H
#define DEVICE_DEV_PSEUDO_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create pseudo-device (e.g. "NULL:", "NUL:", "ZERO:", "RANDOM:", "URANDOM:", "PORT:")
VDev dev_pseudo_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_PSEUDO_H

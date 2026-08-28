// FILENAME: fujinet.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Implements virtual device and graphics rendering logic for fujinet.
//
// ---- Includes ----

#ifndef DEVICE_FUJINET_H
#define DEVICE_FUJINET_H

#include "device/vdev.h"
#include "vm/vm.h"

// @brief Initialize FujiNet drivers and configurations.
void fujinet_init_system(VMContext *vm);

// @brief Clean up and close FujiNet system resources.
void fujinet_shutdown_system(void);

// @brief Create the 'N:' network adapter virtual device.
VDev fujinet_create_n_dev(VMContext *vm);

// @brief Create the 'FUJI:' configuration virtual device.
VDev fujinet_create_fuji_dev(VMContext *vm);

// @brief Create the 'CLOCK:' network time virtual device.
VDev fujinet_create_clock_dev(VMContext *vm);

#endif // DEVICE_FUJINET_H

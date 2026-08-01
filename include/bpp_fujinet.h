/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_fujinet.h
 * @brief FujiNet Virtual Device Driver API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares functions to initialize, create, and shut down
 *   the VDev2 drivers for N:, FUJI:, and CLOCK: virtual devices.
 * - Why it exists: Emulates FujiNet hardware network, configuration, and time interfaces (Phase 20).
 * - Why it works this way: It bridges standard BASIC++ I/O channel calls to Winsock/POSIX sockets and host OS time.
 */

#ifndef BPP_FUJINET_H
#define BPP_FUJINET_H

#include "bpp_vdev.h"
#include "bpp_vm.h"

/**
 * @brief Initialize FujiNet drivers and configurations.
 */
void fujinet_init_system(VMContext *vm);

/**
 * @brief Clean up and close FujiNet system resources.
 */
void fujinet_shutdown_system(void);

/**
 * @brief Create the 'N:' network adapter virtual device.
 */
VDev fujinet_create_n_dev(VMContext *vm);

/**
 * @brief Create the 'FUJI:' configuration virtual device.
 */
VDev fujinet_create_fuji_dev(VMContext *vm);

/**
 * @brief Create the 'CLOCK:' network time virtual device.
 */
VDev fujinet_create_clock_dev(VMContext *vm);

#endif /* BPP_FUJINET_H */

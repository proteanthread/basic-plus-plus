/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file fujinet.h
 * @brief Device component implementation and public API surface for fujinet.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for fujinet.h within the device subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file fujinet.h
 * @brief FujiNet Virtual Device Driver API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares functions to initialize, create, and shut down
 *   the VDev2 drivers for N:, FUJI:, and CLOCK: virtual devices.
 * - Why it exists: Emulates FujiNet hardware network, configuration, and time interfaces (Phase 20).
 * - Why it works this way: It bridges standard BASIC++ I/O channel calls to Winsock/POSIX sockets and host OS time.
 */

#ifndef DEVICE_FUJINET_H
#define DEVICE_FUJINET_H

#include "device/vdev.h"
#include "vm/vm.h"

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

#endif /* DEVICE_FUJINET_H */

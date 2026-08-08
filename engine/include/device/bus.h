/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bus.h
 * @brief Device component implementation and public API surface for bus.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for bus.h within the device subsystem.
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
 * @file bus.h
 * @brief Virtual Hardware Port & MMIO Bus interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares routing callbacks for INP/OUT port instructions and
 *   PEEK/POKE conventional memory intercepts.
 * - Why it exists: Connects execution engines directly to emulated PC peripheral chips
 *   and BIOS structures.
 */

#ifndef DEVICE_BUS_H
#define DEVICE_BUS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BIOS_MODEL_NONE = 0,
    BIOS_MODEL_MSDOS,
    BIOS_MODEL_IBMPC,
    BIOS_MODEL_PCJR,
    BIOS_MODEL_PCXT,
    BIOS_MODEL_PCAT,
    BIOS_MODEL_APPLE2,
    BIOS_MODEL_C64,
    BIOS_MODEL_VIC20,
    BIOS_MODEL_ATARI,
    BIOS_MODEL_TANDY
} MockBiosModel;

void vdev_bus_out(int port, int value);
int  vdev_bus_in(int port);

uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted);
void    vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted);

/* Port handler registration */
bool vdev_bus_register_port(int start, int end, int (*read_fn)(int), void (*write_fn)(int, int));
void vdev_bus_reset(void);

/* Mock BIOS linkage functions */
void vdev_bus_set_model(MockBiosModel model);
MockBiosModel vdev_bus_get_model(void);
void vdev_bus_set_ram(uint8_t *ram, size_t size);

#endif /* DEVICE_BUS_H */

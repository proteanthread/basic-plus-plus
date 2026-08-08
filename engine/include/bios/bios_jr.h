/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bios_jr.h
 * @brief Bios component implementation and public API surface for bios_jr.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for bios_jr.h within the bios subsystem.
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
 * @file bios_jr.h
 * @brief IBM PCjr 4860 BIOS definitions (Video Array 0x3DA, SN76496 PSG 0xC0).
 */

#ifndef ENGINE_BIOS_BIOS_JR_H
#define ENGINE_BIOS_BIOS_JR_H

#include "bios/bios.h"

#ifdef __cplusplus
extern "C" {
#endif

void bios_jr_init(BiosContext* ctx);
uint8_t bios_jr_inp(BiosContext* ctx, uint16_t port);
void bios_jr_out(BiosContext* ctx, uint16_t port, uint8_t val);
bool bios_jr_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs);

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_BIOS_BIOS_JR_H */

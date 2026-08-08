/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file editor.h
 * @brief Editor component implementation and public API surface for editor.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for editor.h within the editor subsystem.
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
#ifndef EDITOR_H
#define EDITOR_H

#include "vm/vm.h"
#include <stdbool.h>

/**
 * Common editor plugin interface.
 */
typedef struct {
    const char *name;
    int (*run_editor)(VMContext *vm, const char *filename);
} BppEditorPlugin;

void tui_multiplexer_init(void);
void tui_multiplexer_shutdown(void);

void editor_manager_init(VMContext *vm);
int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename);

/* Plugin entry points */
int mod_ws_main(VMContext *vm, const char *filename);
int mod_vi_main(VMContext *vm, const char *filename);
int mod_edit_main(VMContext *vm, const char *filename);
int mod_edlin_main(VMContext *vm, const char *filename);

#endif /* EDITOR_H */

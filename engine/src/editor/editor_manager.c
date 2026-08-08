/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file editor_manager.c
 * @brief Editor component implementation and public API surface for editor_manager.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for editor_manager.c within the editor subsystem.
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

#include "editor/editor.h"
#include <string.h>

void editor_manager_init(VMContext *vm) {
    (void)vm;
}

int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename) {
    if (!editor_name) return 0;
    
    if (strcmp(editor_name, "ws") == 0) {
        return mod_ws_main(vm, filename);
    } else if (strcmp(editor_name, "vi") == 0) {
        return mod_vi_main(vm, filename);
    } else if (strcmp(editor_name, "edit") == 0) {
        return mod_edit_main(vm, filename);
    } else if (strcmp(editor_name, "edlin") == 0) {
        return mod_edlin_main(vm, filename);
    }
    
    return -1;
}

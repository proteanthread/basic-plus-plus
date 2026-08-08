/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file spec.h
 * @brief Runtime component implementation and public API surface for spec.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for spec.h within the runtime subsystem.
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
 * @file spec.h
 * @brief Dynamic Keyword Specification & Feature Registry API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines structures and functions for loading, parsing, and registering
 *   custom keywords (.spec, .yaml, or inline) and binding them to library-space logic.
 * - Why it exists: Enables developers to define new keywords dynamically (Phase 19).
 * - Why it works this way: It maintains a registry of loaded SpecObjects, mapping names
 *   to dynamic lexer keyword IDs and companion BASIC++ library file paths.
 */

#ifndef RUNTIME_SPEC_H
#define RUNTIME_SPEC_H

#include <stdbool.h>

#include "lexer/lexer.h"
#include "vm/vm.h"

#define MAX_SPECS 128

typedef enum {
    SPEC_CAT_STATEMENT = 0,
    SPEC_CAT_FUNCTION
} SpecCategory;

typedef struct {
    char         name[64];
    char         version[16];
    SpecCategory category;
    char         lib_path[256];
    char         required_level[32];
    BppKeywordId kw_id;
    bool         lib_loaded;
} SpecObject;

/**
 * @brief Initialize the specification registry.
 */
void spec_registry_init(void);

/**
 * @brief Load a specification from an external file (.spec or .yaml).
 */
int spec_load_file(VMContext *vm, const char *filename);

/**
 * @brief Load all companion libraries for registered specifications.
 */
int spec_load_companion_libraries(VMContext *vm, const char *dir_part);

/**
 * @brief Register an inline specification from a metadata block context.
 */
int spec_register_inline(VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level);

/**
 * @brief Look up a specification object by its name.
 */
SpecObject *spec_find_by_name(const char *name);

/**
 * @brief Look up a specification object by its assigned keyword ID.
 */
SpecObject *spec_find_by_kw_id(BppKeywordId kw_id);

/**
 * @brief Get the total number of registered specifications.
 */
int spec_get_count(void);

/**
 * @brief Retrieve a registered specification by index.
 */
SpecObject *spec_get_by_index(int idx);

#endif /* RUNTIME_SPEC_H */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_spec.h
 * @brief Dynamic Keyword Specification & Feature Registry API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines structures and functions for loading, parsing, and registering
 *   custom keywords (.spec, .yaml, or inline) and binding them to library-space logic.
 * - Why it exists: Enables developers to define new keywords dynamically (Phase 19).
 * - Why it works this way: It maintains a registry of loaded SpecObjects, mapping names
 *   to dynamic lexer keyword IDs and companion BASIC++ library file paths.
 */

#ifndef BPP_SPEC_H
#define BPP_SPEC_H

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

#endif /* BPP_SPEC_H */

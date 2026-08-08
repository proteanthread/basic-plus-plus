/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file keyword_props.h
 * @brief Header for KEYWORD runtime metadata properties subsystem.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares API functions for attached keyword properties (category, flags, deprecation status,
 *   docstrings, and custom metadata key-value attributes).
 * - Why it exists: Enables dynamic keyword introspection, tooling, and property inspection.
 * - Why it works this way: Keyword properties are stored in a key-value attribute table attached to VMContext.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Property key strings and maximum property capacity limits.
 * - What cannot be changed: Fundamental property query and set function signatures.
 * - What to expect: Pure interface declarations with self-contained direct header includes.
 * - What to do if something breaks: Ensure prerequisite types.h and vm.h headers are included.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 compliant, case-insensitive keyword name matching.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add pre-defined property key macros (KW_PROP_CATEGORY, KW_PROP_DEPRECATED).
 */

#ifndef KEYWORD_PROPS_H
#define KEYWORD_PROPS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#define MAX_KEYWORD_PROPERTIES 256

typedef struct {
    char keyword[64];
    char property_name[64];
    char property_value[256];
} BppKeywordProperty;

typedef struct {
    BppKeywordProperty props[MAX_KEYWORD_PROPERTIES];
    int count;
} BppKeywordPropTable;

/**
 * @brief Initialize keyword properties subsystem.
 */
void kw_props_init(VMContext *vm);

/**
 * @brief Clear all keyword properties.
 */
void kw_props_clear(VMContext *vm);

/**
 * @brief Set or update a property value on a keyword.
 */
bool kw_props_set_property(VMContext *vm, const char *kw, const char *prop_name, const char *prop_val);

/**
 * @brief Get a property value from a keyword. Returns NULL if not set.
 */
const char *kw_props_get_property(VMContext *vm, const char *kw, const char *prop_name);

/**
 * @brief Print all set properties for a keyword via vdev_printf.
 */
void kw_props_list(VMContext *vm, const char *kw);

#endif /* KEYWORD_PROPS_H */

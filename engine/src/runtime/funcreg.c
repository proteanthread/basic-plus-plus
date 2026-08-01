/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file funcreg.c
 * @brief Dynamic Function Registry system implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Manages registration, overriding, counting, and retrieving callable functions.
 * - Why it exists: Provides dynamic modular vocabulary expansion securely.
 * - Why it works this way: It stores registered entries in a linear array. Lookups perform case-insensitive
 *   string matching.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Limits on MAX_FUNCTIONS.
 * - What cannot be changed: Case-insensitive name comparisons, safety checks, and basic entry layouts.
 * - What to expect: funcreg_register copies entries by value.
 * - What to do if something breaks: Trace indices and verify that names do not contain spaces.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17.
 * - Portability concerns: Case-insensitive lookups use standard helper functions to ensure portability.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Upgrade the lookup array to a hash table or binary search tree.
 * - How to write external extensions: Declare static FunctionEntry lists and call funcreg_register.
 */

#include "runtime/funcreg.h"
#include "module/module.h"
#include <string.h>
#include <ctype.h>

static FunctionEntry funcreg_table[MAX_FUNCTIONS];
static int           funcreg_table_count = 0;
static const char   *current_registering_module = NULL;

static int str_iequal(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void funcreg_init(void) {
    memset(funcreg_table, 0, sizeof(funcreg_table));
    funcreg_table_count = 0;
    current_registering_module = NULL;
}

void funcreg_set_registering_module(const char *name) {
    current_registering_module = name;
}

int funcreg_register(const FunctionEntry *entry) {
    if (!entry || !entry->name) return -1;

    if (funcreg_table_count >= MAX_FUNCTIONS) {
        return -1;
    }

    /* Check for duplicates by name */
    for (int i = 0; i < funcreg_table_count; i++) {
        if (str_iequal(funcreg_table[i].name, entry->name)) {
            /* Duplicate name - override the entry */
            funcreg_table[i] = *entry;
            funcreg_table[i].module_name = current_registering_module;
            return 0;
        }
    }

    funcreg_table[funcreg_table_count] = *entry;
    funcreg_table[funcreg_table_count].module_name = current_registering_module;
    funcreg_table_count++;
    return 0;
}

const FunctionEntry *funcreg_find_by_name(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < funcreg_table_count; i++) {
        if (str_iequal(funcreg_table[i].name, name)) {
            if (funcreg_table[i].module_name) {
                if (!module_is_active(funcreg_table[i].module_name)) {
                    continue; /* Skip inactive module functions */
                }
            }
            return &funcreg_table[i];
        }
    }
    return NULL;
}

const FunctionEntry *funcreg_find_by_keyword(BppKeywordId kw) {
    if (kw == KW_NONE) return NULL;
    for (int i = 0; i < funcreg_table_count; i++) {
        if (funcreg_table[i].keyword == kw) {
            return &funcreg_table[i];
        }
    }
    return NULL;
}

int funcreg_override(BppKeywordId kw, FuncHandler handler) {
    if (kw == KW_NONE || !handler) return -1;
    for (int i = 0; i < funcreg_table_count; i++) {
        if (funcreg_table[i].keyword == kw) {
            funcreg_table[i].handler = handler;
            return 0;
        }
    }
    return -1;
}

int funcreg_count(void) {
    return funcreg_table_count;
}

const FunctionEntry *funcreg_get(int index) {
    if (index < 0 || index >= funcreg_table_count) return NULL;
    return &funcreg_table[index];
}

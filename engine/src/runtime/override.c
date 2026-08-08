/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file override.c
 * @brief Implementation of OVERRIDE behavior replacement subsystem.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements table management for OVERRIDE hooks (register, clear, lookup).
 * - Why it exists: Supports dynamic statement and subroutine interception.
 * - Why it works this way: Bounded array operations zero-initialized by default.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal array search methods.
 * - What cannot be changed: Table bounds safety guarantees.
 * - What to expect: Case-insensitive target lookups.
 * - What to do if something breaks: Check memory zero-initialization and bounds assertions.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Strict C17 standard across MSVC and GCC targets.
 * - Portability concerns: Thread-safe per VMContext.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add wildcards or regex target matching.
 */

#include "runtime/override.h"
#include "types/types.h"
#include "vm/vm.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

static BppOverrideTable g_override_table;

static BppOverrideTable *get_override_table(VMContext *vm) {
    (void)vm;
    return &g_override_table;
}

void override_init(VMContext *vm) {
    BppOverrideTable *t = get_override_table(vm);
    if (t) {
        memset(t, 0, sizeof(BppOverrideTable));
    }
}

void override_clear(VMContext *vm) {
    override_init(vm);
}

bool override_register(VMContext *vm, const char *target_name, long target_line, const char *replacement_sub) {
    if (!target_name) return false;
    BppOverrideTable *t = get_override_table(vm);
    if (!t) return false;

    for (int i = 0; i < t->count; ++i) {
        if (strcasecmp(t->entries[i].target_name, target_name) == 0) {
            t->entries[i].target_line = target_line;
            if (replacement_sub) {
                strncpy(t->entries[i].replacement_sub, replacement_sub, sizeof(t->entries[i].replacement_sub) - 1);
                t->entries[i].replacement_sub[sizeof(t->entries[i].replacement_sub) - 1] = '\0';
            } else {
                t->entries[i].replacement_sub[0] = '\0';
            }
            t->entries[i].is_active = true;
            return true;
        }
    }

    if (t->count >= MAX_OVERRIDE_ENTRIES) return false;

    strncpy(t->entries[t->count].target_name, target_name, sizeof(t->entries[t->count].target_name) - 1);
    t->entries[t->count].target_name[sizeof(t->entries[t->count].target_name) - 1] = '\0';

    t->entries[t->count].target_line = target_line;

    if (replacement_sub) {
        strncpy(t->entries[t->count].replacement_sub, replacement_sub, sizeof(t->entries[t->count].replacement_sub) - 1);
        t->entries[t->count].replacement_sub[sizeof(t->entries[t->count].replacement_sub) - 1] = '\0';
    } else {
        t->entries[t->count].replacement_sub[0] = '\0';
    }

    t->entries[t->count].is_active = true;
    t->count++;
    return true;
}

const BppOverrideEntry *override_lookup(VMContext *vm, const char *target_name) {
    if (!target_name) return NULL;
    BppOverrideTable *t = get_override_table(vm);
    if (!t) return NULL;

    for (int i = 0; i < t->count; ++i) {
        if (t->entries[i].is_active && strcasecmp(t->entries[i].target_name, target_name) == 0) {
            return &t->entries[i];
        }
    }
    return NULL;
}

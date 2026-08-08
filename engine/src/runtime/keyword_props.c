/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file keyword_props.c
 * @brief Implementation of KEYWORD metadata properties subsystem.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements storage, retrieval, clearing, and listing of key-value properties for BASIC keywords.
 * - Why it exists: Supports introspection, metadata tags, and dynamic property assignment.
 * - Why it works this way: Bounded string array allocations zero-initialized by default.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal property table lookup algorithms.
 * - What cannot be changed: Property bounds and string copy safety guarantees.
 * - What to expect: Case-insensitive keyword and property name lookups.
 * - What to do if something breaks: Check string buffer sizes and bounds assertions.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 compliant across MSVC and GCC targets.
 * - Portability concerns: Thread-safe within VMContext bounds.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add fast hash table lookup for properties if count grows.
 */

#include "runtime/keyword_props.h"
#include "types/types.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

static BppKeywordPropTable g_kw_prop_table;

static BppKeywordPropTable *get_prop_table(VMContext *vm) {
    (void)vm;
    return &g_kw_prop_table;
}

void kw_props_init(VMContext *vm) {
    BppKeywordPropTable *t = get_prop_table(vm);
    if (t) {
        memset(t, 0, sizeof(BppKeywordPropTable));
    }
}

void kw_props_clear(VMContext *vm) {
    kw_props_init(vm);
}

bool kw_props_set_property(VMContext *vm, const char *kw, const char *prop_name, const char *prop_val) {
    if (!kw || !prop_name || !prop_val) return false;
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return false;

    for (int i = 0; i < t->count; ++i) {
        if (strcasecmp(t->props[i].keyword, kw) == 0 && strcasecmp(t->props[i].property_name, prop_name) == 0) {
            strncpy(t->props[i].property_value, prop_val, sizeof(t->props[i].property_value) - 1);
            t->props[i].property_value[sizeof(t->props[i].property_value) - 1] = '\0';
            return true;
        }
    }

    if (t->count >= MAX_KEYWORD_PROPERTIES) return false;

    strncpy(t->props[t->count].keyword, kw, sizeof(t->props[t->count].keyword) - 1);
    t->props[t->count].keyword[sizeof(t->props[t->count].keyword) - 1] = '\0';

    strncpy(t->props[t->count].property_name, prop_name, sizeof(t->props[t->count].property_name) - 1);
    t->props[t->count].property_name[sizeof(t->props[t->count].property_name) - 1] = '\0';

    strncpy(t->props[t->count].property_value, prop_val, sizeof(t->props[t->count].property_value) - 1);
    t->props[t->count].property_value[sizeof(t->props[t->count].property_value) - 1] = '\0';

    t->count++;
    return true;
}

const char *kw_props_get_property(VMContext *vm, const char *kw, const char *prop_name) {
    if (!kw || !prop_name) return NULL;
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return NULL;

    for (int i = 0; i < t->count; ++i) {
        if (strcasecmp(t->props[i].keyword, kw) == 0 && strcasecmp(t->props[i].property_name, prop_name) == 0) {
            return t->props[i].property_value;
        }
    }
    return NULL;
}

void kw_props_list(VMContext *vm, const char *kw) {
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return;

    vdev_printf(vm_get_vdev(vm), "Properties for keyword '%s':\n", kw ? kw : "*");
    int count = 0;
    for (int i = 0; i < t->count; ++i) {
        if (!kw || strcasecmp(t->props[i].keyword, kw) == 0) {
            vdev_printf(vm_get_vdev(vm), "  [%s] %s = %s\n", t->props[i].keyword, t->props[i].property_name, t->props[i].property_value);
            count++;
        }
    }
    vdev_printf(vm_get_vdev(vm), "Total Properties Found: %d\n", count);
}

// FILENAME: funcreg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext
// NEEDS: libcore (ctype.h, ctype.c, funcreg.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for funcreg within BASIC++.
//
// ---- Includes ----

#include "runtime/funcreg.h"
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

    // Check for duplicates by name
    for (int i = 0; i < funcreg_table_count; i++) {
        if (str_iequal(funcreg_table[i].name, entry->name)) {
            // Duplicate name - override the entry
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

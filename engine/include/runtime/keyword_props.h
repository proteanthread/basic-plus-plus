// FILENAME: keyword_props.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (keyword_props.c)
// NEEDED BY: libengine (exec_internal.h, keyword.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for keyword_props within BASIC++.
//
// ---- Includes ----

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

// @brief Initialize keyword properties subsystem.
void kw_props_init(VMContext *vm);

// @brief Clear all keyword properties.
void kw_props_clear(VMContext *vm);

// @brief Set or update a property value on a keyword.
bool kw_props_set_property(VMContext *vm, const char *kw, const char *prop_name, const char *prop_val);

// @brief Get a property value from a keyword. Returns NULL if not set.
const char *kw_props_get_property(VMContext *vm, const char *kw, const char *prop_name);

// @brief Print all set properties for a keyword via vdev_printf.
void kw_props_list(VMContext *vm, const char *kw);

#endif // KEYWORD_PROPS_H

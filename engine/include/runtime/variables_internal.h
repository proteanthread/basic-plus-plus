// FILENAME: variables_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (var_lookup.c, var_magic.c, var_scope.c, variables.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides core logic and interface definitions for variables_internal within BASIC++.
//
// ---- Includes ----

#ifndef VARIABLES_INTERNAL_H
#define VARIABLES_INTERNAL_H

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug/logger.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/map.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "types/errors.h"
#include "types/types.h"

//
// ---- Hash Table & Cache Constants ----

#define HASH_BUCKETS 1024
#define VAR_CACHE_SIZE 64

//
// ---- Data Structures ----

typedef struct VarEntry {
    char            *name;
    BValue           value;
    struct VarEntry *next;
} VarEntry;

typedef struct ScopeDefMapping {
    char                    scope_name[128];
    ValueType               def_types[26];
    struct ScopeDefMapping *next;
} ScopeDefMapping;

typedef struct VarCacheEntry {
    char    name[64];
    BValue *val_ptr;
    bool    valid;
} VarCacheEntry;

struct VariableContext {
    MemoryContext   *mem;
    StringContext   *str;
    VarEntry        *buckets[HASH_BUCKETS];
    bool             is_explicit;
    char             active_scope[256];
    char             active_namespace[64];
    char             shared_vars[64][64];
    int              shared_count;
    char             common_vars[128][64];
    int              common_count;
    ValueType        global_def_types[26];
    ScopeDefMapping *scope_defs;
    bool             case_sensitive;

    VarCacheEntry    mru_cache[VAR_CACHE_SIZE];
    uint8_t          mru_head;
};

//
// ---- Helper Functions ----

int basic_strcasecmp(const char *s1, const char *s2);
char *basic_strdup(const char *src);
unsigned int hash_name(VariableContext *ctx, const char *name);
void normalize_name(VariableContext *ctx, char *out, const char *in, size_t max_len);
bool is_magic_virtual_var(const char *name);
BValue *get_magic_virtual_var(VariableContext *ctx, const char *norm_name);

#endif // VARIABLES_INTERNAL_H

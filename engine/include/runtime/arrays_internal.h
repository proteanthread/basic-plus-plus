// FILENAME: arrays_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (arr_access.c, arr_core.c, arr_persist.c, arrays.c)
// NEEDS: libcore (alloc.h, alloc.c, arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (hal.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (map.h, map.c)
// Provides core logic and interface definitions for arrays_internal within BASIC++.
//
// ---- Includes ----

#ifndef ARRAYS_INTERNAL_H
#define ARRAYS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#include "runtime/arrays.h"
#include "runtime/map.h"
#include "runtime/variables.h"


//
// ---- Constants and Struct Definitions ----

#define HASH_BUCKETS 512
#define MRU_ARRAY_CACHE_SIZE 16

typedef struct ArrayEntry {
    char              *name;
    ValueType          type;
    int                num_dims;
    int                bounds[4];
    BValue            *elements;
    int                total_size;
    int                channel;
    bool               is_alias;
    struct ArrayEntry *next;
} ArrayEntry;

typedef struct {
    char        name[64];
    ArrayEntry *entry;
} ArrayMruCacheEntry;

struct ArrayContext {
    MemoryContext      *mem;
    StringContext      *str;
    ArrayEntry         *buckets[HASH_BUCKETS];
    ArrayMruCacheEntry  mru_cache[MRU_ARRAY_CACHE_SIZE];
    int                 mru_head;
    int                 option_base;
    double              last_det;
};

//
// ---- Internal Helpers ----

static inline unsigned int hash_name(const char *name) {
    unsigned int hash = 5381;
    while (*name) {
        hash = ((hash << 5) + hash) + (unsigned int)runtime_toupper((unsigned char)*name);
        name++;
    }
    return hash % HASH_BUCKETS;
}

static inline void normalize_name(char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = (char)runtime_toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}


static inline char *basic_strdup(const char *src) {
    if (!src) return NULL;
    size_t len = runtime_strlen(src);
    HalContext *hal = hal_get();
    char *dest = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(len + 1) : NULL);
    if (dest) {
        runtime_memcpy(dest, src, len + 1);
    }
    return dest;
}


void free_entry(ArrayContext *ctx, ArrayEntry *entry);
int get_flat_index(int option_base, int num_dims, const int *bounds, const int *indices);
bool auto_expand_array(ArrayContext *ctx, ArrayEntry *entry, const int *indices);

#endif // ARRAYS_INTERNAL_H

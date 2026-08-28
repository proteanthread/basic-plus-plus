// FILENAME: parser.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (stmt.h, string.c)
// Implements AST parsing and evaluation structures for parser.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    BppKeywordId   kw;
    BppStmtHandler handler;
    const char    *name;
    uint32_t       flags;
} StmtEntry;

struct StmtRegistry {
    MemoryContext *mem;
    StmtEntry     *entries;
    size_t         count;
    size_t         capacity;
};

StmtRegistry *stmt_registry_init(MemoryContext *mem) {
    if (!mem) return NULL;
    StmtRegistry *reg = (StmtRegistry *)calloc(1, sizeof(StmtRegistry));
    if (!reg) return NULL;
    reg->mem = mem;
    reg->capacity = 32;
    reg->count = 0;
    reg->entries = (StmtEntry *)calloc(reg->capacity, sizeof(StmtEntry));
    if (!reg->entries) {
        free(reg);
        return NULL;
    }
    return reg;
}

void stmt_registry_shutdown(StmtRegistry *reg) {
    if (!reg) return;
    free(reg->entries);
    free(reg);
}

void stmt_register(StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags) {
    if (!reg || !handler) return;

    // Check if already registered (override if exists)
    for (size_t i = 0; i < reg->count; ++i) {
        if (reg->entries[i].kw == kw) {
            reg->entries[i].handler = handler;
            reg->entries[i].name = name;
            reg->entries[i].flags = flags;
            return;
        }
    }

    // Grow registry if full
    if (reg->count >= reg->capacity) {
        size_t new_cap = reg->capacity * 2;
        StmtEntry *new_entries = (StmtEntry *)realloc(reg->entries, new_cap * sizeof(StmtEntry));
        if (!new_entries) return;
        memset(&new_entries[reg->capacity], 0, (new_cap - reg->capacity) * sizeof(StmtEntry));
        reg->entries = new_entries;
        reg->capacity = new_cap;
    }

    reg->entries[reg->count].kw = kw;
    reg->entries[reg->count].handler = handler;
    reg->entries[reg->count].name = name;
    reg->entries[reg->count].flags = flags;
    reg->count++;
}

BppStmtHandler stmt_lookup(StmtRegistry *reg, BppKeywordId kw) {
    if (!reg) return NULL;

    for (size_t i = 0; i < reg->count; ++i) {
        if (reg->entries[i].kw == kw) {
            return reg->entries[i].handler;
        }
    }

    return NULL;
}

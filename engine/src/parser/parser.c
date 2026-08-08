/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file parser.c
 * @brief Statement Registration Table and Command Dispatcher implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `stmt_registry_init()`, `stmt_registry_cleanup()`, `stmt_register()`, `stmt_lookup()`, and keyword-to-handler dispatch lookups.
 *
 * 2. WHY IT EXISTS:
 * Decouples statement parsing and handler dispatch from the VM core, allowing modular keyword handler registration without editing VM switch statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Stores registered statement handlers (`BppStmtHandler`) keyed by `BppKeywordId` in dynamic arrays (`StmtEntry`), performing fast lookups for the VM execution loop.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "stmt/stmt.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Register new keyword handlers via `stmt_register()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * `BppStmtHandler` function signature contract: `(VMContext *ctx, BValue *result)`.
 *
 * 8. WHAT TO EXPECT:
 * `stmt_lookup(kw)` returns pointer to registered `BppStmtHandler` callback or NULL if statement is unhandled.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check keyword registration sequence during VM boot in `boot.c`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * `stmt_registry_init()` executed before boot registration loop.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero pointer casting to integer types.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/lexer/lexer.c
 * Prerequisite Header Files:
 * - engine/include/stmt/stmt.h
 * - engine/include/lexer/lexer.h
 */

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

    /* Check if already registered (override if exists) */
    for (size_t i = 0; i < reg->count; ++i) {
        if (reg->entries[i].kw == kw) {
            reg->entries[i].handler = handler;
            reg->entries[i].name = name;
            reg->entries[i].flags = flags;
            return;
        }
    }

    /* Grow registry if full */
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

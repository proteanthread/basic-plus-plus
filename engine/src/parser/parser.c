/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file parser.c
 * @brief Statement Registry and Command Dispatcher implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements statement registration and command lookup. Matches keywords
 *   against registered statement handlers.
 * - Why it exists: Decouples statement logic from the core parser dispatch loop. Prevents
 *   regression errors where adding a command breaks parsing of others.
 * - Why it works this way: It maintains a dynamic array of entries containing the KeywordId, name,
 *   flags, and BppStmtHandler callback pointer. Dispatches are routed via linear scan (very fast
 *   for typical statement counts).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal lookup structures (hash map vs sorted registry), capacities.
 * - What cannot be changed: Callback structures conforming to the BppStmtHandler signature.
 * - What to expect: Registering a command adds it dynamically to the parser's lookup targets.
 * - What to do if something breaks: If statements aren't dispatched or return unknown, verify
 *   that the statement registered at boot and that the lexer produces the correct KeywordId.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Handlers return a structured error. Linear scanning is sufficient for speed.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add statement alias mappings or pre-dispatch validations.
 * - How to write external extensions: External plugins call stmt_register at load time to add
 *   custom commands to the execution registry.
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

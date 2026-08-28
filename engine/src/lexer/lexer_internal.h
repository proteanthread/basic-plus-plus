// FILENAME: lexer_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (lexer.c, scan_keyword.c, scan_number.c, scan_string.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libengine (lexer.h, lexer.c)
// Implements lexical scanning and token stream processing for lexer_internal.
//
// ---- Includes ----

#ifndef LEXER_INTERNAL_H
#define LEXER_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer/lexer.h"
#include "memory/memory.h"

//
// ---- Internal Types & Constants ----

#define MAX_CUSTOM_KEYWORDS 128

typedef struct {
    char         name[64];
    BppKeywordId id;
} CustomKeywordMap;

typedef struct {
    const char  *name;
    BppKeywordId id;
} KeywordMap;

// opaque lexer context structure
struct LexerContext {
    MemoryContext *mem;
    const char    *source;
    const char    *pos;
};

//
// ---- Internal Function Prototypes ----

bool        scan_is_kw_bound(char c);
bool        scan_match_directive(const char *start, int len, const char *target);
bool        scan_is_followed_by_assignment(const char *pos);
void        scan_skip_whitespace(LexerContext *ctx);

bool        scan_try_radix_number(LexerContext *ctx, BppToken *tok);
bool        scan_try_decimal_number(LexerContext *ctx, BppToken *tok);
bool        scan_try_docstring(LexerContext *ctx, BppToken *tok);
bool        scan_try_directive_or_label(LexerContext *ctx, BppToken *tok);
bool        scan_try_string_literal(LexerContext *ctx, BppToken *tok);
bool        scan_try_rpn_literal(LexerContext *ctx, BppToken *tok);

#endif // LEXER_INTERNAL_H

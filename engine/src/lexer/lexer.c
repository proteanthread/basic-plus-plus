// FILENAME: lexer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, memory.h, memory.c, strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer_internal.h)
// NEEDS: libkernel (config.h)
// Implements component functionality for lexer.c.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "memory/memory.h"
#include "types/config.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "hal/hal.h"

//
// ---- Lifecycle & State Management ----
//

// initializes a new lexer scanner state for a given null-terminated source buffer
LexerContext *lex_init(MemoryContext *mem, const char *source) {
    if (!source) return NULL;
    HalContext *hal = hal_get();
    LexerContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (LexerContext *)hal->mem.alloc(sizeof(LexerContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(LexerContext));
    ctx->mem = mem;
    ctx->source = source;
    ctx->pos = source;
    return ctx;
}

// releases memory held by the lexer context
void lex_shutdown(LexerContext *ctx) {
    if (!ctx) return;
    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(ctx);
    }
}

// returns current character read position within the source buffer
const char *lex_get_pos(LexerContext *ctx) {
    return ctx ? ctx->pos : NULL;
}

// updates scanner position pointer for rewinding or fast-forwarding
void lex_set_pos(LexerContext *ctx, const char *pos) {
    if (ctx && pos) {
        ctx->pos = pos;
    }
}

//
/// ---- Identifier & Keyword Lookahead Scanner ----

// scans an identifier or keyword starting with an alphabetic character or underscore
static BppToken scan_ident_or_keyword(LexerContext *ctx) {
    BppToken tok;
    runtime_memset(&tok, 0, sizeof(tok));
    const char *start = ctx->pos;
    tok.start = start;

    bool is_start = true;
    const char *p = start - 1;
    while (p >= ctx->source && runtime_isspace((unsigned char)*p)) {
        p--;
    }
    if (p >= ctx->source) {
        if (*p == ':') {
            is_start = true;
        } else if (p >= ctx->source + 3 && runtime_strncasecmp(p - 3, "THEN", 4) == 0) {
            is_start = true;
        } else if (p >= ctx->source + 3 && runtime_strncasecmp(p - 3, "ELSE", 4) == 0) {
            is_start = true;
        } else {
            const char *ln = p;
            while (ln >= ctx->source && runtime_isdigit((unsigned char)*ln)) ln--;
            while (ln >= ctx->source && runtime_isspace((unsigned char)*ln)) ln--;
            is_start = (ln < ctx->source);
        }
    }

    if (is_start && runtime_strncasecmp(start, "REM", 3) == 0 &&
        runtime_strncasecmp(start, "REMOVE", 6) != 0 && runtime_strncasecmp(start, "REMOVE$", 7) != 0) {
        if (!scan_is_followed_by_assignment(start + 3)) {
            while (*ctx->pos && *ctx->pos != '\n') {
                ctx->pos++;
            }
            tok.type = TOK_EOL;
            tok.length = ctx->pos - tok.start;
            tok.as.keyword = KW_REM;
            return tok;
        }
    }

    if (is_start && runtime_strncasecmp(start, "LET", 3) == 0 && scan_is_kw_bound(start[3])) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_LET;
        tok.length = 3;
        ctx->pos = start + 3;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "PRINT", 5) == 0 && (scan_is_kw_bound(start[5]) || (!scan_is_followed_by_assignment(start + 5) && runtime_isalnum((unsigned char)start[5])))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_PRINT;
        tok.length = 5;
        ctx->pos = start + 5;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "INPUT", 5) == 0 && (scan_is_kw_bound(start[5]) || (!scan_is_followed_by_assignment(start + 5) && runtime_isalnum((unsigned char)start[5])))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_INPUT;
        tok.length = 5;
        ctx->pos = start + 5;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "DIM", 3) == 0 && (scan_is_kw_bound(start[3]) || (!scan_is_followed_by_assignment(start + 3) && runtime_isalnum((unsigned char)start[3])))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_DIM;
        tok.length = 3;
        ctx->pos = start + 3;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "READ", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isalpha((unsigned char)start[4]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_READ;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "DATA", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isdigit((unsigned char)start[4]) || start[4] == '-' || start[4] == '.' || start[4] == '"')) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_DATA;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "RESTORE", 7) == 0 && (scan_is_kw_bound(start[7]) || runtime_isdigit((unsigned char)start[7]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_RESTORE;
        tok.length = 7;
        ctx->pos = start + 7;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "RETURN", 6) == 0 && scan_is_kw_bound(start[6])) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_RETURN;
        tok.length = 6;
        ctx->pos = start + 6;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "FOR", 3) == 0 && runtime_strncasecmp(start, "FORM", 4) != 0 && runtime_strncasecmp(start, "FORMAT", 6) != 0 && (scan_is_kw_bound(start[3]) || runtime_isalpha((unsigned char)start[3]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_FOR;
        tok.length = 3;
        ctx->pos = start + 3;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "NEXT", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isalpha((unsigned char)start[4]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_NEXT;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "IF", 2) == 0 && (scan_is_kw_bound(start[2]) || runtime_isalnum((unsigned char)start[2]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_IF;
        tok.length = 2;
        ctx->pos = start + 2;
        return tok;
    }

    if (is_start && runtime_strncasecmp(start, "ON", 2) == 0 && (scan_is_kw_bound(start[2]) || (!scan_is_followed_by_assignment(start + 2) && runtime_isalnum((unsigned char)start[2])))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_ON;
        tok.length = 2;
        ctx->pos = start + 2;
        return tok;
    }

    if (runtime_strncasecmp(start, "GOTO", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isdigit((unsigned char)start[4]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_GOTO;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (runtime_strncasecmp(start, "GOSUB", 5) == 0 && (scan_is_kw_bound(start[5]) || runtime_isdigit((unsigned char)start[5]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_GOSUB;
        tok.length = 5;
        ctx->pos = start + 5;
        return tok;
    }

    if (runtime_strncasecmp(start, "THEN", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isalnum((unsigned char)start[4]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_THEN;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (runtime_strncasecmp(start, "ELSE", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isalnum((unsigned char)start[4]))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_ELSE;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    if (runtime_strncasecmp(start, "TO", 2) == 0 && (start[2] == '\0' || (!runtime_isalpha((unsigned char)start[2]) && start[2] != '_'))) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_TO;
        tok.length = 2;
        ctx->pos = start + 2;
        return tok;
    }

    if (runtime_strncasecmp(start, "STEP", 4) == 0 && (scan_is_kw_bound(start[4]) || runtime_isdigit((unsigned char)start[4]) || start[4] == '-')) {
        tok.type = TOK_KEYWORD;
        tok.as.keyword = KW_STEP;
        tok.length = 4;
        ctx->pos = start + 4;
        return tok;
    }

    ctx->pos++;
    while (runtime_isalnum((unsigned char)*ctx->pos) || *ctx->pos == '_' || *ctx->pos == '.') {
        if (*ctx->pos == '.') {
            size_t prefix_len = (size_t)(ctx->pos - start);
            if (prefix_len < 64) {
                char prefix[64];
                runtime_memcpy(prefix, start, prefix_len);
                prefix[prefix_len] = '\0';
                if (lex_find_keyword_by_name(prefix) != KW_NONE) {
                    break;
                }
            }
        }
        bool is_short_var = (ctx->pos == start + 1 || (ctx->pos == start + 2 && runtime_isdigit((unsigned char)start[1])));
        if (is_short_var) {
            if ((runtime_strncasecmp(ctx->pos, "GOTO", 4) == 0 && (scan_is_kw_bound(ctx->pos[4]) || runtime_isdigit((unsigned char)ctx->pos[4]))) ||
                (runtime_strncasecmp(ctx->pos, "GOSUB", 5) == 0 && (scan_is_kw_bound(ctx->pos[5]) || runtime_isdigit((unsigned char)ctx->pos[5]))) ||
                (runtime_strncasecmp(ctx->pos, "THEN", 4) == 0 && (scan_is_kw_bound(ctx->pos[4]) || runtime_isalnum((unsigned char)ctx->pos[4]))) ||
                (runtime_strncasecmp(ctx->pos, "ELSE", 4) == 0 && (scan_is_kw_bound(ctx->pos[4]) || runtime_isalnum((unsigned char)ctx->pos[4])))) {
                break;
            }
        }
        ctx->pos++;
    }

    if (*ctx->pos == '$' || *ctx->pos == '%' || *ctx->pos == '&' || *ctx->pos == '!' || *ctx->pos == '#') {
        ctx->pos++;
        if (*ctx->pos == '%' || *ctx->pos == '$' || *ctx->pos == 'F' || *ctx->pos == 'f') {
            size_t base_len = (size_t)(ctx->pos - start - 1);
            if ((base_len == 3 && runtime_strncasecmp(start, "CVT", 3) == 0) ||
                (base_len == 4 && runtime_strncasecmp(start, "SWAP", 4) == 0) ||
                (base_len == 5 && runtime_strncasecmp(start, "_SWAP", 5) == 0)) {
                ctx->pos++;
            }
        }
    }
    tok.length = ctx->pos - start;
    tok.as.string = start;

    char temp_name[64];
    if (tok.length < sizeof(temp_name)) {
        runtime_memcpy(temp_name, start, tok.length);
        temp_name[tok.length] = '\0';
        BppKeywordId kw_id = lex_find_keyword_by_name(temp_name);
        if (kw_id != KW_NONE) {
            if (kw_id == KW_AND) tok.type = TOK_AND;
            else if (kw_id == KW_OR) tok.type = TOK_OR;
            else if (kw_id == KW_NOT) tok.type = TOK_NOT;
            else if (kw_id == KW_XOR) tok.type = TOK_XOR;
            else if (kw_id == KW_EQ) tok.type = TOK_EQ;
            else if (kw_id == KW_NE) tok.type = TOK_NE;
            else if (kw_id == KW_LT) tok.type = TOK_LT;
            else if (kw_id == KW_GT) tok.type = TOK_GT;
            else if (kw_id == KW_LE) tok.type = TOK_LE;
            else if (kw_id == KW_GE) tok.type = TOK_GE;
            else if (kw_id == KW_IMP) tok.type = TOK_IMP;
            else if (kw_id == KW_EQV) tok.type = TOK_EQV;
            else if (kw_id == KW_MOD) tok.type = TOK_MOD;
            else if (kw_id == KW_SHL) tok.type = TOK_SHL;
            else if (kw_id == KW_SHR) tok.type = TOK_SHR;
            else if (kw_id == KW_READBIT) tok.type = TOK_READBIT;
            else if (kw_id == KW_SETBIT) tok.type = TOK_SETBIT;
            else if (kw_id == KW_RESETBIT) tok.type = TOK_RESETBIT;
            else if (kw_id == KW_TOGGLEBIT) tok.type = TOK_TOGGLEBIT;
            else if (kw_id == KW_MIN) tok.type = TOK_MIN;
            else if (kw_id == KW_MAX) tok.type = TOK_MAX;
            else if (kw_id == KW_HYPOT) tok.type = TOK_HYPOT;
            else if (kw_id == KW_REMAINDER) tok.type = TOK_REMAINDER;
            else if (kw_id == KW_ATAN2) tok.type = TOK_ATAN2;
            else {
                tok.type = TOK_KEYWORD;
                tok.as.keyword = kw_id;
            }
            if (tok.as.keyword == KW_REM) {
                while (*ctx->pos && *ctx->pos != '\n') ctx->pos++;
                tok.type = TOK_EOL;
                tok.length = ctx->pos - tok.start;
            }
            return tok;
        }
    }

    tok.type = TOK_IDENT;
    return tok;
}

//
// ---- Main Tokenizer Scanner Loop ----
//

// extracts and consumes the next token from the input stream
BppToken lex_next(LexerContext *ctx) {
    BppToken tok;
    runtime_memset(&tok, 0, sizeof(tok));
    tok.type = TOK_UNKNOWN;

    if (!ctx) {
        tok.type = TOK_EOF;
        return tok;
    }

    scan_skip_whitespace(ctx);
    tok.start = ctx->pos;

    if (scan_try_directive_or_label(ctx, &tok)) return tok;
    if (scan_try_docstring(ctx, &tok)) return tok;

    if (*ctx->pos == '\0') {
        tok.type = TOK_EOF;
        tok.length = 0;
        return tok;
    }

    if (*ctx->pos == '\n' || *ctx->pos == ':') {
        tok.type = TOK_EOL;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }

    if (*ctx->pos == '\'' || (*ctx->pos == '!' && *(ctx->pos + 1) != '=')) {
        while (*ctx->pos && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok.type = TOK_EOL;
        tok.length = ctx->pos - tok.start;
        return tok;
    }

    if (scan_try_rpn_literal(ctx, &tok)) return tok;
    if (scan_try_string_literal(ctx, &tok)) return tok;
    if (scan_try_radix_number(ctx, &tok)) return tok;
    if (scan_try_decimal_number(ctx, &tok)) return tok;

    if (runtime_isalpha((unsigned char)*ctx->pos) || *ctx->pos == '_') {
        return scan_ident_or_keyword(ctx);
    }

    if (*ctx->pos == '<') {
        if (*(ctx->pos + 1) == '=') {
            tok.type = TOK_LE; tok.length = 2; ctx->pos += 2; return tok;
        }
        if (*(ctx->pos + 1) == '>') {
            tok.type = TOK_NE; tok.length = 2; ctx->pos += 2; return tok;
        }
        tok.type = TOK_LT; tok.length = 1; ctx->pos++; return tok;
    }

    if (*ctx->pos == '>') {
        if (*(ctx->pos + 1) == '=') {
            tok.type = TOK_GE; tok.length = 2; ctx->pos += 2; return tok;
        }
        if (*(ctx->pos + 1) == '<') {
            tok.type = TOK_NE; tok.length = 2; ctx->pos += 2; return tok;
        }
        tok.type = TOK_GT; tok.length = 1; ctx->pos++; return tok;
    }

    if (*ctx->pos == '=') {
        if (*(ctx->pos + 1) == '>') {
            tok.type = TOK_GE; tok.length = 2; ctx->pos += 2; return tok;
        }
        if (*(ctx->pos + 1) == '<') {
            tok.type = TOK_LE; tok.length = 2; ctx->pos += 2; return tok;
        }
        tok.type = TOK_EQ; tok.length = 1; ctx->pos++; return tok;
    }

    char c = *ctx->pos;
    ctx->pos++;
    tok.length = 1;

    switch (c) {
        case '+': tok.type = TOK_PLUS; break;
        case '-': tok.type = TOK_MINUS; break;
        case '^': tok.type = TOK_POW; break;
        case '*':
            if (*ctx->pos == '*') { ctx->pos++; tok.length = 2; tok.type = TOK_POW; }
            else { tok.type = TOK_MUL; }
            break;
        case '/': tok.type = TOK_DIV; break;
        case '=': tok.type = TOK_EQ; break;
        case '.': tok.type = TOK_PERIOD; break;
        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case ',': tok.type = TOK_COMMA; break;
        case ';': tok.type = TOK_SEMICOLON; break;
        case '#': tok.type = TOK_HASH; break;
        case '[': tok.type = TOK_LBRACKET; break;
        case ']': tok.type = TOK_RBRACKET; break;
        case '&': tok.type = TOK_AMPERSAND; break;
        case '\\': tok.type = TOK_BACKSLASH; break;
        case '@': tok.type = TOK_AT; break;
        default:  tok.type = TOK_UNKNOWN; break;
    }

    return tok;
}

// looks ahead at the next token without advancing the stream position
BppToken lex_peek(LexerContext *ctx) {
    if (!ctx) {
        BppToken tok;
        runtime_memset(&tok, 0, sizeof(tok));
        tok.type = TOK_EOF;
        return tok;
    }
    const char *old_pos = ctx->pos;
    BppToken tok = lex_next(ctx);
    ctx->pos = old_pos;
    return tok;
}

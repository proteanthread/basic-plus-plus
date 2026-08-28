// FILENAME: scan_string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c, lexer_internal.h)
// Implements lexical scanning and token stream processing for scan_string.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

//
// ---- Scanner Character & Lookahead Helpers ----
//

// checks whether character is a valid identifier or keyword boundary delimiter
bool scan_is_kw_bound(char c) {
    return c == '\0' || runtime_isspace((unsigned char)c) ||
           c == '"' || c == '\'' || c == '(' || c == '$' || c == '%' ||
           c == '!' || c == '#' || c == '&' || c == '-' || c == '.' ||
           c == ':' || c == ';' || c == ',';
}

// matches case-insensitive directive against target string
bool scan_match_directive(const char *start, int len, const char *target) {
    if (len != (int)runtime_strlen(target)) return false;
    for (int i = 0; i < len; i++) {
        if (runtime_toupper((unsigned char)start[i]) != target[i]) return false;
    }
    return true;
}

// checks if current token is followed by an assignment operator
bool scan_is_followed_by_assignment(const char *pos) {
    if (!pos) return false;
    const char *p = pos;
    if (*p == '$' || *p == '%' || *p == '!' || *p == '#' || *p == '&') p++;
    while (*p && runtime_isspace((unsigned char)*p)) p++;
    if (*p == '(') {
        int paren_depth = 1;
        p++;
        while (*p && paren_depth > 0) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p + 1)) p++;
                    p++;
                }
                if (*p == '"') p++;
            } else if (*p == '(') {
                paren_depth++;
                p++;
            } else if (*p == ')') {
                paren_depth--;
                p++;
            } else {
                p++;
            }
        }
        while (*p && runtime_isspace((unsigned char)*p)) p++;
    }
    return (*p == '=' && *(p + 1) != '=');
}

// advances lexer pointer past horizontal whitespace characters
void scan_skip_whitespace(LexerContext *ctx) {
    if (!ctx) return;
    while (*ctx->pos && *ctx->pos != '\n' && runtime_isspace((unsigned char)*ctx->pos)) {
        ctx->pos++;
    }
}

//
// ---- String, Docstring & Namespace Scanners ----
//

// parses double colon constructs including namespaces, directives, and global labels
bool scan_try_directive_or_label(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != ':' || *(ctx->pos + 1) != ':') return false;

    ctx->pos += 2;

    // Namespace declaration ::[name]
    if (*ctx->pos == '[') {
        ctx->pos++;
        tok->start = ctx->pos;
        while (*ctx->pos && *ctx->pos != ']' && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok->type = TOK_NAMESPACE_DECL;
        tok->length = ctx->pos - tok->start;
        tok->as.string = tok->start;
        if (*ctx->pos == ']') {
            ctx->pos++;
        }
        return true;
    }

    // Identifier or directive after ::
    const char *ident_start = ctx->pos;
    while (runtime_isalnum((unsigned char)*ctx->pos) || *ctx->pos == '_') {
        ctx->pos++;
    }
    int len = (int)(ctx->pos - ident_start);


    if (len > 0) {
        bool is_directive = false;
        if (scan_match_directive(ident_start, len, "OPTION")) is_directive = true;
        else if (scan_match_directive(ident_start, len, "INCLUDE")) is_directive = true;
        else if (scan_match_directive(ident_start, len, "IMPORT")) is_directive = true;
        else if (scan_match_directive(ident_start, len, "KEYWORD")) is_directive = true;
        else if (scan_match_directive(ident_start, len, "SCOPE")) is_directive = true;
        else if (scan_match_directive(ident_start, len, "ALIAS")) is_directive = true;

        if (is_directive) {
            tok->type = TOK_DIRECTIVE;
            tok->start = ident_start;
            tok->length = len;
            tok->as.string = ident_start;
            return true;
        } else {
            tok->type = TOK_GLOBAL_LABEL;
            tok->start = ident_start;
            tok->length = len;
            tok->as.string = ident_start;
            if (*ctx->pos == ':') {
                ctx->pos++;
            }
            return true;
        }
    } else {
        tok->type = TOK_DOUBLE_COLON;
        tok->length = 2;
        tok->as.string = tok->start;
        return true;
    }
}

// parses C++-style // docstrings and inline documentation comments
bool scan_try_docstring(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '/' || *(ctx->pos + 1) != '/') return false;

    ctx->pos += 2;
    while (*ctx->pos == ' ' || *ctx->pos == '\t') {
        ctx->pos++;
    }
    tok->start = ctx->pos;
    if (*ctx->pos == '"') {
        ctx->pos++;
        tok->start = ctx->pos;
        while (*ctx->pos && *ctx->pos != '"' && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok->type = TOK_DOCSTRING;
        tok->length = ctx->pos - tok->start;
        tok->as.string = tok->start;
        if (*ctx->pos == '"') {
            ctx->pos++;
        }
    } else {
        while (*ctx->pos && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok->type = TOK_DOCSTRING;
        tok->length = ctx->pos - tok->start;
        tok->as.string = tok->start;
    }
    return true;
}

// parses double-quoted string literals
bool scan_try_string_literal(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '"') return false;

    ctx->pos++;
    tok->start = ctx->pos;
    while (*ctx->pos && *ctx->pos != '"' && *ctx->pos != '\n') {
        ctx->pos++;
    }
    tok->type = TOK_STRING;
    tok->length = ctx->pos - tok->start;
    tok->as.string = tok->start;
    if (*ctx->pos == '"') {
        ctx->pos++;
    }
    return true;
}

// parses inline RPN expressions enclosed in curly braces
bool scan_try_rpn_literal(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '{') return false;

    const char *orig = ctx->pos;
    ctx->pos++;
    tok->start = ctx->pos;
    while (*ctx->pos && *ctx->pos != '}' && *ctx->pos != '\n') {
        ctx->pos++;
    }
    if (*ctx->pos == '}') {
        tok->type = TOK_RPN_LITERAL;
        tok->length = ctx->pos - tok->start;
        tok->as.string = tok->start;
        ctx->pos++;
        return true;
    }
    ctx->pos = orig;
    return false;
}

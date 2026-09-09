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
    while (*p && (runtime_isalnum((unsigned char)*p) || *p == '_' || *p == '.')) p++;
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

// checks if target keyword appears later on the same statement line (up to EOL, :, or comment)
bool scan_has_keyword_on_line(const char *pos, const char *target_kw) {
    if (!pos || !target_kw) return false;
    size_t kw_len = runtime_strlen(target_kw);
    const char *p = pos;
    bool in_quotes = false;
    while (*p && *p != '\n' && *p != '\r') {
        if (*p == '"') {
            in_quotes = !in_quotes;
            p++;
            continue;
        }
        if (!in_quotes) {
            if (*p == ':' || *p == '\'') break;
            if (runtime_strncasecmp(p, "REM", 3) == 0 && scan_is_kw_bound(p[3])) break;
            if (runtime_strncasecmp(p, target_kw, kw_len) == 0) {
                char prev = (p > pos) ? *(p - 1) : ' ';
                char next = *(p + kw_len);
                if (scan_is_kw_bound(prev) && (scan_is_kw_bound(next) || runtime_isdigit((unsigned char)next) || next == '-' || next == '.')) {
                    return true;
                }
            }
        }
        p++;
    }
    return false;
}

// checks if the token starting at pos is preceded by the LET keyword
bool scan_is_preceded_by_let(const char *pos, const char *source_start) {
    if (!pos || !source_start || pos <= source_start) return false;
    const char *p = pos - 1;
    while (p >= source_start && runtime_isspace((unsigned char)*p)) p--;
    if (p >= source_start + 2 && runtime_strncasecmp(p - 2, "LET", 3) == 0) {
        if (p - 2 == source_start || scan_is_kw_bound(*(p - 3))) {
            return true;
        }
    }
    return false;
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

// parses dedicated beginning-of-line !! pragmas
bool scan_try_pragma(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '!' || *(ctx->pos + 1) != '!') return false;

    // Verify beginning-of-line: allow optional leading whitespace and line numbers
    const char *p = ctx->pos - 1;
    while (p >= ctx->source && (*p == ' ' || *p == '\t')) {
        p--;
    }
    while (p >= ctx->source && runtime_isdigit((unsigned char)*p)) {
        p--;
    }
    while (p >= ctx->source && (*p == ' ' || *p == '\t')) {
        p--;
    }
    bool is_bol = (p < ctx->source || *p == '\n' || *p == '\r');
    if (!is_bol) {
        // Mid-line !! is strictly prohibited
        tok->type = TOK_UNKNOWN;
        tok->start = ctx->pos;
        tok->length = 2;
        ctx->pos += 2;
        return true;
    }

    ctx->pos += 2;
    while (*ctx->pos == ' ' || *ctx->pos == '\t') {
        ctx->pos++;
    }

    const char *ident_start = ctx->pos;
    while (runtime_isalnum((unsigned char)*ctx->pos) || *ctx->pos == '_') {
        ctx->pos++;
    }
    int len = (int)(ctx->pos - ident_start);
    if (len <= 0) {
        tok->type = TOK_UNKNOWN;
        tok->start = ident_start - 2;
        tok->length = 2;
        return true;
    }

    tok->type = TOK_PRAGMA;
    tok->start = ident_start;
    tok->length = len;
    tok->as.string = ident_start;
    return true;
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

// parses inline RPN expressions, set literals, and groups enclosed in curly braces
bool scan_try_rpn_literal(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '{') return false;

    const char *orig = ctx->pos;
    ctx->pos++;
    tok->start = ctx->pos;
    int brace_depth = 1;
    bool in_str = false;

    while (*ctx->pos && *ctx->pos != '\n') {
        char c = *ctx->pos;
        if (c == '"') {
            in_str = !in_str;
        } else if (!in_str) {
            if (c == '{') {
                brace_depth++;
            } else if (c == '}') {
                brace_depth--;
                if (brace_depth == 0) {
                    tok->type = TOK_RPN_LITERAL;
                    tok->length = ctx->pos - tok->start;
                    tok->as.string = tok->start;
                    ctx->pos++;
                    return true;
                }
            }
        }
        ctx->pos++;
    }
    ctx->pos = orig;
    return false;
}

// parses inline Prefix Polish Notation expressions enclosed in square brackets
bool scan_try_pn_literal(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;
    if (*ctx->pos != '[') return false;

    const char *orig = ctx->pos;
    const char *p = ctx->pos + 1;
    while (*p == ' ' || *p == '\t') p++;

    bool is_op = false;
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '\\' || *p == '^' ||
        *p == '<' || *p == '>' || *p == '=' || *p == '%') {
        is_op = true;
    } else if (runtime_isalpha((unsigned char)*p)) {
        const char *w = p;
        while (runtime_isalnum((unsigned char)*w) || *w == '_') w++;
        size_t wlen = (size_t)(w - p);
        if ((wlen == 3 && (runtime_strncasecmp(p, "NEG", 3) == 0 ||
                           runtime_strncasecmp(p, "NOT", 3) == 0 ||
                           runtime_strncasecmp(p, "ABS", 3) == 0 ||
                           runtime_strncasecmp(p, "SQR", 3) == 0 ||
                           runtime_strncasecmp(p, "SGN", 3) == 0 ||
                           runtime_strncasecmp(p, "INT", 3) == 0 ||
                           runtime_strncasecmp(p, "FIX", 3) == 0 ||
                           runtime_strncasecmp(p, "MIN", 3) == 0 ||
                           runtime_strncasecmp(p, "MAX", 3) == 0 ||
                           runtime_strncasecmp(p, "SIN", 3) == 0 ||
                           runtime_strncasecmp(p, "COS", 3) == 0 ||
                           runtime_strncasecmp(p, "TAN", 3) == 0 ||
                           runtime_strncasecmp(p, "LOG", 3) == 0 ||
                           runtime_strncasecmp(p, "EXP", 3) == 0 ||
                           runtime_strncasecmp(p, "RND", 3) == 0 ||
                           runtime_strncasecmp(p, "MOD", 3) == 0 ||
                           runtime_strncasecmp(p, "AND", 3) == 0 ||
                           runtime_strncasecmp(p, "XOR", 3) == 0)) ||
            (wlen == 2 && runtime_strncasecmp(p, "OR", 2) == 0) ||
            (wlen == 5 && (runtime_strncasecmp(p, "HYPOT", 5) == 0 ||
                           runtime_strncasecmp(p, "ATAN2", 5) == 0))) {
            is_op = true;
        }
    }

    if (!is_op) return false;

    int depth = 1;
    ctx->pos++;
    tok->start = ctx->pos;
    while (*ctx->pos && *ctx->pos != '\n') {
        if (*ctx->pos == '[') depth++;
        else if (*ctx->pos == ']') {
            depth--;
            if (depth == 0) break;
        }
        ctx->pos++;
    }
    if (*ctx->pos == ']' && depth == 0) {
        tok->type = TOK_PN_LITERAL;
        tok->length = ctx->pos - tok->start;
        tok->as.string = tok->start;
        ctx->pos++;
        return true;
    }
    ctx->pos = orig;
    return false;
}


// FILENAME: color_map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cls.c, stmt_home.c)
// NEEDS: libcore (strings.h), libengine (color_map.h, eval.h, lexer.h, vm.h)
// Implements color parsing and IBM PC 16-color palette translation.
//
// ---- Includes ----

#include "device/color_map.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/num_parse.h"

typedef struct {
    const char *name;
    int index;
} ColorNameEntry;

static const ColorNameEntry g_ibm_colors[] = {
    {"BLACK",   0},
    {"BLUE",    1},
    {"NAVY",    1},
    {"GREEN",   2},
    {"CYAN",    3},
    {"TEAL",    3},
    {"RED",     4},
    {"MAROON",  4},
    {"MAGENTA", 5},
    {"PURPLE",  5},
    {"BROWN",   6},
    {"ORANGE",  6},
    {"OLIVE",   6},
    {"WHITE",   7},
    {"SILVER",  7},
    {"GRAY",    8},
    {"GREY",    8},
    {"LIME",    10},
    {"AQUA",    11},
    {"PINK",    12},
    {"FUCHSIA", 13},
    {"VIOLET",  13},
    {"YELLOW",  14},
    {"GOLD",    14},
    {NULL, -1}
};

int color_name_to_index(const char *name) {
    if (!name) return -1;
    // Skip leading whitespace or quote if present
    while (*name && runtime_isspace((unsigned char)*name)) name++;
    if (*name == '"' || *name == '\'') name++;

    char clean[32];
    size_t len = 0;
    while (name[len] && name[len] != '"' && name[len] != '\'' && !runtime_isspace((unsigned char)name[len]) && len < sizeof(clean) - 1) {
        clean[len] = (char)runtime_toupper((unsigned char)name[len]);
        len++;
    }
    clean[len] = '\0';
    if (len == 0) return -1;

    for (size_t i = 0; g_ibm_colors[i].name != NULL; i++) {
        if (runtime_strcmp(clean, g_ibm_colors[i].name) == 0) {
            return g_ibm_colors[i].index;
        }
    }
    return -1;
}

static int parse_hex_color(const char *hex_str) {
    if (!hex_str) return -1;
    if (*hex_str == '#') hex_str++;
    size_t len = runtime_strlen(hex_str);
    if (len != 6 && len != 3) return -1;

    uint32_t r = 0, g = 0, b = 0;
    if (len == 6) {
        for (int i = 0; i < 6; i++) {
            char c = hex_str[i];
            if (!runtime_isxdigit((unsigned char)c)) return -1;
        }
        r = (uint32_t)runtime_strtoul((char[]){hex_str[0], hex_str[1], '\0'}, NULL, 16);
        g = (uint32_t)runtime_strtoul((char[]){hex_str[2], hex_str[3], '\0'}, NULL, 16);
        b = (uint32_t)runtime_strtoul((char[]){hex_str[4], hex_str[5], '\0'}, NULL, 16);
    } else {
        for (int i = 0; i < 3; i++) {
            char c = hex_str[i];
            if (!runtime_isxdigit((unsigned char)c)) return -1;
        }
        r = (uint32_t)runtime_strtoul((char[]){hex_str[0], hex_str[0], '\0'}, NULL, 16);
        g = (uint32_t)runtime_strtoul((char[]){hex_str[1], hex_str[1], '\0'}, NULL, 16);
        b = (uint32_t)runtime_strtoul((char[]){hex_str[2], hex_str[2], '\0'}, NULL, 16);
    }
    return (int)((r << 16) | (g << 8) | b);
}

int color_parse_token_or_expr(VMContext *vm, LexerContext *lex, BppError *err) {
    if (!vm || !lex || !err) return -1;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_COMMA) {
        return -1;
    }

    // Check for unquoted color keyword or identifier
    if (tok.type == TOK_KEYWORD) {
        const char *kname = lex_keyword_name(tok.as.keyword);
        if (kname) {
            int idx = color_name_to_index(kname);
            if (idx >= 0) {
                lex_next(lex);
                return idx;
            }
        }
    } else if (tok.type == TOK_IDENT) {
        char name_buf[32];
        size_t nlen = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
        if (tok.start && nlen > 0) {
            runtime_memcpy(name_buf, tok.start, nlen);
            name_buf[nlen] = '\0';
            int idx = color_name_to_index(name_buf);
            if (idx >= 0) {
                lex_next(lex);
                return idx;
            }
        }
    }

    // Evaluate expression (handles numbers, numeric variables, string variables/literals)
    BValue val = eval_expression(vm, lex, err);
    if (err->code != 0) return -1;

    if (val.type == VAL_STRING) {
        int color_res = -1;
        if (val.as.string) {
            const char *sdata = str_data(val.as.string);
            if (sdata && *sdata == '#') {
                color_res = parse_hex_color(sdata);
            } else {
                color_res = color_name_to_index(sdata);
            }
            str_release(vm_get_str(vm), val.as.string);
        }
        if (color_res < 0) {
            err->code = 2;
            err->message = "Syntax error: Unrecognized color name or hex code";
            return -1;
        }
        return color_res;
    }

    if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
        double num = val.as.number;
        int color_idx = (int)(num >= 0.0 ? num + 0.5 : num - 0.5);
        if (color_idx < 0) {
            err->code = 5;
            err->message = "Illegal Function Call: Color index cannot be negative";
            return -1;
        }
        return color_idx;
    }

    err->code = 5;
    err->message = "Illegal Function Call: Invalid color value";
    return -1;
}

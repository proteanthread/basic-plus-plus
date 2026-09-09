// FILENAME: redim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (dim.h, dim.c, redim.h, string.c)
// Provides runtime implementation for the REDIM statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/redim.h"
#include "statements/variables/declaration/dim.h"
#include "runtime/arrays.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_redim_desc = {
    .name = "REDIM",
    .category = "Variables & Memory",
    .syntax = "REDIM [PRESERVE] array_name(subscripts...)",
    .description = "Changes the dimensions and size of dynamic arrays, optionally preserving existing data.",
    .error_summary = "Error 2: Syntax Error, Error 9: Subscript out of range",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

#ifdef _WIN32
#define runtime_strncasecmp runtime_strncasecmp
#endif

void stmt_redim_register(void) {
    lang_desc_register(&g_redim_desc);
}

BppError stmt_redim_handler(VMContext *vm, LexerContext *lex) {
    LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
    if (temp_lex) {
        BppToken tok = lex_peek(temp_lex);
        if ((tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SHARED || tok.as.keyword == KW_PRESERVE)) ||
            (tok.type == TOK_IDENT && tok.length == 8 && runtime_strncasecmp(tok.start, "PRESERVE", 8) == 0)) {
            lex_next(temp_lex);
            tok = lex_peek(temp_lex);
        }
        if (tok.type == TOK_IDENT) {
            char arr_name[64];
            size_t len = (tok.length < sizeof(arr_name) - 1) ? tok.length : (sizeof(arr_name) - 1);
            runtime_memcpy(arr_name, tok.start, len);
            arr_name[len] = '\0';
            arr_erase(vm_get_arr(vm), arr_name);
        }
        lex_shutdown(temp_lex);
    }
    return stmt_dim_handler(vm, lex);
}

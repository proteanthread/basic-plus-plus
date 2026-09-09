// FILENAME: stmt_extern.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libengine (eval.h, stmt.h), libplatform (platform.h)
// Provides runtime implementation for the dynamic FFI EXTERN statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/stmt_extern.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "lexer/lexer.h"
#include "types/errors.h"

#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_extern_desc = {
    .name = "EXTERN",
    .category = "Control Flow",
    .syntax = "EXTERN {SUB | FUNCTION} name [ALIAS \"aliasname\"] [(params)]",
    .description = "Dynamically binds an external dynamic shared library (DLL/.so) C symbol for direct calling in BASIC++.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

#if defined(_WIN32)
#define strcasecmp_compat runtime_strcasecmp
#else
#define strcasecmp_compat runtime_strcasecmp
#endif

#define MAX_EXTERN_SYMBOLS 64

typedef struct {
    char  alias[64];
    char  lib_name[128];
    char  func_name[128];
    void *lib_handle;
    void *func_ptr;
    bool  active;
} ExternSymbolEntry;

static ExternSymbolEntry s_extern_table[MAX_EXTERN_SYMBOLS];
static int               s_extern_count = 0;

void *interop_get_extern_func(const char *name) {
    if (!name || !*name) return NULL;
    for (int i = 0; i < s_extern_count; i++) {
        if (s_extern_table[i].active && strcasecmp_compat(s_extern_table[i].alias, name) == 0) {
            return s_extern_table[i].func_ptr;
        }
    }
    return NULL;
}

BppError stmt_extern_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    // 1. Evaluate library name string
    BValue lib_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (lib_val.type != VAL_STRING || !lib_val.as.string) {
        if (lib_val.type == VAL_STRING) str_release(vm_get_str(vm), lib_val.as.string);
        err.code = 13; err.message = "Type Mismatch: Expected library path string";
        return err;
    }

    const char *lib_str = str_data(lib_val.as.string);
    char lib_path[128] = {0};
    if (lib_str) runtime_strncpy(lib_path, lib_str, sizeof(lib_path) - 1);
    str_release(vm_get_str(vm), lib_val.as.string);

    // 2. Consume comma
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected comma after library path";
        return err;
    }

    // 3. Evaluate function symbol name string or identifier
    BppToken sym_tok = lex_peek(lex);
    char sym_name[128] = {0};
    if (sym_tok.type == TOK_STRING) {
        BValue sym_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        const char *sdata = str_data(sym_val.as.string);
        if (sdata) runtime_strncpy(sym_name, sdata, sizeof(sym_name) - 1);
        str_release(vm_get_str(vm), sym_val.as.string);
    } else if (sym_tok.type == TOK_IDENT) {
        lex_next(lex);
        size_t len = sym_tok.length < sizeof(sym_name) - 1 ? sym_tok.length : sizeof(sym_name) - 1;
        runtime_memcpy(sym_name, sym_tok.start, len);
        sym_name[len] = '\0';
    } else {
        err.code = 2; err.message = "Expected exported symbol name in EXTERN statement";
        return err;
    }

    // 4. Optional return type & parameter list (skip/parse)
    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
        if (runtime_strncasecmp(next_tok.start, "ALIAS", 5) != 0) {
            lex_next(lex); // Skip return type
        }
    }
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex);
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_RPAREN) {
            lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RPAREN) {
            lex_next(lex);
        }
    }

    // 5. Optional ALIAS "alias_name"
    char alias_name[64] = {0};
    runtime_strncpy(alias_name, sym_name, sizeof(alias_name) - 1);

    next_tok = lex_peek(lex);
    if ((next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_ALIAS) ||
        (next_tok.type == TOK_IDENT && next_tok.length == 5 && runtime_strncasecmp(next_tok.start, "ALIAS", 5) == 0)) {
        lex_next(lex);
        BppToken al_tok = lex_peek(lex);
        if (al_tok.type == TOK_STRING) {
            BValue al_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            const char *adata = str_data(al_val.as.string);
            if (adata) runtime_strncpy(alias_name, adata, sizeof(alias_name) - 1);
            str_release(vm_get_str(vm), al_val.as.string);
        } else if (al_tok.type == TOK_IDENT) {
            lex_next(lex);
            size_t len = al_tok.length < sizeof(alias_name) - 1 ? al_tok.length : sizeof(alias_name) - 1;
            runtime_memcpy(alias_name, al_tok.start, len);
            alias_name[len] = '\0';
        }
    }

    // 6. Dynamically load library and resolve procedure address
    void *lib_handle = platform_load_library(lib_path);
    if (!lib_handle) {
        // Try system fallback paths
        #if defined(_WIN32)
        char sys_path[256];
        runtime_snprintf(sys_path, sizeof(sys_path), "%s.dll", lib_path);
        lib_handle = platform_load_library(sys_path);
        #endif
    }

    void *func_ptr = lib_handle ? platform_get_proc_address(lib_handle, sym_name) : NULL;

    // 7. Store in extern table
    if (s_extern_count < MAX_EXTERN_SYMBOLS) {
        runtime_strncpy(s_extern_table[s_extern_count].alias, alias_name, sizeof(s_extern_table[0].alias) - 1);
        runtime_strncpy(s_extern_table[s_extern_count].lib_name, lib_path, sizeof(s_extern_table[0].lib_name) - 1);
        runtime_strncpy(s_extern_table[s_extern_count].func_name, sym_name, sizeof(s_extern_table[0].func_name) - 1);
        s_extern_table[s_extern_count].lib_handle = lib_handle;
        s_extern_table[s_extern_count].func_ptr = func_ptr;
        s_extern_table[s_extern_count].active = true;
        s_extern_count++;
    }

    return err;
}

void stmt_extern_register(void) {
    lang_desc_register(&g_extern_desc);
}

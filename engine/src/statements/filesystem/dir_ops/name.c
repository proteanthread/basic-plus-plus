// FILENAME: name.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, name.h, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the NAME statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/name.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

void stmt_name_register(void) {
    MicroLibMetadata meta = {
        .name = "NAME",
        .category = "Filesystem I/O",
        .syntax = "NAME oldspec AS newspec",
        .help_text = "Renames an existing disk file or directory.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 58: File Already Exists"
    };
    microlib_register(&meta);
}

BppError stmt_name_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue old_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (old_val.type != VAL_STRING || !old_val.as.string) {
        if (old_val.type == VAL_STRING && old_val.as.string) str_release(vm_get_str(vm), old_val.as.string);
        err.code = 13; err.message = "Type mismatch (expected old filename string in NAME)";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_AS) ||
        (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "AS", 2) == 0)) {
        lex_next(lex);
    } else {
        str_release(vm_get_str(vm), old_val.as.string);
        err.code = 2; err.message = "Expected AS in NAME statement";
        return err;
    }

    BValue new_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), old_val.as.string);
        return err;
    }

    if (new_val.type != VAL_STRING || !new_val.as.string) {
        str_release(vm_get_str(vm), old_val.as.string);
        if (new_val.type == VAL_STRING && new_val.as.string) str_release(vm_get_str(vm), new_val.as.string);
        err.code = 13; err.message = "Type mismatch (expected new filename string in NAME)";
        return err;
    }

    const char *old_path = str_data(old_val.as.string);
    const char *new_path = str_data(new_val.as.string);

    if (old_path && new_path) {
        if (rename(old_path, new_path) != 0) {
            err.code = 53; err.message = "File not found or rename failed";
        }
    }

    str_release(vm_get_str(vm), old_val.as.string);
    str_release(vm_get_str(vm), new_val.as.string);
    return err;
}

BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (path_val.type != VAL_STRING || !path_val.as.string) {
        if (path_val.type == VAL_STRING && path_val.as.string) str_release(vm_get_str(vm), path_val.as.string);
        err.code = 13; err.message = "Type mismatch (expected filename string in SETATTR)";
        return err;
    }

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue attr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), path_val.as.string);
        return err;
    }

    (void)attr_val;
    str_release(vm_get_str(vm), path_val.as.string);
    return err;
}

BppError stmt_environ_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_STRING || !val.as.string) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = 13; err.message = "Type mismatch (expected environment string in ENVIRON)";
        return err;
    }

    const char *s = str_data(val.as.string);
    if (s) {
        const char *eq = strchr(s, '=');
        if (eq) {
            char var_name[128];
            size_t nlen = (size_t)(eq - s);
            if (nlen >= sizeof(var_name)) nlen = sizeof(var_name) - 1;
            memcpy(var_name, s, nlen);
            var_name[nlen] = '\0';
            platform_setenv(var_name, eq + 1);
        }
    }

    str_release(vm_get_str(vm), val.as.string);
    return err;
}

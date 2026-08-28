// FILENAME: field.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, field.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FIELD statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/field.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_field_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FIELD",
        .category = "Filesystem I/O",
        .syntax = "FIELD [#]file_num, width AS string_var [, width AS string_var...]",
        .help_text = "Allocates space in a random file buffer for record variables.",
        .error_codes = "Error 2: Syntax Error, Error 50: Field Overflow, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_field_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int channel = (int)ch_val.as.number;
    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    VariableContext *vc = vm_get_var(vm);
    int offset = 0;
    int rec_len = file_get_record_len(fc, channel);

    while (true) {
        BValue width_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int width = (int)width_val.as.number;

        tok = lex_next(lex);
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            char kw[32];
            if (tok.length >= sizeof(kw)) tok.length = sizeof(kw) - 1;
            memcpy(kw, tok.start, tok.length);
            kw[tok.length] = '\0';
            if (platform_strcasecmp(kw, "AS") == 0) {
                tok = lex_next(lex);
            }
        }

        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Syntax Error in FIELD (expected string variable)";
            return err;
        }

        char var_name[64];
        if (tok.length >= sizeof(var_name)) tok.length = sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tok.length);
        var_name[tok.length] = '\0';

        if (offset + width > rec_len) {
            err.code = 50;
            err.message = "Field Overflow";
            return err;
        }

        BValue val;
        memset(&val, 0, sizeof(val));
        val.type = VAL_FIELD_STRING;
        val.as.field_str.channel = channel;
        val.as.field_str.offset = offset;
        val.as.field_str.length = width;
        var_assign(vc, var_name, val);

        offset += width;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

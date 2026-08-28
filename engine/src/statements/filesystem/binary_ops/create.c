// FILENAME: create.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (create.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the CREATE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/create.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/file.h"
#include "types/errors.h"
#include <stdio.h>
#include <string.h>

void stmt_create_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CREATE",
        .category = "Filesystem",
        .syntax = "CREATE filename$ [, type] | CREATE #ch, filename$ [: mode]",
        .help_text = "Creates a new file on disk (Apple /// Business BASIC & BASIC09).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

BppError stmt_create_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int ch = (int)ch_val.as.number;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        }

        BValue path_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (path_val.type != VAL_STRING) {
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        const char *path = str_data(path_val.as.string);
        err = file_open(vm_get_file(vm), vm_get_vdev(vm), ch, path, FILE_MODE_OUTPUT, FILE_ACCESS_READ_WRITE, FILE_LOCK_SHARED, 0);
        str_release(vm_get_str(vm), path_val.as.string);
        if (err.code != 0) return err;

        // Optional mode / type
        tok = lex_peek(lex);
        if ((tok.start && tok.start[0] == ':') || tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue mode_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (mode_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), mode_val.as.string);
            }
        }
        return err;
    }

    // Apple /// SOS file creation: CREATE filename$ [, type]
    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (path_val.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *path = str_data(path_val.as.string);
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        str_release(vm_get_str(vm), path_val.as.string);
        err.code = ERR_PERMISSION_DENIED;
        return err;
    }
    fclose(fp);

    str_release(vm_get_str(vm), path_val.as.string);

    // Optional file type code
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        BValue type_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (type_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), type_val.as.string);
        }
    }

    return err;
}

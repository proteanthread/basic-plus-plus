// FILENAME: restore.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, metadata.h, metadata.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, restore.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the RESTORE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/restore.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/metadata.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_restore_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RESTORE",
        .category = "Variables & Memory",
        .syntax = "RESTORE [line_number | label] | RESTORE #file_num",
        .help_text = "Resets the DATA statement reading pointer or file position to the beginning or specified line.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_restore_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = 13;
            return err;
        }
        int ch = (int)ch_val.as.number;
        file_seek(vm_get_file(vm), ch, 0); // Seek to beginning
        return err;
    }

    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        vm_set_data_ptr(vm, 0);
        return err;
    }

    if (tok.type == TOK_GLOBAL_LABEL || tok.type == TOK_IDENT) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, (tok.type == TOK_GLOBAL_LABEL) ? tok.as.string : tok.start, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            lex_next(lex);
            int count = vm_get_data_count(vm);
            BppDataPosition *items = vm_get_data_items(vm);
            int target_ptr = 0;
            if (items) {
                for (int i = 0; i < count; ++i) {
                    if (items[i].line >= target_line) {
                        target_ptr = i;
                        break;
                    }
                }
            }
            vm_set_data_ptr(vm, target_ptr);
            return err;
        }
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
        double line_num = val.as.number;
        int count = vm_get_data_count(vm);
        BppDataPosition *items = vm_get_data_items(vm);
        int target_ptr = 0;
        if (items) {
            for (int i = 0; i < count; ++i) {
                if (items[i].line >= line_num) {
                    target_ptr = i;
                    break;
                }
            }
        }
        vm_set_data_ptr(vm, target_ptr);
    } else {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        vm_set_data_ptr(vm, 0);
    }

    return err;
}

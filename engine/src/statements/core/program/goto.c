// FILENAME: goto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, metadata.h, metadata.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, goto.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GOTO statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/goto.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "memory/memory.h"
#include "runtime/metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_goto_register(void) {
    MicroLibMetadata meta = {
        .name = "GOTO",
        .category = "Control Flow",
        .syntax = "GOTO line_num | expr | label",
        .help_text = "Unconditionally transfers execution to the specified program line number, expression, or label.",
        .error_codes = "Error 8: Undefined line number (target line does not exist), Error 2: Syntax error (missing line number)"
    };
    microlib_register(&meta);
}

BppError stmt_goto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_GLOBAL_LABEL) {
        lex_next(lex);
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (!metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            err.code = 8;
            err.message = "Undefined global label";
            return err;
        }

        const char *cur_file = vm_get_current_filename(vm);
        if (filename[0] != '\0' && cur_file[0] != '\0' && strcasecmp(filename, cur_file) != 0) {
            BppError load_err = vm_load_program_file(vm, filename);
            if (load_err.code != 0) return load_err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    // Check if target is a local alphanumeric label identifier e.g. GOTO SUB_LOOP
    if (tok.type == TOK_IDENT) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.start, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            lex_next(lex);
            vm_jump(vm, target_line, NULL);
            return err;
        }
    }

    // Evaluate target as numeric line number or computed expression (e.g. GOTO 100 + X * 10 or GOTO L)
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_NUMBER) {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch in GOTO destination";
        return err;
    }

    BppLineNumber target = (BppLineNumber)val.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; // Undefined line number
        err.message = "Undefined line number in GOTO";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}

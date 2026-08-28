// FILENAME: gosub.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, metadata.h, metadata.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, gosub.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GOSUB statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/gosub.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "memory/memory.h"
#include "runtime/metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_gosub_register(void) {
    MicroLibMetadata meta = {
        .name = "GOSUB",
        .category = "Control Flow",
        .syntax = "GOSUB line_num | expr | label",
        .help_text = "Pushes current line onto stack and branches execution to specified subroutine line number, expression, or label.",
        .error_codes = "Error 8: Undefined line number (subroutine target missing), Error 2: Syntax error (missing line number)"
    };
    microlib_register(&meta);
}

BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex) {
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
            err.code = 38;
            err.message = "Cross-file GOSUB not supported";
            return err;
        }

        if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
            err.code = 12;
            err.message = "Subroutine nesting limit exceeded";
            return err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    // Check if target is a local alphanumeric label identifier e.g. GOSUB CALC_ROUTINE
    if (tok.type == TOK_IDENT) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.start, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            lex_next(lex);
            if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
                err.code = 14;
                err.message = "GOSUB stack overflow";
                return err;
            }
            vm_jump(vm, target_line, NULL);
            return err;
        }
    }

    // Evaluate target as numeric line number or computed expression (e.g. GOSUB 100 + X * 10 or GOSUB L)
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_NUMBER) {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch in GOSUB destination";
        return err;
    }

    BppLineNumber target = (BppLineNumber)val.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; // Undefined line number
        err.message = "Undefined line number in GOSUB";
        return err;
    }

    BppLineNumber current = vm_get_current_line(vm);
    const char *resume_pos = lex_get_pos(lex);

    if (!vm_gosub_push(vm, current, resume_pos)) {
        err.code = 14; // Out of memory / Stack overflow
        err.message = "GOSUB stack overflow";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}

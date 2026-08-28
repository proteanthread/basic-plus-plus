// FILENAME: save.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, save.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SAVE statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/save.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <string.h>

BppError stmt_save_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) {
            str_release(vm_get_str(vm), fn_val.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    const char *filename = str_data(fn_val.as.string);
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_PERMISSION_DENIED;
        return err;
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    for (size_t i = 0; i < count; i++) {
        fprintf(fp, "%lld %s\n", (long long)lines[i].line_number, lines[i].text ? lines[i].text : "");
    }

    fclose(fp);
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}

void stmt_save_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SAVE",
        .category = "Program Mgmt & Editing",
        .syntax = "SAVE filename_expr [, A | P]",
        .help_text = "Saves the program currently in memory to a file on disk.",
        .error_codes = "Error 2: Syntax Error, Error 64: Bad File Name, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}


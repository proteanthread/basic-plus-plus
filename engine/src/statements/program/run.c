// FILENAME: run.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, run.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the RUN statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/run.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/arrays.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_run_desc = {
    .name = "RUN",
    .category = "Program Mgmt & Editing",
    .syntax = "RUN [line_number | filename [, R]]",
    .description = "Starts execution of the program currently in memory or loads and runs a specified file.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

extern BppError vm_load_program_file(VMContext *vm, const char *filename);

BppError stmt_run_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        var_clear_all(vm_get_var(vm));
        arr_clear_all(vm_get_arr(vm));
        vm_reset_for_run(vm);
        vm_set_start_line(vm, tok.as.number);
        vm_run_program(vm);
        if (vm_has_error(vm)) {
            return vm_get_error(vm);
        }
        return err;
    } else if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_LPAREN) {
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
        char path_buf[512];
        runtime_strncpy(path_buf, filename, sizeof(path_buf) - 1);
        path_buf[sizeof(path_buf) - 1] = '\0';
        str_release(vm_get_str(vm), fn_val.as.string);

        bool keep_open = false;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            tok = lex_peek(lex);
            if (tok.type == TOK_IDENT && tok.length == 1 && (tok.start[0] == 'R' || tok.start[0] == 'r')) {
                lex_next(lex);
                keep_open = true;
            }
        }

        if (!keep_open) {
            var_clear_all(vm_get_var(vm));
            arr_clear_all(vm_get_arr(vm));
        } else {
            var_clear_for_chain(vm_get_var(vm));
            arr_clear_for_chain(vm_get_arr(vm), vm_get_var(vm));
        }

        mem_program_clear(vm_get_mem(vm));
        BppError load_err = vm_load_program_file(vm, path_buf);
        if (load_err.code != 0) {
            return load_err;
        }
        vm_reset_for_run(vm);
        vm_run_program(vm);
        if (vm_has_error(vm)) {
            return vm_get_error(vm);
        }
        return err;
    } else {
        var_clear_all(vm_get_var(vm));
        arr_clear_all(vm_get_arr(vm));
        vm_reset_for_run(vm);
        vm_set_start_line(vm, 0.0);
        vm_run_program(vm);
        if (vm_has_error(vm)) {
            return vm_get_error(vm);
        }
        return err;
    }
}

void stmt_run_register(void) {
    lang_desc_register(&g_run_desc);
}


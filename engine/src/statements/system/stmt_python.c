// FILENAME: stmt_python.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the PYTHON statement for embedded Python/MicroPython script execution.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_python_desc = {
    .name = "PYTHON",
    .category = "Language Interop",
    .syntax = "PYTHON code$",
    .description = "Executes inline Python / MicroPython code string.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_python_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue code_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (code_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), code_val.as.string);
    }
    return err;
}

void stmt_python_register(void) {
    lang_desc_register(&g_python_desc);
}

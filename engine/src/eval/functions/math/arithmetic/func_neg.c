// FILENAME: func_neg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident.c, ast_parse_expr.c
// NEEDS: libkernel, libcore
// Implementation for arithmetic negation function (NEG) in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/arithmetic/func_neg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"

static const LangDesc g_neg_desc = {
    .name = "NEG",
    .category = "Math",
    .syntax = "NEG(x) or NEG x",
    .description = "Returns the arithmetic negation (-x) of a numeric expression.",
    .error_summary = "Error 13: Type mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_neg_register(void) {
    lang_desc_register(&g_neg_desc);
}

BValue func_neg_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err) {
    (void)vm; (void)name;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    if (argc < 1 || !argv) {
        if (err) {
            err->code = 5;
            err->message = "Illegal function call: NEG requires 1 numeric argument";
        }
        return res;
    }

    if (argv[0].type == VAL_INTEGER) {
        res.type = VAL_INTEGER;
        res.as.number = -argv[0].as.number;
        return res;
    } else if (argv[0].type == VAL_NUMBER) {
        res.type = VAL_NUMBER;
        res.as.number = -argv[0].as.number;
        return res;
    }

    if (err) {
        err->code = 13;
        err->message = "Type mismatch: NEG requires a numeric argument";
    }
    return res;
}

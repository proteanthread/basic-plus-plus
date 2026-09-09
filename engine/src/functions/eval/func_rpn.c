// FILENAME: func_rpn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: eval/rpn.h, runtime/funcreg.h, runtime/language_descriptor.h
// Implements the freestanding RPN / STACK postfix calculator function in BASIC++.

#include "functions/eval/func_rpn.h"
#include "eval/rpn.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"

static const LangDesc g_func_rpn_desc = {
    .name = "RPN",
    .category = "Math & Calculations",
    .syntax = "RPN(expr$) | STACK(expr$) | RPN(op$, ...)",
    .description = "Evaluates reverse Polish notation expression strings and manipulates HP operational stack levels.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

BValue func_rpn_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (arg_count == 0) {
        rpn_reg_get("X", &res);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *s = str_data(args[0].as.string);

        if (arg_count == 1) {
            if (runtime_strcasecmp(s, "POP") == 0) {
                rpn_stack_pop(&res);
                return res;
            }
            if (runtime_strcasecmp(s, "CLEAR") == 0) {
                rpn_stack_clear();
                res.type = VAL_INTEGER;
                res.as.number = 0.0;
                return res;
            }
            if (runtime_strcasecmp(s, "SWAP") == 0) {
                rpn_stack_swap();
                rpn_reg_get("X", &res);
                return res;
            }
            if (runtime_strcasecmp(s, "ROLLUP") == 0) {
                rpn_stack_rollup();
                rpn_reg_get("X", &res);
                return res;
            }
            if (runtime_strcasecmp(s, "ROLLDN") == 0) {
                rpn_stack_rolldn();
                rpn_reg_get("X", &res);
                return res;
            }
            if (runtime_strcasecmp(s, "X") == 0 || runtime_strcasecmp(s, "Y") == 0 ||
                runtime_strcasecmp(s, "Z") == 0 || runtime_strcasecmp(s, "T") == 0 ||
                runtime_strcasecmp(s, "LASTX") == 0) {
                rpn_reg_get(s, &res);
                return res;
            }

            // Evaluate postfix expression string
            BppError err;
            runtime_memset(&err, 0, sizeof(err));
            res = rpn_eval_expr_string(vm, s, &err);
            return res;
        }

        if (runtime_strcasecmp(s, "PUSH") == 0 && arg_count >= 2) {
            rpn_stack_push(args[1]);
            rpn_reg_get("X", &res);
            return res;
        }

        if (runtime_strcasecmp(s, "EVAL") == 0 && arg_count >= 2 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            BppError err;
            runtime_memset(&err, 0, sizeof(err));
            res = rpn_eval_expr_string(vm, str_data(args[1].as.string), &err);
            return res;
        }

        if (runtime_strcasecmp(s, "REG") == 0 && arg_count >= 2 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            const char *rname = str_data(args[1].as.string);
            if (arg_count >= 3) {
                rpn_reg_set(rname, args[2]);
            }
            rpn_reg_get(rname, &res);
            return res;
        }

        if (runtime_strcasecmp(s, "SET") == 0 && arg_count >= 3 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            rpn_reg_set(str_data(args[1].as.string), args[2]);
            rpn_reg_get(str_data(args[1].as.string), &res);
            return res;
        }
    }

    // Default: push value to stack
    rpn_stack_push(args[0]);
    rpn_reg_get("X", &res);
    return res;
}

void func_rpn_register(void) {
    lang_desc_register(&g_func_rpn_desc);

    FunctionEntry entry_rpn = {
        .name = "RPN",
        .keyword = KW_NONE,
        .category = FCAT_MATH,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 3,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_rpn_eval,
        .help_text = "Reverse Polish notation evaluation function",
        .module_name = "Math"
    };
    funcreg_register(&entry_rpn);

    FunctionEntry entry_stack = {
        .name = "STACK",
        .keyword = KW_STACK,
        .category = FCAT_MATH,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 3,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_rpn_eval,
        .help_text = "HP operational stack manipulation function",
        .module_name = "Math"
    };
    funcreg_register(&entry_stack);
}

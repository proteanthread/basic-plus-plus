// FILENAME: func_pn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: eval/pn.h, runtime/funcreg.h, runtime/language_descriptor.h
// Implements the freestanding PN prefix calculator function in BASIC++.

#include "functions/eval/func_pn.h"
#include "eval/pn.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"

static const LangDesc g_func_pn_desc = {
    .name = "PN",
    .category = "Math & Calculations",
    .syntax = "PN(expr$) | PN(op$, ...)",
    .description = "Evaluates prefix Polish notation expression strings and controls prefix calculator registers.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

BValue func_pn_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (arg_count == 0) {
        pn_reg_get("X", &res);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *s = str_data(args[0].as.string);

        if (arg_count == 1) {
            // Check commands
            if (runtime_strcasecmp(s, "CLEAR") == 0) {
                pn_reset();
                res.type = VAL_INTEGER;
                res.as.number = 0.0;
                return res;
            }
            if (runtime_strcasecmp(s, "X") == 0 || runtime_strcasecmp(s, "ACC") == 0 ||
                runtime_strcasecmp(s, "Y") == 0 || runtime_strcasecmp(s, "Z") == 0 ||
                runtime_strcasecmp(s, "T") == 0 || runtime_strcasecmp(s, "LASTX") == 0) {
                pn_reg_get(s, &res);
                return res;
            }

            // Evaluate prefix expression string
            BppError err;
            runtime_memset(&err, 0, sizeof(err));
            res = pn_eval_expr_string(vm, s, &err);
            return res;
        }

        // Multi-argument operations
        if (runtime_strcasecmp(s, "EVAL") == 0 && arg_count >= 2 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            BppError err;
            runtime_memset(&err, 0, sizeof(err));
            res = pn_eval_expr_string(vm, str_data(args[1].as.string), &err);
            return res;
        }

        if (runtime_strcasecmp(s, "REG") == 0 && arg_count >= 2 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            const char *rname = str_data(args[1].as.string);
            if (arg_count >= 3) {
                pn_reg_set(rname, args[2]);
            }
            pn_reg_get(rname, &res);
            return res;
        }

        if (runtime_strcasecmp(s, "SET") == 0 && arg_count >= 3 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            pn_reg_set(str_data(args[1].as.string), args[2]);
            pn_reg_get(str_data(args[1].as.string), &res);
            return res;
        }
    }

    return res;
}

void func_pn_register(void) {
    lang_desc_register(&g_func_pn_desc);

    FunctionEntry entry = {
        .name = "PN",
        .keyword = KW_NONE,
        .category = FCAT_MATH,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 3,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_pn_eval,
        .help_text = "Prefix Polish notation evaluation function",
        .module_name = "Math"
    };
    funcreg_register(&entry);
}

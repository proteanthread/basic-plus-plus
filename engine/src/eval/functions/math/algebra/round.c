// FILENAME: runtime_round.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, runtime_round.h, string.c)
// Provides runtime implementation for the ROUND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/round.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_round_desc = {
    .name = "ROUND",
    .category = "Math Functions",
    .syntax = "ROUND(x [, decimals])",
    .description = "Rounds a numeric expression x to the specified number of decimal places (default 0).",
    .error_summary = "Error 13: Type Mismatch (ROUND expects 1 or 2 numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_round_register(void) {
    lang_desc_register(&g_round_desc);
}

static inline double runtime_banker_round(double x) {
    double f = runtime_floor(x);
    double diff = x - f;
    if (diff > 0.5) {
        return f + 1.0;
    } else if (diff < 0.5) {
        return f;
    } else {
        double half_f = f * 0.5;
        if (half_f == runtime_floor(half_f)) {
            return f;
        } else {
            return f + 1.0;
        }
    }
}

BValue func_round_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    bool is_banker = (runtime_strcmp(uname, "BANKER_ROUND") == 0 ||
                      runtime_strcmp(uname, "BANKROUND") == 0 ||
                      runtime_strcmp(uname, "ROUND_BANK") == 0);
    if (!is_banker && runtime_strcmp(uname, "_ROUND") != 0 &&
        runtime_strcmp(uname, "ROUND") != 0 &&
        runtime_strcmp(uname, "MATH.ROUND") != 0) {
        return res;
    }

    if (arg_count < 1 || arg_count > 2 || args[0].type == VAL_STRING || (arg_count == 2 && args[1].type == VAL_STRING)) {
        err->code = 13;
        err->message = is_banker ? "BANKER_ROUND expects 1 or 2 numeric arguments" : "ROUND expects 1 or 2 numeric arguments";
        return res;
    }

    double val = args[0].as.number;
    if (arg_count == 1) {
        res.type = VAL_NUMBER;
        res.as.number = is_banker ? runtime_banker_round(val) : runtime_round(val);
        return res;
    }

    int decimals = (int)runtime_round(args[1].as.number);
    if (decimals == 0) {
        res.type = VAL_NUMBER;
        res.as.number = is_banker ? runtime_banker_round(val) : runtime_round(val);
        return res;
    }

    double factor = runtime_pow(10.0, (double)decimals);
    res.type = VAL_NUMBER;
    res.as.number = is_banker ? (runtime_banker_round(val * factor) / factor) : (runtime_round(val * factor) / factor);
    return res;
}


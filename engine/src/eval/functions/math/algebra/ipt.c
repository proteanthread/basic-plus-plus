// FILENAME: ipt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h, string.h)
// NEEDS: libengine (ipt.h)
// Provides runtime implementation for the IPT (Integer Part) function in BASIC++.

#include "eval/functions/math/algebra/ipt.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_ipt_desc = {
    .name = "IPT",
    .category = "Math Functions",
    .syntax = "IPT(num) / IP(num)",
    .description = "Returns the integer part of a number (Business BASIC / BBx).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_ipt_register(void) {
    lang_desc_register(&g_ipt_desc);
}

BValue func_ipt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "IPT") != 0 && runtime_strcmp(uname, "_IPT") != 0 &&
        runtime_strcmp(uname, "MATH.IPT") != 0 && runtime_strcmp(uname, "IP") != 0 &&
        runtime_strcmp(uname, "_IP") != 0 && runtime_strcmp(uname, "MATH.IP") != 0) {
        return res;
    }

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "IP/IPT expects 1 numeric argument";
        return res;
    }

    double v = args[0].as.number;
    double intpart;
    runtime_modf(v, &intpart);
    res.as.number = intpart;
    return res;
}

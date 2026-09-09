// FILENAME: datevalue.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (datevalue.h, datetime_common.h)
// Provides runtime implementation for the DATEVALUE function in BASIC++.

#include "eval/functions/datetime/datevalue.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/format/sscanf.h"

static const LangDesc g_datevalue_desc = {
    .name = "DATEVALUE",
    .category = "Date and Time",
    .syntax = "DATEVALUE(date_str$)",
    .description = "Parses date string and returns Microsoft Serial Date double-precision number.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_datevalue_register(void) {
    lang_desc_register(&g_datevalue_desc);
}

BValue func_datevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "DATEVALUE expects date string argument";
        return res;
    }
    const char *str = str_data(args[0].as.string);
    int a = 0, b = 0, c = 0;
    if (runtime_sscanf(str, "%d-%d-%d", &a, &b, &c) == 3 ||
        runtime_sscanf(str, "%d/%d/%d", &a, &b, &c) == 3) {
        if (a > 31) {
            // YYYY-MM-DD
            res.as.number = ymd_to_serial(a, b, c);
        } else if (c > 31) {
            // MM-DD-YYYY
            res.as.number = ymd_to_serial(c, a, b);
        } else {
            // Default MM-DD-YY
            res.as.number = ymd_to_serial(c, a, b);
        }
    }
    return res;
}

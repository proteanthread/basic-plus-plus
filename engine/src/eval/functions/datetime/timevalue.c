// FILENAME: timevalue.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (datetime_common.h, timevalue.h)
// Provides runtime implementation for the TIMEVALUE function in BASIC++.

#include "eval/functions/datetime/timevalue.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/format/sscanf.h"
#include "runtime/string/strops.h"

static const LangDesc g_timevalue_desc = {
    .name = "TIMEVALUE",
    .category = "Date and Time",
    .syntax = "TIMEVALUE(time_str$)",
    .description = "Parses time string and returns fractional Microsoft Serial Time number.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_timevalue_register(void) {
    lang_desc_register(&g_timevalue_desc);
}

BValue func_timevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "TIMEVALUE expects time string argument";
        return res;
    }
    const char *str = str_data(args[0].as.string);
    int h = 0, m = 0, s = 0;
    char ampm[16] = "";
    if (runtime_sscanf(str, "%d:%d:%d %15s", &h, &m, &s, ampm) >= 3 ||
        runtime_sscanf(str, "%d:%d:%d", &h, &m, &s) == 3) {
        if (runtime_strcasecmp(ampm, "PM") == 0 && h < 12) h += 12;
        if (runtime_strcasecmp(ampm, "AM") == 0 && h == 12) h = 0;
        res.as.number = (double)(h * 3600 + m * 60 + s) / 86400.0;
    } else if (runtime_sscanf(str, "%d:%d", &h, &m) == 2) {
        res.as.number = (double)(h * 3600 + m * 60) / 86400.0;
    }
    return res;
}

// FILENAME: clk.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (clk.h)
// Provides runtime implementation and LanguageDescriptor for CLK and CLK$ system variables in BASIC++.

#include "eval/functions/datetime/clk.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"
#include "platform/platform.h"

static const LangDesc g_clk_desc = {
    .name = "CLK",
    .category = "System Functions",
    .syntax = "CLK",
    .description = "Returns current UTC (Zulu) time as an unpunctuated 24-hr numeric value (hhmmss).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

static const LangDesc g_clk_str_desc = {
    .name = "CLK$",
    .category = "System Functions",
    .syntax = "CLK$",
    .description = "Returns current UTC (Zulu) time formatted as a 12-hr delimited string with AM/PM (hh:mm:ss AM/PM).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_clk_register(void) {
    lang_desc_register(&g_clk_desc);
    lang_desc_register(&g_clk_str_desc);
}

BValue func_clk_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    (void)err;
    (void)arg_count;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_b;
    struct tm *gt = platform_gmtime(&raw_t, &tm_b);
    if (!gt) {
        tm_b.tm_hour = 12; tm_b.tm_min = 0; tm_b.tm_sec = 0;
        gt = &tm_b;
    }

    if (uname && runtime_strcmp(uname, "CLK$") == 0) {
        int h12 = (gt->tm_hour % 12 == 0 ? 12 : gt->tm_hour % 12);
        char buf[24];
        runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d %s", h12, gt->tm_min, gt->tm_sec, (gt->tm_hour >= 12 ? "PM" : "AM"));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)(gt->tm_hour * 10000 + gt->tm_min * 100 + gt->tm_sec);
    return res;
}

// FILENAME: utc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (utc.h)
// Provides registration and descriptor for the UTC and UTC$ system variables in BASIC++.

#include "eval/functions/datetime/utc.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_utc_desc = {
    .name = "UTC",
    .category = "System Functions",
    .syntax = "UTC",
    .description = "Returns current UTC timestamp as a 14-digit numeric value (YYYYMMDDhhmmss).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

static const LangDesc g_utc_str_desc = {
    .name = "UTC$",
    .category = "System Functions",
    .syntax = "UTC$",
    .description = "Returns current UTC timestamp formatted as a date/time string without trailing Z (YYYY-MM-DD hh:mm:ss).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_utc_register(void) {
    lang_desc_register(&g_utc_desc);
    lang_desc_register(&g_utc_str_desc);
}

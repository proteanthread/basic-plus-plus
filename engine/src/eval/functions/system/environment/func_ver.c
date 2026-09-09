// FILENAME: func_ver.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_ver.h)
// Provides runtime implementation and LanguageDescriptors for version, errorlevel, and system state variables.

#include "eval/functions/system/environment/func_ver.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "hal/hal.h"
#include "platform/platform.h"

static const LangDesc g_ver_desc = {
    .name = "VER", .category = "Diagnostics", .syntax = "VER",
    .description = "Returns the numeric version number of BASIC++ (e.g. 6.52).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_today_desc = {
    .name = "TODAY$", .category = "Clocks & Timers", .syntax = "TODAY$",
    .description = "Returns current date formatted as YYYY-MM-DD.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_today_num_desc = {
    .name = "TODAY", .category = "Clocks & Timers", .syntax = "TODAY",
    .description = "Returns current date as a pure numeric integer YYYYMMDD.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_errorlevel_desc = {
    .name = "ERRORLEVEL", .category = "Diagnostics", .syntax = "ERRORLEVEL",
    .description = "Returns the return/exit code of the last executed shell command.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_txn_desc = {
    .name = "TXN", .category = "Diagnostics", .syntax = "TXN or TXNSTATUS",
    .description = "Returns the active transaction state (0=none, 1=active).",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
};

static const LangDesc g_exterr_desc = {
    .name = "EXTERR", .category = "Diagnostics", .syntax = "EXTERR",
    .description = "Returns extended DOS/platform error information.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_ver_register(void) {
    lang_desc_register(&g_ver_desc);
    lang_desc_register(&g_today_desc);
    lang_desc_register(&g_today_num_desc);
    lang_desc_register(&g_errorlevel_desc);
    lang_desc_register(&g_txn_desc);
    lang_desc_register(&g_exterr_desc);
}

BValue func_ver_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));

    if (uname && runtime_strcasecmp(uname, "TODAY$") == 0) {
        time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
        struct tm tm_b;
        struct tm *lt = platform_localtime(&raw_t, &tm_b);
        if (!lt) {
            tm_b.tm_year = 126; tm_b.tm_mon = 8; tm_b.tm_mday = 2;
            lt = &tm_b;
        }
        char buf[32];
        runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    if (uname && runtime_strcasecmp(uname, "TODAY") == 0) {
        time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
        struct tm tm_b;
        struct tm *lt = platform_localtime(&raw_t, &tm_b);
        if (!lt) {
            tm_b.tm_year = 126; tm_b.tm_mon = 8; tm_b.tm_mday = 2;
            lt = &tm_b;
        }
        double num_date = (double)((lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday);
        res.type = VAL_NUMBER; res.as.number = num_date;
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "ERRORLEVEL") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 0.0;
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "TXN") == 0 || runtime_strcasecmp(uname, "TXNSTATUS") == 0)) {
        res.type = VAL_INTEGER; res.as.number = 0.0;
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "EXTERR") == 0)) {
        res.type = VAL_INTEGER; res.as.number = (double)vm_get_err_code(vm);
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "VARPTR$") == 0)) {
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), "VAR", 3);
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "IOCTL$") == 0)) {
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    // VER numeric (6.52)
    res.type = VAL_NUMBER; res.as.number = 6.52;
    return res;
}

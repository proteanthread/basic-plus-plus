// FILENAME: clock_num.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (clock_num.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the CLOCK_NUM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/clock_num.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_clock_num_register(void) {
    MicroLibMetadata meta = {
        .name = "CLOCK",
        .category = "System Functions",
        .syntax = "CLOCK()",
        .help_text = "Returns the current system Unix timestamp in seconds.",
        .error_codes = "Error 5: Illegal Function Call (CLOCK expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_clock_num_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CLOCK expects no arguments";
        return res;
    }

    time_t t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    double val = 0.0;
    if (lt) {
        long long yr = (long long)(lt->tm_year + 1900);
        long long mon = (long long)(lt->tm_mon + 1);
        long long mday = (long long)lt->tm_mday;
        long long hr = (long long)lt->tm_hour;
        long long min = (long long)lt->tm_min;
        long long sec = (long long)lt->tm_sec;
        val = (double)(yr * 10000000000LL + mon * 100000000LL + mday * 1000000LL + hr * 10000LL + min * 100LL + sec);
    }

    res.type = VAL_NUMBER;
    res.as.number = val;
    return res;
}

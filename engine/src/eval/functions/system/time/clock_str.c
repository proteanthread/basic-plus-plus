// FILENAME: clock_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (clock_str.h, string.c)
// Provides runtime implementation for the CLOCK_STR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/clock_str.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_clock_str_register(void) {
    MicroLibMetadata meta = {
        .name = "CLOCK$",
        .category = "System Functions",
        .syntax = "CLOCK$()",
        .help_text = "Returns the current ISO 8601 formatted date/time string (YYYY-MM-DD HH:MM:SS).",
        .error_codes = "Error 5: Illegal Function Call (CLOCK$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_clock_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CLOCK$ expects no arguments";
        return res;
    }

    time_t t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_buf;
    extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
    struct tm *gt = platform_gmtime(&t, &tm_buf);

    char buf[64] = "";
    if (gt) {
        runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 gt->tm_year + 1900, gt->tm_mon + 1, gt->tm_mday,
                 gt->tm_hour, gt->tm_min, gt->tm_sec);
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}

// FILENAME: time_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (string.c, time_fn.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TIME_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/time_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_time_fn_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TIME",
        .category = "System Functions",
        .syntax = "TIME(code)",
        .help_text = "Returns DEC PDP-10 timesharing CPU execution seconds (0), seconds since midnight (1), or minutes since midnight (2).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_time_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    int code = 0;
    if (arg_count >= 1) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        code = (int)args[0].as.number;
    }

    time_t rawtime = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_buf;
    struct tm *info = platform_localtime(&rawtime, &tm_buf);


    switch (code) {
        case 0:
            // Elapsed CPU execution time in seconds
            res.as.number = (double)((long)(platform_get_uptime() * 1000.0)) / (double)1000;
            break;
        case 1:
            // Seconds since midnight
            if (info) {
                res.as.number = (double)(info->tm_hour * 3600 + info->tm_min * 60 + info->tm_sec);
            }
            break;
        case 2:
            // Minutes since midnight
            if (info) {
                res.as.number = (double)(info->tm_hour * 60 + info->tm_min);
            }
            break;
        case 3:
            // Milliseconds
            res.as.number = (double)(((long)(platform_get_uptime() * 1000.0)) * 1000 / 1000);
            break;
        case 4:
            // Clock ticks
            res.as.number = (double)((long)(platform_get_uptime() * 1000.0));
            break;
        default:
            res.as.number = 0.0;
            break;
    }

    return res;
}

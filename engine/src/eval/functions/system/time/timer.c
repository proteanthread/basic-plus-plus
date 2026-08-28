// FILENAME: timer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, math.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, string.c, timer.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TIMER built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/timer.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/math.h"
void func_timer_register(void) {
    MicroLibMetadata meta = {
        .name = "TIMER",
        .category = "System Functions",
        .syntax = "TIMER()",
        .help_text = "Returns the number of seconds elapsed since midnight.",
        .error_codes = "Error 5: Illegal Function Call (TIMER expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_timer_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "TIMER / TI expects no arguments";
        return res;
    }

    if (runtime_strcmp(uname, "TI") == 0) {
        double val = platform_get_uptime() + vm_get_ti_offset(vm);
        val = runtime_fmod(val, 999999999.0);
        res.type = VAL_NUMBER;
        res.as.number = val;
    } else {
        time_t t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            double uptime = platform_get_uptime();
            double subsecond = uptime - runtime_floor(uptime);
            val = (double)lt->tm_hour * 3600.0 + (double)lt->tm_min * 60.0 + (double)lt->tm_sec + subsecond;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}

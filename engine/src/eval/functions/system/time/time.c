// FILENAME: time.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libplatform
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, time.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TIME built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/time.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_time_register(void) {
    MicroLibMetadata meta = {
        .name = "TIME$",
        .category = "System Functions",
        .syntax = "TIME$ | TIME$(seconds_or_mins)",
        .help_text = "Returns the current system time string, or formats a numeric second/minute count into HH:MM:SS (DEC BASIC-PLUS).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_time_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count > 1) {
        err->code = 13;
        err->message = "TIME / TIME$ expects at most 1 argument";
        return res;
    }

    if (arg_count == 1) {
        if (args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "Type mismatch in TIME$: expected numeric argument";
            return res;
        }

        int val = (int)args[0].as.number;
        int hours = 0, mins = 0, secs = 0;
        if (val <= 1440) {
            // Minutes since midnight
            hours = (val / 60) % 24;
            mins = val % 60;
            secs = 0;
        } else {
            // Seconds since midnight
            hours = (val / 3600) % 24;
            mins = (val % 3600) / 60;
            secs = val % 60;
        }

        char buf[64] = "";
        runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, mins, secs);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    time_t t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    if (runtime_strcmp(uname, "TIME$") == 0) {
        char buf[64] = "";
        if (lt) {
            runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        double val = 0.0;
        if (lt) {
            val = lt->tm_hour * 10000.0 + lt->tm_min * 100.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}

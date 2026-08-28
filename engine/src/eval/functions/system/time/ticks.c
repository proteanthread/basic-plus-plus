// FILENAME: ticks.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ticks.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TICKS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/ticks.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_ticks_register(void) {
    MicroLibMetadata meta = {
        .name = "TICKS",
        .category = "System Functions",
        .syntax = "TICKS()",
        .help_text = "Returns the number of milliseconds elapsed since system start / platform boot.",
        .error_codes = "Error 5: Illegal Function Call (TICKS expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_ticks_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "TI$ expects no arguments";
        return res;
    }

    double adjusted = platform_get_uptime() + vm_get_ti_offset(vm);
    if (adjusted < 0.0) adjusted = 0.0;

    long long sec_total = (long long)adjusted;
    long long days = sec_total / 86400;
    long long remaining = sec_total % 86400;
    int hr = (int)(remaining / 3600);
    int min = (int)((remaining / 60) % 60);
    int sec = (int)(remaining % 60);

    char buf[64];
    if (days == 0) {
        runtime_snprintf(buf, sizeof(buf), "%02d%02d%02d", hr, min, sec);
    } else {
        runtime_snprintf(buf, sizeof(buf), "%03lld:%02d%02d%02d", days, hr, min, sec);
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}

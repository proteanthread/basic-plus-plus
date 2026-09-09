// FILENAME: timer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, math.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, timer.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TIMER built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/timer.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/math.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_timer_desc = {
    .name = "TIMER",
    .category = "System Functions",
    .syntax = "TIMER",
    .description = "Returns the number of seconds elapsed since midnight (0..86399) with subsecond precision.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_timer_register(void) {
    lang_desc_register(&g_timer_desc);
}

BValue func_timer_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    (void)arg_count;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };

    if (uname && runtime_strcmp(uname, "TI") == 0) {
        double val = platform_get_uptime() + vm_get_ti_offset(vm);
        res.as.number = runtime_fmod(val, 999999999.0);
    } else {
        res.as.number = platform_get_timer();
    }
    return res;
}

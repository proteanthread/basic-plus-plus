// FILENAME: ticks.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ticks.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TICKS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/time/ticks.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_ticks_desc = {
    .name = "TICKS",
    .category = "System Functions",
    .syntax = "TICKS",
    .description = "Returns the number of elapsed 60 Hz timer ticks (1/60th second increments) since system boot.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_ticks_register(void) {
    lang_desc_register(&g_ticks_desc);
}

BValue func_ticks_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    res.as.number = (double)(uint64_t)(platform_get_uptime() * 60.0);
    return res;
}

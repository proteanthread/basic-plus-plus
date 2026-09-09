// FILENAME: func_doevents.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_doevents.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DOEVENTS function in BASIC++.

#include "eval/functions/ui/dialogs/func_doevents.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"

static const LangDesc g_func_doevents_desc = {
    .name = "DOEVENTS",
    .category = "Control",
    .syntax = "numForms% = DOEVENTS()",
    .description = "Yields execution so the operating system can process event queue messages.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_doevents_register(void) {
    lang_desc_register(&g_func_doevents_desc);
}

BValue func_doevents_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    (void)arg_count;
    (void)err;

    platform_sleep_ms(0);

    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    return res;
}

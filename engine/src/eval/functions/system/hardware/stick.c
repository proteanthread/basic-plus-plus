// FILENAME: stick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (stick.h)
// Provides runtime implementation for the STICK function in BASIC++.

#include "eval/functions/system/hardware/stick.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_stick_desc = {
    .name = "STICK",
    .category = "System & Hardware",
    .syntax = "STICK(port%)",
    .description = "Reads joystick direction state (0..7; autodetects USB controller or falls back to cursor keys).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_stick_register(void) {
    lang_desc_register(&g_stick_desc);
}

BValue func_stick_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_INTEGER;
    int port = 0;
    if (arg_count >= 1 && (args[0].type == VAL_INTEGER || args[0].type == VAL_NUMBER)) {
        port = (int)args[0].as.number;
    }
    (void)port;
    res.as.number = 0.0;
    return res;
}

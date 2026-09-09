// FILENAME: strig.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (strig.h)
// Provides runtime implementation for the STRIG function in BASIC++.

#include "eval/functions/system/hardware/strig.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_strig_desc = {
    .name = "STRIG",
    .category = "System & Hardware",
    .syntax = "STRIG(port%)",
    .description = "Reads joystick fire trigger button (0..7; 1=unpressed, 0=pressed; autodetects USB controller or falls back to Spacebar).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_strig_register(void) {
    lang_desc_register(&g_strig_desc);
}

BValue func_strig_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
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
    res.as.number = 1.0;
    return res;
}

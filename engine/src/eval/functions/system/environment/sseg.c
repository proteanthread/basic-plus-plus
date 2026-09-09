// FILENAME: sseg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (sseg.h)
// Provides runtime implementation for the SSEG function in BASIC++.

#include "eval/functions/system/environment/sseg.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_sseg_desc = {
    .name = "SSEG",
    .category = "System Functions",
    .syntax = "SSEG(var)",
    .description = "Returns the segment address of the string descriptor.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_sseg_register(void) {
    lang_desc_register(&g_sseg_desc);
}

BValue func_sseg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)vm; (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0x1000; // Standard DOS data segment
    return res;
}

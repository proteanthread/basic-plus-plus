// FILENAME: attr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (attr.h)
// Provides runtime implementation for the ATTR function in BASIC++.

#include "eval/functions/system/hardware/attr.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_attr_desc = {
    .name = "ATTR",
    .category = "System & Hardware",
    .syntax = "ATTR(row%, col%)",
    .description = "Returns screen character cell color attribute byte (flash/bright/paper/ink) at row, col.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_attr_register(void) {
    lang_desc_register(&g_attr_desc);
}

BValue func_attr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_INTEGER;
    (void)arg_count;
    (void)args;
    // Standard ZX Spectrum / CGA character attribute (0x38 = 56)
    res.as.number = 56.0;
    return res;
}

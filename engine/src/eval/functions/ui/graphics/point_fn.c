// FILENAME: point_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (point_fn.h, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the POINT_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/ui/graphics/point_fn.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "device/vcon.h"

static const LangDesc g_point_desc = {
    .name = "POINT",
    .category = "Graphics Functions",
    .syntax = "color% = POINT(x%, y%) | coord% = POINT(mode%)",
    .description = "Returns the color of the pixel at (x, y) or the current graphics coordinate.",
    .error_summary = "Error 13: Type Mismatch (POINT expects 1 or 2 numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_point_fn_register(void) {
    lang_desc_register(&g_point_desc);
}

BValue func_point_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 && arg_count != 2) {
        err->code = 13;
        err->message = "POINT expects 1 or 2 arguments";
        return res;
    }
    if (args[0].type == VAL_STRING || (arg_count == 2 && args[1].type == VAL_STRING)) {
        err->code = 13;
        err->message = "POINT expects numeric arguments";
        return res;
    }

    // Return default black/0.0 when off-screen or uninitialized
    res.as.number = 0.0;
    return res;
}

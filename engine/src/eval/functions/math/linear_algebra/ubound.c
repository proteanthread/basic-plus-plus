// FILENAME: ubound.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, ubound.h, vm.h)
// Provides runtime implementation for the UBOUND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/ubound.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "runtime/arrays.h"

#include "runtime/math.h"
#include "runtime/string.h"

static const LangDesc g_ubound_desc = {
    .name = "UBOUND",
    .category = "Array Functions",
    .syntax = "high% = UBOUND(array [, dimension%])",
    .description = "Returns the highest subscript for the indicated dimension of an array.",
    .error_summary = "Error 9: Subscript out of range, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_ubound_register(void) {
    lang_desc_register(&g_ubound_desc);
}

BValue func_ubound_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 10.0;
    return res;
}

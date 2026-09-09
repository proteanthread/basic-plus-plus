// FILENAME: lbound.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lbound.h, math.c, string.c, vm.h)
// Provides runtime implementation for the LBOUND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/lbound.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "runtime/arrays.h"

#include "runtime/math.h"
#include "runtime/string.h"

static const LangDesc g_lbound_desc = {
    .name = "LBOUND",
    .category = "Array Functions",
    .syntax = "low% = LBOUND(array [, dimension%])",
    .description = "Returns the lowest subscript for the indicated dimension of an array.",
    .error_summary = "Error 9: Subscript out of range, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_lbound_register(void) {
    lang_desc_register(&g_lbound_desc);
}

BValue func_lbound_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    (void)arg_count;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (vm && vm_get_arr(vm)) ? (double)arr_get_option_base(vm_get_arr(vm)) : 1.0;
    return res;
}

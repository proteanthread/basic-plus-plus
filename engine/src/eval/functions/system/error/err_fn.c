// FILENAME: err_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (err_fn.h, string.c, vm.h)
// Provides runtime implementation for the ERR_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/error/err_fn.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"

#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_err_desc = {
    .name = "ERR",
    .category = "System / Error Functions",
    .syntax = "code% = ERR",
    .description = "Returns the run-time error code of the last occurred error.",
    .error_summary = "Error 13: Type Mismatch (ERR expects no arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_err_fn_register(void) {
    lang_desc_register(&g_err_desc);
}

BValue func_err_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "ERR expects no arguments";
        return res;
    }

    if (vm) {
        res.as.number = (double)vm_get_err_code(vm);
    }
    return res;
}

// FILENAME: erl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (erl.h, string.c, vm.h)
// Provides runtime implementation for the ERL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/error/erl.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"

#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_erl_desc = {
    .name = "ERL",
    .category = "System / Error Functions",
    .syntax = "line% = ERL",
    .description = "Returns the line number where the last error occurred.",
    .error_summary = "Error 13: Type Mismatch (ERL expects no arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_erl_register(void) {
    lang_desc_register(&g_erl_desc);
}

BValue func_erl_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "ERL expects no arguments";
        return res;
    }

    if (vm) {
        res.as.number = (double)vm_get_err_line(vm);
    }
    return res;
}

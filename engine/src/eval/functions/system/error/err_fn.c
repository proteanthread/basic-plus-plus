// FILENAME: err_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (err_fn.h, string.c, vm.h)
// Provides runtime implementation for the ERR_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/error/err_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"

#include "runtime/string.h"
#include "runtime/memory.h"
void func_err_fn_register(void) {
    MicroLibMetadata meta = {
        .name = "ERR",
        .category = "System / Error Functions",
        .syntax = "code% = ERR",
        .help_text = "Returns the run-time error code of the last occurred error.",
        .error_codes = "Error 13: Type Mismatch (ERR expects no arguments)"
    };
    microlib_register(&meta);
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

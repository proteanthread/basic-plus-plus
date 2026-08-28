// FILENAME: erl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (erl.h, string.c, vm.h)
// Provides runtime implementation for the ERL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/error/erl.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"

#include "runtime/string.h"
#include "runtime/memory.h"
void func_erl_register(void) {
    MicroLibMetadata meta = {
        .name = "ERL",
        .category = "System / Error Functions",
        .syntax = "line% = ERL",
        .help_text = "Returns the line number where the last error occurred.",
        .error_codes = "Error 13: Type Mismatch (ERL expects no arguments)"
    };
    microlib_register(&meta);
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

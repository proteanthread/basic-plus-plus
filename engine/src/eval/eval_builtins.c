// FILENAME: eval_builtins.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c)
// NEEDS: libengine (builtins_internal.h, eval.h, eval.c, eval_internal.h, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for eval_builtins within BASIC++.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#include "eval/builtins_internal.h"
#include "eval/eval.h"
#include "eval/eval_internal.h"
#include "runtime/funcreg.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Built-in Function Router ----

// coordinates category-based evaluation for all built-in and intrinsic functions
BValue eval_builtin_function_impl(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (!uname || !vm || !err) {
        if (err) {
            err->code = 5;
            err->line = vm_get_current_line(vm);
        }
        return res;
    }

    if (eval_builtin_math(vm, uname, arg_count, args, err, &res)) {
        return res;
    }

    if (eval_builtin_string(vm, uname, arg_count, args, err, &res)) {
        return res;
    }

    if (eval_builtin_sys(vm, uname, arg_count, args, err, &res)) {
        return res;
    }

    if (eval_builtin_conversion(vm, uname, arg_count, args, err, &res)) {
        return res;
    }

    // Check dynamic function registry if not matched by standard built-in suites
    const FunctionEntry *entry = funcreg_find_by_name(uname);
    if (entry && entry->handler) {
        return entry->handler(args, arg_count, vm);
    }

    err->code = 5; // Illegal function call
    err->line = vm_get_current_line(vm);
    return res;
}

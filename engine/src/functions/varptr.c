// FILENAME: varptr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (varptr.h)
// NEEDS: libengine (string.c, vm.h)
// Provides runtime implementation for the VARPTR built-in function in BASIC++.
//
// ---- Includes ----

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "functions/varptr.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"

BValue func_varptr_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    memset(&res, 0, sizeof(res));

    StringContext *str_ctx = vm_get_str(vm);
    uintptr_t addr = 0;
    if (arg_count > 0) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            addr = (uintptr_t)args[0].as.string;
        } else {
            addr = (uintptr_t)&args[0];
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)addr;

    // Release input string args
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(str_ctx, args[i].as.string);
        }
    }

    return res;
}

BValue func_varptr_str_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    memset(&res, 0, sizeof(res));

    StringContext *str_ctx = vm_get_str(vm);
    uintptr_t addr = 0;
    if (arg_count > 0) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            addr = (uintptr_t)args[0].as.string;
        } else {
            addr = (uintptr_t)&args[0];
        }
    }

    char desc[64];
    snprintf(desc, sizeof(desc), "VARPTR:0x%016llX", (unsigned long long)addr);

    res.type = VAL_STRING;
    res.as.string = str_create(str_ctx, desc, strlen(desc));

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(str_ctx, args[i].as.string);
        }
    }

    return res;
}

void func_varptr_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VARPTR",
        .category = "Variables & Memory",
        .syntax = "VARPTR(var) | VARPTR$(var)",
        .help_text = "Returns the 64-bit integer memory address or descriptor string for a variable.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);

    FunctionEntry entry1 = {
        .name = "VARPTR",
        .keyword = KW_VARPTR,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_PURE,
        .overridable = 0,
        .handler = func_varptr_eval,
        .help_text = "Returns variable memory address",
        .module_name = "Memory"
    };
    funcreg_register(&entry1);

    FunctionEntry entry2 = {
        .name = "VARPTR$",
        .keyword = KW_VARPTR_STR,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_PURE,
        .overridable = 0,
        .handler = func_varptr_str_eval,
        .help_text = "Returns variable memory address descriptor string",
        .module_name = "Memory"
    };
    funcreg_register(&entry2);
}

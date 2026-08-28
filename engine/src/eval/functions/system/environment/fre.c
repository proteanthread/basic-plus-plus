// FILENAME: fre.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (fre.h, string.c)
// Provides runtime implementation for the FRE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/fre.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "memory/memory.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_fre_register(void) {
    MicroLibMetadata meta = {
        .name = "FRE",
        .category = "System Functions",
        .syntax = "FRE(dummy)",
        .help_text = "Returns the number of available free memory bytes in the VM heap.",
        .error_codes = "Error 13: Type Mismatch (FRE expects 1 argument)"
    };
    microlib_register(&meta);
}

BValue func_fre_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "FRE") == 0) {
        if (arg_count > 1) {
            err->code = 13;
            err->message = "FRE expects 0 or 1 arguments";
            return res;
        }
        if (arg_count == 1 && args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
    } else {
        if (arg_count != 0) {
            err->code = 13;
            err->message = "MEM expects no arguments";
            return res;
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    return res;
}

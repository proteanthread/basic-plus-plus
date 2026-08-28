// FILENAME: environ.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (environ.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ENVIRON built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/environ.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_environ_register(void) {
    MicroLibMetadata meta = {
        .name = "ENVIRON$",
        .category = "System Functions",
        .syntax = "ENVIRON$(var_name$)",
        .help_text = "Returns the value of host environment variable var_name$. Returns empty string if not set.",
        .error_codes = "Error 13: Type Mismatch (ENVIRON$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_environ_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "ENVIRON$ expects one string argument";
        return res;
    }

    char *val = platform_getenv(str_data(args[0].as.string));
    res.type = VAL_STRING;
    if (val) {
        res.as.string = str_create(vm_get_str(vm), val, runtime_strlen(val));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }

    str_release(vm_get_str(vm), args[0].as.string);
    return res;
}

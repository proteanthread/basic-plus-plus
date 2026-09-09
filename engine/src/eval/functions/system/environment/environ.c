// FILENAME: environ.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (environ.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ENVIRON built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/environ.h"
#include "platform/platform.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_environ_desc = {
    .name = "ENVIRON$",
    .category = "System Functions",
    .syntax = "ENVIRON$(var_name$)",
    .description = "Returns the value of host environment variable var_name$. Returns empty string if not set.",
    .error_summary = "Error 13: Type Mismatch (ENVIRON$ expects one string argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_environ_register(void) {
    lang_desc_register(&g_environ_desc);
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

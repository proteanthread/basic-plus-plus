// FILENAME: inkey.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (inkey.h, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the INKEY built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/inkey.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_inkey_register(void) {
    MicroLibMetadata meta = {
        .name = "INKEY$",
        .category = "System Functions",
        .syntax = "INKEY$()",
        .help_text = "Reads a character non-blockingly from console buffer. Returns empty string if no key pressed.",
        .error_codes = "Error 5: Illegal Function Call (INKEY$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_inkey_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "INKEY$") != 0 && runtime_strcmp(uname, "INKEY") != 0 &&
        runtime_strcmp(uname, "KEYIN$") != 0 && runtime_strcmp(uname, "KEYIN") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 13;
        err->message = "INKEY$ expects no arguments";
        return res;
    }

    int ch = platform_inkey_char();
    char buf[2] = {0};
    if (ch > 0) {
        buf[0] = (char)ch;
    }
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, buf[0] ? 1 : 0);
    return res;
}

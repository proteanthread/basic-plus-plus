// FILENAME: inkey.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (inkey.h, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the INKEY built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/inkey.h"
#include "platform/platform.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_inkey_desc = {
    .name = "INKEY$",
    .category = "System Functions",
    .syntax = "INKEY$()",
    .description = "Reads a character non-blockingly from console buffer. Returns empty string if no key pressed.",
    .error_summary = "Error 5: Illegal Function Call (INKEY$ expects 0 arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_inkey_register(void) {
    lang_desc_register(&g_inkey_desc);
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

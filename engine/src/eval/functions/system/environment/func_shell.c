// FILENAME: func_shell.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_shell.h, vm.h)
// Provides runtime implementation for the SHELL$ shell capture function in BASIC++.

#include "eval/functions/system/environment/func_shell.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "platform/platform.h"

static const LangDesc g_func_shell_desc = {
    .name = "SHELL$",
    .category = "System & OS",
    .syntax = "SHELL$(cmd$)",
    .description = "Executes an operating system shell command and captures its standard output as a string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_FUNCTION
};

void func_shell_register(void) {
    lang_desc_register(&g_func_shell_desc);
}

BValue func_shell_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "SHELL$ expects one string argument";
        if (arg_count > 0 && args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    const char *cmd = str_data(sr);

    char *captured = platform_execute_capture(cmd);
    str_release(vm_get_str(vm), sr);

    res.type = VAL_STRING;
    if (captured) {
        res.as.string = str_create(vm_get_str(vm), captured, runtime_strlen(captured));
        platform_execute_capture_free(captured);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

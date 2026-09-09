// FILENAME: func_exec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_exec.h, vm.h)
// Provides runtime implementation for the EXEC$ direct process execution function in BASIC++.

#include "eval/functions/system/environment/func_exec.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "platform/platform.h"

static const LangDesc g_func_exec_desc = {
    .name = "EXEC$",
    .category = "System & OS",
    .syntax = "EXEC$(binary$ [, arg1$ [, arg2$...]])",
    .description = "Directly executes a binary executable with arguments and captures standard output without shell interpretation.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_FUNCTION
};

void func_exec_register(void) {
    lang_desc_register(&g_func_exec_desc);
}

BValue func_exec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "EXEC$ expects at least one binary path argument";
        for (int i = 0; i < arg_count; ++i) {
            if (args[i].type == VAL_STRING && args[i].as.string) {
                str_release(vm_get_str(vm), args[i].as.string);
            }
        }
        return res;
    }

    size_t total_len = 0;
    for (int i = 0; i < arg_count; ++i) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            total_len += str_len(args[i].as.string) + 3; // quotes and space
        }
    }

    char *cmd_line = (char *)runtime_malloc(total_len + 16);
    if (!cmd_line) {
        err->code = 7;
        err->message = "Out of memory in EXEC$";
        for (int i = 0; i < arg_count; ++i) {
            if (args[i].type == VAL_STRING && args[i].as.string) {
                str_release(vm_get_str(vm), args[i].as.string);
            }
        }
        return res;
    }
    cmd_line[0] = '\0';

    size_t pos = 0;
    for (int i = 0; i < arg_count; ++i) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            const char *arg_str = str_data(args[i].as.string);
            size_t alen = str_len(args[i].as.string);
            if (i > 0) {
                cmd_line[pos++] = ' ';
            }
            cmd_line[pos++] = '"';
            runtime_memcpy(cmd_line + pos, arg_str, alen);
            pos += alen;
            cmd_line[pos++] = '"';
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }
    cmd_line[pos] = '\0';

    char *captured = platform_execute_capture(cmd_line);
    runtime_free(cmd_line);

    res.type = VAL_STRING;
    if (captured) {
        res.as.string = str_create(vm_get_str(vm), captured, runtime_strlen(captured));
        platform_execute_capture_free(captured);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

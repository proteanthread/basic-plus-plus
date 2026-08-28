// FILENAME: command_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (conversion_fn.c, eval_expr_internal.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (command_fn.h, string.c)
// Provides runtime implementation for the COMMAND_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/command_fn.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
static char g_command_line[2048] = {0};

void func_command_register(void) {
    MicroLibMetadata meta = {
        .name = "COMMAND$",
        .category = "System",
        .syntax = "COMMAND$",
        .help_text = "Returns the command-line arguments string passed to the BASIC program.",
        .error_codes = "None"
    };
    microlib_register(&meta);
}

void runtime_set_command_line(const char *cmd) {
    if (cmd) {
        runtime_strncpy(g_command_line, cmd, sizeof(g_command_line) - 1);
        g_command_line[sizeof(g_command_line) - 1] = '\0';
    } else {
        g_command_line[0] = '\0';
    }
}

const char *runtime_get_command_line(void) {
    return g_command_line;
}

BValue func_command_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    (void)arg_count;
    (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;

    res.as.string = str_create(vm_get_str(vm), g_command_line, runtime_strlen(g_command_line));
    return res;
}

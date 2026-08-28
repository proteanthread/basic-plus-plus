// FILENAME: sys_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, sys_fn.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SYS_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/sys_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "platform/platform.h"

#include "runtime/string.h"
#include "runtime/memory.h"
void func_sys_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SYS",
        .category = "System Functions",
        .syntax = "SYS(code) | SYS(cmd_str) | SYS()",
        .help_text = "Returns DEC PDP-10 / RSTS/E timesharing system metadata, or executes a host shell command.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_sys_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname; (void)err;

    if (arg_count >= 1 && args[0].type == VAL_STRING && args[0].as.string) {
        // Command execution
        const char *cmd = str_data(args[0].as.string);
        platform_execute_command(cmd);
        res.as.number = 0.0;
        return res;
    }

    int code = 0;
    if (arg_count >= 1 && args[0].type != VAL_STRING) {
        code = (int)args[0].as.number;
    }

    switch (code) {
        case 0:
            res.as.number = 1000.0; // Virtual Process ID
            break;
        case 1: {
            BppPlatformId pid = platform_get_id();
            if (pid == PLAT_WINDOWS) res.as.number = 1.0;
            else if (pid == PLAT_POSIX) res.as.number = 2.0;
            else if (pid == PLAT_DOS) res.as.number = 3.0;
            else res.as.number = 5.0;
            break;
        }
        case 2:
            res.as.number = 671088640.0; // 640 MB standard pool
            break;
        case 3:
            res.as.number = 0.0; // Virtual terminal / console index
            break;
        default:
            res.as.number = 0.0;
            break;
    }

    return res;
}

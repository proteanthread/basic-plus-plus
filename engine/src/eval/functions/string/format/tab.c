// FILENAME: tab.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, tab.h, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the TAB built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/format/tab.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "device/vcon.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_tab_register(void) {
    MicroLibMetadata meta = {
        .name = "TAB",
        .category = "Print / Formatting Functions",
        .syntax = "TAB(column%)",
        .help_text = "Positions output at the specified column in a PRINT statement.",
        .error_codes = "Error 13: Type Mismatch (TAB expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_tab_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "TAB expects one numeric argument";
        return res;
    }

    int target_col = (int)args[0].as.number;
    if (target_col < 1) target_col = 1;

    int cur_col = 1;
    VConContext *vcon = vm ? vm_get_vcon(vm) : NULL;
    if (vcon) {
        int r = 0, c = 0;
        int active_idx = vcon_get_active_index(vcon);
        vcon_get_cursor(vcon, active_idx, &r, &c);
        cur_col = c + 1;
    }

    char *buf = NULL;
    size_t len = 0;
    if (target_col >= cur_col) {
        len = (size_t)(target_col - cur_col);
        buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (len + 1)) : NULL);
        if (!buf) {
            err->code = 14;
            err->message = "Out of memory";
            return res;
        }
        runtime_memset(buf, ' ', len);
    } else {
        // If already past target_col, space to target_col on the next line
        len = (size_t)target_col; // '\n' + (target_col - 1) spaces
        buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (len + 1)) : NULL);
        if (!buf) {
            err->code = 14;
            err->message = "Out of memory";
            return res;
        }
        buf[0] = '\n';
        runtime_memset(buf + 1, ' ', target_col - 1);
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm ? vm_get_str(vm) : NULL, buf, (int)len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}

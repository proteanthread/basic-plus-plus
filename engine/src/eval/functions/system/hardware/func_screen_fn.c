// FILENAME: func_screen_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for SCREEN function in BASIC++.

#include "eval/functions/system/hardware/func_screen_fn.h"
#include "runtime/language_descriptor.h"
#include "device/vcon.h"
#include "types/errors.h"

static const LangDesc g_screen_fn_desc = {
    .name = "SCREEN",
    .category = "Console & Screen",
    .syntax = "SCREEN(row%, col% [, flag%])",
    .description = "Returns the ASCII code or color attribute of the character at specified screen coordinates.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_screen_fn_register(void) {
    lang_desc_register(&g_screen_fn_desc);
}

BValue func_screen_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (!vm || !err) return res;

    if (arg_count < 2 || arg_count > 3) {
        err->code = 5;
        err->message = "Illegal Function Call (Expected 2 or 3 arguments)";
        return res;
    }

    if (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER) {
        err->code = 13; err->message = "Type Mismatch"; return res;
    }
    if (args[1].type != VAL_NUMBER && args[1].type != VAL_INTEGER) {
        err->code = 13; err->message = "Type Mismatch"; return res;
    }

    int row = (int)args[0].as.number;
    int col = (int)args[1].as.number;
    int flag = 0;
    if (arg_count >= 3) {
        if (args[2].type != VAL_NUMBER && args[2].type != VAL_INTEGER) {
            err->code = 13; err->message = "Type Mismatch"; return res;
        }
        flag = (int)args[2].as.number;
    }

    if (row < 1 || col < 1) {
        err->code = 5;
        err->message = "Illegal Function Call";
        return res;
    }

    VConContext *vcon = vm_get_vcon(vm);
    if (!vcon) {
        res.as.number = (flag != 0) ? 7.0 : 32.0;
        return res;
    }

    int active_idx = vcon_get_active_index(vcon);
    if (flag != 0) {
        int attr = vcon_get_attr_at(vcon, active_idx, row - 1, col - 1);
        res.as.number = (attr >= 0) ? (double)attr : 7.0;
    } else {
        int ch = vcon_get_char_at(vcon, active_idx, row - 1, col - 1);
        res.as.number = (ch >= 0) ? (double)ch : 32.0;
    }

    return res;
}

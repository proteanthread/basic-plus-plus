// FILENAME: pos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (pos.h, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the POS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/pos.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "device/vcon.h"

#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/memory.h"

static const LangDesc g_pos_desc = {
    .name = "POS",
    .category = "System / Screen Functions",
    .syntax = "POS(dummy%) | POS(s$, target$ [, start])",
    .description = "Returns cursor column (1-indexed) or searches for substring position (Wang 2200).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_pos_register(void) {
    lang_desc_register(&g_pos_desc);
}

BValue func_pos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    // Wang 2200: POS(s$, target$ [, start])
    if (arg_count >= 2 && args && args[0].type == VAL_STRING) {
        if (args[1].type != VAL_STRING) {
            err->code = 13;
            err->message = "POS requires string search target";
            return res;
        }

        const char *s = (args[0].as.string) ? str_data(args[0].as.string) : "";
        size_t slen = (args[0].as.string) ? str_len(args[0].as.string) : 0;
        const char *target = (args[1].as.string) ? str_data(args[1].as.string) : "";
        size_t tlen = (args[1].as.string) ? str_len(args[1].as.string) : 0;

        int start = 1;
        if (arg_count >= 3 && args[2].type != VAL_STRING) {
            start = (int)args[2].as.number;
            if (start < 1) start = 1;
        }

        if (tlen == 0) {
            res.as.number = (start <= (int)slen + 1) ? (double)start : 0.0;
            return res;
        }

        if ((size_t)start > slen || tlen > slen) {
            res.as.number = 0.0;
            return res;
        }

        size_t max_search = slen - tlen;
        for (size_t i = (size_t)(start - 1); i <= max_search; i++) {
            if (runtime_memcmp(s + i, target, tlen) == 0) {
                res.as.number = (double)(i + 1);
                return res;
            }
        }

        res.as.number = 0.0;
        return res;
    }

    // Standard Vintage: POS(dummy) -> cursor column
    int r = 0, c = 0;
    VConContext *vcon = vm ? vm_get_vcon(vm) : NULL;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &r, &c);
        res.as.number = (double)(c + 1);
    }
    return res;
}

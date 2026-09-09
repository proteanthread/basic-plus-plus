// FILENAME: csrlin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (csrlin.h, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the CSRLIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/csrlin.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "device/vcon.h"

#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_csrlin_desc = {
    .name = "CSRLIN",
    .category = "System / Screen Functions",
    .syntax = "row% = CSRLIN",
    .description = "Returns the current vertical line (row) position of the cursor (1-indexed).",
    .error_summary = "Error 13: Type Mismatch (CSRLIN expects no arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_csrlin_register(void) {
    lang_desc_register(&g_csrlin_desc);
}

BValue func_csrlin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CSRLIN expects no arguments";
        return res;
    }

    int r = 0, c = 0;
    VConContext *vcon = vm ? vm_get_vcon(vm) : NULL;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &r, &c);
        res.as.number = (double)(r + 1);
    }
    return res;
}

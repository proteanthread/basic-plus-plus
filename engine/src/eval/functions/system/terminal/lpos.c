// FILENAME: lpos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lpos.h, string.c)
// Provides runtime implementation for the LPOS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/lpos.h"
#include "runtime/language_descriptor.h"

#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_lpos_desc = {
    .name = "LPOS",
    .category = "System / Printer Functions",
    .syntax = "LPOS(printer_channel%)",
    .description = "Returns the current horizontal print position in the printer buffer (1-indexed).",
    .error_summary = "Error 13: Type Mismatch (LPOS expects 1 argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
extern int g_lpos;

void func_lpos_register(void) {
    lang_desc_register(&g_lpos_desc);
}

BValue func_lpos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)g_lpos;

    if (arg_count != 1) {
        err->code = 13;
        err->message = "LPOS expects 1 argument";
        return res;
    }
    return res;
}

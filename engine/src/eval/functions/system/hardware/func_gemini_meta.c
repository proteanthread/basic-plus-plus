// FILENAME: func_gemini_meta.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (gemini.h, gemini.c, string.h, strings.h, strings.c)
// NEEDS: libengine (func_gemini_meta.h, string.c)
// Implements GEMINI.STATUS%() and GEMINI.META$() built-in functions.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_gemini_meta.h"
#include "runtime/gemini.h"
#include "runtime/strings.h"

#include <string.h>

BValue func_gemini_status(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm; (void)argc; (void)argv; (void)err;
    return (BValue){.type = VAL_NUMBER, .as.number = (double)net_gemini_get_last_status()};
}

BValue func_gemini_meta(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *meta = net_gemini_get_last_meta();
    if (!meta) meta = "";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), meta, strlen(meta))};
}

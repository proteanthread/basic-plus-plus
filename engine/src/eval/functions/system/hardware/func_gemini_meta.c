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
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_gemini_status_desc = {
    .name = "GEMINI.STATUS%",
    .category = "Network & Protocols",
    .syntax = "GEMINI.STATUS%()",
    .description = "Returns the integer status code of the last Gemini protocol request.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_gemini_meta_desc = {
    .name = "GEMINI.META$",
    .category = "Network & Protocols",
    .syntax = "GEMINI.META$()",
    .description = "Returns the response header metadata string from the last Gemini protocol transaction.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_gemini_meta_register(void) {
    lang_desc_register(&g_gemini_status_desc);
    lang_desc_register(&g_gemini_meta_desc);
}

BValue func_gemini_status(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm; (void)argc; (void)argv; (void)err;
    return (BValue){.type = VAL_NUMBER, .as.number = (double)net_gemini_get_last_status()};
}

BValue func_gemini_meta(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *meta = net_gemini_get_last_meta();
    if (!meta) meta = "";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), meta, runtime_strlen(meta))};
}

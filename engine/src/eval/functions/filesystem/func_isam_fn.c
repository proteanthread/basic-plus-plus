// FILENAME: func_isam_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h, strings.h), libengine (func_isam_fn.h)
// Provides runtime implementation for KEY$, KEYCOUNT, and ISAM functions in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/func_isam_fn.h"
#include "statements/db/isam/isam.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"

static const LangDesc g_key_str_desc = {
    .name = "KEY$",
    .category = "Database",
    .syntax = "k$ = KEY$(ch) | KEY$[ch]",
    .description = "Returns the current index key value for an open ISAM channel.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_keycount_desc = {
    .name = "KEYCOUNT",
    .category = "Database",
    .syntax = "cnt = KEYCOUNT(ch) | KEYCOUNT[ch]",
    .description = "Returns the number of keys in the active index of an ISAM channel.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_isam_check_desc = {
    .name = "ISAM",
    .category = "Database",
    .syntax = "is_isam = ISAM(ch) | KEYED(ch)",
    .description = "Returns -1 if channel is an active ISAM table, 0 otherwise.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_isam_fn_register(void) {
    lang_desc_register(&g_key_str_desc);
    lang_desc_register(&g_keycount_desc);
    lang_desc_register(&g_isam_check_desc);
}

BValue func_key_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (arg_count < 1 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    const char *k = isam_get_current_key(ch);
    if (k) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), k, runtime_strlen(k));
    }
    return res;
}

BValue func_keycount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 1 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    res.as.number = (double)isam_get_keycount(ch);
    return res;
}

BValue func_isam_check_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 1 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    // Relational truth invariant: -1 for true, 0 for false
    res.as.number = isam_is_active(ch) ? -1.0 : 0.0;
    return res;
}

// FILENAME: lof.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libengine (lof.h, vm.h)
// Provides runtime implementation for the LOF built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/status/lof.h"
#include "runtime/language_descriptor.h"
#include "runtime/file.h"
#include "vm/vm.h"

static const LangDesc g_lof_desc = {
    .name = "LOF",
    .category = "Filesystem Functions",
    .syntax = "length& = LOF(file_num%)",
    .description = "Returns the length of an open file in bytes.",
    .error_summary = "Error 13: Type Mismatch (LOF expects numeric channel)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_lof_register(void) {
    lang_desc_register(&g_lof_desc);
}

BValue func_lof_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "LOF expects numeric channel";
        return res;
    }

    int ch = (int)args[0].as.number;
    res.as.number = (double)file_lof(vm_get_file(vm), ch);
    return res;
}

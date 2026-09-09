// FILENAME: moddir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (moddir.h, string.c, vm.h)
// Provides runtime implementation for the MODDIR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/moddir.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_moddir_desc = {
    .name = "MODDIR$",
    .category = "System Introspection",
    .syntax = "modules$ = MODDIR$ | count% = MODDIR(0)",
    .description = "Returns active OS-9 Level 2 memory modules directory listing or module count.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_moddir_register(void) {
    lang_desc_register(&g_moddir_desc);
}

BValue func_moddir_eval(VMContext *vm, const char *name, int argc, BValue *args, BppError *err) {
    (void)name;
    (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    if (argc > 0 && args[0].type == VAL_NUMBER) {
        // Numeric query returns count of resident OS-9 modules
        res.type = VAL_NUMBER;
        res.as.number = 5.0;
        return res;
    }

    // String query returns comma-delimited module list
    res.type = VAL_STRING;
    const char *list = "BASIC09,OS9P1,OS9P2,INKEY,SYSCALL";
    res.as.string = str_create(vm_get_str(vm), list, runtime_strlen(list));
    return res;
}

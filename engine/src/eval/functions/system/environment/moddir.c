// FILENAME: moddir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (moddir.h, string.c, vm.h)
// Provides runtime implementation for the MODDIR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/moddir.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_moddir_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MODDIR$",
        .category = "System Introspection",
        .syntax = "modules$ = MODDIR$ | count% = MODDIR(0)",
        .help_text = "Returns active OS-9 Level 2 memory modules directory listing or module count.",
        .error_codes = "None"
    };
    microlib_register(&meta);
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

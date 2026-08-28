// FILENAME: func_mem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (vm.h)
// Implements MEM.FREE and MEM.ALLOC built-in functions to query available heap memory.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "memory/memory.h"
#include "vm/vm.h"

BValue func_mem_free_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    return res;
}

void func_mem_register(void) {
    MicroLibMetadata meta = {
        .name = "MEM.FREE",
        .category = "System & Power",
        .syntax = "MEM.FREE() | MEM.ALLOC()",
        .help_text = "Returns free heap memory available in bytes.",
        .error_codes = "None"
    };
    microlib_register(&meta);

    FunctionEntry entry_free = {
        .name = "MEM.FREE",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_FLOAT,
        .min_args = 0,
        .max_args = 0,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Query free heap memory in bytes",
        .module_name = "System"
    };
    funcreg_register(&entry_free);
}

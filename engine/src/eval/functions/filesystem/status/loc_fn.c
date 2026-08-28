// FILENAME: loc_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (loc_fn.h, vm.h)
// Provides runtime implementation for the LOC_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/status/loc_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/file.h"
#include "vm/vm.h"

void func_loc_fn_register(void) {
    MicroLibMetadata meta = {
        .name = "LOC",
        .category = "Filesystem Functions",
        .syntax = "pos& = LOC(file_num%)",
        .help_text = "Returns the current read/write position in an open file.",
        .error_codes = "Error 13: Type Mismatch (LOC expects numeric channel)"
    };
    microlib_register(&meta);
}

BValue func_loc_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "LOC expects numeric channel";
        return res;
    }

    int ch = (int)args[0].as.number;
    res.as.number = (double)file_loc(vm_get_file(vm), ch);
    return res;
}

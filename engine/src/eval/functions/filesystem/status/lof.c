// FILENAME: lof.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (lof.h, vm.h)
// Provides runtime implementation for the LOF built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/status/lof.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/file.h"
#include "vm/vm.h"

void func_lof_register(void) {
    MicroLibMetadata meta = {
        .name = "LOF",
        .category = "Filesystem Functions",
        .syntax = "length& = LOF(file_num%)",
        .help_text = "Returns the length of an open file in bytes.",
        .error_codes = "Error 13: Type Mismatch (LOF expects numeric channel)"
    };
    microlib_register(&meta);
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

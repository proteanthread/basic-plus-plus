// FILENAME: eof_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (eof_fn.h, vm.h)
// Provides runtime implementation for the EOF_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/status/eof_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/file.h"
#include "vm/vm.h"

void func_eof_fn_register(void) {
    MicroLibMetadata meta = {
        .name = "EOF",
        .category = "Filesystem Functions",
        .syntax = "flag% = EOF(file_num%)",
        .help_text = "Returns true (-1) if end-of-file is reached on the specified channel, otherwise false (0).",
        .error_codes = "Error 13: Type Mismatch (EOF expects numeric channel)"
    };
    microlib_register(&meta);
}

BValue func_eof_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "EOF expects numeric channel";
        return res;
    }

    int ch = (int)args[0].as.number;
    res.as.number = file_eof(vm_get_file(vm), ch) ? -1.0 : 0.0;
    return res;
}

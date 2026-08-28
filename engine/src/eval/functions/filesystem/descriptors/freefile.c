// FILENAME: freefile.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (freefile.h, vm.h)
// Provides runtime implementation for the FREEFILE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/descriptors/freefile.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/file.h"
#include "vm/vm.h"

void func_freefile_register(void) {
    MicroLibMetadata meta = {
        .name = "FREEFILE",
        .category = "Filesystem Functions",
        .syntax = "ch% = FREEFILE",
        .help_text = "Returns the next available file channel number.",
        .error_codes = "Error 13: Type Mismatch (FREEFILE expects no arguments)"
    };
    microlib_register(&meta);
}

BValue func_freefile_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "FREEFILE expects no arguments";
        return res;
    }

    int free_ch = 1;
    FileContext *fctx = vm ? vm_get_file(vm) : NULL;
    if (fctx) {
        for (int c = 1; c <= 16; ++c) {
            if (!file_is_open(fctx, c)) {
                free_ch = c;
                break;
            }
        }
    }
    res.as.number = (double)free_ch;
    return res;
}

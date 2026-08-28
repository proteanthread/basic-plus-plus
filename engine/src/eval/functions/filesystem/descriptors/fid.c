// FILENAME: fid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (fid.h, string.c)
// Provides runtime implementation for the FID built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/descriptors/fid.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"

void func_fid_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FID",
        .category = "File & Device Telemetry",
        .syntax = "FID(channel) / FIN(channel)",
        .help_text = "Returns file/device identification string (FID) or numeric position/length info (FIN) (Basic Four / BBx).",
        .error_codes = "Error 13: Type Mismatch, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BValue func_fid_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (runtime_strcmp(uname, "FID") != 0 && runtime_strcmp(uname, "FID$") != 0 &&
        runtime_strcmp(uname, "_FID") != 0 && runtime_strcmp(uname, "_FID$") != 0 &&
        runtime_strcmp(uname, "FILE.FID") != 0 && runtime_strcmp(uname, "FILE.FID$") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "FID expects 1 numeric channel argument";
        return res;
    }

    int ch = (int)args[0].as.number;
    const char *fname = file_get_filename(vm_get_file(vm), ch);
    if (!fname) fname = "";

    res.as.string = str_create(vm_get_str(vm), fname, runtime_strlen(fname));
    return res;
}

BValue func_fin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "FIN") != 0 && runtime_strcmp(uname, "_FIN") != 0 && runtime_strcmp(uname, "FILE.FIN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "FIN expects 1 numeric channel argument";
        return res;
    }

    int ch = (int)args[0].as.number;
    if (file_is_open(vm_get_file(vm), ch)) {
        long loc = file_loc(vm_get_file(vm), ch);
        if (loc <= 0) loc = file_lof(vm_get_file(vm), ch);
        res.as.number = (double)loc;
    }

    return res;
}


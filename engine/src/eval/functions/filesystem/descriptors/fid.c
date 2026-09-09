// FILENAME: fid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (fid.h, string.c)
// Provides runtime implementation for the FID built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/descriptors/fid.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_fid_desc = {
    .name = "FID",
    .category = "File & Device Telemetry",
    .syntax = "FID(channel) / FIN(channel)",
    .description = "Returns file/device identification string (FID) or numeric position/length info (FIN) (Basic Four / BBx).",
    .error_summary = "Error 13: Type Mismatch, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_fid_register(void) {
    lang_desc_register(&g_fid_desc);
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


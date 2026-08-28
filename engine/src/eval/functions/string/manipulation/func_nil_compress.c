// FILENAME: func_nil_compress.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_compress.h, nil_compress.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (func_nil_compress.h, string.c)
// Evaluates COMSTR$(), DECOMSTR$(), NIL.COMPRESS$(), NIL.DECOMPRESS$() built-ins.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/func_nil_compress.h"
#include "runtime/nil_compress.h"
#include "runtime/strings.h"
#include <stdlib.h>
#include <string.h>

BValue func_comstr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13;
        err->message = "COMSTR$ expects string argument";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *raw = str_data(args[0].as.string);
    size_t in_len = str_len(args[0].as.string);

    size_t out_cap = in_len * 2 + 16;
    char *out_buf = (char *)malloc(out_cap);
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in COMSTR$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    size_t out_len = nil_comstring_encode(raw, in_len, out_buf, out_cap);
    BValue res = {.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), out_buf, out_len)};
    free(out_buf);
    return res;
}

BValue func_decomstr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13;
        err->message = "DECOMSTR$ expects string argument";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    const char *comp = str_data(args[0].as.string);
    size_t in_len = str_len(args[0].as.string);

    size_t out_cap = (in_len * 8 < 65536) ? (in_len * 8 + 64) : 65536;
    char *out_buf = (char *)malloc(out_cap);
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in DECOMSTR$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    size_t out_len = nil_comstring_decode(comp, in_len, out_buf, out_cap);
    BValue res = {.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), out_buf, out_len)};
    free(out_buf);
    return res;
}

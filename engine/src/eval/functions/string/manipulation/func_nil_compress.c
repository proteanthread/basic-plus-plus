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
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "memory/memory.h"
#include "vm/vm.h"

static const LangDesc g_comstr_desc = {
    .name = "COMSTR$",
    .category = "String Manipulation",
    .syntax = "COMSTR$(str$)",
    .description = "Compresses input string using RFC 51 Run-Length Encoding and byte packing.",
    .error_summary = "Error 13: Type Mismatch (string argument required)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_decomstr_desc = {
    .name = "DECOMSTR$",
    .category = "String Manipulation",
    .syntax = "DECOMSTR$(comp_str$)",
    .description = "Decompresses an RFC 51 RLE packed byte-string back to plain text.",
    .error_summary = "Error 13: Type Mismatch (string argument required)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_nil_compress_register(void) {
    lang_desc_register(&g_comstr_desc);
    lang_desc_register(&g_decomstr_desc);
}

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
    char stack_buf[512];
    char *out_buf = stack_buf;
    if (out_cap > sizeof(stack_buf)) {
        out_buf = (char *)mem_scratch_alloc(vm_get_mem(vm), out_cap);
    }
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in COMSTR$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    size_t out_len = nil_comstring_encode(raw, in_len, out_buf, out_cap);
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), out_buf, out_len)};
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
    char stack_buf[1024];
    char *out_buf = stack_buf;
    if (out_cap > sizeof(stack_buf)) {
        out_buf = (char *)mem_scratch_alloc(vm_get_mem(vm), out_cap);
    }
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in DECOMSTR$";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    size_t out_len = nil_comstring_decode(comp, in_len, out_buf, out_cap);
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), out_buf, out_len)};
}


// FILENAME: translate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, stmt_translate.c)
// NEEDS: libkernel (vm.h, eval.h, strings.h)
// Provides runtime implementation for TRANSLATE$ character mapping in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/translate.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_translate_desc = {
    .name = "TRANSLATE$",
    .category = "String Manipulation",
    .syntax = "TRANSLATE$(src$, from$, to$) | TRANSLATE$(src$, table$) | TRANSLATE$[from$, to$](src$)",
    .description = "Maps and translates characters in a string according to translation table or pair mappings (Wang 2200).",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_translate_register(void) {
    lang_desc_register(&g_translate_desc);
}

void translate_string_in_place(char *dest, size_t max_len, const char *src, size_t src_len,
                               const char *from_str, size_t from_len,
                               const char *to_str, size_t to_len, size_t *out_len) {
    if (!dest || max_len == 0) return;
    if (!src || src_len == 0) {
        dest[0] = '\0';
        if (out_len) *out_len = 0;
        return;
    }

    uint8_t lut[256];
    for (int i = 0; i < 256; i++) {
        lut[i] = (uint8_t)i;
    }

    if (to_str && to_len > 0) {
        size_t limit = (from_len < to_len) ? from_len : to_len;
        for (size_t k = 0; k < limit; k++) {
            lut[(unsigned char)from_str[k]] = (uint8_t)to_str[k];
        }
    } else if (from_str && from_len == 256) {
        for (int i = 0; i < 256; i++) {
            lut[i] = (uint8_t)from_str[i];
        }
    } else if (from_str && from_len >= 2) {
        for (size_t k = 0; k + 1 < from_len; k += 2) {
            lut[(unsigned char)from_str[k]] = (uint8_t)from_str[k + 1];
        }
    }

    for (size_t i = 0; i < src_len && i + 1 < max_len; i++) {
        dest[i] = (char)lut[(unsigned char)src[i]];
    }
    size_t written = (src_len < max_len - 1) ? src_len : max_len - 1;
    dest[written] = '\0';
    if (out_len) *out_len = written;
}

BValue func_translate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (!args || (arg_count != 2 && arg_count != 3)) {
        err->code = 5;
        err->message = "TRANSLATE$ requires 2 or 3 arguments";
        return res;
    }

    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = 13;
        err->message = "TRANSLATE$ requires string arguments";
        return res;
    }

    const char *src = (args[0].as.string) ? str_data(args[0].as.string) : "";
    size_t src_len = (args[0].as.string) ? str_len(args[0].as.string) : 0;

    const char *from_str = (args[1].as.string) ? str_data(args[1].as.string) : "";
    size_t from_len = (args[1].as.string) ? str_len(args[1].as.string) : 0;

    const char *to_str = NULL;
    size_t to_len = 0;
    if (arg_count == 3) {
        if (args[2].type != VAL_STRING) {
            err->code = 13;
            err->message = "TRANSLATE$ requires string arguments";
            return res;
        }
        to_str = (args[2].as.string) ? str_data(args[2].as.string) : "";
        to_len = (args[2].as.string) ? str_len(args[2].as.string) : 0;
    }

    if (src_len == 0) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    char stack_buf[2048];
    char *dest = stack_buf;

    if (src_len + 1 > sizeof(stack_buf)) {
        dest = (char *)mem_scratch_alloc(vm_get_mem(vm), src_len + 1);
        if (!dest) {
            err->code = 7;
            err->message = "Out of memory in TRANSLATE$";
            return res;
        }
    }

    size_t final_len = 0;
    translate_string_in_place(dest, src_len + 1, src, src_len, from_str, from_len, to_str, to_len, &final_len);

    res.as.string = str_create(vm_get_str(vm), dest, final_len);
    return res;
}

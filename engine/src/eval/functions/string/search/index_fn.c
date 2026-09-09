// FILENAME: index_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (index_fn.h, string.c)
// Provides runtime implementation for INDEX and INDEX$ array and string search in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/index_fn.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/arrays.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_index_desc = {
    .name = "INDEX",
    .category = "Array & String Functions",
    .syntax = "INDEX(arr_or_str, target [, start_pos])",
    .description = "Returns 1-based index of target in numeric array, string array, or string.",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_index_str_desc = {
    .name = "INDEX$",
    .category = "Array & String Functions",
    .syntax = "INDEX$(arr_or_str, target [, start_pos])",
    .description = "Returns 1-based string index pointer of target in string array or string.",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_index_fn_register(void) {
    lang_desc_register(&g_index_desc);
    lang_desc_register(&g_index_str_desc);
}

static BValue make_index_result(VMContext *vm, bool is_str_fn, int idx) {
    BValue res;
    if (is_str_fn) {
        char buf[32];
        runtime_snprintf(buf, sizeof(buf), "%d", idx);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        res.type = VAL_NUMBER;
        res.as.number = (double)idx;
    }
    return res;
}

BValue func_index_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    bool is_str_fn = (uname && (runtime_strcmp(uname, "INDEX$") == 0 || runtime_strcasecmp(uname, "INDEX$") == 0));
    BValue res = make_index_result(vm, is_str_fn, 0);

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "INDEX expects 2 or 3 arguments (target, search [, start_pos])";
        return res;
    }

    int start_pos = 1;
    if (arg_count == 3) {
        if (args[2].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        start_pos = (int)args[2].as.number;
        if (start_pos < 1) start_pos = 1;
    }

    // Case 1: First argument is an Array Reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "Array not found in INDEX";
            return res;
        }

        int total = entry->total_size;
        int found_idx = 0;
        int opt_base = arr_get_option_base(vm_get_arr(vm));
        int start_idx = (arg_count == 3) ? (start_pos - opt_base) : ((opt_base == 0) ? 1 : 0);
        if (start_idx < 0) start_idx = 0;

        if (args[1].type == VAL_STRING && args[1].as.string) {
            const char *target = str_data(args[1].as.string);
            for (int i = start_idx; i < total; i++) {
                if (entry->elements[i].type == VAL_STRING && entry->elements[i].as.string) {
                    if (runtime_strcmp(str_data(entry->elements[i].as.string), target) == 0) {
                        found_idx = i + opt_base;
                        break;
                    }
                }
            }
        } else {
            double target_num = (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 0.0;
            for (int i = start_idx; i < total; i++) {
                if (entry->elements[i].type == VAL_NUMBER || entry->elements[i].type == VAL_INTEGER) {
                    if (entry->elements[i].as.number == target_num) {
                        found_idx = i + opt_base;
                        break;
                    }
                }
            }
        }
        return make_index_result(vm, is_str_fn, found_idx);
    }

    // Case 2: First argument is a String
    if (args[0].type == VAL_STRING && args[0].as.string) {
        if (args[1].type != VAL_STRING || !args[1].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "Type mismatch in INDEX: expected string arguments";
            return res;
        }

        const char *src = str_data(args[0].as.string);
        size_t src_len = str_len(args[0].as.string);
        const char *target = str_data(args[1].as.string);
        size_t target_len = str_len(args[1].as.string);

        if (target_len == 0) {
            int ret_idx = (start_pos <= (int)src_len + 1) ? start_pos : 0;
            return make_index_result(vm, is_str_fn, ret_idx);
        }

        if (start_pos < 1 || (size_t)start_pos > src_len || target_len > src_len) {
            return make_index_result(vm, is_str_fn, 0);
        }

        // Check for Pick MultiValue dynamic array field match (^ or \xfe)
        if (runtime_strchr(src, '\xfe') || runtime_strchr(src, '^')) {
            char delim = runtime_strchr(src, '\xfe') ? '\xfe' : '^';
            int field_idx = 1;
            const char *p = src;
            while (*p) {
                const char *next = runtime_strchr(p, delim);
                size_t flen = next ? (size_t)(next - p) : runtime_strlen(p);
                if (flen == target_len && runtime_strncmp(p, target, flen) == 0) {
                    return make_index_result(vm, is_str_fn, field_idx);
                }
                if (!next) break;
                p = next + 1;
                field_idx++;
            }
        }

        // Standard substring search
        const char *match = runtime_strstr(src + (start_pos - 1), target);
        if (match) {
            int pos = (int)(match - src + 1);
            return make_index_result(vm, is_str_fn, pos);
        } else {
            return make_index_result(vm, is_str_fn, 0);
        }
    }

    err->code = ERR_TYPE_MISMATCH;
    err->message = "INDEX expects array or string as first argument";
    return res;
}

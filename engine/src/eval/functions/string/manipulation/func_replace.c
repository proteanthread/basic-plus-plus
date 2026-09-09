// FILENAME: func_replace.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_replace.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, arrays.h)
// Provides runtime implementation for REPLACE and REPLACE$ in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/func_replace.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/arrays.h"
#include "runtime/set.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "platform/platform.h"

static const LangDesc g_func_replace_desc = {
    .name = "REPLACE$",
    .category = "Array & String Functions",
    .syntax = "REPLACE$(target$, search$, replace$ [, count [, mode]]) | REPLACE(arr, pos, val) | REPLACE$(dyn$, attr, val$)",
    .description = "Replaces elements in numeric arrays, string arrays, strings, or dynamic arrays.",
    .error_summary = "Error 13: Type Mismatch, Error 7: Out of Memory",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_replace_register(void) {
    lang_desc_register(&g_func_replace_desc);
}

static void get_dyn_delims(const char *s, char *out_am, char *out_vm, char *out_svm) {
    *out_am = '^';
    *out_vm = ']';
    *out_svm = '\\';
    if (s) {
        if (runtime_strchr(s, '\xfe')) *out_am = '\xfe';
        if (runtime_strchr(s, '\xfd')) *out_vm = '\xfd';
        if (runtime_strchr(s, '\xfc')) *out_svm = '\xfc';
    }
}

static BValue replace_dynamic_string(VMContext *vm, const char *str_val, int target_attr, const char *new_item) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    char am, vm_c, svm;
    get_dyn_delims(str_val, &am, &vm_c, &svm);
    size_t item_len = runtime_strlen(new_item);
    size_t orig_len = runtime_strlen(str_val);
    if (target_attr <= 0) target_attr = 1;

    const char *p = str_val;
    int cur_attr = 1;
    const char *rep_start = NULL;
    const char *rep_end = NULL;

    while (*p) {
        const char *next = runtime_strchr(p, am);
        if (cur_attr == target_attr) {
            rep_start = p;
            rep_end = next ? next : p + runtime_strlen(p);
            break;
        }
        if (!next) break;
        p = next + 1; cur_attr++;
    }

    if (!rep_start) {
        int pad = target_attr - cur_attr;
        if (pad < 1) pad = 1;
        size_t new_len = orig_len + (size_t)pad + item_len;
        char stack_buf[2048];
        char *out_buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);
        runtime_memcpy(out_buf, str_val, orig_len);
        for (int i = 0; i < pad; i++) out_buf[orig_len + i] = am;
        runtime_memcpy(out_buf + orig_len + pad, new_item, item_len);
        out_buf[new_len] = '\0';
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), out_buf, new_len);
        return res;
    }

    size_t prefix_len = (size_t)(rep_start - str_val);
    size_t suffix_len = runtime_strlen(rep_end);
    size_t new_len = prefix_len + item_len + suffix_len;
    char stack_buf[2048];
    char *out_buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);
    runtime_memcpy(out_buf, str_val, prefix_len);
    runtime_memcpy(out_buf + prefix_len, new_item, item_len);
    runtime_memcpy(out_buf + prefix_len + item_len, rep_end, suffix_len);
    out_buf[new_len] = '\0';
    str_release(vm_get_str(vm), res.as.string);
    res.as.string = str_create(vm_get_str(vm), out_buf, new_len);
    return res;
}

BValue func_replace_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 3 || !args) {
        err->code = 13;
        err->message = "REPLACE expects at least 3 arguments";
        return res;
    }

    // Case 0: First argument is a Set or Group
    if (args[0].type == VAL_SET || args[0].type == VAL_GROUP) {
        int attr = (int)args[1].as.number;
        int val_idx = (arg_count >= 4) ? (int)args[2].as.number : 0;
        int subval_idx = (arg_count >= 5) ? (int)args[3].as.number : 0;
        BValue new_val = args[arg_count - 1];
        return set_dyn_replace(vm_get_str(vm), args[0], attr, val_idx, subval_idx, new_val, err);
    }

    // Case 1: First argument is an Array Reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = 9;
            err->message = "Array not found in REPLACE";
            return res;
        }

        int pos = (int)((args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 1);
        if (pos < 1 || pos > entry->total_size) {
            err->code = 9;
            err->message = "Subscript out of range in REPLACE";
            return res;
        }

        int idx = pos - 1;
        entry->elements[idx] = args[2];
        if (args[2].type == VAL_STRING && args[2].as.string) {
            str_add_ref(args[2].as.string);
        }

        res = args[2];
        return res;
    }

    // Case 2: First argument is a String
    if (args[0].type == VAL_STRING && args[0].as.string) {
        // Subcase 2A: Pick MultiValue dynamic array attribute replace
        if (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) {
            int attr = (int)args[1].as.number;
            int val_idx = (arg_count == 3) ? 2 : (arg_count - 1);
            const char *new_item = "";
            char num_buf[64];
            if (args[val_idx].type == VAL_STRING && args[val_idx].as.string) {
                new_item = str_data(args[val_idx].as.string);
            } else if (args[val_idx].type == VAL_NUMBER || args[val_idx].type == VAL_INTEGER) {
                runtime_snprintf(num_buf, sizeof(num_buf), "%g", args[val_idx].as.number);
                new_item = num_buf;
            }
            return replace_dynamic_string(vm, str_data(args[0].as.string), attr, new_item);
        }

        // Subcase 2B: Standard substring search-and-replace
        if (args[1].type == VAL_STRING && args[1].as.string &&
            args[2].type == VAL_STRING && args[2].as.string) {
            BppStringRef sr_target = args[0].as.string;
            BppStringRef sr_search = args[1].as.string;
            BppStringRef sr_replace = args[2].as.string;
            const char *target = str_data(sr_target);
            size_t target_len = str_len(sr_target);
            const char *search = str_data(sr_search);
            size_t search_len = str_len(sr_search);
            const char *rep = str_data(sr_replace);
            size_t rep_len = str_len(sr_replace);

            int max_count = (arg_count >= 4 && args[3].type != VAL_STRING) ? (int)args[3].as.number : 0;
            int mode = (arg_count >= 5 && args[4].type != VAL_STRING) ? (int)args[4].as.number : 0;

            if (search_len == 0 || target_len == 0) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), target, target_len);
                return res;
            }

            size_t matches = 0;
            size_t i = 0;
            while (i < target_len) {
                bool match = false;
                if (i + search_len <= target_len) {
                    if (mode == 1) {
                        match = (runtime_strncasecmp(target + i, search, search_len) == 0);
                    } else {
                        match = (runtime_memcmp(target + i, search, search_len) == 0);
                    }
                }
                if (match) {
                    matches++;
                    i += search_len;
                    if (max_count > 0 && (int)matches >= max_count) break;
                } else {
                    i++;
                }
            }

            size_t alloc_size = target_len + (matches * (rep_len > search_len ? (rep_len - search_len) : 0)) + 1;
            char stack_buf[2048];
            char *buf = (alloc_size < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), alloc_size);

            size_t out_len = 0;
            i = 0;
            size_t replaced = 0;
            while (i < target_len) {
                bool match = false;
                if (i + search_len <= target_len) {
                    if (mode == 1) {
                        match = (runtime_strncasecmp(target + i, search, search_len) == 0);
                    } else {
                        match = (runtime_memcmp(target + i, search, search_len) == 0);
                    }
                }

                if (match && (max_count <= 0 || (int)replaced < max_count)) {
                    runtime_memcpy(buf + out_len, rep, rep_len);
                    out_len += rep_len;
                    i += search_len;
                    replaced++;
                } else {
                    buf[out_len++] = target[i++];
                }
            }
            buf[out_len] = '\0';

            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, out_len);
            return res;
        }
    }

    err->code = 13;
    err->message = "Invalid arguments for REPLACE";
    return res;
}

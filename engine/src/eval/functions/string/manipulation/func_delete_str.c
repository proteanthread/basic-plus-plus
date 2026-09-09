// FILENAME: func_delete_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_delete_str.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, arrays.h)
// Provides runtime implementation for DELETE and DELETE$ in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/func_delete_str.h"
#include "eval/functions/string/manipulation/func_remove.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/arrays.h"
#include "runtime/set.h"
#include "runtime/string/strops.h"

static const LangDesc g_func_delete_str_desc = {
    .name = "DELETE$",
    .category = "Array & String Functions",
    .syntax = "DELETE$(target$, start, count) | DELETE(arr, pos [, count]) | DELETE$(dyn$, attr)",
    .description = "Deletes elements from numeric arrays, string arrays, strings, or dynamic arrays.",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_delete_str_register(void) {
    lang_desc_register(&g_func_delete_str_desc);
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

static BValue delete_dynamic_string(VMContext *vm, const char *str_val, int target_attr) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (target_attr <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), str_val, runtime_strlen(str_val));
        return res;
    }

    char am, vm_c, svm;
    get_dyn_delims(str_val, &am, &vm_c, &svm);
    const char *p = str_val;
    int cur_attr = 1;
    const char *del_start = NULL;
    const char *del_end = NULL;

    while (*p) {
        const char *next = runtime_strchr(p, am);
        if (cur_attr == target_attr) {
            del_start = p;
            del_end = next ? next + 1 : p + runtime_strlen(p);
            if (!next && del_start > str_val && *(del_start - 1) == am) del_start--;
            break;
        }
        if (!next) break;
        p = next + 1; cur_attr++;
    }

    if (!del_start) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), str_val, runtime_strlen(str_val));
        return res;
    }

    size_t prefix_len = (size_t)(del_start - str_val);
    size_t suffix_len = runtime_strlen(del_end);
    size_t new_len = prefix_len + suffix_len;
    char stack_buf[2048];
    char *out_buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);
    if (prefix_len > 0) runtime_memcpy(out_buf, str_val, prefix_len);
    if (suffix_len > 0) runtime_memcpy(out_buf + prefix_len, del_end, suffix_len);
    out_buf[new_len] = '\0';

    str_release(vm_get_str(vm), res.as.string);
    res.as.string = str_create(vm_get_str(vm), out_buf, new_len);
    return res;
}

BValue func_delete_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 2 || !args) {
        err->code = 13;
        err->message = "DELETE expects at least 2 arguments";
        return res;
    }

    // Case 0: First argument is a Set or Group
    if (args[0].type == VAL_SET || args[0].type == VAL_GROUP) {
        int attr = (int)args[1].as.number;
        int val_idx = (arg_count >= 3) ? (int)args[2].as.number : 0;
        int subval_idx = (arg_count >= 4) ? (int)args[3].as.number : 0;
        return set_dyn_delete(vm_get_str(vm), args[0], attr, val_idx, subval_idx, err);
    }

    // Case 1: First argument is an Array Reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = 9;
            err->message = "Array not found in DELETE";
            return res;
        }

        int pos = (int)((args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 1);
        int cnt = (arg_count >= 3 && (args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER)) ? (int)args[2].as.number : 1;
        if (cnt < 1) cnt = 1;
        if (pos < 1 || pos > entry->total_size) {
            err->code = 9;
            err->message = "Subscript out of range in DELETE";
            return res;
        }

        int del_idx = pos - 1;
        for (int i = del_idx; i + cnt < entry->total_size; i++) {
            entry->elements[i] = entry->elements[i + cnt];
        }
        for (int i = entry->total_size - cnt; i < entry->total_size; i++) {
            if (i >= 0) {
                entry->elements[i].type = (entry->type == VAL_STRING) ? VAL_STRING : VAL_NUMBER;
                entry->elements[i].as.number = 0.0;
            }
        }

        res.type = VAL_NUMBER;
        res.as.number = 1.0;
        return res;
    }

    // Case 2: First argument is a String
    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *src = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);

        // Subcase 2A: Dynamic string attribute deletion (arg_count == 2 or contains ^ or \xfe and arg 2 is small)
        if (arg_count == 2 && (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER)) {
            int target_attr = (int)args[1].as.number;
            return delete_dynamic_string(vm, src, target_attr);
        }

        // Subcase 2C: Pattern deletion DELETE$(target$, pattern$) -> delegates to REMOVE$
        if (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
            return func_remove_eval(vm, uname, arg_count, args, err);
        }

        // Subcase 2B: Positional slice deletion (target$, start, count)
        if (arg_count == 3 && (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) &&
            (args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER)) {
            int raw_start = (int)args[1].as.number;
            int count = (int)args[2].as.number;

            if (count <= 0 || len == 0) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), src, len);
                return res;
            }

            int start_pos = raw_start;
            if (start_pos < 0) start_pos = (int)len + start_pos + 1;
            if (start_pos < 1) start_pos = 1;
            if (start_pos > (int)len) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), src, len);
                return res;
            }

            size_t del_idx = (size_t)(start_pos - 1);
            size_t del_cnt = (size_t)count;
            if (del_idx + del_cnt > len) del_cnt = len - del_idx;

            size_t new_len = len - del_cnt;
            char stack_buf[2048];
            char *buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);

            if (del_idx > 0) runtime_memcpy(buf, src, del_idx);
            size_t after_del = len - (del_idx + del_cnt);
            if (after_del > 0) runtime_memcpy(buf + del_idx, src + del_idx + del_cnt, after_del);
            buf[new_len] = '\0';

            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            return res;
        }
    }

    err->code = 13;
    err->message = "DELETE expects (array, pos [, count]) or (target$, start, count)";
    return res;
}

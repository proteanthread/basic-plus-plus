// FILENAME: func_insert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_insert.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, arrays.h)
// Provides runtime implementation for INSERT and INSERT$ in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/func_insert.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/arrays.h"
#include "runtime/set.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_func_insert_desc = {
    .name = "INSERT$",
    .category = "Array & String Functions",
    .syntax = "INSERT$(target$, source$, pos) | INSERT(arr, pos, val) | INSERT$(dyn$, attr, val$)",
    .description = "Inserts elements into numeric arrays, string arrays, strings, or dynamic arrays.",
    .error_summary = "Error 13: Type Mismatch, Error 7: Out of Memory",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_insert_register(void) {
    lang_desc_register(&g_func_insert_desc);
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

static BValue insert_dynamic_string(VMContext *vm, const char *str_val, int target_attr, const char *new_item) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    char am, vm_c, svm;
    get_dyn_delims(str_val, &am, &vm_c, &svm);
    size_t item_len = runtime_strlen(new_item);
    size_t orig_len = runtime_strlen(str_val);

    if (orig_len == 0) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), new_item, item_len);
        return res;
    }

    if (target_attr <= 1) {
        size_t new_len = item_len + 1 + orig_len;
        char stack_buf[2048];
        char *out_buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);
        runtime_memcpy(out_buf, new_item, item_len);
        out_buf[item_len] = am;
        runtime_memcpy(out_buf + item_len + 1, str_val, orig_len);
        out_buf[new_len] = '\0';
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), out_buf, new_len);
        return res;
    }

    const char *p = str_val;
    int cur_attr = 1;
    const char *ins_pos = NULL;
    while (*p) {
        const char *next = runtime_strchr(p, am);
        if (cur_attr == target_attr) { ins_pos = p; break; }
        if (!next) break;
        p = next + 1; cur_attr++;
    }

    if (!ins_pos) {
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

    size_t prefix_len = (size_t)(ins_pos - str_val);
    size_t suffix_len = runtime_strlen(ins_pos);
    size_t new_len = prefix_len + item_len + 1 + suffix_len;
    char stack_buf[2048];
    char *out_buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);
    runtime_memcpy(out_buf, str_val, prefix_len);
    runtime_memcpy(out_buf + prefix_len, new_item, item_len);
    out_buf[prefix_len + item_len] = am;
    runtime_memcpy(out_buf + prefix_len + item_len + 1, ins_pos, suffix_len);
    out_buf[new_len] = '\0';
    str_release(vm_get_str(vm), res.as.string);
    res.as.string = str_create(vm_get_str(vm), out_buf, new_len);
    return res;
}

BValue func_insert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 3 || !args) {
        err->code = 13;
        err->message = "INSERT expects at least 3 arguments";
        return res;
    }

    // Case 0: First argument is a Set or Group
    if (args[0].type == VAL_SET || args[0].type == VAL_GROUP) {
        int attr = (int)args[1].as.number;
        int val_idx = (arg_count >= 4) ? (int)args[2].as.number : 0;
        int subval_idx = (arg_count >= 5) ? (int)args[3].as.number : 0;
        BValue new_val = args[arg_count - 1];
        return set_dyn_insert(vm_get_str(vm), args[0], attr, val_idx, subval_idx, new_val, err);
    }

    // Case 1: First argument is an Array Reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = 9;
            err->message = "Array not found in INSERT";
            return res;
        }

        int pos = (int)((args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 1);
        if (pos < 1) pos = 1;
        if (pos > entry->total_size) pos = entry->total_size;

        int idx = pos - 1;
        for (int i = entry->total_size - 1; i > idx; i--) {
            entry->elements[i] = entry->elements[i - 1];
        }
        entry->elements[idx] = args[2];
        if (args[2].type == VAL_STRING && args[2].as.string) {
            str_add_ref(args[2].as.string);
        }

        res.type = VAL_NUMBER;
        res.as.number = 1.0;
        return res;
    }

    // Case 2: First argument is a String
    if (args[0].type == VAL_STRING && args[0].as.string) {
        // Subcase 2A: Pick MultiValue dynamic array attribute insert (arg 1 is a number)
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
            return insert_dynamic_string(vm, str_data(args[0].as.string), attr, new_item);
        }

        // Subcase 2B: Standard string insertion (target$, source$, pos)
        if (args[1].type == VAL_STRING && args[1].as.string &&
            (args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER)) {
            const char *target = str_data(args[0].as.string);
            size_t target_len = str_len(args[0].as.string);
            const char *source = str_data(args[1].as.string);
            size_t source_len = str_len(args[1].as.string);
            int raw_pos = (int)args[2].as.number;

            if (source_len == 0) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), target, target_len);
                return res;
            }

            int ins_pos = raw_pos;
            if (ins_pos < 0) ins_pos = (int)target_len + ins_pos + 1;
            if (ins_pos < 1) ins_pos = 1;
            if (ins_pos > (int)target_len + 1) ins_pos = (int)target_len + 1;

            size_t ins_idx = (size_t)(ins_pos - 1);
            size_t new_len = target_len + source_len;
            char stack_buf[2048];
            char *buf = (new_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), new_len + 1);

            if (ins_idx > 0) runtime_memcpy(buf, target, ins_idx);
            runtime_memcpy(buf + ins_idx, source, source_len);
            if (target_len > ins_idx) {
                runtime_memcpy(buf + ins_idx + source_len, target + ins_idx, target_len - ins_idx);
            }
            buf[new_len] = '\0';

            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            return res;
        }
    }

    err->code = 13;
    err->message = "Invalid arguments for INSERT";
    return res;
}

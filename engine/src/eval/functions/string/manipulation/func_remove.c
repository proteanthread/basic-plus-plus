// FILENAME: func_remove.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_remove.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, arrays.h)
// Provides runtime implementation for REMOVE and REMOVE$ in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/func_remove.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/arrays.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"

static const LangDesc g_func_remove_desc = {
    .name = "REMOVE$",
    .category = "Array & String Functions",
    .syntax = "REMOVE$(target$, pattern$ [, count [, mode]]) | REMOVE(arr, val)",
    .description = "Removes matching elements from arrays or occurrences from strings.",
    .error_summary = "Error 13: Type Mismatch, Error 7: Out of Memory",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_remove_register(void) {
    lang_desc_register(&g_func_remove_desc);
}

BValue func_remove_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 2 || !args) {
        err->code = 13;
        err->message = "REMOVE expects at least 2 arguments";
        return res;
    }

    // Case 1: First argument is an Array Reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = 9;
            err->message = "Array not found in REMOVE";
            return res;
        }

        int found_idx = -1;
        if (args[1].type == VAL_STRING && args[1].as.string) {
            const char *target = str_data(args[1].as.string);
            for (int i = 0; i < entry->total_size; i++) {
                if (entry->elements[i].type == VAL_STRING && entry->elements[i].as.string) {
                    if (runtime_strcmp(str_data(entry->elements[i].as.string), target) == 0) {
                        found_idx = i;
                        break;
                    }
                }
            }
        } else {
            double target_num = (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 0.0;
            for (int i = 0; i < entry->total_size; i++) {
                if (entry->elements[i].type == VAL_NUMBER || entry->elements[i].type == VAL_INTEGER) {
                    if (entry->elements[i].as.number == target_num) {
                        found_idx = i;
                        break;
                    }
                }
            }
        }

        if (found_idx >= 0) {
            for (int i = found_idx; i < entry->total_size - 1; i++) {
                entry->elements[i] = entry->elements[i + 1];
            }
            int last = entry->total_size - 1;
            entry->elements[last].type = (entry->type == VAL_STRING) ? VAL_STRING : VAL_NUMBER;
            entry->elements[last].as.number = 0.0;
            res.type = VAL_NUMBER;
            res.as.number = 1.0;
        } else {
            res.type = VAL_NUMBER;
            res.as.number = 0.0;
        }
        return res;
    }

    // Case 2: First argument is a String (Pattern Stripping)
    if (args[0].type == VAL_STRING && args[0].as.string && args[1].type == VAL_STRING && args[1].as.string) {
        BppStringRef sr_target = args[0].as.string;
        BppStringRef sr_pat = args[1].as.string;
        const char *target = str_data(sr_target);
        size_t target_len = str_len(sr_target);
        const char *pat = str_data(sr_pat);
        size_t pat_len = str_len(sr_pat);

        int max_count = (arg_count >= 3 && args[2].type != VAL_STRING) ? (int)args[2].as.number : 0;
        int mode = (arg_count >= 4 && args[3].type != VAL_STRING) ? (int)args[3].as.number : 0;

        if (pat_len == 0 || target_len == 0) {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), target, target_len);
            return res;
        }

        char stack_buf[2048];
        char *buf = (target_len < sizeof(stack_buf)) ? stack_buf : (char *)mem_scratch_alloc(vm_get_mem(vm), target_len + 1);

        size_t out_len = 0;
        size_t i = 0;
        int removed_count = 0;

        while (i < target_len) {
            bool match = false;
            if (i + pat_len <= target_len) {
                if (mode == 1) {
                    match = (runtime_strncasecmp(target + i, pat, pat_len) == 0);
                } else {
                    match = (runtime_memcmp(target + i, pat, pat_len) == 0);
                }
            }

            if (match && (max_count <= 0 || removed_count < max_count)) {
                i += pat_len;
                removed_count++;
            } else {
                buf[out_len++] = target[i++];
            }
        }
        buf[out_len] = '\0';

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, out_len);
        return res;
    }

    err->code = 13;
    err->message = "REMOVE expects (array, val) or (target$, pattern$ [, count [, mode]])";
    return res;
}

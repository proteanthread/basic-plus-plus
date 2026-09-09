// FILENAME: mid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mid.h, string.c, vm.h)
// Provides runtime implementation for the MID built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/mid.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

static const LangDesc g_mid_desc = {
    .name = "MID$",
    .category = "String Functions",
    .syntax = "MID$(str$, start [, length [, step]])",
    .description = "Returns a substring of str$ with optional negative start/length and strided step slicing.",
    .error_summary = "Error 13: Type Mismatch (MID$ argument type error)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_mid_register(void) {
    lang_desc_register(&g_mid_desc);
}

BValue func_mid_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MID$") != 0 && runtime_strcmp(uname, "MID") != 0) {
        return res;
    }

    if ((arg_count < 2 || arg_count > 4) || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects string, start_index [, length [, step]]";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    if (arg_count >= 3 && args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects numeric length argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    if (arg_count == 4 && args[3].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects numeric step argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int start = (int)args[1].as.number;
    const char *data = sr ? str_data(sr) : "";
    size_t len = sr ? str_len(sr) : 0;

    if (len == 0 || start == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        if (sr) str_release(vm_get_str(vm), sr);
        return res;
    }

    size_t start_idx = 0;
    if (start < 0) {
        // Negative indexing from end of string: -1 is last character
        long long calc_start = (long long)len + start + 1;
        if (calc_start <= 0) calc_start = 1;
        if ((size_t)calc_start > len) {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), "", 0);
            if (sr) str_release(vm_get_str(vm), sr);
            return res;
        }
        start_idx = (size_t)(calc_start - 1);
    } else {
        // Classic 1-based positive start index
        if ((size_t)start > len) {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), "", 0);
            if (sr) str_release(vm_get_str(vm), sr);
            return res;
        }
        start_idx = (size_t)(start - 1);
    }

    int step = 1;
    if (arg_count == 4) {
        step = (int)args[3].as.number;
        if (step == 0) {
            step = 1;
        }
    }

    size_t count = 0;
    if (step < 0) {
        size_t available = start_idx + 1;
        count = available;
        if (arg_count >= 3) {
            int n = (int)args[2].as.number;
            if (n >= 0 && (size_t)n < count) {
                count = (size_t)n;
            }
        }
    } else {
        size_t remaining = len - start_idx;
        count = remaining;
        if (arg_count >= 3) {
            int n = (int)args[2].as.number;
            if (n < 0) {
                size_t skip_from_end = (size_t)(-n);
                if (start_idx + skip_from_end >= len) {
                    count = 0;
                } else {
                    count = len - start_idx - skip_from_end;
                }
            } else if ((size_t)n < count) {
                count = (size_t)n;
            }
        }
    }

    if (step == 1) {
        // Fast path: contiguous substring
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), data + start_idx, count);
        if (sr) str_release(vm_get_str(vm), sr);
        return res;
    }

    if (step > 0) {
        // Strided forward step slice
        size_t out_len = (count + (size_t)step - 1) / (size_t)step;
        char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(out_len + 1) : NULL);
        if (!buf) {
            if (sr) str_release(vm_get_str(vm), sr);
            err->code = 14;
            err->message = "Out of memory";
            return res;
        }

        size_t out_idx = 0;
        for (size_t i = 0; i < count && out_idx < out_len; i += (size_t)step) {
            buf[out_idx++] = data[start_idx + i];
        }
        buf[out_idx] = '\0';

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, out_idx);
        if (hal_get()) hal_get()->mem.free(buf);
        if (sr) str_release(vm_get_str(vm), sr);
        return res;
    }

    // Strided reverse step slice (step < 0)
    size_t abs_step = (size_t)(-step);
    size_t out_len = (count + abs_step - 1) / abs_step;
    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(out_len + 1) : NULL);
    if (!buf) {
        if (sr) str_release(vm_get_str(vm), sr);
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    size_t out_idx = 0;
    for (size_t i = 0; i < count && out_idx < out_len; i += abs_step) {
        if (start_idx >= i) {
            buf[out_idx++] = data[start_idx - i];
        } else {
            break;
        }
    }
    buf[out_idx] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, out_idx);
    if (hal_get()) hal_get()->mem.free(buf);
    if (sr) str_release(vm_get_str(vm), sr);
    return res;
}

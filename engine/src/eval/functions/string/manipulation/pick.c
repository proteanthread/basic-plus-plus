// FILENAME: pick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (pick.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for standard Pick built-in functions in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/pick.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "types/errors.h"
#include "memory/memory.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/arrays.h"
#include "runtime/set.h"
#include "runtime/string.h"
#include "statements/variables/data/pick_locate.h"
#include "hal/hal.h"

static const LangDesc g_field_desc = {
    .name = "FIELD",
    .category = "Pick Dynamic Arrays",
    .syntax = "FIELD(str_expr, delim_expr, instance_expr [, count_expr])",
    .description = "Extracts one or more delimited substrings from a string (Pick OS / Universe).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_extract_desc = {
    .name = "EXTRACT",
    .category = "Pick Dynamic Arrays",
    .syntax = "EXTRACT(dyn_arr, attr [, val [, subval]])",
    .description = "Extracts an attribute, value, or subvalue from a Pick dynamic array string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_count_desc = {
    .name = "COUNT",
    .category = "Array & String Functions",
    .syntax = "COUNT(arr_or_str [, match])",
    .description = "Returns element count of array or occurrences of substring in string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_count_str_desc = {
    .name = "COUNT$",
    .category = "Array & String Functions",
    .syntax = "COUNT$(arr$ [, match$]) | COUNT$(dyn$ [, delim$])",
    .description = "Returns element count of string array or field count of delimited dynamic string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_dynarray_desc = {
    .name = "DYNARRAY$",
    .category = "Pick Dynamic Arrays",
    .syntax = "DYNARRAY$(set_or_group)",
    .description = "Serializes a Set or Group structure into a Pick MultiValue dynamic array string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_parse_dynarray_desc = {
    .name = "PARSE_DYNARRAY",
    .category = "Pick Dynamic Arrays",
    .syntax = "PARSE_DYNARRAY(dyn_str$) | GROUP_MAP(dyn_str$)",
    .description = "Parses a Pick dynamic array string into a three-tier Set/Group structure.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_pick_register(void) {
    lang_desc_register(&g_field_desc);
    lang_desc_register(&g_extract_desc);
    lang_desc_register(&g_count_desc);
    lang_desc_register(&g_count_str_desc);
    lang_desc_register(&g_dynarray_desc);
    lang_desc_register(&g_parse_dynarray_desc);
}

BValue func_count_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count < 1 || !args) {
        err->code = ERR_SYNTAX;
        err->message = "COUNT expects at least 1 argument";
        return res;
    }

    // Case: Set or Group
    if (args[0].type == VAL_SET && args[0].as.set) {
        if (arg_count == 1) {
            res.as.number = (double)args[0].as.set->count;
            return res;
        }
        int matches = 0;
        for (int i = 0; i < args[0].as.set->count; i++) {
            if (value_equals(args[0].as.set->items[i], args[1])) matches++;
        }
        res.as.number = (double)matches;
        return res;
    }
    if (args[0].type == VAL_GROUP && args[0].as.group) {
        res.as.number = (double)args[0].as.group->count;
        return res;
    }

    // Case 1: Array reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        const char *arr_name = str_data(args[0].as.string);
        ArrayEntry *entry = arr_find_entry(vm_get_arr(vm), arr_name);
        if (!entry) {
            err->code = 9;
            err->message = "Array not found in COUNT";
            return res;
        }
        if (arg_count == 1) {
            res.as.number = (double)entry->total_size;
            return res;
        }
        int matches = 0;
        if (args[1].type == VAL_STRING && args[1].as.string) {
            const char *target = str_data(args[1].as.string);
            for (int i = 0; i < entry->total_size; i++) {
                if (entry->elements[i].type == VAL_STRING && entry->elements[i].as.string) {
                    if (runtime_strcmp(str_data(entry->elements[i].as.string), target) == 0) matches++;
                }
            }
        } else {
            double target_num = (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) ? args[1].as.number : 0.0;
            for (int i = 0; i < entry->total_size; i++) {
                if (entry->elements[i].type == VAL_NUMBER || entry->elements[i].type == VAL_INTEGER) {
                    if (entry->elements[i].as.number == target_num) matches++;
                }
            }
        }
        res.as.number = (double)matches;
        return res;
    }

    // Case 2: String occurrence search COUNT(str$, substr$)
    if (args[0].type == VAL_STRING && args[0].as.string) {
        if (arg_count != 2 || args[1].type != VAL_STRING || !args[1].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        const char *str_val = str_data(args[0].as.string);
        const char *sub = str_data(args[1].as.string);
        if (runtime_strlen(str_val) == 0 || runtime_strlen(sub) == 0) return res;

        int count = 0;
        size_t slen = runtime_strlen(sub);
        const char *p = str_val;
        while ((p = runtime_strstr(p, sub)) != NULL) {
            count++;
            p += slen;
        }
        res.as.number = (double)count;
        return res;
    }

    err->code = ERR_TYPE_MISMATCH;
    return res;
}

BValue func_count_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count < 1 || !args) {
        err->code = ERR_SYNTAX;
        err->message = "COUNT$ expects at least 1 argument";
        return res;
    }

    // Case 1: String array reference
    if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        return func_count_eval(vm, uname, arg_count, args, err);
    }

    // Case 2: Delimited dynamic array string field count COUNT$(dyn$ [, delim$])
    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *str_val = str_data(args[0].as.string);
        if (runtime_strlen(str_val) == 0) return res;

        const char *delim = "^";
        if (runtime_strchr(str_val, '\xfe')) delim = "\xfe";
        if (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
            delim = str_data(args[1].as.string);
        }

        if (runtime_strlen(delim) == 0) {
            res.as.number = (double)runtime_strlen(str_val);
            return res;
        }

        int count = 1;
        size_t dlen = runtime_strlen(delim);
        const char *p = str_val;
        while ((p = runtime_strstr(p, delim)) != NULL) {
            count++;
            p += dlen;
        }
        res.as.number = (double)count;
        return res;
    }

    err->code = ERR_TYPE_MISMATCH;
    return res;
}


BValue func_field_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    (void)uname;

    if (arg_count < 3 || arg_count > 4) {
        err->code = ERR_SYNTAX;
        err->message = "FIELD expects 3 or 4 arguments: FIELD(string, delimiter, instance [, count])";
        return res;
    }

    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    const char *delim = args[1].as.string ? str_data(args[1].as.string) : "";
    int instance = (int)args[2].as.number;
    int count = (arg_count == 4) ? (int)args[3].as.number : 1;

    if (instance < 1 || count < 1 || runtime_strlen(str_val) == 0 || runtime_strlen(delim) == 0) {
        return res;
    }

    size_t dlen = runtime_strlen(delim);
    const char *cur = str_val;
    int cur_field = 1;

    while (cur_field < instance && cur) {
        const char *next = runtime_strstr(cur, delim);
        if (!next) return res;
        cur = next + dlen;
        cur_field++;
    }

    if (!cur) return res;

    const char *start_pos = cur;
    const char *end_pos = NULL;
    for (int i = 0; i < count; i++) {
        const char *next = runtime_strstr(cur, delim);
        if (next) {
            end_pos = next;
            cur = next + dlen;
        } else {
            end_pos = str_val + runtime_strlen(str_val);
            break;
        }
    }

    size_t len = (size_t)(end_pos - start_pos);
    str_release(vm_get_str(vm), res.as.string);
    res.as.string = str_create(vm_get_str(vm), start_pos, len);
    return res;
}

BValue func_extract_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    (void)uname;

    if (arg_count < 2 || arg_count > 4) {
        err->code = ERR_SYNTAX;
        err->message = "EXTRACT expects 2 to 4 arguments: EXTRACT(dyn_arr, attr [, val [, subval]])";
        return res;
    }

    int attr = (int)args[1].as.number;
    int val = (arg_count >= 3) ? (int)args[2].as.number : 0;
    int subval = (arg_count >= 4) ? (int)args[3].as.number : 0;

    if (args[0].type == VAL_SET || args[0].type == VAL_GROUP) {
        str_release(vm_get_str(vm), res.as.string);
        return set_dyn_extract(vm_get_str(vm), args[0], attr, val, subval, err);
    }

    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";

    if (attr <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), str_val, runtime_strlen(str_val));
        return res;
    }

    char am_delim[2] = {'^', 0};
    if (runtime_strchr(str_val, '\xfe')) am_delim[0] = '\xfe';

    BValue a_args[3];
    a_args[0] = args[0];
    a_args[1].type = VAL_STRING;
    a_args[1].as.string = str_create(vm_get_str(vm), am_delim, 1);
    a_args[2].type = VAL_NUMBER;
    a_args[2].as.number = (double)attr;

    BValue a_res = func_field_eval(vm, "FIELD", 3, a_args, err);
    str_release(vm_get_str(vm), a_args[1].as.string);
    if (err->code != 0 || val <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        return a_res;
    }

    char vm_delim[2] = {']', 0};
    const char *a_str = a_res.as.string ? str_data(a_res.as.string) : "";
    if (runtime_strchr(a_str, '\xfd')) vm_delim[0] = '\xfd';

    BValue v_args[3];
    v_args[0] = a_res;
    v_args[1].type = VAL_STRING;
    v_args[1].as.string = str_create(vm_get_str(vm), vm_delim, 1);
    v_args[2].type = VAL_NUMBER;
    v_args[2].as.number = (double)val;

    BValue v_res = func_field_eval(vm, "FIELD", 3, v_args, err);
    str_release(vm_get_str(vm), v_args[1].as.string);
    str_release(vm_get_str(vm), a_res.as.string);
    if (err->code != 0 || subval <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        return v_res;
    }

    char svm_delim[2] = {'\\', 0};
    const char *v_str = v_res.as.string ? str_data(v_res.as.string) : "";
    if (runtime_strchr(v_str, '\xfc')) svm_delim[0] = '\xfc';

    BValue s_args[3];
    s_args[0] = v_res;
    s_args[1].type = VAL_STRING;
    s_args[1].as.string = str_create(vm_get_str(vm), svm_delim, 1);
    s_args[2].type = VAL_NUMBER;
    s_args[2].as.number = (double)subval;

    BValue s_res = func_field_eval(vm, "FIELD", 3, s_args, err);
    str_release(vm_get_str(vm), s_args[1].as.string);
    str_release(vm_get_str(vm), v_res.as.string);
    str_release(vm_get_str(vm), res.as.string);
    return s_res;
}

BValue func_dynarray_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (arg_count < 1 || !args) {
        err->code = ERR_SYNTAX;
        err->message = "DYNARRAY$ expects 1 argument";
        return res;
    }

    char *ser = set_to_dynarray(vm_get_str(vm), args[0]);
    if (ser) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), ser, runtime_strlen(ser));
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) hal->mem.free(ser);
    }
    return res;
}

BValue func_parse_dynarray_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    if (arg_count < 1 || !args || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "PARSE_DYNARRAY / GROUP_MAP expects string argument";
        return res;
    }

    res.type = VAL_SET;
    res.as.set = set_from_dynarray(vm_get_str(vm), str_data(args[0].as.string));
    return res;
}

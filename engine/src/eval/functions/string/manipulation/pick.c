// FILENAME: pick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (pick.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PICK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/pick.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "types/errors.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_pick_register(void) {
    static const MicroLibMetadata meta_dcount = {
        .name = "DCOUNT",
        .category = "Pick Dynamic Arrays",
        .syntax = "DCOUNT(str_expr, delim_expr)",
        .help_text = "Returns the count of delimited fields in a string (Pick OS / PICK/BASIC).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta_dcount);

    static const MicroLibMetadata meta_field = {
        .name = "FIELD",
        .category = "Pick Dynamic Arrays",
        .syntax = "FIELD(str_expr, delim_expr, instance_expr [, count_expr])",
        .help_text = "Extracts one or more delimited substrings from a string (Pick OS / Universe).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta_field);

    static const MicroLibMetadata meta_extract = {
        .name = "EXTRACT",
        .category = "Pick Dynamic Arrays",
        .syntax = "EXTRACT(dyn_arr, attr [, val [, subval]])",
        .help_text = "Extracts an attribute, value, or subvalue from a Pick dynamic array string.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta_extract);
}

BValue func_dcount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count < 1 || arg_count > 2) {
        err->code = ERR_SYNTAX;
        err->message = "DCOUNT expects 1 or 2 arguments: DCOUNT(string, delimiter)";
        return res;
    }

    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "Type mismatch in DCOUNT: string expected for argument 1";
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    if (runtime_strlen(str_val) == 0) {
        res.as.number = 0.0;
        return res;
    }

    const char *delim = "^"; // Default attribute mark or delimiter
    if (arg_count == 2) {
        if (args[1].type != VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "Type mismatch in DCOUNT: delimiter must be string";
            return res;
        }
        delim = args[1].as.string ? str_data(args[1].as.string) : "^";
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

BValue func_count_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 2) {
        err->code = ERR_SYNTAX;
        err->message = "COUNT expects 2 arguments: COUNT(string, substring)";
        return res;
    }

    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    const char *sub = args[1].as.string ? str_data(args[1].as.string) : "";
    if (runtime_strlen(str_val) == 0 || runtime_strlen(sub) == 0) {
        res.as.number = 0.0;
        return res;
    }

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

    // Advance to the starting instance
    while (cur_field < instance && cur) {
        const char *next = runtime_strstr(cur, delim);
        if (!next) return res;
        cur = next + dlen;
        cur_field++;
    }

    if (!cur) return res;

    // Collect 'count' fields
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

    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    int attr = (int)args[1].as.number;
    int val = (arg_count >= 3) ? (int)args[2].as.number : 0;
    int subval = (arg_count >= 4) ? (int)args[3].as.number : 0;

    if (attr <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), str_val, runtime_strlen(str_val));
        return res;
    }

    // Extract Attribute (delimiter '^' or char 254 / 0xFE)
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

    // Extract Value (delimiter ']' or char 253 / 0xFD)
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

    // Extract Subvalue (delimiter '\\' or char 252 / 0xFC)
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

BValue func_insert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    if (arg_count < 3) {
        err->code = ERR_SYNTAX;
        return res;
    }
    return res;
}

BValue func_delete_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    if (arg_count < 2) {
        err->code = ERR_SYNTAX;
        return res;
    }
    return res;
}

BValue func_replace_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    if (arg_count < 3) {
        err->code = ERR_SYNTAX;
        return res;
    }
    return res;
}

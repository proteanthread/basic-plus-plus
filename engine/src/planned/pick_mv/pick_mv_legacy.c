// FILENAME: pick_mv_legacy.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: Planned Pick compatibility module
// NEEDS: libkernel (bvalue.h, errors.h), libcore (strings.h, memory.h)
// Provides shelved implementations of DINDEX, DINSERT, DREPLACE, DDELETE, DCOUNT.

#include "engine/src/planned/pick_mv/pick_mv_legacy.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "memory/memory.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "statements/variables/data/pick_locate.h"

BValue planned_dcount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
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

    const char *delim = "^";
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

BValue planned_dindex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_SYNTAX;
        err->message = "DINDEX expects 2 or 3 arguments: DINDEX(dyn_arr, target [, order])";
        return res;
    }

    const char *dyn_str = (args[0].type == VAL_STRING && args[0].as.string) ? str_data(args[0].as.string) : "";
    const char *target_str = (args[1].type == VAL_STRING && args[1].as.string) ? str_data(args[1].as.string) : "";
    const char *order = (arg_count == 3 && args[2].type == VAL_STRING && args[2].as.string) ? str_data(args[2].as.string) : NULL;

    int pos = 0;
    if (pick_locate_search(target_str, dyn_str, &pos, order)) {
        res.as.number = (double)pos;
    } else {
        res.as.number = 0.0;
    }
    return res;
}

static void planned_get_delims(const char *s, char *out_am, char *out_vm, char *out_svm) {
    *out_am = '^';
    *out_vm = ']';
    *out_svm = '\\';
    if (s) {
        if (runtime_strchr(s, '\xfe')) *out_am = '\xfe';
        if (runtime_strchr(s, '\xfd')) *out_vm = '\xfd';
        if (runtime_strchr(s, '\xfc')) *out_svm = '\xfc';
    }
}

BValue planned_dinsert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (arg_count < 3 || arg_count > 5) {
        err->code = ERR_SYNTAX;
        err->message = "DINSERT expects 3 to 5 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    int target_attr = (int)args[1].as.number;
    int val_idx = (arg_count == 3) ? 2 : (arg_count - 1);
    const char *new_item = "";
    char num_item_buf[64];
    if (args[val_idx].type == VAL_STRING && args[val_idx].as.string) {
        new_item = str_data(args[val_idx].as.string);
    } else if (args[val_idx].type == VAL_NUMBER || args[val_idx].type == VAL_INTEGER) {
        runtime_snprintf(num_item_buf, sizeof(num_item_buf), "%g", args[val_idx].as.number);
        new_item = num_item_buf;
    }

    char am, vm_c, svm;
    planned_get_delims(str_val, &am, &vm_c, &svm);
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

BValue planned_dreplace_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (arg_count < 3 || arg_count > 5) {
        err->code = ERR_SYNTAX;
        err->message = "DREPLACE expects 3 to 5 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    int target_attr = (int)args[1].as.number;
    int val_idx = (arg_count == 3) ? 2 : (arg_count - 1);
    const char *new_item = "";
    char num_item_buf[64];
    if (args[val_idx].type == VAL_STRING && args[val_idx].as.string) {
        new_item = str_data(args[val_idx].as.string);
    } else if (args[val_idx].type == VAL_NUMBER || args[val_idx].type == VAL_INTEGER) {
        runtime_snprintf(num_item_buf, sizeof(num_item_buf), "%g", args[val_idx].as.number);
        new_item = num_item_buf;
    }

    char am, vm_c, svm;
    planned_get_delims(str_val, &am, &vm_c, &svm);
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

BValue planned_ddelete_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);

    if (arg_count < 2 || arg_count > 4) {
        err->code = ERR_SYNTAX;
        err->message = "DDELETE expects 2 to 4 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *str_val = args[0].as.string ? str_data(args[0].as.string) : "";
    int target_attr = (int)args[1].as.number;
    if (target_attr <= 0) {
        str_release(vm_get_str(vm), res.as.string);
        res.as.string = str_create(vm_get_str(vm), str_val, runtime_strlen(str_val));
        return res;
    }

    char am, vm_c, svm;
    planned_get_delims(str_val, &am, &vm_c, &svm);
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

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file string_ext.c
 * @brief Extended string manipulation functions.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements advanced string functions like SPLIT$, LPAD$, RPAD$, SPRINTF$, LSET$, and RSET$.
 * - Why it exists: Matches and exceeds modern BASIC string handling capabilities.
 * - Why it works this way: Utilizes isolated StringContext for memory safety and snprintf for C-style formatting.
 */

#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include "runtime/num_format.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* SPLIT$(string$, delimiter$, index) -> String */
BValue string_split_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_NUMBER) {
        return bval_float(0);
    }
    const char *str = str_data(args[0].as.string);
    const char *delim = str_data(args[1].as.string);
    int index = (int)args[2].as.number;
    
    if (index < 0 || strlen(delim) == 0) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    int current_idx = 0;
    const char *start = str;
    const char *end = strstr(start, delim);
    
    while (end != NULL) {
        if (current_idx == index) {
            BValue res;
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), start, end - start);
            return res;
        }
        start = end + strlen(delim);
        end = strstr(start, delim);
        current_idx++;
    }
    
    if (current_idx == index) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), start, strlen(start));
        return res;
    }
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), "", 0);
    return res;
}

/* SPRINTF$(format$, arg1, ...) -> String */
BValue string_sprintf_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    
    /*
     * Safe format string handling: We parse the format string manually
     * and only allow a restricted set of safe format specifiers.
     * This prevents:
     *   - %n (memory write via format string)
     *   - %s with numeric arg (undefined behavior / crash)
     *   - %d/%f with string arg (undefined behavior)
     *   - Any other dangerous format specifiers
     */
    const char *fmt = str_data(args[0].as.string);
    char buf[1024];
    size_t buf_pos = 0;
    int arg_idx = 1; /* Next argument to consume (args[0] is the format string) */
    
    for (const char *p = fmt; *p != '\0' && buf_pos < sizeof(buf) - 1; p++) {
        if (*p != '%') {
            buf[buf_pos++] = *p;
            continue;
        }
        
        /* Found '%' - parse the format specifier */
        const char *spec_start = p;
        p++; /* Skip '%' */
        
        if (*p == '\0') {
            /* Trailing '%' at end of string - emit it literally */
            buf[buf_pos++] = '%';
            break;
        }
        
        /* Handle %% (literal percent) */
        if (*p == '%') {
            buf[buf_pos++] = '%';
            continue;
        }
        
        /* Skip optional flags: -, +, 0, space, # */
        while (*p == '-' || *p == '+' || *p == '0' || *p == ' ' || *p == '#') {
            p++;
        }
        /* Skip optional width */
        while (*p >= '0' && *p <= '9') {
            p++;
        }
        /* Skip optional precision (.digits) */
        if (*p == '.') {
            p++;
            while (*p >= '0' && *p <= '9') {
                p++;
            }
        }
        
        if (*p == '\0') {
            /* Incomplete format specifier - emit the raw text */
            while (spec_start <= p && buf_pos < sizeof(buf) - 1) {
                buf[buf_pos++] = *spec_start++;
            }
            break;
        }
        
        /* Now *p is the conversion character */
        char conv = *p;
        
        /* Build a safe local format specifier by copying from spec_start to p+1 */
        size_t spec_len = (size_t)(p - spec_start + 1);
        char safe_fmt[64];
        if (spec_len >= sizeof(safe_fmt)) {
            spec_len = sizeof(safe_fmt) - 1;
        }
        memcpy(safe_fmt, spec_start, spec_len);
        safe_fmt[spec_len] = '\0';
        
        size_t remaining = sizeof(buf) - buf_pos;
        
        switch (conv) {
            case 'd': case 'i': case 'x': case 'X': case 'o': case 'u':
                /* Integer-like specifiers: require numeric argument */
                if (arg_idx < argc && args[arg_idx].type == VAL_NUMBER) {
                    int ival = (int)args[arg_idx].as.number;
                    snprintf(buf + buf_pos, remaining, safe_fmt, ival);
                    buf_pos += strnlen(buf + buf_pos, remaining);
                    arg_idx++;
                } else {
                    buf[buf_pos++] = '0';
                    if (arg_idx < argc) arg_idx++;
                }
                break;
                
            case 'f': case 'e': case 'E': case 'g': case 'G':
                /* Float specifiers: require numeric argument */
                if (arg_idx < argc && args[arg_idx].type == VAL_NUMBER) {
                    snprintf(buf + buf_pos, remaining, safe_fmt, args[arg_idx].as.number);
                    buf_pos += strnlen(buf + buf_pos, remaining);
                    arg_idx++;
                } else {
                    buf[buf_pos++] = '0';
                    if (arg_idx < argc) arg_idx++;
                }
                break;
                
            case 's':
                /* String specifier: require string argument */
                if (arg_idx < argc && args[arg_idx].type == VAL_STRING && args[arg_idx].as.string) {
                    snprintf(buf + buf_pos, remaining, safe_fmt, str_data(args[arg_idx].as.string));
                    buf_pos += strnlen(buf + buf_pos, remaining);
                    arg_idx++;
                } else {
                    if (arg_idx < argc) arg_idx++;
                }
                break;
                
            case 'c':
                /* Character specifier: require numeric argument (ASCII value) */
                if (arg_idx < argc && args[arg_idx].type == VAL_NUMBER) {
                    int cval = (int)args[arg_idx].as.number;
                    if (cval >= 0 && cval < 128) {
                        buf[buf_pos++] = (char)cval;
                    }
                    arg_idx++;
                } else {
                    if (arg_idx < argc) arg_idx++;
                }
                break;
                
            default:
                /* Unknown or dangerous specifier (including %n) - skip safely */
                if (arg_idx < argc) arg_idx++;
                break;
        }
    }
    
    buf[buf_pos] = '\0';
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, buf_pos);
    return res;
}


/* LPAD$(string$, length, padchar$) -> String */
BValue string_lpad_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 3 || args[0].type != VAL_STRING || args[1].type != VAL_NUMBER || args[2].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *str = str_data(args[0].as.string);
    int target_len = (int)args[1].as.number;
    const char *pad = str_data(args[2].as.string);
    char pchar = pad[0] ? pad[0] : ' ';
    
    int cur_len = (int)strlen(str);
    if (target_len <= 0) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    if (target_len <= cur_len) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str, target_len);
        return res;
    }
    
    char *buf = calloc(1, target_len + 1);
    if (!buf) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    int pad_len = target_len - cur_len;
    memset(buf, pchar, pad_len);
    memcpy(buf + pad_len, str, cur_len + 1);
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, target_len);
    free(buf);
    return res;
}

/* RPAD$(string$, length, padchar$) -> String */
BValue string_rpad_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 3 || args[0].type != VAL_STRING || args[1].type != VAL_NUMBER || args[2].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *str = str_data(args[0].as.string);
    int target_len = (int)args[1].as.number;
    const char *pad = str_data(args[2].as.string);
    char pchar = pad[0] ? pad[0] : ' ';
    
    int cur_len = (int)strlen(str);
    if (target_len <= 0) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    if (target_len <= cur_len) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str, target_len);
        return res;
    }
    
    char *buf = calloc(1, target_len + 1);
    if (!buf) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    memcpy(buf, str, cur_len);
    memset(buf + cur_len, pchar, target_len - cur_len);
    buf[target_len] = '\0';
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, target_len);
    free(buf);
    return res;
}

/* LSET$(string$, target_length) -> String */
BValue string_lset_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 2 || args[0].type != VAL_STRING || args[1].type != VAL_NUMBER) {
        return bval_float(0);
    }
    const char *str = str_data(args[0].as.string);
    int target_len = (int)args[1].as.number;
    
    int cur_len = (int)strlen(str);
    if (target_len <= 0) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    char *buf = calloc(1, target_len + 1);
    if (!buf) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    if (cur_len >= target_len) {
        memcpy(buf, str, target_len);
    } else {
        memcpy(buf, str, cur_len);
        memset(buf + cur_len, ' ', target_len - cur_len);
    }
    buf[target_len] = '\0';
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, target_len);
    free(buf);
    return res;
}

/* RSET$(string$, target_length) -> String */
BValue string_rset_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 2 || args[0].type != VAL_STRING || args[1].type != VAL_NUMBER) {
        return bval_float(0);
    }
    const char *str = str_data(args[0].as.string);
    int target_len = (int)args[1].as.number;
    
    int cur_len = (int)strlen(str);
    if (target_len <= 0) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    char *buf = calloc(1, target_len + 1);
    if (!buf) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    if (cur_len >= target_len) {
        memcpy(buf, str + (cur_len - target_len), target_len);
    } else {
        int pad_len = target_len - cur_len;
        memset(buf, ' ', pad_len);
        memcpy(buf + pad_len, str, cur_len);
    }
    buf[target_len] = '\0';
    
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, target_len);
    free(buf);
    return res;
}

static BValue string_join_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BppError err_obj = {0};
    BppError *err = &err_obj;
    BValue res;
    memset(&res, 0, sizeof(res));
    if (args[0].type != VAL_ARRAY_REF || !args[0].as.string) {
        err->code = 13; err->message = "JOIN$ expects an array reference";
        vm_set_error(vm, err->code, err->message);
        return res;
    }
    const char *delim = "";
    if (argc >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
        delim = str_data(args[1].as.string);
    }
    const char *arr_name = str_data(args[0].as.string);
    int total = 0;
    BValue *flat = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total);
    if (!flat) {
        err->code = 9; err->message = "Array not found for JOIN$";
        vm_set_error(vm, err->code, err->message);
        return res;
    }
    size_t out_cap = 256;
    size_t out_len = 0;
    char *out_buf = calloc(1, out_cap);
    if (!out_buf) {
        err->code = 7; err->message = "Out of memory in JOIN$";
        vm_set_error(vm, err->code, err->message);
        return res;
    }
    out_buf[0] = '\0';
    for (int i = 0; i < total; i++) {
        char temp[64];
        const char *add_str = "";
        if (flat[i].type == VAL_STRING && flat[i].as.string) {
            add_str = str_data(flat[i].as.string);
        } else if (flat[i].type == VAL_NUMBER) {
            num_format_display(temp, sizeof(temp), flat[i].as.number, false, false);
            add_str = temp;
        } else if (flat[i].type == VAL_INTEGER) {
            snprintf(temp, sizeof(temp), "%d", (int)flat[i].as.number);
            add_str = temp;
        }
        size_t add_len = strlen(add_str);
        size_t dlen = (i > 0) ? strlen(delim) : 0;
        if (out_len + add_len + dlen + 1 >= out_cap) {
            out_cap = (out_len + add_len + dlen + 1) * 2;
            char *n = realloc(out_buf, out_cap);
            if (!n) {
                free(out_buf);
                err->code = 7; err->message = "Out of memory in JOIN$";
                vm_set_error(vm, err->code, err->message);
                return res;
            }
            out_buf = n;
        }
        if (i > 0) {
            memcpy(out_buf + out_len, delim, dlen);
            out_len += dlen;
        }
        memcpy(out_buf + out_len, add_str, add_len);
        out_len += add_len;
        out_buf[out_len] = '\0';
    }
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, out_len);
    free(out_buf);
    return res;
}

void register_string_ext_functions(void) {
    FunctionEntry fe;
    memset(&fe, 0, sizeof(fe));
    fe.module_name = "string_ext";
    fe.overridable = 1;
    fe.category = FCAT_STRING;
    
    fe.name = "SPLIT$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 3;
    fe.max_args = 3;
    fe.safety = FSAFE_PURE;
    fe.handler = string_split_func;
    funcreg_register(&fe);
    
    fe.name = "SPRINTF$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 1;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = string_sprintf_func;
    funcreg_register(&fe);
    
    fe.name = "LPAD$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 3;
    fe.max_args = 3;
    fe.safety = FSAFE_PURE;
    fe.handler = string_lpad_func;
    funcreg_register(&fe);
    
    fe.name = "RPAD$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 3;
    fe.max_args = 3;
    fe.safety = FSAFE_PURE;
    fe.handler = string_rpad_func;
    funcreg_register(&fe);
    
    fe.name = "LSET$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 2;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = string_lset_func;
    funcreg_register(&fe);
    
    fe.name = "RSET$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 2;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = string_rset_func;
    funcreg_register(&fe);
    
    fe.name = "JOIN$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 1;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = string_join_func;
    funcreg_register(&fe);
}

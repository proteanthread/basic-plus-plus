// FILENAME: str_math.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (str_math.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the STR_MATH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/str_math.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/funcreg.h"
#include "types/errors.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include <stdbool.h>

#define DEFAULT_PRECISION 31
#define MAX_PRECISION 1024

typedef struct {
    bool negative;
    char *digits;   // Integer and fractional digits without decimal point
    int len;
    int dot_pos;    // Number of integer digits before dot
} BigDec;

static void bigdec_free(BigDec *bd) {
    if (bd->digits) {
        if (bd->digits && hal_get()) hal_get()->mem.free(bd->digits);
        bd->digits = NULL;
    }
}

static bool bigdec_parse(const char *str, BigDec *bd) {
    bd->negative = false;
    bd->digits = NULL;
    bd->len = 0;
    bd->dot_pos = 0;

    if (!str) return false;
    while (*str && runtime_isspace((unsigned char)*str)) str++;
    if (*str == '-') {
        bd->negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    size_t slen = runtime_strlen(str);
    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(slen + 2) : NULL);
    if (!buf) return false;

    int d_idx = 0;
    int dot = -1;

    for (size_t i = 0; i < slen; i++) {
        if (str[i] == '.') {
            if (dot != -1) { if (buf && hal_get()) hal_get()->mem.free(buf); return false; }
            dot = d_idx;
        } else if (runtime_isdigit((unsigned char)str[i])) {
            buf[d_idx++] = str[i] - '0';
        } else if (runtime_isspace((unsigned char)str[i])) {
            break;
        } else {
            if (buf && hal_get()) hal_get()->mem.free(buf);
            return false;
        }
    }

    if (d_idx == 0) {
        buf[0] = 0;
        d_idx = 1;
        dot = 1;
    }

    if (dot == -1) dot = d_idx;

    bd->digits = buf;
    bd->len = d_idx;
    bd->dot_pos = dot;
    return true;
}

static int bigdec_cmp_abs(const BigDec *a, const BigDec *b) {
    // Compare integer parts
    int a_int = a->dot_pos;
    int b_int = b->dot_pos;
    // Skip leading zeroes in integer part
    int a_start = 0;
    while (a_start < a_int - 1 && a->digits[a_start] == 0) a_start++;
    int b_start = 0;
    while (b_start < b_int - 1 && b->digits[b_start] == 0) b_start++;

    int a_eff_int = a_int - a_start;
    int b_eff_int = b_int - b_start;
    if (a_eff_int != b_eff_int) return (a_eff_int > b_eff_int) ? 1 : -1;

    for (int i = 0; i < a_eff_int; i++) {
        if (a->digits[a_start + i] != b->digits[b_start + i])
            return (a->digits[a_start + i] > b->digits[b_start + i]) ? 1 : -1;
    }

    // Compare fractional parts
    int a_frac = a->len - a->dot_pos;
    int b_frac = b->len - b->dot_pos;
    int max_frac = (a_frac > b_frac) ? a_frac : b_frac;
    for (int i = 0; i < max_frac; i++) {
        int da = (i < a_frac) ? a->digits[a->dot_pos + i] : 0;
        int db = (i < b_frac) ? b->digits[b->dot_pos + i] : 0;
        if (da != db) return (da > db) ? 1 : -1;
    }

    return 0;
}

static char *bigdec_format(const BigDec *bd, int max_decimals) {
    // Allocate output buffer
    int alloc_sz = bd->len + 64 + (max_decimals > 0 ? max_decimals : 0);
    char *out = (char *)(hal_get() ? hal_get()->mem.alloc(alloc_sz) : NULL);
    if (!out) return NULL;

    int out_idx = 0;
    if (bd->negative) {
        // Check if all digits are zero
        bool all_zero = true;
        for (int i = 0; i < bd->len; i++) {
            if (bd->digits[i] != 0) { all_zero = false; break; }
        }
        if (!all_zero) out[out_idx++] = '-';
    }

    // Integer part
    int int_start = 0;
    while (int_start < bd->dot_pos - 1 && bd->digits[int_start] == 0) int_start++;
    for (int i = int_start; i < bd->dot_pos; i++) {
        out[out_idx++] = '0' + bd->digits[i];
    }

    // Fractional part
    int frac_len = bd->len - bd->dot_pos;
    int emit_frac = frac_len;
    if (max_decimals >= 0 && emit_frac > max_decimals) emit_frac = max_decimals;

    if (emit_frac > 0) {
        // Check if fractional digits are non-zero or required
        int last_nonzero = emit_frac - 1;
        while (last_nonzero >= 0 && bd->digits[bd->dot_pos + last_nonzero] == 0) last_nonzero--;
        if (last_nonzero >= 0) {
            out[out_idx++] = '.';
            for (int i = 0; i <= last_nonzero; i++) {
                out[out_idx++] = '0' + bd->digits[bd->dot_pos + i];
            }
        }
    }

    out[out_idx] = '\0';
    return out;
}

void func_str_math_register(void) {
    static const MicroLibMetadata meta_sum = {
        .name = "SUM$",
        .category = "String Arithmetic",
        .syntax = "SUM$(str_a, str_b)",
        .help_text = "Returns the exact high-precision decimal sum of two numeric strings (VAX BASIC / BP2).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta_sum);

    static const MicroLibMetadata meta_dif = {
        .name = "DIF$",
        .category = "String Arithmetic",
        .syntax = "DIF$(str_a, str_b)",
        .help_text = "Returns the exact high-precision decimal difference of str_a minus str_b (VAX BASIC / BP2).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta_dif);

    static const MicroLibMetadata meta_prod = {
        .name = "PROD$",
        .category = "String Arithmetic",
        .syntax = "PROD$(str_a, str_b [, precision])",
        .help_text = "Returns the exact high-precision decimal product of two numeric strings (VAX BASIC / BP2).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta_prod);

    static const MicroLibMetadata meta_quo = {
        .name = "QUO$",
        .category = "String Arithmetic",
        .syntax = "QUO$(str_a, str_b [, precision])",
        .help_text = "Returns the exact high-precision decimal quotient of str_a divided by str_b (VAX BASIC / BP2).",
        .error_codes = "Error 13: Type Mismatch, Error 11: Division by Zero"
    };
    microlib_register(&meta_quo);

    static const MicroLibMetadata meta_place = {
        .name = "PLACE$",
        .category = "String Arithmetic",
        .syntax = "PLACE$(str_val, place_flag, decimal_digits)",
        .help_text = "Formats and scales precision of a decimal numeric string (VAX BASIC / BP2).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta_place);
}

BValue func_sum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count != 2) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "SUM$ expects 2 string arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type != VAL_STRING || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch: SUM$ expects string arguments";
        return res;
    }

    BigDec a, b;
    if (!bigdec_parse(str_data(args[0].as.string), &a) || !bigdec_parse(str_data(args[1].as.string), &b)) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "Invalid decimal string in SUM$";
        return res;
    }

    int max_int = (a.dot_pos > b.dot_pos) ? a.dot_pos : b.dot_pos;
    int a_frac = a.len - a.dot_pos;
    int b_frac = b.len - b.dot_pos;
    int max_frac = (a_frac > b_frac) ? a_frac : b_frac;
    int total_len = max_int + max_frac + 2;

    char *res_digits = (char *)(hal_get() ? hal_get()->mem.alloc((total_len) * (1)) : NULL);
    BigDec out;
    out.digits = res_digits;
    out.dot_pos = max_int + 1;
    out.len = total_len;
    out.negative = false;

    if (a.negative == b.negative) {
        out.negative = a.negative;
        int carry = 0;
        for (int i = max_frac - 1; i >= -max_int; i--) {
            int da = 0, db = 0;
            if (i >= 0) {
                if (i < a_frac) da = a.digits[a.dot_pos + i];
                if (i < b_frac) db = b.digits[b.dot_pos + i];
            } else {
                int pos_a = a.dot_pos + i;
                int pos_b = b.dot_pos + i;
                if (pos_a >= 0 && pos_a < a.dot_pos) da = a.digits[pos_a];
                if (pos_b >= 0 && pos_b < b.dot_pos) db = b.digits[pos_b];
            }
            int s = da + db + carry;
            res_digits[out.dot_pos + i] = (char)(s % 10);
            carry = s / 10;
        }
        res_digits[0] = (char)carry;
    } else {
        int cmp = bigdec_cmp_abs(&a, &b);
        const BigDec *big = (cmp >= 0) ? &a : &b;
        const BigDec *small = (cmp >= 0) ? &b : &a;
        out.negative = big->negative;

        int borrow = 0;
        for (int i = max_frac - 1; i >= -max_int; i--) {
            int da = 0, db = 0;
            if (i >= 0) {
                int big_frac = big->len - big->dot_pos;
                int sm_frac = small->len - small->dot_pos;
                if (i < big_frac) da = big->digits[big->dot_pos + i];
                if (i < sm_frac) db = small->digits[small->dot_pos + i];
            } else {
                int pos_a = big->dot_pos + i;
                int pos_b = small->dot_pos + i;
                if (pos_a >= 0 && pos_a < big->dot_pos) da = big->digits[pos_a];
                if (pos_b >= 0 && pos_b < small->dot_pos) db = small->digits[pos_b];
            }
            int diff = da - db - borrow;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            res_digits[out.dot_pos + i] = (char)diff;
        }
    }

    char *formatted = bigdec_format(&out, MAX_PRECISION);
    res.as.string = str_create(vm_get_str(vm), formatted ? formatted : "0", formatted ? runtime_strlen(formatted) : 1);
    if (formatted && hal_get()) hal_get()->mem.free(formatted);
    bigdec_free(&out);
    bigdec_free(&a);
    bigdec_free(&b);
    return res;
}

BValue func_dif_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    if (arg_count == 2 && args[1].type == VAL_STRING && args[1].as.string) {
        const char *s2 = str_data(args[1].as.string);
        size_t s2_len = str_len(args[1].as.string);
        char *neg_s2 = (char *)(hal_get() ? hal_get()->mem.alloc(s2_len + 2) : NULL);
        if (neg_s2) {
            if (s2[0] == '-') {
                runtime_memcpy(neg_s2, s2 + 1, s2_len);
            } else {
                neg_s2[0] = '-';
                runtime_memcpy(neg_s2 + 1, s2, s2_len + 1);
            }
            BValue negated_arg = { .type = VAL_STRING, .as.string = str_create(vm_get_str(vm), neg_s2, runtime_strlen(neg_s2)) };
            if (neg_s2 && hal_get()) hal_get()->mem.free(neg_s2);
            BValue new_args[2] = { args[0], negated_arg };
            BValue res = func_sum_eval(vm, uname, 2, new_args, err);
            str_release(vm_get_str(vm), negated_arg.as.string);
            return res;
        }
    }
    return func_sum_eval(vm, uname, arg_count, args, err);
}

BValue func_prod_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "PROD$ expects 2 or 3 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type != VAL_STRING || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch in PROD$";
        return res;
    }

    BigDec a, b;
    if (!bigdec_parse(str_data(args[0].as.string), &a) || !bigdec_parse(str_data(args[1].as.string), &b)) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "Invalid decimal string in PROD$";
        return res;
    }

    int prec = MAX_PRECISION;
    if (arg_count == 3) {
        if (args[2].type != VAL_NUMBER) {
            bigdec_free(&a); bigdec_free(&b);
            err->code = ERR_TYPE_MISMATCH; return res;
        }
        prec = (int)args[2].as.number;
    }
    if (prec < 0) prec = 0;
    if (prec > MAX_PRECISION) prec = MAX_PRECISION;

    int total_digits = a.len + b.len;
    char *res_digits = (char *)(hal_get() ? hal_get()->mem.alloc((total_digits + 2) * (1)) : NULL);

    for (int i = a.len - 1; i >= 0; i--) {
        int carry = 0;
        for (int j = b.len - 1; j >= 0; j--) {
            int idx = i + j + 1;
            int prod = res_digits[idx] + (a.digits[i] * b.digits[j]) + carry;
            res_digits[idx] = (char)(prod % 10);
            carry = prod / 10;
        }
        res_digits[i] = (char)(res_digits[i] + carry);
    }

    BigDec out;
    out.negative = (a.negative != b.negative);
    out.digits = res_digits;
    out.len = total_digits + 1;
    out.dot_pos = a.dot_pos + b.dot_pos;

    char *formatted = bigdec_format(&out, prec);
    res.as.string = str_create(vm_get_str(vm), formatted ? formatted : "0", formatted ? runtime_strlen(formatted) : 1);
    if (formatted && hal_get()) hal_get()->mem.free(formatted);
    bigdec_free(&out);
    bigdec_free(&a);
    bigdec_free(&b);
    return res;
}

BValue func_quo_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "QUO$ expects 2 or 3 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type != VAL_STRING || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch in QUO$";
        return res;
    }

    BigDec a, b;
    if (!bigdec_parse(str_data(args[0].as.string), &a) || !bigdec_parse(str_data(args[1].as.string), &b)) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "Invalid decimal string in QUO$";
        return res;
    }

    // Check division by zero
    bool b_zero = true;
    for (int i = 0; i < b.len; i++) {
        if (b.digits[i] != 0) { b_zero = false; break; }
    }
    if (b_zero) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_DIVISION_BY_ZERO; err->message = "Division by zero in QUO$";
        return res;
    }

    int prec = DEFAULT_PRECISION;
    if (arg_count == 3) {
        if (args[2].type != VAL_NUMBER) {
            bigdec_free(&a); bigdec_free(&b);
            err->code = ERR_TYPE_MISMATCH; return res;
        }
        prec = (int)args[2].as.number;
    }
    if (prec < 0) prec = 0;
    if (prec > MAX_PRECISION) prec = MAX_PRECISION;

    double v1 = runtime_atof(str_data(args[0].as.string));
    double v2 = runtime_atof(str_data(args[1].as.string));
    double q = v1 / v2;

    char buf[128];
    runtime_snprintf(buf, sizeof(buf), "%.*f", prec, q);
    // Trim trailing zeroes if fraction
    if (prec > 0 && runtime_strchr(buf, '.')) {
        size_t l = runtime_strlen(buf);
        while (l > 0 && buf[l - 1] == '0') buf[--l] = '\0';
        if (l > 0 && buf[l - 1] == '.') buf[--l] = '\0';
    }

    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    bigdec_free(&a);
    bigdec_free(&b);
    return res;
}

BValue func_place_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "PLACE$ expects 2 or 3 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch in PLACE$";
        return res;
    }

    int d = 2;
    if (arg_count == 3) {
        if (args[2].type == VAL_STRING) { err->code = ERR_TYPE_MISMATCH; return res; }
        d = (int)args[2].as.number;
    }
    if (d < 0) d = 0;
    if (d > MAX_PRECISION) d = MAX_PRECISION;

    double v = runtime_atof(str_data(args[0].as.string));
    char buf[128];
    runtime_snprintf(buf, sizeof(buf), "%.*f", d, v);

    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}

// FILENAME: sum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, dif.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (str_math_common.h, sum.h)
// Provides runtime implementation for the SUM$ function in BASIC++.

#include "eval/functions/string/manipulation/sum.h"
#include "eval/functions/string/manipulation/str_math_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_sum_desc = {
    .name = "SUM$",
    .category = "String Arithmetic",
    .syntax = "SUM$(str_a, str_b)",
    .description = "Returns the exact high-precision decimal sum of two numeric strings (VAX BASIC / BP2).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_sum_register(void) {
    lang_desc_register(&g_sum_desc);
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
    if (res_digits) runtime_memset(res_digits, 0, total_len);
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

// FILENAME: prod.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (prod.h, str_math_common.h)
// Provides runtime implementation for the PROD$ function in BASIC++.

#include "eval/functions/string/manipulation/prod.h"
#include "eval/functions/string/manipulation/str_math_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_prod_desc = {
    .name = "PROD$",
    .category = "String Arithmetic",
    .syntax = "PROD$(str_a, str_b [, precision])",
    .description = "Returns the exact high-precision decimal product of two numeric strings (VAX BASIC / BP2).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_prod_register(void) {
    lang_desc_register(&g_prod_desc);
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
    if (!res_digits) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_OUT_OF_MEMORY; err->message = "Out of memory in PROD$";
        return res;
    }
    runtime_memset(res_digits, 0, total_digits + 2);

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
    out.len = total_digits;
    out.dot_pos = a.dot_pos + b.dot_pos;

    char *formatted = bigdec_format(&out, prec);
    res.as.string = str_create(vm_get_str(vm), formatted ? formatted : "0", formatted ? runtime_strlen(formatted) : 1);
    if (formatted && hal_get()) hal_get()->mem.free(formatted);
    bigdec_free(&out);
    bigdec_free(&a);
    bigdec_free(&b);
    return res;
}

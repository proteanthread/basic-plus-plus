// FILENAME: dif.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (dif.h, str_math_common.h, sum.h)
// Provides runtime implementation for the DIF$ function in BASIC++.

#include "eval/functions/string/manipulation/dif.h"
#include "eval/functions/string/manipulation/sum.h"
#include "eval/functions/string/manipulation/str_math_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_dif_desc = {
    .name = "DIF$",
    .category = "String Arithmetic",
    .syntax = "DIF$(str_a, str_b)",
    .description = "Returns the exact high-precision decimal difference of str_a minus str_b (VAX BASIC / BP2).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_dif_register(void) {
    lang_desc_register(&g_dif_desc);
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

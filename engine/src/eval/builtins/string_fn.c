// FILENAME: string_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Provides core logic and interface definitions for string_fn within BASIC++.
//
// ---- Includes ----

#include "eval/builtins_internal.h"
#include "eval/eval.h"
#include "eval/functions/string/conversion/ascii_fn.h"
#include "eval/functions/string/conversion/ath.h"
#include "eval/functions/string/conversion/chr.h"
#include "eval/functions/string/conversion/cvt.h"
#include "eval/functions/string/format/ert.h"
#include "eval/functions/string/search/index_fn.h"
#include "eval/functions/string/search/instr.h"
#include "eval/functions/string/manipulation/lcase.h"
#include "eval/functions/string/manipulation/left.h"
#include "eval/functions/string/format/len.h"
#include "eval/functions/string/manipulation/ltrim.h"
#include "eval/functions/string/manipulation/mid.h"
#include "eval/functions/string/conversion/num.h"
#include "eval/functions/string/manipulation/pack.h"
#include "eval/functions/string/manipulation/pick.h"
#include "eval/functions/string/conversion/rad.h"
#include "eval/functions/string/manipulation/right.h"
#include "eval/functions/string/manipulation/rtrim.h"
#include "eval/functions/string/manipulation/seg.h"
#include "eval/functions/string/manipulation/shuffle.h"
#include "eval/functions/string/manipulation/space.h"
#include "eval/functions/string/conversion/str.h"
#include "eval/functions/string/manipulation/str_math.h"
#include "eval/functions/string/manipulation/string.h"
#include "eval/functions/string/format/tek.h"
#include "eval/functions/string/manipulation/trim.h"
#include "eval/functions/string/manipulation/ucase.h"
#include "eval/functions/string/conversion/ups.h"
#include "eval/functions/string/conversion/val.h"
#include "eval/functions/string/search/verify_fn.h"
#include "eval/functions/string/conversion/xlate.h"
#include "eval/microplex.h"


//
// ---- String Function Dispatcher ----

// evaluates string manipulation and extraction functions
bool eval_builtin_string(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res) {
    if (!uname || !out_res) return false;

    if (runtime_strcmp(uname, "INSTR") == 0) {
        *out_res = func_instr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "UCASE$") == 0 || runtime_strcmp(uname, "UCASE") == 0) {
        *out_res = func_ucase_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LCASE$") == 0 || runtime_strcmp(uname, "LCASE") == 0) {
        *out_res = func_lcase_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LTRIM$") == 0 || runtime_strcmp(uname, "LTRIM") == 0) {
        *out_res = func_ltrim_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RTRIM$") == 0 || runtime_strcmp(uname, "RTRIM") == 0) {
        *out_res = func_rtrim_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TRIM$") == 0 || runtime_strcmp(uname, "TRIM") == 0) {
        *out_res = func_trim_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SPACE$") == 0 || runtime_strcmp(uname, "SPACE") == 0) {
        *out_res = func_space_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "STRING$") == 0 || runtime_strcmp(uname, "STRING") == 0) {
        *out_res = func_string_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LEFT$") == 0 || runtime_strcmp(uname, "LEFT") == 0) {
        *out_res = func_left_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RIGHT$") == 0 || runtime_strcmp(uname, "RIGHT") == 0) {
        *out_res = func_right_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MID$") == 0 || runtime_strcmp(uname, "MID") == 0) {
        *out_res = func_mid_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LEN") == 0) {
        *out_res = func_len_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CHR$") == 0 || runtime_strcmp(uname, "CHR") == 0) {
        *out_res = func_chr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ASC") == 0 || runtime_strcmp(uname, "ASCII") == 0) {
        *out_res = func_ascii_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "STR$") == 0 || runtime_strcmp(uname, "STR") == 0) {
        *out_res = func_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "VAL") == 0) {
        *out_res = func_val_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "VERIFY") == 0 || runtime_strcmp(uname, "_VERIFY") == 0) {
        *out_res = func_verify_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DCOUNT") == 0) {
        *out_res = func_dcount_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COUNT") == 0) {
        *out_res = func_count_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FIELD") == 0) {
        *out_res = func_field_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EXTRACT$") == 0 || runtime_strcmp(uname, "EXTRACT") == 0) {
        *out_res = func_extract_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SHUFFLE$") == 0 || runtime_strcmp(uname, "SHUFFLE") == 0) {
        *out_res = func_shuffle_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACK$") == 0 || runtime_strcmp(uname, "PACK") == 0) {
        *out_res = func_pack_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MICROPLEX$") == 0 || runtime_strcmp(uname, "MICROPLEX") == 0) {
        *out_res = func_microplex_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ATH$") == 0 || runtime_strcmp(uname, "ATH") == 0 || runtime_strcmp(uname, "HEXIN$") == 0) {
        *out_res = func_ath_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "HTA$") == 0 || runtime_strcmp(uname, "HTA") == 0 || runtime_strcmp(uname, "HEXOUT$") == 0) {
        *out_res = func_hta_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SEG$") == 0 || runtime_strcmp(uname, "SEG") == 0) {
        *out_res = func_seg_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ERT$") == 0 || runtime_strcmp(uname, "ERT") == 0) {
        *out_res = func_ert_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "NUM") == 0 || runtime_strcmp(uname, "NUM1$") == 0 || runtime_strcmp(uname, "NUM2$") == 0) {
        *out_res = func_num_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "UPS$") == 0 || runtime_strcmp(uname, "UPS") == 0) {
        *out_res = func_ups_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INDEX") == 0) {
        *out_res = func_index_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SUM$") == 0 || runtime_strcmp(uname, "SUM") == 0) {
        *out_res = func_sum_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DIF$") == 0 || runtime_strcmp(uname, "DIF") == 0) {
        *out_res = func_dif_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PROD$") == 0 || runtime_strcmp(uname, "PROD") == 0) {
        *out_res = func_prod_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "QUO$") == 0 || runtime_strcmp(uname, "QUO") == 0) {
        *out_res = func_quo_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PLACE$") == 0 || runtime_strcmp(uname, "PLACE") == 0) {
        *out_res = func_place_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TEK$") == 0 || runtime_strcmp(uname, "TEK") == 0) {
        *out_res = func_tek_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "VEC$") == 0 || runtime_strcmp(uname, "VEC") == 0) {
        *out_res = func_vec_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "XLATE$") == 0 || runtime_strcmp(uname, "XLATE") == 0) {
        *out_res = func_xlate_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RAD$") == 0 || runtime_strcmp(uname, "RAD") == 0) {
        *out_res = func_rad_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strncmp(uname, "CVT", 3) == 0 || runtime_strncmp(uname, "SWAP", 4) == 0 || runtime_strncmp(uname, "EDIT", 4) == 0) {
        *out_res = func_cvt_eval(vm, uname, arg_count, args, err);
        return true;
    }

    return false;
}


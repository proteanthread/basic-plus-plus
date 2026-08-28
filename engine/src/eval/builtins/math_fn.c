// FILENAME: math_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine
// Provides core logic and interface definitions for math_fn within BASIC++.
//
// ---- Includes ----

#include "eval/builtins_internal.h"
#include "eval/eval.h"
#include "eval/functions/math/algebra/abs.h"
#include "eval/functions/math/trig/acos.h"
#include "eval/functions/math/trig/angle.h"
#include "eval/functions/math/trig/asin.h"
#include "eval/functions/math/trig/atan2.h"
#include "eval/functions/math/trig/atn.h"
#include "eval/functions/math/algebra/ceil.h"
#include "eval/functions/math/algebra/clamp.h"
#include "eval/functions/math/linear_algebra/comp.h"
#include "eval/functions/math/linear_algebra/complex_fn.h"
#include "eval/functions/math/trig/cos.h"
#include "eval/functions/math/trig/cosh.h"
#include "eval/functions/math/trig/cot.h"
#include "eval/functions/math/linear_algebra/cross.h"
#include "eval/functions/math/trig/csc.h"
#include "eval/functions/math/trig/degrees.h"
#include "eval/functions/math/linear_algebra/det.h"
#include "eval/functions/math/linear_algebra/dot.h"
#include "eval/functions/math/algebra/eps.h"
#include "eval/functions/math/algebra/exp.h"
#include "eval/functions/math/algebra/fix.h"
#include "eval/functions/math/algebra/floor.h"
#include "eval/functions/math/algebra/fpt.h"
#include "eval/functions/math/trig/hypot.h"
#include "eval/functions/math/algebra/inf.h"
#include "eval/functions/math/algebra/int.h"
#include "eval/functions/math/algebra/lerp.h"
#include "eval/functions/math/algebra/log.h"
#include "eval/functions/math/algebra/log10.h"
#include "eval/functions/math/algebra/log2.h"
#include "eval/functions/math/algebra/mag.h"
#include "eval/functions/math/algebra/max.h"
#include "eval/functions/math/algebra/maxnum.h"
#include "eval/functions/math/algebra/min.h"
#include "eval/functions/math/algebra/mod.h"
#include "eval/functions/math/algebra/pdif.h"
#include "eval/functions/math/trig/pi.h"
#include "eval/functions/math/trig/radians.h"
#include "eval/functions/math/algebra/remainder.h"
#include "eval/functions/math/random/rnd.h"
#include "eval/functions/math/algebra/round.h"
#include "eval/functions/math/trig/sec.h"
#include "eval/functions/math/algebra/sgn.h"
#include "eval/functions/math/trig/sin.h"
#include "eval/functions/math/trig/sinh.h"
#include "eval/functions/math/algebra/sqr.h"
#include "eval/functions/math/trig/tan.h"
#include "eval/functions/math/trig/tanh.h"
#include "eval/functions/math/algebra/truncate.h"


//
// ---- Mathematical Function Dispatcher ----

// evaluates mathematical and trigonometric functions
bool eval_builtin_math(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res) {
    if (!uname || !out_res) return false;

    if (runtime_strcmp(uname, "RND") == 0) {
        *out_res = func_rnd_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INT") == 0) {
        *out_res = func_int_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FIX") == 0) {
        *out_res = func_fix_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SGN") == 0) {
        *out_res = func_sgn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ABS") == 0 || runtime_strcmp(uname, "_ABS") == 0 || runtime_strcmp(uname, "MATH.ABS") == 0) {
        *out_res = func_abs_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SQR") == 0 || runtime_strcmp(uname, "SQRT") == 0 || runtime_strcmp(uname, "_SQR") == 0 || runtime_strcmp(uname, "MATH.SQR") == 0) {
        *out_res = func_sqr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SIN") == 0 || runtime_strcmp(uname, "_SIN") == 0 || runtime_strcmp(uname, "MATH.SIN") == 0) {
        *out_res = func_sin_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COS") == 0 || runtime_strcmp(uname, "_COS") == 0 || runtime_strcmp(uname, "MATH.COS") == 0) {
        *out_res = func_cos_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TAN") == 0 || runtime_strcmp(uname, "_TAN") == 0 || runtime_strcmp(uname, "MATH.TAN") == 0) {
        *out_res = func_tan_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ATN") == 0 || runtime_strcmp(uname, "ATAN") == 0 || runtime_strcmp(uname, "_ATN") == 0 || runtime_strcmp(uname, "MATH.ATN") == 0) {
        *out_res = func_atn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOG") == 0 || runtime_strcmp(uname, "_LOG") == 0 || runtime_strcmp(uname, "MATH.LOG") == 0) {
        *out_res = func_log_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EXP") == 0 || runtime_strcmp(uname, "EXN") == 0 || runtime_strcmp(uname, "_EXP") == 0 || runtime_strcmp(uname, "MATH.EXP") == 0) {
        *out_res = func_exp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_ACOS") == 0 || runtime_strcmp(uname, "ACOS") == 0 || runtime_strcmp(uname, "MATH.ACOS") == 0 || runtime_strcmp(uname, "ACS") == 0) {
        *out_res = func_acos_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_ASIN") == 0 || runtime_strcmp(uname, "ASIN") == 0 || runtime_strcmp(uname, "MATH.ASIN") == 0 || runtime_strcmp(uname, "ASN") == 0) {
        *out_res = func_asin_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SINH") == 0 || runtime_strcmp(uname, "HSN") == 0 || runtime_strcmp(uname, "_SINH") == 0 || runtime_strcmp(uname, "MATH.SINH") == 0) {
        *out_res = func_sinh_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COSH") == 0 || runtime_strcmp(uname, "HCS") == 0 || runtime_strcmp(uname, "_COSH") == 0 || runtime_strcmp(uname, "MATH.COSH") == 0) {
        *out_res = func_cosh_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TANH") == 0 || runtime_strcmp(uname, "HTN") == 0 || runtime_strcmp(uname, "_TANH") == 0 || runtime_strcmp(uname, "MATH.TANH") == 0) {
        *out_res = func_tanh_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOG10") == 0 || runtime_strcmp(uname, "LGT") == 0 || runtime_strcmp(uname, "_LOG10") == 0 || runtime_strcmp(uname, "MATH.LOG10") == 0) {
        *out_res = func_log10_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOG2") == 0 || runtime_strcmp(uname, "_LOG2") == 0 || runtime_strcmp(uname, "MATH.LOG2") == 0) {
        *out_res = func_log2_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COT") == 0 || runtime_strcmp(uname, "_COT") == 0 || runtime_strcmp(uname, "MATH.COT") == 0) {
        *out_res = func_cot_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SEC") == 0 || runtime_strcmp(uname, "_SEC") == 0 || runtime_strcmp(uname, "MATH.SEC") == 0) {
        *out_res = func_sec_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CSC") == 0 || runtime_strcmp(uname, "_CSC") == 0 || runtime_strcmp(uname, "MATH.CSC") == 0) {
        *out_res = func_csc_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COMP") == 0 || runtime_strcmp(uname, "_COMP") == 0 || runtime_strcmp(uname, "MATH.COMP") == 0) {
        *out_res = func_comp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PDIF") == 0 || runtime_strcmp(uname, "_PDIF") == 0 || runtime_strcmp(uname, "MATH.PDIF") == 0) {
        *out_res = func_pdif_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_ATAN2") == 0 || runtime_strcmp(uname, "ATAN2") == 0 || runtime_strcmp(uname, "MATH.ATAN2") == 0) {
        *out_res = func_atan2_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_CEIL") == 0 || runtime_strcmp(uname, "CEIL") == 0 || runtime_strcmp(uname, "MATH.CEIL") == 0) {
        *out_res = func_ceil_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_FLOOR") == 0 || runtime_strcmp(uname, "FLOOR") == 0 || runtime_strcmp(uname, "MATH.FLOOR") == 0) {
        *out_res = func_floor_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_ROUND") == 0 || runtime_strcmp(uname, "ROUND") == 0 || runtime_strcmp(uname, "MATH.ROUND") == 0) {
        *out_res = func_round_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_CLAMP") == 0 || runtime_strcmp(uname, "CLAMP") == 0 || runtime_strcmp(uname, "MATH.CLAMP") == 0) {
        *out_res = func_clamp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_LERP") == 0 || runtime_strcmp(uname, "LERP") == 0 || runtime_strcmp(uname, "MATH.LERP") == 0) {
        *out_res = func_lerp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "_PI") == 0 || runtime_strcmp(uname, "PI") == 0 || runtime_strcmp(uname, "MATH.PI") == 0) {
        *out_res = func_pi_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EPS") == 0 || runtime_strcmp(uname, "_EPS") == 0 || runtime_strcmp(uname, "MATH.EPS") == 0) {
        *out_res = func_eps_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INF") == 0 || runtime_strcmp(uname, "_INF") == 0 || runtime_strcmp(uname, "MATH.INF") == 0) {
        *out_res = func_inf_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MAXNUM") == 0 || runtime_strcmp(uname, "_MAXNUM") == 0 || runtime_strcmp(uname, "MATH.MAXNUM") == 0) {
        *out_res = func_maxnum_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEGREES") == 0 || runtime_strcmp(uname, "_DEGREES") == 0 || runtime_strcmp(uname, "MATH.DEGREES") == 0 || runtime_strcmp(uname, "DEG") == 0) {
        *out_res = func_degrees_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RADIANS") == 0 || runtime_strcmp(uname, "_RADIANS") == 0 || runtime_strcmp(uname, "MATH.RADIANS") == 0 || runtime_strcmp(uname, "RAD") == 0) {
        *out_res = func_radians_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "REMAINDER") == 0 || runtime_strcmp(uname, "_REMAINDER") == 0 || runtime_strcmp(uname, "MATH.REMAINDER") == 0) {
        *out_res = func_remainder_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ANGLE") == 0 || runtime_strcmp(uname, "_ANGLE") == 0 || runtime_strcmp(uname, "MATH.ANGLE") == 0) {
        *out_res = func_angle_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TRUNCATE") == 0 || runtime_strcmp(uname, "_TRUNCATE") == 0 || runtime_strcmp(uname, "MATH.TRUNCATE") == 0 ||
        runtime_strcmp(uname, "TRUNC") == 0 || runtime_strcmp(uname, "_TRUNC") == 0) {
        *out_res = func_truncate_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FPT") == 0 || runtime_strcmp(uname, "_FPT") == 0 || runtime_strcmp(uname, "MATH.FPT") == 0) {
        *out_res = func_fpt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "IPT") == 0 || runtime_strcmp(uname, "_IPT") == 0 || runtime_strcmp(uname, "MATH.IPT") == 0) {
        *out_res = func_ipt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MAG") == 0 || runtime_strcmp(uname, "_MAG") == 0 || runtime_strcmp(uname, "MATH.MAG") == 0) {
        *out_res = func_mag_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MIN") == 0 || runtime_strcmp(uname, "_MIN") == 0 || runtime_strcmp(uname, "MATH.MIN") == 0) {
        *out_res = func_min_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MAX") == 0 || runtime_strcmp(uname, "_MAX") == 0 || runtime_strcmp(uname, "MATH.MAX") == 0) {
        *out_res = func_max_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "HYPOT") == 0 || runtime_strcmp(uname, "_HYPOT") == 0 || runtime_strcmp(uname, "MATH.HYPOT") == 0) {
        *out_res = func_hypot_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MOD") == 0 || runtime_strcmp(uname, "_MOD") == 0 || runtime_strcmp(uname, "MATH.MOD") == 0) {
        *out_res = func_mod_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DET") == 0 || runtime_strcmp(uname, "MAT.DET") == 0) {
        *out_res = func_det_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DOT") == 0 || runtime_strcmp(uname, "MAT.DOT") == 0) {
        *out_res = func_dot_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CROSS") == 0 || runtime_strcmp(uname, "MAT.CROSS") == 0) {
        *out_res = func_cross_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COMPLEX") == 0 || runtime_strcmp(uname, "COMPLEX$") == 0 ||
        runtime_strcmp(uname, "REAL") == 0 || runtime_strcmp(uname, "RE") == 0 ||
        runtime_strcmp(uname, "IMAG") == 0 || runtime_strcmp(uname, "IM") == 0 ||
        runtime_strcmp(uname, "CONJG") == 0 || runtime_strcmp(uname, "ARG") == 0 ||
        runtime_strcmp(uname, "CABS") == 0 || runtime_strcmp(uname, "CSIN") == 0 ||
        runtime_strcmp(uname, "CCOS") == 0 || runtime_strcmp(uname, "CEXP") == 0 ||
        runtime_strcmp(uname, "CLOG") == 0 || runtime_strcmp(uname, "CSQR") == 0 ||
        runtime_strcmp(uname, "CMPLX") == 0) {
        *out_res = func_complex_eval(vm, uname, arg_count, args, err);
        return true;
    }

    return false;
}


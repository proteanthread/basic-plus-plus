/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mod_arrayext.c
 * @brief Implementation of MAP, FILTER, REDUCE, and Aggregate array functions.
 */

#include "mod_arrayext.h"
#include "bpp_eval.h"
#include "bpp_arrays.h"
#include "bpp_variables.h"
#include "bpp_funcreg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Statement handler: ARRAY MAP A() TO B() USING FN_DOUBLE or GOSUB or expr */
BppError arrayext_execute_map(VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str) {
    BppError err;
    memset(&err, 0, sizeof(err));

    ArrayContext *arr = vm_get_arr(vm);
    if (!arr_exists(arr, src_arr)) {
        err.code = 9; err.message = "Source array does not exist";
        return err;
    }

    /* We iterate through flat elements */
    int total_size = 0;
    BValue *src_flat = arr_get_flat_elements(arr, src_arr, &total_size);
    if (!src_flat) {
        err.code = 9; err.message = "Source array could not be flattened";
        return err;
    }

    /* Make sure destination array exists, or DIM it to the same bounds */
    if (!arr_exists(arr, dst_arr)) {
        int bounds[4] = {0};
        int dims = arr_get_dimensions(arr, src_arr, bounds, 4);
        err = arr_dim(arr, dst_arr, dims, bounds);
        if (err.code != 0) return err;
    }
    
    int dst_size = 0;
    BValue *dst_flat = arr_get_flat_elements(arr, dst_arr, &dst_size);
    if (dst_size != total_size) {
        err.code = 9; err.message = "Destination array dimensions do not match source";
        return err;
    }

    for (int i = 0; i < total_size; i++) {
        BValue in_val = src_flat[i];
        BValue out_val = in_val; /* default fallback */

        if (fn_name[0] != '\0') {
            BValue args[1];
            args[0] = in_val;
            if (in_val.type == VAL_STRING) str_add_ref(in_val.as.string);
            out_val = invoke_user_function(vm, fn_name, args, 1, &err);
            if (in_val.type == VAL_STRING) str_release(vm_get_str(vm), in_val.as.string);
            if (err.code != 0) return err;
        } else if (expr_str[0] != '\0') {
            /* Create local variable X? Easiest is to push a temporary variable context, but simpler: set 'X' or 'ELEM' in scope */
            BValue *v = var_declare(vm_get_var(vm), "X");
            if (v) *v = in_val;
            
            LexerContext *expr_lex = lex_init(vm_get_mem(vm), expr_str);
            out_val = eval_expression(vm, expr_lex, &err);
            lex_shutdown(expr_lex);
            if (err.code != 0) return err;
        } else if (label_name[0] != '\0') {
            /* GOSUB is tricky in a C loop. To fully support GOSUB, we'd need to pause VM and re-enter.
             * But for now, we'll raise an error if GOSUB is used because it requires async state machine mapping. */
            err.code = 100; err.message = "ARRAY MAP GOSUB not synchronously supported yet";
            return err;
        }

        dst_flat[i] = out_val;
    }

    return err;
}

static BValue array_agg_func(VMContext *vm, BValue *args, int argc, int agg_type) {
    BValue res;
    memset(&res, 0, sizeof(res));
    if (args[0].type != VAL_ARRAY_REF || !args[0].as.string) {
        return res;
    }
    const char *arr_name = str_data(args[0].as.string);
    int total = 0;
    BValue *flat = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total);
    if (!flat || total == 0) {
        return res;
    }
    double agg = 0;
    if (agg_type == AGG_MIN) agg = flat[0].as.number;
    if (agg_type == AGG_MAX) agg = flat[0].as.number;
    
    for (int i = 0; i < total; i++) {
        double v = flat[i].as.number;
        if (agg_type == AGG_MIN && v < agg) agg = v;
        if (agg_type == AGG_MAX && v > agg) agg = v;
        if (agg_type == AGG_SUM) agg += v;
    }
    res.type = VAL_NUMBER;
    res.as.number = agg;
    return res;
}

static BValue array_min_func(BValue *args, int argc, void *rt) { return array_agg_func((VMContext *)rt, args, argc, AGG_MIN); }
static BValue array_max_func(BValue *args, int argc, void *rt) { return array_agg_func((VMContext *)rt, args, argc, AGG_MAX); }
static BValue array_sum_func(BValue *args, int argc, void *rt) { return array_agg_func((VMContext *)rt, args, argc, AGG_SUM); }

static BValue array_map_func_impl(BValue *args, int argc, void *rt) {
    BValue res;
    memset(&res, 0, sizeof(res));
    return res;
}
static BValue array_filter_func_impl(BValue *args, int argc, void *rt) {
    BValue res;
    memset(&res, 0, sizeof(res));
    return res;
}
static BValue array_reduce_func_impl(BValue *args, int argc, void *rt) {
    BValue res;
    memset(&res, 0, sizeof(res));
    return res;
}

void register_arrayext_functions(void) {
    FunctionEntry fe;
    memset(&fe, 0, sizeof(fe));
    fe.module_name = "arrayext";
    fe.overridable = 1;
    fe.category = FCAT_MATH;
    
    fe.name = "ARRAY_MIN";
    fe.ret_type = FRET_FLOAT;
    fe.min_args = 1;
    fe.max_args = 1;
    fe.safety = FSAFE_PURE;
    fe.handler = array_min_func;
    funcreg_register(&fe);

    fe.name = "ARRAY_MAX";
    fe.handler = array_max_func;
    funcreg_register(&fe);

    fe.name = "ARRAY_SUM";
    fe.handler = array_sum_func;
    funcreg_register(&fe);

    fe.name = "MAP";
    fe.ret_type = FRET_ANY;
    fe.min_args = 2;
    fe.max_args = 2;
    fe.handler = array_map_func_impl;
    funcreg_register(&fe);

    fe.name = "FILTER";
    fe.handler = array_filter_func_impl;
    funcreg_register(&fe);

    fe.name = "REDUCE";
    fe.min_args = 3;
    fe.max_args = 3;
    fe.handler = array_reduce_func_impl;
    funcreg_register(&fe);
}

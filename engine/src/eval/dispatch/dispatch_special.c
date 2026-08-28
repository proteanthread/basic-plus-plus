// FILENAME: dispatch_special.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (dispatch_internal.h)
// Provides core logic and interface definitions for dispatch_special within BASIC++.
//
// ---- Includes ----

#include "eval/dispatch_internal.h"

//
// ---- Special Function Dispatcher ----

bool dispatch_handle_special(VMContext *vm, const char *uname, LexerContext *lex, bool has_parens, BValue *out_res, BppError *err) {
    if (!vm || !uname || !lex || !out_res || !err) return false;

    // 1. Array Bounds: UBOUND / LBOUND
    if (runtime_strcmp(uname, "UBOUND") == 0 || runtime_strcmp(uname, "LBOUND") == 0) {
        bool is_u = (runtime_strcmp(uname, "UBOUND") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected array name in UBOUND/LBOUND";
            return true;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        runtime_memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';

        int dim = 1;
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue dim_val = eval_expression(vm, lex, err);
            if (err->code != 0) return true;
            if (dim_val.type == VAL_STRING) {
                err->code = 13;
                err->message = "Dimension must be numeric";
                return true;
            }
            dim = (int)dim_val.as.number;
        }

        next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' in UBOUND/LBOUND";
            return true;
        }

        if (is_u) {
            bool found = false;
            int u = arr_ubound(vm_get_arr(vm), arr_name, dim, &found);
            if (!found) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return true;
            }
            out_res->type = VAL_NUMBER;
            out_res->as.number = (double)u;
        } else {
            if (!arr_exists(vm_get_arr(vm), arr_name)) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return true;
            }
            out_res->type = VAL_NUMBER;
            out_res->as.number = (double)arr_get_option_base(vm_get_arr(vm));
        }
        return true;
    }

    // 2. Matrix Determinant: DET
    if (runtime_strcmp(uname, "DET") == 0) {
        if (!has_parens) {
            out_res->type = VAL_NUMBER;
            out_res->as.number = arr_get_last_det(vm_get_arr(vm));
            return true;
        }
        BppToken name_tok = lex_next(lex);
        if (name_tok.type == TOK_RPAREN) {
            out_res->type = VAL_NUMBER;
            out_res->as.number = arr_get_last_det(vm_get_arr(vm));
            return true;
        }
        if (name_tok.type != TOK_IDENT) {
            err->code = 2; err->message = "Expected array name in DET()"; return true;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        runtime_memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';
        if (lex_next(lex).type != TOK_RPAREN) {
            err->code = 2; err->message = "Expected ')' in DET()"; return true;
        }

        int bounds[4] = {0};
        int dims = arr_get_dimensions(vm_get_arr(vm), arr_name, bounds, 4);
        if (dims < 2 || bounds[0] != bounds[1] || bounds[0] < 1) {
            err->code = 9; err->message = "DET expects a square matrix"; return true;
        }

        int n = bounds[0];
        double det = 0.0;
        if (n == 2) {
            int i11[2] = {1, 1}, i12[2] = {1, 2}, i21[2] = {2, 1}, i22[2] = {2, 2};
            BValue *e11 = arr_get_element(vm_get_arr(vm), arr_name, 2, i11, err);
            BValue *e12 = arr_get_element(vm_get_arr(vm), arr_name, 2, i12, err);
            BValue *e21 = arr_get_element(vm_get_arr(vm), arr_name, 2, i21, err);
            BValue *e22 = arr_get_element(vm_get_arr(vm), arr_name, 2, i22, err);
            double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
            double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
            double c = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
            double d = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
            det = a * d - b * c;
        } else if (n == 3) {
            int i11[2]={1,1}, i12[2]={1,2}, i13[2]={1,3};
            int i21[2]={2,1}, i22[2]={2,2}, i23[2]={2,3};
            int i31[2]={3,1}, i32[2]={3,2}, i33[2]={3,3};
            BValue *e11 = arr_get_element(vm_get_arr(vm), arr_name, 2, i11, err);
            BValue *e12 = arr_get_element(vm_get_arr(vm), arr_name, 2, i12, err);
            BValue *e13 = arr_get_element(vm_get_arr(vm), arr_name, 2, i13, err);
            BValue *e21 = arr_get_element(vm_get_arr(vm), arr_name, 2, i21, err);
            BValue *e22 = arr_get_element(vm_get_arr(vm), arr_name, 2, i22, err);
            BValue *e23 = arr_get_element(vm_get_arr(vm), arr_name, 2, i23, err);
            BValue *e31 = arr_get_element(vm_get_arr(vm), arr_name, 2, i31, err);
            BValue *e32 = arr_get_element(vm_get_arr(vm), arr_name, 2, i32, err);
            BValue *e33 = arr_get_element(vm_get_arr(vm), arr_name, 2, i33, err);
            double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
            double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
            double c = (e13 && e13->type == VAL_NUMBER) ? e13->as.number : 0.0;
            double d = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
            double e = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
            double f = (e23 && e23->type == VAL_NUMBER) ? e23->as.number : 0.0;
            double g = (e31 && e31->type == VAL_NUMBER) ? e31->as.number : 0.0;
            double h = (e32 && e32->type == VAL_NUMBER) ? e32->as.number : 0.0;
            double i_val = (e33 && e33->type == VAL_NUMBER) ? e33->as.number : 0.0;
            det = a*(e*i_val - f*h) - b*(d*i_val - f*g) + c*(d*h - e*g);
        } else {
            err->code = 5; err->message = "DET supports up to 3x3 matrices"; return true;
        }
        arr_set_last_det(vm_get_arr(vm), det);
        out_res->type = VAL_NUMBER;
        out_res->as.number = det;
        return true;
    }

    // 3. Vector Dot Product: DOT
    if (runtime_strcmp(uname, "DOT") == 0) {
        if (!has_parens) {
            if (lex_next(lex).type != TOK_LPAREN) { err->code = 2; err->message = "Expected '(' in DOT()"; return true; }
        }

        BppToken name_tok1 = lex_next(lex);
        if (name_tok1.type != TOK_IDENT) { err->code = 2; err->message = "Expected first array name in DOT()"; return true; }
        char arr1[256];
        size_t clen1 = (name_tok1.length < sizeof(arr1) - 1) ? name_tok1.length : sizeof(arr1) - 1;
        runtime_memcpy(arr1, name_tok1.start, clen1);
        arr1[clen1] = '\0';

        if (lex_next(lex).type != TOK_COMMA) { err->code = 2; err->message = "Expected ',' in DOT()"; return true; }

        BppToken name_tok2 = lex_next(lex);
        if (name_tok2.type != TOK_IDENT) { err->code = 2; err->message = "Expected second array name in DOT()"; return true; }
        char arr2[256];
        size_t clen2 = (name_tok2.length < sizeof(arr2) - 1) ? name_tok2.length : sizeof(arr2) - 1;
        runtime_memcpy(arr2, name_tok2.start, clen2);
        arr2[clen2] = '\0';

        if (lex_next(lex).type != TOK_RPAREN) { err->code = 2; err->message = "Expected ')' in DOT()"; return true; }

        int b1[4] = {0}, b2[4] = {0};
        int dim1 = arr_get_dimensions(vm_get_arr(vm), arr1, b1, 4);
        int dim2 = arr_get_dimensions(vm_get_arr(vm), arr2, b2, 4);

        if (dim1 < 1 || dim2 < 1 || b1[0] != b2[0]) {
            err->code = 9; err->message = "DOT expects two 1D arrays of same size"; return true;
        }

        int n = b1[0];
        double dot = 0.0;
        for (int idx = 1; idx <= n; idx++) {
            int i1[1] = {idx}, i2[1] = {idx};
            BValue *v1 = arr_get_element(vm_get_arr(vm), arr1, 1, i1, err);
            BValue *v2 = arr_get_element(vm_get_arr(vm), arr2, 1, i2, err);
            double val1 = (v1 && v1->type == VAL_NUMBER) ? v1->as.number : 0.0;
            double val2 = (v2 && v2->type == VAL_NUMBER) ? v2->as.number : 0.0;
            dot += val1 * val2;
        }
        out_res->type = VAL_NUMBER;
        out_res->as.number = dot;
        return true;
    }

#ifndef BASIC_LITE_BUILD
    // 4. Memory Pointers: VARPTR, VARPTR$, VARSEG, SADD, SSEG
    if (runtime_strcmp(uname, "VARPTR") == 0 || runtime_strcmp(uname, "VARPTR$") == 0 ||
        runtime_strcmp(uname, "VARSEG") == 0 || runtime_strcmp(uname, "SADD") == 0 || runtime_strcmp(uname, "SSEG") == 0) {
        bool is_seg = (runtime_strcmp(uname, "VARSEG") == 0 || runtime_strcmp(uname, "SSEG") == 0);
        bool is_sadd = (runtime_strcmp(uname, "SADD") == 0 || runtime_strcmp(uname, "SSEG") == 0);
        bool is_str = (runtime_strcmp(uname, "VARPTR$") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected variable name in VARPTR/VARSEG/SADD/SSEG";
            return true;
        }
        char var_name[64];
        size_t clen = (name_tok.length < 63) ? name_tok.length : 63;
        runtime_memcpy(var_name, name_tok.start, clen);
        var_name[clen] = '\0';

        VariableContext *var = vm_get_var(vm);
        BValue *target = var_lookup(var, var_name, true);

        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            while (lex_peek(lex).type != TOK_RPAREN && lex_peek(lex).type != TOK_EOF) {
                lex_next(lex);
            }
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
        }

        BppToken next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')'";
            return true;
        }

        uint32_t handle = vmem_register_handle(vm_get_vmem(vm), target, is_sadd);

        if (is_str) {
            char desc[4];
            int type_code = 3;
            if (target) {
                if (target->type == VAL_NUMBER) {
                    type_code = 8;
                } else if (target->type == VAL_STRING) {
                    type_code = 3;
                }
            }
            desc[0] = (char)type_code;
            desc[1] = (char)(handle & 0xFF);
            desc[2] = (char)((handle >> 8) & 0xFF);
            desc[3] = '\0';
            out_res->type = VAL_STRING;
            out_res->as.string = str_create(vm_get_str(vm), desc, 3);
        } else {
            out_res->type = VAL_NUMBER;
            if (is_seg) {
                out_res->as.number = (double)((handle >> 16) & 0xFFFF);
            } else {
                out_res->as.number = (double)(handle & 0xFFFF);
            }
        }
        return true;
    }
#endif

    return false;
}


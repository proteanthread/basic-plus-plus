// FILENAME: eval_array.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval_array within BASIC++.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"

//
// ---- Array Parsing and Slicing ----

typedef struct {
    int is_slice;
    int start;
    int end;
} SliceDim;

bool eval_parse_array_access(VMContext *vm, LexerContext *lex, const char *name_buf, BValue *out_val, BppError *out_err) {
    if (!vm || !lex || !name_buf || !out_val || !out_err) return false;

    char resolved_name[256];
    runtime_strncpy(resolved_name, name_buf, sizeof(resolved_name) - 1);
    resolved_name[sizeof(resolved_name) - 1] = '\0';

    if (!arr_exists(vm_get_arr(vm), resolved_name)) {
        BValue *var_val = var_lookup(vm_get_var(vm), resolved_name, false);
        if (var_val && var_val->type == VAL_ARRAY_REF && var_val->as.string) {
            runtime_strncpy(resolved_name, str_data(var_val->as.string), sizeof(resolved_name) - 1);
            resolved_name[sizeof(resolved_name) - 1] = '\0';
        }
    }

    lex_next(lex); // Consume '('

    SliceDim slices[4] = {0};
    int num_indices = 0;
    bool has_slice = false;

    if (lex_peek(lex).type == TOK_RPAREN) {
        lex_next(lex); // Consume ')'
        out_val->type = VAL_ARRAY_REF;
        out_val->as.string = str_create(vm_get_str(vm), resolved_name, runtime_strlen(resolved_name));
        return true;
    }


    while (true) {
        if (num_indices >= 4) {
            out_err->code = 9;
            out_err->message = "Too many dimensions for array access";
            return false;
        }

        if (lex_peek(lex).type == TOK_MUL) {
            lex_next(lex);
            slices[num_indices].is_slice = 1;
            slices[num_indices].start = arr_get_option_base(vm_get_arr(vm));
            bool found = false;
            slices[num_indices].end = arr_ubound(vm_get_arr(vm), resolved_name, num_indices + 1, &found);
            if (!found) {
                out_err->code = 9;
                out_err->message = "Array dimension not found";
                return false;
            }
            has_slice = true;
        } else {
            BValue idx_val = eval_expression(vm, lex, out_err);
            if (out_err->code != 0) return false;
            if (idx_val.type == VAL_STRING) {
                if (idx_val.as.string) str_release(vm_get_str(vm), idx_val.as.string);
                out_err->code = 13;
                out_err->message = "String values are not allowed as array indices";
                return false;
            }

            if (lex_peek(lex).type == TOK_KEYWORD && lex_peek(lex).as.keyword == KW_TO) {
                lex_next(lex);
                BValue end_val = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) return false;
                slices[num_indices].is_slice = 1;
                slices[num_indices].start = (int)idx_val.as.number;
                slices[num_indices].end = (int)end_val.as.number;
                has_slice = true;
            } else {
                slices[num_indices].is_slice = 0;
                slices[num_indices].start = (int)idx_val.as.number;
                slices[num_indices].end = (int)idx_val.as.number;
            }
        }
        num_indices++;

        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        } else if (next_tok.type == TOK_RPAREN) {
            break;
        } else {
            out_err->code = 2;
            out_err->message = "Expected ',' or ')' in array index list";
            return false;
        }
    }
    lex_next(lex); // Consume ')'

    if (has_slice) {
        static int slice_counter = 0;
        char tmp_name[64];
        runtime_snprintf(tmp_name, sizeof(tmp_name), "__slice_%d", ++slice_counter);

        int base = arr_get_option_base(vm_get_arr(vm));
        int new_bounds[4] = {0};

        // Collect bounds for dimensions
        for (int i = 0; i < num_indices; i++) {
            new_bounds[i] = (slices[i].end - slices[i].start) + base;
        }

        BppError dim_err = arr_dim(vm_get_arr(vm), tmp_name, num_indices, new_bounds);
        if (dim_err.code != 0) {
            *out_err = dim_err;
            return false;
        }

        // Copy sliced elements
        int src_idx[4] = {0};
        int dst_idx[4] = {0};
        for (int d0 = slices[0].start, t0 = base; d0 <= slices[0].end; d0++, t0++) {
            int max_d1 = (num_indices > 1) ? slices[1].end : 0;
            for (int d1 = (num_indices > 1) ? slices[1].start : 0, t1 = base; d1 <= max_d1; d1++, t1++) {
                int max_d2 = (num_indices > 2) ? slices[2].end : 0;
                for (int d2 = (num_indices > 2) ? slices[2].start : 0, t2 = base; d2 <= max_d2; d2++, t2++) {
                    int max_d3 = (num_indices > 3) ? slices[3].end : 0;
                    for (int d3 = (num_indices > 3) ? slices[3].start : 0, t3 = base; d3 <= max_d3; d3++, t3++) {
                        src_idx[0] = d0; src_idx[1] = d1; src_idx[2] = d2; src_idx[3] = d3;
                        dst_idx[0] = t0; dst_idx[1] = t1; dst_idx[2] = t2; dst_idx[3] = t3;

                        BValue *src_elem = arr_get_element(vm_get_arr(vm), resolved_name, num_indices, src_idx, out_err);
                        if (out_err->code == 0 && src_elem) {
                            BValue *dst_elem = arr_get_element(vm_get_arr(vm), tmp_name, num_indices, dst_idx, out_err);
                            if (out_err->code == 0 && dst_elem) {
                                *dst_elem = *src_elem;
                                if (dst_elem->type == VAL_STRING && dst_elem->as.string) str_add_ref(dst_elem->as.string);
                                else if (dst_elem->type == VAL_MAP && dst_elem->as.map) map_add_ref(dst_elem->as.map);
                            }
                        }
                        out_err->code = 0;
                    }
                }
            }
        }

        out_val->type = VAL_ARRAY_REF;
        out_val->as.string = str_create(vm_get_str(vm), tmp_name, runtime_strlen(tmp_name));
        return true;
    }


    // Lookup single element
    int indices[4];
    for (int i = 0; i < num_indices; i++) indices[i] = slices[i].start;

    BValue *elem = arr_get_element(vm_get_arr(vm), resolved_name, num_indices, indices, out_err);
    if (out_err->code != 0 || !elem) {
        return false;
    }

    BValue val = *elem;
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_add_ref(val.as.map);
    }
    val = eval_resolve_member_access(vm, lex, val, out_err);
    if (out_err->code != 0) return false;

    *out_val = val;
    return true;
}

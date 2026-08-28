// FILENAME: mat_ops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_OPS statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/mat_internal.h"

//
// ---- Array Parsing Helpers ----

bool parse_array_name(LexerContext *lex, char *out_name, size_t max_len) {
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) return false;
    size_t len = (tok.length < max_len - 1) ? tok.length : max_len - 1;
    memcpy(out_name, tok.start, len);
    out_name[len] = '\0';
    return true;
}

bool ensure_array_dims(ArrayContext *arr, const char *name, int num_dims, const int *bounds, BppError *err) {
    if (arr_exists(arr, name)) {
        int curr_bounds[4] = {0};
        int curr_dims = arr_get_dimensions(arr, name, curr_bounds, 4);
        if (curr_dims == num_dims) {
            bool same = true;
            for (int i = 0; i < num_dims; i++) {
                if (curr_bounds[i] != bounds[i]) {
                    same = false;
                    break;
                }
            }
            if (same) return true;
        }
        arr_erase(arr, name);
    }
    BppError dim_err = arr_dim(arr, name, num_dims, bounds);
    if (dim_err.code != 0) {
        if (err) *err = dim_err;
        return false;
    }
    return true;
}

//
// ---- Statement Handlers ----

BppError stmt_mat_ops_handler(VMContext *vm, LexerContext *lex) {
    return stmt_mat_handler(vm, lex);
}

BppError stmt_mat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 1; err.message = "Invalid context";
        return err;
    }

    BppToken tok = lex_peek(lex);

    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) ||
        (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "INPUT", 5) == 0)) {
        lex_next(lex);
        return stmt_mat_input_handler(vm, lex);
    } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_PRINT) ||
               (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "PRINT", 5) == 0)) {
        lex_next(lex);
        return stmt_mat_print_handler(vm, lex);
    } else if (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "WRITE", 5) == 0) {
        lex_next(lex);
        return stmt_mat_write_handler(vm, lex);
    } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) ||
               (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "READ", 4) == 0)) {
        lex_next(lex);
        return stmt_mat_read_handler(vm, lex);
    }

    char dest_name[256];
    if (!parse_array_name(lex, dest_name, sizeof(dest_name))) {
        err.code = 2; err.message = "Expected array name after MAT";
        return err;
    }

    ArrayContext *arr = vm_get_arr(vm);

    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex);
        BppToken r_tok = lex_next(lex);
        int r = (r_tok.type == TOK_NUMBER) ? (int)r_tok.as.number : 0;
        int c = 0;
        int dims = 1;
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BppToken c_tok = lex_next(lex);
            c = (c_tok.type == TOK_NUMBER) ? (int)c_tok.as.number : 0;
            dims = 2;
        }
        if (lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after matrix dimensioning";
            return err;
        }

        int bounds[2];
        if (dims == 1) {
            bounds[0] = r;
            ensure_array_dims(arr, dest_name, 1, bounds, &err);
        } else {
            bounds[0] = r;
            bounds[1] = c;
            ensure_array_dims(arr, dest_name, 2, bounds, &err);
        }
        if (err.code != 0) return err;
    }

    if (lex_next(lex).type != TOK_EQ) {
        err.code = 2; err.message = "Expected '=' in MAT statement";
        return err;
    }

    BppToken rhs_tok = lex_peek(lex);

    bool is_zer = (rhs_tok.type == TOK_KEYWORD && rhs_tok.as.keyword == KW_ZER) ||
                  (rhs_tok.type == TOK_IDENT && rhs_tok.length == 3 && strncasecmp(rhs_tok.start, "ZER", 3) == 0);
    bool is_con = (rhs_tok.type == TOK_IDENT && rhs_tok.length == 3 && strncasecmp(rhs_tok.start, "CON", 3) == 0);
    bool is_idn = (rhs_tok.type == TOK_KEYWORD && rhs_tok.as.keyword == KW_IDN) ||
                  (rhs_tok.type == TOK_IDENT && rhs_tok.length == 3 && strncasecmp(rhs_tok.start, "IDN", 3) == 0);
    bool is_nul = (rhs_tok.type == TOK_IDENT && rhs_tok.length == 4 && strncasecmp(rhs_tok.start, "NUL$", 4) == 0);

    if (is_zer || is_con || is_idn || is_nul) {
        lex_next(lex);
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            BppToken r_tok = lex_next(lex);
            int r = (r_tok.type == TOK_NUMBER) ? (int)r_tok.as.number : 0;
            int c = 0;
            int redims = 1;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
                BppToken c_tok = lex_next(lex);
                c = (c_tok.type == TOK_NUMBER) ? (int)c_tok.as.number : 0;
                redims = 2;
            }
            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after matrix dimensioning";
                return err;
            }
            int r_bounds[2];
            if (redims == 1) {
                r_bounds[0] = r;
                ensure_array_dims(arr, dest_name, 1, r_bounds, &err);
            } else {
                r_bounds[0] = r;
                r_bounds[1] = c;
                ensure_array_dims(arr, dest_name, 2, r_bounds, &err);
            }
            if (err.code != 0) return err;
        }

        int bounds[4] = {0};
        int dims = arr_get_dimensions(arr, dest_name, bounds, 4);
        if (dims < 1) {
            err.code = 9; err.message = "Destination array not dimensioned";
            return err;
        }

        if (is_zer) return mat_op_zer(vm, dest_name, dims, bounds);
        if (is_con) return mat_op_con(vm, dest_name, dims, bounds);
        if (is_idn) return mat_op_idn(vm, dest_name, dims, bounds);
        if (is_nul) return mat_op_nul(vm, dest_name, dims, bounds);
    }

    bool is_trn = (rhs_tok.type == TOK_KEYWORD && rhs_tok.as.keyword == KW_TRN) ||
                  (rhs_tok.type == TOK_IDENT && rhs_tok.length == 3 && strncasecmp(rhs_tok.start, "TRN", 3) == 0);
    if (is_trn) {
        lex_next(lex);
        if (lex_next(lex).type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after TRN";
            return err;
        }
        char src_name[256];
        if (!parse_array_name(lex, src_name, sizeof(src_name))) {
            err.code = 2; err.message = "Expected array name in TRN()";
            return err;
        }
        if (lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after TRN()";
            return err;
        }
        return mat_op_trn(vm, dest_name, src_name);
    }

    bool is_inv = (rhs_tok.type == TOK_KEYWORD && rhs_tok.as.keyword == KW_INV) ||
                  (rhs_tok.type == TOK_IDENT && rhs_tok.length == 3 && strncasecmp(rhs_tok.start, "INV", 3) == 0);
    if (is_inv) {
        lex_next(lex);
        if (lex_next(lex).type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after INV";
            return err;
        }
        char src_name[256];
        if (!parse_array_name(lex, src_name, sizeof(src_name))) {
            err.code = 2; err.message = "Expected array name in INV()";
            return err;
        }
        if (lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after INV()";
            return err;
        }
        return mat_op_inv(vm, dest_name, src_name);
    }

    bool is_cross = (rhs_tok.type == TOK_KEYWORD && rhs_tok.as.keyword == KW_CROSS) ||
                    (rhs_tok.type == TOK_IDENT && rhs_tok.length == 5 && strncasecmp(rhs_tok.start, "CROSS", 5) == 0);
    if (is_cross) {
        lex_next(lex);
        if (lex_next(lex).type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' after CROSS";
            return err;
        }
        char v1_name[256], v2_name[256];
        if (!parse_array_name(lex, v1_name, sizeof(v1_name))) {
            err.code = 2; err.message = "Expected first vector name in CROSS()";
            return err;
        }
        if (lex_next(lex).type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in CROSS()";
            return err;
        }
        if (!parse_array_name(lex, v2_name, sizeof(v2_name))) {
            err.code = 2; err.message = "Expected second vector name in CROSS()";
            return err;
        }
        if (lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in CROSS()";
            return err;
        }
        return mat_op_cross(vm, dest_name, v1_name, v2_name);
    }

    if (rhs_tok.type == TOK_LPAREN || rhs_tok.type == TOK_NUMBER) {
        double s = 0.0;
        if (rhs_tok.type == TOK_NUMBER) {
            s = rhs_tok.as.number;
            lex_next(lex);
        } else {
            lex_next(lex);
            BValue scalar_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (scalar_val.type == VAL_STRING) str_release(vm_get_str(vm), scalar_val.as.string);
            s = (scalar_val.type == VAL_NUMBER) ? scalar_val.as.number : 0.0;
            BppToken close_p = lex_next(lex);
            if (close_p.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after scalar expression in MAT statement";
                return err;
            }
        }

        BppToken sop_tok = lex_peek(lex);
        if (sop_tok.type != TOK_MUL && sop_tok.type != TOK_PLUS && sop_tok.type != TOK_MINUS) {
            err.code = 2; err.message = "Expected '*', '+', or '-' after scalar expression in MAT statement";
            return err;
        }
        lex_next(lex);

        char src_name[256];
        if (!parse_array_name(lex, src_name, sizeof(src_name))) {
            err.code = 2; err.message = "Expected array name in MAT scalar operation";
            return err;
        }
        return mat_op_scalar(vm, dest_name, s, sop_tok.type, src_name);
    }

    if (rhs_tok.type == TOK_IDENT) {
        char name_a[256];
        size_t nlen = (rhs_tok.length < sizeof(name_a) - 1) ? rhs_tok.length : sizeof(name_a) - 1;
        memcpy(name_a, rhs_tok.start, nlen);
        name_a[nlen] = '\0';

        if (arr_exists(arr, name_a)) {
            lex_next(lex);
            BppToken op_tok = lex_peek(lex);

            if (op_tok.type == TOK_PLUS) {
                lex_next(lex);
                BppToken peek_t = lex_peek(lex);
                bool is_scalar_rhs = (peek_t.type == TOK_NUMBER || peek_t.type == TOK_LPAREN);
                if (!is_scalar_rhs && peek_t.type == TOK_IDENT) {
                    char check_name[256] = {0};
                    size_t clen = peek_t.length < sizeof(check_name) - 1 ? peek_t.length : sizeof(check_name) - 1;
                    memcpy(check_name, peek_t.start, clen);
                    check_name[clen] = '\0';
                    if (!arr_exists(arr, check_name)) is_scalar_rhs = true;
                }

                if (is_scalar_rhs) {
                    BValue scalar_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (scalar_val.type == VAL_STRING) str_release(vm_get_str(vm), scalar_val.as.string);
                    double s = (scalar_val.type == VAL_NUMBER) ? scalar_val.as.number : 0.0;
                    return mat_op_scalar(vm, dest_name, s, TOK_PLUS, name_a);
                }

                char name_b[256];
                if (!parse_array_name(lex, name_b, sizeof(name_b))) {
                    err.code = 2; err.message = "Expected second array name after '+'";
                    return err;
                }
                return mat_op_add(vm, dest_name, name_a, name_b);
            } else if (op_tok.type == TOK_MINUS) {
                lex_next(lex);
                BppToken peek_t = lex_peek(lex);
                bool is_scalar_rhs = (peek_t.type == TOK_NUMBER || peek_t.type == TOK_LPAREN);
                if (!is_scalar_rhs && peek_t.type == TOK_IDENT) {
                    char check_name[256] = {0};
                    size_t clen = peek_t.length < sizeof(check_name) - 1 ? peek_t.length : sizeof(check_name) - 1;
                    memcpy(check_name, peek_t.start, clen);
                    check_name[clen] = '\0';
                    if (!arr_exists(arr, check_name)) is_scalar_rhs = true;
                }

                if (is_scalar_rhs) {
                    BValue scalar_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (scalar_val.type == VAL_STRING) str_release(vm_get_str(vm), scalar_val.as.string);
                    double s = (scalar_val.type == VAL_NUMBER) ? scalar_val.as.number : 0.0;
                    return mat_op_scalar(vm, dest_name, s, TOK_MINUS, name_a);
                }

                char name_b[256];
                if (!parse_array_name(lex, name_b, sizeof(name_b))) {
                    err.code = 2; err.message = "Expected second array name after '-'";
                    return err;
                }
                return mat_op_sub(vm, dest_name, name_a, name_b);
            } else if (op_tok.type == TOK_MUL) {
                lex_next(lex);
                BppToken peek_t = lex_peek(lex);
                bool is_scalar_rhs = (peek_t.type == TOK_NUMBER || peek_t.type == TOK_LPAREN);
                if (!is_scalar_rhs && peek_t.type == TOK_IDENT) {
                    char check_name[256] = {0};
                    size_t clen = peek_t.length < sizeof(check_name) - 1 ? peek_t.length : sizeof(check_name) - 1;
                    memcpy(check_name, peek_t.start, clen);
                    check_name[clen] = '\0';
                    if (!arr_exists(arr, check_name)) is_scalar_rhs = true;
                }

                if (is_scalar_rhs) {
                    BValue scalar_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (scalar_val.type == VAL_STRING) str_release(vm_get_str(vm), scalar_val.as.string);
                    double s = (scalar_val.type == VAL_NUMBER) ? scalar_val.as.number : 0.0;
                    return mat_op_scalar(vm, dest_name, s, TOK_MUL, name_a);
                }

                char name_b[256];
                if (!parse_array_name(lex, name_b, sizeof(name_b))) {
                    err.code = 2; err.message = "Expected second array name after '*'";
                    return err;
                }
                return mat_op_mul(vm, dest_name, name_a, name_b);
            } else if (op_tok.type == TOK_DIV) {
                lex_next(lex);
                BValue scalar_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (scalar_val.type == VAL_STRING) str_release(vm_get_str(vm), scalar_val.as.string);
                double s = (scalar_val.type == VAL_NUMBER) ? scalar_val.as.number : 0.0;
                return mat_op_div_scalar(vm, dest_name, name_a, s);
            } else {
                return mat_op_copy(vm, dest_name, name_a);
            }
        } else {
            BValue *kval = var_lookup(vm_get_var(vm), name_a, false);
            double s = (kval && kval->type == VAL_NUMBER) ? kval->as.number : 0.0;
            lex_next(lex);

            BppToken sop_tok = lex_peek(lex);
            if (sop_tok.type != TOK_MUL && sop_tok.type != TOK_PLUS && sop_tok.type != TOK_MINUS) {
                err.code = 2; err.message = "Expected '*', '+', or '-' in MAT scalar operation";
                return err;
            }
            lex_next(lex);

            char src_name[256];
            if (!parse_array_name(lex, src_name, sizeof(src_name))) {
                err.code = 2; err.message = "Expected array name in MAT scalar operation";
                return err;
            }
            return mat_op_scalar(vm, dest_name, s, sop_tok.type, src_name);
        }
    }

    err.code = 2; err.message = "Unrecognized MAT statement expression";
    return err;
}

//
// ---- Metadata Registration ----

void stmt_mat_ops_register(void) {
    MicroLibMetadata meta = {
        .name = "MAT",
        .category = "Matrix Operations",
        .syntax = "MAT var = expr",
        .help_text = "Performs matrix operations including addition, subtraction, multiplication, scalar, transpose, and inverse.",
        .error_codes = "Error 9: Subscript Out of Range, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

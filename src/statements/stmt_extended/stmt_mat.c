/**
 * @file stmt_mat.c
 * @brief ECMA-116 Matrix Operations (MAT) statement implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements matrix math operations (MAT) including reading, printing,
 *   initialization (ZER, CON, IDN), transposition (TRN), inversion (INV), addition,
 *   subtraction, matrix multiplication, and scalar multiplication.
 * - Why it exists: Fulfills the Phase 9 matrix math requirements to support Dartmouth
 *   and ECMA-116 compatible advanced calculations.
 * - Why it works this way: It queries array descriptors from the ArrayContext and performs
 *   nested iterations. In-place safety is guaranteed during matrix multiplication by copying
 *   source data if it overlaps with the target array.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Inversion size limits (currently 15x15), prompt formatting, print separators.
 * - What cannot be changed: Standard row-major indexing loops (ignoring row/col 0 under standard MAT behavior).
 * - What to expect: Invalid dimension counts or bounds mismatch raises error 9 (Subscript out of range).
 * - What to do if something breaks: Trace dimension sizes and index coordinate arrays.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Matrix indices are 1-based per Dartmouth standard MAT statement specifications.
 * - Portability concerns: Gauss-Jordan elimination uses standard double floating-point math.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional matrix solvers or eigenvalue solvers.
 * - How to write external extensions: Custom matrix plugins can query elements via arr_get_element.
 */


#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_arrays.h"
#include "bpp_eval.h"
#include "bpp_vdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/* Helper: parse bare array name */
static bool mat_get_array_name(LexerContext *lex, char *name_buf, size_t buf_size) {
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_IDENT) {
        return false;
    }
    lex_next(lex); /* Consume name identifier */
    size_t copy_len = (tok.length < buf_size - 1) ? tok.length : buf_size - 1;
    memcpy(name_buf, tok.as.string, copy_len);
    name_buf[copy_len] = '\0';
    return true;
}

/* Helper: match specific uppercase context identifier (ZER, CON, IDN, TRN, INV) */
static bool mat_match_ident(LexerContext *lex, const char *target) {
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_IDENT) {
        return false;
    }
    char name_buf[64];
    size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
    memcpy(name_buf, tok.as.string, copy_len);
    name_buf[copy_len] = '\0';

    for (size_t i = 0; i < copy_len; i++) {
        name_buf[i] = (char)toupper((unsigned char)name_buf[i]);
    }
    if (strcmp(name_buf, target) == 0) {
        lex_next(lex); /* Consume matched identifier */
        return true;
    }
    return false;
}

BppError stmt_mat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    ArrayContext *arr_ctx = vm_get_arr(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    BppToken tok = lex_peek(lex);

    /* --- MAT PRINT --- */
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PRINT) {
        lex_next(lex); /* Consume PRINT */

        char name_a[256];
        if (!mat_get_array_name(lex, name_a, sizeof(name_a))) {
            err.code = 2; err.message = "Expected array name in MAT PRINT";
            return err;
        }

        bool use_compact = false;
        BppToken trailing = lex_peek(lex);
        if (trailing.type == TOK_SEMICOLON) {
            use_compact = true;
            lex_next(lex);
        } else if (trailing.type == TOK_COMMA) {
            lex_next(lex);
        }

        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1 || num_dims > 2) {
            err.code = 9; err.message = "MAT PRINT requires a 1D or 2D array";
            return err;
        }

        int opt_base = arr_get_option_base(arr_ctx);

        if (num_dims == 1) {
            /* Print 1D vector row-wise */
            int start = (opt_base == 0) ? 0 : 1;
            for (int i = start; i <= bounds[0]; i++) {
                int indices[1] = {i};
                BValue *val = arr_get_element(arr_ctx, name_a, 1, indices, &err);
                if (err.code != 0 || !val) return err;

                if (val->type == VAL_STRING) {
                    vdev_printf(vdev, "%s", val->as.string ? str_data(val->as.string) : "");
                } else {
                    vdev_printf(vdev, "%g", val->as.number);
                }

                if (i < bounds[0]) {
                    vdev_puts(vdev, use_compact ? " " : "\t");
                }
            }
            vdev_putc(vdev, '\n');
        } else {
            /* Print 2D matrix */
            for (int r = 1; r <= bounds[0]; r++) {
                for (int c = 1; c <= bounds[1]; c++) {
                    int indices[2] = {r, c};
                    BValue *val = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                    if (err.code != 0 || !val) return err;

                    if (val->type == VAL_STRING) {
                        vdev_printf(vdev, "%s", val->as.string ? str_data(val->as.string) : "");
                    } else {
                        vdev_printf(vdev, "%g", val->as.number);
                    }

                    if (c < bounds[1]) {
                        vdev_puts(vdev, use_compact ? " " : "\t");
                    }
                }
                vdev_putc(vdev, '\n');
            }
        }
        return err;
    }

    /* --- MAT READ --- */
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) {
        lex_next(lex); /* Consume READ */

        char name_a[256];
        if (!mat_get_array_name(lex, name_a, sizeof(name_a))) {
            err.code = 2; err.message = "Expected array name in MAT READ";
            return err;
        }

        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1 || num_dims > 2) {
            err.code = 9; err.message = "MAT READ requires a 1D or 2D array";
            return err;
        }

        int opt_base = arr_get_option_base(arr_ctx);

        if (num_dims == 1) {
            int start = (opt_base == 0) ? 0 : 1;
            for (int i = start; i <= bounds[0]; i++) {
                int data_ptr = vm_get_data_ptr(vm);
                int data_count = vm_get_data_count(vm);
                BppDataPosition *data_items = vm_get_data_items(vm);

                if (data_ptr >= data_count || !data_items) {
                    err.code = 4; err.message = "Out of DATA in MAT READ";
                    return err;
                }

                BppDataPosition dp = data_items[data_ptr];
                vm_set_data_ptr(vm, data_ptr + 1);

                LexerContext *temp_lex = lex_init(mem, dp.pos);
                if (!temp_lex) {
                    err.code = 14; err.message = "Out of memory in MAT READ";
                    return err;
                }

                BppToken val_tok = lex_next(temp_lex);
                BValue val;
                val.type = VAL_NONE;
                val.as.number = 0.0;

                if (val_tok.type == TOK_NUMBER) {
                    val.type = VAL_NUMBER;
                    val.as.number = val_tok.as.number;
                } else {
                    val.type = VAL_STRING;
                    val.as.string = str_create(vm_get_str(vm), val_tok.as.string, val_tok.length);
                }
                lex_shutdown(temp_lex);

                int indices[1] = {i};
                BValue *elem = arr_get_element(arr_ctx, name_a, 1, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                *elem = val;
            }
        } else {
            for (int r = 1; r <= bounds[0]; r++) {
                for (int c = 1; c <= bounds[1]; c++) {
                    int data_ptr = vm_get_data_ptr(vm);
                    int data_count = vm_get_data_count(vm);
                    BppDataPosition *data_items = vm_get_data_items(vm);

                    if (data_ptr >= data_count || !data_items) {
                        err.code = 4; err.message = "Out of DATA in MAT READ";
                        return err;
                    }

                    BppDataPosition dp = data_items[data_ptr];
                    vm_set_data_ptr(vm, data_ptr + 1);

                    LexerContext *temp_lex = lex_init(mem, dp.pos);
                    if (!temp_lex) {
                        err.code = 14; err.message = "Out of memory in MAT READ";
                        return err;
                    }

                    BppToken val_tok = lex_next(temp_lex);
                    BValue val;
                    val.type = VAL_NONE;
                    val.as.number = 0.0;

                    if (val_tok.type == TOK_NUMBER) {
                        val.type = VAL_NUMBER;
                        val.as.number = val_tok.as.number;
                    } else {
                        val.type = VAL_STRING;
                        val.as.string = str_create(vm_get_str(vm), val_tok.as.string, val_tok.length);
                    }
                    lex_shutdown(temp_lex);

                    int indices[2] = {r, c};
                    BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                    if (err.code != 0 || !elem) return err;

                    if (elem->type == VAL_STRING && elem->as.string) {
                        str_release(vm_get_str(vm), elem->as.string);
                    }
                    *elem = val;
                }
            }
        }
        return err;
    }

    /* --- MAT INPUT --- */
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) {
        lex_next(lex); /* Consume INPUT */

        char name_a[256];
        if (!mat_get_array_name(lex, name_a, sizeof(name_a))) {
            err.code = 2; err.message = "Expected array name in MAT INPUT";
            return err;
        }

        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1 || num_dims > 2) {
            err.code = 9; err.message = "MAT INPUT requires a 1D vector or 2D matrix";
            return err;
        }

        char ibuf[256];
        int opt_base = arr_get_option_base(arr_ctx);

        if (num_dims == 1) {
            int start = (opt_base == 0) ? 0 : 1;
            for (int i = start; i <= bounds[0]; i++) {
                vdev_printf(vdev, "(%d)? ", i);

                VDev *con_dev = vdev_get(vdev, "CON:");
                if (con_dev && con_dev->ops.gets) {
                    con_dev->ops.gets(con_dev, ibuf, sizeof(ibuf));
                } else {
                    if (!fgets(ibuf, sizeof(ibuf), stdin)) {
                        err.code = 13; err.message = "Console input error in MAT INPUT";
                        return err;
                    }
                }

                double val = strtod(ibuf, NULL);
                int indices[1] = {i};
                BValue *elem = arr_get_element(arr_ctx, name_a, 1, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                elem->type = VAL_NUMBER;
                elem->as.number = val;
            }
        } else {
            for (int r = 1; r <= bounds[0]; r++) {
                for (int c = 1; c <= bounds[1]; c++) {
                    vdev_printf(vdev, "(%d,%d)? ", r, c);

                    VDev *con_dev = vdev_get(vdev, "CON:");
                    if (con_dev && con_dev->ops.gets) {
                        con_dev->ops.gets(con_dev, ibuf, sizeof(ibuf));
                    } else {
                        if (!fgets(ibuf, sizeof(ibuf), stdin)) {
                            err.code = 13; err.message = "Console input error in MAT INPUT";
                            return err;
                        }
                    }

                    double val = strtod(ibuf, NULL);
                    int indices[2] = {r, c};
                    BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                    if (err.code != 0 || !elem) return err;

                    if (elem->type == VAL_STRING && elem->as.string) {
                        str_release(vm_get_str(vm), elem->as.string);
                    }
                    elem->type = VAL_NUMBER;
                    elem->as.number = val;
                }
            }
        }
        return err;
    }

    /* --- MAT A = B ... assignments --- */
    char name_a[256];
    if (!mat_get_array_name(lex, name_a, sizeof(name_a))) {
        err.code = 2; err.message = "Expected array name in MAT";
        return err;
    }

    BppToken eq = lex_peek(lex);
    if (eq.type != TOK_EQ) {
        char uname[256];
        size_t len = strlen(name_a);
        for(size_t i=0; i<len; i++) uname[i] = (char)toupper((unsigned char)name_a[i]);
        uname[len] = '\0';
        
        if (strcmp(uname, "EIG") == 0) {
            /* MAT EIG A, E, V */
            char mat_a[64], mat_e[64], mat_v[64];
            if (!mat_get_array_name(lex, mat_a, sizeof(mat_a))) { err.code = 2; err.message = "Expected source array in EIG"; return err; }
            if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
            if (!mat_get_array_name(lex, mat_e, sizeof(mat_e))) { err.code = 2; err.message = "Expected eigenvalues array in EIG"; return err; }
            if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
            if (!mat_get_array_name(lex, mat_v, sizeof(mat_v))) { err.code = 2; err.message = "Expected eigenvectors array in EIG"; return err; }
            
            int b_a[4], b_e[4], b_v[4];
            int d_a = arr_get_dimensions(arr_ctx, mat_a, b_a, 4);
            int d_e = arr_get_dimensions(arr_ctx, mat_e, b_e, 4);
            int d_v = arr_get_dimensions(arr_ctx, mat_v, b_v, 4);
            
            if (d_a != 2 || b_a[0] != b_a[1]) { err.code = 9; err.message = "EIG requires square 2D matrix A"; return err; }
            if (d_e != 1 || b_e[0] != b_a[0]) { err.code = 9; err.message = "EIG requires 1D array E of same size"; return err; }
            if (d_v != 2 || b_v[0] != b_a[0] || b_v[1] != b_a[0]) { err.code = 9; err.message = "EIG requires 2D array V of same size"; return err; }
            
            int sz_a=0, sz_e=0, sz_v=0;
            BValue *ea = arr_get_flat_elements(arr_ctx, mat_a, &sz_a);
            BValue *ee = arr_get_flat_elements(arr_ctx, mat_e, &sz_e);
            BValue *ev = arr_get_flat_elements(arr_ctx, mat_v, &sz_v);
            if (!ea || !ee || !ev) { err.code = 9; err.message = "Arrays not found"; return err; }
            
            int base = arr_get_option_base(arr_ctx);
            int n = b_a[0] + (1 - base);
            int stride = b_a[1] + 1;
            
            /* Make a copy of A since Jacobi modifies it */
            double *A_copy = (double*)calloc(n * n, sizeof(double));
            if (!A_copy) { err.code = 7; err.message = "Out of memory in EIG"; return err; }
            
            for (int i=0; i<n; i++) {
                for (int j=0; j<n; j++) {
                    int flat = (i + base) * stride + (j + base);
                    A_copy[i*n + j] = ea[flat].as.number;
                }
            }
            
            /* Initialize V to Identity and E to diag(A) */
            for (int i=0; i<n; i++) {
                for (int j=0; j<n; j++) {
                    int flat = (i + base) * stride + (j + base);
                    if (ev[flat].type == VAL_STRING) str_release(vm_get_str(vm), ev[flat].as.string);
                    ev[flat].type = VAL_NUMBER;
                    ev[flat].as.number = (i == j) ? 1.0 : 0.0;
                }
                int flat_e = i + base;
                if (ee[flat_e].type == VAL_STRING) str_release(vm_get_str(vm), ee[flat_e].as.string);
                ee[flat_e].type = VAL_NUMBER;
                ee[flat_e].as.number = A_copy[i*n + i];
            }
            
            /* Jacobi method */
            int max_iter = 100 * n * n;
            for (int iter = 0; iter < max_iter; iter++) {
                double max_off = 0.0;
                int p = 0, q = 1;
                for (int i=0; i<n; i++) {
                    for (int j=i+1; j<n; j++) {
                        double off = fabs(A_copy[i*n + j]);
                        if (off > max_off) {
                            max_off = off;
                            p = i;
                            q = j;
                        }
                    }
                }
                
                if (max_off < 1e-12) break; /* Converged */
                
                double app = ee[p + base].as.number;
                double aqq = ee[q + base].as.number;
                double apq = A_copy[p*n + q];
                
                double theta = (aqq - app) / (2.0 * apq);
                double t = 1.0 / (fabs(theta) + sqrt(theta*theta + 1.0));
                if (theta < 0) t = -t;
                
                double c = 1.0 / sqrt(t*t + 1.0);
                double s = t * c;
                
                ee[p + base].as.number -= t * apq;
                ee[q + base].as.number += t * apq;
                A_copy[p*n + q] = 0.0;
                
                /* Rotate A */
                for (int i=0; i<n; i++) {
                    if (i != p && i != q) {
                        double aip = (i < p) ? A_copy[i*n + p] : A_copy[p*n + i];
                        double aiq = (i < q) ? A_copy[i*n + q] : A_copy[q*n + i];
                        double p_new = c * aip - s * aiq;
                        double q_new = s * aip + c * aiq;
                        if (i < p) A_copy[i*n + p] = p_new; else A_copy[p*n + i] = p_new;
                        if (i < q) A_copy[i*n + q] = q_new; else A_copy[q*n + i] = q_new;
                    }
                }
                
                /* Rotate V */
                for (int i=0; i<n; i++) {
                    int vip = (i + base) * stride + (p + base);
                    int viq = (i + base) * stride + (q + base);
                    double vp = ev[vip].as.number;
                    double vq = ev[viq].as.number;
                    ev[vip].as.number = c * vp - s * vq;
                    ev[viq].as.number = s * vp + c * vq;
                }
            }
            
            free(A_copy);
            return err;
        }
        
        if (strcmp(uname, "ROW") == 0 || strcmp(uname, "COL") == 0) {
            bool is_row = (strcmp(uname, "ROW") == 0);
            /* MAT ROW A(1) = B */
            char target_a[64];
            if (!mat_get_array_name(lex, target_a, sizeof(target_a))) { err.code = 2; err.message = "Expected array name"; return err; }
            if (lex_next(lex).type != TOK_LPAREN) { err.code = 2; err.message = "Expected '('"; return err; }
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            int idx = (int)idx_val.as.number;
            if (lex_next(lex).type != TOK_RPAREN) { err.code = 2; err.message = "Expected ')'"; return err; }
            
            if (lex_next(lex).type != TOK_EQ) { err.code = 2; err.message = "Expected '='"; return err; }
            
            char src_b[64];
            if (!mat_get_array_name(lex, src_b, sizeof(src_b))) { err.code = 2; err.message = "Expected source array name"; return err; }
            
            int b_a[4], b_b[4];
            if (arr_get_dimensions(arr_ctx, target_a, b_a, 4) != 2) { err.code = 9; err.message = "Target must be 2D"; return err; }
            if (arr_get_dimensions(arr_ctx, src_b, b_b, 4) != 1) { err.code = 9; err.message = "Source must be 1D"; return err; }
            
            int expected_len = is_row ? b_a[1] : b_a[0];
            if (b_b[0] != expected_len) { err.code = 9; err.message = "Dimension mismatch"; return err; }
            
            
            for (int i = 1; i <= expected_len; i++) {
                int src_idx[1] = {i};
                BValue *src_elem = arr_get_element(arr_ctx, src_b, 1, src_idx, &err);
                
                int dst_idx[2] = {is_row ? idx : i, is_row ? i : idx};
                BValue *dst_elem = arr_get_element(arr_ctx, target_a, 2, dst_idx, &err);
                
                if (src_elem && dst_elem) {
                    if (dst_elem->type == VAL_STRING) str_release(vm_get_str(vm), dst_elem->as.string);
                    dst_elem->type = VAL_NUMBER;
                    dst_elem->as.number = (src_elem->type == VAL_NUMBER) ? src_elem->as.number : 0.0;
                }
            }
            return err;
        }

        err.code = 2; err.message = "Expected '=' in MAT assignment";
        return err;
    }
    lex_next(lex); /* Consume '=' */

    tok = lex_peek(lex);

    /* --- ZER / CON / IDN --- */
    if (mat_match_ident(lex, "ZER")) {
        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1) {
            err.code = 9; err.message = "Array not dimensioned";
            return err;
        }
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, name_a, &total_size);
        if (!elems) {
            err.code = 9; err.message = "Array elements not found";
            return err;
        }
        for (int i = 0; i < total_size; i++) {
            if (elems[i].type == VAL_STRING && elems[i].as.string) {
                str_release(vm_get_str(vm), elems[i].as.string);
            }
            elems[i].type = VAL_NUMBER;
            elems[i].as.number = 0.0;
        }
        return err;
    }

    if (mat_match_ident(lex, "CON")) {
        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1) {
            err.code = 9; err.message = "Array not dimensioned";
            return err;
        }
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, name_a, &total_size);
        if (!elems) {
            err.code = 9; err.message = "Array elements not found";
            return err;
        }
        for (int i = 0; i < total_size; i++) {
            if (elems[i].type == VAL_STRING && elems[i].as.string) {
                str_release(vm_get_str(vm), elems[i].as.string);
            }
            elems[i].type = VAL_NUMBER;
            elems[i].as.number = 1.0;
        }
        return err;
    }

    if (mat_match_ident(lex, "IDN")) {
        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims != 2 || bounds[0] != bounds[1]) {
            err.code = 9; err.message = "MAT IDN requires square 2D matrix";
            return err;
        }
        for (int r = 1; r <= bounds[0]; r++) {
            for (int c = 1; c <= bounds[1]; c++) {
                int indices[2] = {r, c};
                BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                elem->type = VAL_NUMBER;
                elem->as.number = (r == c) ? 1.0 : 0.0;
            }
        }
        return err;
    }
    
    if (mat_match_ident(lex, "RND")) {
        int bounds[4];
        int num_dims = arr_get_dimensions(arr_ctx, name_a, bounds, 4);
        if (num_dims < 1) { err.code = 9; err.message = "Array not dimensioned"; return err; }
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, name_a, &total_size);
        if (!elems) { err.code = 9; err.message = "Array elements not found"; return err; }

        bool has_args = false;
        bool is_shuffle = false;
        char source_name[64] = {0};
        BValue arg1 = {0}; arg1.type = VAL_NONE; arg1.as.number = 0.0;
        BValue arg2 = {0}; arg2.type = VAL_NONE; arg2.as.number = 0.0;
        bool has_arg2 = false;

        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            has_args = true;
            
            BppToken arg_tok = lex_peek(lex);
            if (arg_tok.type == TOK_IDENT) {
                char test_name[64];
                size_t clen = arg_tok.length < 63 ? arg_tok.length : 63;
                memcpy(test_name, arg_tok.as.string, clen);
                test_name[clen] = '\0';
                for(size_t i=0; i<clen; i++) test_name[i] = (char)toupper((unsigned char)test_name[i]);
                
                int dummy_b[4];
                if (arr_get_dimensions(arr_ctx, test_name, dummy_b, 4) > 0) {
                    lex_next(lex); /* Consume ident */
                    is_shuffle = true;
                    snprintf(source_name, sizeof(source_name), "%s", test_name);
                }
            }
            if (!is_shuffle) {
                arg1 = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (lex_peek(lex).type == TOK_COMMA) {
                    lex_next(lex);
                    arg2 = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    has_arg2 = true;
                }
            }
            if (lex_next(lex).type != TOK_RPAREN) { err.code = 2; err.message = "Expected ')'"; return err; }
        }

        if (is_shuffle) {
            int src_bounds[4];
            int src_dims = arr_get_dimensions(arr_ctx, source_name, src_bounds, 4);
            if (src_dims < 1) { err.code = 9; err.message = "Source array not dimensioned"; return err; }
            int src_total = 0;
            BValue *src_elems = arr_get_flat_elements(arr_ctx, source_name, &src_total);
            
            if (src_total != total_size) { err.code = 9; err.message = "Dimension mismatch in MAT RND shuffle"; return err; }
            
            if (strcmp(name_a, source_name) != 0) {
                for (int i = 0; i < total_size; i++) {
                    if (elems[i].type == VAL_STRING && elems[i].as.string) str_release(vm_get_str(vm), elems[i].as.string);
                    elems[i].type = src_elems[i].type;
                    if (src_elems[i].type == VAL_STRING) {
                        elems[i].as.string = src_elems[i].as.string;
                        if (elems[i].as.string) str_add_ref(elems[i].as.string);
                    } else {
                        elems[i].as.number = src_elems[i].as.number;
                    }
                }
            }
            for (int i = total_size - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                BValue temp = elems[i];
                elems[i] = elems[j];
                elems[j] = temp;
            }
        } else if (has_args && arg1.type == VAL_STRING) {
            const char *fmt = arg1.as.string ? str_data(arg1.as.string) : "";
            bool is_hex = (strstr(fmt, "%x") || strstr(fmt, "%X") || strstr(fmt, "HEX"));
            bool is_oct = (strstr(fmt, "%o") || strstr(fmt, "OCT"));
            for (int i = 0; i < total_size; i++) {
                if (elems[i].type == VAL_STRING && elems[i].as.string) str_release(vm_get_str(vm), elems[i].as.string);
                elems[i].type = VAL_STRING;
                char buf[32];
                if (is_hex) {
                    snprintf(buf, sizeof(buf), "%X", rand() % 0x10000);
                } else if (is_oct) {
                    snprintf(buf, sizeof(buf), "%o", rand() % 010000);
                } else {
                    snprintf(buf, sizeof(buf), "%d", rand());
                }
                elems[i].as.string = str_create(vm_get_str(vm), buf, strlen(buf));
            }
        } else if (has_args && arg1.type == VAL_NUMBER) {
            double min_val = arg1.as.number;
            double max_val = has_arg2 ? arg2.as.number : min_val;
            if (!has_arg2) min_val = 0.0;
            
            for (int i = 0; i < total_size; i++) {
                if (elems[i].type == VAL_STRING && elems[i].as.string) str_release(vm_get_str(vm), elems[i].as.string);
                elems[i].type = VAL_NUMBER;
                double r = (double)rand() / RAND_MAX;
                elems[i].as.number = min_val + r * (max_val - min_val);
            }
        } else {
            for (int i = 0; i < total_size; i++) {
                if (elems[i].type == VAL_STRING && elems[i].as.string) str_release(vm_get_str(vm), elems[i].as.string);
                elems[i].type = VAL_NUMBER;
                elems[i].as.number = (double)rand() / RAND_MAX;
            }
        }
        return err;
    }
    
    bool is_ext_row = false, is_ext_col = false;
    BppToken ptok = lex_peek(lex);
    if (ptok.type == TOK_IDENT) {
        char pu[64];
        size_t clen = (ptok.length < 63) ? ptok.length : 63;
        memcpy(pu, ptok.as.string, clen); pu[clen] = '\0';
        for(size_t i=0; i<clen; i++) pu[i] = (char)toupper((unsigned char)pu[i]);
        if (strcmp(pu, "ROW") == 0) is_ext_row = true;
        else if (strcmp(pu, "COL") == 0) is_ext_col = true;
    }
    
    if (is_ext_row || is_ext_col) {
        lex_next(lex); /* Consume ROW or COL */
        char src_b[64];
        if (!mat_get_array_name(lex, src_b, sizeof(src_b))) { err.code = 2; err.message = "Expected source array name"; return err; }
        if (lex_next(lex).type != TOK_LPAREN) { err.code = 2; err.message = "Expected '('"; return err; }
        BValue idx_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int idx = (int)idx_val.as.number;
        if (lex_next(lex).type != TOK_RPAREN) { err.code = 2; err.message = "Expected ')'"; return err; }
        
        int b_a[4], b_b[4];
        if (arr_get_dimensions(arr_ctx, name_a, b_a, 4) != 1) { err.code = 9; err.message = "Target must be 1D"; return err; }
        if (arr_get_dimensions(arr_ctx, src_b, b_b, 4) != 2) { err.code = 9; err.message = "Source must be 2D"; return err; }
        
        int expected_len = is_ext_row ? b_b[1] : b_b[0];
        if (b_a[0] != expected_len) { err.code = 9; err.message = "Dimension mismatch in ROW/COL extraction"; return err; }
        
        for (int i = 1; i <= expected_len; i++) {
            int src_idx[2] = {is_ext_row ? idx : i, is_ext_row ? i : idx};
            BValue *src_elem = arr_get_element(arr_ctx, src_b, 2, src_idx, &err);
            
            int dst_idx[1] = {i};
            BValue *dst_elem = arr_get_element(arr_ctx, name_a, 1, dst_idx, &err);
            
            if (src_elem && dst_elem) {
                if (dst_elem->type == VAL_STRING) str_release(vm_get_str(vm), dst_elem->as.string);
                dst_elem->type = VAL_NUMBER;
                dst_elem->as.number = (src_elem->type == VAL_NUMBER) ? src_elem->as.number : 0.0;
            }
        }
        return err;
    }

    if (mat_match_ident(lex, "CROSS")) {
        if (lex_next(lex).type != TOK_LPAREN) { err.code = 2; err.message = "Expected '(' in CROSS"; return err; }
        char src_b[64], src_c[64];
        if (!mat_get_array_name(lex, src_b, sizeof(src_b))) { err.code = 2; err.message = "Expected first array"; return err; }
        if (lex_next(lex).type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
        if (!mat_get_array_name(lex, src_c, sizeof(src_c))) { err.code = 2; err.message = "Expected second array"; return err; }
        if (lex_next(lex).type != TOK_RPAREN) { err.code = 2; err.message = "Expected ')'"; return err; }
        
        int b_b[4], b_c[4], b_a[4];
        if (arr_get_dimensions(arr_ctx, src_b, b_b, 4) != 1 || arr_get_dimensions(arr_ctx, src_c, b_c, 4) != 1 || arr_get_dimensions(arr_ctx, name_a, b_a, 4) != 1) {
            err.code = 9; err.message = "CROSS requires 1D arrays"; return err;
        }
        if (b_b[0] != 3 || b_c[0] != 3 || b_a[0] != 3) {
            err.code = 9; err.message = "CROSS requires vectors of size 3"; return err;
        }
        int i1[1]={1}, i2[1]={2}, i3[1]={3};
        BValue *ub1 = arr_get_element(arr_ctx, src_b, 1, i1, &err); BValue *ub2 = arr_get_element(arr_ctx, src_b, 1, i2, &err); BValue *ub3 = arr_get_element(arr_ctx, src_b, 1, i3, &err);
        BValue *vc1 = arr_get_element(arr_ctx, src_c, 1, i1, &err); BValue *vc2 = arr_get_element(arr_ctx, src_c, 1, i2, &err); BValue *vc3 = arr_get_element(arr_ctx, src_c, 1, i3, &err);
        BValue *ea1 = arr_get_element(arr_ctx, name_a, 1, i1, &err); BValue *ea2 = arr_get_element(arr_ctx, name_a, 1, i2, &err); BValue *ea3 = arr_get_element(arr_ctx, name_a, 1, i3, &err);
        if(err.code != 0 || !ub1 || !ub2 || !ub3 || !vc1 || !vc2 || !vc3 || !ea1 || !ea2 || !ea3) return err;
        
        double u1 = ub1->as.number, u2 = ub2->as.number, u3 = ub3->as.number;
        double v1 = vc1->as.number, v2 = vc2->as.number, v3 = vc3->as.number;
        
        if (ea1->type == VAL_STRING) { str_release(vm_get_str(vm), ea1->as.string); } ea1->type = VAL_NUMBER; ea1->as.number = u2*v3 - u3*v2;
        if (ea2->type == VAL_STRING) { str_release(vm_get_str(vm), ea2->as.string); } ea2->type = VAL_NUMBER; ea2->as.number = u3*v1 - u1*v3;
        if (ea3->type == VAL_STRING) { str_release(vm_get_str(vm), ea3->as.string); } ea3->type = VAL_NUMBER; ea3->as.number = u1*v2 - u2*v1;
        return err;
    }

    /* --- TRN(B) --- */
    if (mat_match_ident(lex, "TRN")) {
        BppToken lp = lex_next(lex);
        if (lp.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' in MAT TRN";
            return err;
        }
        char name_b[256];
        if (!mat_get_array_name(lex, name_b, sizeof(name_b))) {
            err.code = 2; err.message = "Expected source array name in MAT TRN";
            return err;
        }
        BppToken rp = lex_next(lex);
        if (rp.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in MAT TRN";
            return err;
        }

        int bounds_a[4], bounds_b[4];
        int num_dims_a = arr_get_dimensions(arr_ctx, name_a, bounds_a, 4);
        int num_dims_b = arr_get_dimensions(arr_ctx, name_b, bounds_b, 4);

        if (num_dims_a != 2 || num_dims_b != 2) {
            err.code = 9; err.message = "MAT TRN requires 2D matrices";
            return err;
        }
        if (bounds_a[0] != bounds_b[1] || bounds_a[1] != bounds_b[0]) {
            err.code = 9; err.message = "Matrix dimension mismatch in MAT TRN";
            return err;
        }

        /* In-place safety swap buffer */
        BValue *temp_b = (BValue *)mem_scratch_alloc(mem, (bounds_b[0] + 1) * (bounds_b[1] + 1) * sizeof(BValue));
        if (!temp_b) {
            err.code = 14; err.message = "Scratch overflow in MAT TRN";
            return err;
        }

        for (int r = 1; r <= bounds_b[0]; r++) {
            for (int c = 1; c <= bounds_b[1]; c++) {
                int indices[2] = {r, c};
                BValue *val = arr_get_element(arr_ctx, name_b, 2, indices, &err);
                if (err.code != 0 || !val) return err;
                temp_b[r * (bounds_b[1] + 1) + c] = *val;
            }
        }

        for (int r = 1; r <= bounds_a[0]; r++) {
            for (int c = 1; c <= bounds_a[1]; c++) {
                int indices[2] = {r, c};
                BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                /* Source B(c, r) mapped to Target A(r, c) */
                BValue src_val = temp_b[c * (bounds_b[1] + 1) + r];
                elem->type = VAL_NUMBER;
                elem->as.number = (src_val.type == VAL_STRING) ? 0.0 : src_val.as.number;
            }
        }
        return err;
    }

    /* --- INV(B) --- */
    if (mat_match_ident(lex, "INV")) {
        BppToken lp = lex_next(lex);
        if (lp.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' in MAT INV";
            return err;
        }
        char name_b[256];
        if (!mat_get_array_name(lex, name_b, sizeof(name_b))) {
            err.code = 2; err.message = "Expected source array name in MAT INV";
            return err;
        }
        BppToken rp = lex_next(lex);
        if (rp.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in MAT INV";
            return err;
        }

        int bounds_a[4], bounds_b[4];
        int num_dims_a = arr_get_dimensions(arr_ctx, name_a, bounds_a, 4);
        int num_dims_b = arr_get_dimensions(arr_ctx, name_b, bounds_b, 4);

        if (num_dims_a != 2 || num_dims_b != 2 || bounds_b[0] != bounds_b[1]) {
            err.code = 9; err.message = "MAT INV requires square 2D matrices";
            return err;
        }
        if (bounds_a[0] != bounds_b[0] || bounds_a[1] != bounds_b[1]) {
            err.code = 9; err.message = "Matrix dimension mismatch in MAT INV";
            return err;
        }

        int n = bounds_b[0];
        if (n > 15 || n < 1) {
            err.code = 9; err.message = "Matrix size exceeds limit (15x15) for INV";
            return err;
        }

        double work[16][32];

        /* Build augmented matrix [B | I] using 1-based indexing */
        for (int r = 1; r <= n; r++) {
            for (int c = 1; c <= n; c++) {
                int indices[2] = {r, c};
                BValue *val = arr_get_element(arr_ctx, name_b, 2, indices, &err);
                if (err.code != 0 || !val) return err;
                work[r - 1][c - 1] = (val->type == VAL_STRING) ? 0.0 : val->as.number;
                work[r - 1][c - 1 + n] = (r == c) ? 1.0 : 0.0;
            }
        }

        double det = 1.0;

        /* Gauss-Jordan Elimination */
        for (int p = 0; p < n; p++) {
            int max_row = p;
            double max_val = fabs(work[p][p]);

            for (int r = p + 1; r < n; r++) {
                double val = fabs(work[r][p]);
                if (val > max_val) {
                    max_val = val;
                    max_row = r;
                }
            }

            if (max_row != p) {
                det = -det;
                for (int c = 0; c < 2 * n; c++) {
                    double tmp = work[p][c];
                    work[p][c] = work[max_row][c];
                    work[max_row][c] = tmp;
                }
            }

            double pivot = work[p][p];
            if (fabs(pivot) < 1e-12) {
                err.code = 9; err.message = "Matrix is singular: cannot invert";
                arr_set_last_det(arr_ctx, 0.0);
                return err;
            }
            
            det *= pivot;

            for (int c = 0; c < 2 * n; c++) {
                work[p][c] /= pivot;
            }

            for (int r = 0; r < n; r++) {
                if (r == p) continue;
                double factor = work[r][p];
                for (int c = 0; c < 2 * n; c++) {
                    work[r][c] -= factor * work[p][c];
                }
            }
        }

        /* Extract inverse into A */
        for (int r = 1; r <= n; r++) {
            for (int c = 1; c <= n; c++) {
                int indices[2] = {r, c};
                BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                elem->type = VAL_NUMBER;
                elem->as.number = work[r - 1][c - 1 + n];
            }
        }
        
        arr_set_last_det(arr_ctx, det);
        return err;
    }

    /* --- MAT A = scalar * B --- */
    bool is_scalar_mul = false;
    BValue scalar;
    memset(&scalar, 0, sizeof(scalar));
    
    if (tok.type == TOK_LPAREN) {
        /* Check if it is indeed a scalar expression: (expr) * B */
        const char *saved_pos = lex_get_pos(lex);
        BppToken temp_tok = lex_next(lex); /* Consume '(' */
        int paren_depth = 0;
        while (temp_tok.type != TOK_EOF && temp_tok.type != TOK_EOL) {
            if (temp_tok.type == TOK_LPAREN) paren_depth++;
            else if (temp_tok.type == TOK_RPAREN) {
                paren_depth--;
                if (paren_depth == 0) {
                    BppToken next_temp = lex_peek(lex);
                    if (next_temp.type == TOK_MUL) {
                        is_scalar_mul = true;
                    }
                    break;
                }
            }
            temp_tok = lex_next(lex);
        }
        lex_set_pos(lex, saved_pos);
        
        if (is_scalar_mul) {
            lex_next(lex); /* Consume '(' */
            scalar = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            lex_next(lex); /* Consume ')' */
            lex_next(lex); /* Consume '*' */
        }
    } else if (tok.type == TOK_NUMBER) {
        const char *saved_pos = lex_get_pos(lex);
        lex_next(lex); /* Consume number */
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_MUL) {
            is_scalar_mul = true;
        }
        lex_set_pos(lex, saved_pos);
        
        if (is_scalar_mul) {
            scalar = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            lex_next(lex); /* Consume '*' */
        }
    }

    if (is_scalar_mul) {
        char name_b[256];
        if (!mat_get_array_name(lex, name_b, sizeof(name_b))) {
            err.code = 2; err.message = "Expected source array name";
            return err;
        }

        int bounds_a[4], bounds_b[4];
        int num_dims_a = arr_get_dimensions(arr_ctx, name_a, bounds_a, 4);
        int num_dims_b = arr_get_dimensions(arr_ctx, name_b, bounds_b, 4);

        if (num_dims_a != num_dims_b || bounds_a[0] != bounds_b[0] || (num_dims_a == 2 && bounds_a[1] != bounds_b[1])) {
            err.code = 9; err.message = "Matrix dimension mismatch in scalar multiply";
            return err;
        }

        double k = (scalar.type == VAL_STRING) ? 0.0 : scalar.as.number;
        int total_size = 0, total_b = 0;
        BValue *elems_a = arr_get_flat_elements(arr_ctx, name_a, &total_size);
        BValue *elems_b = arr_get_flat_elements(arr_ctx, name_b, &total_b);

        if (elems_a && elems_b && total_size == total_b) {
            for (int i = 0; i < total_size; i++) {
                if (elems_a[i].type == VAL_STRING && elems_a[i].as.string) {
                    str_release(vm_get_str(vm), elems_a[i].as.string);
                }
                elems_a[i].type = VAL_NUMBER;
                elems_a[i].as.number = ((elems_b[i].type == VAL_STRING) ? 0.0 : elems_b[i].as.number) * k;
            }
        }
        return err;
    }

    /* --- MAT A = B + or - or * C --- */
    char name_b[256];
    if (!mat_get_array_name(lex, name_b, sizeof(name_b))) {
        err.code = 2; err.message = "Expected source array name";
        return err;
    }

    BppToken op_tok = lex_peek(lex);
    if (op_tok.type != TOK_PLUS && op_tok.type != TOK_MINUS && op_tok.type != TOK_MUL) {
        /* Simple copy MAT A = B */
        int bounds_a[4], bounds_b[4];
        int num_dims_a = arr_get_dimensions(arr_ctx, name_a, bounds_a, 4);
        int num_dims_b = arr_get_dimensions(arr_ctx, name_b, bounds_b, 4);

        if (num_dims_a != num_dims_b || bounds_a[0] != bounds_b[0] || (num_dims_a == 2 && bounds_a[1] != bounds_b[1])) {
            err.code = 9; err.message = "Matrix dimension mismatch in copy";
            return err;
        }

        int total_a = 0, total_b = 0;
        BValue *elems_a = arr_get_flat_elements(arr_ctx, name_a, &total_a);
        BValue *elems_b = arr_get_flat_elements(arr_ctx, name_b, &total_b);

        if (!elems_a || !elems_b || total_a != total_b) {
            err.code = 9; err.message = "Matrix data not found";
            return err;
        }

        for (int i = 0; i < total_a; i++) {
            if (elems_a[i].type == VAL_STRING && elems_a[i].as.string) {
                str_release(vm_get_str(vm), elems_a[i].as.string);
            }
            elems_a[i] = elems_b[i];
            if (elems_a[i].type == VAL_STRING && elems_a[i].as.string) {
                str_add_ref(elems_a[i].as.string);
            }
        }
        return err;
    }

    lex_next(lex); /* Consume operation operator */

    char name_c[256];
    if (!mat_get_array_name(lex, name_c, sizeof(name_c))) {
        err.code = 2; err.message = "Expected second source array name";
        return err;
    }

    int bounds_a[4], bounds_b[4], bounds_c[4];
    int num_dims_a = arr_get_dimensions(arr_ctx, name_a, bounds_a, 4);
    int num_dims_b = arr_get_dimensions(arr_ctx, name_b, bounds_b, 4);
    int num_dims_c = arr_get_dimensions(arr_ctx, name_c, bounds_c, 4);

    if (num_dims_a != 2 || num_dims_b != 2 || num_dims_c != 2) {
        err.code = 9; err.message = "MAT arithmetic requires 2D matrices";
        return err;
    }

    if (op_tok.type == TOK_PLUS || op_tok.type == TOK_MINUS) {
        if (bounds_a[0] != bounds_b[0] || bounds_a[1] != bounds_b[1] || bounds_b[0] != bounds_c[0] || bounds_b[1] != bounds_c[1]) {
            err.code = 9; err.message = "Matrix dimension mismatch in MAT add/sub";
            return err;
        }

        for (int r = 1; r <= bounds_a[0]; r++) {
            for (int c = 1; c <= bounds_a[1]; c++) {
                int indices[2] = {r, c};
                BValue *val_b = arr_get_element(arr_ctx, name_b, 2, indices, &err);
                if (err.code != 0 || !val_b) return err;

                BValue *val_c = arr_get_element(arr_ctx, name_c, 2, indices, &err);
                if (err.code != 0 || !val_c) return err;

                BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                if (err.code != 0 || !elem) return err;

                double bv = (val_b->type == VAL_STRING) ? 0.0 : val_b->as.number;
                double cv = (val_c->type == VAL_STRING) ? 0.0 : val_c->as.number;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                elem->type = VAL_NUMBER;
                elem->as.number = (op_tok.type == TOK_PLUS) ? (bv + cv) : (bv - cv);
            }
        }
    } else {
        /* MAT A = B * C (matrix multiplication) */
        if (bounds_b[1] != bounds_c[0]) {
            err.code = 9; err.message = "Inner matrix dimensions must match for multiplication";
            return err;
        }
        if (bounds_a[0] != bounds_b[0] || bounds_a[1] != bounds_c[1]) {
            err.code = 9; err.message = "Target matrix dimensions mismatch in multiply";
            return err;
        }

        /* In-place safety buffer */
        int size_b = (bounds_b[0] + 1) * (bounds_b[1] + 1);
        int size_c = (bounds_c[0] + 1) * (bounds_c[1] + 1);

        BValue *temp_b = (BValue *)mem_scratch_alloc(mem, size_b * sizeof(BValue));
        BValue *temp_c = (BValue *)mem_scratch_alloc(mem, size_c * sizeof(BValue));

        if (!temp_b || !temp_c) {
            err.code = 14; err.message = "Scratch overflow in MAT multiply";
            return err;
        }

        for (int r = 1; r <= bounds_b[0]; r++) {
            for (int c = 1; c <= bounds_b[1]; c++) {
                int indices[2] = {r, c};
                BValue *val = arr_get_element(arr_ctx, name_b, 2, indices, &err);
                if (err.code != 0 || !val) return err;
                temp_b[r * (bounds_b[1] + 1) + c] = *val;
            }
        }

        for (int r = 1; r <= bounds_c[0]; r++) {
            for (int c = 1; c <= bounds_c[1]; c++) {
                int indices[2] = {r, c};
                BValue *val = arr_get_element(arr_ctx, name_c, 2, indices, &err);
                if (err.code != 0 || !val) return err;
                temp_c[r * (bounds_c[1] + 1) + c] = *val;
            }
        }

        for (int r = 1; r <= bounds_a[0]; r++) {
            for (int c = 1; c <= bounds_a[1]; c++) {
                double sum = 0.0;
                for (int k = 1; k <= bounds_b[1]; k++) {
                    BValue val_b = temp_b[r * (bounds_b[1] + 1) + k];
                    BValue val_c = temp_c[k * (bounds_c[1] + 1) + c];
                    double bv = (val_b.type == VAL_STRING) ? 0.0 : val_b.as.number;
                    double cv = (val_c.type == VAL_STRING) ? 0.0 : val_c.as.number;
                    sum += bv * cv;
                }

                int indices[2] = {r, c};
                BValue *elem = arr_get_element(arr_ctx, name_a, 2, indices, &err);
                if (err.code != 0 || !elem) return err;

                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                elem->type = VAL_NUMBER;
                elem->as.number = sum;
            }
        }
    }

    return err;
}


/**
 * @file stmt_remove.c
 * @brief REMOVE and REMOVE$ Statement Handler Subsystem
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements in-place modification statement handlers for REMOVE and REMOVE$.
 *   These statements take a variable or array (either scalar string, string array, scalar numeric, or numeric array),
 *   find occurrences matching a target pattern, and delete/zero them in-place.
 * - Why it exists: Provides high-level bulk removal utilities for BASIC++ arrays and scalar variables,
 *   reducing code complexity for common data cleansing and processing loops.
 * - Why it works this way: It resolves references directly via the Variable and Array contexts. If it resolves
 *   to a whole array (e.g. names$() or names$), it iterates over flat elements sequentially.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Target matching constraints or search algorithms.
 * - What cannot be changed: In-place string reference management. Releasing target and source strings
 *   must be done safely to prevent memory leaks or dangling pointers.
 * - What to expect: Standard execution modifies the destination variable/array directly.
 * - What to do if something breaks: Verify variable types match (VAL_STRING vs VAL_NUMBER) and that the VM and Array
 *   contexts are initialized.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Valid variable names and types are evaluated.
 * - Portability concerns: C17 compliant, zero dependency.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Extend array dimensions mapping or allow regex support inside REMOVE$.
 * - How to write external extensions: External plugins can hook into the statements registry.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/arrays.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>

BppError stmt_remove_str_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Parse variable or array name */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable or array name in REMOVE$";
        return err;
    }

    char var_name[64];
    size_t clen = (tok.length < 63) ? tok.length : 63;
    memcpy(var_name, tok.start, clen);
    var_name[clen] = '\0';

    bool is_array = false;
    ArrayContext *arr_ctx = vm_get_arr(vm);

    /* Check if it is explicitly followed by empty parens () or is an array */
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_RPAREN) {
            lex_next(lex); /* Consume ')' */
            is_array = true;
        } else {
            /* It has indices, meaning it is an element reference, e.g. arr$(i, j) */
            int indices[4];
            int num_indices = 0;
            while (true) {
                if (num_indices >= 4) {
                    err.code = 9; err.message = "Too many dimensions"; return err;
                }
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                indices[num_indices++] = (int)idx_val.as.number;

                BppToken sep = lex_peek(lex);
                if (sep.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (sep.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')'"; return err;
                }
            }
            
            /* Read next token which must be a comma */
            BppToken comma = lex_peek(lex);
            if (comma.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ','"; return err;
            }
            lex_next(lex); /* Consume ',' */

            /* Parse find string */
            BValue find_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (find_val.type != VAL_STRING) {
                err.code = 13; err.message = "Expected string argument to remove";
                return err;
            }

            /* Resolve element */
            BValue *target = arr_get_element(arr_ctx, var_name, num_indices, indices, &err);
            if (err.code != 0 || !target) {
                str_release(vm_get_str(vm), find_val.as.string);
                return err;
            }
            if (target->type != VAL_STRING) {
                err.code = 13; err.message = "Array element must be a string";
                str_release(vm_get_str(vm), find_val.as.string);
                return err;
            }

            /* Remove substring */
            BppStringRef sr_orig = target->as.string;
            BppStringRef sr_find = find_val.as.string;
            const char *orig = str_data(sr_orig);
            const char *find = str_data(sr_find);
            size_t orig_len = str_len(sr_orig);
            size_t find_len = str_len(sr_find);

            if (find_len > 0) {
                size_t count = 0;
                const char *p = orig;
                while ((p = strstr(p, find)) != NULL) {
                    count++;
                    p += find_len;
                }

                size_t new_len = orig_len - count * find_len;
                char *buf = (char *)calloc(1, new_len + 1);
                if (!buf) {
                    err.code = 14; err.message = "Out of memory";
                    str_release(vm_get_str(vm), find_val.as.string);
                    return err;
                }

                char *dst = buf;
                const char *src = orig;
                while (true) {
                    const char *next = strstr(src, find);
                    if (!next) {
                        strcpy(dst, src);
                        break;
                    }
                    size_t prefix_len = next - src;
                    memcpy(dst, src, prefix_len);
                    dst += prefix_len;
                    src = next + find_len;
                }
                target->as.string = str_create(vm_get_str(vm), buf, new_len);
                free(buf);
                str_release(vm_get_str(vm), sr_orig);
            }
            str_release(vm_get_str(vm), find_val.as.string);
            return err;
        }
    } else {
        /* Check if the name represents an existing array */
        if (arr_exists(arr_ctx, var_name)) {
            is_array = true;
        }
    }

    /* Expect comma */
    BppToken comma = lex_peek(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ','"; return err;
    }
    lex_next(lex); /* Consume ',' */

    /* Parse find string */
    BValue find_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (find_val.type != VAL_STRING) {
        err.code = 13; err.message = "Expected string argument to remove";
        return err;
    }

    if (is_array) {
        /* Iterate over array elements */
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, var_name, &total_size);
        if (!elems || total_size <= 0) {
            err.code = 9; err.message = "Array not dimensioned or empty";
            str_release(vm_get_str(vm), find_val.as.string);
            return err;
        }

        BppStringRef sr_find = find_val.as.string;
        const char *find = str_data(sr_find);
        size_t find_len = str_len(sr_find);

        if (find_len > 0) {
            for (int idx = 0; idx < total_size; idx++) {
                if (elems[idx].type == VAL_STRING) {
                    BppStringRef sr_orig = elems[idx].as.string;
                    const char *orig = str_data(sr_orig);
                    size_t orig_len = str_len(sr_orig);

                    size_t count = 0;
                    const char *p = orig;
                    while ((p = strstr(p, find)) != NULL) {
                        count++;
                        p += find_len;
                    }

                    if (count > 0) {
                        size_t new_len = orig_len - count * find_len;
                        char *buf = (char *)calloc(1, new_len + 1);
                        if (!buf) {
                            err.code = 14; err.message = "Out of memory";
                            str_release(vm_get_str(vm), find_val.as.string);
                            return err;
                        }

                        char *dst = buf;
                        const char *src = orig;
                        while (true) {
                            const char *next = strstr(src, find);
                            if (!next) {
                                strcpy(dst, src);
                                break;
                            }
                            size_t prefix_len = next - src;
                            memcpy(dst, src, prefix_len);
                            dst += prefix_len;
                            src = next + find_len;
                        }
                        elems[idx].as.string = str_create(vm_get_str(vm), buf, new_len);
                        free(buf);
                        str_release(vm_get_str(vm), sr_orig);
                    }
                }
            }
        }
    } else {
        /* Scalar string variable */
        VariableContext *var_ctx = vm_get_var(vm);
        BValue *target = var_lookup(var_ctx, var_name, true);
        if (!target) {
            err.code = 2; err.message = "Undefined variable in REMOVE$";
            str_release(vm_get_str(vm), find_val.as.string);
            return err;
        }

        if (target->type != VAL_STRING) {
            err.code = 13; err.message = "Variable must be a string";
            str_release(vm_get_str(vm), find_val.as.string);
            return err;
        }

        BppStringRef sr_orig = target->as.string;
        BppStringRef sr_find = find_val.as.string;
        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);

        if (find_len > 0) {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len;
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err.code = 14; err.message = "Out of memory";
                str_release(vm_get_str(vm), find_val.as.string);
                return err;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                src = next + find_len;
            }
            target->as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
            str_release(vm_get_str(vm), sr_orig);
        }
    }

    str_release(vm_get_str(vm), find_val.as.string);
    return err;
}

BppError stmt_remove_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Parse variable or array name */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable or array name in REMOVE";
        return err;
    }

    char var_name[64];
    size_t clen = (tok.length < 63) ? tok.length : 63;
    memcpy(var_name, tok.start, clen);
    var_name[clen] = '\0';

    bool is_array = false;
    ArrayContext *arr_ctx = vm_get_arr(vm);

    /* Check if it is explicitly followed by empty parens () or is an array */
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); /* Consume '(' */
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_RPAREN) {
            lex_next(lex); /* Consume ')' */
            is_array = true;
        } else {
            /* It has indices, meaning it is an element reference, e.g. arr(i, j) */
            int indices[4];
            int num_indices = 0;
            while (true) {
                if (num_indices >= 4) {
                    err.code = 9; err.message = "Too many dimensions"; return err;
                }
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                indices[num_indices++] = (int)idx_val.as.number;

                BppToken sep = lex_peek(lex);
                if (sep.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (sep.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')'"; return err;
                }
            }
            
            /* Read next token which must be a comma */
            BppToken comma = lex_peek(lex);
            if (comma.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ','"; return err;
            }
            lex_next(lex); /* Consume ',' */

            /* Parse find value */
            BValue find_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (find_val.type == VAL_STRING) {
                err.code = 13; err.message = "Expected numeric argument to remove";
                return err;
            }

            /* Resolve element */
            BValue *target = arr_get_element(arr_ctx, var_name, num_indices, indices, &err);
            if (err.code != 0 || !target) return err;
            if (target->type == VAL_STRING) {
                err.code = 13; err.message = "Array element must be numeric";
                return err;
            }

            /* Remove value (zero out if matches) */
            if (target->as.number == find_val.as.number) {
                target->as.number = 0.0;
            }
            return err;
        }
    } else {
        /* Check if the name represents an existing array */
        if (arr_exists(arr_ctx, var_name)) {
            is_array = true;
        }
    }

    /* Expect comma */
    BppToken comma = lex_peek(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ','"; return err;
    }
    lex_next(lex); /* Consume ',' */

    /* Parse find value */
    BValue find_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (find_val.type == VAL_STRING) {
        err.code = 13; err.message = "Expected numeric argument to remove";
        return err;
    }

    if (is_array) {
        /* Iterate over array elements */
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, var_name, &total_size);
        if (!elems || total_size <= 0) {
            err.code = 9; err.message = "Array not dimensioned or empty";
            return err;
        }

        double val = find_val.as.number;
        for (int idx = 0; idx < total_size; idx++) {
            if (elems[idx].type != VAL_STRING) {
                if (elems[idx].as.number == val) {
                    elems[idx].as.number = 0.0;
                }
            }
        }
    } else {
        /* Scalar numeric variable */
        VariableContext *var_ctx = vm_get_var(vm);
        BValue *target = var_lookup(var_ctx, var_name, true);
        if (!target) {
            err.code = 2; err.message = "Undefined variable in REMOVE";
            return err;
        }

        if (target->type == VAL_STRING) {
            err.code = 13; err.message = "Variable must be numeric";
            return err;
        }

        if (target->as.number == find_val.as.number) {
            target->as.number = 0.0;
        }
    }

    return err;
}

void stmt_remove_register(void) {
    static const MicroLibMetadata meta = {
        .name = "REMOVE",
        .category = "Variables & Memory",
        .syntax = "REMOVE var_or_array, value | REMOVE$ str_var_or_array, target_str$",
        .help_text = "In-place removal/cleansing of values or substring patterns from variables and arrays.",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}


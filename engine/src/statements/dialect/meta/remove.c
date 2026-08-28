// FILENAME: remove.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, stmt.h)
// Provides runtime implementation for the REMOVE statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/arrays.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_remove_str_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // Parse variable or array name
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable or array name in REMOVE$";
        return err;
    }

    char var_name[64];
    size_t clen = (tok.length < 63) ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(var_name, tok.start, clen);
    }
    var_name[clen] = '\0';

    bool is_array = false;
    ArrayContext *arr_ctx = vm_get_arr(vm);

    // Check if it is explicitly followed by empty parens () or is an array
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_RPAREN) {
            lex_next(lex); // Consume ')'
            is_array = true;
        } else {
            // It has indices, meaning it is an element reference, e.g. arr$(i, j)
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
            
            // Read next token which must be a comma
            BppToken comma = lex_peek(lex);
            if (comma.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ','"; return err;
            }
            lex_next(lex); // Consume ','

            // Parse find string
            BValue find_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (find_val.type != VAL_STRING) {
                err.code = 13; err.message = "Expected string argument to remove";
                return err;
            }

            // Resolve element
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

            // Remove substring
            BppStringRef sr_orig = target->as.string;
            BppStringRef sr_find = find_val.as.string;
            const char *orig = str_data(sr_orig);
            const char *find = str_data(sr_find);
            size_t orig_len = str_len(sr_orig);
            size_t find_len = str_len(sr_find);

            if (find_len > 0) {
                size_t count = 0;
                const char *p = orig;
                while ((p = runtime_strstr(p, find)) != NULL) {
                    count++;
                    p += find_len;
                }

                size_t new_len = orig_len - count * find_len;
                HalContext *hal = hal_get();
                char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(new_len + 1) : NULL;
                if (!buf) {
                    err.code = 14; err.message = "Out of memory";
                    str_release(vm_get_str(vm), find_val.as.string);
                    return err;
                }
                runtime_memset(buf, 0, new_len + 1);

                char *dst = buf;
                const char *src = orig;
                while (true) {
                    const char *next = runtime_strstr(src, find);
                    if (!next) {
                        runtime_strcpy(dst, src);
                        break;
                    }
                    size_t prefix_len = next - src;
                    runtime_memcpy(dst, src, prefix_len);
                    dst += prefix_len;
                    src = next + find_len;
                }
                target->as.string = str_create(vm_get_str(vm), buf, new_len);
                if (hal && hal->mem.free) hal->mem.free(buf);
                str_release(vm_get_str(vm), sr_orig);
            }
            str_release(vm_get_str(vm), find_val.as.string);
            return err;
        }
    } else {
        // Check if the name represents an existing array
        if (arr_exists(arr_ctx, var_name)) {
            is_array = true;
        }
    }

    // Expect comma
    BppToken comma = lex_peek(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ','"; return err;
    }
    lex_next(lex); // Consume ','

    // Parse find string
    BValue find_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (find_val.type != VAL_STRING) {
        err.code = 13; err.message = "Expected string argument to remove";
        return err;
    }

    HalContext *hal = hal_get();

    if (is_array) {
        // Iterate over array elements
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
                    while ((p = runtime_strstr(p, find)) != NULL) {
                        count++;
                        p += find_len;
                    }

                    if (count > 0) {
                        size_t new_len = orig_len - count * find_len;
                        char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(new_len + 1) : NULL;
                        if (!buf) {
                            err.code = 14; err.message = "Out of memory";
                            str_release(vm_get_str(vm), find_val.as.string);
                            return err;
                        }
                        runtime_memset(buf, 0, new_len + 1);

                        char *dst = buf;
                        const char *src = orig;
                        while (true) {
                            const char *next = runtime_strstr(src, find);
                            if (!next) {
                                runtime_strcpy(dst, src);
                                break;
                            }
                            size_t prefix_len = next - src;
                            runtime_memcpy(dst, src, prefix_len);
                            dst += prefix_len;
                            src = next + find_len;
                        }
                        elems[idx].as.string = str_create(vm_get_str(vm), buf, new_len);
                        if (hal && hal->mem.free) hal->mem.free(buf);
                        str_release(vm_get_str(vm), sr_orig);
                    }
                }
            }
        }
    } else {
        // Scalar string variable
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
            while ((p = runtime_strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len;
            char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(new_len + 1) : NULL;
            if (!buf) {
                err.code = 14; err.message = "Out of memory";
                str_release(vm_get_str(vm), find_val.as.string);
                return err;
            }
            runtime_memset(buf, 0, new_len + 1);

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = runtime_strstr(src, find);
                if (!next) {
                    runtime_strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                runtime_memcpy(dst, src, prefix_len);
                dst += prefix_len;
                src = next + find_len;
            }
            target->as.string = str_create(vm_get_str(vm), buf, new_len);
            if (hal && hal->mem.free) hal->mem.free(buf);
            str_release(vm_get_str(vm), sr_orig);
        }
    }

    str_release(vm_get_str(vm), find_val.as.string);
    return err;
}

BppError stmt_remove_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // Parse variable or array name
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable or array name in REMOVE";
        return err;
    }

    char var_name[64];
    size_t clen = (tok.length < 63) ? tok.length : 63;
    if (tok.start) {
        runtime_memcpy(var_name, tok.start, clen);
    }
    var_name[clen] = '\0';

    bool is_array = false;
    ArrayContext *arr_ctx = vm_get_arr(vm);

    // Check if it is explicitly followed by empty parens () or is an array
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_RPAREN) {
            lex_next(lex); // Consume ')'
            is_array = true;
        } else {
            // It has indices, meaning it is an element reference, e.g. arr(i, j)
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
            
            // Read next token which must be a comma
            BppToken comma = lex_peek(lex);
            if (comma.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ','"; return err;
            }
            lex_next(lex); // Consume ','

            // Parse find value
            BValue find_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (find_val.type == VAL_STRING) {
                err.code = 13; err.message = "Expected numeric argument to remove";
                return err;
            }

            // Resolve element
            BValue *target = arr_get_element(arr_ctx, var_name, num_indices, indices, &err);
            if (err.code != 0 || !target) return err;
            if (target->type == VAL_STRING) {
                err.code = 13; err.message = "Array element must be numeric";
                return err;
            }

            // Remove value (zero out if matches)
            if (target->as.number == find_val.as.number) {
                target->as.number = 0.0;
            }
            return err;
        }
    } else {
        // Check if the name represents an existing array
        if (arr_exists(arr_ctx, var_name)) {
            is_array = true;
        }
    }

    // Expect comma
    BppToken comma = lex_peek(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ','"; return err;
    }
    lex_next(lex); // Consume ','

    // Parse find value
    BValue find_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (find_val.type == VAL_STRING) {
        err.code = 13; err.message = "Expected numeric argument to remove";
        return err;
    }

    if (is_array) {
        // Iterate over array elements
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
        // Scalar numeric variable
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

/**
 * @file stmt_data.c
 * @brief DATA, READ, and RESTORE statements command handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements static data block statement handlers:
 *   - DATA: Declares embedded literal data values (NOP during sequential VM execution).
 *   - READ: Fetches the next static literal from the data table, assigns it to a variable/array, and increments the read pointer.
 *   - RESTORE: Resets the read pointer, optionally pointing to the first data item on or after a specified line.
 * - Why it exists: Provides classic static data serialization and retrieval.
 * - Why it works this way: It reads coordinates from a pre-constructed data items table in VMContext.
 *   This avoids complex scanning and parsing overheads during active execution, ensuring C-stack independence.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Unquoted string parsing rules, default data allocations.
 * - What cannot be changed: Memory clear guarantees on read pointers.
 * - What to expect: Reading beyond the last DATA item returns error code 4 (Out of DATA).
 * - What to do if something breaks: Trace vm_build_data_table scan results.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Literal data matches the variable type to prevent type mismatches.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Support structured data types (UDTs) or type conversions.
 * - How to write external extensions: Plugins access data items via VM table hooks.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "runtime/using.h"
#include "runtime/arrays.h"
#include <string.h>
#include <stdlib.h>

/* DATA handler (NOP during run) */
BppError stmt_data_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;

    /* Skip all tokens in this statement */
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }
    return err;
}

/* READ handler */
BppError stmt_read_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_using = false;
    char format_mask[128] = "";

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        BValue fmt_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fmt_val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: READ USING expects format string";
            return err;
        }
        strncpy(format_mask, str_data(fmt_val.as.string), sizeof(format_mask) - 1);
        str_release(vm_get_str(vm), fmt_val.as.string);
        is_using = true;

        /* Consume comma or semicolon */
        tok = lex_next(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_SEMICOLON) {
            err.code = 2; err.message = "Expected ',' or ';' after USING format string";
            return err;
        }
    }

    /* Build data table if it hasn't been built yet */
    if (vm_get_data_count(vm) == 0 && vm_get_data_items(vm) == NULL) {
        vm_build_data_table(vm);
    }

    while (true) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT) {
            err.code = 2; /* Syntax error */
            err.message = "Expected variable name in READ statement";
            return err;
        }

        char var_name[256];
        size_t clen = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        memcpy(var_name, var_tok.start, clen);
        var_name[clen] = '\0';

        /* Check for array subscript */
        bool is_array = false;
        int indices[4];
        int num_indices = 0;

        if (lex_peek(lex).type == TOK_LPAREN) {
            is_array = true;
            lex_next(lex); /* Consume '(' */
            while (true) {
                if (num_indices >= 4) {
                    err.code = 9;
                    err.message = "Too many dimensions for array in READ";
                    return err;
                }
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                indices[num_indices++] = (int)idx_val.as.number;

                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (next_tok.type == TOK_RPAREN) {
                    break;
                } else {
                    err.code = 2;
                    err.message = "Expected ',' or ')' in READ";
                    return err;
                }
            }
            lex_next(lex); /* Consume ')' */
        }

        /* Check for out of data */
        int data_ptr = vm_get_data_ptr(vm);
        int data_count = vm_get_data_count(vm);
        BppDataPosition *data_items = vm_get_data_items(vm);

        if (data_ptr >= data_count || !data_items) {
            err.code = 4; /* Out of DATA */
            err.message = "Out of DATA: READ statement has no more data items to read";
            return err;
        }

        BppDataPosition dp = data_items[data_ptr];
        vm_set_data_ptr(vm, data_ptr + 1);

        /* Parse literal */
        LexerContext *temp_lex = lex_init(vm_get_mem(vm), dp.pos);
        if (!temp_lex) {
            err.code = 14;
            err.message = "Out of memory in READ literal parsing";
            return err;
        }

        BppToken val_tok = lex_next(temp_lex);
        BValue val;
        val.type = VAL_NONE;
        val.as.number = 0.0;

        if (val_tok.type == TOK_NUMBER) {
            val.type = VAL_NUMBER;
            val.as.number = val_tok.as.number;
        } else if (val_tok.type == TOK_STRING) {
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), val_tok.as.string, val_tok.length);
        } else if (val_tok.type == TOK_IDENT) {
            /* Support unquoted strings */
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), val_tok.as.string, val_tok.length);
        } else {
            err.code = 2;
            err.message = "Syntax error in DATA literal";
            lex_shutdown(temp_lex);
            return err;
        }

        lex_shutdown(temp_lex);

        if (is_using && format_mask[0]) {
            char val_str[256];
            if (val.type == VAL_STRING) {
                strncpy(val_str, str_data(val.as.string), sizeof(val_str) - 1);
            } else {
                snprintf(val_str, sizeof(val_str), "%g", val.as.number);
            }
            char val_err[256] = "";
            if (!using_validate_input_string(val_str, format_mask, val_err, sizeof(val_err))) {
                err.code = 13;
                err.message = "READ USING validation failure";
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }
        }

        if (is_array) {
            BValue *elem = arr_get_element(vm_get_arr(vm), var_name, num_indices, indices, &err);
            if (err.code != 0 || !elem) {
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }

            /* Type conversion or compatibility check */
            if (elem->type != val.type) {
                if (elem->type == VAL_INTEGER && val.type == VAL_NUMBER) {
                    elem->as.number = (double)((int32_t)val.as.number);
                } else {
                    err.code = 13; /* Type mismatch */
                    err.message = "Type mismatch in READ array assignment";
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                    return err;
                }
            } else {
                if (elem->type == VAL_STRING && elem->as.string) {
                    str_release(vm_get_str(vm), elem->as.string);
                }
                *elem = val;
            }
        } else {
            VariableContext *var_ctx = vm_get_var(vm);
            if (!var_assign(var_ctx, var_name, val)) {
                err.code = 13;
                err.message = "Type mismatch in READ assignment";
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }
            if (val.type == VAL_STRING && val.as.string) {
                str_release(vm_get_str(vm), val.as.string);
            }
        }

        BppToken peek_tok = lex_peek(lex);
        if (peek_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    return err;
}

/* RESTORE handler */
BppError stmt_restore_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Build data table if it hasn't been built yet */
    if (vm_get_data_count(vm) == 0 && vm_get_data_items(vm) == NULL) {
        vm_build_data_table(vm);
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        lex_next(lex); /* Consume line number */
        BppLineNumber target = tok.as.number;

        int data_count = vm_get_data_count(vm);
        BppDataPosition *data_items = vm_get_data_items(vm);
        bool found = false;

        if (data_items) {
            for (int i = 0; i < data_count; ++i) {
                if (data_items[i].line >= target) {
                    vm_set_data_ptr(vm, i);
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            vm_set_data_ptr(vm, data_count);
        }
    } else {
        /* Reset to start */
        vm_set_data_ptr(vm, 0);
    }

    return err;
}

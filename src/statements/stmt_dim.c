/**
 * @file stmt_dim.c
 * @brief DIM (dimension) and ERASE statement command handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements statement handlers for array operations:
 *   - DIM: Allocates multi-dimensional arrays with specified upper bounds.
 *   - ERASE: Deallocates and erases arrays from the VM state.
 * - Why it exists: Provides dynamic array creation and deallocation capabilities.
 * - Why it works this way: It parses identifiers and specified index ranges,
 *   interacting directly with the ArrayContext of the VMContext. Multiple arrays
 *   can be declared or erased in a single statement via comma separation.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Default dimensions, OPTION BASE integration, dynamic error code mappings.
 * - What cannot be changed: Memory release guarantees on ERASE.
 * - What to expect: Declaring arrays already dimensioned returns duplicate definition error.
 * - What to do if something breaks: Trace dimension parser loops and comma checks.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Specified array bounds fit within standard integer limits.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add REDIM resizing capabilities.
 * - How to write external extensions: Plugins utilize arr_dim to create matrices dynamically.
 */

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_arrays.h"
#include "bpp_struct.h"
#include <string.h>

/* DIM statement handler */
BppError stmt_dim_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; /* Syntax error */
            err.message = "Expected array or variable name in DIM statement";
            return err;
        }

        char name[256];
        size_t copy_len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, copy_len);
        name[copy_len] = '\0';

        /* Expect '(' or check for scalar variable declaration */
        tok = lex_peek(lex);
        bool is_array = (tok.type == TOK_LPAREN);
        int bounds[4];
        int num_dims = 0;

        if (is_array) {
            lex_next(lex); /* Consume '(' */
            while (true) {
                if (num_dims >= 4) {
                    err.code = 9; /* Subscript out of range */
                    err.message = "Too many dimensions in DIM statement";
                    return err;
                }

                BValue val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;

                if (val.type == VAL_STRING) {
                    err.code = 13; /* Type mismatch */
                    err.message = "String values are not allowed as array bounds";
                    return err;
                }

                int bound = (int)val.as.number;
                if (bound < 0) {
                    err.code = 5; /* Illegal function call */
                    err.message = "Negative array bounds are not allowed";
                    return err;
                }

                bounds[num_dims++] = bound;

                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_COMMA) {
                    lex_next(lex); /* Consume ',' */
                } else if (next_tok.type == TOK_RPAREN) {
                    break;
                } else {
                    err.code = 2;
                    err.message = "Expected ',' or ')' in DIM statement";
                    return err;
                }
            }
            /* Consume ')' */
            lex_next(lex);

            /* Dimension array (initially as standard VAL_NUMBER) */
            err = arr_dim(vm_get_arr(vm), name, num_dims, bounds);
            if (err.code != 0) return err;
        } else {
            /* Declare scalar variable */
            BValue *var_val = var_declare(vm_get_var(vm), name);
            if (!var_val) {
                err.code = 14;
                err.message = "Out of memory declaring scalar variable";
                return err;
            }
        }

        /* Check optional AS TypeName */
        tok = lex_peek(lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_AS) {
            lex_next(lex); /* Consume 'AS' */
            BppToken type_tok = lex_next(lex);
            if (type_tok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected type name after AS";
                return err;
            }
            char type_str[64];
            size_t tlen = (type_tok.length < 63) ? type_tok.length : 63;
            memcpy(type_str, type_tok.start, tlen);
            type_str[tlen] = '\0';

            /* If it's primitive: AS INTEGER / LONG / SINGLE / DOUBLE / STRING */
            if (strcasecmp(type_str, "INTEGER") == 0 || strcasecmp(type_str, "LONG") == 0 ||
                strcasecmp(type_str, "SINGLE") == 0 || strcasecmp(type_str, "DOUBLE") == 0 ||
                strcasecmp(type_str, "NUMBER") == 0) {
                if (is_array) {
                    arr_set_type(vm_get_arr(vm), name, VAL_NUMBER);
                } else {
                    BValue val;
                    val.type = VAL_NUMBER;
                    val.as.number = 0.0;
                    var_assign(vm_get_var(vm), name, val);
                }
            } else if (strcasecmp(type_str, "STRING") == 0) {
                /* Check fixed string: * N */
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_MUL || (next_tok.type == TOK_UNKNOWN && *next_tok.start == '*')) {
                    lex_next(lex); /* Consume '*' */
                    BppToken num_tok = lex_next(lex);
                    if (num_tok.type != TOK_NUMBER) {
                        err.code = 2; err.message = "Expected length after STRING *";
                        return err;
                    }
                }
                if (is_array) {
                    arr_set_type(vm_get_arr(vm), name, VAL_STRING);
                    int total_size = 0;
                    BValue *elems = arr_get_flat_elements(vm_get_arr(vm), name, &total_size);
                    if (elems) {
                        for (int i = 0; i < total_size; ++i) {
                            elems[i].type = VAL_STRING;
                            elems[i].as.string = str_create(vm_get_str(vm), "", 0);
                        }
                    }
                } else {
                    BValue val;
                    val.type = VAL_STRING;
                    val.as.string = str_create(vm_get_str(vm), "", 0);
                    var_assign(vm_get_var(vm), name, val);
                }
            } else {
                /* Custom Type instantiation */
                const BppUserTypeDef *nested_def = struct_find_type(vm_get_types(vm), type_str);
                if (!nested_def) {
                    err.code = 35; err.message = "Type not defined";
                    return err;
                }
                if (is_array) {
                    arr_set_type(vm_get_arr(vm), name, VAL_MAP);
                    int total_size = 0;
                    BValue *elems = arr_get_flat_elements(vm_get_arr(vm), name, &total_size);
                    if (elems) {
                        for (int i = 0; i < total_size; ++i) {
                            char reg_err[128];
                            BppMap *inst = struct_instantiate(vm, vm_get_types(vm), type_str, reg_err, sizeof(reg_err));
                            if (!inst) {
                                err.code = 35; err.message = "Failed to instantiate struct element";
                                return err;
                            }
                            elems[i].type = VAL_MAP;
                            elems[i].as.map = inst;
                        }
                    }
                } else {
                    char reg_err[128];
                    BppMap *inst = struct_instantiate(vm, vm_get_types(vm), type_str, reg_err, sizeof(reg_err));
                    if (!inst) {
                        err.code = 35; err.message = "Failed to instantiate struct";
                        return err;
                    }
                    BValue val;
                    val.type = VAL_MAP;
                    val.as.map = inst;
                    var_assign(vm_get_var(vm), name, val);
                }
            }
        }

        /* Check for more arrays/variables separated by commas */
        BppToken peek_tok = lex_peek(lex);
        if (peek_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    return err;
}

/* ERASE statement handler */
BppError stmt_erase_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected array name in ERASE statement";
            return err;
        }

        char name[256];
        size_t copy_len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, copy_len);
        name[copy_len] = '\0';

        if (!arr_erase(vm_get_arr(vm), name)) {
            err.code = 9; /* Subscript out of range / not dimensioned */
            err.message = "Array not dimensioned or already erased";
            return err;
        }

        /* Check for more arrays separated by commas */
        BppToken peek_tok = lex_peek(lex);
        if (peek_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_shared_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in SHARED";
            return err;
        }

        char name[256];
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);
        name[len] = '\0';

        var_set_shared(vm_get_var(vm), name);

        BppToken peek_tok = lex_peek(lex);
        if (peek_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_local_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in LOCAL/STATIC";
            return err;
        }
        char var_name[64];
        size_t clen = (tok.length < 63) ? tok.length : 63;
        memcpy(var_name, tok.start, clen);
        var_name[clen] = '\0';
        
        var_declare(vm_get_var(vm), var_name);
        
        BppToken next = lex_peek(lex);
        if (next.type == TOK_COMMA) lex_next(lex);
        else break;
    }
    return err;
}

BppError stmt_static_handler(VMContext *vm, LexerContext *lex) {
    return stmt_local_handler(vm, lex);
}

BppError stmt_redim_handler(VMContext *vm, LexerContext *lex) {
    BppToken ntok = lex_peek(lex);
    if ((ntok.type == TOK_IDENT && ntok.length == 8 && strncmp(ntok.start, "PRESERVE", 8) == 0) ||
        (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_PRESERVE)) {
        lex_next(lex);
    }
    return stmt_dim_handler(vm, lex);
}

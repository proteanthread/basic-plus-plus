// FILENAME: dim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (redim.c, vdim.c)
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (struct.h, struct.c, variables.h, variables.c)
// NEEDS: libengine (dim.h, eval.h, eval.c, lexer.h, lexer.c, map.h, map.c)
// NEEDS: libengine (string.c, vm.h)
// Provides runtime implementation for the DIM statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/dim.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/variables.h"
#include "core/struct.h"
#include "runtime/map.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_dim_desc = {
    .name = "DIM",
    .category = "Variables & Memory",
    .syntax = "DIM [#channel,] [SHARED] [DYNAMIC | STATIC] array_name(subscripts...) [*len] [AS type [*len]] [, ...]",
    .description = "Allocates storage space for arrays, virtual arrays, fixed strings, and class instances.",
    .error_summary = "Error 2: Syntax Error, Error 9: Subscript out of range, Error 10: Duplicate definition",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

#ifdef _WIN32
#define runtime_strcasecmp runtime_strcasecmp
#endif

void stmt_dim_register(void) {
    lang_desc_register(&g_dim_desc);
}

BppError stmt_dim_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5;
        err.message = "Illegal function call";
        return err;
    }

    ArrayContext *arr_ctx = vm_get_arr(vm);

    // Check for optional DEC / Timeshare virtual array channel syntax: DIM #channel, ...
    int channel = -1;
    BppToken ch_tok = lex_peek(lex);
    if (ch_tok.type == TOK_HASH || (ch_tok.start && ch_tok.length == 1 && ch_tok.start[0] == '#')) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        channel = (int)ch_val.as.number;
        BppToken comma = lex_peek(lex);
        if (comma.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        }
    }

    while (1) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        // Check for optional modifiers like SHARED, DYNAMIC, STATIC
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_SHARED) ||
            (tok.type == TOK_IDENT && tok.length == 6 && runtime_strncasecmp(tok.start, "SHARED", 6) == 0)) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_IDENT && tok.length == 7 && runtime_strncasecmp(tok.start, "DYNAMIC", 7) == 0) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_IDENT && tok.length == 6 && runtime_strncasecmp(tok.start, "STATIC", 6) == 0) {
            lex_next(lex);
            tok = lex_peek(lex);
        }

        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected array or variable name in DIM";
            return err;
        }

        BppToken name_tok = lex_next(lex);
        char arr_name[64];
        size_t copy_len = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        runtime_memcpy(arr_name, name_tok.start, copy_len);
        arr_name[copy_len] = '\0';

        bool is_array = false;
        size_t fixed_str_len = 0;
        tok = lex_peek(lex);
        if (tok.type == TOK_LPAREN) {
            is_array = true;
            lex_next(lex); // Consume '('
            int dims[8];
            int dim_count = 0;

            while (dim_count < 8) {
                BValue bval = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;

                // Check for optional 'TO' e.g. 1 TO 10
                BppToken maybe_to = lex_peek(lex);
                if ((maybe_to.type == TOK_KEYWORD && maybe_to.as.keyword == KW_TO) ||
                    (maybe_to.type == TOK_IDENT && maybe_to.length == 2 && runtime_strncasecmp(maybe_to.start, "TO", 2) == 0)) {
                    lex_next(lex); // Consume 'TO'
                    BValue upper_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    dims[dim_count++] = (int)upper_val.as.number;
                } else {
                    dims[dim_count++] = (int)bval.as.number;
                }

                BppToken sep = lex_peek(lex);
                if (sep.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (sep.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2;
                    err.message = "Expected ',' or ')' in array dimension";
                    return err;
                }
            }

            if (arr_ctx) {
                BppError dim_err;
                if (channel >= 0) {
                    dim_err = arr_dim_virtual(arr_ctx, arr_name, dim_count, dims, channel);
                } else {
                    dim_err = arr_dim(arr_ctx, arr_name, dim_count, dims);
                }
                if (dim_err.code != 0) {
                    return dim_err;
                }
            }
        } else if (tok.type == TOK_LBRACKET) {
            // Sized string modifier: DIM S$[len]
            lex_next(lex); // Consume '['
            BValue len_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            fixed_str_len = (size_t)len_val.as.number;
            BppToken rbrk = lex_peek(lex);
            if (rbrk.type == TOK_RBRACKET) {
                lex_next(lex); // Consume ']'
            } else {
                err.code = 2;
                err.message = "Expected ']' in string sizing";
                return err;
            }
        } else if (tok.type == TOK_EQ) {
            // Sized string assignment syntax: DIM S$ = len
            lex_next(lex); // Consume '='
            BValue len_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            fixed_str_len = (size_t)len_val.as.number;
        }

        // Check for optional fixed-length sizing e.g. DIM A$(10) * 20 or DIM S$ * 80
        tok = lex_peek(lex);
        if (tok.type == TOK_MUL || (tok.start && tok.length == 1 && tok.start[0] == '*')) {
            lex_next(lex); // Consume '*'
            BValue len_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            fixed_str_len = (size_t)len_val.as.number;
        }

        // Check for optional AS <type> e.g. DIM f1 AS Form or DIM S AS STRING * 40
        tok = lex_peek(lex);
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_AS) ||
            (tok.type == TOK_IDENT && tok.length == 2 && runtime_strncasecmp(tok.start, "AS", 2) == 0)) {
            lex_next(lex); // Consume AS
            BppToken type_tok = lex_next(lex); // Consume type keyword or ident
            char type_str[64] = {0};
            size_t tlen = (type_tok.length < sizeof(type_str) - 1) ? type_tok.length : sizeof(type_str) - 1;
            runtime_memcpy(type_str, type_tok.start, tlen);
            type_str[tlen] = '\0';

            // Check for optional * length, = length, or [length] after AS STRING
            BppToken star_tok = lex_peek(lex);
            if (star_tok.type == TOK_MUL || (star_tok.start && star_tok.length == 1 && star_tok.start[0] == '*')) {
                lex_next(lex); // Consume '*'
                BValue len_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                fixed_str_len = (size_t)len_val.as.number;
            } else if (star_tok.type == TOK_EQ) {
                lex_next(lex); // Consume '='
                BValue len_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                fixed_str_len = (size_t)len_val.as.number;
            } else if (star_tok.type == TOK_LBRACKET) {
                lex_next(lex); // Consume '['
                BValue len_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                fixed_str_len = (size_t)len_val.as.number;
                BppToken rbrk = lex_peek(lex);
                if (rbrk.type == TOK_RBRACKET) {
                    lex_next(lex);
                } else {
                    err.code = 2;
                    err.message = "Expected ']' in string sizing";
                    return err;
                }
            }

            // If scalar (not an array), instantiate class/struct or declare variable
            if (!is_array) {
                const BppUserTypeDef *udt = struct_find_type(vm_get_types(vm), type_str);
                if (udt) {
                    char err_buf[128] = {0};
                    BppMap *inst = struct_instantiate(vm, vm_get_types(vm), type_str, err_buf, sizeof(err_buf));
                    if (inst) {
                        BValue map_val;
                        map_val.type = VAL_MAP;
                        map_val.as.map = inst;
                        var_assign(vm_get_var(vm), arr_name, map_val);
                        map_release(vm_get_str(vm), inst);
                    }
                } else if (runtime_strcasecmp(type_str, "STRING") == 0) {
                    BValue s_val;
                    s_val.type = VAL_STRING;
                    s_val.as.string = str_create(vm_get_str(vm), "", 0);
                    var_assign(vm_get_var(vm), arr_name, s_val);
                    str_release(vm_get_str(vm), s_val.as.string);
                    if (fixed_str_len > 0) {
                        var_set_max_len(vm_get_var(vm), arr_name, fixed_str_len);
                    }
                } else {
                    var_declare(vm_get_var(vm), arr_name);
                }
            }
        } else if (!is_array) {
            // Scalar declaration without AS type
            var_declare(vm_get_var(vm), arr_name);
            if (fixed_str_len > 0) {
                var_set_max_len(vm_get_var(vm), arr_name, fixed_str_len);
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ',' for next array/variable in list
        } else {
            break;
        }
    }

    return err;
}

/*
 * Copyright (c) 2025 Basic++ Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file eval_rpn.c
 * @brief RPN Expression Evaluation
 */

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "runtime/file.h"
#include "runtime/funcreg.h"
#include "core/struct.h"
#include "core/dialect.h"
#include "module/module.h"

BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    memset(&null_val, 0, sizeof(null_val));

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* Allocate stacks from scratch arena */
    BValue *val_stack = (BValue *)mem_scratch_alloc(mem, sizeof(BValue) * MAX_EVAL_DEPTH);
    if (!val_stack) {
        out_err->code = 14;
        out_err->message = "Evaluation stack overflow (scratch exhausted)";
        return null_val;
    }

    size_t val_ptr = 0;
    int open_parens = 0;
    BppToken tok = lex_peek(lex);

    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA &&
           tok.type != TOK_SEMICOLON && (tok.type != TOK_RPAREN || open_parens > 0) &&
           tok.type != TOK_RBRACKET &&
           (tok.type != TOK_KEYWORD || tok.as.keyword == KW_NONE ||
            tok.as.keyword == KW_TASK || tok.as.keyword == KW_PLAY || tok.as.keyword == KW_HELP ||
            tok.as.keyword == KW_SCREEN || tok.as.keyword == KW_SEEK ||
            tok.as.keyword == KW_TIMER || tok.as.keyword == KW_KEY ||
            tok.as.keyword == KW_REMOVE || tok.as.keyword == KW_REMOVE_STR ||
            tok.as.keyword == KW_ALARM || tok.as.keyword == KW_ALARM_STR ||
            tok.as.keyword == KW_RANDOMIZE)) {

        /* Stop parsing if we see 'AT' identifier */
        if (tok.type == TOK_IDENT && tok.length == 2 &&
            (tok.start[0] == 'A' || tok.start[0] == 'a') &&
            (tok.start[1] == 'T' || tok.start[1] == 't')) {
            break;
        }

        /* Read the peeked token */
        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_STRING) {
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_RPN_LITERAL) {
            char *rpn_str = (char *)mem_scratch_alloc(mem, tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14;
                out_err->message = "Scratch memory exhausted";
                return null_val;
            }
            memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(mem, rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) return null_val;

            val_stack[val_ptr++] = res;
        } else if (tok.type == TOK_IDENT) {
            /* Variable or function lookup */
            char name_buf[256];
            size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
            memcpy(name_buf, tok.start, copy_len);
            name_buf[copy_len] = '\0';

            if (strcasecmp(name_buf, "DUP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DUP";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 1];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "DROP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DROP";
                    return null_val;
                }
                val_ptr--;
                BValue val = val_stack[val_ptr];
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
            } else if (strcasecmp(name_buf, "SWAP") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on SWAP";
                    return null_val;
                }
                BValue temp = val_stack[val_ptr - 1];
                val_stack[val_ptr - 1] = val_stack[val_ptr - 2];
                val_stack[val_ptr - 2] = temp;
            } else if (strcasecmp(name_buf, "OVER") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on OVER";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 2];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "ROT") == 0) {
                if (val_ptr < 3) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROT";
                    return null_val;
                }
                BValue a = val_stack[val_ptr - 3];
                BValue b = val_stack[val_ptr - 2];
                BValue c = val_stack[val_ptr - 1];
                val_stack[val_ptr - 3] = b;
                val_stack[val_ptr - 2] = c;
                val_stack[val_ptr - 1] = a;
            } else if (strcasecmp(name_buf, "CLEAR") == 0) {
                while (val_ptr > 0) {
                    val_ptr--;
                    BValue val = val_stack[val_ptr];
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                }
            } else if (strcasecmp(name_buf, "DEPTH") == 0) {
                BValue val;
                val.type = VAL_NUMBER;
                val.as.number = (double)val_ptr;
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "PICK") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on PICK";
                    return null_val;
                }
                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "PICK index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "PICK index out of bounds";
                    return null_val;
                }
                BValue picked = val_stack[val_ptr - 1 - n];
                if (picked.type == VAL_STRING && picked.as.string) {
                    str_add_ref(picked.as.string);
                }
                val_stack[val_ptr++] = picked;
            } else if (strcasecmp(name_buf, "ROLL") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROLL";
                    return null_val;
                }
                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "ROLL index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "ROLL index out of bounds";
                    return null_val;
                }
                if (n > 0) {
                    BValue rolled = val_stack[(int)val_ptr - 1 - n];
                    for (int i = (int)val_ptr - 1 - n; i < (int)val_ptr - 1; ++i) {
                        val_stack[i] = val_stack[i + 1];
                    }
                    val_stack[(int)val_ptr - 1] = rolled;
                }
            } else {
                /* Check if followed by '(' */
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); /* Consume '(' */
                    if (eval_is_builtin_function(name_buf)) {
                        BValue val = eval_builtin_function(vm, name_buf, lex, true, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else if (!arr_exists(vm_get_arr(vm), name_buf)) {
                        BValue args[8];
                        int argc = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (argc >= 8) {
                                for (int ci = 0; ci < argc; ci++) {
                                    if (args[ci].type == VAL_STRING && args[ci].as.string)
                                        str_release(vm_get_str(vm), args[ci].as.string);
                                }
                                out_err->code = 2;
                                out_err->message = "Too many arguments in function call";
                                return null_val;
                            }
                            args[argc++] = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) {
                                for (int ci = 0; ci < argc; ci++) {
                                    if (args[ci].type == VAL_STRING && args[ci].as.string)
                                        str_release(vm_get_str(vm), args[ci].as.string);
                                }
                                return null_val;
                            }
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue val = invoke_user_function(vm, name_buf, args, argc, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else {
                        /* Array access */
                        int indices[4];
                        int num_indices = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (num_indices >= 4) {
                                out_err->code = 9;
                                out_err->message = "Subscript out of range (max 4 dimensions)";
                                return null_val;
                            }
                            BValue idx_val = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) return null_val;
                            indices[num_indices++] = (int)idx_val.as.number;
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue *elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, indices, out_err);
                        if (out_err->code != 0 || !elem) {
                            return null_val;
                        }
                        BValue val = *elem;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                } else {
                    /* Simple variable lookup */
                    BValue *v = var_lookup(var, name_buf, false);
                    if (!v) {
                        char base_name[256];
                        char member_chain[8][64];
                        int member_count = 0;
                        eval_split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                        if (member_count > 0) {
                            v = var_lookup(var, base_name, false);
                            if (v) {
                                BValue val = *v;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
                                
                                /* Walk up to the last member */
                                bool walk_err = false;
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        walk_err = true; break;
                                    }
                                    BValue next_val;
                                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                                        walk_err = true; break;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                if (!walk_err) {
                                    /* Check if followed by '(' -> Method call */
                                    if (lex_peek(lex).type == TOK_LPAREN) {
                                        lex_next(lex); /* Consume '(' */
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue type_val;
                                            if (map_get(val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                                                char fully_qualified_method[512];
                                                snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                         str_data(type_val.as.string), member_chain[member_count - 1]);
                                                
                                                BValue args[9];
                                                int argc = 0;
                                                args[argc++] = val;
                                                map_add_ref(val.as.map);
                                                
                                                while (true) {
                                                    BppToken next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    }
                                                    if (argc >= 9) {
                                                        walk_err = true; break;
                                                    }
                                                    args[argc++] = eval_expression(vm, lex, out_err);
                                                    if (out_err->code != 0) {
                                                        walk_err = true; break;
                                                    }
                                                    next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_COMMA) {
                                                        lex_next(lex);
                                                    } else if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    } else {
                                                        walk_err = true; break;
                                                    }
                                                }
                                                
                                                if (!walk_err) {
                                                    BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                                                    for (int i = 0; i < argc; i++) {
                                                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                                    }
                                                    if (out_err->code == 0) {
                                                        val_stack[val_ptr++] = ret_val;
                                                        tok = lex_peek(lex);
                                                        continue;
                                                    }
                                                } else {
                                                    /* Cleanup leaked args on walk_err */
                                                    for (int i = 0; i < argc; i++) {
                                                        if (args[i].type == VAL_STRING && args[i].as.string) str_release(vm_get_str(vm), args[i].as.string);
                                                        else if (args[i].type == VAL_MAP && args[i].as.map) map_release(vm_get_str(vm), args[i].as.map);
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        /* Standard field lookup on last member */
                                        int m = member_count - 1;
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue next_val;
                                            if (map_get(val.as.map, member_chain[m], &next_val)) {
                                                BValue copy = next_val;
                                                if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                                else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                                
                                                map_release(vm_get_str(vm), val.as.map);
                                                val_stack[val_ptr++] = copy;
                                                tok = lex_peek(lex);
                                                continue;
                                            }
                                        }
                                    }
                                }
                                
                                if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                            }
                        }
                        
                        /* Return uninitialized variable (numeric 0.0) */
                        BValue val;
                        val.type = VAL_NUMBER;
                        val.as.number = 0.0;
                        val_stack[val_ptr++] = val;
                    } else {
                        BValue val = *v;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                }
            }
        } else if (eval_is_operator(tok.type)) {
            BppTokenType op = tok.type;
            if (val_ptr == 1 && op == TOK_MINUS) {
                op = TOK_UNARY_MINUS;
            }
            if (!eval_execute_op(vm, op, val_stack, &val_ptr, out_err)) {
                return null_val;
            }
        } else if (tok.type == TOK_LPAREN) {
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            open_parens--;
        } else {
            out_err->code = 2;
            out_err->message = "Unexpected token in RPN expression";
            return null_val;
        }

        tok = lex_peek(lex);
    }

    if (val_ptr == 0) {
        BValue zero;
        zero.type = VAL_NUMBER;
        zero.as.number = 0.0;
        return zero;
    }

    return val_stack[val_ptr - 1];
}

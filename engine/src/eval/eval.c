/**
 * @file eval.c
 * @brief Master non-recursive Shunting-Yard expression evaluator for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `eval_expression()`, consuming tokens, managing value/operator stacks, evaluating operators (+, -, *, /, ^, MOD, AND, OR, XOR, NOT, relational), array subscripts, string slicing, and function calls.
 *
 * 2. WHY IT EXISTS:
 * Serves as the core expression evaluation engine for all BASIC statement handlers and VM execution steps without host stack recursion per Rule #2.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Utilizes iterative Shunting-Yard algorithm with value and operator stacks; enforces evaluation depth limit (64) to prevent host stack overflow; returns tagged `BValue` structs with refcounted strings per Rule #3.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'eval'. Includes "eval/eval_internal.h", "runtime/variables.h",
 * "runtime/map.h", "core/struct.h", "runtime/funcreg.h", "runtime/file.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new operators or custom operator precedence levels in Shunting-Yard precedence tables.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Refcount ownership invariant: Every `BValue` containing `VAL_STRING` returned by `eval_expression()` holds an INCREMENTED refcount; caller MUST call `str_release()` (Rule #3). Relational truth values MUST evaluate to -1.0 (true) or 0.0 (false).
 *
 * 8. WHAT TO EXPECT:
 * Returns BValue result (VAL_NUMBER, VAL_STRING, VAL_ERROR) and sets `out_err`.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Trace stack push/pop balance and check operator precedence handling in precedence table.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext, LexerContext, and out_err pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Floating-point comparisons and IEEE 754 float math.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/dispatch.c
 * - engine/src/runtime/variables.c
 * - engine/src/core/string.c
 * Prerequisite Header Files:
 * - engine/include/eval/eval.h
 * - engine/include/eval/dispatch.h
 * - engine/include/runtime/variables.h
 */

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "core/struct.h"
#include "runtime/funcreg.h"
#include "runtime/file.h"

BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    memset(&null_val, 0, sizeof(null_val));
    #define EVAL_EARLY_RETURN do { vm_dec_eval_depth(vm); return null_val; } while(0)
    #define VAL_PUSH(v) do { if (val_ptr >= MAX_EVAL_DEPTH) { out_err->code = 14; out_err->message = "Evaluation stack overflow"; EVAL_EARLY_RETURN; } val_stack[val_ptr++] = (v); } while(0)
    #define OP_PUSH(o) do { if (op_ptr >= MAX_EVAL_DEPTH) { out_err->code = 14; out_err->message = "Operator stack overflow"; EVAL_EARLY_RETURN; } op_stack[op_ptr++] = (o); } while(0)

    /* Guard against C-stack overflow from deeply nested expression evaluation.
     * eval_expression calls itself recursively for function arguments, array
     * subscripts, and string slices. Without a guard, pathological input like
     * deeply nested function calls could overflow the host C stack. */
    vm_inc_eval_depth(vm);
    if (vm_get_eval_depth(vm) > 64) {
        vm_dec_eval_depth(vm);
        out_err->code = 14;
        out_err->message = "Expression nesting too deep (limit 64)";
        return null_val;
    }

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* Allocate Shunting-Yard stacks from scratch arena */
    BValue *val_stack = (BValue *)mem_scratch_alloc(mem, sizeof(BValue) * MAX_EVAL_DEPTH);
    BppTokenType *op_stack = (BppTokenType *)mem_scratch_alloc(mem, sizeof(BppTokenType) * MAX_EVAL_DEPTH);

    if (!val_stack || !op_stack) {
        out_err->code = 14; /* Out of memory */
        out_err->message = "Evaluation stack overflow (scratch exhausted)";
        EVAL_EARLY_RETURN;
    }

    size_t val_ptr = 0;
    size_t op_ptr = 0;
    int open_parens = 0;

    bool expect_operand = true;
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

        /* Implied semicolon check: if we are expecting an operator, but see an operand */
        if (!expect_operand) {
            if (tok.type == TOK_NUMBER || tok.type == TOK_STRING || tok.type == TOK_RPN_LITERAL ||
                tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_LPAREN) {
                break;
            }
        }

        /* Read the peeked token */
        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got number";
                EVAL_EARLY_RETURN;
            }
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_STRING) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got string";
                EVAL_EARLY_RETURN;
            }
            /* Create string handle */
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_RPN_LITERAL) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got RPN literal";
                EVAL_EARLY_RETURN;
            }
            char *rpn_str = (char *)mem_scratch_alloc(vm_get_mem(vm), tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14;
                out_err->message = "Scratch memory exhausted";
                EVAL_EARLY_RETURN;
            }
            memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(vm_get_mem(vm), rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) EVAL_EARLY_RETURN;

            VAL_PUSH(res);
            expect_operand = false;
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_PERIOD) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got variable, keyword or '.'";
                EVAL_EARLY_RETURN;
            }
            /* Variable or function lookup */
            char name_buf[256];
            if (tok.type == TOK_PERIOD) {
                const char *with_prefix = vm_with_stack_peek(vm);
                if (!with_prefix) {
                    out_err->code = 2; out_err->message = "Leading '.' outside of WITH block";
                    EVAL_EARLY_RETURN;
                }
                BppToken sub_tok = lex_next(lex);
                if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
                    out_err->code = 2; out_err->message = "Expected identifier after '.' in WITH member access";
                    EVAL_EARLY_RETURN;
                }
                snprintf(name_buf, sizeof(name_buf), "%s.%.*s", with_prefix, (int)sub_tok.length, sub_tok.start);
            } else {
                size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
                memcpy(name_buf, tok.start, copy_len);
                name_buf[copy_len] = '\0';
            }

            /* Check if namespace prefix (e.g. bits.xxx or math.xxx) */
            if (lex_peek(lex).type == TOK_PERIOD &&
                (strcasecmp(name_buf, "bits") == 0 || strcasecmp(name_buf, "math") == 0 ||
                 strcasecmp(name_buf, "sound") == 0 || strcasecmp(name_buf, "music") == 0 ||
                 strcasecmp(name_buf, "mouse") == 0 || strcasecmp(name_buf, "joystick") == 0 ||
                 strcasecmp(name_buf, "input") == 0 || strcasecmp(name_buf, "window") == 0)) {
                lex_next(lex); /* Consume '.' */
                BppToken sub_tok = lex_next(lex);
                if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
                    out_err->code = 2; out_err->message = "Expected identifier after '.' in namespace call";
                    EVAL_EARLY_RETURN;
                }
                char sub_name[128];
                size_t sub_len = (sub_tok.length < sizeof(sub_name) - 1) ? sub_tok.length : sizeof(sub_name) - 1;
                memcpy(sub_name, sub_tok.start, sub_len);
                sub_name[sub_len] = '\0';
                
                char combined[512];
                snprintf(combined, sizeof(combined), "%s.%s", name_buf, sub_name);
                strncpy(name_buf, combined, sizeof(name_buf) - 1);
                name_buf[sizeof(name_buf) - 1] = '\0';
            }

            /* If name_buf refers to a VAL_ARRAY_REF (e.g., parameter passed by reference), resolve it to the original array name */
            BValue *ref_var = var_lookup(var, name_buf, false);
            if (ref_var && ref_var->type == VAL_ARRAY_REF && ref_var->as.string) {
                const char *orig_name = str_data(ref_var->as.string);
                size_t olen = strlen(orig_name);
                if (olen < sizeof(name_buf) - 1) {
                    memcpy(name_buf, orig_name, olen);
                    name_buf[olen] = '\0';
                }
            }

            bool is_func = false;
            /* Check if followed by '(' or '[' */
            BppTokenType next_tok_type = lex_peek(lex).type;
            if (next_tok_type == TOK_LBRACKET) {
                is_func = true;
                BValue val = eval_parse_string_slice(vm, lex, name_buf, TOK_LBRACKET, out_err);
                if (out_err->code != 0) EVAL_EARLY_RETURN;
                VAL_PUSH(val);
                expect_operand = false;
            } else if (next_tok_type == TOK_LPAREN) {
                /* Disambiguate Sinclair string slicing A$(start TO end) from function calls / array accesses */
                bool is_slicing = false;
                if (name_buf[strlen(name_buf) - 1] == '$' && strchr(name_buf, '.') == NULL) {
                    if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL) &&
                        !eval_is_builtin_function(name_buf) &&
                        !arr_exists(vm_get_arr(vm), name_buf)) {
                        is_slicing = true;
                    }
                }
                if (is_slicing) {
                    is_func = true;
                    BValue val = eval_parse_string_slice(vm, lex, name_buf, TOK_LPAREN, out_err);
                    if (out_err->code != 0) EVAL_EARLY_RETURN;
                    VAL_PUSH(val);
                    expect_operand = false;
                } else if (eval_is_builtin_function(name_buf)) {
                    is_func = true;
                    lex_next(lex); /* Consume '(' */
                    BValue val = eval_builtin_function(vm, name_buf, lex, true, out_err);
                    if (out_err->code != 0) EVAL_EARLY_RETURN;
                    VAL_PUSH(val);
                    expect_operand = false;
                } else if (!arr_exists(vm_get_arr(vm), name_buf) &&
                           !(var_lookup(var, name_buf, false) && var_lookup(var, name_buf, false)->type == VAL_ARRAY_REF && var_lookup(var, name_buf, false)->as.string)) {
                    is_func = true;
                    lex_next(lex); /* Consume '(' */

                    /* Check if name_buf contains '.' -> Method call check */
                    bool is_method = false;
                    char base_name[256] = "";
                    char member_chain[8][64];
                    int member_count = 0;
                    char fully_qualified_method[512] = "";
                    BValue obj_val;
                    memset(&obj_val, 0, sizeof(obj_val));

                    if (strchr(name_buf, '.') != NULL) {
                        if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL)) {
                            /* Not a global namespaced function: try to resolve as method call */
                            eval_split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                            if (member_count > 0) {
                                BValue *var_val = var_lookup(var, base_name, false);
                                if (var_val) {
                                    obj_val = *var_val;
                                    if (obj_val.type == VAL_STRING && obj_val.as.string) str_add_ref(obj_val.as.string);
                                    else if (obj_val.type == VAL_MAP && obj_val.as.map) map_add_ref(obj_val.as.map);
                                    
                                    /* Walk nested fields up to last member */
                                    bool walk_err = false;
                                    for (int m = 0; m < member_count - 1; m++) {
                                        if (obj_val.type != VAL_MAP || !obj_val.as.map) {
                                            walk_err = true; break;
                                        }
                                        BValue next_val;
                                        if (!map_get(obj_val.as.map, member_chain[m], &next_val)) {
                                            walk_err = true; break;
                                        }
                                        BValue copy = next_val;
                                        if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                        else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                        
                                        if (obj_val.type == VAL_MAP && obj_val.as.map) map_release(vm_get_str(vm), obj_val.as.map);
                                        else if (obj_val.type == VAL_STRING && obj_val.as.string) str_release(vm_get_str(vm), obj_val.as.string);
                                        obj_val = copy;
                                    }
                                    
                                    if (!walk_err && obj_val.type == VAL_MAP && obj_val.as.map) {
                                        BValue type_val;
                                        if (map_get(obj_val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                                            snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                     str_data(type_val.as.string), member_chain[member_count - 1]);
                                            is_method = true;
                                        }
                                    }
                                    if (!is_method) {
                                        if (obj_val.type == VAL_MAP && obj_val.as.map) map_release(vm_get_str(vm), obj_val.as.map);
                                        else if (obj_val.type == VAL_STRING && obj_val.as.string) str_release(vm_get_str(vm), obj_val.as.string);
                                    }
                                }
                            }
                        }
                    }

                    BValue args[9];
                    int argc = 0;
                    if (is_method) {
                        args[argc++] = obj_val; /* Implicit THIS */
                    }

                    while (true) {
                        BppToken next_tok = lex_peek(lex);
                        if (next_tok.type == TOK_RPAREN) {
                            lex_next(lex);
                            break;
                        }

                        if (argc >= 9) {
                            out_err->code = 2;
                            out_err->message = "Too many arguments in function/method call";
                            for (int i = 0; i < argc; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            EVAL_EARLY_RETURN;
                        }

                        args[argc++] = eval_expression(vm, lex, out_err);
                        if (out_err->code != 0) {
                            for (int i = 0; i < argc - 1; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            EVAL_EARLY_RETURN;
                        }

                        next_tok = lex_peek(lex);
                        if (next_tok.type == TOK_COMMA) {
                            lex_next(lex);
                        } else if (next_tok.type == TOK_RPAREN) {
                            lex_next(lex);
                            break;
                        } else {
                            out_err->code = 2;
                            out_err->message = "Expected ',' or ')' in function argument list";
                            for (int i = 0; i < argc; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            EVAL_EARLY_RETURN;
                        }
                    }

                    BValue val = invoke_user_function(vm, is_method ? fully_qualified_method : name_buf, args, argc, out_err);
                    for (int i = 0; i < argc; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                    }
                    if (out_err->code != 0) EVAL_EARLY_RETURN;

                    VAL_PUSH(val);
                    expect_operand = false;
                } else {
                    /* Array access! E.g. A(1, 2) or A(1 TO 5, *) */
                    if (!arr_exists(vm_get_arr(vm), name_buf)) {
                        BValue *var_val = var_lookup(var, name_buf, false);
                        if (var_val && var_val->type == VAL_ARRAY_REF && var_val->as.string) {
                            strncpy(name_buf, str_data(var_val->as.string), 256 - 1);
                            name_buf[256 - 1] = '\0';
                        }
                    }
                    lex_next(lex); /* Consume '(' */

                    typedef struct { int is_slice; int start; int end; } SliceDim;
                    SliceDim slices[4] = {0};
                    int num_indices = 0;
                    bool has_slice = false;

                    if (lex_peek(lex).type == TOK_RPAREN) {
                        lex_next(lex); /* Consume ')' */
                        BValue val;
                        val.type = VAL_ARRAY_REF;
                        val.as.string = str_create(vm_get_str(vm), name_buf, strlen(name_buf));
                        VAL_PUSH(val);
                        expect_operand = false;
                        is_func = true;
                    } else {
                        while (true) {
                            if (num_indices >= 4) {
                                out_err->code = 9; out_err->message = "Too many dimensions for array access";
                                EVAL_EARLY_RETURN;
                            }
                            
                            if (lex_peek(lex).type == TOK_MUL) {
                                lex_next(lex);
                                slices[num_indices].is_slice = 1;
                                slices[num_indices].start = arr_get_option_base(vm_get_arr(vm));
                                bool found = false;
                                slices[num_indices].end = arr_ubound(vm_get_arr(vm), name_buf, num_indices + 1, &found);
                                if (!found) {
                                    out_err->code = 9; out_err->message = "Array dimension not found";
                                    EVAL_EARLY_RETURN;
                                }
                                has_slice = true;
                            } else {
                                BValue idx_val = eval_expression(vm, lex, out_err);
                                if (out_err->code != 0) EVAL_EARLY_RETURN;
                                if (idx_val.type == VAL_STRING) {
                                    if (idx_val.as.string) str_release(vm_get_str(vm), idx_val.as.string);
                                    out_err->code = 13; out_err->message = "String values are not allowed as array indices";
                                    EVAL_EARLY_RETURN;
                                }
                                
                                if (lex_peek(lex).type == TOK_KEYWORD && lex_peek(lex).as.keyword == KW_TO) {
                                    lex_next(lex);
                                    BValue end_val = eval_expression(vm, lex, out_err);
                                    if (out_err->code != 0) EVAL_EARLY_RETURN;
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
                                lex_next(lex); /* Consume ',' */
                            } else if (next_tok.type == TOK_RPAREN) {
                                break;
                            } else {
                                out_err->code = 2; out_err->message = "Expected ',' or ')' in array index list";
                                EVAL_EARLY_RETURN;
                            }
                        }
                        lex_next(lex); /* Consume ')' */

                        if (has_slice) {
                            static int slice_counter = 0;
                            char tmp_name[64];
                            snprintf(tmp_name, sizeof(tmp_name), "__slice_%d", ++slice_counter);
                            
                            int base = arr_get_option_base(vm_get_arr(vm));
                            int new_bounds[4] = {0};
                            
                            /* Collect bounds for dimensions that are actually slices, to reduce dimensionality if possible */
                            for (int i = 0; i < num_indices; i++) {
                                new_bounds[i] = (slices[i].end - slices[i].start) + base;
                            }
                            
                            BppError dim_err = arr_dim(vm_get_arr(vm), tmp_name, num_indices, new_bounds);
                            if (dim_err.code != 0) {
                                *out_err = dim_err; EVAL_EARLY_RETURN;
                            }
                            
                            /* Copy elements */
                            int src_idx[4] = {0};
                            int dst_idx[4] = {0};
                            for(int d0 = slices[0].start, t0 = base; d0 <= slices[0].end; d0++, t0++) {
                                int max_d1 = (num_indices > 1) ? slices[1].end : 0;
                                for(int d1 = (num_indices > 1) ? slices[1].start : 0, t1 = base; d1 <= max_d1; d1++, t1++) {
                                    int max_d2 = (num_indices > 2) ? slices[2].end : 0;
                                    for(int d2 = (num_indices > 2) ? slices[2].start : 0, t2 = base; d2 <= max_d2; d2++, t2++) {
                                        int max_d3 = (num_indices > 3) ? slices[3].end : 0;
                                        for(int d3 = (num_indices > 3) ? slices[3].start : 0, t3 = base; d3 <= max_d3; d3++, t3++) {
                                            src_idx[0] = d0; src_idx[1] = d1; src_idx[2] = d2; src_idx[3] = d3;
                                            dst_idx[0] = t0; dst_idx[1] = t1; dst_idx[2] = t2; dst_idx[3] = t3;
                                            
                                            BValue *src_elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, src_idx, out_err);
                                            if (out_err->code == 0 && src_elem) {
                                                BValue *dst_elem = arr_get_element(vm_get_arr(vm), tmp_name, num_indices, dst_idx, out_err);
                                                if (out_err->code == 0 && dst_elem) {
                                                    *dst_elem = *src_elem;
                                                    if (dst_elem->type == VAL_STRING && dst_elem->as.string) str_add_ref(dst_elem->as.string);
                                                    else if (dst_elem->type == VAL_MAP && dst_elem->as.map) map_add_ref(dst_elem->as.map);
                                                }
                                            }
                                            out_err->code = 0; /* Clear out of bounds errors during slice copy if any */
                                        }
                                    }
                                }
                            }
                            
                            BValue val;
                            val.type = VAL_ARRAY_REF;
                            val.as.string = str_create(vm_get_str(vm), tmp_name, strlen(tmp_name));
                            VAL_PUSH(val);
                            expect_operand = false;
                            is_func = true;
                        } else {
                            /* Lookup single element */
                            int indices[4];
                            for (int i = 0; i < num_indices; i++) indices[i] = slices[i].start;
                            
                            BValue *elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, indices, out_err);
                            if (out_err->code != 0 || !elem) {
                                EVAL_EARLY_RETURN;
                            }

                            /* Push copy. If string, add reference! */
                            BValue val = *elem;
                            if (val.type == VAL_STRING && val.as.string) {
                                str_add_ref(val.as.string);
                            } else if (val.type == VAL_MAP && val.as.map) {
                                map_add_ref(val.as.map);
                            }
                            val = eval_resolve_member_access(vm, lex, val, out_err);
                            if (out_err->code != 0) EVAL_EARLY_RETURN;

                            VAL_PUSH(val);
                            expect_operand = false;
                            is_func = true;
                        }
                    }
                }
            } else if (eval_is_builtin_function(name_buf)) {
                BValue val;
                memset(&val, 0, sizeof(val));
                if (strcmp(name_buf, "RND") == 0) {
                    BppToken next = lex_peek(lex);
                    bool has_arg = false;
                    bool is_negative = false;
                    if (next.type == TOK_MINUS) {
                        LexerContext *temp = lex_init(vm_get_mem(vm), lex_get_pos(lex));
                        if (temp) {
                            lex_next(temp); /* consume '-' */
                            BppToken sub = lex_next(temp);
                            if (sub.type == TOK_NUMBER) {
                                has_arg = true;
                                is_negative = true;
                            }
                            lex_shutdown(temp);
                        }
                    } else if (next.type == TOK_NUMBER || next.type == TOK_IDENT || next.type == TOK_KEYWORD) {
                        has_arg = true;
                    }

                    if (has_arg) {
                        if (is_negative) {
                            lex_next(lex); /* Consume '-' */
                        }
                        BppToken arg_tok = lex_next(lex); /* Consume the argument token */
                        if (arg_tok.type == TOK_NUMBER) {
                            double num_val = arg_tok.as.number;
                            if (is_negative) num_val = -num_val;
                            int base = 0;
                            if (arg_tok.length > 2 && arg_tok.start[0] == '&') {
                                char b = (char)toupper((unsigned char)arg_tok.start[1]);
                                if (b == 'H') base = 16;
                                else if (b == 'O') base = 8;
                                else if (b == 'B') base = 2;
                                else if (isdigit((unsigned char)arg_tok.start[1])) base = 8;
                            }
                            
                            if (base > 0) {
                                long max_val = (long)num_val;
                                long r_val = 0;
                                if (max_val > 0) {
                                    r_val = rand() % (max_val + 1);
                                }
                                char buf[128] = "";
                                if (base == 16) snprintf(buf, sizeof(buf), "%X", (unsigned int)r_val);
                                else if (base == 8) snprintf(buf, sizeof(buf), "%o", (unsigned int)r_val);
                                else if (base == 2) {
                                    char bin[64] = "";
                                    int idx = 0;
                                    unsigned long tmp = r_val;
                                    if (tmp == 0) {
                                        strcpy(buf, "0");
                                    } else {
                                        while (tmp > 0) {
                                            bin[idx++] = (tmp & 1) ? '1' : '0';
                                            tmp >>= 1;
                                        }
                                        for (int j = 0; j < idx; j++) {
                                            buf[j] = bin[idx - 1 - j];
                                        }
                                        buf[idx] = '\0';
                                    }
                                }
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else {
                                long limit = (long)num_val;
                                long r_val = 0;
                                if (limit > 0) {
                                    r_val = rand() % (limit + 1);
                                } else if (limit < 0) {
                                    r_val = -(rand() % (-limit + 1));
                                }
                                val.type = VAL_NUMBER;
                                val.as.number = (double)r_val;
                            }
                        } else if (arg_tok.type == TOK_IDENT || arg_tok.type == TOK_KEYWORD) {
                            char arg_name[64] = "";
                            size_t alen = (arg_tok.length < 63) ? arg_tok.length : 63;
                            memcpy(arg_name, arg_tok.start, alen);
                            arg_name[alen] = '\0';
                            for (size_t k = 0; k < alen; k++) {
                                arg_name[k] = (char)toupper((unsigned char)arg_name[k]);
                            }
                            
                            if (strcmp(arg_name, "TIME") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else if (strcmp(arg_name, "TI") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                val.type = VAL_NUMBER;
                                val.as.number = h * 10000.0 + m * 100.0 + s;
                            } else if (strcmp(arg_name, "TIMER") == 0) {
                                double r_sec = ((double)rand() / (double)RAND_MAX) * 86400.0;
                                val.type = VAL_NUMBER;
                                val.as.number = r_sec;
                            } else {
                                val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                            }
                        } else {
                            val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                        }
                    } else {
                        val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                    }
                } else if (strcmp(name_buf, "RANDOMIZE") == 0) {
                    BppToken next = lex_peek(lex);
                    if (next.type == TOK_STRING || next.type == TOK_IDENT) {
                        BppToken arg_tok = lex_next(lex);
                        BValue arg_val;
                        memset(&arg_val, 0, sizeof(arg_val));
                        if (arg_tok.type == TOK_STRING) {
                            arg_val.type = VAL_STRING;
                            arg_val.as.string = str_create(vm_get_str(vm), arg_tok.as.string, arg_tok.length);
                        } else {
                            char var_name[256];
                            size_t vlen = (arg_tok.length < 255) ? arg_tok.length : 255;
                            memcpy(var_name, arg_tok.start, vlen);
                            var_name[vlen] = '\0';
                            BValue *lookup = var_lookup(var, var_name, false);
                            if (lookup) {
                                arg_val = *lookup;
                                if (arg_val.type == VAL_STRING && arg_val.as.string) str_add_ref(arg_val.as.string);
                                else if (arg_val.type == VAL_MAP && arg_val.as.map) map_add_ref(arg_val.as.map);
                            }
                        }

                        const char *mode = NULL;
                        if (arg_val.type == VAL_STRING && arg_val.as.string) {
                            mode = str_data(arg_val.as.string);
                        }
                        if (mode) {
                            if (strcmp(mode, "STRING$") == 0) {
                                int len = 8;
                                char *buf = (char *)calloc(1, len + 1);
                                if (!buf) { if (out_err) { out_err->code = 14; out_err->message = "Out of memory"; } return val; }
                                const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                                for (int i = 0; i < len; i++) {
                                    buf[i] = charset[rand() % (sizeof(charset) - 1)];
                                }
                                buf[len] = '\0';
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, len);
                                free(buf);
                            } else if (strcmp(mode, "DATE$") == 0) {
                                int m = rand() % 12 + 1;
                                int d = rand() % 28 + 1;
                                int y = rand() % 100;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d-%02d-%02d", m, d, y);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else if (strcmp(mode, "DAY$") == 0) {
                                const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
                                const char *day = days[rand() % 7];
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), day, strlen(day));
                            } else if (strcmp(mode, "TIME$") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else {
                                size_t len = strlen(mode);
                                char *buf = (char *)calloc(1, len + 1);
                                if (!buf) { if (out_err) { out_err->code = 14; out_err->message = "Out of memory"; } return val; }
                                strcpy(buf, mode);
                                for (size_t i = len - 1; i > 0; i--) {
                                    size_t j = rand() % (i + 1);
                                    char tmp = buf[i];
                                    buf[i] = buf[j];
                                    buf[j] = tmp;
                                }
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, len);
                                free(buf);
                            }
                        } else {
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), "", 0);
                        }

                        if (arg_val.type == VAL_STRING && arg_val.as.string) {
                            str_release(vm_get_str(vm), arg_val.as.string);
                        } else if (arg_val.type == VAL_MAP && arg_val.as.map) {
                            map_release(vm_get_str(vm), arg_val.as.map);
                        }
                    } else {
                        val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                    }
                } else {
                    val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                }
                if (out_err->code != 0) EVAL_EARLY_RETURN;
                VAL_PUSH(val);
                expect_operand = false;
                is_func = true;
            }

            if (!is_func) {
                if (tok.type == TOK_KEYWORD) {
                    out_err->code = 2;
                    out_err->message = "Unexpected keyword in expression";
                    EVAL_EARLY_RETURN;
                }

                BValue temp_val;
                memset(&temp_val, 0, sizeof(temp_val));
                bool is_special = false;
                if (strcmp(name_buf, "ERR") == 0) {
                    temp_val.type = VAL_NUMBER;
                    temp_val.as.number = (double)vm_get_err_code(vm);
                    is_special = true;
                } else if (strcmp(name_buf, "ERL") == 0) {
                    temp_val.type = VAL_NUMBER;
                    temp_val.as.number = (double)vm_get_err_line(vm);
                    is_special = true;
                }

                if (is_special) {
                    VAL_PUSH(temp_val);
                    expect_operand = false;
                } else {
                    BValue *var_val = var_lookup(var, name_buf, false);
                    if (!var_val) {
                        char base_name[256];
                        char member_chain[8][64];
                        int member_count = 0;
                        eval_split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                        
                        if (member_count > 0) {
                            var_val = var_lookup(var, base_name, false);
                            if (var_val) {
                                BValue val = *var_val;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
                                
                                /* Walk up to the last member */
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Member access on non-object value";
                                        EVAL_EARLY_RETURN;
                                    }
                                    BValue next_val;
                                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                                        out_err->code = 35; out_err->message = "Member field not found";
                                        if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                        EVAL_EARLY_RETURN;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                /* Check if followed by '(' -> Method call on the resolved object 'val' */
                                if (lex_peek(lex).type == TOK_LPAREN) {
                                    lex_next(lex); /* Consume '(' */
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Method call on non-object value";
                                        EVAL_EARLY_RETURN;
                                    }
                                    BValue type_val;
                                    if (!map_get(val.as.map, "__type__", &type_val) || type_val.type != VAL_STRING || !type_val.as.string) {
                                        out_err->code = 13; out_err->message = "Object missing class type metadata";
                                        EVAL_EARLY_RETURN;
                                    }
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
                                            out_err->code = 2; out_err->message = "Too many arguments in method call";
                                            for (int i = 0; i < argc; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            EVAL_EARLY_RETURN;
                                        }
                                        args[argc++] = eval_expression(vm, lex, out_err);
                                        if (out_err->code != 0) {
                                            for (int i = 0; i < argc - 1; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            EVAL_EARLY_RETURN;
                                        }
                                        next_tok = lex_peek(lex);
                                        if (next_tok.type == TOK_COMMA) {
                                            lex_next(lex);
                                        } else if (next_tok.type == TOK_RPAREN) {
                                            lex_next(lex);
                                            break;
                                        } else {
                                            out_err->code = 2; out_err->message = "Expected ',' or ')' in method call";
                                            for (int i = 0; i < argc; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            EVAL_EARLY_RETURN;
                                        }
                                    }
                                    
                                    BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                                    for (int i = 0; i < argc; i++) {
                                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                    }
                                    if (out_err->code != 0) EVAL_EARLY_RETURN;
                                    
                                    VAL_PUSH(ret_val);
                                    expect_operand = false;
                                    tok = lex_peek(lex);
                                    continue;
                                }
                                
                                /* Standard lookup for the last field */
                                {
                                    int m = member_count - 1;
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Member access on non-object value";
                                        EVAL_EARLY_RETURN;
                                    }
                                    BValue next_val;
                                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                                        out_err->code = 35; out_err->message = "Member field not found";
                                        if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                        EVAL_EARLY_RETURN;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                VAL_PUSH(val);
                                expect_operand = false;
                                tok = lex_peek(lex);
                                continue;
                            }
                        }
                        
                        var_val = var_lookup(var, name_buf, true);
                        if (!var_val) {
                            out_err->code = 2;
                            out_err->message = "Variable not declared (OPTION EXPLICIT)";
                            EVAL_EARLY_RETURN;
                        }
                    }

                    BValue val = *var_val;
                    if (val.type == VAL_STRING && val.as.string) {
                        str_add_ref(val.as.string);
                    } else if (val.type == VAL_MAP && val.as.map) {
                        map_add_ref(val.as.map);
                    } else if (val.type == VAL_FIELD_STRING) {
                        /* Read from random access file buffer */
                        int ch = val.as.field_str.channel;
                        unsigned char *rec_buf = file_get_record_buffer(vm_get_file(vm), ch);
                        if (rec_buf) {
                            char *buf_slice = (char *)calloc(1, val.as.field_str.length + 1);
                            if (!buf_slice) {
                                out_err->code = 7; out_err->message = "Out of memory";
                                EVAL_EARLY_RETURN;
                            }
                            memcpy(buf_slice, rec_buf + val.as.field_str.offset, val.as.field_str.length);
                            buf_slice[val.as.field_str.length] = '\0';
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), buf_slice, val.as.field_str.length);
                            free(buf_slice);
                        } else {
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), "", 0);
                        }
                    }
                    val = eval_resolve_member_access(vm, lex, val, out_err);
                    if (out_err->code != 0) EVAL_EARLY_RETURN;

                    VAL_PUSH(val);
                    expect_operand = false;
                }
            }
        } else if (tok.type == TOK_LPAREN) {
            if (!expect_operand) {
                /* JOSS style discrete range or separate block: stop parsing */
                break;
            }
            OP_PUSH(TOK_LPAREN);
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            if (expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operand before ')'";
                EVAL_EARLY_RETURN;
            }
            bool found_paren = false;
            while (op_ptr > 0) {
                BppTokenType top = op_stack[--op_ptr];
                if (top == TOK_LPAREN) {
                    found_paren = true;
                    break;
                }
                if (!eval_execute_op(vm, top, val_stack, &val_ptr, out_err)) {
                    EVAL_EARLY_RETURN;
                }
            }
            if (!found_paren) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Mismatched parentheses";
                EVAL_EARLY_RETURN;
            }
            open_parens--;
        } else if (eval_is_operator(tok.type)) {
            BppTokenType op = tok.type;
            if (expect_operand) {
                /* Translate to unary */
                if (op == TOK_MINUS) op = TOK_UNARY_MINUS;
                else if (op == TOK_PLUS) op = TOK_UNARY_PLUS;
                else if (op == TOK_NOT) {
                    /* TOK_NOT is unary, keep it */
                }
                else {
                    out_err->code = 2; /* Syntax error */
                    out_err->message = "Expected operand, got operator";
                    EVAL_EARLY_RETURN;
                }
            } else {
                /* Binary operator, but NOT cannot be binary */
                if (op == TOK_NOT) {
                    out_err->code = 2; /* Syntax error */
                    out_err->message = "Unexpected NOT operator";
                    EVAL_EARLY_RETURN;
                }
            }

            while (op_ptr > 0) {
                BppTokenType top = op_stack[op_ptr - 1];
                if (top == TOK_LPAREN) break;
                if (eval_has_precedence(vm, top, op)) {
                    op_ptr--;
                    if (!eval_execute_op(vm, top, val_stack, &val_ptr, out_err)) {
                        EVAL_EARLY_RETURN;
                    }
                } else {
                    break;
                }
            }
            OP_PUSH(op);
            expect_operand = true;
        } else {
            /* Stop parsing expression */
            break;
        }

        tok = lex_peek(lex);
    }

    /* Pop all remaining operators */
    while (op_ptr > 0) {
        BppTokenType top = op_stack[--op_ptr];
        if (top == TOK_LPAREN) {
            out_err->code = 2; /* Syntax error */
            out_err->message = "Mismatched parentheses";
            EVAL_EARLY_RETURN;
        }
        if (!eval_execute_op(vm, top, val_stack, &val_ptr, out_err)) {
            EVAL_EARLY_RETURN;
        }
    }

    if (val_ptr != 1) {
        out_err->code = 2; /* Syntax error */
        out_err->message = "Invalid expression structure";
        vm_dec_eval_depth(vm);
        return null_val;
    }

    vm_dec_eval_depth(vm);
    return val_stack[0];
    #undef EVAL_EARLY_RETURN
    #undef VAL_PUSH
    #undef OP_PUSH
}

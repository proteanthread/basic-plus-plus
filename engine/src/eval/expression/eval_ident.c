// FILENAME: eval_ident.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval_ident within BASIC++.
//
// ---- Includes ----
#include "eval/eval_expr_internal.h"
#include "runtime/variables_internal.h"
#include "statements/oop/sub.h"
#include "eval/functions/network/func_ip.h"
#include "eval/functions/system/time/func_tim.h"
#include "eval/functions/datetime/time_part.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "eval/functions/system/hardware/func_clocks.h"
#include "eval/functions/system/hardware/stick.h"
#include "eval/functions/system/hardware/strig.h"
#include "eval/functions/system/hardware/paddle.h"
#include "eval/functions/system/hardware/ptrig.h"
#include "bios/bios_cpu8086.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "eval/functions/math/random/rnd.h"

//
// ---- Identifier & Member Resolution Helpers ----
static inline bool is_delim_token(BppTokenType t) {
    return (t == TOK_LPAREN || t == TOK_LBRACKET || t == TOK_LBRACE ||
            t == TOK_PN_LITERAL || t == TOK_RPN_LITERAL);
}

static inline bool eval_check_bare_hw(VMContext *vm, const char *name, BValue *out_val, BppError *err) {
    if (runtime_strcasecmp(name, "BAUD") == 0) { *out_val = func_baud_eval(vm, name, 0, NULL, err); return true; }
    if (runtime_strcasecmp(name, "CPUSPEED") == 0 || runtime_strcasecmp(name, "CPUSPEED$") == 0 ||
        runtime_strcasecmp(name, "SYS.CPUSPEED") == 0 || runtime_strcasecmp(name, "SYS.CPUSPEED$") == 0) {
        *out_val = func_cpuspeed_eval(vm, name, 0, NULL, err); return true;
    }
    if (runtime_strcasecmp(name, "CLOCKS") == 0 || runtime_strcasecmp(name, "CLOCKS$") == 0 ||
        runtime_strcasecmp(name, "SYS.CLOCKS") == 0 || runtime_strcasecmp(name, "SYS.CLOCKS$") == 0) {
        *out_val = func_clocks_eval(vm, name, 0, NULL, err); return true;
    }
    if (runtime_strcasecmp(name, "STICK") == 0) { *out_val = func_stick_eval(vm, name, 0, NULL, err); return true; }
    if (runtime_strcasecmp(name, "STRIG") == 0) { *out_val = func_strig_eval(vm, name, 0, NULL, err); return true; }
    if (runtime_strcasecmp(name, "PADDLE") == 0) { *out_val = func_paddle_eval(vm, name, 0, NULL, err); return true; }
    if (runtime_strcasecmp(name, "PTRIG") == 0) { *out_val = func_ptrig_eval(vm, name, 0, NULL, err); return true; }
    return false;
}

bool eval_parse_identifier_expression(VMContext *vm, LexerContext *lex, BppToken tok, BValue *out_val, bool *out_is_func, BppError *out_err) {
    if (!vm || !lex || !out_val || !out_is_func || !out_err) return false;

    VariableContext *var = vm_get_var(vm);
    *out_is_func = false;

    // Ultra-Fast Path: 1-character identifier (e.g. X, Y, I, J, K, A..Z)
    if (tok.type == TOK_IDENT && tok.length == 1 && var && var->shared_count == 0 && var->active_scope[0] == '\0') {
        BppTokenType peek_type = lex_peek(lex).type;
        if (!is_delim_token(peek_type) && peek_type != TOK_PERIOD) {
            char c0 = tok.start[0];
            if ((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z')) {
                int idx = (c0 >= 'a') ? (c0 - 'a') : (c0 - 'A');
                if (var->fast_scalars_valid[idx]) {
                    *out_val = var->fast_scalars[idx];
                    return true;
                }
            }
        }
    }

    char name_buf[256];
    if (tok.type == TOK_PERIOD) {
        const char *with_prefix = vm_with_stack_peek(vm);
        if (!with_prefix) {
            out_err->code = 2;
            out_err->message = "Leading '.' outside of WITH block";
            return false;
        }
        BppToken sub_tok = lex_next(lex);
        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
            out_err->code = 2;
            out_err->message = "Expected identifier after '.' in WITH member access";
            return false;
        }
        runtime_snprintf(name_buf, sizeof(name_buf), "%s.%.*s", with_prefix, (int)sub_tok.length, sub_tok.start);
    } else if (tok.type == TOK_AT) {
        BppToken sub_tok = lex_next(lex);
        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
            out_err->code = 2;
            out_err->message = "Expected identifier after '@'";
            return false;
        }
        runtime_snprintf(name_buf, sizeof(name_buf), "@%.*s", (int)sub_tok.length, sub_tok.start);
    } else {
        size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
        runtime_memcpy(name_buf, tok.start, copy_len);
        name_buf[copy_len] = '\0';
    }

    // Accumulate all dotted tokens early so names like SYS.OS$, MATH.PI, WIFI.SSID$ are fully populated
    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub_tok = lex_next(lex);
        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
            out_err->code = 2;
            out_err->message = "Expected identifier after '.' in member access";
            return false;
        }
        char sub_name[128];
        size_t sub_len = (sub_tok.length < sizeof(sub_name) - 1) ? sub_tok.length : sizeof(sub_name) - 1;
        runtime_memcpy(sub_name, sub_tok.start, sub_len);
        sub_name[sub_len] = '\0';

        char combined[512];
        runtime_snprintf(combined, sizeof(combined), "%s.%s", name_buf, sub_name);
        runtime_strncpy(name_buf, combined, sizeof(name_buf) - 1);
        name_buf[sizeof(name_buf) - 1] = '\0';
    }

    DynamicVarEntry *dvar = var_find_dynamic(var, name_buf);
    if (dvar && !is_delim_token(lex_peek(lex).type)) {
        if (dvar->getter) {
            *out_val = dvar->getter(vm, name_buf, dvar->user_data);
            *out_is_func = true;
            return true;
        }
        if (dvar->read_fn[0] != '\0') {
            BppError fn_err;
            runtime_memset(&fn_err, 0, sizeof(fn_err));
            *out_val = invoke_user_function(vm, dvar->read_fn, NULL, 0, &fn_err);
            if (fn_err.code != 0) {
                *out_err = fn_err;
                return false;
            }
            *out_is_func = true;
            return true;
        }
    }

    // Check built-in constants and system identifiers
    if (!is_delim_token(lex_peek(lex).type)) {
        if (eval_try_resolve_builtin_constant_or_system_var(vm, name_buf, out_val)) {
            return true;
        }
    }

    // Check user variable table if not followed by delimiter
    if (!is_delim_token(lex_peek(lex).type)) {
        BValue *existing_user_var = var_lookup(var, name_buf, false);
        if (existing_user_var && existing_user_var->type != VAL_NONE) {
            *out_val = *existing_user_var;
            if (existing_user_var->type == VAL_STRING && existing_user_var->as.string) {
                str_add_ref(existing_user_var->as.string);
            } else if (existing_user_var->type == VAL_MAP && existing_user_var->as.map) {
                map_add_ref(existing_user_var->as.map);
            }
            return true;
        }

        // Check if name_buf is an object property access: base.field
        const char *dot_pos = runtime_strchr(name_buf, '.');
        if (dot_pos) {
            char base_name[128];
            size_t blen = (size_t)(dot_pos - name_buf);
            if (blen < sizeof(base_name)) {
                runtime_memcpy(base_name, name_buf, blen);
                base_name[blen] = '\0';
                const char *field_name = dot_pos + 1;
                BValue *base_val = var_lookup(var, base_name, false);
                if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
                    BValue prop_val;
                    if (map_get(base_val->as.map, field_name, &prop_val)) {
                        *out_val = prop_val;
                        if (prop_val.type == VAL_STRING && prop_val.as.string) {
                            str_add_ref(prop_val.as.string);
                        } else if (prop_val.type == VAL_MAP && prop_val.as.map) {
                            map_add_ref(prop_val.as.map);
                        }
                        return true;
                    }
                }
            }
        }
    }

    // Special handling for RND without parentheses (e.g. RND, RND 255, RND 15, RND -5, RND min, max)
    if (runtime_strcasecmp(name_buf, "RND") == 0 && !is_delim_token(lex_peek(lex).type)) {
        BppToken next = lex_peek(lex);
        bool has_arg = (next.type == TOK_NUMBER || next.type == TOK_IDENT || next.type == TOK_KEYWORD || next.type == TOK_MINUS);
        if (has_arg) {
            BValue arg = eval_expression(vm, lex, out_err);
            if (out_err->code == 0) {
                if (lex_peek(lex).type == TOK_COMMA) {
                    lex_next(lex); // consume comma
                    BValue arg2 = eval_expression(vm, lex, out_err);
                    if (out_err->code == 0) {
                        BValue args[2] = { arg, arg2 };
                        *out_val = func_rnd_eval(vm, "RND", 2, args, out_err);
                    } else {
                        out_val->type = VAL_NUMBER; out_val->as.number = 0.0;
                    }
                    *out_is_func = true;
                    return true;
                }

                double num_val = (arg.type == VAL_NUMBER || arg.type == VAL_INTEGER) ? arg.as.number : 0.0;
                if (num_val <= 1.0) {
                    BValue args[1] = { arg };
                    *out_val = func_rnd_eval(vm, "RND", 1, args, out_err);
                } else {
                    // Unparenthesized RND n (e.g. RND 255 or RND 15) -> returns integer in [0, n] inclusive (0-based range)
                    BValue rnd0 = func_rnd_eval(vm, "RND", 0, NULL, out_err);
                    double v = rnd0.as.number;
                    if (num_val == runtime_floor(num_val)) {
                        int64_t n_int = (int64_t)num_val;
                        int64_t rand_int = (int64_t)(v * (double)(n_int + 1));
                        if (rand_int > n_int) rand_int = n_int;
                        out_val->type = VAL_NUMBER;
                        out_val->as.number = (double)rand_int;
                    } else {
                        out_val->type = VAL_NUMBER;
                        out_val->as.number = v * num_val;
                    }
                }
            } else {
                out_val->type = VAL_NUMBER;
                out_val->as.number = 0.0;
            }
        } else {
            *out_val = func_rnd_eval(vm, "RND", 0, NULL, out_err);
        }
        *out_is_func = true;
        return true;
    }

    // Unparenthesized NEG <expr> (e.g. NEG X, NEG 5, NEG A+B)
    if (runtime_strcasecmp(name_buf, "NEG") == 0 && !is_delim_token(lex_peek(lex).type)) {
        BValue arg = eval_expression(vm, lex, out_err);
        if (out_err->code == 0) {
            if (arg.type == VAL_INTEGER) {
                out_val->type = VAL_INTEGER;
                out_val->as.number = -arg.as.number;
            } else if (arg.type == VAL_NUMBER) {
                out_val->type = VAL_NUMBER;
                out_val->as.number = -arg.as.number;
            } else {
                out_err->code = 13;
                out_err->message = "Type mismatch: NEG requires a numeric argument";
                return false;
            }
        } else {
            out_val->type = VAL_NUMBER;
            out_val->as.number = 0.0;
        }
        *out_is_func = true;
        return true;
    }

    // Bare IP$ without parentheses
    if (runtime_strcasecmp(name_buf, "IP$") == 0 && lex_peek(lex).type != TOK_LPAREN) {
        *out_val = func_ip_eval(vm, "IP$", 0, NULL, out_err);
        return (out_err->code == 0);
    }

    // Bare TIM without parentheses
    if (runtime_strcasecmp(name_buf, "TIM") == 0 && lex_peek(lex).type != TOK_LPAREN) {
        *out_val = func_tim_eval(vm, "TIM", 0, NULL, out_err);
        return (out_err->code == 0);
    }

    // Bare ALARM / ALARM$ without parentheses
    if (runtime_strcasecmp(name_buf, "ALARM") == 0 && lex_peek(lex).type != TOK_LPAREN) {
        runtime_memset(out_val, 0, sizeof(*out_val));
        out_val->type = VAL_NUMBER;
        out_val->as.number = 0.0;
        return true;
    }
    if (runtime_strcasecmp(name_buf, "ALARM$") == 0 && lex_peek(lex).type != TOK_LPAREN) {
        runtime_memset(out_val, 0, sizeof(*out_val));
        out_val->type = VAL_STRING;
        out_val->as.string = str_create(vm_get_str(vm), "", 0);
        return true;
    }
    if (!is_delim_token(lex_peek(lex).type) && eval_check_bare_hw(vm, name_buf, out_val, out_err)) {
        return (out_err->code == 0);
    } 

    BValue *ref_var = var_lookup(var, name_buf, false);
    if (ref_var && ref_var->type == VAL_ARRAY_REF && ref_var->as.string) {
        const char *orig_name = str_data(ref_var->as.string);
        size_t olen = runtime_strlen(orig_name);
        if (olen < sizeof(name_buf) - 1) {
            runtime_memcpy(name_buf, orig_name, olen);
            name_buf[olen] = '\0';
        }
    }

    BppTokenType next_tok_type = lex_peek(lex).type;
    if (next_tok_type == TOK_PN_LITERAL) {
        if (eval_is_builtin_function(name_buf)) {
            *out_is_func = true;
            BppToken pn_tok = lex_next(lex);
            char *pn_buf = (char *)mem_scratch_alloc(vm_get_mem(vm), pn_tok.length + 3);
            if (pn_buf) {
                runtime_memcpy(pn_buf, pn_tok.as.string, pn_tok.length);
                pn_buf[pn_tok.length] = ']';
                pn_buf[pn_tok.length + 1] = '\0';
                LexerContext *sub_lex = lex_init(vm_get_mem(vm), pn_buf);
                *out_val = eval_builtin_function_delim(vm, name_buf, sub_lex, TOK_LBRACKET, out_err);
                lex_shutdown(sub_lex);
                return (out_err->code == 0);
            }
        }
    }

    if (next_tok_type == TOK_RPN_LITERAL) {
        if (eval_is_builtin_function(name_buf)) {
            *out_is_func = true;
            BppToken rpn_tok = lex_next(lex);
            char *rpn_buf = (char *)mem_scratch_alloc(vm_get_mem(vm), rpn_tok.length + 3);
            if (rpn_buf) {
                runtime_memcpy(rpn_buf, rpn_tok.as.string, rpn_tok.length);
                rpn_buf[rpn_tok.length] = '}';
                rpn_buf[rpn_tok.length + 1] = '\0';
                LexerContext *sub_lex = lex_init(vm_get_mem(vm), rpn_buf);
                *out_val = eval_builtin_function_delim(vm, name_buf, sub_lex, TOK_LBRACE, out_err);
                lex_shutdown(sub_lex);
                return (out_err->code == 0);
            }
        }
        if (arr_exists(vm_get_arr(vm), name_buf) || var_lookup(vm_get_var(vm), name_buf, false)) {
            *out_is_func = true;
            return eval_parse_brace_access(vm, lex, name_buf, out_val, out_err);
        }
    }

    if (next_tok_type == TOK_LBRACKET) {
        if (eval_is_builtin_function(name_buf)) {
            *out_is_func = true;
            lex_next(lex);
            *out_val = eval_builtin_function_delim(vm, name_buf, lex, TOK_LBRACKET, out_err);
            return (out_err->code == 0);
        }
        if (arr_exists(vm_get_arr(vm), name_buf)) {
            *out_is_func = true;
            return eval_parse_array_access(vm, lex, name_buf, out_val, out_err);
        }
        *out_is_func = true;
        *out_val = eval_parse_string_slice(vm, lex, name_buf, TOK_LBRACKET, out_err);
        return (out_err->code == 0);
    }
    
    if (next_tok_type == TOK_LBRACE) {
        if (eval_is_builtin_function(name_buf)) {
            *out_is_func = true;
            lex_next(lex);
            *out_val = eval_builtin_function_delim(vm, name_buf, lex, TOK_LBRACE, out_err);
            return (out_err->code == 0);
        }
        if (arr_exists(vm_get_arr(vm), name_buf) || var_lookup(vm_get_var(vm), name_buf, false)) {
            *out_is_func = true;
            return eval_parse_brace_access(vm, lex, name_buf, out_val, out_err);
        }
        const BppUserTypeDef *utype = struct_find_type(vm_get_types(vm), name_buf);
        if (utype) {
            *out_is_func = true;
            lex_next(lex);
            BppMap *rec_map = map_create();
            BValue tname_val;
            tname_val.type = VAL_STRING;
            tname_val.as.string = str_create(vm_get_str(vm), utype->name, runtime_strlen(utype->name));
            map_set(vm_get_str(vm), rec_map, "__type__", tname_val);
            str_release(vm_get_str(vm), tname_val.as.string);

            while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                BppToken fld_tok = lex_next(lex);
                char fld_name[64];
                size_t flen = (fld_tok.length < sizeof(fld_name) - 1) ? fld_tok.length : sizeof(fld_name) - 1;
                runtime_memcpy(fld_name, fld_tok.start, flen);
                fld_name[flen] = '\0';

                if (lex_peek(lex).type == TOK_EQ || (lex_peek(lex).type == TOK_EOL && lex_peek(lex).start && lex_peek(lex).start[0] == ':')) {
                    lex_next(lex);
                }
                BValue fld_val = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) {
                    map_release(vm_get_str(vm), rec_map);
                    return false;
                }
                map_set(vm_get_str(vm), rec_map, fld_name, fld_val);
                if (fld_val.type == VAL_STRING && fld_val.as.string) str_release(vm_get_str(vm), fld_val.as.string);
                else if (fld_val.type == VAL_MAP && fld_val.as.map) map_release(vm_get_str(vm), fld_val.as.map);

                if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
                else if (lex_peek(lex).type == TOK_RBRACE) break;
            }
            if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
            out_val->type = VAL_MAP;
            out_val->as.map = rec_map;
            return true;
        }
    }

    if (next_tok_type == TOK_LPAREN) {
        bool is_slicing = false;
        if (name_buf[runtime_strlen(name_buf) - 1] == '$' && runtime_strchr(name_buf, '.') == NULL) {
            if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL) &&
                !def_fn_exists(name_buf) &&
                !eval_is_builtin_function(name_buf) &&
                !arr_exists(vm_get_arr(vm), name_buf)) {
                is_slicing = true;
            }
        }
        if (is_slicing) {
            *out_is_func = true;
            *out_val = eval_parse_string_slice(vm, lex, name_buf, TOK_LPAREN, out_err);
            return (out_err->code == 0);
        }
        
        if (eval_is_builtin_function(name_buf)) {
            *out_is_func = true;
            lex_next(lex);
            *out_val = eval_builtin_function_delim(vm, name_buf, lex, TOK_LPAREN, out_err);
            return (out_err->code == 0);
        }
        
        if (!arr_exists(vm_get_arr(vm), name_buf) &&
            !(var_lookup(var, name_buf, false) && var_lookup(var, name_buf, false)->type == VAL_ARRAY_REF && var_lookup(var, name_buf, false)->as.string)) {
            *out_is_func = true;
            lex_next(lex);

            bool is_method = false;
            char base_name[256] = "";
            char member_chain[8][64];
            int member_count = 0;
            char fully_qualified_method[512] = "";
            BValue obj_val;
            runtime_memset(&obj_val, 0, sizeof(obj_val));

            if (runtime_strchr(name_buf, '.') != NULL) {
                if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL)) {
                    eval_split_member_chain(name_buf, runtime_strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                    if (member_count > 0) {
                        if (runtime_strcasecmp(base_name, "BASE") == 0 || runtime_strcasecmp(base_name, "SUPER") == 0) {
                            BValue *this_ptr = var_lookup(var, "THIS", false);
                            if (!this_ptr) this_ptr = var_lookup(var, "ME", false);
                            if (this_ptr && this_ptr->type == VAL_MAP && this_ptr->as.map) {
                                BValue type_val;
                                if (map_get(this_ptr->as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                                    const BppUserTypeDef *pdef = struct_find_type(vm_get_types(vm), str_data(type_val.as.string));
                                    if (pdef && pdef->parent_name[0] != '\0') {
                                        runtime_snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                         pdef->parent_name, member_chain[member_count - 1]);
                                        obj_val = *this_ptr;
                                        map_add_ref(obj_val.as.map);
                                        is_method = true;
                                    }
                                }
                            }
                        } else {
                            BValue *var_val = var_lookup(var, base_name, false);
                            if (var_val) {
                                obj_val = *var_val;
                                if (obj_val.type == VAL_STRING && obj_val.as.string) str_add_ref(obj_val.as.string);
                                else if (obj_val.type == VAL_MAP && obj_val.as.map) map_add_ref(obj_val.as.map);

                                bool walk_err = false;
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (obj_val.type != VAL_MAP || !obj_val.as.map) { walk_err = true; break; }
                                    BValue next_val;
                                    if (!map_get(obj_val.as.map, member_chain[m], &next_val)) { walk_err = true; break; }
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
                                        runtime_snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
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
            }


            BValue args[9];
            int argc = 0;
            if (is_method) args[argc++] = obj_val;

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
                    return false;
                }

                args[argc++] = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) {
                    for (int i = 0; i < argc - 1; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return false;
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
                    return false;
                }
            }

            *out_val = invoke_user_function(vm, is_method ? fully_qualified_method : name_buf, args, argc, out_err);
            for (int i = 0; i < argc; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
            }
            return (out_err->code == 0);
        }

        // Array access
        *out_is_func = true;
        return eval_parse_array_access(vm, lex, name_buf, out_val, out_err);
    }

    if (!var_lookup(var, name_buf, false) && eval_is_zero_arg_builtin_function(name_buf)) {
        *out_val = eval_builtin_function(vm, name_buf, lex, false, out_err);
        if (out_err->code != 0) return false;
        *out_is_func = true;
        return true;
    }

    // Variable and Object Property Lookup
    BValue temp_val;
    runtime_memset(&temp_val, 0, sizeof(temp_val));
    bool is_special = false;

    if (runtime_strcmp(name_buf, "ERR") == 0) {
        temp_val.type = VAL_NUMBER; temp_val.as.number = (double)vm_get_err_code(vm); is_special = true;
    } else if (runtime_strcmp(name_buf, "ERL") == 0) {
        temp_val.type = VAL_NUMBER; temp_val.as.number = (double)vm_get_err_line(vm); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "COMMAND$") == 0 || runtime_strcasecmp(name_buf, "COMMAND") == 0) {
        const char *cmd = runtime_get_command_line();
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), cmd ? cmd : "", cmd ? runtime_strlen(cmd) : 0); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "DOEVENTS") == 0) {
        platform_sleep_ms(0); temp_val.type = VAL_NUMBER; temp_val.as.number = 0.0; is_special = true;
    } else if (runtime_strcasecmp(name_buf, "Screen.ActiveForm") == 0) {
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), "Form1", 5); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "Screen.ActiveControl") == 0) {
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), "Text1", 5); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "Screen.ControlCount") == 0) {
        temp_val.type = VAL_NUMBER; temp_val.as.number = 5.0; is_special = true;
    } else if (runtime_strcasecmp(name_buf, "App.Title") == 0) {
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), "BASIC++ App", 11); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "App.Path") == 0) {
        char cwd_buf[1024] = {0}; platform_getcwd(cwd_buf, sizeof(cwd_buf) - 1);
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), cwd_buf, runtime_strlen(cwd_buf)); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "App.EXEName") == 0) {
        temp_val.type = VAL_STRING; temp_val.as.string = str_create(vm_get_str(vm), "baspp", 5); is_special = true;
    } else if (runtime_strcasecmp(name_buf, "App.PrevInstance") == 0) {
        temp_val.type = VAL_NUMBER; temp_val.as.number = 0.0; is_special = true;
    }

    if (is_special) {
        *out_val = temp_val;
        return true;
    }

    BValue *var_val = var_lookup(var, name_buf, false);
    if (!var_val) {
        char base_name[256];
        char member_chain[8][64];
        int member_count = 0;
        eval_split_member_chain(name_buf, runtime_strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);

        if (member_count > 0) {
            var_val = var_lookup(var, base_name, false);
            if (var_val) {
                BValue val = *var_val;
                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);

                for (int m = 0; m < member_count - 1; m++) {
                    if (val.type != VAL_MAP || !val.as.map) {
                        out_err->code = 13; out_err->message = "Member access on non-object value";
                        return false;
                    }
                    BValue next_val;
                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                        out_err->code = 35; out_err->message = "Member field not found";
                        if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                        return false;
                    }
                    BValue copy = next_val;
                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);

                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                    val = copy;
                }

                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex);
                    if (val.type != VAL_MAP || !val.as.map) {
                        out_err->code = 13; out_err->message = "Method call on non-object value";
                        return false;
                    }
                    BValue type_val;
                    if (!map_get(val.as.map, "__type__", &type_val) || type_val.type != VAL_STRING || !type_val.as.string) {
                        out_err->code = 13; out_err->message = "Object missing class type metadata";
                        return false;
                    }
                    char fully_qualified_method[512];
                    runtime_snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                     str_data(type_val.as.string), member_chain[member_count - 1]);

                    BValue args[9];
                    int argc = 0;
                    args[argc++] = val;
                    map_add_ref(val.as.map);

                    while (true) {
                        BppToken next_tok = lex_peek(lex);
                        if (next_tok.type == TOK_RPAREN) { lex_next(lex); break; }
                        if (argc >= 9) {
                            out_err->code = 2; out_err->message = "Too many arguments in method call";
                            for (int i = 0; i < argc; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return false;
                        }
                        args[argc++] = eval_expression(vm, lex, out_err);
                        if (out_err->code != 0) {
                            for (int i = 0; i < argc - 1; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return false;
                        }
                        next_tok = lex_peek(lex);
                        if (next_tok.type == TOK_COMMA) lex_next(lex);
                        else if (next_tok.type == TOK_RPAREN) { lex_next(lex); break; }
                        else {
                            out_err->code = 2; out_err->message = "Expected ',' or ')' in method call";
                            for (int i = 0; i < argc; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return false;
                        }
                    }

                    *out_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                    for (int i = 0; i < argc; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return (out_err->code == 0);
                }

                int m = member_count - 1;
                if (val.type != VAL_MAP || !val.as.map) {
                    out_err->code = 13; out_err->message = "Member access on non-object value";
                    return false;
                }
                BValue type_val;
                bool has_prop_get = false;
                if (map_get(val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                    char prop_proc[128];
                    runtime_snprintf(prop_proc, sizeof(prop_proc), "%s.%s", str_data(type_val.as.string), member_chain[m]);
                    BppLineNumber target_line = 0;
                    const char *target_text = NULL;
                    bool is_lib = false;
                    if (find_procedure_ex(vm, prop_proc, KW_PROPERTY, &target_line, &target_text, &is_lib) ||
                        find_procedure_ex(vm, prop_proc, KW_FUNCTION, &target_line, &target_text, &is_lib)) {
                        BValue p_arg = val;
                        map_add_ref(p_arg.as.map);
                        BppError p_err;
                        runtime_memset(&p_err, 0, sizeof(p_err));
                        BValue res = invoke_user_function(vm, prop_proc, &p_arg, 1, &p_err);
                        map_release(vm_get_str(vm), p_arg.as.map);
                        if (p_err.code == 0) {
                            if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                            val = res;
                            has_prop_get = true;
                        }
                    }
                }
                if (!has_prop_get) {
                    BValue next_val;
                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                        out_err->code = 35; out_err->message = "Member field not found";
                        if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                        return false;
                    }
                    BValue copy = next_val;
                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);

                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                    val = copy;
                }

                *out_val = val;
                return true;
            }
        }

        if (arr_exists(vm_get_arr(vm), name_buf)) {
            runtime_memset(out_val, 0, sizeof(*out_val));
            out_val->type = VAL_ARRAY_REF;
            out_val->as.string = str_create(vm_get_str(vm), name_buf, runtime_strlen(name_buf));
            return true;
        }

        var_val = var_lookup(var, name_buf, true);
        if (!var_val) {
            out_err->code = 2;
            out_err->message = "Variable not declared (OPTION EXPLICIT)";
            return false;
        }
    }

    BValue val = *var_val;
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_add_ref(val.as.map);
    } else if (val.type == VAL_FIELD_STRING) {
        int ch = val.as.field_str.channel;
        unsigned char *rec_buf = file_get_record_buffer(vm_get_file(vm), ch);
        if (rec_buf) {
            HalContext *hal = hal_get();
            char *buf_slice = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(val.as.field_str.length + 1) : NULL;
            if (!buf_slice) {
                out_err->code = 7; out_err->message = "Out of memory";
                return false;
            }
            runtime_memset(buf_slice, 0, val.as.field_str.length + 1);
            runtime_memcpy(buf_slice, rec_buf + val.as.field_str.offset, val.as.field_str.length);
            buf_slice[val.as.field_str.length] = '\0';
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), buf_slice, val.as.field_str.length);
            if (hal && hal->mem.free) hal->mem.free(buf_slice);
        } else {
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }

    val = eval_resolve_member_access(vm, lex, val, out_err);
    if (out_err->code != 0) return false;

    *out_val = val;
    return true;
}

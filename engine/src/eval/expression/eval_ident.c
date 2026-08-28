// FILENAME: eval_ident.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval_ident within BASIC++.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"

//
// ---- Identifier & Member Resolution Helpers ----

static BValue eval_handle_special_builtin(VMContext *vm, LexerContext *lex, const char *name_buf, BppError *out_err) {
    BValue val;
    runtime_memset(&val, 0, sizeof(val));

    if (runtime_strcmp(name_buf, "RND") == 0) {
        BppToken next = lex_peek(lex);
        bool has_arg = false;
        bool is_negative = false;
        if (next.type == TOK_MINUS) {
            LexerContext *temp = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp) {
                lex_next(temp);
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
            if (is_negative) lex_next(lex);
            BppToken arg_tok = lex_next(lex);
            if (arg_tok.type == TOK_NUMBER) {
                double num_val = arg_tok.as.number;
                if (is_negative) num_val = -num_val;
                int base = 0;
                if (arg_tok.length > 2 && arg_tok.start[0] == '&') {
                    char b = (char)runtime_toupper((unsigned char)arg_tok.start[1]);
                    if (b == 'H') base = 16;
                    else if (b == 'O') base = 8;
                    else if (b == 'B') base = 2;
                    else if (runtime_isdigit((unsigned char)arg_tok.start[1])) base = 8;
                }

                double last = vm_get_last_rnd(vm);
                uint64_t seed = (uint64_t)(last * 4294967296.0);
                if (seed == 0) seed = 123456789ULL;
                seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                uint32_t raw_r = (uint32_t)(seed & 0xFFFFFFFF);
                vm_set_last_rnd(vm, (double)raw_r / 4294967296.0);

                if (base > 0) {
                    long max_val = (long)num_val;
                    long r_val = (max_val > 0) ? (long)(raw_r % (uint32_t)(max_val + 1)) : 0;
                    char buf[128] = "";
                    if (base == 16) runtime_snprintf(buf, sizeof(buf), "%X", (unsigned int)r_val);
                    else if (base == 8) runtime_snprintf(buf, sizeof(buf), "%o", (unsigned int)r_val);
                    else if (base == 2) {
                        char bin[64] = "";
                        int idx = 0;
                        unsigned long tmp = r_val;
                        if (tmp == 0) {
                            runtime_strcpy(buf, "0");
                        } else {
                            while (tmp > 0) {
                                bin[idx++] = (tmp & 1) ? '1' : '0';
                                tmp >>= 1;
                            }
                            for (int j = 0; j < idx; j++) buf[j] = bin[idx - 1 - j];
                            buf[idx] = '\0';
                        }
                    }
                    val.type = VAL_STRING;
                    val.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
                } else {
                    long limit = (long)num_val;
                    long r_val = (limit > 0) ? (long)(raw_r % (uint32_t)(limit + 1)) : ((limit < 0) ? -(long)(raw_r % (uint32_t)(-limit + 1)) : 0);
                    val.type = VAL_NUMBER;
                    val.as.number = (double)r_val;
                }
            } else if (arg_tok.type == TOK_IDENT || arg_tok.type == TOK_KEYWORD) {
                char arg_name[64] = "";
                size_t alen = (arg_tok.length < 63) ? arg_tok.length : 63;
                runtime_memcpy(arg_name, arg_tok.start, alen);
                arg_name[alen] = '\0';
                for (size_t k = 0; k < alen; k++) arg_name[k] = (char)runtime_toupper((unsigned char)arg_name[k]);

                double last = vm_get_last_rnd(vm);
                uint64_t seed = (uint64_t)(last * 4294967296.0);
                if (seed == 0) seed = 123456789ULL;
                seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                uint32_t raw_r = (uint32_t)(seed & 0xFFFFFFFF);
                vm_set_last_rnd(vm, (double)raw_r / 4294967296.0);

                if (runtime_strcmp(arg_name, "TIME") == 0) {
                    char buf[16];
                    runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", (int)(raw_r % 24), (int)((raw_r / 24) % 60), (int)((raw_r / 1440) % 60));
                    val.type = VAL_STRING;
                    val.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
                } else if (runtime_strcmp(arg_name, "TI") == 0) {
                    val.type = VAL_NUMBER;
                    val.as.number = (double)((raw_r % 24) * 10000 + ((raw_r / 24) % 60) * 100 + ((raw_r / 1440) % 60));
                } else if (runtime_strcmp(arg_name, "TIMER") == 0) {
                    val.type = VAL_NUMBER;
                    val.as.number = ((double)raw_r / 4294967296.0) * 86400.0;
                } else {
                    val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                }
            } else {
                val = eval_builtin_function(vm, name_buf, lex, false, out_err);
            }
        } else {
            val = eval_builtin_function(vm, name_buf, lex, false, out_err);
        }
    } else if (runtime_strcasecmp(name_buf, "COMMAND$") == 0 || runtime_strcasecmp(name_buf, "COMMAND") == 0) {
        const char *cmd = runtime_get_command_line();
        val.type = VAL_STRING;
        val.as.string = str_create(vm_get_str(vm), cmd ? cmd : "", cmd ? runtime_strlen(cmd) : 0);
    } else if (runtime_strcasecmp(name_buf, "DOEVENTS") == 0) {
        platform_sleep_ms(0);
        val.type = VAL_NUMBER;
        val.as.number = 0.0;
    } else {
        val = eval_builtin_function(vm, name_buf, lex, false, out_err);
    }

    return val;
}


bool eval_parse_identifier_expression(VMContext *vm, LexerContext *lex, BppToken tok, BValue *out_val, bool *out_is_func, BppError *out_err) {
    if (!vm || !lex || !out_val || !out_is_func || !out_err) return false;

    VariableContext *var = vm_get_var(vm);
    *out_is_func = false;

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

    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_NOTHING) ||
        (tok.type == TOK_IDENT && (runtime_strcasecmp(name_buf, "NOTHING") == 0 || runtime_strcasecmp(name_buf, "NULL") == 0))) {
        runtime_memset(out_val, 0, sizeof(*out_val));
        out_val->type = VAL_MAP;
        out_val->as.map = NULL;
        return true;
    }

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
    if (next_tok_type == TOK_LBRACKET) {
        *out_is_func = true;
        *out_val = eval_parse_string_slice(vm, lex, name_buf, TOK_LBRACKET, out_err);
        return (out_err->code == 0);
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
            *out_val = eval_builtin_function(vm, name_buf, lex, true, out_err);
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
        *out_val = eval_handle_special_builtin(vm, lex, name_buf, out_err);
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


// FILENAME: sub_invoke.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (sub_internal.h)
// Provides runtime implementation for the SUB_INVOKE statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/sub_internal.h"

//
// ---- User Function & Method Invocation ----

BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err) {
    BValue val;
    memset(&val, 0, sizeof(val));
    val.type = VAL_NUMBER;
    val.as.number = 0.0;

    if (!vm || !name) {
        if (err) { err->code = 5; err->message = "Null VM or name"; }
        return val;
    }

    bool found_def = false;
    BValue def_val = def_fn_eval(vm, name, args, argc, err, &found_def);
    if (found_def) {
        return def_val;
    }

    BppLineNumber target_line = 0;
    const char *target_text = NULL;
    bool is_lib = false;
    if (!find_procedure_ex(vm, name, KW_FUNCTION, &target_line, &target_text, &is_lib) &&
        !find_procedure_ex(vm, name, KW_SUB, &target_line, &target_text, &is_lib)) {
        if (err) { err->code = 35; err->message = "Undefined user FUNCTION or SUB"; }
        return val;
    }

    const char *dot = strchr(name, '.');
    const char *fn_var = dot ? (dot + 1) : name;

    FormalParam formal_params[16];
    memset(formal_params, 0, sizeof(formal_params));
    int param_count = parse_formal_params(vm_get_mem(vm), target_text, formal_params, 16);

    VariableContext *vc = vm_get_var(vm);
    ArrayContext *ac = vm_get_arr(vm);
    BValue old_vals[16];
    bool has_old[16] = {false};

    int arg_offset = 0;
    BValue old_this = {0};
    bool has_old_this = false;
    BValue old_me = {0};
    bool has_old_me = false;

    if (argc > 0 && args && args[0].type == VAL_MAP) {
        BValue *t_ptr = var_lookup(vc, "THIS", false);
        if (t_ptr) {
            has_old_this = true;
            old_this = *t_ptr;
            if (old_this.type == VAL_STRING && old_this.as.string) str_add_ref(old_this.as.string);
            else if (old_this.type == VAL_MAP && old_this.as.map) map_add_ref(old_this.as.map);
        }
        BValue *m_ptr = var_lookup(vc, "ME", false);
        if (m_ptr) {
            has_old_me = true;
            old_me = *m_ptr;
            if (old_me.type == VAL_STRING && old_me.as.string) str_add_ref(old_me.as.string);
            else if (old_me.type == VAL_MAP && old_me.as.map) map_add_ref(old_me.as.map);
        }
        var_assign(vc, "THIS", args[0]);
        var_assign(vc, "ME", args[0]);

        if (param_count > 0 && (strcasecmp(formal_params[0].name, "THIS") == 0 || strcasecmp(formal_params[0].name, "ME") == 0)) {
            arg_offset = 0;
        } else if (argc >= param_count + 1) {
            arg_offset = 1;
        }
    }

    for (int i = 0; i < param_count && i < 16; i++) {
        if (!formal_params[i].is_array) {
            BValue *old_ptr = var_lookup(vc, formal_params[i].name, false);
            if (old_ptr) {
                has_old[i] = true;
                old_vals[i] = *old_ptr;
                if (old_vals[i].type == VAL_STRING && old_vals[i].as.string) {
                    str_add_ref(old_vals[i].as.string);
                } else if (old_vals[i].type == VAL_MAP && old_vals[i].as.map) {
                    map_add_ref(old_vals[i].as.map);
                }
            }
            int actual_idx = i + arg_offset;
            if (actual_idx < argc && args) {
                var_assign(vc, formal_params[i].name, args[actual_idx]);
            } else if (formal_params[i].default_expr[0] != '\0') {
                LexerContext *def_lex = lex_init(vm_get_mem(vm), formal_params[i].default_expr);
                if (def_lex) {
                    BppError def_err;
                    memset(&def_err, 0, sizeof(def_err));
                    BValue evaled_def = eval_expression(vm, def_lex, &def_err);
                    if (def_err.code == 0) {
                        var_assign(vc, formal_params[i].name, evaled_def);
                    }
                    lex_shutdown(def_lex);
                }
            }
        } else {
            int actual_idx = i + arg_offset;
            if (actual_idx < argc && args && args[actual_idx].type == VAL_ARRAY_REF && args[actual_idx].as.string) {
                const char *passed_array_name = str_data(args[actual_idx].as.string);
                arr_create_alias(ac, formal_params[i].name, passed_array_name);
            }
        }
    }

    BValue old_fn_val = {0};
    bool has_old_fn = false;
    BValue *fn_ptr = var_lookup(vc, fn_var, false);
    if (fn_ptr) {
        has_old_fn = true;
        old_fn_val = *fn_ptr;
        if (old_fn_val.type == VAL_STRING && old_fn_val.as.string) {
            str_add_ref(old_fn_val.as.string);
        } else if (old_fn_val.type == VAL_MAP && old_fn_val.as.map) {
            map_add_ref(old_fn_val.as.map);
        }
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = is_lib ? mem_lib_program_get_all(mem, &count) : mem_program_get_all(mem, &count);

    BppLineNumber caller_line = vm_get_current_line(vm);
    BppLineNumber cur_exec_line = 0;
    const char *cur_exec_pos = NULL;

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number > target_line) {
            cur_exec_line = lines[i].line_number;
            break;
        }
    }

    int nest = 0;
    while (cur_exec_line > 0 && vm_is_running(vm) && !vm_has_error(vm)) {
        lines = is_lib ? mem_lib_program_get_all(mem, &count) : mem_program_get_all(mem, &count);
        size_t line_idx = 0;
        bool found_line = false;
        for (size_t i = 0; i < count; i++) {
            if (lines[i].line_number == cur_exec_line) {
                line_idx = i;
                found_line = true;
                break;
            }
        }
        if (!found_line) break;

        LexerContext *chk_lex = lex_init(mem, lines[line_idx].text);
        bool is_end_proc = false;
        if (chk_lex) {
            BppToken tok = lex_next(chk_lex);
            if (tok.type == TOK_NUMBER) tok = lex_next(chk_lex);
            if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_PUBLIC || tok.as.keyword == KW_PRIVATE)) {
                tok = lex_next(chk_lex);
            }
            if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION || tok.as.keyword == KW_DEF || tok.as.keyword == KW_PROCEDURE)) {
                nest++;
            } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
                BppToken ntok = lex_next(chk_lex);
                if (ntok.type == TOK_KEYWORD && (ntok.as.keyword == KW_SUB || ntok.as.keyword == KW_FUNCTION || ntok.as.keyword == KW_DEF || ntok.as.keyword == KW_PROCEDURE)) {
                    if (nest > 0) nest--;
                    else is_end_proc = true;
                }
            } else if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUBEND || tok.as.keyword == KW_FNEND)) {
                if (nest > 0) nest--;
                else is_end_proc = true;
            } else if (tok.type == TOK_IDENT) {
                if ((tok.length == 6 && strncasecmp(tok.start, "SUBEND", 6) == 0) ||
                    (tok.length == 6 && strncasecmp(tok.start, "ENDSUB", 6) == 0) ||
                    (tok.length == 11 && strncasecmp(tok.start, "ENDFUNCTION", 11) == 0) ||
                    (tok.length == 5 && strncasecmp(tok.start, "FNEND", 5) == 0)) {
                    if (nest > 0) nest--;
                    else is_end_proc = true;
                }
            }
            lex_shutdown(chk_lex);
        }
        if (is_end_proc) {
            break;
        }

        const char *to_exec = cur_exec_pos ? cur_exec_pos : lines[line_idx].text;
        cur_exec_pos = NULL;

        vm_set_current_line(vm, lines[line_idx].line_number);
        BppError line_err = vm_execute_line(vm, to_exec);
        if (line_err.code != 0) {
            if (err) *err = line_err;
            break;
        }

        if (vm_is_jump_active(vm)) {
            BppLineNumber target = 0;
            const char *pos = NULL;
            vm_get_jump_target(vm, &target, &pos);
            vm_clear_jump(vm);
            if (target > 0) cur_exec_line = target;
            cur_exec_pos = pos;
        } else {
            cur_exec_line = 0;
            for (size_t k = line_idx + 1; k < count; k++) {
                if (lines[k].line_number > lines[line_idx].line_number) {
                    cur_exec_line = lines[k].line_number;
                    break;
                }
            }
        }
    }

    BValue *res_ptr = var_lookup(vc, fn_var, false);
    if (!res_ptr && strncmp(fn_var, "OPERATOR_", 9) == 0) {
        res_ptr = var_lookup(vc, "OPERATOR", false);
    }
    if (res_ptr) {
        val = *res_ptr;
        if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
        else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
    }

    if (has_old_fn) {
        var_assign(vc, fn_var, old_fn_val);
        if (old_fn_val.type == VAL_STRING && old_fn_val.as.string) {
            str_release(vm_get_str(vm), old_fn_val.as.string);
        } else if (old_fn_val.type == VAL_MAP && old_fn_val.as.map) {
            map_release(vm_get_str(vm), old_fn_val.as.map);
        }
    } else {
        BValue def_num = {.type = VAL_NUMBER, .as.number = 0.0};
        var_assign(vc, fn_var, def_num);
    }

    for (int i = 0; i < param_count && i < 16; i++) {
        if (!formal_params[i].is_array) {
            if (has_old[i]) {
                var_assign(vc, formal_params[i].name, old_vals[i]);
                if (old_vals[i].type == VAL_STRING && old_vals[i].as.string) {
                    str_release(vm_get_str(vm), old_vals[i].as.string);
                } else if (old_vals[i].type == VAL_MAP && old_vals[i].as.map) {
                    map_release(vm_get_str(vm), old_vals[i].as.map);
                }
            } else {
                BValue def_num = {.type = VAL_NUMBER, .as.number = 0.0};
                var_assign(vc, formal_params[i].name, def_num);
            }
        } else {
            arr_remove_alias(ac, formal_params[i].name);
        }
    }

    if (has_old_this) {
        var_assign(vc, "THIS", old_this);
        if (old_this.type == VAL_STRING && old_this.as.string) str_release(vm_get_str(vm), old_this.as.string);
        else if (old_this.type == VAL_MAP && old_this.as.map) map_release(vm_get_str(vm), old_this.as.map);
    } else {
        BValue def_map = {.type = VAL_MAP, .as.map = NULL};
        var_assign(vc, "THIS", def_map);
    }

    if (has_old_me) {
        var_assign(vc, "ME", old_me);
        if (old_me.type == VAL_STRING && old_me.as.string) str_release(vm_get_str(vm), old_me.as.string);
        else if (old_me.type == VAL_MAP && old_me.as.map) map_release(vm_get_str(vm), old_me.as.map);
    } else {
        BValue def_map = {.type = VAL_MAP, .as.map = NULL};
        var_assign(vc, "ME", def_map);
    }

    vm_set_current_line(vm, caller_line);
    return val;
}

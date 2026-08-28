// FILENAME: sub_exec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (sub_internal.h)
// Provides runtime implementation for the SUB_EXEC statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/sub_internal.h"

//
// ---- Subroutine Call Execution ----

BppError vm_call_sub_procedure(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected SUB procedure name";
        return err;
    }

    char sub_name[128] = {0};
    size_t len = (name_tok.length < sizeof(sub_name) - 1) ? name_tok.length : sizeof(sub_name) - 1;
    memcpy(sub_name, name_tok.start, len);
    sub_name[len] = '\0';

    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub_tok = lex_next(lex);
        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected identifier after '.' in procedure call";
            return err;
        }
        char sub_part[64];
        size_t slen = (sub_tok.length < sizeof(sub_part) - 1) ? sub_tok.length : sizeof(sub_part) - 1;
        memcpy(sub_part, sub_tok.start, slen);
        sub_part[slen] = '\0';

        char combined[128];
        snprintf(combined, sizeof(combined), "%s.%s", sub_name, sub_part);
        strncpy(sub_name, combined, sizeof(sub_name) - 1);
        sub_name[sizeof(sub_name) - 1] = '\0';
    }

    BppLineNumber target_line = 0;
    const char *target_text = NULL;
    bool is_lib = false;
    BValue obj_this;
    memset(&obj_this, 0, sizeof(obj_this));
    bool has_obj_this = false;

    if (!find_procedure_ex(vm, sub_name, KW_SUB, &target_line, &target_text, &is_lib)) {
        if (strchr(sub_name, '.') != NULL) {
            char base_name[64] = {0};
            const char *dot_pos = strchr(sub_name, '.');
            size_t b_len = (size_t)(dot_pos - sub_name);
            if (b_len < sizeof(base_name)) {
                memcpy(base_name, sub_name, b_len);
                base_name[b_len] = '\0';
                const char *method_name = dot_pos + 1;
                BValue *var_val = var_lookup(vm_get_var(vm), base_name, false);
                if (var_val && var_val->type == VAL_MAP && var_val->as.map) {
                    BValue type_val;
                    if (map_get(var_val->as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                        char class_method[128];
                        snprintf(class_method, sizeof(class_method), "%s.%s", str_data(type_val.as.string), method_name);
                        if (find_procedure_ex(vm, class_method, KW_SUB, &target_line, &target_text, &is_lib)) {
                            strncpy(sub_name, class_method, sizeof(sub_name) - 1);
                            sub_name[sizeof(sub_name) - 1] = '\0';
                            obj_this = *var_val;
                            map_add_ref(obj_this.as.map);
                            has_obj_this = true;
                        }
                    }
                }
            }
        }
        if (!has_obj_this && target_line == 0) {
            err.code = 35; err.message = "Undefined SUB procedure";
            return err;
        }
    }

    FormalParam formal_params[16];
    memset(formal_params, 0, sizeof(formal_params));
    int param_count = parse_formal_params(vm_get_mem(vm), target_text, formal_params, 16);

    BValue args[16];
    char arg_var_names[16][64];
    bool is_arg_byref[16];
    int argc = parse_call_args(vm, lex, formal_params, param_count, args, arg_var_names, is_arg_byref, 16, &err);
    if (err.code != 0) return err;

    VariableContext *vc = vm_get_var(vm);
    ArrayContext *ac = vm_get_arr(vm);
    BValue old_vals[16];
    bool has_old[16] = {false};

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
            if (i < argc) {
                var_assign(vc, formal_params[i].name, args[i]);
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
            if (i < argc && args[i].type == VAL_ARRAY_REF && args[i].as.string) {
                const char *passed_array_name = str_data(args[i].as.string);
                arr_create_alias(ac, formal_params[i].name, passed_array_name);
            }
        }
    }

    if (has_obj_this) {
        var_assign(vc, "THIS", obj_this);
        var_assign(vc, "ME", obj_this);
    }

    if (!vm_sub_push(vm, sub_name, target_line, NULL, false)) {
        err.code = 14; err.message = "Out of memory in SUB call stack";
        if (has_obj_this && obj_this.type == VAL_MAP && obj_this.as.map) map_release(vm_get_str(vm), obj_this.as.map);
        return err;
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
        BppSubFrame cur_f;
        if (!vm_sub_peek(vm, &cur_f) || strcasecmp(cur_f.name, sub_name) != 0) {
            break;
        }

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
            err = line_err;
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

    BppSubFrame pop_f;
    if (vm_sub_peek(vm, &pop_f) && strcasecmp(pop_f.name, sub_name) == 0) {
        vm_sub_pop(vm, &pop_f);
    }
    for (int i = 0; i < param_count && i < 16; i++) {
        if (!formal_params[i].is_array) {
            if (is_arg_byref[i] && arg_var_names[i][0] != '\0') {
                BValue *final_val = var_lookup(vc, formal_params[i].name, false);
                if (final_val) {
                    var_assign(vc, arg_var_names[i], *final_val);
                }
            }
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

    if (has_obj_this) {
        if (obj_this.type == VAL_MAP && obj_this.as.map) {
            map_release(vm_get_str(vm), obj_this.as.map);
        }
    }

    vm_set_current_line(vm, caller_line);
    return err;
}

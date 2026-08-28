// FILENAME: exec_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (exec_control_internal.h)
// Implements bytecode virtual machine execution and state for exec_stmt.
//
// ---- Includes ----

#include "vm/exec_control_internal.h"

//
// ---- Subroutine Hook Dispatcher ----

BppError dispatch_gosub_target(VMContext *vm, LexerContext *lex, BppLineNumber line) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!mem_program_get(vm_get_mem(vm), line)) {
        err.code = 8;
        err.message = "Undefined line number in GOSUB hook/override";
        return err;
    }
    if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 12;
        err.message = "Subroutine nesting limit exceeded";
        return err;
    }
    vm_jump(vm, line, NULL);
    return err;
}

//
// ---- Single Statement Execution ----

BppError execute_single_statement(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    vm->current_stmt_pos = tok.start;
    BppKeywordId kw = KW_NONE;

    if (tok.type == TOK_DOCSTRING || tok.type == TOK_NAMESPACE_DECL || tok.type == TOK_GLOBAL_LABEL || tok.type == TOK_EOL || tok.type == TOK_EOF) {
        if (tok.type != TOK_EOF) {
            lex_next(lex);
        }
        return err;
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char word_buf[64];
        size_t w_len = (tok.length < 63) ? tok.length : 63;
        memcpy(word_buf, tok.start, w_len);
        word_buf[w_len] = '\0';

        const char *expansion = vm_lookup_alias(vm, word_buf);
        if (expansion && vm->alias_expansion_depth < 10) {
            lex_next(lex);
            const char *rest = lex_get_pos(lex);

            char *expanded = (char *)mem_scratch_alloc(vm_get_mem(vm), strlen(expansion) + strlen(rest) + 2);
            if (!expanded) {
                err.code = 14;
                err.message = "Scratch memory exhausted during alias expansion";
                return err;
            }
            snprintf(expanded, strlen(expansion) + strlen(rest) + 2, "%s %s", expansion, rest);

            while (tok.type != TOK_EOF && tok.type != TOK_EOL) {
                lex_next(lex);
                tok = lex_peek(lex);
            }

            vm->alias_expansion_depth++;
            err = vm_execute_line(vm, expanded);
            vm->alias_expansion_depth--;
            return err;
        }
    }

    const char *initial_stmt_head_pos = lex_get_pos(lex);

    if (!vm_check_watchdog(vm, &err)) {
        return err;
    }

    if (tok.type == TOK_DIRECTIVE) {
        lex_next(lex);
        char dir_name[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(dir_name, tok.as.string, len);
        dir_name[len] = '\0';

        if (strcasecmp(dir_name, "KEYWORD") == 0 || strcasecmp(dir_name, "SCOPE") == 0 || strcasecmp(dir_name, "ALIAS") == 0 || strcasecmp(dir_name, "OPTION") == 0) {
            err = skip_metadata_block(vm, lex, dir_name);
            return err;
        }

        err = execute_directive(vm, lex, tok);
        return err;
    }

    if (tok.type == TOK_PERIOD && vm_with_stack_peek(vm) != NULL) {
        bool is_method_call = false;
        LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (temp_lex) {
            lex_next(temp_lex);
            BppToken sub = lex_next(temp_lex);
            (void)sub;
            while (lex_peek(temp_lex).type == TOK_PERIOD) {
                lex_next(temp_lex);
                lex_next(temp_lex);
            }
            if (lex_peek(temp_lex).type == TOK_LPAREN) {
                is_method_call = true;
            }
            lex_shutdown(temp_lex);
        }

        if (is_method_call) {
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }
        kw = KW_LET;
    } else if (tok.type == TOK_KEYWORD) {
        kw = tok.as.keyword;
        bool is_assignment = false;
        bool is_method_call = false;

        if (kw != KW_ASSERT && kw != KW_PRINT && kw != KW_IF && kw != KW_UNLESS && kw != KW_WHILE &&
            kw != KW_UNTIL && kw != KW_CASE && kw != KW_SELECT && kw != KW_RETURN &&
            kw != KW_ON && kw != KW_DEF && kw != KW_FOR && kw != KW_NEXT &&
            kw != KW_GOTO && kw != KW_GOSUB && kw != KW_DIM && kw != KW_REDIM &&
            kw != KW_INPUT && kw != KW_READ && kw != KW_DATA && kw != KW_REM &&
            kw != KW_VIEWPORT && kw != KW_WINDOW && kw != KW_SCREEN && kw != KW_COLOR) {
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {
                lex_next(temp_lex);
                bool has_dot = false;
                while (lex_peek(temp_lex).type == TOK_PERIOD) {
                    has_dot = true;
                    lex_next(temp_lex);
                    lex_next(temp_lex);
                }
                BppToken next_tok = lex_peek(temp_lex);
                if (next_tok.type == TOK_LPAREN) {
                    if (has_dot) is_method_call = true;
                    int p_depth = 0;
                    while (next_tok.type != TOK_EOF && next_tok.type != TOK_EOL) {
                        if (next_tok.type == TOK_LPAREN) p_depth++;
                        else if (next_tok.type == TOK_RPAREN) {
                            p_depth--;
                            if (p_depth == 0) {
                                lex_next(temp_lex);
                                break;
                            }
                        }
                        lex_next(temp_lex);
                        next_tok = lex_peek(temp_lex);
                    }
                    next_tok = lex_peek(temp_lex);
                }
                if (next_tok.type == TOK_EQ) {
                    is_assignment = true;
                    is_method_call = false;
                }
                lex_shutdown(temp_lex);
            }
        }

        if (is_method_call) {
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }

        if (is_assignment || kw == KW_LET) {
            kw = KW_LET;
        } else {
            lex_next(lex);
        }
    } else if (tok.type == TOK_IDENT) {
        bool is_method_call = false;
        if (memchr(tok.start, '.', tok.length) != NULL) {
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {
                lex_next(temp_lex);
                BppToken next_tok = lex_next(temp_lex);
                if (next_tok.type == TOK_LPAREN) is_method_call = true;
                lex_shutdown(temp_lex);
            }
        }

        if (is_method_call) {
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }

        bool is_assign = false;
        LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (temp_lex) {
            lex_next(temp_lex);
            while (lex_peek(temp_lex).type == TOK_PERIOD) {
                lex_next(temp_lex);
                lex_next(temp_lex);
            }
            BppToken next_tok = lex_peek(temp_lex);
            if (next_tok.type == TOK_LPAREN) {
                int p_depth = 0;
                while (next_tok.type != TOK_EOF && next_tok.type != TOK_EOL) {
                    if (next_tok.type == TOK_LPAREN) p_depth++;
                    else if (next_tok.type == TOK_RPAREN) {
                        p_depth--;
                        if (p_depth == 0) {
                            lex_next(temp_lex);
                            break;
                        }
                    }
                    lex_next(temp_lex);
                    next_tok = lex_peek(temp_lex);
                }
                next_tok = lex_peek(temp_lex);
            }
            if (next_tok.type == TOK_EQ) {
                is_assign = true;
            }
            lex_shutdown(temp_lex);
        }

        if (is_assign) {
            kw = KW_LET;
        } else {
            char sub_name[256];
            size_t slen = (tok.length < sizeof(sub_name) - 1) ? tok.length : sizeof(sub_name) - 1;
            memcpy(sub_name, tok.start, slen);
            sub_name[slen] = '\0';
            if (find_procedure(vm, sub_name, KW_SUB, NULL, NULL)) {
                kw = KW_CALL;
            } else {
                kw = KW_LET;
            }
        }
    } else if (tok.type == TOK_LPAREN || tok.type == TOK_LBRACKET) {
        BppTokenType open_type = tok.type;
        BppTokenType match_type = (open_type == TOK_LPAREN) ? TOK_RPAREN : TOK_RBRACKET;
        lex_next(lex);
        const char *inner_start = lex_get_pos(lex);
        const char *inner_end = NULL;
        int depth = 1;
        while (true) {
            BppToken t = lex_peek(lex);
            if (t.type == TOK_EOF || (t.type == TOK_EOL && t.start && *t.start == '\n')) {
                err.code = 2;
                err.message = "Unmatched parenthesis or bracket in statement block";
                return err;
            }
            if (t.type == open_type) {
                depth++;
            } else if (t.type == match_type) {
                depth--;
                if (depth == 0) {
                    inner_end = t.start;
                    lex_next(lex);
                    break;
                }
            }
            lex_next(lex);
        }

        size_t inner_len = (inner_end >= inner_start) ? (size_t)(inner_end - inner_start) : 0;
        char *inner_source = (char *)calloc(1, inner_len + 1);
        if (!inner_source) {
            err.code = 14; err.message = "Out of memory in statement block";
            return err;
        }
        if (inner_len > 0) {
            memcpy(inner_source, inner_start, inner_len);
            inner_source[inner_len] = '\0';
        }

        BppToken post_tok = lex_peek(lex);
        if (post_tok.type == TOK_KEYWORD && post_tok.as.keyword == KW_FOR) {
            lex_next(lex);
            BppToken fvar_tok = lex_next(lex);
            char fvar_name[64] = {0};
            size_t nlen = (fvar_tok.length < 63) ? fvar_tok.length : 63;
            memcpy(fvar_name, fvar_tok.start, nlen);
            fvar_name[nlen] = '\0';
            lex_next(lex);
            BValue fstart = eval_expression(vm, lex, &err);
            lex_next(lex);
            BValue fend = eval_expression(vm, lex, &err);
            double fstep = 1.0;
            BppToken step_tok = lex_peek(lex);
            if (step_tok.type == TOK_KEYWORD && (step_tok.as.keyword == KW_STEP || step_tok.as.keyword == KW_BY)) {
                lex_next(lex);
                BValue fstep_val = eval_expression(vm, lex, &err);
                fstep = fstep_val.as.number;
            }
            BppToken cond_tok = lex_peek(lex);
            const char *cond_pos = NULL;
            BppKeywordId cond_kw = KW_NONE;
            if (cond_tok.type == TOK_KEYWORD && (cond_tok.as.keyword == KW_IF || cond_tok.as.keyword == KW_UNLESS)) {
                cond_kw = cond_tok.as.keyword;
                cond_pos = lex_get_pos(lex);
                eval_expression(vm, lex, &err);
            }

            double cur_val = fstart.as.number;
            int max_iter = 1000000;
            int iter = 0;
            VariableContext *vc = vm_get_var(vm);
            while ((fstep >= 0.0 ? cur_val <= fend.as.number : cur_val >= fend.as.number) && iter < max_iter) {
                var_assign(vc, fvar_name, (BValue){.type = VAL_NUMBER, .as.number = cur_val});
                if (cond_pos) {
                    LexerContext *cond_lex = lex_init(vm_get_mem(vm), cond_pos);
                    lex_next(cond_lex);
                    BValue cval = eval_expression(vm, cond_lex, &err);
                    lex_shutdown(cond_lex);
                    bool truth = (cval.as.number != 0.0);
                    if ((cond_kw == KW_IF && !truth) || (cond_kw == KW_UNLESS && truth)) {
                        cur_val += fstep;
                        iter++;
                        continue;
                    }
                }
                err = vm_execute_line(vm, inner_source);
                if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
                cur_val += fstep;
                iter++;
            }
            var_assign(vc, fvar_name, (BValue){.type = VAL_NUMBER, .as.number = cur_val});
        } else if (post_tok.type == TOK_KEYWORD && post_tok.as.keyword == KW_WHILE) {
            const char *cond_pos = lex_get_pos(lex);
            lex_next(lex);
            eval_expression(vm, lex, &err);
            int max_iter = 1000000;
            int iter = 0;
            while (iter < max_iter) {
                LexerContext *cond_lex = lex_init(vm_get_mem(vm), cond_pos);
                lex_next(cond_lex);
                BValue cval = eval_expression(vm, cond_lex, &err);
                lex_shutdown(cond_lex);
                if (err.code != 0 || cval.as.number == 0.0) break;
                err = vm_execute_line(vm, inner_source);
                if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
                iter++;
            }
        } else if (post_tok.type == TOK_KEYWORD && post_tok.as.keyword == KW_UNTIL) {
            const char *cond_pos = lex_get_pos(lex);
            lex_next(lex);
            eval_expression(vm, lex, &err);
            int max_iter = 1000000;
            int iter = 0;
            while (iter < max_iter) {
                err = vm_execute_line(vm, inner_source);
                if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
                LexerContext *cond_lex = lex_init(vm_get_mem(vm), cond_pos);
                lex_next(cond_lex);
                BValue cval = eval_expression(vm, cond_lex, &err);
                lex_shutdown(cond_lex);
                if (err.code != 0 || cval.as.number != 0.0) break;
                iter++;
            }
        } else if (post_tok.type == TOK_KEYWORD && (post_tok.as.keyword == KW_IF || post_tok.as.keyword == KW_UNLESS)) {
            BppKeywordId ikw = post_tok.as.keyword;
            lex_next(lex);
            BValue cval = eval_expression(vm, lex, &err);
            bool truth = (cval.as.number != 0.0);
            if ((ikw == KW_IF && truth) || (ikw == KW_UNLESS && !truth)) {
                err = vm_execute_line(vm, inner_source);
            }
        } else {
            err = vm_execute_line(vm, inner_source);
        }

        free(inner_source);
        return err;
    } else if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        return err;
    } else {
        err.code = 2;
        err.message = "Expected statement keyword or assignment";
        return err;
    }

    if (security_is_keyword_restricted((int)kw)) {
        err.code = 70;
        err.message = "Keyword restricted via SECURITY RESTRICT";
        return err;
    }

    if (kw >= 1000) {
        err = execute_custom_keyword_statement(vm, lex, kw);
        return err;
    }

    int postfix_count = 0;
    const char *postfix_ptrs[8];
    BppKeywordId postfix_kws[8];
    int loop_idx = -1;

    BppTokenType cur_tok_type = tok.type;
    if (cur_tok_type != TOK_KEYWORD || !is_postfix_exempt_keyword(kw)) {
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (scan_lex) {
            int p_depth = 0;
            int b_depth = 0;
            while (true) {
                BppToken s_tok = lex_next(scan_lex);
                if (s_tok.type == TOK_EOF || s_tok.type == TOK_EOL || s_tok.type == TOK_BACKSLASH) {
                    break;
                }
                if (s_tok.type == TOK_LPAREN) {
                    p_depth++;
                } else if (s_tok.type == TOK_RPAREN) {
                    if (p_depth > 0) p_depth--;
                } else if (s_tok.type == TOK_LBRACKET) {
                    b_depth++;
                } else if (s_tok.type == TOK_RBRACKET) {
                    if (b_depth > 0) b_depth--;
                } else if (p_depth == 0 && b_depth == 0 && s_tok.type == TOK_KEYWORD) {
                    BppKeywordId s_kw = s_tok.as.keyword;
                    if (s_kw == KW_IF || s_kw == KW_UNLESS || s_kw == KW_WHILE || s_kw == KW_UNTIL || (s_kw == KW_FOR && kw != KW_OPEN)) {
                        if (postfix_count < 8) {
                            if (s_kw == KW_WHILE || s_kw == KW_UNTIL || s_kw == KW_FOR) {
                                loop_idx = postfix_count;
                            }
                            postfix_ptrs[postfix_count] = s_tok.start;
                            postfix_kws[postfix_count] = s_kw;
                            postfix_count++;
                        }
                    }
                }
            }
            lex_shutdown(scan_lex);
        }
    }

    if (loop_idx != -1 && postfix_kws[loop_idx] == KW_WHILE) {
        const char *p_pos = postfix_ptrs[loop_idx];
        char *mut_body = (char *)postfix_ptrs[0];
        int max_iter = 1000000;
        int iter = 0;
        while (iter < max_iter) {
            LexerContext *cond_lex = lex_init(vm_get_mem(vm), p_pos);
            lex_next(cond_lex);
            BValue cval = eval_expression(vm, cond_lex, &err);
            lex_shutdown(cond_lex);
            if (err.code != 0 || cval.as.number == 0.0) break;

            bool cond_pass = true;
            for (int ci = 0; ci < postfix_count; ci++) {
                if (ci == loop_idx) continue;
                LexerContext *sub_lex = lex_init(vm_get_mem(vm), postfix_ptrs[ci]);
                BppToken ctok = lex_next(sub_lex);
                BValue sub_val = eval_expression(vm, sub_lex, &err);
                lex_shutdown(sub_lex);
                if (err.code != 0) { cond_pass = false; break; }
                bool truth = (sub_val.as.number != 0.0);
                if ((ctok.as.keyword == KW_IF && !truth) || (ctok.as.keyword == KW_UNLESS && truth)) {
                    cond_pass = false;
                    break;
                }
            }
            if (!cond_pass) {
                iter++;
                continue;
            }

            char saved = *mut_body;
            *mut_body = '\0';
            const char *loop_target = vm->active_line_copy ? vm->active_line_copy : initial_stmt_head_pos;
            err = vm_execute_line(vm, loop_target);
            *mut_body = saved;
            if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
            iter++;
        }
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_BACKSLASH) lex_next(lex);
        return err;
    }

    if (loop_idx != -1 && postfix_kws[loop_idx] == KW_UNTIL) {
        const char *p_pos = postfix_ptrs[loop_idx];
        char *mut_body = (char *)postfix_ptrs[0];
        int max_iter = 1000000;
        int iter = 0;
        while (iter < max_iter) {
            LexerContext *cond_lex = lex_init(vm_get_mem(vm), p_pos);
            lex_next(cond_lex);
            BValue cval = eval_expression(vm, cond_lex, &err);
            lex_shutdown(cond_lex);
            if (err.code != 0 || cval.as.number != 0.0) break;

            bool cond_pass = true;
            for (int ci = 0; ci < postfix_count; ci++) {
                if (ci == loop_idx) continue;
                LexerContext *sub_lex = lex_init(vm_get_mem(vm), postfix_ptrs[ci]);
                BppToken ctok = lex_next(sub_lex);
                BValue sub_val = eval_expression(vm, sub_lex, &err);
                lex_shutdown(sub_lex);
                if (err.code != 0) { cond_pass = false; break; }
                bool truth = (sub_val.as.number != 0.0);
                if ((ctok.as.keyword == KW_IF && !truth) || (ctok.as.keyword == KW_UNLESS && truth)) {
                    cond_pass = false;
                    break;
                }
            }
            if (!cond_pass) {
                iter++;
                continue;
            }

            char saved = *mut_body;
            *mut_body = '\0';
            const char *loop_target = vm->active_line_copy ? vm->active_line_copy : initial_stmt_head_pos;
            err = vm_execute_line(vm, loop_target);
            *mut_body = saved;
            if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
            iter++;
        }
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_BACKSLASH) lex_next(lex);
        return err;
    }

    if (loop_idx != -1 && postfix_kws[loop_idx] == KW_FOR) {
        const char *p_pos = postfix_ptrs[loop_idx];
        LexerContext *for_lex = lex_init(vm_get_mem(vm), p_pos);
        lex_next(for_lex);
        BppToken fvar_tok = lex_next(for_lex);
        char fvar_name[64] = {0};
        memcpy(fvar_name, fvar_tok.start, fvar_tok.length < 63 ? fvar_tok.length : 63);
        lex_next(for_lex);
        BValue fstart = eval_expression(vm, for_lex, &err);
        lex_next(for_lex);
        BValue fend = eval_expression(vm, for_lex, &err);
        double fstep = 1.0;
        BppToken step_tok = lex_peek(for_lex);
        if (step_tok.type == TOK_KEYWORD && (step_tok.as.keyword == KW_STEP || step_tok.as.keyword == KW_BY)) {
            lex_next(for_lex);
            BValue fstep_val = eval_expression(vm, for_lex, &err);
            fstep = fstep_val.as.number;
        }
        lex_shutdown(for_lex);

        double cur_val = fstart.as.number;
        int max_iter = 1000000;
        int iter = 0;
        VariableContext *vc = vm_get_var(vm);
        char *mut_body = (char *)postfix_ptrs[0];
        while ((fstep >= 0.0 ? cur_val <= fend.as.number : cur_val >= fend.as.number) && iter < max_iter) {
            var_assign(vc, fvar_name, (BValue){.type = VAL_NUMBER, .as.number = cur_val});

            bool cond_pass = true;
            for (int ci = 0; ci < postfix_count; ci++) {
                if (ci == loop_idx) continue;
                LexerContext *sub_lex = lex_init(vm_get_mem(vm), postfix_ptrs[ci]);
                BppToken ctok = lex_next(sub_lex);
                BValue sub_val = eval_expression(vm, sub_lex, &err);
                lex_shutdown(sub_lex);
                if (err.code != 0) { cond_pass = false; break; }
                bool truth = (sub_val.as.number != 0.0);
                if ((ctok.as.keyword == KW_IF && !truth) || (ctok.as.keyword == KW_UNLESS && truth)) {
                    cond_pass = false;
                    break;
                }
            }
            if (!cond_pass) {
                cur_val += fstep;
                iter++;
                continue;
            }

            char saved = *mut_body;
            *mut_body = '\0';
            const char *loop_target = vm->active_line_copy ? vm->active_line_copy : initial_stmt_head_pos;
            err = vm_execute_line(vm, loop_target);
            *mut_body = saved;
            if (err.code != 0 || vm_exit_requested(vm) || (vm->current_line > 0.0 && !vm->running)) break;
            cur_val += fstep;
            iter++;
        }
        var_assign(vc, fvar_name, (BValue){.type = VAL_NUMBER, .as.number = cur_val});
        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_BACKSLASH) lex_next(lex);
        return err;
    }

    bool condition_met = true;
    const char *postfix_end_pos = NULL;

    for (int i = postfix_count - 1; i >= 0; i--) {
        const char *p_pos = postfix_ptrs[i];
        BppKeywordId p_kw = postfix_kws[i];

        char saved_char = '\0';
        char *mutable_next = NULL;
        if (i + 1 < postfix_count) {
            mutable_next = (char *)postfix_ptrs[i + 1];
            saved_char = *mutable_next;
            *mutable_next = '\0';
        }

        LexerContext *cond_lex = lex_init(vm_get_mem(vm), p_pos);
        if (cond_lex) {
            lex_next(cond_lex);
            BValue cond_val = eval_expression(vm, cond_lex, &err);
            if (err.code == 0) {
                if (cond_val.type == VAL_STRING) {
                    if (cond_val.as.string) str_release(vm_get_str(vm), cond_val.as.string);
                    err.code = 13;
                    err.message = "Type mismatch in conditional expression";
                    condition_met = false;
                } else if (cond_val.type == VAL_NUMBER || cond_val.type == VAL_INTEGER) {
                    bool truth = (cond_val.as.number != 0.0);
                    if (p_kw == KW_IF && !truth) {
                        condition_met = false;
                    } else if (p_kw == KW_UNLESS && truth) {
                        condition_met = false;
                    } else if (p_kw == KW_UNTIL && truth) {
                        condition_met = false;
                    }
                }
            } else {
                condition_met = false;
            }
            if (postfix_end_pos == NULL) {
                postfix_end_pos = lex_get_pos(cond_lex);
            }
            lex_shutdown(cond_lex);
        }

        if (mutable_next) {
            *mutable_next = saved_char;
        }

        if (!condition_met) {
            break;
        }
    }

    if (!condition_met) {
        if (postfix_count > 0 && postfix_ptrs[0]) {
            while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_BACKSLASH) {
                lex_next(lex);
            }
        }
        return err;
    }

    BppStmtHandler handler = stmt_lookup(vm->stmt_reg, kw);
    if (!handler) {
        err.code = 2;
        err.message = "Unknown command";
        if (vm->opt_eh) err.message = "Eh?";
        return err;
    }

    char *mutable_postfix = (postfix_count > 0) ? (char *)postfix_ptrs[0] : NULL;
    char saved_char = '\0';
    if (postfix_count > 0 && mutable_postfix) {
        saved_char = *mutable_postfix;
        *mutable_postfix = '\0';
    }

    const char *kw_name = lex_keyword_name(kw);

    if (kw_name && scope_is_keyword_disabled(vm, kw_name)) {
        err.code = 13;
        err.message = "Keyword disabled by SCOPE";
        return err;
    }

    const BppScopeHook *before_hk = kw_name ? scope_lookup_hook(vm, kw_name, HOOK_BEFORE) : NULL;
    if (before_hk && before_hk->target_line > 0) {
        dispatch_gosub_target(vm, lex, (BppLineNumber)before_hk->target_line);
    }

    const BppOverrideEntry *ovr = kw_name ? override_lookup(vm, kw_name) : NULL;
    const BppScopeHook *ovr_hk = kw_name ? scope_lookup_hook(vm, kw_name, HOOK_OVERRIDE) : NULL;

    if (ovr && ovr->is_active) {
        if (ovr->target_line > 0) {
            err = dispatch_gosub_target(vm, lex, (BppLineNumber)ovr->target_line);
        } else if (ovr->replacement_sub[0] != '\0') {
            BValue res = invoke_user_function(vm, ovr->replacement_sub, NULL, 0, &err);
            if (res.type == VAL_STRING && res.as.string) str_release(vm_get_str(vm), res.as.string);
            else if (res.type == VAL_MAP && res.as.map) map_release(vm_get_str(vm), res.as.map);
        }
    } else if (ovr_hk) {
        if (ovr_hk->target_line > 0) {
            err = dispatch_gosub_target(vm, lex, (BppLineNumber)ovr_hk->target_line);
        }
    } else {
        err = handler(vm, lex);
    }

    const BppScopeHook *after_hk = (err.code == 0 && kw_name) ? scope_lookup_hook(vm, kw_name, HOOK_AFTER) : NULL;
    if (after_hk && after_hk->target_line > 0) {
        dispatch_gosub_target(vm, lex, (BppLineNumber)after_hk->target_line);
    }

    if (postfix_count > 0 && mutable_postfix) {
        *mutable_postfix = saved_char;
    }

    if (err.code == 0 && postfix_count > 0) {
        lex_set_pos(lex, postfix_end_pos);
    }
    if (err.code != 0 && vm->opt_eh) {
        err.message = "Eh?";
    }
    return err;
}

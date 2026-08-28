// FILENAME: ast_parse_block.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ast_internal.h, eval_internal.h, lexer.h, lexer.c, vm.h)
// NEEDS: libplatform (platform.h)
// Implements AST parsing and evaluation structures for ast_parse_block.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "eval/eval_internal.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "vm/vm.h"

//
// ---- Multi-Line Block Compilers ----

// compiles a multi-line IF...THEN...ELSE...END IF block into an AST node
EvalAstNode *eval_ast_try_compile_multiline_if(VMContext *vm, const BppProgramLine *lines, size_t start_idx, size_t total_count, size_t *out_lines_skipped) {
    if (!vm || !lines || start_idx >= total_count || !out_lines_skipped) return NULL;
    *out_lines_skipped = 0;

    const char *header = skip_line_number_and_spaces(lines[start_idx].text);
    if (!header || !*header) return NULL;
    if (runtime_strncasecmp(header, "IF", 2) != 0 || (!runtime_isspace((unsigned char)header[2]) && !runtime_isalpha((unsigned char)header[2]))) return NULL;


    LexerContext *lex = lex_init(vm_get_mem(vm), header);
    if (!lex) return NULL;
    if (lex_peek(lex).type == TOK_NUMBER) {
        lex_next(lex);
    }

    BppToken tok = lex_next(lex);
    if (!is_tok_kw(tok, KW_IF, "IF", 2)) {
        lex_shutdown(lex);
        return NULL;
    }

    EvalAstNode *cond = eval_ast_parse_expression(lex);
    if (!cond) {
        lex_shutdown(lex);
        return NULL;
    }

    BppToken ttok = lex_peek(lex);
    if (!is_tok_kw(ttok, KW_THEN, "THEN", 4)) {
        eval_ast_free_tree(cond);
        lex_shutdown(lex);
        return NULL;
    }
    lex_next(lex);

    if (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        eval_ast_free_tree(cond);
        lex_shutdown(lex);
        return NULL;
    }
    lex_shutdown(lex);

    EvalAstNode *then_head = NULL;
    EvalAstNode *then_tail = NULL;
    EvalAstNode *else_head = NULL;
    EvalAstNode *else_tail = NULL;
    bool in_else = false;
    bool found_endif = false;
    size_t end_idx = start_idx + 1;
    int if_depth = 1;

    for (; end_idx < total_count; end_idx++) {
        const char *ltext = skip_line_number_and_spaces(lines[end_idx].text);
        if (!ltext || *ltext == '\0' || *ltext == '\'' || runtime_strncasecmp(ltext, "REM", 3) == 0) continue;

        if ((runtime_strncasecmp(ltext, "END IF", 6) == 0) || (runtime_strncasecmp(ltext, "ENDIF", 5) == 0)) {
            if_depth--;
            if (if_depth == 0) {
                found_endif = true;
                break;
            }
        } else if (if_depth == 1 && !in_else && runtime_strncasecmp(ltext, "ELSE", 4) == 0 &&
                   (ltext[4] == '\0' || ltext[4] == ' ' || ltext[4] == '\t' || ltext[4] == ':')) {
            in_else = true;
            continue;
        } else if (runtime_strncasecmp(ltext, "IF", 2) == 0 && (runtime_isspace((unsigned char)ltext[2]) || runtime_isalpha((unsigned char)ltext[2]))) {
            size_t nested_skipped = 0;
            EvalAstNode *nested_if = eval_ast_try_compile_multiline_if(vm, lines, end_idx, total_count, &nested_skipped);
            if (nested_if) {
                if (!in_else) {
                    if (!then_head) then_head = nested_if;
                    else then_tail->next = nested_if;
                    while (nested_if->next) nested_if = nested_if->next;
                    then_tail = nested_if;
                } else {
                    if (!else_head) else_head = nested_if;
                    else else_tail->next = nested_if;
                    while (nested_if->next) nested_if = nested_if->next;
                    else_tail = nested_if;
                }
                end_idx += nested_skipped;
                continue;
            }
            if_depth++;
        } else if ((runtime_strncasecmp(ltext, "FOR", 3) == 0 && (ltext[3] == ' ' || ltext[3] == '\t' || runtime_isalpha((unsigned char)ltext[3]))) ||
                   (runtime_strncasecmp(ltext, "WHILE", 5) == 0 && (ltext[5] == ' ' || ltext[5] == '\t' || runtime_isalpha((unsigned char)ltext[5])))) {
            size_t inner_skipped = 0;
            EvalAstNode *inner_node = eval_ast_try_compile_multiline_block(vm, lines, end_idx, total_count, &inner_skipped);

            if (inner_node) {
                if (!in_else) {
                    if (!then_head) then_head = inner_node;
                    else then_tail->next = inner_node;
                    while (inner_node->next) inner_node = inner_node->next;
                    then_tail = inner_node;
                } else {
                    if (!else_head) else_head = inner_node;
                    else else_tail->next = inner_node;
                    while (inner_node->next) inner_node = inner_node->next;
                    else_tail = inner_node;
                }
                end_idx += inner_skipped;
                continue;
            }
        }

        EvalAstNode *line_ast = eval_ast_try_parse_line(vm, lines[end_idx].text);
        if (!line_ast) {
            eval_ast_free_tree(cond);
            if (then_head) eval_ast_free_tree(then_head);
            if (else_head) eval_ast_free_tree(else_head);
            return NULL;
        }

        EvalAstNode *chk = line_ast;
        bool has_jump = false;
        while (chk) {
            if (chk->type == AST_NODE_GOTO || chk->type == AST_NODE_GOSUB || chk->type == AST_NODE_RETURN) {
                has_jump = true;
                break;
            }
            chk = chk->next;
        }
        if (has_jump) {
            eval_ast_free_tree(line_ast);
            eval_ast_free_tree(cond);
            if (then_head) eval_ast_free_tree(then_head);
            if (else_head) eval_ast_free_tree(else_head);
            return NULL;
        }

        if (!in_else) {
            if (!then_head) then_head = line_ast;
            else then_tail->next = line_ast;
            while (line_ast->next) line_ast = line_ast->next;
            then_tail = line_ast;
        } else {
            if (!else_head) else_head = line_ast;
            else else_tail->next = line_ast;
            while (line_ast->next) line_ast = line_ast->next;
            else_tail = line_ast;
        }
    }

    if (!found_endif) {
        eval_ast_free_tree(cond);
        if (then_head) eval_ast_free_tree(then_head);
        if (else_head) eval_ast_free_tree(else_head);
        return NULL;
    }

    *out_lines_skipped = end_idx - start_idx;
    return eval_ast_create_if(NULL, cond, then_head, else_head);
}

// compiles multi-line loop constructs (FOR/NEXT, WHILE/WEND) into an AST node
EvalAstNode *eval_ast_try_compile_multiline_block(VMContext *vm, const BppProgramLine *lines, size_t start_idx, size_t total_count, size_t *out_lines_skipped) {
    if (!vm || !lines || start_idx >= total_count || !out_lines_skipped) return NULL;
    *out_lines_skipped = 0;

    const char *header_source = skip_line_number_and_spaces(lines[start_idx].text);
    if (!header_source || !*header_source) return NULL;

    // 1. FOR ... NEXT Loop
    if (runtime_strncasecmp(header_source, "FOR", 3) == 0 && (header_source[3] == ' ' || header_source[3] == '\t' || runtime_isalpha((unsigned char)header_source[3]))) {
        LexerContext *lex = lex_init(vm_get_mem(vm), header_source);
        if (!lex) return NULL;
        if (lex_peek(lex).type == TOK_NUMBER) {
            lex_next(lex);
        }

        BppToken tok = lex_next(lex);
        if (!is_tok_kw(tok, KW_FOR, "FOR", 3)) {
            lex_shutdown(lex);
            return NULL;
        }

        BppToken vtok = lex_peek(lex);
        if (vtok.type != TOK_IDENT) {
            lex_shutdown(lex);
            return NULL;
        }
        char var_name[64];
        size_t nlen = (vtok.length < sizeof(var_name) - 1) ? vtok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, vtok.start, nlen);
        var_name[nlen] = '\0';
        lex_next(lex);

        if (lex_peek(lex).type != TOK_EQ) {
            lex_shutdown(lex);
            return NULL;
        }
        lex_next(lex);

        EvalAstNode *start_expr = eval_ast_parse_expression(lex);
        if (!start_expr) {
            lex_shutdown(lex);
            return NULL;
        }

        BppToken to_tok = lex_peek(lex);
        if (!is_tok_kw(to_tok, KW_TO, "TO", 2)) {
            eval_ast_free_tree(start_expr);
            lex_shutdown(lex);
            return NULL;
        }
        lex_next(lex);

        EvalAstNode *end_expr = eval_ast_parse_expression(lex);
        if (!end_expr) {
            eval_ast_free_tree(start_expr);
            lex_shutdown(lex);
            return NULL;
        }

        EvalAstNode *step_expr = NULL;
        if (is_tok_kw(lex_peek(lex), KW_STEP, "STEP", 4)) {
            lex_next(lex);
            step_expr = eval_ast_parse_expression(lex);
            if (!step_expr) {
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                lex_shutdown(lex);
                return NULL;
            }
        }

        EvalAstNode *same_line_body = NULL;
        EvalAstNode *same_line_tail = NULL;
        bool closed_same_line = false;

        while (lex_peek(lex).type != TOK_EOF) {
            if (lex_peek(lex).type == TOK_EOL) {
                if (*lex_peek(lex).start == ':') {
                    lex_next(lex);
                } else {
                    break;
                }
            }
            if (is_tok_kw(lex_peek(lex), KW_NEXT, "NEXT", 4)) {
                lex_next(lex);
                if (lex_peek(lex).type == TOK_IDENT) lex_next(lex);
                closed_same_line = true;
                break;
            }
            EvalAstNode *b_stmt = parse_single_statement(lex);
            if (!b_stmt) {
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                if (step_expr) eval_ast_free_tree(step_expr);
                if (same_line_body) eval_ast_free_tree(same_line_body);
                lex_shutdown(lex);
                return NULL;
            }
            if (!same_line_body) {
                same_line_body = b_stmt;
                same_line_tail = b_stmt;
            } else {
                same_line_tail->next = b_stmt;
                same_line_tail = b_stmt;
            }
        }
        lex_shutdown(lex);

        if (closed_same_line) {
            *out_lines_skipped = 0;
            return eval_ast_create_for_loop(NULL, var_name, start_expr, end_expr, step_expr, same_line_body);
        }

        EvalAstNode *body_head = same_line_body;
        EvalAstNode *body_tail = same_line_tail;
        size_t end_idx = start_idx + 1;
        int depth = 1;
        bool found_close = false;

        for (; end_idx < total_count; end_idx++) {
            const char *ltext = skip_line_number_and_spaces(lines[end_idx].text);
            if (!ltext || *ltext == '\0' || *ltext == '\'' || runtime_strncasecmp(ltext, "REM", 3) == 0) {
                continue;
            }

            if (runtime_strncasecmp(ltext, "NEXT", 4) == 0 && (ltext[4] == '\0' || ltext[4] == ' ' || ltext[4] == '\t' || ltext[4] == ':' || runtime_isalpha((unsigned char)ltext[4]))) {
                depth--;
                if (depth == 0) {
                    found_close = true;
                    break;
                }
            } else if ((runtime_strncasecmp(ltext, "FOR", 3) == 0 && (ltext[3] == ' ' || ltext[3] == '\t' || runtime_isalpha((unsigned char)ltext[3]))) ||
                       (runtime_strncasecmp(ltext, "WHILE", 5) == 0 && (ltext[5] == ' ' || ltext[5] == '\t' || ltext[5] == '(' || runtime_isalpha((unsigned char)ltext[5])))) {
                size_t inner_skipped = 0;
                EvalAstNode *inner_node = eval_ast_try_compile_multiline_block(vm, lines, end_idx, total_count, &inner_skipped);
                if (inner_node) {
                    if (!body_head) {
                        body_head = inner_node;
                    } else {
                        body_tail->next = inner_node;
                    }
                    while (inner_node->next) inner_node = inner_node->next;
                    body_tail = inner_node;
                    end_idx += inner_skipped;
                    continue;
                }
                depth++;
            } else if (runtime_strncasecmp(ltext, "IF", 2) == 0 && (runtime_isspace((unsigned char)ltext[2]) || runtime_isalpha((unsigned char)ltext[2]))) {
                size_t if_skipped = 0;
                EvalAstNode *if_node = eval_ast_try_compile_multiline_if(vm, lines, end_idx, total_count, &if_skipped);

                if (if_node) {
                    if (!body_head) {
                        body_head = if_node;
                    } else {
                        body_tail->next = if_node;
                    }
                    while (if_node->next) if_node = if_node->next;
                    body_tail = if_node;
                    end_idx += if_skipped;
                    continue;
                }
            }

            EvalAstNode *line_ast = eval_ast_try_parse_line(vm, lines[end_idx].text);
            if (!line_ast) {
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                if (step_expr) eval_ast_free_tree(step_expr);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }

            EvalAstNode *chk = line_ast;
            bool has_jump = false;
            while (chk) {
                if (chk->type == AST_NODE_GOTO) {
                    has_jump = true;
                    break;
                }
                if (chk->type == AST_NODE_GOSUB) {
                    if (!chk->target_ast) {
                        chk->target_ast = eval_ast_compile_subroutine(vm, lines, total_count, chk->target_line);
                    }
                    if (!chk->target_ast) {
                        has_jump = true;
                        break;
                    }
                }
                chk = chk->next;
            }
            if (has_jump) {
                eval_ast_free_tree(line_ast);
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                if (step_expr) eval_ast_free_tree(step_expr);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }

            if (!body_head) {
                body_head = line_ast;
            } else {
                body_tail->next = line_ast;
            }
            while (line_ast->next) line_ast = line_ast->next;
            body_tail = line_ast;
        }

        if (!found_close) {
            eval_ast_free_tree(start_expr);
            eval_ast_free_tree(end_expr);
            if (step_expr) eval_ast_free_tree(step_expr);
            if (body_head) eval_ast_free_tree(body_head);
            return NULL;
        }

        *out_lines_skipped = end_idx - start_idx;
        return eval_ast_create_for_loop(NULL, var_name, start_expr, end_expr, step_expr, body_head);
    }

    // 2. WHILE ... WEND Loop
    if (runtime_strncasecmp(header_source, "WHILE", 5) == 0 && (header_source[5] == ' ' || header_source[5] == '\t' || header_source[5] == '(' || runtime_isalpha((unsigned char)header_source[5]))) {
        LexerContext *lex = lex_init(vm_get_mem(vm), header_source);
        if (!lex) return NULL;
        if (lex_peek(lex).type == TOK_NUMBER) {
            lex_next(lex);
        }

        BppToken tok = lex_next(lex);
        if (!is_tok_kw(tok, KW_WHILE, "WHILE", 5)) {
            lex_shutdown(lex);
            return NULL;
        }

        EvalAstNode *cond_expr = eval_ast_parse_expression(lex);
        if (!cond_expr) {
            lex_shutdown(lex);
            return NULL;
        }

        EvalAstNode *same_line_body = NULL;
        EvalAstNode *same_line_tail = NULL;
        bool closed_same_line = false;

        while (lex_peek(lex).type != TOK_EOF) {
            if (lex_peek(lex).type == TOK_EOL) {
                if (*lex_peek(lex).start == ':') {
                    lex_next(lex);
                } else {
                    break;
                }
            }
            if (is_tok_kw(lex_peek(lex), KW_WEND, "WEND", 4)) {
                lex_next(lex);
                closed_same_line = true;
                break;
            }
            EvalAstNode *b_stmt = parse_single_statement(lex);
            if (!b_stmt) {
                eval_ast_free_tree(cond_expr);
                if (same_line_body) eval_ast_free_tree(same_line_body);
                lex_shutdown(lex);
                return NULL;
            }
            if (!same_line_body) {
                same_line_body = b_stmt;
                same_line_tail = b_stmt;
            } else {
                same_line_tail->next = b_stmt;
                same_line_tail = b_stmt;
            }
        }
        lex_shutdown(lex);

        if (closed_same_line) {
            *out_lines_skipped = 0;
            return eval_ast_create_while_loop(NULL, cond_expr, same_line_body);
        }

        EvalAstNode *body_head = same_line_body;
        EvalAstNode *body_tail = same_line_tail;
        size_t end_idx = start_idx + 1;
        int depth = 1;
        bool found_close = false;

        for (; end_idx < total_count; end_idx++) {
            const char *ltext = skip_line_number_and_spaces(lines[end_idx].text);
            if (!ltext || *ltext == '\0' || *ltext == '\'' || runtime_strncasecmp(ltext, "REM", 3) == 0) {
                continue;
            }

            if (runtime_strncasecmp(ltext, "WEND", 4) == 0 && (ltext[4] == '\0' || ltext[4] == ' ' || ltext[4] == '\t' || ltext[4] == ':')) {
                depth--;
                if (depth == 0) {
                    found_close = true;
                    break;
                }
            } else if ((runtime_strncasecmp(ltext, "FOR", 3) == 0 && (ltext[3] == ' ' || ltext[3] == '\t' || runtime_isalpha((unsigned char)ltext[3]))) ||
                       (runtime_strncasecmp(ltext, "WHILE", 5) == 0 && (ltext[5] == ' ' || ltext[5] == '\t' || ltext[5] == '(' || runtime_isalpha((unsigned char)ltext[5])))) {
                size_t inner_skipped = 0;
                EvalAstNode *inner_node = eval_ast_try_compile_multiline_block(vm, lines, end_idx, total_count, &inner_skipped);
                if (inner_node) {
                    if (!body_head) {
                        body_head = inner_node;
                    } else {
                        body_tail->next = inner_node;
                    }
                    while (inner_node->next) inner_node = inner_node->next;
                    body_tail = inner_node;
                    end_idx += inner_skipped;
                    continue;
                }
                depth++;
            } else if (runtime_strncasecmp(ltext, "IF", 2) == 0 && (runtime_isspace((unsigned char)ltext[2]) || runtime_isalpha((unsigned char)ltext[2]))) {
                size_t if_skipped = 0;
                EvalAstNode *if_node = eval_ast_try_compile_multiline_if(vm, lines, end_idx, total_count, &if_skipped);

                if (if_node) {
                    if (!body_head) {
                        body_head = if_node;
                    } else {
                        body_tail->next = if_node;
                    }
                    while (if_node->next) if_node = if_node->next;
                    body_tail = if_node;
                    end_idx += if_skipped;
                    continue;
                }
            }

            EvalAstNode *line_ast = eval_ast_try_parse_line(vm, lines[end_idx].text);
            if (!line_ast) {
                eval_ast_free_tree(cond_expr);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }

            EvalAstNode *chk = line_ast;
            bool has_jump = false;
            while (chk) {
                if (chk->type == AST_NODE_GOTO) {
                    has_jump = true;
                    break;
                }
                chk = chk->next;
            }
            if (has_jump) {
                eval_ast_free_tree(line_ast);
                eval_ast_free_tree(cond_expr);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }

            if (!body_head) {
                body_head = line_ast;
            } else {
                body_tail->next = line_ast;
            }
            while (line_ast->next) line_ast = line_ast->next;
            body_tail = line_ast;
        }

        if (!found_close) {
            eval_ast_free_tree(cond_expr);
            if (body_head) eval_ast_free_tree(body_head);
            return NULL;
        }

        *out_lines_skipped = end_idx - start_idx;
        return eval_ast_create_while_loop(NULL, cond_expr, body_head);
    }

    return NULL;
}

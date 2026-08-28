// FILENAME: ast_parse_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ast_internal.h, eval_internal.h, lexer.h, lexer.c, vm.h)
// NEEDS: libplatform (platform.h)
// Implements AST parsing and evaluation structures for ast_parse_stmt.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "eval/eval_internal.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "vm/vm.h"

//
// ---- Single Statement Parser ----

// parses a single statement from a lexer token stream into an AST node
EvalAstNode *parse_single_statement(LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    if (is_tok_kw(tok, KW_LET, "LET", 3)) {
        lex_next(lex);
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_IDENT) {
        char var_name[64];
        size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, tok.start, nlen);
        var_name[nlen] = '\0';
        lex_next(lex);

        if (lex_peek(lex).type == TOK_PERIOD) {
            lex_next(lex);
            BppToken ftok = lex_peek(lex);
            if (ftok.type == TOK_IDENT) {
                lex_next(lex);
                size_t flen = (ftok.length < sizeof(var_name) - runtime_strlen(var_name) - 2) ? ftok.length : (sizeof(var_name) - runtime_strlen(var_name) - 2);
                runtime_strcat(var_name, ".");
                runtime_strncat(var_name, ftok.start, flen);
            }
        }

        if (runtime_strncasecmp(var_name, "App.", 4) == 0 || runtime_strncasecmp(var_name, "Screen.", 7) == 0 ||
            runtime_strncasecmp(var_name, "Printer.", 8) == 0 || runtime_strncasecmp(var_name, "Err.", 4) == 0 ||
            runtime_strncasecmp(var_name, "Clipboard.", 10) == 0) {
            return NULL;
        }


        if (lex_peek(lex).type == TOK_EQ) {
            lex_next(lex);
            EvalAstNode *expr = eval_ast_parse_expression(lex);
            if (!expr) return NULL;
            return eval_ast_create_assignment(NULL, var_name, expr);
        }
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            EvalAstNode *idx_expr = eval_ast_parse_expression(lex);
            if (!idx_expr) return NULL;
            EvalAstNode *idx2_expr = NULL;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
                idx2_expr = eval_ast_parse_expression(lex);
                if (!idx2_expr) {
                    eval_ast_free_tree(idx_expr);
                    return NULL;
                }
            }
            if (lex_peek(lex).type != TOK_RPAREN) {
                eval_ast_free_tree(idx_expr);
                if (idx2_expr) eval_ast_free_tree(idx2_expr);
                return NULL;
            }
            lex_next(lex);
            if (lex_peek(lex).type == TOK_EQ) {
                lex_next(lex);
                EvalAstNode *val_expr = eval_ast_parse_expression(lex);
                if (!val_expr) {
                    eval_ast_free_tree(idx_expr);
                    if (idx2_expr) eval_ast_free_tree(idx2_expr);
                    return NULL;
                }
                if (idx2_expr) {
                    return eval_ast_create_array2d_assign(NULL, var_name, idx_expr, idx2_expr, val_expr);
                } else {
                    return eval_ast_create_array_assign(NULL, var_name, idx_expr, val_expr);
                }
            }
            eval_ast_free_tree(idx_expr);
            if (idx2_expr) eval_ast_free_tree(idx2_expr);
            return NULL;
        }
        return NULL;
    }
    if (is_tok_kw(tok, KW_IF, "IF", 2)) {
        lex_next(lex);
        EvalAstNode *cond = eval_ast_parse_expression(lex);
        if (!cond) return NULL;
        BppToken then_tok = lex_peek(lex);
        if (is_tok_kw(then_tok, KW_THEN, "THEN", 4)) {
            lex_next(lex);
        } else if (is_tok_kw(then_tok, KW_GOTO, "GOTO", 4) || then_tok.type == TOK_NUMBER) {
        } else {
            eval_ast_free_tree(cond);
            return NULL;
        }

        EvalAstNode *then_head = NULL;
        EvalAstNode *then_tail = NULL;

        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
            if (is_tok_kw(lex_peek(lex), KW_ELSE, "ELSE", 4)) {
                break;
            }
            EvalAstNode *stmt = parse_single_statement(lex);
            if (!stmt) {
                eval_ast_free_tree(cond);
                if (then_head) eval_ast_free_tree(then_head);
                return NULL;
            }
            if (!then_head) {
                then_head = stmt;
                then_tail = stmt;
            } else {
                then_tail->next = stmt;
                then_tail = stmt;
            }
            while (then_tail->next) then_tail = then_tail->next;

            if (lex_peek(lex).type == TOK_EOL && *lex_peek(lex).start == ':') {
                lex_next(lex);
            } else if (lex_peek(lex).type == TOK_BACKSLASH) {
                lex_next(lex);
            } else {
                break;
            }
        }

        EvalAstNode *else_head = NULL;
        EvalAstNode *else_tail = NULL;
        if (is_tok_kw(lex_peek(lex), KW_ELSE, "ELSE", 4)) {
            lex_next(lex);
            while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
                EvalAstNode *stmt = parse_single_statement(lex);
                if (!stmt) {
                    eval_ast_free_tree(cond);
                    if (then_head) eval_ast_free_tree(then_head);
                    if (else_head) eval_ast_free_tree(else_head);
                    return NULL;
                }
                if (!else_head) {
                    else_head = stmt;
                    else_tail = stmt;
                } else {
                    else_tail->next = stmt;
                    else_tail = stmt;
                }
                while (else_tail->next) else_tail = else_tail->next;

                if (lex_peek(lex).type == TOK_EOL && *lex_peek(lex).start == ':') {
                    lex_next(lex);
                } else if (lex_peek(lex).type == TOK_BACKSLASH) {
                    lex_next(lex);
                } else {
                    break;
                }
            }
        }
        return eval_ast_create_if(NULL, cond, then_head, else_head);
    }
    if (is_tok_kw(tok, KW_GOTO, "GOTO", 4)) {
        lex_next(lex);
        BppToken target = lex_next(lex);
        if (target.type == TOK_NUMBER) {
            return eval_ast_create_goto(NULL, target.as.number);
        }
        return NULL;
    }
    if (is_tok_kw(tok, KW_GOSUB, "GOSUB", 5)) {
        lex_next(lex);
        BppToken target = lex_next(lex);
        if (target.type == TOK_NUMBER) {
            return eval_ast_create_gosub(NULL, target.as.number);
        }
        return NULL;
    }
    if (is_tok_kw(tok, KW_RETURN, "RETURN", 6)) {
        lex_next(lex);
        return eval_ast_create_return(NULL);
    }
    if (is_tok_kw(tok, KW_FOR, "FOR", 3)) {
        lex_next(lex);
        BppToken vtok = lex_peek(lex);
        if (vtok.type != TOK_IDENT) return NULL;
        char var_name[64];
        size_t nlen = (vtok.length < sizeof(var_name) - 1) ? vtok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, vtok.start, nlen);
        var_name[nlen] = '\0';
        lex_next(lex);


        if (lex_peek(lex).type != TOK_EQ) return NULL;
        lex_next(lex);

        EvalAstNode *start_expr = eval_ast_parse_expression(lex);
        if (!start_expr) return NULL;

        BppToken to_tok = lex_peek(lex);
        if (!is_tok_kw(to_tok, KW_TO, "TO", 2)) {
            eval_ast_free_tree(start_expr);
            return NULL;
        }
        lex_next(lex);

        EvalAstNode *end_expr = eval_ast_parse_expression(lex);
        if (!end_expr) {
            eval_ast_free_tree(start_expr);
            return NULL;
        }

        EvalAstNode *step_expr = NULL;
        if (is_tok_kw(lex_peek(lex), KW_STEP, "STEP", 4)) {
            lex_next(lex);
            step_expr = eval_ast_parse_expression(lex);
            if (!step_expr) {
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                return NULL;
            }
        }

        EvalAstNode *body_head = NULL;
        EvalAstNode *body_tail = NULL;

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
                if (lex_peek(lex).type == TOK_IDENT) {
                    lex_next(lex);
                }
                return eval_ast_create_for_loop(NULL, var_name, start_expr, end_expr, step_expr, body_head);
            }
            EvalAstNode *b_stmt = parse_single_statement(lex);
            if (!b_stmt) {
                eval_ast_free_tree(start_expr);
                eval_ast_free_tree(end_expr);
                if (step_expr) eval_ast_free_tree(step_expr);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }
            if (!body_head) {
                body_head = b_stmt;
                body_tail = b_stmt;
            } else {
                body_tail->next = b_stmt;
                body_tail = b_stmt;
            }
        }
        eval_ast_free_tree(start_expr);
        eval_ast_free_tree(end_expr);
        if (step_expr) eval_ast_free_tree(step_expr);
        if (body_head) eval_ast_free_tree(body_head);
        return NULL;
    }
    if (is_tok_kw(tok, KW_WHILE, "WHILE", 5)) {
        lex_next(lex);
        EvalAstNode *cond = eval_ast_parse_expression(lex);
        if (!cond) return NULL;

        EvalAstNode *body_head = NULL;
        EvalAstNode *body_tail = NULL;

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
                return eval_ast_create_while_loop(NULL, cond, body_head);
            }
            EvalAstNode *b_stmt = parse_single_statement(lex);
            if (!b_stmt) {
                eval_ast_free_tree(cond);
                if (body_head) eval_ast_free_tree(body_head);
                return NULL;
            }
            if (!body_head) {
                body_head = b_stmt;
                body_tail = b_stmt;
            } else {
                body_tail->next = b_stmt;
                body_tail = b_stmt;
            }
        }
        eval_ast_free_tree(cond);
        if (body_head) eval_ast_free_tree(body_head);
        return NULL;
    }
    if (is_tok_kw(tok, KW_POKE, "POKE", 4)) {
        lex_next(lex);
        EvalAstNode *addr = eval_ast_parse_expression(lex);
        if (!addr) return NULL;
        if (lex_peek(lex).type != TOK_COMMA) {
            eval_ast_free_tree(addr);
            return NULL;
        }
        lex_next(lex);
        EvalAstNode *val = eval_ast_parse_expression(lex);
        if (!val) {
            eval_ast_free_tree(addr);
            return NULL;
        }
        return eval_ast_create_poke(NULL, addr, val);
    }
    if (is_tok_kw(tok, KW_LINE, "LINE", 4)) {
        lex_next(lex);
        if (is_tok_kw(lex_peek(lex), KW_INPUT, "INPUT", 5)) {
            lex_next(lex);
            int ch = 0;
            if (lex_peek(lex).type == TOK_HASH) {
                lex_next(lex);
                BppToken ntok = lex_peek(lex);
                if (ntok.type == TOK_NUMBER) {
                    ch = (int)ntok.as.number;
                    lex_next(lex);
                    if (lex_peek(lex).type == TOK_COMMA) {
                        lex_next(lex);
                    }
                } else {
                    return NULL;
                }
            }
            BppToken vtok = lex_peek(lex);
            if (vtok.type == TOK_IDENT) {
                char vname[64];
                size_t nlen = (vtok.length < sizeof(vname) - 1) ? vtok.length : sizeof(vname) - 1;
                runtime_memcpy(vname, vtok.start, nlen);
                vname[nlen] = '\0';
                lex_next(lex);
                return eval_ast_create_line_input(NULL, ch, vname);
            }
        }
        return NULL;
    }
    if (is_tok_kw(tok, KW_PRINT, "PRINT", 5)) {
        lex_next(lex);
        int ch = 0;
        if (lex_peek(lex).type == TOK_HASH) {
            lex_next(lex);
            BppToken ntok = lex_peek(lex);
            if (ntok.type == TOK_NUMBER) {
                ch = (int)ntok.as.number;
                lex_next(lex);
                if (lex_peek(lex).type == TOK_COMMA) {
                    lex_next(lex);
                }
            } else {
                return NULL;
            }
        }
        if (lex_peek(lex).type == TOK_EOF || lex_peek(lex).type == TOK_EOL || (lex_peek(lex).type == TOK_EOL && *lex_peek(lex).start == ':')) {
            if (ch > 0) return eval_ast_create_file_print(NULL, ch, NULL);
            return eval_ast_create_print(NULL, NULL);
        }

        EvalAstNode *head_p = NULL;
        EvalAstNode *tail_p = NULL;

        while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL && !(lex_peek(lex).type == TOK_EOL && *lex_peek(lex).start == ':')) {
            EvalAstNode *expr = eval_ast_parse_expression(lex);
            if (!expr) {
                if (head_p) eval_ast_free_tree(head_p);
                return NULL;
            }
            BppTokenType sep = TOK_EOL;
            if (lex_peek(lex).type == TOK_SEMICOLON || lex_peek(lex).type == TOK_COMMA) {
                sep = lex_peek(lex).type;
                lex_next(lex);
            }
            EvalAstNode *pnode = (ch > 0) ? eval_ast_create_file_print(NULL, ch, expr) : eval_ast_create_print(NULL, expr);
            if (!pnode) {
                eval_ast_free_tree(expr);
                if (head_p) eval_ast_free_tree(head_p);
                return NULL;
            }
            pnode->op = sep;
            if (!head_p) {
                head_p = pnode;
                tail_p = pnode;
            } else {
                tail_p->next = pnode;
                tail_p = pnode;
            }
            if (sep == TOK_EOL) {
                break;
            }
        }
        return head_p;
    }
    return NULL;
}

// parses a full line into a chain of AST statement nodes
EvalAstNode *eval_ast_try_parse_line(VMContext *vm, const char *source) {
    if (!vm || !source) return NULL;
    source = skip_line_number_and_spaces(source);
    if (!*source) return NULL;
    if (runtime_strchr(source, '[') != NULL || runtime_strchr(source, ']') != NULL) {
        return NULL;
    }


    LexerContext *lex = lex_init(vm_get_mem(vm), source);
    if (!lex) return NULL;
    if (lex_peek(lex).type == TOK_NUMBER) {
        lex_next(lex);
    }

    EvalAstNode *head = NULL;
    EvalAstNode *tail = NULL;

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok_start = lex_peek(lex);
        EvalAstNode *stmt = parse_single_statement(lex);
        if (!stmt) {
            eval_ast_free_tree(head);
            lex_shutdown(lex);
            return NULL;
        }
        stmt->source_pos = tok_start.start;
        if (!head) {
            head = stmt;
        } else {
            tail->next = stmt;
        }
        while (stmt->next) stmt = stmt->next;
        tail = stmt;

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_EOL && *sep.start == ':') {
            lex_next(lex);
        } else if (sep.type == TOK_BACKSLASH) {
            lex_next(lex);
        } else if (sep.type == TOK_EOF || sep.type == TOK_EOL) {
            break;
        } else {
            eval_ast_free_tree(head);
            lex_shutdown(lex);
            return NULL;
        }
    }
    lex_shutdown(lex);
    return head;
}

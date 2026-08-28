// FILENAME: eval_op.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval_op within BASIC++.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"

//
// ---- Clause Delimiters and Operator Processing ----

bool eval_is_clause_delimiter(BppToken tok) {
    if (tok.type == TOK_KEYWORD) {
        BppKeywordId kw = tok.as.keyword;
        return (kw == KW_THEN || kw == KW_ELSE || kw == KW_TO || kw == KW_STEP ||
                kw == KW_AS || kw == KW_GOTO || kw == KW_GOSUB || kw == KW_IF ||
                kw == KW_UNLESS || kw == KW_UNTIL || kw == KW_BY || kw == KW_FOR ||
                kw == KW_WHILE);
    }
    if (tok.type == TOK_IDENT && tok.start && tok.length > 0) {
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "TO", 2) == 0) return true;
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "AS", 2) == 0) return true;
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "IF", 2) == 0) return true;
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "BY", 2) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "FOR", 3) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "THEN", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "ELSE", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "STEP", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "GOTO", 4) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "GOSUB", 5) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "UNTIL", 5) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "WHILE", 5) == 0) return true;
        if (tok.length == 6 && runtime_strncasecmp(tok.start, "UNLESS", 6) == 0) return true;
    }
    return false;
}


bool eval_process_operator(VMContext *vm, LexerContext *lex, BppToken tok, bool expect_operand, int open_parens,
                           BValue *val_stack, size_t *val_ptr, BppTokenType *op_stack, size_t *op_ptr,
                           bool *out_expect_operand, bool *out_break, BppError *out_err) {
    if (!vm || !lex || !val_stack || !val_ptr || !op_stack || !op_ptr || !out_expect_operand || !out_break || !out_err) return false;

    *out_break = false;
    BppTokenType op = tok.type;

    if (op == TOK_BACKSLASH && !expect_operand && open_parens == 0) {
        // Check if backslash is DEC PDP-11 statement separator
        bool is_stmt_sep = false;
        LexerContext *peek_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (peek_lex) {
            BppToken after_bs = lex_peek(peek_lex);
            if (after_bs.type == TOK_EOF || after_bs.type == TOK_EOL || after_bs.type == TOK_LBRACKET) {
                is_stmt_sep = true;
            } else if (after_bs.type == TOK_KEYWORD) {
                BppKeywordId akw = after_bs.as.keyword;
                if (akw != KW_NONE && akw != KW_AND && akw != KW_OR && akw != KW_NOT &&
                    akw != KW_XOR && akw != KW_IMP && akw != KW_EQV && akw != KW_MOD &&
                    akw != KW_TO && akw != KW_STEP && akw != KW_BY) {
                    is_stmt_sep = true;
                }
            } else if (after_bs.type == TOK_IDENT) {
                lex_next(peek_lex);
                BppToken after_id = lex_peek(peek_lex);
                if (after_id.type == TOK_EQ) {
                    is_stmt_sep = true;
                }
            }
            lex_shutdown(peek_lex);
        }
        if (is_stmt_sep) {
            *out_break = true;
            return true;
        }
    }

    if (expect_operand) {
        // Dual-mode operator used as function call
        if (lex_peek(lex).type == TOK_LPAREN &&
            (op == TOK_SHL || op == TOK_SHR || op == TOK_READBIT || op == TOK_SETBIT ||
             op == TOK_RESETBIT || op == TOK_TOGGLEBIT || op == TOK_MIN || op == TOK_MAX ||
             op == TOK_HYPOT || op == TOK_REMAINDER || op == TOK_ATAN2 || op == TOK_MOD ||
             op == TOK_AND || op == TOK_OR || op == TOK_XOR || op == TOK_IMP || op == TOK_EQV || op == TOK_NOT)) {
            lex_next(lex);
            const char *fn_name = "SHL";
            if (op == TOK_SHL) fn_name = "SHL";
            else if (op == TOK_SHR) fn_name = "SHR";
            else if (op == TOK_READBIT) fn_name = "READBIT";
            else if (op == TOK_SETBIT) fn_name = "SETBIT";
            else if (op == TOK_RESETBIT) fn_name = "RESETBIT";
            else if (op == TOK_TOGGLEBIT) fn_name = "TOGGLEBIT";
            else if (op == TOK_MIN) fn_name = "MIN";
            else if (op == TOK_MAX) fn_name = "MAX";
            else if (op == TOK_HYPOT) fn_name = "HYPOT";
            else if (op == TOK_REMAINDER) fn_name = "REMAINDER";
            else if (op == TOK_ATAN2) fn_name = "ATAN2";
            else if (op == TOK_MOD) fn_name = "MOD";
            else if (op == TOK_AND) fn_name = "AND";
            else if (op == TOK_OR) fn_name = "OR";
            else if (op == TOK_XOR) fn_name = "XOR";
            else if (op == TOK_IMP) fn_name = "IMP";
            else if (op == TOK_EQV) fn_name = "EQV";
            else if (op == TOK_NOT) fn_name = "NOT";

            BValue val = eval_builtin_function(vm, fn_name, lex, true, out_err);
            if (out_err->code != 0) return false;

            if (*val_ptr >= MAX_EVAL_DEPTH) {
                out_err->code = 14;
                out_err->message = "Evaluation stack overflow";
                return false;
            }
            val_stack[(*val_ptr)++] = val;
            *out_expect_operand = false;
            return true;
        }

        // Translate unary operators
        if (op == TOK_MINUS) op = TOK_UNARY_MINUS;
        else if (op == TOK_PLUS) op = TOK_UNARY_PLUS;
        else if (op == TOK_NOT) {
            // TOK_NOT is unary
        } else {
            out_err->code = 2;
            out_err->message = "Expected operand, got operator";
            return false;
        }
    } else {
        if (op == TOK_NOT) {
            out_err->code = 2;
            out_err->message = "Unexpected NOT operator";
            return false;
        }
    }

    while (*op_ptr > 0) {
        BppTokenType top = op_stack[*op_ptr - 1];
        if (top == TOK_LPAREN) break;
        if (eval_has_precedence(vm, top, op)) {
            (*op_ptr)--;
            if (!eval_execute_op(vm, top, val_stack, val_ptr, out_err)) {
                return false;
            }
        } else {
            break;
        }
    }

    if (*op_ptr >= MAX_EVAL_DEPTH) {
        out_err->code = 14;
        out_err->message = "Operator stack overflow";
        return false;
    }
    op_stack[(*op_ptr)++] = op;
    *out_expect_operand = true;
    return true;
}

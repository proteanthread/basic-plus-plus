// FILENAME: ast_parse_expr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ast_internal.h, eval_internal.h, lexer.h, lexer.c, vm.h)
// NEEDS: libplatform (platform.h)
// Implements AST parsing and evaluation structures for ast_parse_expr.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "eval/eval_internal.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "vm/vm.h"

//
// ---- Recursive Descent Expression Parser ----

static EvalAstNode *parse_expr_internal(LexerContext *lex);

// parses atomic primary expressions, literals, variable identifiers, and built-in functions
static EvalAstNode *parse_primary(LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        lex_next(lex);
        BValue val = { .type = VAL_NUMBER, .as.number = tok.as.number };
        return eval_ast_create_literal(NULL, val);
    }
    if (tok.type == TOK_STRING) {
        lex_next(lex);
        const char *s_start = tok.start;
        size_t s_len = tok.length;
        if (s_len >= 2 && s_start[0] == '"') {
            s_start++;
            s_len -= 2;
        }
        BppString *bs = str_create_static(s_start, s_len);
        if (bs) {
            BValue val = { .type = VAL_STRING, .as.string = bs };
            return eval_ast_create_literal(NULL, val);
        }
        return NULL;
    }
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        lex_next(lex);
        char name[64];
        size_t nlen = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        runtime_memcpy(name, tok.start, nlen);
        name[nlen] = '\0';

        if (lex_peek(lex).type == TOK_PERIOD) {
            lex_next(lex);
            BppToken ftok = lex_peek(lex);
            if (ftok.type == TOK_IDENT) {
                lex_next(lex);
                size_t flen = (ftok.length < sizeof(name) - runtime_strlen(name) - 2) ? ftok.length : (sizeof(name) - runtime_strlen(name) - 2);
                runtime_strcat(name, ".");
                runtime_strncat(name, ftok.start, flen);
            }
        }

        if (runtime_strncasecmp(name, "App.", 4) == 0 || runtime_strncasecmp(name, "Screen.", 7) == 0 ||
            runtime_strncasecmp(name, "Printer.", 8) == 0 || runtime_strncasecmp(name, "Err.", 4) == 0 ||
            runtime_strncasecmp(name, "Clipboard.", 10) == 0) {
            return NULL;
        }

        AstMathFunc mf = 0;
        if (runtime_strcasecmp(name, "SIN") == 0) mf = AST_MATH_SIN;
        else if (runtime_strcasecmp(name, "COS") == 0) mf = AST_MATH_COS;
        else if (runtime_strcasecmp(name, "TAN") == 0) mf = AST_MATH_TAN;
        else if (runtime_strcasecmp(name, "SQR") == 0) mf = AST_MATH_SQR;
        else if (runtime_strcasecmp(name, "ABS") == 0) mf = AST_MATH_ABS;
        else if (runtime_strcasecmp(name, "INT") == 0) mf = AST_MATH_INT;
        else if (runtime_strcasecmp(name, "FIX") == 0) mf = AST_MATH_FIX;
        else if (runtime_strcasecmp(name, "EXP") == 0) mf = AST_MATH_EXP;
        else if (runtime_strcasecmp(name, "LOG") == 0) mf = AST_MATH_LOG;
        else if (runtime_strcasecmp(name, "ATN") == 0) mf = AST_MATH_ATN;
        else if (runtime_strcasecmp(name, "SGN") == 0) mf = AST_MATH_SGN;
        else if (runtime_strcasecmp(name, "RND") == 0) mf = AST_MATH_RND;
        else if (runtime_strcasecmp(name, "EOF") == 0) mf = AST_MATH_EOF;

        if (mf != 0) {
            if (lex_peek(lex).type == TOK_LPAREN) {
                lex_next(lex);
                EvalAstNode *arg = parse_expr_internal(lex);
                if (!arg) return NULL;
                if (lex_peek(lex).type != TOK_RPAREN) {
                    eval_ast_free_tree(arg);
                    return NULL;
                }
                lex_next(lex);
                return eval_ast_create_math_func(NULL, mf, arg);
            } else if (mf == AST_MATH_RND) {
                EvalAstNode *zero_lit = eval_ast_create_literal(NULL, (BValue){.type = VAL_NUMBER, .as.number = 0.0});
                return eval_ast_create_math_func(NULL, AST_MATH_RND, zero_lit);
            }
        }

        AstStringFunc sf = 0;
        if (runtime_strcasecmp(name, "STR$") == 0 || runtime_strcasecmp(name, "STR") == 0) sf = AST_STR_STR;
        else if (runtime_strcasecmp(name, "UCASE$") == 0 || runtime_strcasecmp(name, "UCASE") == 0) sf = AST_STR_UCASE;
        else if (runtime_strcasecmp(name, "LCASE$") == 0 || runtime_strcasecmp(name, "LCASE") == 0) sf = AST_STR_LCASE;
        else if (runtime_strcasecmp(name, "MID$") == 0 || runtime_strcasecmp(name, "MID") == 0) sf = AST_STR_MID;
        else if (runtime_strcasecmp(name, "LEFT$") == 0 || runtime_strcasecmp(name, "LEFT") == 0) sf = AST_STR_LEFT;
        else if (runtime_strcasecmp(name, "RIGHT$") == 0 || runtime_strcasecmp(name, "RIGHT") == 0) sf = AST_STR_RIGHT;
        else if (runtime_strcasecmp(name, "LEN") == 0) sf = AST_STR_LEN;
        else if (runtime_strcasecmp(name, "CHR$") == 0 || runtime_strcasecmp(name, "CHR") == 0) sf = AST_STR_CHR;
        else if (runtime_strcasecmp(name, "ASC") == 0) sf = AST_STR_ASC;
        else if (runtime_strcasecmp(name, "SPACE$") == 0 || runtime_strcasecmp(name, "SPACE") == 0) sf = AST_STR_SPACE;
        else if (runtime_strcasecmp(name, "STRING$") == 0 || runtime_strcasecmp(name, "STRING") == 0) sf = AST_STR_STRING;
        else if (runtime_strcasecmp(name, "HEX$") == 0 || runtime_strcasecmp(name, "HEX") == 0) sf = AST_STR_HEX;
        else if (runtime_strcasecmp(name, "OCT$") == 0 || runtime_strcasecmp(name, "OCT") == 0) sf = AST_STR_OCT;
        else if (runtime_strcasecmp(name, "BIN$") == 0 || runtime_strcasecmp(name, "BIN") == 0) sf = AST_STR_BIN;
        else if (runtime_strcasecmp(name, "INSTR") == 0) sf = AST_STR_INSTR;
        else if (runtime_strcasecmp(name, "SHA256$") == 0 || runtime_strcasecmp(name, "SHA256") == 0) sf = AST_STR_SHA256;
        else if (runtime_strcasecmp(name, "MD5$") == 0 || runtime_strcasecmp(name, "MD5") == 0) sf = AST_STR_MD5;
        else if (runtime_strcasecmp(name, "PEEK") == 0) sf = AST_STR_PEEK;


        if (sf != 0) {
            if (lex_peek(lex).type == TOK_LPAREN) {
                lex_next(lex);
                EvalAstNode *arg1 = parse_expr_internal(lex);
                if (!arg1) return NULL;
                EvalAstNode *arg2 = NULL;
                EvalAstNode *arg3 = NULL;
                if (lex_peek(lex).type == TOK_COMMA) {
                    lex_next(lex);
                    arg2 = parse_expr_internal(lex);
                    if (!arg2) {
                        eval_ast_free_tree(arg1);
                        return NULL;
                    }
                    if (lex_peek(lex).type == TOK_COMMA) {
                        lex_next(lex);
                        arg3 = parse_expr_internal(lex);
                        if (!arg3) {
                            eval_ast_free_tree(arg1);
                            eval_ast_free_tree(arg2);
                            return NULL;
                        }
                    }
                }
                if (lex_peek(lex).type != TOK_RPAREN) {
                    eval_ast_free_tree(arg1);
                    if (arg2) eval_ast_free_tree(arg2);
                    if (arg3) eval_ast_free_tree(arg3);
                    return NULL;
                }
                lex_next(lex);
                return eval_ast_create_string_func(NULL, sf, arg1, arg2, arg3);
            }
        }

        if (lex_peek(lex).type == TOK_LPAREN) {
            if (eval_is_builtin_function(name)) {
                return NULL;
            }
            lex_next(lex);
            EvalAstNode *idx_expr = parse_expr_internal(lex);
            if (!idx_expr) return NULL;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
                EvalAstNode *idx2_expr = parse_expr_internal(lex);
                if (!idx2_expr) {
                    eval_ast_free_tree(idx_expr);
                    return NULL;
                }
                if (lex_peek(lex).type != TOK_RPAREN) {
                    eval_ast_free_tree(idx_expr);
                    eval_ast_free_tree(idx2_expr);
                    return NULL;
                }
                lex_next(lex);
                return eval_ast_create_array2d_read(NULL, name, idx_expr, idx2_expr);
            } else if (lex_peek(lex).type == TOK_RPAREN) {
                lex_next(lex);
                return eval_ast_create_array_read(NULL, name, idx_expr);
            } else {
                eval_ast_free_tree(idx_expr);
                return NULL;
            }
        }

        if (eval_is_zero_arg_builtin_function(name)) {
            return NULL;
        }

        if (lex_peek(lex).type == TOK_LBRACKET) {
            return NULL;
        }
        return eval_ast_create_variable(NULL, name);
    }
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        EvalAstNode *sub = parse_expr_internal(lex);
        if (!sub) return NULL;
        if (lex_peek(lex).type != TOK_RPAREN) {
            eval_ast_free_tree(sub);
            return NULL;
        }
        lex_next(lex);
        return sub;
    }
    return NULL;
}

// parses unary operations: unary minus and logical NOT
static EvalAstNode *parse_unary(LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_MINUS) {
        lex_next(lex);
        EvalAstNode *op = parse_unary(lex);
        if (!op) return NULL;
        return eval_ast_create_unary(NULL, TOK_UNARY_MINUS, op);
    }
    if (tok.type == TOK_NOT || is_tok_kw(tok, KW_NOT, "NOT", 3)) {
        lex_next(lex);
        EvalAstNode *op = parse_unary(lex);
        if (!op) return NULL;
        return eval_ast_create_unary(NULL, TOK_NOT, op);
    }
    return parse_primary(lex);
}

// parses exponentiation operation: ^
static EvalAstNode *parse_power(LexerContext *lex) {
    EvalAstNode *left = parse_unary(lex);
    if (!left) return NULL;
    if (lex_peek(lex).type == TOK_POW) {
        lex_next(lex);
        EvalAstNode *right = parse_power(lex);
        if (!right) {
            eval_ast_free_tree(left);
            return NULL;
        }
        return eval_ast_create_binary(NULL, TOK_POW, left, right);
    }
    return left;
}

// parses multiplicative operations: *, /, \, MOD
static EvalAstNode *parse_mul_div(LexerContext *lex) {
    EvalAstNode *left = parse_power(lex);
    if (!left) return NULL;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_MUL || tok.type == TOK_DIV || tok.type == TOK_BACKSLASH || tok.type == TOK_MOD || is_tok_kw(tok, KW_MOD, "MOD", 3)) {
            if (tok.type == TOK_BACKSLASH) {
                bool is_stmt_sep = false;
                LexerContext *peek_lex = lex_init(NULL, lex_get_pos(lex));
                if (peek_lex) {
                    lex_next(peek_lex);
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
                    break;
                }
            }
            BppTokenType op = (tok.type == TOK_MOD || is_tok_kw(tok, KW_MOD, "MOD", 3)) ? TOK_MOD : tok.type;
            lex_next(lex);
            EvalAstNode *right = parse_power(lex);
            if (!right) {
                eval_ast_free_tree(left);
                return NULL;
            }
            left = eval_ast_create_binary(NULL, op, left, right);
        } else {
            break;
        }
    }
    return left;
}

// parses additive operations: +, -
static EvalAstNode *parse_add_sub(LexerContext *lex) {
    EvalAstNode *left = parse_mul_div(lex);
    if (!left) return NULL;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_PLUS || tok.type == TOK_MINUS) {
            BppTokenType op = tok.type;
            lex_next(lex);
            EvalAstNode *right = parse_mul_div(lex);
            if (!right) {
                eval_ast_free_tree(left);
                return NULL;
            }
            left = eval_ast_create_binary(NULL, op, left, right);
        } else {
            break;
        }
    }
    return left;
}

// parses relational comparison operations: =, <>, <, >, <=, >=
static EvalAstNode *parse_relational(LexerContext *lex) {
    EvalAstNode *left = parse_add_sub(lex);
    if (!left) return NULL;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EQ || tok.type == TOK_NE || tok.type == TOK_LT ||
            tok.type == TOK_GT || tok.type == TOK_LE || tok.type == TOK_GE) {
            BppTokenType op = tok.type;
            lex_next(lex);
            EvalAstNode *right = parse_add_sub(lex);
            if (!right) {
                eval_ast_free_tree(left);
                return NULL;
            }
            left = eval_ast_create_binary(NULL, op, left, right);
        } else {
            break;
        }
    }
    return left;
}

// parses logical and bitwise operations: AND, OR, XOR, IMP, EQV, SHL, SHR
static EvalAstNode *parse_logical(LexerContext *lex) {
    EvalAstNode *left = parse_relational(lex);
    if (!left) return NULL;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_AND || tok.type == TOK_OR || tok.type == TOK_XOR || tok.type == TOK_IMP || tok.type == TOK_EQV ||
            tok.type == TOK_SHL || tok.type == TOK_SHR ||
            is_tok_kw(tok, KW_AND, "AND", 3) || is_tok_kw(tok, KW_OR, "OR", 2) || is_tok_kw(tok, KW_XOR, "XOR", 3) ||
            is_tok_kw(tok, KW_IMP, "IMP", 3) || is_tok_kw(tok, KW_EQV, "EQV", 3) ||
            is_tok_kw(tok, KW_SHL, "SHL", 3) || is_tok_kw(tok, KW_SHR, "SHR", 3)) {
            BppTokenType op = tok.type;
            if (is_tok_kw(tok, KW_AND, "AND", 3)) op = TOK_AND;
            else if (is_tok_kw(tok, KW_OR, "OR", 2)) op = TOK_OR;
            else if (is_tok_kw(tok, KW_XOR, "XOR", 3)) op = TOK_XOR;
            else if (is_tok_kw(tok, KW_IMP, "IMP", 3)) op = TOK_IMP;
            else if (is_tok_kw(tok, KW_EQV, "EQV", 3)) op = TOK_EQV;
            else if (is_tok_kw(tok, KW_SHL, "SHL", 3)) op = TOK_SHL;
            else if (is_tok_kw(tok, KW_SHR, "SHR", 3)) op = TOK_SHR;
            lex_next(lex);
            EvalAstNode *right = parse_relational(lex);
            if (!right) {
                eval_ast_free_tree(left);
                return NULL;
            }
            left = eval_ast_create_binary(NULL, op, left, right);
        } else {
            break;
        }
    }
    return left;
}

static EvalAstNode *parse_expr_internal(LexerContext *lex) {
    return parse_logical(lex);
}

EvalAstNode *eval_ast_parse_expression(LexerContext *lex) {
    return parse_expr_internal(lex);
}

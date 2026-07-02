/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ast.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

 // ---
 // BASIC++ Compiler - ast.c
 // ---
 //
 // AST builder implementation.
 //
 // DESIGN RATIONALE:
 // This module mirrors the structure of parser.c's parse-and-execute
 // functions, but instead of calling runtime functions, it builds
 // AstExpr and AstStmt nodes. The grammar and precedence rules are
 // identical to the interpreter's parser.
 //
 // Memory management uses malloc/free. Each node is individually
 // allocated. The caller (compiler.c) is responsible for calling
 // ast_free_line() after code generation completes.
 //
//
// HOW TO EXTEND:
//   Adding support for a new statement in code generation:
//   1. Add the AST node type in ast.h.
//   2. Add the emit case in this file's switch statement.
//   3. Generate the corresponding C code output.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "dialect.h"
#include "funcreg.h"
#include "errors.h"

// --- Internal forward declarations ---
static AstStmt *build_statement(Lexer *lex, int line_num);

// --- Expression node constructors ---

static AstExpr *expr_new(AstExprType type)
{
 AstExpr *e = (AstExpr *)malloc(sizeof(AstExpr));
 if (e) {
 memset(e, 0, sizeof(AstExpr));
 e->type = type;
 }
 return e;
}

static AstExpr *expr_int(long val)
{
 AstExpr *e = expr_new(EXPR_INT_LIT);
 if (e) e->v.ival = val;
 return e;
}

static AstExpr *expr_float(double val)
{
 AstExpr *e = expr_new(EXPR_FLOAT_LIT);
 if (e) e->v.fval = val;
 return e;
}

static AstExpr *expr_string(const char *data, int length)
{
 AstExpr *e = expr_new(EXPR_STRING_LIT);
 if (e) {
 e->v.sval.data = (char *)malloc((size_t)(length + 1));
 if (e->v.sval.data) {
 memcpy(e->v.sval.data, data, (size_t)length);
 e->v.sval.data[length] = '\0';
 }
 e->v.sval.length = length;
 }
 return e;
}

static AstExpr *expr_var(char name)
{
 AstExpr *e = expr_new(EXPR_VAR);
 if (e) e->v.var_name = name;
 return e;
}

static AstExpr *expr_string_var(char name)
{
 AstExpr *e = expr_new(EXPR_STRING_VAR);
 if (e) e->v.var_name = name;
 return e;
}

static AstExpr *expr_named_var(const char *name, int len)
{
 AstExpr *e = expr_new(EXPR_NAMED_VAR);
 if (e) {
 int clen = len;
 if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
 memcpy(e->v.named.name, name, (size_t)clen);
 e->v.named.name[clen] = '\0';
 e->v.named.name_len = clen;
 }
 return e;
}

static AstExpr *expr_binop(AstBinOp op, AstExpr *left, AstExpr *right)
{
 AstExpr *e = expr_new(EXPR_BINOP);
 if (e) {
 e->v.binop.op = op;
 e->v.binop.left = left;
 e->v.binop.right = right;
 }
 return e;
}

static AstExpr *expr_unop(AstUnOp op, AstExpr *operand)
{
 AstExpr *e = expr_new(EXPR_UNOP);
 if (e) {
 e->v.unop.op = op;
 e->v.unop.operand = operand;
 }
 return e;
}

static AstExpr *expr_func(AstFuncId func, int argc, AstExpr *a0,
 AstExpr *a1, AstExpr *a2)
{
 AstExpr *e = expr_new(EXPR_FUNC_CALL);
 if (e) {
 e->v.func_call.func = func;
 e->v.func_call.arg_count = argc;
 e->v.func_call.args[0] = a0;
 e->v.func_call.args[1] = a1;
 e->v.func_call.args[2] = a2;
 e->v.func_call.builtin_kw = 0;
 }
 return e;
}

static AstExpr *expr_array_at(AstExpr *index)
{
 AstExpr *e = expr_new(EXPR_ARRAY_AT);
 if (e) e->v.array_at.index = index;
 return e;
}

static AstExpr *expr_dim_access(const char *name, int nlen,
 AstExpr *idx1, AstExpr *idx2)
{
 AstExpr *e = expr_new(EXPR_DIM_ACCESS);
 if (e) {
 int clen = nlen;
 if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
 memcpy(e->v.dim_access.name, name, (size_t)clen);
 e->v.dim_access.name[clen] = '\0';
 e->v.dim_access.name_len = clen;
 e->v.dim_access.idx1 = idx1;
 e->v.dim_access.idx2 = idx2;
 }
 return e;
}

// --- Statement node constructor ---
static AstStmt *stmt_new(AstStmtType type)
{
 AstStmt *s = (AstStmt *)malloc(sizeof(AstStmt));
 if (s) {
 memset(s, 0, sizeof(AstStmt));
 s->type = type;
 }
 return s;
}

// --- Expression Builder - mirrors parse_expression_bval ---

enum {
    OP_UNARY_PLUS = 2000,
    OP_UNARY_MINUS,
    OP_NOT,
    OP_AT,
    OP_FUNC,
    OP_USER_FUNC,
    OP_ARRAY,
    OP_LPAREN
};

static int ast_apply_operator(AstExpr **val_stack, int *val_top, int op,
                              AstFuncId func_id, int builtin_kw, int arg_count, char fn_letter,
                              const char *name, int name_len, int line_num)
{
    if (op == TOK_PLUS || op == TOK_MINUS || op == TOK_STAR || op == TOK_SLASH ||
        op == TOK_CARET || op == TOK_EQUALS || op == TOK_NOT_EQ || op == TOK_LT ||
        op == TOK_GT || op == TOK_LT_EQ || op == TOK_GT_EQ || op == TOK_HASH ||
        op == KW_AND || op == KW_OR || op == KW_MOD) {
        
        if (*val_top < 2) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        AstExpr *right = val_stack[--(*val_top)];
        AstExpr *left = val_stack[--(*val_top)];
        
        AstBinOp bop;
        switch (op) {
            case TOK_PLUS: bop = BOP_ADD; break;
            case TOK_MINUS: bop = BOP_SUB; break;
            case TOK_STAR: bop = BOP_MUL; break;
            case TOK_SLASH: bop = BOP_DIV; break;
            case TOK_CARET: bop = BOP_POW; break;
            case TOK_EQUALS: bop = BOP_EQ; break;
            case TOK_NOT_EQ: bop = BOP_NE; break;
            case TOK_HASH: bop = BOP_NE; break;
            case TOK_LT: bop = BOP_LT; break;
            case TOK_GT: bop = BOP_GT; break;
            case TOK_LT_EQ: bop = BOP_LE; break;
            case TOK_GT_EQ: bop = BOP_GE; break;
            case KW_AND: bop = BOP_AND; break;
            case KW_OR: bop = BOP_OR; break;
            case KW_MOD: bop = BOP_MOD; break;
            default: bop = BOP_ADD; break;
        }
        val_stack[(*val_top)++] = expr_binop(bop, left, right);
    } else if (op == OP_UNARY_PLUS) {
        // Unary plus is a no-op, just leave it as-is
    } else if (op == OP_UNARY_MINUS) {
        if (*val_top < 1) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        AstExpr *val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = expr_unop(UOP_NEG, val);
    } else if (op == OP_NOT) {
        if (*val_top < 1) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        AstExpr *val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = expr_unop(UOP_NOT, val);
    } else if (op == OP_AT) {
        if (*val_top < 1) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        AstExpr *val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = expr_array_at(val);
    } else if (op == OP_FUNC) {
        AstExpr *args[3] = {NULL, NULL, NULL};
        int i;
        if (*val_top < arg_count || arg_count > 3) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        for (i = arg_count - 1; i >= 0; i--) {
            args[i] = val_stack[--(*val_top)];
        }
        AstExpr *f_expr = expr_func(func_id, arg_count, args[0], args[1], args[2]);
        if (f_expr) {
            f_expr->v.func_call.builtin_kw = builtin_kw;
        }
        val_stack[(*val_top)++] = f_expr;
    } else if (op == OP_USER_FUNC) {
        if (*val_top < 1) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        AstExpr *arg = val_stack[--(*val_top)];
        AstExpr *fn_expr = (AstExpr *)calloc(1, sizeof(AstExpr));
        if (fn_expr) {
            fn_expr->type = EXPR_FUNC_CALL;
            fn_expr->v.func_call.func = FUNC_FN_USER;
            fn_expr->v.func_call.args[0] = arg;
            fn_expr->v.func_call.fn_letter = fn_letter;
            fn_expr->v.func_call.arg_count = 1;
        }
        val_stack[(*val_top)++] = fn_expr;
    } else if (op == OP_ARRAY) {
        AstExpr *args[3] = {NULL, NULL, NULL};
        int i;
        if (*val_top < arg_count || arg_count > 3) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        for (i = arg_count - 1; i >= 0; i--) {
            args[i] = val_stack[--(*val_top)];
        }
        val_stack[(*val_top)++] = expr_dim_access(name, name_len, args[0], args[1]);
        if (arg_count > 2 && args[2] != NULL) {
            ast_free_expr(args[2]);
        }
    }
    return 1;
}

static int ast_get_op_precedence(int op, int is_unary)
{
    if (is_unary) {
        if (op == OP_UNARY_PLUS || op == OP_UNARY_MINUS) return 8;
        if (op == OP_NOT) return 3;
        return 9;
    }
    switch (op) {
        case KW_OR: return 1;
        case KW_AND: return 2;
        case TOK_EQUALS:
        case TOK_NOT_EQ:
        case TOK_HASH:
        case TOK_LT:
        case TOK_GT:
        case TOK_LT_EQ:
        case TOK_GT_EQ:
            return 4;
        case TOK_PLUS:
        case TOK_MINUS:
            return 5;
        case TOK_STAR:
        case TOK_SLASH:
        case KW_MOD:
            return 6;
        case TOK_CARET:
            return 7;
        default:
            return 0;
    }
}

AstExpr *ast_build_expr(Lexer *lex, int line_num)
{
    AstExpr *val_stack[64];
    int val_top = 0;
    
    struct {
        int op;
        int precedence;
        int assoc;
        int is_unary;
        AstFuncId func_id;
        char fn_letter;
        char name[MAX_VAR_NAME_LEN + 1];
        int name_len;
        int arg_count;
        int builtin_kw;
    } op_stack[64];
    int op_top = 0;
    
    int arg_count_stack[64];
    int arg_count_top = 0;
    int expect_operand = 1;
    int i;
    
    if (error_occurred()) return NULL;
    
    while (!error_occurred()) {
        Token tok = lex->current;
        
        if (!expect_operand) {
            if (tok.type == TOK_CR || tok.type == TOK_EOF || tok.type == TOK_COLON) {
                break;
            }
            if (tok.type == TOK_KEYWORD) {
                KeywordId kw = tok.value.keyword;
                if (kw != KW_AND && kw != KW_OR && kw != KW_MOD) {
                    break;
                }
            }
            
            if (tok.type == TOK_COMMA || tok.type == TOK_RPAREN) {
                int has_lparen = 0;
                for (i = 0; i < op_top; i++) {
                    if (op_stack[i].op == OP_LPAREN) {
                        has_lparen = 1;
                        break;
                    }
                }
                if (!has_lparen) {
                    break;
                }
            }
        }
        
        if (expect_operand) {
            if (tok.type == TOK_NUMBER) {
                val_stack[val_top++] = expr_int(tok.value.num_value);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_FLOAT_LIT) {
                val_stack[val_top++] = expr_float(tok.value.fval);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_STRING) {
                val_stack[val_top++] = expr_string(tok.str_start, tok.str_length);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_PLUS) {
                op_stack[op_top].op = OP_UNARY_PLUS;
                op_stack[op_top].precedence = 8;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 1;
                op_top++;
                lexer_next(lex);
            } else if (tok.type == TOK_MINUS) {
                op_stack[op_top].op = OP_UNARY_MINUS;
                op_stack[op_top].precedence = 8;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 1;
                op_top++;
                lexer_next(lex);
            } else if (tok.type == TOK_KEYWORD && tok.value.keyword == KW_NOT) {
                op_stack[op_top].op = OP_NOT;
                op_stack[op_top].precedence = 3;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 1;
                op_top++;
                lexer_next(lex);
            } else if (tok.type == TOK_LPAREN) {
                op_stack[op_top].op = OP_LPAREN;
                op_stack[op_top].precedence = 0;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 0;
                op_top++;
                lexer_next(lex);
            } else if (tok.type == TOK_AT) {
                lexer_next(lex);
                if (!lexer_expect(lex, TOK_LPAREN)) goto clean_fail;
                
                op_stack[op_top].op = OP_AT;
                op_stack[op_top].precedence = 9;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 1;
                op_top++;
                
                op_stack[op_top].op = OP_LPAREN;
                op_stack[op_top].precedence = 0;
                op_stack[op_top].assoc = 0;
                op_stack[op_top].is_unary = 0;
                op_top++;
            } else if (tok.type == TOK_VARIABLE || tok.type == TOK_STRING_VAR || tok.type == TOK_NAMED_VAR) {
                char nm[MAX_VAR_NAME_LEN + 1];
                int nlen = 0;
                
                if (tok.type == TOK_VARIABLE) {
                    nm[0] = tok.value.var_name;
                    nm[1] = '\0';
                    nlen = 1;
                } else if (tok.type == TOK_STRING_VAR) {
                    nm[0] = tok.value.var_name;
                    nm[1] = '$';
                    nm[2] = '\0';
                    nlen = 2;
                } else {
                    nlen = tok.str_length < MAX_VAR_NAME_LEN ? tok.str_length : MAX_VAR_NAME_LEN;
                    memcpy(nm, tok.str_start, (size_t)nlen);
                    nm[nlen] = '\0';
                }
                
                lexer_next(lex);
                
                if (lex->current.type == TOK_LPAREN) {
                    lexer_next(lex);
                    
                    if (nlen >= 3 && (nm[0] == 'F' || nm[0] == 'f') && (nm[1] == 'N' || nm[1] == 'n')) {
                        op_stack[op_top].op = OP_USER_FUNC;
                        op_stack[op_top].fn_letter = nm[2];
                    } else {
                        op_stack[op_top].op = OP_ARRAY;
                        memcpy(op_stack[op_top].name, nm, (size_t)nlen + 1);
                        op_stack[op_top].name_len = nlen;
                    }
                    op_stack[op_top].precedence = 9;
                    op_stack[op_top].assoc = 0;
                    op_stack[op_top].is_unary = 1;
                    op_top++;
                    
                    op_stack[op_top].op = OP_LPAREN;
                    op_stack[op_top].precedence = 0;
                    op_stack[op_top].assoc = 0;
                    op_stack[op_top].is_unary = 0;
                    op_top++;
                    
                    arg_count_stack[arg_count_top++] = 1;
                } else {
                    if (tok.type == TOK_VARIABLE) {
                        val_stack[val_top++] = expr_var(tok.value.var_name);
                    } else if (tok.type == TOK_STRING_VAR) {
                        val_stack[val_top++] = expr_string_var(tok.value.var_name);
                    } else {
                        val_stack[val_top++] = expr_named_var(nm, nlen);
                    }
                    expect_operand = 0;
                }
            } else if (tok.type == TOK_KEYWORD) {
                KeywordId kw = tok.value.keyword;
                AstFuncId fid;
                int argc = 1;
                int has_args = 1;
                
                switch (kw) {
                    case KW_ABS: fid = FUNC_ABS; break;
                    case KW_RND: fid = FUNC_RND; break;
                    case KW_SIZE: fid = FUNC_SIZE; argc = 0; has_args = 0; break;
                    case KW_SIN: fid = FUNC_SIN; break;
                    case KW_COS: fid = FUNC_COS; break;
                    case KW_TAN: fid = FUNC_TAN; break;
                    case KW_ATN: fid = FUNC_ATN; break;
                    case KW_SQR: fid = FUNC_SQR; break;
                    case KW_LOG_FUNC: fid = FUNC_LOG; break;
                    case KW_EXP: fid = FUNC_EXP; break;
                    case KW_SGN: fid = FUNC_SGN; break;
                    case KW_INT_FUNC: fid = FUNC_INT; break;
                    case KW_LEN: fid = FUNC_LEN; break;
                    case KW_ASC: fid = FUNC_ASC; break;
                    case KW_VAL_FUNC: fid = FUNC_VAL; break;
                    case KW_CHR: fid = FUNC_CHR; break;
                    case KW_STR_FUNC: fid = FUNC_STR; break;
                    case KW_LEFT: fid = FUNC_LEFT; argc = 2; break;
                    case KW_RIGHT: fid = FUNC_RIGHT; argc = 2; break;
                    case KW_MID: fid = FUNC_MID; argc = 3; break;
                    case KW_TAB_FUNC: fid = FUNC_TAB; break;
                    case KW_MEMMAP_FUNC: fid = FUNC_MEMMAP; argc = 0; has_args = 0; break;
                    case KW_VPATH_FUNC: fid = FUNC_VPATH; argc = 0; has_args = 0; break;
                    case KW_CWD_FUNC: fid = FUNC_CWD; argc = 0; has_args = 0; break;
                    case KW_PWD: fid = FUNC_PWD; argc = 0; has_args = 0; break;
                    default:
                    {
                        const FunctionEntry *fn = funcreg_find_by_keyword(kw);
                        if (fn != NULL) {
                            fid = FUNC_BUILTIN;
                            argc = fn->min_args;
                            has_args = (fn->max_args > 0) ? 1 : 0;
                        } else {
                            error_raise(ERR_WHAT, line_num);
                            goto clean_fail;
                        }
                        break;
                    }
                }
                
                lexer_next(lex);
                
                if (has_args) {
                    if (!lexer_expect(lex, TOK_LPAREN)) goto clean_fail;
                    
                    op_stack[op_top].op = OP_FUNC;
                    op_stack[op_top].func_id = fid;
                    op_stack[op_top].precedence = 9;
                    op_stack[op_top].assoc = 0;
                    op_stack[op_top].is_unary = 1;
                    op_stack[op_top].builtin_kw = (fid == FUNC_BUILTIN) ? kw : 0;
                    op_top++;
                    
                    op_stack[op_top].op = OP_LPAREN;
                    op_stack[op_top].precedence = 0;
                    op_stack[op_top].assoc = 0;
                    op_stack[op_top].is_unary = 0;
                    op_top++;
                    
                    arg_count_stack[arg_count_top++] = 1;
                } else {
                    AstExpr *f_expr = expr_func(fid, 0, NULL, NULL, NULL);
                    if (f_expr && fid == FUNC_BUILTIN) {
                        f_expr->v.func_call.builtin_kw = kw;
                    }
                    val_stack[val_top++] = f_expr;
                    expect_operand = 0;
                }
            } else {
                error_raise(ERR_WHAT, line_num);
                goto clean_fail;
            }
        } else {
            if (tok.type == TOK_COMMA) {
                while (op_top > 0 && op_stack[op_top - 1].op != OP_LPAREN) {
                    int top_op = op_stack[--op_top].op;
                    if (!ast_apply_operator(val_stack, &val_top, top_op,
                                            op_stack[op_top].func_id,
                                            op_stack[op_top].builtin_kw,
                                            op_stack[op_top].arg_count,
                                            op_stack[op_top].fn_letter,
                                            op_stack[op_top].name,
                                            op_stack[op_top].name_len,
                                            line_num)) {
                        goto clean_fail;
                    }
                }
                if (op_top == 0 || arg_count_top == 0) {
                    error_raise(ERR_WHAT, line_num);
                    goto clean_fail;
                }
                arg_count_stack[arg_count_top - 1]++;
                lexer_next(lex);
                expect_operand = 1;
            } else if (tok.type == TOK_RPAREN) {
                while (op_top > 0 && op_stack[op_top - 1].op != OP_LPAREN) {
                    int top_op = op_stack[--op_top].op;
                    if (!ast_apply_operator(val_stack, &val_top, top_op,
                                            op_stack[op_top].func_id,
                                            op_stack[op_top].builtin_kw,
                                            op_stack[op_top].arg_count,
                                            op_stack[op_top].fn_letter,
                                            op_stack[op_top].name,
                                            op_stack[op_top].name_len,
                                            line_num)) {
                        goto clean_fail;
                    }
                }
                if (op_top == 0) {
                    error_raise(ERR_WHAT, line_num);
                    goto clean_fail;
                }
                op_top--;
                
                if (op_top > 0 && op_stack[op_top - 1].is_unary) {
                    int top_op = op_stack[--op_top].op;
                    int ac = 1;
                    if (top_op == OP_FUNC || top_op == OP_ARRAY) {
                        if (arg_count_top > 0) {
                            ac = arg_count_stack[--arg_count_top];
                        }
                    }
                    if (!ast_apply_operator(val_stack, &val_top, top_op,
                                            op_stack[op_top].func_id,
                                            op_stack[op_top].builtin_kw,
                                            ac,
                                            op_stack[op_top].fn_letter,
                                            op_stack[op_top].name,
                                            op_stack[op_top].name_len,
                                            line_num)) {
                        goto clean_fail;
                    }
                }
                lexer_next(lex);
            } else {
                int token_op = tok.type;
                if (tok.type == TOK_KEYWORD) {
                    token_op = tok.value.keyword;
                }
                int precedence = ast_get_op_precedence(token_op, 0);
                if (precedence == 0) {
                    break;
                }
                
                while (op_top > 0) {
                    int top_op = op_stack[op_top - 1].op;
                    int top_prec = op_stack[op_top - 1].precedence;
                    
                    if (top_prec > precedence || (top_prec == precedence && op_stack[op_top - 1].assoc == 0)) {
                        op_top--;
                        if (!ast_apply_operator(val_stack, &val_top, top_op,
                                                op_stack[op_top].func_id,
                                                op_stack[op_top].builtin_kw,
                                                op_stack[op_top].arg_count,
                                                op_stack[op_top].fn_letter,
                                                op_stack[op_top].name,
                                                op_stack[op_top].name_len,
                                                line_num)) {
                            goto clean_fail;
                        }
                    } else {
                        break;
                    }
                }
                
                op_stack[op_top].op = token_op;
                op_stack[op_top].precedence = precedence;
                op_stack[op_top].assoc = (token_op == TOK_CARET) ? 1 : 0;
                op_stack[op_top].is_unary = 0;
                op_top++;
                
                lexer_next(lex);
                expect_operand = 1;
            }
        }
    }
    
    while (op_top > 0) {
        int top_op = op_stack[--op_top].op;
        if (top_op == OP_LPAREN) {
            error_raise(ERR_WHAT, line_num);
            goto clean_fail;
        }
        if (!ast_apply_operator(val_stack, &val_top, top_op,
                                op_stack[op_top].func_id,
                                op_stack[op_top].builtin_kw,
                                op_stack[op_top].arg_count,
                                op_stack[op_top].fn_letter,
                                op_stack[op_top].name,
                                op_stack[op_top].name_len,
                                line_num)) {
            goto clean_fail;
        }
    }
    
    if (val_top == 1) {
        return val_stack[0];
    }
    
    error_raise(ERR_WHAT, line_num);
    
clean_fail:
    for (i = 0; i < val_top; i++) {
        ast_free_expr(val_stack[i]);
    }
    return NULL;
}

// --- Statement Builders ---

 // build_print - Parse PRINT statement into AST.
static AstStmt *build_print(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_PRINT);
 AstPrintItem *items = NULL;
 int count = 0;
 int capacity = 8;
 char sep = dialect_get_separator();

 if (!s) return NULL;

 items = (AstPrintItem *)malloc(sizeof(AstPrintItem) * (size_t)capacity);
 if (!items) { free(s); return NULL; }

 s->v.print.trailing_comma = 0;

 // Handle empty PRINT (newline only)
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':') ||
 (lex->current.type == TOK_SEMICOLON && sep == ';')) {
 s->v.print.items = items;
 s->v.print.item_count = 0;
 return s;
 }

 while (!error_occurred()) {
 AstPrintItem item;
 memset(&item, 0, sizeof(item));

 // Handle comma at current position (leading or double comma)
 if (lex->current.type == TOK_COMMA) {
 // Insert a NULL-expr item to represent tab advance
 item.expr = NULL;
 item.suppress_space = 0;
 if (count >= capacity) {
 capacity *= 2;
 items = (AstPrintItem *)realloc(items,
 sizeof(AstPrintItem) * (size_t)capacity);
 }
 items[count++] = item;
 lexer_next(lex);
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 continue;
 }

 // Check for #width
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex);
 item.expr = ast_build_expr(lex, line_num);
 if (error_occurred()) break;
 item.is_hash_width = 1;
 } else {
 item.expr = ast_build_expr(lex, line_num);
 if (error_occurred()) break;
 }

 // Store item
 if (count >= capacity) {
 capacity *= 2;
 items = (AstPrintItem *)realloc(items,
 sizeof(AstPrintItem) * (size_t)capacity);
 }
 items[count++] = item;

 // Check separator
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 items[count - 1].suppress_space = 0;
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 } else if (lex->current.type == TOK_SEMICOLON) {
 lexer_next(lex);
 items[count - 1].suppress_space = 1;
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type == TOK_COLON && sep == ':')) {
 s->v.print.trailing_comma = 1;
 break;
 }
 } else {
 break;
 }
 }

 if (error_occurred()) {
 int i;
 for (i = 0; i < count; i++) ast_free_expr(items[i].expr);
 free(items);
 free(s);
 return NULL;
 }

 s->v.print.items = items;
 s->v.print.item_count = count;
 return s;
}

 // build_let - Parse LET/assignment into AST.
static AstStmt *build_let(Lexer *lex, int line_num)
{
 // @() array assignment
 if (lex->current.type == TOK_AT) {
 AstStmt *s = stmt_new(STMT_LET_ARRAY_AT);
 AstExpr *idx, *val;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) { free(s); return NULL; }
 idx = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx); free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx); free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(idx); free(s); return NULL; }
 s->v.let_array_at.index = idx;
 s->v.let_array_at.value = val;
 return s;
 }

 // Standard variable
 if (lex->current.type == TOK_VARIABLE) {
 char name = lex->current.value.var_name;
 lexer_next(lex);

 // Check for DIM array assignment: A(i) = expr
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *idx1, *idx2 = NULL, *val;
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); free(s); return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 s->v.let_dim.name[0] = name;
 s->v.let_dim.name[1] = '\0';
 s->v.let_dim.name_len = 1;
 s->v.let_dim.idx1 = idx1;
 s->v.let_dim.idx2 = idx2;
 s->v.let_dim.value = val;
 return s;
 }

 // Simple variable assignment
 {
 AstStmt *s = stmt_new(STMT_LET);
 AstExpr *val;
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let.var_name = name;
 s->v.let.value = val;
 return s;
 }
 }

 // Named variable
 if (lex->current.type == TOK_NAMED_VAR) {
 char nm[MAX_VAR_NAME_LEN + 1];
 int nlen = lex->current.str_length;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(nm, lex->current.str_start, (size_t)nlen);
 nm[nlen] = '\0';
 lexer_next(lex);

 // Check for DIM array assignment: A1(i) = expr or T1(G1) = expr
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *idx1, *idx2 = NULL, *val;
 lexer_next(lex);
 idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); free(s); return NULL;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 if (!lexer_expect(lex, TOK_EQUALS)) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(idx1); ast_free_expr(idx2);
 free(s); return NULL;
 }
 memcpy(s->v.let_dim.name, nm, (size_t)(nlen + 1));
 s->v.let_dim.name_len = nlen;
 s->v.let_dim.idx1 = idx1;
 s->v.let_dim.idx2 = idx2;
 s->v.let_dim.value = val;
 return s;
 }

 // Simple named variable assignment
 {
 int is_str = (nlen > 0 && nm[nlen - 1] == '$');
 if (is_str) {
 // String named var: X0$ = expr
 AstStmt *s = stmt_new(STMT_LET_STRVAR);
 AstExpr *val;
 s->v.let_strvar.var_name = nm[0];
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_strvar.value = val;
 return s;
 } else {
 AstStmt *s = stmt_new(STMT_LET);
 AstExpr *val;
 s->v.let.var_name = nm[0];
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let.value = val;
 return s;
 }
 }
 }

 // String variable
 if (lex->current.type == TOK_STRING_VAR) {
 char sv_name = lex->current.value.var_name;
 lexer_next(lex);
 // Check for string array subscript: A$(idx) = expr
 if (lex->current.type == TOK_LPAREN) {
 AstStmt *s = stmt_new(STMT_LET_DIM);
 AstExpr *val;
 s->v.let_dim.name[0] = sv_name;
 s->v.let_dim.name[1] = '$';
 s->v.let_dim.name[2] = '\0';
 lexer_next(lex); // consume (
 s->v.let_dim.idx1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_dim.idx2 = NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->v.let_dim.idx2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) { free(s); return NULL; }
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_dim.value = val;
 return s;
 }
 // Simple string variable: A$ = expr
 {
 AstStmt *s = stmt_new(STMT_LET_STRVAR);
 AstExpr *val;
 s->v.let_strvar.var_name = sv_name;
 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }
 val = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.let_strvar.value = val;
 return s;
 }
 }

 error_raise(ERR_WHAT, line_num);
 return NULL;
}

 // build_if - Parse IF statement into AST.
 //
 // Now uses a full boolean expression for the condition.
 // Comparisons (=, <>, <, >, <=, >=) and logical operators
 // (AND, OR, NOT) are handled by the expression parser.
static AstStmt *build_if(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_IF);

 if (!s) return NULL;

 // Parse the full condition expression
 s->v.if_stmt.condition = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 // Optional THEN keyword
 if (dialect_get_config()->has_then_keyword &&
 lexer_match_keyword(lex, KW_THEN)) {
 lexer_next(lex);
 }

 // IF cond THEN <linenumber> - synthesize GOTO
 if (lex->current.type == TOK_NUMBER) {
 AstStmt *g = stmt_new(STMT_GOTO);
 g->v.goto_stmt.target = ast_build_expr(lex, line_num);
 s->v.if_stmt.then_stmt = g;
 return s;
 }

 s->v.if_stmt.then_stmt = build_statement(lex, line_num);
 return s;
}

 // build_for - Parse FOR statement.
static AstStmt *build_for(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_FOR);
 if (!s) return NULL;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.for_stmt.var_name = lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 // Multi-char var (R1, Q4, DELAY) - use first letter
 s->v.for_stmt.var_name = (char)(lex->current.str_start[0]);
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) { free(s); return NULL; }

 s->v.for_stmt.init = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 // Expect TO keyword
 if (!lexer_match_keyword(lex, KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 ast_free_expr(s->v.for_stmt.init); free(s);
 return NULL;
 }
 lexer_next(lex);

 s->v.for_stmt.limit = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.for_stmt.init); free(s);
 return NULL;
 }

 // Optional STEP
 s->v.for_stmt.step = NULL;
 if (lexer_match_keyword(lex, KW_STEP)) {
 lexer_next(lex);
 s->v.for_stmt.step = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.for_stmt.init);
 ast_free_expr(s->v.for_stmt.limit);
 free(s); return NULL;
 }
 }

 return s;
}

 // build_input - Parse INPUT statement.
static AstStmt *build_input(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_INPUT);
 (void)line_num; // used by ast_build_expr below
 if (!s) return NULL;

 s->v.input.prompt = NULL;
 s->v.input.var_count = 0;

 // Optional prompt string
 if (lex->current.type == TOK_STRING) {
 s->v.input.prompt = expr_string(lex->current.str_start,
 lex->current.str_length);
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA) lexer_next(lex);
 }

 // Variable list
 while (!error_occurred()) {
 int idx = s->v.input.var_count;
 if (idx >= 26) break;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.input.var_names[idx] = lex->current.value.var_name;
 s->v.input.var_types[idx] = 0;
 s->v.input.var_count++;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 s->v.input.var_names[idx] = lex->current.value.var_name;
 s->v.input.var_types[idx] = 1;
 s->v.input.var_count++;
 lexer_next(lex);
 } else {
 break;
 }

 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 } else {
 break;
 }
 }

 return s;
}

 // build_dim - Parse DIM statement.
static AstStmt *build_dim(Lexer *lex, int line_num)
{
 AstStmt *s = stmt_new(STMT_DIM);
 if (!s) return NULL;

 if (lex->current.type == TOK_VARIABLE) {
 s->v.dim.name[0] = lex->current.value.var_name;
 s->v.dim.name[1] = '\0';
 s->v.dim.name_len = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 // String array: DIM A$(20)
 s->v.dim.name[0] = lex->current.value.var_name;
 s->v.dim.name[1] = '$';
 s->v.dim.name[2] = '\0';
 s->v.dim.name_len = 2;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 int nlen = lex->current.str_length;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(s->v.dim.name, lex->current.str_start, (size_t)nlen);
 s->v.dim.name[nlen] = '\0';
 s->v.dim.name_len = nlen;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 free(s); return NULL;
 }

 if (!lexer_expect(lex, TOK_LPAREN)) { free(s); return NULL; }

 s->v.dim.dim1 = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }

 s->v.dim.dim2 = NULL;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->v.dim.dim2 = ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.dim.dim1);
 free(s); return NULL;
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(s->v.dim.dim1);
 ast_free_expr(s->v.dim.dim2);
 free(s); return NULL;
 }

 // Handle comma-separated multiple DIMs: DIM A$(20),B(10),C(5)
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s->next = build_dim(lex, line_num);
 }

 return s;
}

 // build_statement - Parse one statement into AST node.
static AstStmt *build_statement(Lexer *lex, int line_num)
{
 const char *stmt_start = lex->source + lex->current.pos;
 if (error_occurred()) return NULL;

 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;

 // Handle REM before lexer_next - the text after REM
 // is raw comment, not valid tokens 
 if (kw == KW_REM) {
 AstStmt *s = stmt_new(STMT_REM);
 // Position past "REM" keyword
 s->v.rem.text = lex->source + lex->pos;
 lexer_skip_to_end(lex);
 return s;
 }
 }

 // Handle identifiers starting with REM (e.g. REMARKABLE, REMEMBER)
 // GW-BASIC greedily matches REM and treats rest as comment 
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length >= 3) {
 char c0 = lex->current.str_start[0];
 char c1 = lex->current.str_start[1];
 char c2 = lex->current.str_start[2];
 if ((c0 == 'R' || c0 == 'r') &&
 (c1 == 'E' || c1 == 'e') &&
 (c2 == 'M' || c2 == 'm')) {
 AstStmt *s = stmt_new(STMT_REM);
 // Include everything from after REM to end of line
 s->v.rem.text = lex->current.str_start + 3;
 lexer_skip_to_end(lex);
 return s;
 }
 }

 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;

 lexer_next(lex);

 switch (kw) {
 case KW_PRINT:
 return build_print(lex, line_num);
 case KW_LET:
 return build_let(lex, line_num);
 case KW_INPUT:
 return build_input(lex, line_num);
 case KW_IF:
 return build_if(lex, line_num);
 case KW_GOTO:
 {
 AstStmt *s = stmt_new(STMT_GOTO);
 s->v.goto_stmt.target = ast_build_expr(lex, line_num);
 return s;
 }
 case KW_GOSUB:
 {
 AstStmt *s = stmt_new(STMT_GOSUB);
 s->v.gosub.target = ast_build_expr(lex, line_num);
 return s;
 }
 case KW_RETURN:
 return stmt_new(STMT_RETURN);
 case KW_FOR:
 return build_for(lex, line_num);
 case KW_NEXT:
 {
 AstStmt *s = stmt_new(STMT_NEXT);
 if (lex->current.type == TOK_VARIABLE) {
 s->v.next.var_name = lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 s->v.next.var_name = (char)(lex->current.str_start[0]);
 lexer_next(lex);
 } else {
 s->v.next.var_name = '\0';
 }
 return s;
 }
 case KW_END:
 {
     if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_WHEN) {
         lexer_next(lex);
         return stmt_new(STMT_END_WHEN);
     }
     if (lex->current.type == TOK_KEYWORD && 
         (lex->current.value.keyword == KW_IF || 
          lex->current.value.keyword == KW_SUB || 
          lex->current.value.keyword == KW_FUNCTION)) {
         lexer_next(lex);
     }
     return stmt_new(STMT_END);
 }
 case KW_STOP:
 return stmt_new(STMT_STOP);
 case KW_REM:
 // Handled before lexer_next above; unreachable
 break;
 case KW_DIM:
 return build_dim(lex, line_num);
 case KW_DATA:
 {
 AstStmt *s = stmt_new(STMT_DATA);
 // Skip DATA values - collected separately
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 lexer_next(lex);
 }
 return s;
 }
 case KW_READ:
 {
 AstStmt *s = stmt_new(STMT_READ);
 int ri;
 s->v.read.var_count = 0;
 for (ri = 0; ri < 26; ri++) {
 s->v.read.var_indices[ri] = NULL;
 s->v.read.var_indices2[ri] = NULL;
 s->v.read.dim_names[ri][0] = '\0';
 }
 while (!error_occurred()) {
 int idx = s->v.read.var_count;
 if (idx >= 26) break;
 if (lex->current.type == TOK_VARIABLE) {
 char vn = lex->current.value.var_name;
 s->v.read.var_names[idx] = vn;
 s->v.read.dim_names[idx][0] = vn;
 s->v.read.dim_names[idx][1] = '\0';
 lexer_next(lex);
 // Check for array subscript: READ A(expr) or A(e1,e2)
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *subscr, *subscr2 = NULL;
 lexer_next(lex);
 subscr = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 subscr2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(subscr); free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(subscr);
 ast_free_expr(subscr2);
 free(s); return NULL;
 }
 s->v.read.var_indices[idx] = subscr;
 s->v.read.var_indices2[idx] = subscr2;
 s->v.read.var_types[idx] = subscr2 ? 3 : 2;
 } else {
 s->v.read.var_types[idx] = 0;
 }
 s->v.read.var_count++;
 } else if (lex->current.type == TOK_NAMED_VAR) {
 // Named var like M, B, etc. with possible subscript
 int nlen = lex->current.str_length;
 char vn = lex->current.str_start[0];
 s->v.read.var_names[idx] = vn;
 if (nlen > MAX_VAR_NAME_LEN) nlen = MAX_VAR_NAME_LEN;
 memcpy(s->v.read.dim_names[idx],
 lex->current.str_start, (size_t)nlen);
 s->v.read.dim_names[idx][nlen] = '\0';
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 AstExpr *subscr, *subscr2 = NULL;
 lexer_next(lex);
 subscr = ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 subscr2 = ast_build_expr(lex, line_num);
 if (error_occurred()) { ast_free_expr(subscr); free(s); return NULL; }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) {
 ast_free_expr(subscr);
 ast_free_expr(subscr2);
 free(s); return NULL;
 }
 s->v.read.var_indices[idx] = subscr;
 s->v.read.var_indices2[idx] = subscr2;
 s->v.read.var_types[idx] = subscr2 ? 3 : 2;
 } else {
 s->v.read.var_types[idx] = 0;
 }
 s->v.read.var_count++;
 } else if (lex->current.type == TOK_STRING_VAR) {
 s->v.read.var_names[idx] =
 lex->current.value.var_name;
 s->v.read.var_types[idx] = 1;
 s->v.read.var_count++;
 lexer_next(lex);
 } else {
 break;
 }
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }
 return s;
 }
 case KW_RESTORE:
 return stmt_new(STMT_RESTORE);
 case KW_WHILE:
 {
 AstStmt *s = stmt_new(STMT_WHILE);
 s->v.while_stmt.left =
 ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 s->v.while_stmt.relop = lex->current.type;
 lexer_next(lex);
 s->v.while_stmt.right =
 ast_build_expr(lex, line_num);
 if (error_occurred()) {
 ast_free_expr(s->v.while_stmt.left);
 free(s); return NULL;
 }
 return s;
 }
 case KW_WEND:
 return stmt_new(STMT_WEND);
 case KW_DO:
 return stmt_new(STMT_DO);
 case KW_LOOP:
 return stmt_new(STMT_LOOP);
 case KW_ON:
 {
 AstStmt *s = stmt_new(STMT_ON_GOTO);
 s->v.on_goto.selector =
 ast_build_expr(lex, line_num);
 if (error_occurred()) { free(s); return NULL; }
 // Expect GOTO or GOSUB
 int is_gosub = 0;
 if (lexer_match_keyword(lex, KW_GOSUB)) {
     is_gosub = 1;
 } else if (!lexer_match_keyword(lex, KW_GOTO)) {
     error_raise(ERR_WHAT, line_num);
     ast_free_expr(s->v.on_goto.selector);
     free(s); return NULL;
 }
 lexer_next(lex);
 s->v.on_goto.is_gosub = is_gosub;
 s->v.on_goto.target_count = 0;
 while (!error_occurred()) {
 int ti = s->v.on_goto.target_count;
 if (ti >= 64) break;
 if (lex->current.type != TOK_NUMBER) break;
 s->v.on_goto.targets[ti] =
 (int)lex->current.value.num_value;
 s->v.on_goto.target_count++;
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }
 return s;
 }
 case KW_WHEN:
 {
     if (lex->current.type == TOK_NAMED_VAR) {
         lexer_next(lex);
     }
     if (lex->current.type == TOK_NAMED_VAR) {
         lexer_next(lex);
     }
     return stmt_new(STMT_WHEN);
 }
 case KW_USE:
     return stmt_new(STMT_USE);
 case KW_RETRY:
     return stmt_new(STMT_RETRY);
 case KW_CONTINUE:
     return stmt_new(STMT_CONTINUE);
 case KW_INT_FUNC:
 {
     AstStmt *s = stmt_new(STMT_INT);
     s->v.int_stmt.interrupt_number = ast_build_expr(lex, line_num);
     if (error_occurred()) { free(s); return NULL; }
     return s;
 }
 case KW_RUN:
 {
 // RUN "MENU" or RUN - skip arguments, emit as comment
 AstStmt *s = stmt_new(STMT_REM);
 s->v.rem.text = "RUN (compiled: restart)";
 // Skip any arguments
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 lexer_next(lex);
 }
 return s;
 }
 default:
 {
     AstStmt *s = stmt_new(STMT_DIRECT_EXEC);
     if (s) {
         while (lex->current.type != TOK_COLON &&
                lex->current.type != TOK_CR &&
                lex->current.type != TOK_EOF) {
             lexer_next(lex);
         }
         int start_idx = (int)(stmt_start - lex->source);
         int end_idx = lex->current.pos;
         int len = end_idx - start_idx;
         if (len < 0) len = 0;
         s->v.direct_exec.text = (char *)malloc((size_t)(len + 1));
         if (s->v.direct_exec.text) {
             memcpy(s->v.direct_exec.text, stmt_start, (size_t)len);
             s->v.direct_exec.text[len] = '\0';
         }
     }
     return s;
 }
 }
 }

 // Bare assignment (LET optional)
 if ((lex->current.type == TOK_VARIABLE ||
 lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_STRING_VAR ||
 lex->current.type == TOK_AT) &&
 dialect_get_config()->has_let_optional) {
 return build_let(lex, line_num);
 }

  if (lex->current.type != TOK_EOF && lex->current.type != TOK_CR) {
      AstStmt *s = stmt_new(STMT_DIRECT_EXEC);
      if (s) {
          while (lex->current.type != TOK_COLON &&
                 lex->current.type != TOK_CR &&
                 lex->current.type != TOK_EOF) {
              lexer_next(lex);
          }
          int start_idx = (int)(stmt_start - lex->source);
          int end_idx = lex->current.pos;
          int len = end_idx - start_idx;
          if (len < 0) len = 0;
          s->v.direct_exec.text = (char *)malloc((size_t)(len + 1));
          if (s->v.direct_exec.text) {
              memcpy(s->v.direct_exec.text, stmt_start, (size_t)len);
              s->v.direct_exec.text[len] = '\0';
          }
      }
      return s;
  }
  return NULL;
}

// --- Public API ---

 // ast_build_line - Parse a full line into a statement chain.
AstStmt *ast_build_line(Lexer *lex, int line_num)
{
 AstStmt *head = NULL;
 AstStmt *tail = NULL;
 char sep = dialect_get_separator();

 while (!error_occurred() &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {

 AstStmt *s = build_statement(lex, line_num);
 if (!s) break;

 if (head == NULL) {
 head = s;
 tail = s;
 } else {
 tail->next = s;
 tail = s;
 }

 // Check for statement separator
 if (lex->current.type == TOK_SEMICOLON && sep == ';') {
 lexer_next(lex);
 } else if (lex->current.type == TOK_COLON && sep == ':') {
 lexer_next(lex);
 } else {
 break;
 }
 }

 if (error_occurred()) {
 ast_free_line(head);
 return NULL;
 }

 return head;
}

// --- Destructor Functions ---

void ast_free_expr(AstExpr *expr)
{
 if (!expr) return;

 switch (expr->type) {
 case EXPR_STRING_LIT:
 free(expr->v.sval.data);
 break;
 case EXPR_ARRAY_AT:
 ast_free_expr(expr->v.array_at.index);
 break;
 case EXPR_DIM_ACCESS:
 ast_free_expr(expr->v.dim_access.idx1);
 ast_free_expr(expr->v.dim_access.idx2);
 break;
 case EXPR_BINOP:
 ast_free_expr(expr->v.binop.left);
 ast_free_expr(expr->v.binop.right);
 break;
 case EXPR_UNOP:
 ast_free_expr(expr->v.unop.operand);
 break;
 case EXPR_FUNC_CALL:
 {
 int i;
 for (i = 0; i < expr->v.func_call.arg_count; i++) {
 ast_free_expr(expr->v.func_call.args[i]);
 }
 }
 break;
 default:
 break;
 }
 free(expr);
}

void ast_free_stmt(AstStmt *stmt)
{
 if (!stmt) return;

 switch (stmt->type) {
 case STMT_PRINT:
 {
 int i;
 for (i = 0; i < stmt->v.print.item_count; i++) {
 ast_free_expr(stmt->v.print.items[i].expr);
 }
 free(stmt->v.print.items);
 }
 break;
 case STMT_LET:
 ast_free_expr(stmt->v.let.value);
 break;
 case STMT_LET_STRVAR:
 ast_free_expr(stmt->v.let_strvar.value);
 break;
 case STMT_LET_ARRAY_AT:
 ast_free_expr(stmt->v.let_array_at.index);
 ast_free_expr(stmt->v.let_array_at.value);
 break;
 case STMT_LET_DIM:
 ast_free_expr(stmt->v.let_dim.idx1);
 ast_free_expr(stmt->v.let_dim.idx2);
 ast_free_expr(stmt->v.let_dim.value);
 break;
 case STMT_IF:
 ast_free_expr(stmt->v.if_stmt.condition);
 ast_free_stmt(stmt->v.if_stmt.then_stmt);
 break;
 case STMT_GOTO:
 ast_free_expr(stmt->v.goto_stmt.target);
 break;
 case STMT_GOSUB:
 ast_free_expr(stmt->v.gosub.target);
 break;
 case STMT_FOR:
 ast_free_expr(stmt->v.for_stmt.init);
 ast_free_expr(stmt->v.for_stmt.limit);
 ast_free_expr(stmt->v.for_stmt.step);
 break;
 case STMT_INPUT:
 ast_free_expr(stmt->v.input.prompt);
 break;
 case STMT_DIM:
 ast_free_expr(stmt->v.dim.dim1);
 ast_free_expr(stmt->v.dim.dim2);
 break;
 case STMT_WHILE:
 ast_free_expr(stmt->v.while_stmt.left);
 ast_free_expr(stmt->v.while_stmt.right);
 break;
 case STMT_ON_GOTO:
 ast_free_expr(stmt->v.on_goto.selector);
 break;
 case STMT_DEF_FN:
 ast_free_expr(stmt->v.def_fn.body);
 break;
 case STMT_READ:
 {
 int ri;
 for (ri = 0; ri < stmt->v.read.var_count; ri++) {
 if (stmt->v.read.var_indices[ri])
 ast_free_expr(stmt->v.read.var_indices[ri]);
 }
 }
 break;
 case STMT_INT:
 ast_free_expr(stmt->v.int_stmt.interrupt_number);
 break;
 case STMT_DIRECT_EXEC:
 if (stmt->v.direct_exec.text) {
     free(stmt->v.direct_exec.text);
 }
 break;
 default:
 break;
 }
 free(stmt);
}

void ast_free_line(AstStmt *stmts)
{
 while (stmts) {
 AstStmt *next = stmts->next;
 ast_free_stmt(stmts);
 stmts = next;
 }
}

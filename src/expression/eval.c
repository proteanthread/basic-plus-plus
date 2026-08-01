/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval.c
 * @brief Iterative Expression Evaluator implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements mathematical, string, and relational expression evaluation
 *   using the iterative Shunting-Yard algorithm. Consumes tokens until expression termination.
 * - Why it exists: Avoids stack overflow crashes during evaluation of complex nested expressions,
 *   supporting standard BASIC operator precedence.
 * - Why it works this way: It maintains value and operator stacks in the scratch arena.
 *   Unary operators are identified contextually and transformed to internal types. Relational
 *   comparisons return standard BASIC truth values (-1.0 for true, 0.0 for false).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Precedence rankings, support for new operators, string comparison behaviors.
 * - What cannot be changed: Memory allocation points (must remain scratch arena-bound) and C-stack independence.
 * - What to expect: Evaluating strings will reference-count results. Concatenation allocates new strings.
 * - What to do if something breaks: Check operator pop loops and trace value type transitions.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relational operations return double values (-1.0 or 0.0). String comparison uses strcmp.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add MOD (modulo), ^ (power), or logical XOR operators.
 * - How to write external extensions: External functions are parsed as identifiers followed by parentheses,
 *   routed to the domain registry.
 */

#include "bpp_eval.h"
#include "bpp_file.h"
#include "bpp_task.h"
#include "bpp_vdev.h"
#include "bpp_vfs.h"
#include "bpp_metadata.h"
#include "bpp_funcreg.h"
#include "bpp_eval.h"
#include "bpp_strings.h"
#ifndef BPP_LITE_BUILD
#include "bpp_segmented_mem.h"
#endif
#include "bpp_module.h"
#include "bpp_security.h"
#include "bpp_vnet.h"
#include "bpp_bus.h"
#include "bpp_dialect.h"
#include "bpp_struct.h"
#include "bpp_fujinet.h"
#include "bpp_platform.h"
#include "bpp_vcon.h"

void bpp_hash_string(const char *algo, const char *data, char *out_buf, size_t out_size);
#include <string.h>
#include <ctype.h>

static BppDirSearch *g_dir_search = NULL;
#include <math.h>
#include <time.h>
#include <stdlib.h>

extern bool find_procedure(struct VMContext *vm, const char *name, BppKeywordId proc_kw, double *out_line, const char **out_text);

extern int platform_inkey_char(void);
extern int platform_mouse_x(void);
extern int platform_mouse_y(void);
extern int platform_mouse_btn(void);
struct tm; /* Forward declaration */
extern struct tm *platform_localtime(const time_t *timep, struct tm *result);

extern double vm_get_last_rnd(VMContext *vm);
extern void vm_set_last_rnd(VMContext *vm, double val);
extern double platform_get_timer(void);
extern double platform_get_uptime(void);
extern double vm_get_ti_offset(VMContext *vm);
extern void vm_set_ti_offset(VMContext *vm, double val);

/**
 * @brief Parse and evaluate a Sinclair/Atari string slicing construct.
 *
 * - What it does: Extracts a substring slice from a scalar string variable. It parses the bounds
 *   specifiers (1-based indices), clipping them dynamically to the string's actual length.
 *   Supports both parentheses A$(start TO end) and square brackets A$[start TO end] (Sinclair)
 *   as well as A$[start, end] (Atari / HP TSB).
 * - Why it exists: Provides language-level compatibility for historical string slicing dialects
 *   without introducing keyword collisions in the unified namespace.
 * - Why it works this way: It parses the next token, evaluating the start and optional end
 *   expressions. It clips index bounds gracefully to emulate historical systems without throwing out of bounds.
 * - Assumptions: var_name references a scalar string. Out-of-bounds start/end values are clipped.
 * - Portability concerns: Native string operations rely on thread-safe BppStringRef reference counting.
 * - Future expansions: Supporting slice assignment is handled inside stmt_let.c.
 * - External extension hooks: None.
 */
static BValue parse_string_slice(VMContext *vm, LexerContext *lex, const char *var_name, BppTokenType open_tok, BppError *out_err) {
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;

    /* Consume opening token '(' or '[' */
    lex_next(lex);

    /* Look up the scalar variable value */
    BValue *var_val = var_lookup(vm_get_var(vm), var_name, false);
    const char *str_val = "";
    size_t orig_len = 0;
    if (var_val && var_val->type == VAL_STRING && var_val->as.string) {
        str_val = str_data(var_val->as.string);
        orig_len = str_len(var_val->as.string);
    }

    int start = 1;
    int end = (int)orig_len;

    BppTokenType close_tok = (open_tok == TOK_LPAREN) ? TOK_RPAREN : TOK_RBRACKET;

    /* Parse bounds */
    BppToken next = lex_peek(lex);
    if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
        /* Case: (TO end) or [TO end] */
        lex_next(lex); /* Consume 'TO' */
        if (lex_peek(lex).type != close_tok) {
            BValue end_val = eval_expression(vm, lex, out_err);
            if (out_err->code != 0) return res;
            if (end_val.type == VAL_STRING) {
                out_err->code = 13; out_err->message = "String slice bound must be numeric";
                return res;
            }
            end = (int)end_val.as.number;
        }
    } else if (next.type == close_tok) {
        /* Case: () or [] -> full string */
        /* Nothing to do, defaults are 1 and str_len */
    } else {
        /* Parse first bound */
        BValue start_val = eval_expression(vm, lex, out_err);
        if (out_err->code != 0) return res;
        if (start_val.type == VAL_STRING) {
            out_err->code = 13; out_err->message = "String slice bound must be numeric";
            return res;
        }
        start = (int)start_val.as.number;

        next = lex_peek(lex);
        if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
            lex_next(lex); /* Consume 'TO' */
            if (lex_peek(lex).type != close_tok) {
                BValue end_val = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) return res;
                if (end_val.type == VAL_STRING) {
                    out_err->code = 13; out_err->message = "String slice bound must be numeric";
                    return res;
                }
                end = (int)end_val.as.number;
            }
        } else if (next.type == TOK_COMMA) {
            /* Atari style A$[start, end] */
            lex_next(lex); /* Consume ',' */
            BValue end_val = eval_expression(vm, lex, out_err);
            if (out_err->code != 0) return res;
            if (end_val.type == VAL_STRING) {
                out_err->code = 13; out_err->message = "String slice bound must be numeric";
                return res;
            }
            end = (int)end_val.as.number;
        } else {
            /* Single index: end = start */
            end = start;
        }
    }

    /* Consume closing token */
    if (lex_peek(lex).type != close_tok) {
        out_err->code = 2; out_err->message = (open_tok == TOK_LPAREN) ? "Expected ')' in slice" : "Expected ']' in slice";
        return res;
    }
    lex_next(lex);

    /* Clip bounds (1-based, inclusive) */
    if (start < 1) start = 1;
    if (end > (int)orig_len) end = (int)orig_len;

    if (start > (int)orig_len || end < start) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    } else {
        size_t slice_len = (size_t)(end - start + 1);
        res.as.string = str_create(vm_get_str(vm), str_val + (start - 1), slice_len);
    }

    return res;
}

static void split_member_chain(const char *start, size_t len, char *var_name, size_t var_name_max, char member_chain[8][64], int *member_count) {
    *member_count = 0;
    
    /* Find first dot */
    size_t dot_idx = 0;
    while (dot_idx < len && start[dot_idx] != '.') {
        dot_idx++;
    }
    
    if (dot_idx == len) {
        /* No dot: single variable name */
        size_t clen = (len < var_name_max - 1) ? len : var_name_max - 1;
        memcpy(var_name, start, clen);
        var_name[clen] = '\0';
        return;
    }
    
    /* Copy var_name */
    size_t clen = (dot_idx < var_name_max - 1) ? dot_idx : var_name_max - 1;
    memcpy(var_name, start, clen);
    var_name[clen] = '\0';
    
    /* Parse members */
    size_t i = dot_idx + 1;
    while (i < len && *member_count < 8) {
        size_t next_dot = i;
        while (next_dot < len && start[next_dot] != '.') {
            next_dot++;
        }
        
        size_t mlen = next_dot - i;
        size_t copy_mlen = (mlen < 63) ? mlen : 63;
        memcpy(member_chain[*member_count], start + i, copy_mlen);
        member_chain[*member_count][copy_mlen] = '\0';
        (*member_count)++;
        
        i = next_dot + 1;
    }
}

#define MAX_EVAL_DEPTH 128

/* Forward declarations for built-in functions */
static bool is_builtin_function(const char *name);
static BValue eval_builtin_function(VMContext *vm, const char *name, LexerContext *lex, bool has_parens, BppError *err);
static BValue eval_builtin_function_impl(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);
BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err);

/* Operator precedence lookup */
static int get_precedence(BppTokenType type) {
    switch (type) {
        case TOK_OR:
        case TOK_XOR:
            return 1; /* Logical OR, XOR */
        case TOK_AND:
            return 2; /* Logical AND */
        case TOK_NOT:
            return 3; /* Logical NOT */
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_GT:
        case TOK_LE:
        case TOK_GE:
            return 4; /* Relational */
        case TOK_PLUS:
        case TOK_MINUS:
            return 5; /* Additive */
        case TOK_MUL:
        case TOK_DIV:
            return 6; /* Multiplicative */
        case TOK_UNARY_MINUS:
        case TOK_UNARY_PLUS:
            return 7; /* Unary (highest) */
        default:
            return 0; /* Parentheses / Symbols */
    }
}

static bool has_precedence(VMContext *vm, BppTokenType top, BppTokenType op) {
    BppDialect *d = vm_get_active_dialect(vm);
    if (d && d->math_precedence == PRECEDENCE_LEFT_TO_RIGHT) {
        int prec_top = get_precedence(top);
        int prec_op = get_precedence(op);
        if (prec_top >= 4 || prec_op >= 4) {
            return prec_top >= prec_op;
        }
        return true;
    }
    return get_precedence(top) >= get_precedence(op);
}

/* Check if token type is an operator */
static bool is_operator(BppTokenType type) {
    return (type == TOK_PLUS || type == TOK_MINUS || type == TOK_MUL || type == TOK_DIV ||
            type == TOK_EQ || type == TOK_NE || type == TOK_LT || type == TOK_GT ||
            type == TOK_LE || type == TOK_GE || type == TOK_UNARY_MINUS || type == TOK_UNARY_PLUS ||
            type == TOK_AND || type == TOK_OR || type == TOK_NOT || type == TOK_XOR);
}

/* Execute a single binary or unary operator */
static double round_to_decimal(double val, int precision) {
    if (val == 0.0 || !isfinite(val)) return val;
    double factor = pow(10.0, precision - ceil(log10(fabs(val))));
    return round(val * factor) / factor;
}

static bool execute_op(VMContext *vm, BppTokenType op, BValue *val_stack, size_t *val_ptr, BppError *err) {
    StringContext *str = vm_get_str(vm);

    if (op == TOK_UNARY_MINUS || op == TOK_UNARY_PLUS || op == TOK_NOT) {
        if (*val_ptr < 1) {
            err->code = 2; /* Syntax error */
            err->message = "Missing operand for unary operator";
            return false;
        }
        BValue *val = &val_stack[*val_ptr - 1];
        if (val->type == VAL_STRING) {
            err->code = 13; /* Type mismatch */
            err->message = "Unary operators not supported on strings";
            return false;
        }
        if (op == TOK_UNARY_MINUS) {
            val->as.number = -val->as.number;
        } else if (op == TOK_NOT) {
            val->as.number = (double)(~(int)val->as.number);
        }
        return true;
    }

    /* Binary operators */
    if (*val_ptr < 2) {
        err->code = 2; /* Syntax error */
        err->message = "Missing operand for binary operator";
        return false;
    }

    BValue rhs = val_stack[--(*val_ptr)];
    BValue lhs = val_stack[*val_ptr - 1];
    BValue *res = &val_stack[*val_ptr - 1];

    /* String operators */
    if (lhs.type == VAL_STRING || rhs.type == VAL_STRING) {
        if (lhs.type != VAL_STRING || rhs.type != VAL_STRING) {
            err->code = 13; /* Type mismatch */
            err->message = "Mixed string and numeric arguments";
            return false;
        }

        if (op == TOK_PLUS) {
            /* String Concatenation */
            BppStringRef concat = str_concat(str, lhs.as.string, rhs.as.string);
            if (!concat) {
                err->code = 14; /* Out of string space */
                err->message = "String heap limit reached in concatenation";
                return false;
            }
            res->type = VAL_STRING;
            res->as.string = concat;
            /* Release temp inputs if needed */
            if (lhs.as.string) str_release(str, lhs.as.string);
            if (rhs.as.string) str_release(str, rhs.as.string);
            return true;
        }

        /* Relational string comparisons */
        const char *s1 = str_data(lhs.as.string);
        const char *s2 = str_data(rhs.as.string);
        int cmp = strcmp(s1, s2);
        double bool_res = 0.0; /* 0.0 is False, -1.0 is True */

        switch (op) {
            case TOK_EQ: bool_res = (cmp == 0) ? -1.0 : 0.0; break;
            case TOK_NE: bool_res = (cmp != 0) ? -1.0 : 0.0; break;
            case TOK_LT: bool_res = (cmp < 0)  ? -1.0 : 0.0; break;
            case TOK_GT: bool_res = (cmp > 0)  ? -1.0 : 0.0; break;
            case TOK_LE: bool_res = (cmp <= 0) ? -1.0 : 0.0; break;
            case TOK_GE: bool_res = (cmp >= 0) ? -1.0 : 0.0; break;
            default:
                err->code = 13; /* Type mismatch */
                err->message = "Invalid operator for strings";
                return false;
        }

        /* Release strings */
        if (lhs.as.string) str_release(str, lhs.as.string);
        if (rhs.as.string) str_release(str, rhs.as.string);

        res->type = VAL_NUMBER;
        res->as.number = bool_res;
        return true;
    }

    /* Numeric operations */
    double n1 = lhs.as.number;
    double n2 = rhs.as.number;
    double ans = 0.0;

    switch (op) {
        case TOK_PLUS:  ans = n1 + n2; break;
        case TOK_MINUS: ans = n1 - n2; break;
        case TOK_MUL:   ans = n1 * n2; break;
        case TOK_DIV:
            if (n2 == 0.0) {
                err->code = 11; /* Division by zero */
                err->message = "Division by zero";
                return false;
            }
            ans = n1 / n2;
            break;
        case TOK_EQ: ans = n1 == n2 ? -1.0 : 0.0; break;
        case TOK_NE: ans = n1 != n2 ? -1.0 : 0.0; break;
        case TOK_LT: ans = n1 < n2  ? -1.0 : 0.0; break;
        case TOK_GT: ans = n1 > n2  ? -1.0 : 0.0; break;
        case TOK_LE: ans = n1 <= n2 ? -1.0 : 0.0; break;
        case TOK_GE: ans = n1 >= n2 ? -1.0 : 0.0; break;
        case TOK_AND: ans = (double)((int)n1 & (int)n2); break;
        case TOK_OR:  ans = (double)((int)n1 | (int)n2); break;
        case TOK_XOR: ans = (double)((int)n1 ^ (int)n2); break;
        default:
            err->code = 2; /* Syntax error */
            err->message = "Invalid numeric operator";
            return false;
    }

    if (vm_get_arithmetic_decimal(vm) && (op == TOK_PLUS || op == TOK_MINUS || op == TOK_MUL || op == TOK_DIV)) {
        ans = round_to_decimal(ans, 12);
    }

    res->type = VAL_NUMBER;
    res->as.number = ans;
    return true;
}

static BValue resolve_member_access(VMContext *vm, LexerContext *lex, BValue val, BppError *out_err) {
    BValue null_val;
    memset(&null_val, 0, sizeof(null_val));

    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex); /* Consume '.' */
        BppToken field_tok = lex_next(lex);
        if (field_tok.type != TOK_IDENT) {
            out_err->code = 2; out_err->message = "Expected member identifier after '.'";
            return null_val;
        }
        char field_name[256];
        size_t flen = (field_tok.length < sizeof(field_name) - 1) ? field_tok.length : sizeof(field_name) - 1;
        memcpy(field_name, field_tok.start, flen);
        field_name[flen] = '\0';

        /* Check if followed by '(' -> Class method call */
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            
            if (val.type != VAL_MAP || !val.as.map) {
                out_err->code = 13; out_err->message = "Method call on non-object value";
                return null_val;
            }
            BValue type_val;
            if (!bpp_map_get(val.as.map, "__type__", &type_val) || type_val.type != VAL_STRING) {
                out_err->code = 13; out_err->message = "Object missing class type metadata";
                return null_val;
            }
            char fully_qualified_method[512];
            snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                     str_data(type_val.as.string), field_name);

            /* Parse arguments */
            BValue args[9];
            int argc = 0;
            
            /* Implicit THIS */
            args[argc++] = val;
            bpp_map_add_ref(val.as.map);

            while (true) {
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                }
                if (argc >= 9) {
                    out_err->code = 2; out_err->message = "Too many arguments in method call";
                    for (int i = 0; i < argc; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return null_val;
                }
                args[argc++] = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) {
                    for (int i = 0; i < argc - 1; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return null_val;
                }
                next_tok = lex_peek(lex);
                if (next_tok.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (next_tok.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    out_err->code = 2; out_err->message = "Expected ',' or ')' in method call";
                    for (int i = 0; i < argc; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return null_val;
                }
            }

            /* Invoke method */
            BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
            for (int i = 0; i < argc; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
            }
            if (out_err->code != 0) return null_val;
            
            val = ret_val;
        } else {
            /* Standard field lookup */
            if (val.type != VAL_MAP || !val.as.map) {
                out_err->code = 13; out_err->message = "Member access on non-object value";
                return null_val;
            }
            BValue field_val;
            if (!bpp_map_get(val.as.map, field_name, &field_val)) {
                out_err->code = 35; out_err->message = "Member field not defined in UDT/Class";
                return null_val;
            }
            BValue old_val = val;
            val = field_val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            } else if (val.type == VAL_MAP && val.as.map) {
                bpp_map_add_ref(val.as.map);
            }
            if (old_val.type == VAL_MAP && old_val.as.map) {
                bpp_map_release(vm_get_str(vm), old_val.as.map);
            }
        }
    }
    return val;
}

BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    memset(&null_val, 0, sizeof(null_val));

    /* Guard against C-stack overflow from deeply nested expression evaluation.
     * eval_expression calls itself recursively for function arguments, array
     * subscripts, and string slices. Without a guard, pathological input like
     * deeply nested function calls could overflow the host C stack. */
    vm_inc_eval_depth(vm);
    if (vm_get_eval_depth(vm) > 64) {
        vm_dec_eval_depth(vm);
        out_err->code = 14;
        out_err->message = "Expression nesting too deep (limit 64)";
        return null_val;
    }

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* Allocate Shunting-Yard stacks from scratch arena */
    BValue *val_stack = (BValue *)mem_scratch_alloc(mem, sizeof(BValue) * MAX_EVAL_DEPTH);
    BppTokenType *op_stack = (BppTokenType *)mem_scratch_alloc(mem, sizeof(BppTokenType) * MAX_EVAL_DEPTH);

    if (!val_stack || !op_stack) {
        out_err->code = 14; /* Out of memory */
        out_err->message = "Evaluation stack overflow (scratch exhausted)";
        return null_val;
    }

    size_t val_ptr = 0;
    size_t op_ptr = 0;
    int open_parens = 0;

    bool expect_operand = true;
    BppToken tok = lex_peek(lex);

    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA &&
           tok.type != TOK_SEMICOLON && (tok.type != TOK_RPAREN || open_parens > 0) &&
           tok.type != TOK_RBRACKET &&
           (tok.type != TOK_KEYWORD || tok.as.keyword == KW_NONE ||
            tok.as.keyword == KW_TASK || tok.as.keyword == KW_PLAY || tok.as.keyword == KW_HELP ||
            tok.as.keyword == KW_SCREEN || tok.as.keyword == KW_SEEK ||
            tok.as.keyword == KW_TIMER || tok.as.keyword == KW_KEY ||
            tok.as.keyword == KW_REMOVE || tok.as.keyword == KW_REMOVE_STR ||
            tok.as.keyword == KW_ALARM || tok.as.keyword == KW_ALARM_STR ||
            tok.as.keyword == KW_RANDOMIZE)) {

        /* Stop parsing if we see 'AT' identifier */
        if (tok.type == TOK_IDENT && tok.length == 2 &&
            (tok.start[0] == 'A' || tok.start[0] == 'a') &&
            (tok.start[1] == 'T' || tok.start[1] == 't')) {
            break;
        }

        /* Implied semicolon check: if we are expecting an operator, but see an operand */
        if (!expect_operand) {
            if (tok.type == TOK_NUMBER || tok.type == TOK_STRING || tok.type == TOK_RPN_LITERAL ||
                tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_LPAREN) {
                break;
            }
        }

        /* Read the peeked token */
        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got number";
                return null_val;
            }
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            val_stack[val_ptr++] = val;
            expect_operand = false;
        } else if (tok.type == TOK_STRING) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got string";
                return null_val;
            }
            /* Create string handle */
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            val_stack[val_ptr++] = val;
            expect_operand = false;
        } else if (tok.type == TOK_RPN_LITERAL) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got RPN literal";
                return null_val;
            }
            char *rpn_str = (char *)mem_scratch_alloc(vm_get_mem(vm), tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14;
                out_err->message = "Scratch memory exhausted";
                return null_val;
            }
            memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(vm_get_mem(vm), rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) return null_val;

            val_stack[val_ptr++] = res;
            expect_operand = false;
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_PERIOD) {
            if (!expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operator, got variable, keyword or '.'";
                return null_val;
            }
            /* Variable or function lookup */
            char name_buf[256];
            if (tok.type == TOK_PERIOD) {
                const char *with_prefix = vm_with_stack_peek(vm);
                if (!with_prefix) {
                    out_err->code = 2; out_err->message = "Leading '.' outside of WITH block";
                    return null_val;
                }
                BppToken sub_tok = lex_next(lex);
                if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
                    out_err->code = 2; out_err->message = "Expected identifier after '.' in WITH member access";
                    return null_val;
                }
                snprintf(name_buf, sizeof(name_buf), "%s.%.*s", with_prefix, (int)sub_tok.length, sub_tok.start);
            } else {
                size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
                memcpy(name_buf, tok.start, copy_len);
                name_buf[copy_len] = '\0';
            }

            /* Check if namespace prefix (e.g. bits.xxx or math.xxx) */
            if (lex_peek(lex).type == TOK_PERIOD &&
                (strcasecmp(name_buf, "bits") == 0 || strcasecmp(name_buf, "math") == 0 ||
                 strcasecmp(name_buf, "sound") == 0 || strcasecmp(name_buf, "music") == 0 ||
                 strcasecmp(name_buf, "mouse") == 0 || strcasecmp(name_buf, "joystick") == 0 ||
                 strcasecmp(name_buf, "input") == 0 || strcasecmp(name_buf, "window") == 0)) {
                lex_next(lex); /* Consume '.' */
                BppToken sub_tok = lex_next(lex);
                if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) {
                    out_err->code = 2; out_err->message = "Expected identifier after '.' in namespace call";
                    return null_val;
                }
                char sub_name[128];
                size_t sub_len = (sub_tok.length < sizeof(sub_name) - 1) ? sub_tok.length : sizeof(sub_name) - 1;
                memcpy(sub_name, sub_tok.start, sub_len);
                sub_name[sub_len] = '\0';
                
                char combined[384];
                snprintf(combined, sizeof(combined), "%s.%s", name_buf, sub_name);
                strncpy(name_buf, combined, sizeof(name_buf) - 1);
                name_buf[sizeof(name_buf) - 1] = '\0';
            }

            /* If name_buf refers to a VAL_ARRAY_REF (e.g., parameter passed by reference), resolve it to the original array name */
            BValue *ref_var = var_lookup(var, name_buf, false);
            if (ref_var && ref_var->type == VAL_ARRAY_REF && ref_var->as.string) {
                const char *orig_name = str_data(ref_var->as.string);
                size_t olen = strlen(orig_name);
                if (olen < sizeof(name_buf) - 1) {
                    memcpy(name_buf, orig_name, olen);
                    name_buf[olen] = '\0';
                }
            }

            bool is_func = false;
            /* Check if followed by '(' or '[' */
            BppTokenType next_tok_type = lex_peek(lex).type;
            if (next_tok_type == TOK_LBRACKET) {
                is_func = true;
                BValue val = parse_string_slice(vm, lex, name_buf, TOK_LBRACKET, out_err);
                if (out_err->code != 0) return null_val;
                val_stack[val_ptr++] = val;
                expect_operand = false;
            } else if (next_tok_type == TOK_LPAREN) {
                /* Disambiguate Sinclair string slicing A$(start TO end) from function calls / array accesses */
                bool is_slicing = false;
                if (name_buf[strlen(name_buf) - 1] == '$' && strchr(name_buf, '.') == NULL) {
                    if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL) &&
                        !is_builtin_function(name_buf) &&
                        !arr_exists(vm_get_arr(vm), name_buf)) {
                        is_slicing = true;
                    }
                }
                if (is_slicing) {
                    is_func = true;
                    BValue val = parse_string_slice(vm, lex, name_buf, TOK_LPAREN, out_err);
                    if (out_err->code != 0) return null_val;
                    val_stack[val_ptr++] = val;
                    expect_operand = false;
                } else if (is_builtin_function(name_buf)) {
                    is_func = true;
                    lex_next(lex); /* Consume '(' */
                    BValue val = eval_builtin_function(vm, name_buf, lex, true, out_err);
                    if (out_err->code != 0) return null_val;
                    val_stack[val_ptr++] = val;
                    expect_operand = false;
                } else if (!arr_exists(vm_get_arr(vm), name_buf) &&
                           !(var_lookup(var, name_buf, false) && var_lookup(var, name_buf, false)->type == VAL_ARRAY_REF && var_lookup(var, name_buf, false)->as.string)) {
                    is_func = true;
                    lex_next(lex); /* Consume '(' */

                    /* Check if name_buf contains '.' -> Method call check */
                    bool is_method = false;
                    char base_name[256] = "";
                    char member_chain[8][64];
                    int member_count = 0;
                    char fully_qualified_method[512] = "";
                    BValue obj_val;
                    memset(&obj_val, 0, sizeof(obj_val));

                    if (strchr(name_buf, '.') != NULL) {
                        if (!find_procedure((struct VMContext *)vm, name_buf, KW_FUNCTION, NULL, NULL)) {
                            /* Not a global namespaced function: try to resolve as method call */
                            split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                            if (member_count > 0) {
                                BValue *var_val = var_lookup(var, base_name, false);
                                if (var_val) {
                                    obj_val = *var_val;
                                    if (obj_val.type == VAL_STRING && obj_val.as.string) str_add_ref(obj_val.as.string);
                                    else if (obj_val.type == VAL_MAP && obj_val.as.map) bpp_map_add_ref(obj_val.as.map);
                                    
                                    /* Walk nested fields up to last member */
                                    bool walk_err = false;
                                    for (int m = 0; m < member_count - 1; m++) {
                                        if (obj_val.type != VAL_MAP || !obj_val.as.map) {
                                            walk_err = true; break;
                                        }
                                        BValue next_val;
                                        if (!bpp_map_get(obj_val.as.map, member_chain[m], &next_val)) {
                                            walk_err = true; break;
                                        }
                                        BValue copy = next_val;
                                        if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                        else if (copy.type == VAL_MAP && copy.as.map) bpp_map_add_ref(copy.as.map);
                                        
                                        if (obj_val.type == VAL_MAP && obj_val.as.map) bpp_map_release(vm_get_str(vm), obj_val.as.map);
                                        else if (obj_val.type == VAL_STRING && obj_val.as.string) str_release(vm_get_str(vm), obj_val.as.string);
                                        obj_val = copy;
                                    }
                                    
                                    if (!walk_err && obj_val.type == VAL_MAP && obj_val.as.map) {
                                        BValue type_val;
                                        if (bpp_map_get(obj_val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING) {
                                            snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                     str_data(type_val.as.string), member_chain[member_count - 1]);
                                            is_method = true;
                                        }
                                    }
                                    if (!is_method) {
                                        if (obj_val.type == VAL_MAP && obj_val.as.map) bpp_map_release(vm_get_str(vm), obj_val.as.map);
                                        else if (obj_val.type == VAL_STRING && obj_val.as.string) str_release(vm_get_str(vm), obj_val.as.string);
                                    }
                                }
                            }
                        }
                    }

                    BValue args[9];
                    int argc = 0;
                    if (is_method) {
                        args[argc++] = obj_val; /* Implicit THIS */
                    }

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
                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return null_val;
                        }

                        args[argc++] = eval_expression(vm, lex, out_err);
                        if (out_err->code != 0) {
                            for (int i = 0; i < argc - 1; i++) {
                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return null_val;
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
                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                            }
                            return null_val;
                        }
                    }

                    BValue val = invoke_user_function(vm, is_method ? fully_qualified_method : name_buf, args, argc, out_err);
                    for (int i = 0; i < argc; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                    }
                    if (out_err->code != 0) return null_val;

                    val_stack[val_ptr++] = val;
                    expect_operand = false;
                } else {
                    /* Array access! E.g. A(1, 2) or A(1 TO 5, *) */
                    if (!arr_exists(vm_get_arr(vm), name_buf)) {
                        BValue *var_val = var_lookup(var, name_buf, false);
                        if (var_val && var_val->type == VAL_ARRAY_REF && var_val->as.string) {
                            strncpy(name_buf, str_data(var_val->as.string), 256 - 1);
                            name_buf[256 - 1] = '\0';
                        }
                    }
                    lex_next(lex); /* Consume '(' */

                    typedef struct { int is_slice; int start; int end; } SliceDim;
                    SliceDim slices[4];
                    int num_indices = 0;
                    bool has_slice = false;

                    if (lex_peek(lex).type == TOK_RPAREN) {
                        lex_next(lex); /* Consume ')' */
                        BValue val;
                        val.type = VAL_ARRAY_REF;
                        val.as.string = str_create(vm_get_str(vm), name_buf, strlen(name_buf));
                        val_stack[val_ptr++] = val;
                        expect_operand = false;
                        is_func = true;
                    } else {
                        while (true) {
                            if (num_indices >= 4) {
                                out_err->code = 9; out_err->message = "Too many dimensions for array access";
                                return null_val;
                            }
                            
                            if (lex_peek(lex).type == TOK_MUL) {
                                lex_next(lex);
                                slices[num_indices].is_slice = 1;
                                slices[num_indices].start = arr_get_option_base(vm_get_arr(vm));
                                bool found = false;
                                slices[num_indices].end = arr_ubound(vm_get_arr(vm), name_buf, num_indices + 1, &found);
                                if (!found) {
                                    out_err->code = 9; out_err->message = "Array dimension not found";
                                    return null_val;
                                }
                                has_slice = true;
                            } else {
                                BValue idx_val = eval_expression(vm, lex, out_err);
                                if (out_err->code != 0) return null_val;
                                if (idx_val.type == VAL_STRING) {
                                    out_err->code = 13; out_err->message = "String values are not allowed as array indices";
                                    return null_val;
                                }
                                
                                if (lex_peek(lex).type == TOK_KEYWORD && lex_peek(lex).as.keyword == KW_TO) {
                                    lex_next(lex);
                                    BValue end_val = eval_expression(vm, lex, out_err);
                                    if (out_err->code != 0) return null_val;
                                    slices[num_indices].is_slice = 1;
                                    slices[num_indices].start = (int)idx_val.as.number;
                                    slices[num_indices].end = (int)end_val.as.number;
                                    has_slice = true;
                                } else {
                                    slices[num_indices].is_slice = 0;
                                    slices[num_indices].start = (int)idx_val.as.number;
                                    slices[num_indices].end = (int)idx_val.as.number;
                                }
                            }
                            num_indices++;

                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex); /* Consume ',' */
                            } else if (next_tok.type == TOK_RPAREN) {
                                break;
                            } else {
                                out_err->code = 2; out_err->message = "Expected ',' or ')' in array index list";
                                return null_val;
                            }
                        }
                        lex_next(lex); /* Consume ')' */

                        if (has_slice) {
                            static int slice_counter = 0;
                            char tmp_name[64];
                            snprintf(tmp_name, sizeof(tmp_name), "__slice_%d", ++slice_counter);
                            
                            int base = arr_get_option_base(vm_get_arr(vm));
                            int new_bounds[4] = {0};
                            
                            /* Collect bounds for dimensions that are actually slices, to reduce dimensionality if possible */
                            for (int i = 0; i < num_indices; i++) {
                                new_bounds[i] = (slices[i].end - slices[i].start) + base;
                            }
                            
                            BppError dim_err = arr_dim(vm_get_arr(vm), tmp_name, num_indices, new_bounds);
                            if (dim_err.code != 0) {
                                *out_err = dim_err; return null_val;
                            }
                            
                            /* Copy elements */
                            int src_idx[4] = {0};
                            int dst_idx[4] = {0};
                            for(int d0 = slices[0].start, t0 = base; d0 <= slices[0].end; d0++, t0++) {
                                int max_d1 = (num_indices > 1) ? slices[1].end : 0;
                                for(int d1 = (num_indices > 1) ? slices[1].start : 0, t1 = base; d1 <= max_d1; d1++, t1++) {
                                    int max_d2 = (num_indices > 2) ? slices[2].end : 0;
                                    for(int d2 = (num_indices > 2) ? slices[2].start : 0, t2 = base; d2 <= max_d2; d2++, t2++) {
                                        int max_d3 = (num_indices > 3) ? slices[3].end : 0;
                                        for(int d3 = (num_indices > 3) ? slices[3].start : 0, t3 = base; d3 <= max_d3; d3++, t3++) {
                                            src_idx[0] = d0; src_idx[1] = d1; src_idx[2] = d2; src_idx[3] = d3;
                                            dst_idx[0] = t0; dst_idx[1] = t1; dst_idx[2] = t2; dst_idx[3] = t3;
                                            
                                            BValue *src_elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, src_idx, out_err);
                                            if (out_err->code == 0 && src_elem) {
                                                BValue *dst_elem = arr_get_element(vm_get_arr(vm), tmp_name, num_indices, dst_idx, out_err);
                                                if (out_err->code == 0 && dst_elem) {
                                                    *dst_elem = *src_elem;
                                                    if (dst_elem->type == VAL_STRING && dst_elem->as.string) str_add_ref(dst_elem->as.string);
                                                    else if (dst_elem->type == VAL_MAP && dst_elem->as.map) bpp_map_add_ref(dst_elem->as.map);
                                                }
                                            }
                                            out_err->code = 0; /* Clear out of bounds errors during slice copy if any */
                                        }
                                    }
                                }
                            }
                            
                            BValue val;
                            val.type = VAL_ARRAY_REF;
                            val.as.string = str_create(vm_get_str(vm), tmp_name, strlen(tmp_name));
                            val_stack[val_ptr++] = val;
                            expect_operand = false;
                            is_func = true;
                        } else {
                            /* Lookup single element */
                            int indices[4];
                            for (int i = 0; i < num_indices; i++) indices[i] = slices[i].start;
                            
                            BValue *elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, indices, out_err);
                            if (out_err->code != 0 || !elem) {
                                return null_val;
                            }

                            /* Push copy. If string, add reference! */
                            BValue val = *elem;
                            if (val.type == VAL_STRING && val.as.string) {
                                str_add_ref(val.as.string);
                            } else if (val.type == VAL_MAP && val.as.map) {
                                bpp_map_add_ref(val.as.map);
                            }
                            val = resolve_member_access(vm, lex, val, out_err);
                            if (out_err->code != 0) return null_val;

                            val_stack[val_ptr++] = val;
                            expect_operand = false;
                            is_func = true;
                        }
                    }
                }
            } else if (is_builtin_function(name_buf)) {
                BValue val;
                memset(&val, 0, sizeof(val));
                if (strcmp(name_buf, "RND") == 0) {
                    BppToken next = lex_peek(lex);
                    bool has_arg = false;
                    bool is_negative = false;
                    if (next.type == TOK_MINUS) {
                        LexerContext *temp = lex_init(vm_get_mem(vm), lex_get_pos(lex));
                        if (temp) {
                            lex_set_dialect(temp, vm_get_active_dialect(vm));
                            lex_next(temp); /* consume '-' */
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
                        if (is_negative) {
                            lex_next(lex); /* Consume '-' */
                        }
                        BppToken arg_tok = lex_next(lex); /* Consume the argument token */
                        if (arg_tok.type == TOK_NUMBER) {
                            double num_val = arg_tok.as.number;
                            if (is_negative) num_val = -num_val;
                            int base = 0;
                            if (arg_tok.length > 2 && arg_tok.start[0] == '&') {
                                char b = (char)toupper((unsigned char)arg_tok.start[1]);
                                if (b == 'H') base = 16;
                                else if (b == 'O') base = 8;
                                else if (b == 'B') base = 2;
                                else if (isdigit((unsigned char)arg_tok.start[1])) base = 8;
                            }
                            
                            if (base > 0) {
                                long max_val = (long)num_val;
                                long r_val = 0;
                                if (max_val > 0) {
                                    r_val = rand() % (max_val + 1);
                                }
                                char buf[128] = "";
                                if (base == 16) snprintf(buf, sizeof(buf), "%X", (unsigned int)r_val);
                                else if (base == 8) snprintf(buf, sizeof(buf), "%o", (unsigned int)r_val);
                                else if (base == 2) {
                                    char bin[64] = "";
                                    int idx = 0;
                                    unsigned long tmp = r_val;
                                    if (tmp == 0) {
                                        strcpy(buf, "0");
                                    } else {
                                        while (tmp > 0) {
                                            bin[idx++] = (tmp & 1) ? '1' : '0';
                                            tmp >>= 1;
                                        }
                                        for (int j = 0; j < idx; j++) {
                                            buf[j] = bin[idx - 1 - j];
                                        }
                                        buf[idx] = '\0';
                                    }
                                }
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else {
                                long limit = (long)num_val;
                                long r_val = 0;
                                if (limit > 0) {
                                    r_val = rand() % (limit + 1);
                                } else if (limit < 0) {
                                    r_val = -(rand() % (-limit + 1));
                                }
                                val.type = VAL_NUMBER;
                                val.as.number = (double)r_val;
                            }
                        } else if (arg_tok.type == TOK_IDENT || arg_tok.type == TOK_KEYWORD) {
                            char arg_name[64] = "";
                            size_t alen = (arg_tok.length < 63) ? arg_tok.length : 63;
                            memcpy(arg_name, arg_tok.start, alen);
                            arg_name[alen] = '\0';
                            for (size_t k = 0; k < alen; k++) {
                                arg_name[k] = (char)toupper((unsigned char)arg_name[k]);
                            }
                            
                            if (strcmp(arg_name, "TIME") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else if (strcmp(arg_name, "TI") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                val.type = VAL_NUMBER;
                                val.as.number = h * 10000.0 + m * 100.0 + s;
                            } else if (strcmp(arg_name, "TIMER") == 0) {
                                double r_sec = ((double)rand() / (double)RAND_MAX) * 86400.0;
                                val.type = VAL_NUMBER;
                                val.as.number = r_sec;
                            } else {
                                val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                            }
                        } else {
                            val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                        }
                    } else {
                        val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                    }
                } else if (strcmp(name_buf, "RANDOMIZE") == 0) {
                    BppToken next = lex_peek(lex);
                    if (next.type == TOK_STRING || next.type == TOK_IDENT) {
                        BppToken arg_tok = lex_next(lex);
                        BValue arg_val;
                        memset(&arg_val, 0, sizeof(arg_val));
                        if (arg_tok.type == TOK_STRING) {
                            arg_val.type = VAL_STRING;
                            arg_val.as.string = str_create(vm_get_str(vm), arg_tok.as.string, arg_tok.length);
                        } else {
                            char var_name[256];
                            size_t vlen = (arg_tok.length < 255) ? arg_tok.length : 255;
                            memcpy(var_name, arg_tok.start, vlen);
                            var_name[vlen] = '\0';
                            BValue *lookup = var_lookup(var, var_name, false);
                            if (lookup) {
                                arg_val = *lookup;
                                if (arg_val.type == VAL_STRING && arg_val.as.string) str_add_ref(arg_val.as.string);
                                else if (arg_val.type == VAL_MAP && arg_val.as.map) bpp_map_add_ref(arg_val.as.map);
                            }
                        }

                        const char *mode = NULL;
                        if (arg_val.type == VAL_STRING && arg_val.as.string) {
                            mode = str_data(arg_val.as.string);
                        }
                        if (mode) {
                            if (strcmp(mode, "STRING$") == 0) {
                                int len = 8;
                                char *buf = (char *)calloc(1, len + 1);
                                const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                                for (int i = 0; i < len; i++) {
                                    buf[i] = charset[rand() % (sizeof(charset) - 1)];
                                }
                                buf[len] = '\0';
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, len);
                                free(buf);
                            } else if (strcmp(mode, "DATE$") == 0) {
                                int m = rand() % 12 + 1;
                                int d = rand() % 28 + 1;
                                int y = rand() % 100;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d-%02d-%02d", m, d, y);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else if (strcmp(mode, "DAY$") == 0) {
                                const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
                                const char *day = days[rand() % 7];
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), day, strlen(day));
                            } else if (strcmp(mode, "TIME$") == 0) {
                                int h = rand() % 24;
                                int m = rand() % 60;
                                int s = rand() % 60;
                                char buf[16];
                                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
                            } else {
                                size_t len = strlen(mode);
                                char *buf = (char *)calloc(1, len + 1);
                                strcpy(buf, mode);
                                for (size_t i = len - 1; i > 0; i--) {
                                    size_t j = rand() % (i + 1);
                                    char tmp = buf[i];
                                    buf[i] = buf[j];
                                    buf[j] = tmp;
                                }
                                val.type = VAL_STRING;
                                val.as.string = str_create(vm_get_str(vm), buf, len);
                                free(buf);
                            }
                        } else {
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), "", 0);
                        }

                        if (arg_val.type == VAL_STRING && arg_val.as.string) {
                            str_release(vm_get_str(vm), arg_val.as.string);
                        } else if (arg_val.type == VAL_MAP && arg_val.as.map) {
                            bpp_map_release(vm_get_str(vm), arg_val.as.map);
                        }
                    } else {
                        val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                    }
                } else {
                    val = eval_builtin_function(vm, name_buf, lex, false, out_err);
                }
                if (out_err->code != 0) return null_val;
                val_stack[val_ptr++] = val;
                expect_operand = false;
                is_func = true;
            }

            if (!is_func) {
                if (tok.type == TOK_KEYWORD) {
                    out_err->code = 2;
                    out_err->message = "Unexpected keyword in expression";
                    return null_val;
                }

                BValue temp_val;
                memset(&temp_val, 0, sizeof(temp_val));
                bool is_special = false;
                if (strcmp(name_buf, "ERR") == 0) {
                    temp_val.type = VAL_NUMBER;
                    temp_val.as.number = (double)vm_get_err_code(vm);
                    is_special = true;
                } else if (strcmp(name_buf, "ERL") == 0) {
                    temp_val.type = VAL_NUMBER;
                    temp_val.as.number = (double)vm_get_err_line(vm);
                    is_special = true;
                }

                if (is_special) {
                    val_stack[val_ptr++] = temp_val;
                    expect_operand = false;
                } else {
                    BValue *var_val = var_lookup(var, name_buf, false);
                    if (!var_val) {
                        char base_name[256];
                        char member_chain[8][64];
                        int member_count = 0;
                        split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                        
                        if (member_count > 0) {
                            var_val = var_lookup(var, base_name, false);
                            if (var_val) {
                                BValue val = *var_val;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) bpp_map_add_ref(val.as.map);
                                
                                /* Walk up to the last member */
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Member access on non-object value";
                                        return null_val;
                                    }
                                    BValue next_val;
                                    if (!bpp_map_get(val.as.map, member_chain[m], &next_val)) {
                                        out_err->code = 35; out_err->message = "Member field not found";
                                        if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                        return null_val;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) bpp_map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                /* Check if followed by '(' -> Method call on the resolved object 'val' */
                                if (lex_peek(lex).type == TOK_LPAREN) {
                                    lex_next(lex); /* Consume '(' */
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Method call on non-object value";
                                        return null_val;
                                    }
                                    BValue type_val;
                                    if (!bpp_map_get(val.as.map, "__type__", &type_val) || type_val.type != VAL_STRING) {
                                        out_err->code = 13; out_err->message = "Object missing class type metadata";
                                        return null_val;
                                    }
                                    char fully_qualified_method[512];
                                    snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                             str_data(type_val.as.string), member_chain[member_count - 1]);
                                    
                                    BValue args[9];
                                    int argc = 0;
                                    args[argc++] = val;
                                    bpp_map_add_ref(val.as.map);
                                    
                                    while (true) {
                                        BppToken next_tok = lex_peek(lex);
                                        if (next_tok.type == TOK_RPAREN) {
                                            lex_next(lex);
                                            break;
                                        }
                                        if (argc >= 9) {
                                            out_err->code = 2; out_err->message = "Too many arguments in method call";
                                            for (int i = 0; i < argc; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            return null_val;
                                        }
                                        args[argc++] = eval_expression(vm, lex, out_err);
                                        if (out_err->code != 0) {
                                            for (int i = 0; i < argc - 1; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            return null_val;
                                        }
                                        next_tok = lex_peek(lex);
                                        if (next_tok.type == TOK_COMMA) {
                                            lex_next(lex);
                                        } else if (next_tok.type == TOK_RPAREN) {
                                            lex_next(lex);
                                            break;
                                        } else {
                                            out_err->code = 2; out_err->message = "Expected ',' or ')' in method call";
                                            for (int i = 0; i < argc; i++) {
                                                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                                            }
                                            return null_val;
                                        }
                                    }
                                    
                                    BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                                    for (int i = 0; i < argc; i++) {
                                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                                    }
                                    if (out_err->code != 0) return null_val;
                                    
                                    val_stack[val_ptr++] = ret_val;
                                    expect_operand = false;
                                    tok = lex_peek(lex);
                                    continue;
                                }
                                
                                /* Standard lookup for the last field */
                                {
                                    int m = member_count - 1;
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        out_err->code = 13; out_err->message = "Member access on non-object value";
                                        return null_val;
                                    }
                                    BValue next_val;
                                    if (!bpp_map_get(val.as.map, member_chain[m], &next_val)) {
                                        out_err->code = 35; out_err->message = "Member field not found";
                                        if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                        else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                        return null_val;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) bpp_map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                val_stack[val_ptr++] = val;
                                expect_operand = false;
                                tok = lex_peek(lex);
                                continue;
                            }
                        }
                        
                        var_val = var_lookup(var, name_buf, true);
                        if (!var_val) {
                            out_err->code = 2;
                            out_err->message = "Variable not declared (OPTION EXPLICIT)";
                            return null_val;
                        }
                    }

                    BValue val = *var_val;
                    if (val.type == VAL_STRING && val.as.string) {
                        str_add_ref(val.as.string);
                    } else if (val.type == VAL_MAP && val.as.map) {
                        bpp_map_add_ref(val.as.map);
                    } else if (val.type == VAL_FIELD_STRING) {
                        /* Read from random access file buffer */
                        int ch = val.as.field_str.channel;
                        unsigned char *rec_buf = file_get_record_buffer(vm_get_file(vm), ch);
                        if (rec_buf) {
                            char *buf_slice = (char *)calloc(1, val.as.field_str.length + 1);
                            if (!buf_slice) {
                                out_err->code = 7; out_err->message = "Out of memory";
                                return null_val;
                            }
                            memcpy(buf_slice, rec_buf + val.as.field_str.offset, val.as.field_str.length);
                            buf_slice[val.as.field_str.length] = '\0';
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), buf_slice, val.as.field_str.length);
                            free(buf_slice);
                        } else {
                            val.type = VAL_STRING;
                            val.as.string = str_create(vm_get_str(vm), "", 0);
                        }
                    }
                    val = resolve_member_access(vm, lex, val, out_err);
                    if (out_err->code != 0) return null_val;

                    val_stack[val_ptr++] = val;
                    expect_operand = false;
                }
            }
        } else if (tok.type == TOK_LPAREN) {
            if (!expect_operand) {
                /* JOSS style discrete range or separate block: stop parsing */
                break;
            }
            op_stack[op_ptr++] = TOK_LPAREN;
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            if (expect_operand) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Expected operand before ')'";
                return null_val;
            }
            bool found_paren = false;
            while (op_ptr > 0) {
                BppTokenType top = op_stack[--op_ptr];
                if (top == TOK_LPAREN) {
                    found_paren = true;
                    break;
                }
                if (!execute_op(vm, top, val_stack, &val_ptr, out_err)) {
                    return null_val;
                }
            }
            if (!found_paren) {
                out_err->code = 2; /* Syntax error */
                out_err->message = "Mismatched parentheses";
                return null_val;
            }
            open_parens--;
        } else if (is_operator(tok.type)) {
            BppTokenType op = tok.type;
            if (expect_operand) {
                /* Translate to unary */
                if (op == TOK_MINUS) op = TOK_UNARY_MINUS;
                else if (op == TOK_PLUS) op = TOK_UNARY_PLUS;
                else if (op == TOK_NOT) {
                    /* TOK_NOT is unary, keep it */
                }
                else {
                    out_err->code = 2; /* Syntax error */
                    out_err->message = "Expected operand, got operator";
                    return null_val;
                }
            } else {
                /* Binary operator, but NOT cannot be binary */
                if (op == TOK_NOT) {
                    out_err->code = 2; /* Syntax error */
                    out_err->message = "Unexpected NOT operator";
                    return null_val;
                }
            }

            while (op_ptr > 0) {
                BppTokenType top = op_stack[op_ptr - 1];
                if (top == TOK_LPAREN) break;
                if (has_precedence(vm, top, op)) {
                    op_ptr--;
                    if (!execute_op(vm, top, val_stack, &val_ptr, out_err)) {
                        return null_val;
                    }
                } else {
                    break;
                }
            }
            op_stack[op_ptr++] = op;
            expect_operand = true;
        } else {
            /* Stop parsing expression */
            break;
        }

        tok = lex_peek(lex);
    }

    /* Pop all remaining operators */
    while (op_ptr > 0) {
        BppTokenType top = op_stack[--op_ptr];
        if (top == TOK_LPAREN) {
            out_err->code = 2; /* Syntax error */
            out_err->message = "Mismatched parentheses";
            return null_val;
        }
        if (!execute_op(vm, top, val_stack, &val_ptr, out_err)) {
            return null_val;
        }
    }

    if (val_ptr != 1) {
        out_err->code = 2; /* Syntax error */
        out_err->message = "Invalid expression structure";
        vm_dec_eval_depth(vm);
        return null_val;
    }

    vm_dec_eval_depth(vm);
    return val_stack[0];
}

#include <math.h>

static void format_double_clean(char *buf, size_t buf_size, double val, bool leading_space, bool trailing_space) {
    if (val == (double)(long long)val && fabs(val) <= 999999999.0) {
        snprintf(buf, buf_size, "%s%lld%s", leading_space ? (val >= 0.0 ? " " : "") : "", (long long)val, trailing_space ? " " : "");
        return;
    }

    if (fabs(val) <= 999999999.0 && fabs(val) >= 0.000001) {
        char temp[128];
        snprintf(temp, sizeof(temp), "%f", val);
        
        char *end = temp + strlen(temp) - 1;
        while (end > temp && *end == '0') {
            *end = '\0';
            end--;
        }
        if (end > temp && *end == '.') {
            *end = '\0';
        }
        
        snprintf(buf, buf_size, "%s%s%s", leading_space ? (val >= 0.0 ? " " : "") : "", temp, trailing_space ? " " : "");
        return;
    }
    
    snprintf(buf, buf_size, "%s%g%s", leading_space ? (val >= 0.0 ? " " : "") : "", val, trailing_space ? " " : "");
}

#include <ctype.h>
#include <stdio.h>

static bool is_builtin_function(const char *name) {
    char uname[64];
    size_t i = 0;
    while (name[i] && i < 63) {
        uname[i] = (char)toupper((unsigned char)name[i]);
        i++;
    }
    uname[i] = '\0';

    if (i > 0 && uname[i - 1] != '$' && i < 62) {
        char test_name[64];
        strcpy(test_name, uname);
        strcat(test_name, "$");
        if (strcmp(test_name, "CHR$") == 0 ||
            strcmp(test_name, "STR$") == 0 ||
            strcmp(test_name, "LEFT$") == 0 ||
            strcmp(test_name, "RIGHT$") == 0 ||
            strcmp(test_name, "MID$") == 0 ||
            strcmp(test_name, "UCASE$") == 0 ||
            strcmp(test_name, "LCASE$") == 0 ||
            strcmp(test_name, "LTRIM$") == 0 ||
            strcmp(test_name, "RTRIM$") == 0 ||
            strcmp(test_name, "TRIM$") == 0 ||
            strcmp(test_name, "SPACE$") == 0 ||
            strcmp(test_name, "STRING$") == 0 ||
            strcmp(test_name, "REMOVE$") == 0 ||
            strcmp(test_name, "REPLACE$") == 0 ||
            strcmp(test_name, "HEX$") == 0 ||
            strcmp(test_name, "OCT$") == 0 ||
            strcmp(test_name, "BIN$") == 0 ||
            strcmp(test_name, "EDIT$") == 0 ||
            strcmp(test_name, "NUM$") == 0 ||
            strcmp(test_name, "TCASE$") == 0 ||
            strcmp(test_name, "ICASE$") == 0 ||
            strcmp(test_name, "REVERSE$") == 0 ||
            strcmp(test_name, "BASEDIR$") == 0 ||
            strcmp(test_name, "BASENAME$") == 0 ||
            strcmp(test_name, "BASEPATH$") == 0 ||
            strcmp(test_name, "HOSTNAME$") == 0 ||
            strcmp(test_name, "USERNAME$") == 0 ||
            strcmp(test_name, "PATH$") == 0 ||
            strcmp(test_name, "FILEMOD$") == 0 ||
            strcmp(test_name, "ERR$") == 0) {
            strcpy(uname, test_name);
            i = strlen(uname);
        }
    }

    if (strcmp(uname, "INKEY$") == 0 ||
        strcmp(uname, "PEN") == 0 ||
        strcmp(uname, "TIME$") == 0 ||
        strcmp(uname, "DATE$") == 0 ||
        strcmp(uname, "TIMER") == 0 ||
        strcmp(uname, "ALARM") == 0 ||
        strcmp(uname, "ALARM$") == 0 ||
        strcmp(uname, "EXISTS") == 0 ||
        strcmp(uname, "RANDOMIZE") == 0 ||
        strcmp(uname, "GUID$") == 0 ||
        strcmp(uname, "TIM") == 0 ||
        strcmp(uname, "TRUE") == 0 ||
        strcmp(uname, "FALSE") == 0 ||
        strcmp(uname, "TI") == 0 ||
        strcmp(uname, "TIME") == 0 ||
        strcmp(uname, "DATE") == 0 ||
        strcmp(uname, "TI$") == 0 ||
        strcmp(uname, "CLOCK$") == 0 ||
        strcmp(uname, "TZ") == 0 ||
        strcmp(uname, "TZ$") == 0 ||
        strcmp(uname, "TIMEZONE$") == 0 ||
        strcmp(uname, "UTC") == 0 ||
        strcmp(uname, "CSRLIN") == 0 ||
        strcmp(uname, "POS") == 0 ||
        strcmp(uname, "LPOS") == 0 ||
        strcmp(uname, "DAY") == 0 ||
        strcmp(uname, "MONTH") == 0 ||
        strcmp(uname, "YEAR") == 0 ||
        strcmp(uname, "DAY$") == 0 ||
        strcmp(uname, "MONTH$") == 0 ||
        strcmp(uname, "HOURS") == 0 ||
        strcmp(uname, "MINUTES") == 0 ||
        strcmp(uname, "SECONDS") == 0 ||
        strcmp(uname, "JIFFIES") == 0 ||
        strcmp(uname, "TICKS") == 0 ||
        strcmp(uname, "HOSTNAME$") == 0 ||
        strcmp(uname, "USERNAME$") == 0 ||
        strcmp(uname, "BASEDIR$") == 0 ||
        strcmp(uname, "BASEPATH$") == 0 ||
        strcmp(uname, "BASENAME$") == 0 ||
        strcmp(uname, "PATH$") == 0 ||
        strcmp(uname, "VER") == 0 ||
        strcmp(uname, "MEM") == 0 ||
        strcmp(uname, "SIZE") == 0 ||
        strcmp(uname, "PLAY") == 0 ||
        strcmp(uname, "TASK") == 0 ||
        strcmp(uname, "SQR") == 0 ||
        strcmp(uname, "ABS") == 0 ||
        strcmp(uname, "SIN") == 0 ||
        strcmp(uname, "COS") == 0 ||
        strcmp(uname, "TAN") == 0 ||
        strcmp(uname, "ATN") == 0 ||
        strcmp(uname, "LOG") == 0 ||
        strcmp(uname, "EXP") == 0 ||
        strcmp(uname, "INT") == 0 ||
        strcmp(uname, "FIX") == 0 ||
        strcmp(uname, "DET") == 0 ||
        strcmp(uname, "DOT") == 0 ||
        strcmp(uname, "CROSS") == 0 ||
        strcmp(uname, "RND") == 0 ||
        strcmp(uname, "LEN") == 0 ||
        strcmp(uname, "ASC") == 0 ||
        strcmp(uname, "CHR$") == 0 ||
        strcmp(uname, "CINT") == 0 ||
        strcmp(uname, "CSNG") == 0 ||
        strcmp(uname, "CDBL") == 0 ||
#ifndef BPP_LITE_BUILD
        strcmp(uname, "VARPTR") == 0 ||
        strcmp(uname, "VARPTR$") == 0 ||
        strcmp(uname, "VARSEG") == 0 ||
        strcmp(uname, "SADD") == 0 ||
#endif
        strcmp(uname, "VAL") == 0 ||
        strcmp(uname, "STR$") == 0 ||
        strcmp(uname, "LEFT$") == 0 ||
        strcmp(uname, "RIGHT$") == 0 ||
        strcmp(uname, "MID$") == 0 ||
        strcmp(uname, "INSTR") == 0 ||
        strcmp(uname, "UCASE$") == 0 ||
        strcmp(uname, "LCASE$") == 0 ||
        strcmp(uname, "LTRIM$") == 0 ||
        strcmp(uname, "RTRIM$") == 0 ||
        strcmp(uname, "TRIM$") == 0 ||
        strcmp(uname, "SPACE$") == 0 ||
        strcmp(uname, "STRING$") == 0 ||
        strcmp(uname, "REPLACE$") == 0 ||
        strcmp(uname, "HEX$") == 0 ||
        strcmp(uname, "OCT$") == 0 ||
        strcmp(uname, "BIN$") == 0 ||
        strcmp(uname, "EDIT$") == 0 ||
        strcmp(uname, "NUM$") == 0 ||
        strcmp(uname, "TCASE$") == 0 ||
        strcmp(uname, "ICASE$") == 0 ||
        strcmp(uname, "REVERSE$") == 0 ||
        strcmp(uname, "REMOVE$") == 0 ||
        strcmp(uname, "REMOVE") == 0 ||
        strcmp(uname, "HASH") == 0 ||
        strcmp(uname, "UBOUND") == 0 ||
        strcmp(uname, "LBOUND") == 0 ||
        strcmp(uname, "EOF") == 0 ||
        strcmp(uname, "LOF") == 0 ||
        strcmp(uname, "LOC") == 0 ||
        strcmp(uname, "SEEK") == 0 ||
        strcmp(uname, "HELP") == 0 ||
        strcmp(uname, "HELP$") == 0 ||
        strcmp(uname, "FREEFILE") == 0 ||
        strcmp(uname, "INPUT$") == 0 ||
        strcmp(uname, "SCREEN") == 0 ||
        strcmp(uname, "IOCTL$") == 0 ||
        strcmp(uname, "TXNSTATUS") == 0 ||
        strcmp(uname, "SIOREAD$") == 0 ||
        strcmp(uname, "SIOREADLN$") == 0 ||
        strcmp(uname, "SIOWRITE") == 0 ||
        strcmp(uname, "SIOSEEK") == 0 ||
        strcmp(uname, "SIOFLUSH") == 0 ||
        strcmp(uname, "SIOSTATUS") == 0 ||
        strcmp(uname, "SIOAVAIL") == 0 ||
        strcmp(uname, "BIOREAD$") == 0 ||
        strcmp(uname, "BIOWRITE") == 0 ||
        strcmp(uname, "BIOCOPY") == 0 ||
        strcmp(uname, "BIOFILL") == 0 ||
        strcmp(uname, "BIOSTATUS") == 0 ||
        strcmp(uname, "BIOSIZE") == 0 ||
        strcmp(uname, "BIOCHECKSUM") == 0 ||
        strcmp(uname, "BIOCOMPARE") == 0 ||
        strcmp(uname, "FILEATTR") == 0 ||
        strcmp(uname, "MKI$") == 0 ||
        strcmp(uname, "MKS$") == 0 ||
        strcmp(uname, "MKD$") == 0 ||
        strcmp(uname, "CVI") == 0 ||
        strcmp(uname, "CVS") == 0 ||
        strcmp(uname, "CVD") == 0 ||
        strcmp(uname, "DEVICECOUNT") == 0 ||
        strcmp(uname, "DEVICE$") == 0 ||
        strcmp(uname, "DEVICECLASS$") == 0 ||
        strcmp(uname, "DEVICEINFO$") == 0 ||
        strcmp(uname, "POLL") == 0 ||
#if BPP_SUPPORT_NET
        strcmp(uname, "NSTATUS") == 0 ||
        strcmp(uname, "NCONNECTED") == 0 ||
        strcmp(uname, "NHTTPSTATUS") == 0 ||
        strcmp(uname, "HTTP_GET$") == 0 ||
#endif
#if BPP_SUPPORT_BIOS
        strcmp(uname, "MEMMAP$") == 0 ||
#endif
        strcmp(uname, "PEEK") == 0 ||
        strcmp(uname, "MAP") == 0 ||
        strcmp(uname, "MAP_NEW") == 0 ||
        strcmp(uname, "MAP_SET") == 0 ||
        strcmp(uname, "MAP_GET") == 0 ||
        strcmp(uname, "MAP_GET$") == 0 ||
        strcmp(uname, "MAP_REMOVE") == 0 ||
        strcmp(uname, "MAP_COUNT") == 0 ||
        strcmp(uname, "MAP_KEY$") == 0 ||
        strcmp(uname, "MAP_HAS") == 0 ||
        strcmp(uname, "JSON_PARSE") == 0 ||
        strcmp(uname, "JSON_STRINGIFY$") == 0 ||
        strcmp(uname, "XML_PARSE") == 0 ||
        strcmp(uname, "XML_STRINGIFY$") == 0 ||
        strcmp(uname, "USR") == 0 || strcmp(uname, "USR0") == 0 ||
        strcmp(uname, "USR1") == 0 || strcmp(uname, "USR2") == 0 ||
        strcmp(uname, "USR3") == 0 || strcmp(uname, "USR4") == 0 ||
        strcmp(uname, "USR5") == 0 || strcmp(uname, "USR6") == 0 ||
        strcmp(uname, "USR7") == 0 || strcmp(uname, "USR8") == 0 ||
        strcmp(uname, "USR9") == 0 ||
        strcmp(uname, "ERDEV") == 0 || strcmp(uname, "ERDEV$") == 0 ||
        strcmp(uname, "EXTERR") == 0 ||
        strcmp(uname, "HASH$") == 0 || strcmp(uname, "SALT$") == 0 ||
        strcmp(uname, "AUDITCRACK") == 0 || strcmp(uname, "AUDITCRACK$") == 0 ||
        strcmp(uname, "SANDBOXAUDIT") == 0 || strcmp(uname, "VMCHECK") == 0 ||
        strcmp(uname, "NETHOST$") == 0 || strcmp(uname, "NETIP$") == 0 ||
        strcmp(uname, "YAML_PARSE") == 0 ||
        strcmp(uname, "YAML_STRINGIFY$") == 0 ||
        strcmp(uname, "INI_PARSE") == 0 ||
        strcmp(uname, "INI_STRINGIFY$") == 0 ||
        strcmp(uname, "DIALECT_LOAD") == 0 ||
        strcmp(uname, "DIALECT_REGISTER") == 0 ||
        strcmp(uname, "DIALECT_VALIDATE") == 0 ||
        strcmp(uname, "DIALECT_DOC$") == 0 ||
        strcmp(uname, "ENVIRON$") == 0 ||
        strcmp(uname, "DIR$") == 0 ||
        strcmp(uname, "GETATTR") == 0 ||
        strcmp(uname, "_SHL") == 0 || strcmp(uname, "BITS.SHL") == 0 ||
        strcmp(uname, "_SHR") == 0 || strcmp(uname, "BITS.SHR") == 0 ||
        strcmp(uname, "_READBIT") == 0 || strcmp(uname, "BITS.READ") == 0 ||
        strcmp(uname, "_SETBIT") == 0 || strcmp(uname, "BITS.SET") == 0 ||
        strcmp(uname, "_RESETBIT") == 0 || strcmp(uname, "BITS.RESET") == 0 ||
        strcmp(uname, "_TOGGLEBIT") == 0 || strcmp(uname, "BITS.TOGGLE") == 0 ||
        strcmp(uname, "_BITCOUNT") == 0 || strcmp(uname, "BITS.COUNT") == 0 ||
        strcmp(uname, "_ACOS") == 0 || strcmp(uname, "MATH.ACOS") == 0 ||
        strcmp(uname, "_ASIN") == 0 || strcmp(uname, "MATH.ASIN") == 0 ||
        strcmp(uname, "_ATAN2") == 0 || strcmp(uname, "MATH.ATAN2") == 0 ||
        strcmp(uname, "_ACOSH") == 0 || strcmp(uname, "MATH.ACOSH") == 0 ||
        strcmp(uname, "_ASINH") == 0 || strcmp(uname, "MATH.ASINH") == 0 ||
        strcmp(uname, "_ATANH") == 0 || strcmp(uname, "MATH.ATANH") == 0 ||
        strcmp(uname, "_CEIL") == 0 || strcmp(uname, "MATH.CEIL") == 0 ||
        strcmp(uname, "_HYPOT") == 0 || strcmp(uname, "MATH.HYPOT") == 0 ||
        strcmp(uname, "_PI") == 0 || strcmp(uname, "MATH.PI") == 0 ||
        strcmp(uname, "_D2R") == 0 || strcmp(uname, "MATH.D2R") == 0 ||
        strcmp(uname, "_R2D") == 0 || strcmp(uname, "MATH.R2D") == 0 ||
        strcmp(uname, "_D2G") == 0 || strcmp(uname, "MATH.D2G") == 0 ||
        strcmp(uname, "_G2D") == 0 || strcmp(uname, "MATH.G2D") == 0 ||
        strcmp(uname, "INP") == 0) {
        return true;
    }

    if (funcreg_find_by_name(uname) != NULL) {
        return true;
    }

    return false;
}

static char *eval_read_file_to_string(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    char *buf = calloc(size + 1, 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, size, fp);
    buf[read_bytes] = '\0';
    fclose(fp);
    return buf;
}

static BValue eval_builtin_function(VMContext *vm, const char *name, LexerContext *lex, bool has_parens, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    char uname[64];
    size_t i = 0;
    while (name[i] && i < 63) {
        uname[i] = (char)toupper((unsigned char)name[i]);
        i++;
    }
    uname[i] = '\0';

    if (i > 0 && uname[i - 1] != '$' && i < 62) {
        char test_name[64];
        strcpy(test_name, uname);
        strcat(test_name, "$");
        if (strcmp(test_name, "CHR$") == 0 ||
            strcmp(test_name, "STR$") == 0 ||
            strcmp(test_name, "LEFT$") == 0 ||
            strcmp(test_name, "RIGHT$") == 0 ||
            strcmp(test_name, "MID$") == 0 ||
            strcmp(test_name, "UCASE$") == 0 ||
            strcmp(test_name, "LCASE$") == 0 ||
            strcmp(test_name, "LTRIM$") == 0 ||
            strcmp(test_name, "RTRIM$") == 0 ||
            strcmp(test_name, "TRIM$") == 0 ||
            strcmp(test_name, "SPACE$") == 0 ||
            strcmp(test_name, "STRING$") == 0 ||
            strcmp(test_name, "REMOVE$") == 0 ||
            strcmp(test_name, "REPLACE$") == 0 ||
            strcmp(test_name, "HEX$") == 0 ||
            strcmp(test_name, "OCT$") == 0 ||
            strcmp(test_name, "BIN$") == 0 ||
            strcmp(test_name, "EDIT$") == 0 ||
            strcmp(test_name, "NUM$") == 0 ||
            strcmp(test_name, "TCASE$") == 0 ||
            strcmp(test_name, "ICASE$") == 0 ||
            strcmp(test_name, "REVERSE$") == 0 ||
            strcmp(test_name, "BASEDIR$") == 0 ||
            strcmp(test_name, "BASENAME$") == 0 ||
            strcmp(test_name, "BASEPATH$") == 0 ||
            strcmp(test_name, "HOSTNAME$") == 0 ||
            strcmp(test_name, "USERNAME$") == 0 ||
            strcmp(test_name, "PATH$") == 0 ||
            strcmp(test_name, "FILEMOD$") == 0 ||
            strcmp(test_name, "ERR$") == 0) {
            strcpy(uname, test_name);
            i = strlen(uname);
        }
    }

    if (strcmp(uname, "UBOUND") == 0 || strcmp(uname, "LBOUND") == 0) {
        bool is_u = (strcmp(uname, "UBOUND") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected array name in UBOUND/LBOUND";
            return res;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';

        int dim = 1;
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
            BValue dim_val = eval_expression(vm, lex, err);
            if (err->code != 0) return res;
            if (dim_val.type == VAL_STRING) {
                err->code = 13;
                err->message = "Dimension must be numeric";
                return res;
            }
            dim = (int)dim_val.as.number;
        }

        next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' in UBOUND/LBOUND";
            return res;
        }

        if (is_u) {
            bool found = false;
            int u = arr_ubound(vm_get_arr(vm), arr_name, dim, &found);
            if (!found) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return res;
            }
            res.type = VAL_NUMBER;
            res.as.number = (double)u;
        } else {
            if (!arr_exists(vm_get_arr(vm), arr_name)) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return res;
            }
            res.type = VAL_NUMBER;
            res.as.number = (double)arr_get_option_base(vm_get_arr(vm));
        }
        return res;
    }

    if (strcmp(uname, "DET") == 0) {
        if (!has_parens) {
            res.type = VAL_NUMBER;
            res.as.number = arr_get_last_det(vm_get_arr(vm));
            return res;
        }
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2; err->message = "Expected array name in DET()"; return res;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';
        if (lex_next(lex).type != TOK_RPAREN) {
            err->code = 2; err->message = "Expected ')' in DET()"; return res;
        }
        
        bool found = false;
        int d1 = arr_ubound(vm_get_arr(vm), arr_name, 1, &found);
        int d2 = arr_ubound(vm_get_arr(vm), arr_name, 2, &found);
        if (!found || d1 != d2 || d1 < 1) {
            err->code = 9; err->message = "DET expects a square matrix"; return res;
        }
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
        if (!elems) { err->code = 9; err->message = "Array not found"; return res; }
        
        /* Basic 2x2 or 3x3 determinant calculation for now */
        int base = arr_get_option_base(vm_get_arr(vm));
        int n = d1 - base + 1;
        double det = 0.0;
        if (n == 2) {
            double a = elems[(1-base)*n + (1-base)].as.number;
            double b = elems[(1-base)*n + (2-base)].as.number;
            double c = elems[(2-base)*n + (1-base)].as.number;
            double d = elems[(2-base)*n + (2-base)].as.number;
            det = a*d - b*c;
        } else if (n == 3) {
            double a = elems[(1-base)*n + (1-base)].as.number;
            double b = elems[(1-base)*n + (2-base)].as.number;
            double c = elems[(1-base)*n + (3-base)].as.number;
            double d = elems[(2-base)*n + (1-base)].as.number;
            double e = elems[(2-base)*n + (2-base)].as.number;
            double f = elems[(2-base)*n + (3-base)].as.number;
            double g = elems[(3-base)*n + (1-base)].as.number;
            double h = elems[(3-base)*n + (2-base)].as.number;
            double i_val = elems[(3-base)*n + (3-base)].as.number;
            det = a*(e*i_val - f*h) - b*(d*i_val - f*g) + c*(d*h - e*g);
        } else {
            err->code = 5; err->message = "DET only supports 2x2 and 3x3 matrices in this version"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = det;
        return res;
    }

    if (strcmp(uname, "DOT") == 0) {
        if (lex_next(lex).type != TOK_LPAREN) { err->code = 2; err->message = "Expected '(' in DOT()"; return res; }
        
        BppToken name_tok1 = lex_next(lex);
        if (name_tok1.type != TOK_IDENT) { err->code = 2; err->message = "Expected first array name in DOT()"; return res; }
        char arr1[256];
        size_t clen1 = (name_tok1.length < sizeof(arr1) - 1) ? name_tok1.length : sizeof(arr1) - 1;
        memcpy(arr1, name_tok1.start, clen1);
        arr1[clen1] = '\0';
        
        if (lex_next(lex).type != TOK_COMMA) { err->code = 2; err->message = "Expected ',' in DOT()"; return res; }
        
        BppToken name_tok2 = lex_next(lex);
        if (name_tok2.type != TOK_IDENT) { err->code = 2; err->message = "Expected second array name in DOT()"; return res; }
        char arr2[256];
        size_t clen2 = (name_tok2.length < sizeof(arr2) - 1) ? name_tok2.length : sizeof(arr2) - 1;
        memcpy(arr2, name_tok2.start, clen2);
        arr2[clen2] = '\0';
        
        if (lex_next(lex).type != TOK_RPAREN) { err->code = 2; err->message = "Expected ')' in DOT()"; return res; }
        
        int b1[4], b2[4];
        int dim1 = arr_get_dimensions(vm_get_arr(vm), arr1, b1, 4);
        int dim2 = arr_get_dimensions(vm_get_arr(vm), arr2, b2, 4);
        
        if (dim1 < 1 || dim2 < 1 || b1[0] != b2[0]) {
            err->code = 9; err->message = "DOT expects two 1D arrays of same size"; return res;
        }
        
        int sz1=0, sz2=0;
        BValue *e1 = arr_get_flat_elements(vm_get_arr(vm), arr1, &sz1);
        BValue *e2 = arr_get_flat_elements(vm_get_arr(vm), arr2, &sz2);
        if (!e1 || !e2 || sz1 != sz2) { err->code = 9; err->message = "Array mismatch in DOT"; return res; }
        
        double dot = 0.0;
        for (int idx=0; idx<sz1; idx++) {
            dot += e1[idx].as.number * e2[idx].as.number;
        }
        res.type = VAL_NUMBER;
        res.as.number = dot;
        return res;
    }

#ifndef BPP_LITE_BUILD
    if (strcmp(uname, "VARPTR") == 0 || strcmp(uname, "VARPTR$") == 0 || strcmp(uname, "VARSEG") == 0 || strcmp(uname, "SADD") == 0) {
        bool is_seg = (strcmp(uname, "VARSEG") == 0);
        bool is_sadd = (strcmp(uname, "SADD") == 0);
        bool is_str = (strcmp(uname, "VARPTR$") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected variable name in VARPTR/VARSEG/SADD";
            return res;
        }
        char var_name[64];
        size_t clen = (name_tok.length < 63) ? name_tok.length : 63;
        memcpy(var_name, name_tok.start, clen);
        var_name[clen] = '\0';
        
        VariableContext *var = vm_get_var(vm);
        BValue *target = var_lookup(var, var_name, true);
        
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            while (lex_peek(lex).type != TOK_RPAREN && lex_peek(lex).type != TOK_EOF) {
                lex_next(lex);
            }
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
        }
        
        BppToken next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')'";
            return res;
        }
        
        uint32_t handle = vmem_register_handle(vm_get_vmem(vm), target, is_sadd);

        if (is_str) {
            char desc[4];
            int type_code = 3; /* default string */
            if (target) {
                if (target->type == VAL_NUMBER) {
                    type_code = 8; /* double */
                } else if (target->type == VAL_STRING) {
                    type_code = 3; /* string */
                }
            }
            desc[0] = (char)type_code;
            desc[1] = (char)(handle & 0xFF);
            desc[2] = (char)((handle >> 8) & 0xFF);
            desc[3] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), desc, 3);
        } else {
            res.type = VAL_NUMBER;
            if (is_seg) {
                res.as.number = (double)((handle >> 16) & 0xFFFF);
            } else {
                res.as.number = (double)(handle & 0xFFFF);
            }
        }
        return res;
    }
#endif

    if (!has_parens) {
        return eval_builtin_function_impl(vm, uname, 0, NULL, err);
    }

    BValue args[10];
    int arg_count = 0;
    memset(args, 0, sizeof(args));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_RPAREN) {
        while (true) {
            if (arg_count >= 10) {
                err->code = 2;
                err->message = "Too many arguments for function";
                break;
            }
            args[arg_count] = eval_expression(vm, lex, err);
            if (err->code != 0) break;
            arg_count++;

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex); /* Consume ',' */
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err->code = 2;
                err->message = "Expected ',' or ')' in function call";
                break;
            }
        }
    }

    if (err->code == 0) {
        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' to close function call";
        }
    }

    if (err->code == 0) {
        res = eval_builtin_function_impl(vm, uname, arg_count, args, err);
    }

    /* Standard leak-free cleanup for all args if function execution failed or parsing failed mid-way */
    if (err->code != 0) {
        for (int j = 0; j < arg_count; j++) {
            if (args[j].type == VAL_STRING && args[j].as.string) {
                str_release(vm_get_str(vm), args[j].as.string);
                args[j].as.string = NULL;
            } else if (args[j].type == VAL_MAP && args[j].as.map) {
                bpp_map_release(vm_get_str(vm), args[j].as.map);
                args[j].as.map = NULL;
            }
        }
    }

    return res;
}

static uint16_t compute_crc16(const unsigned char *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)(data[i] << 8);
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static BValue eval_builtin_function_impl(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "SQR") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SQR expects one numeric argument"; return res;
        }
        if (args[0].as.number < 0.0) {
            err->code = 5; err->message = "SQR of negative number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = sqrt(args[0].as.number);
    }
    else if (strcmp(uname, "_SHL") == 0 || strcmp(uname, "BITS.SHL") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SHL expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)args[0].as.number << (uint64_t)args[1].as.number);
    }
    else if (strcmp(uname, "_SHR") == 0 || strcmp(uname, "BITS.SHR") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SHR expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)args[0].as.number >> (uint64_t)args[1].as.number);
    }
    else if (strcmp(uname, "_READBIT") == 0 || strcmp(uname, "BITS.READ") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "READBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)(((uint64_t)args[0].as.number >> (uint64_t)args[1].as.number) & 1);
    }
    else if (strcmp(uname, "_SETBIT") == 0 || strcmp(uname, "BITS.SET") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SETBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)args[0].as.number | ((uint64_t)1 << (uint64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_RESETBIT") == 0 || strcmp(uname, "BITS.RESET") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "RESETBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)args[0].as.number & ~((uint64_t)1 << (uint64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_TOGGLEBIT") == 0 || strcmp(uname, "BITS.TOGGLE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "TOGGLEBIT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)((uint64_t)args[0].as.number ^ ((uint64_t)1 << (uint64_t)args[1].as.number));
    }
    else if (strcmp(uname, "_BITCOUNT") == 0 || strcmp(uname, "BITS.COUNT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BITCOUNT expects one numeric argument"; return res;
        }
        uint64_t temp = (uint64_t)args[0].as.number;
        int count = 0;
        while (temp) {
            if (temp & 1) count++;
            temp >>= 1;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)count;
    }
    else if (strcmp(uname, "_ACOS") == 0 || strcmp(uname, "MATH.ACOS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ACOS expects one numeric argument"; return res;
        }
        if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
            err->code = 5; err->message = "ACOS domain error"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = acos(args[0].as.number);
    }
    else if (strcmp(uname, "_ASIN") == 0 || strcmp(uname, "MATH.ASIN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ASIN expects one numeric argument"; return res;
        }
        if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
            err->code = 5; err->message = "ASIN domain error"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = asin(args[0].as.number);
    }
    else if (strcmp(uname, "_ATAN2") == 0 || strcmp(uname, "MATH.ATAN2") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "ATAN2 expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atan2(args[0].as.number, args[1].as.number);
    }
    else if (strcmp(uname, "_ACOSH") == 0 || strcmp(uname, "MATH.ACOSH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ACOSH expects one numeric argument"; return res;
        }
        if (args[0].as.number < 1.0) {
            err->code = 5; err->message = "ACOSH domain error (x must be >= 1.0)"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = acosh(args[0].as.number);
    }
    else if (strcmp(uname, "_ASINH") == 0 || strcmp(uname, "MATH.ASINH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ASINH expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = asinh(args[0].as.number);
    }
    else if (strcmp(uname, "_ATANH") == 0 || strcmp(uname, "MATH.ATANH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ATANH expects one numeric argument"; return res;
        }
        if (args[0].as.number <= -1.0 || args[0].as.number >= 1.0) {
            err->code = 5; err->message = "ATANH domain error (x must be between -1.0 and 1.0)"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atanh(args[0].as.number);
    }
    else if (strcmp(uname, "_CEIL") == 0 || strcmp(uname, "MATH.CEIL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CEIL expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = ceil(args[0].as.number);
    }
    else if (strcmp(uname, "_HYPOT") == 0 || strcmp(uname, "MATH.HYPOT") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "HYPOT expects two numeric arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = hypot(args[0].as.number, args[1].as.number);
    }
    else if (strcmp(uname, "_PI") == 0 || strcmp(uname, "MATH.PI") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "PI expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 3.14159265358979323846;
    }
    else if (strcmp(uname, "_D2R") == 0 || strcmp(uname, "MATH.D2R") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "D2R expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (3.14159265358979323846 / 180.0);
    }
    else if (strcmp(uname, "_R2D") == 0 || strcmp(uname, "MATH.R2D") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "R2D expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (180.0 / 3.14159265358979323846);
    }
    else if (strcmp(uname, "_D2G") == 0 || strcmp(uname, "MATH.D2G") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "D2G expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (10.0 / 9.0);
    }
    else if (strcmp(uname, "_G2D") == 0 || strcmp(uname, "MATH.G2D") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "G2D expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number * (9.0 / 10.0);
    }
    else if (strcmp(uname, "INKEY$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "INKEY$ expects no arguments"; return res;
        }
        int ch = platform_inkey_char();
        char buf[2] = {0};
        if (ch > 0) {
            buf[0] = (char)ch;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, buf[0] ? 1 : 0);
    }
    else if (strcmp(uname, "PEN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PEN expects one numeric argument"; return res;
        }
        int n = (int)args[0].as.number;
        int val = 0;
        if (n == 3) val = platform_mouse_btn();
        else if (n == 4) val = platform_mouse_x();
        else if (n == 5) val = platform_mouse_y();
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "TIME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIME$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[32] = "";
        if (lt) {
            int hour12 = lt->tm_hour % 12;
            if (hour12 == 0) hour12 = 12;
            char ap = (lt->tm_hour >= 12) ? 'P' : 'A';
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d %c", hour12, lt->tm_min, lt->tm_sec, ap);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TI$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TI$ expects no arguments"; return res;
        }
        double adjusted = platform_get_uptime() + vm_get_ti_offset(vm);
        if (adjusted < 0.0) adjusted = 0.0;
        long long sec_total = (long long)adjusted;
        long long days = sec_total / 86400;
        long long remaining = sec_total % 86400;
        int hr = (int)(remaining / 3600);
        int min = (int)((remaining / 60) % 60);
        int sec = (int)(remaining % 60);
        char buf[64];
        if (days == 0) {
            snprintf(buf, sizeof(buf), "%02d%02d%02d", hr, min, sec);
        } else {
            snprintf(buf, sizeof(buf), "%03lld:%02d%02d%02d", days, hr, min, sec);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "CLOCK$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "CLOCK$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        char buf[64] = "";
        if (gt) {
            snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                     gt->tm_year + 1900, gt->tm_mon + 1, gt->tm_mday,
                     gt->tm_hour, gt->tm_min, gt->tm_sec);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "CSRLIN") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "CSRLIN expects no arguments"; return res;
        }
        extern int g_cursor_y;
        res.type = VAL_NUMBER;
        res.as.number = (double)(g_cursor_y + 1);
    }
    else if (strcmp(uname, "POS") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "POS expects 1 argument"; return res;
        }
        extern int g_cursor_x;
        res.type = VAL_NUMBER;
        res.as.number = (double)(g_cursor_x + 1);
    }
    else if (strcmp(uname, "LPOS") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "LPOS expects 1 argument"; return res;
        }
        extern int g_lpos;
        res.type = VAL_NUMBER;
        res.as.number = (double)g_lpos;
    }
    else if (strcmp(uname, "TZ$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TZ$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[64] = "UTC";
        if (lt) {
            char tz_buf[64] = "";
            strftime(tz_buf, sizeof(tz_buf), "%Z", lt);
            if (tz_buf[0] != '\0') {
                size_t len = strlen(tz_buf);
                bool is_abbr = (len <= 5);
                if (is_abbr) {
                    for (size_t i = 0; i < len; i++) {
                        if (tz_buf[i] >= 'a' && tz_buf[i] <= 'z') {
                            is_abbr = false;
                            break;
                        }
                    }
                }
                if (is_abbr) {
                    strncpy(buf, tz_buf, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                } else {
                    int w_idx = 0;
                    for (size_t i = 0; i < len; i++) {
                        if (tz_buf[i] >= 'A' && tz_buf[i] <= 'Z') {
                            if (w_idx < (int)sizeof(buf) - 1) {
                                buf[w_idx++] = tz_buf[i];
                            }
                        }
                    }
                    buf[w_idx] = '\0';
                    if (w_idx == 0) {
                        strcpy(buf, "UTC");
                    }
                }
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TIMEZONE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIMEZONE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[64] = "UTC";
        if (lt) {
            char tz_buf[64] = "";
            strftime(tz_buf, sizeof(tz_buf), "%Z", lt);
            if (tz_buf[0] != '\0') {
                strncpy(buf, tz_buf, sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TZ") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TZ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            char buf[16] = "";
            strftime(buf, sizeof(buf), "%z", lt);
            val = atof(buf);
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "UTC") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "UTC expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            char buf[16] = "";
            strftime(buf, sizeof(buf), "%z", lt);
            if (strlen(buf) >= 5) {
                int sign = (buf[0] == '-') ? -1 : 1;
                int hours = (buf[1] - '0') * 10 + (buf[2] - '0');
                int mins = (buf[3] - '0') * 10 + (buf[4] - '0');
                val = sign * (hours * 3600.0 + mins * 60.0);
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "DATE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TRUE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TRUE expects no arguments"; return res;
        }
        res.type = VAL_NUMBER; res.as.number = 1.0;
    }
    else if (strcmp(uname, "FALSE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "FALSE expects no arguments"; return res;
        }
        res.type = VAL_NUMBER; res.as.number = -1.0;
    }
    else if (strcmp(uname, "TI") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TI expects no arguments"; return res;
        }
        double val = platform_get_uptime() + vm_get_ti_offset(vm);
        val = fmod(val, 999999999.0);
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "TIME") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIME expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = lt->tm_hour * 10000.0 + lt->tm_min * 100.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "DATE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = (lt->tm_year + 1900) * 10000.0 + (lt->tm_mon + 1) * 100.0 + lt->tm_mday;
        }
        res.type = VAL_NUMBER; res.as.number = val;
    }
    else if (strcmp(uname, "DAY") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DAY expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_mday : 1.0;
    }
    else if (strcmp(uname, "MONTH") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MONTH expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_mon + 1 : 1.0;
    }
    else if (strcmp(uname, "YEAR") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "YEAR expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = lt ? lt->tm_year + 1900 : 2026.0;
    }
    else if (strcmp(uname, "DAY$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DAY$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
        const char *day_str = (lt && lt->tm_wday >= 0 && lt->tm_wday < 7) ? days[lt->tm_wday] : "SUN";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), day_str, strlen(day_str));
    }
    else if (strcmp(uname, "MONTH$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MONTH$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        const char *mon_str = (lt && lt->tm_mon >= 0 && lt->tm_mon < 12) ? months[lt->tm_mon] : "JAN";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), mon_str, strlen(mon_str));
    }
    else if (strcmp(uname, "HOURS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "HOURS expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_hour : 0.0;
    }
    else if (strcmp(uname, "MINUTES") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MINUTES expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_min : 0.0;
    }
    else if (strcmp(uname, "SECONDS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "SECONDS expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
        struct tm *gt = platform_gmtime(&t, &tm_buf);
        res.type = VAL_NUMBER; res.as.number = gt ? gt->tm_sec : 0.0;
    }
    else if (strcmp(uname, "JIFFIES") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "JIFFIES expects 0 or 1 arguments"; return res;
        }
        double mult = vm_get_jiffies_multiplier(vm);
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "JIFFIES expects a string argument"; return res;
            }
            const char *mode = str_data(args[0].as.string);
            if (mode) {
                if (strcasecmp(mode, "NTSC") == 0) mult = 60.0;
                else if (strcasecmp(mode, "PAL") == 0) mult = 50.0;
                else if (strcasecmp(mode, "SECAM") == 0) mult = 50.0;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = platform_get_uptime() * mult;
    }
    else if (strcmp(uname, "TICKS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TICKS expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = platform_get_uptime() * 100.0;
    }
    else if (strcmp(uname, "HOSTNAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "HOSTNAME$ expects no arguments"; return res;
        }
        const char *env_val = getenv("HOSTNAME");
        if (!env_val) env_val = getenv("COMPUTERNAME");
        if (!env_val) env_val = "localhost";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    }
    else if (strcmp(uname, "USERNAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "USERNAME$ expects no arguments"; return res;
        }
        const char *env_val = getenv("USERNAME");
        if (!env_val) env_val = getenv("USER");
        if (!env_val) env_val = "user";
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), env_val, strlen(env_val));
    }
    else if (strcmp(uname, "BASEPATH$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "BASEPATH$ expects no arguments"; return res;
        }
        const char *path = vfs_get_category_path(vm_get_vfs(vm), "WORKING");
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), path ? path : "", path ? strlen(path) : 0);
    }
    else if (strcmp(uname, "BASEDIR$") == 0 || strcmp(uname, "BASENAME$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "BASEDIR$/BASENAME$ expects no arguments"; return res;
        }
        const char *path = vfs_get_category_path(vm_get_vfs(vm), "WORKING");
        char tmp[512] = "";
        if (path) {
            size_t len = strlen(path);
            if (len < sizeof(tmp)) {
                strcpy(tmp, path);
            }
        }
        size_t len = strlen(tmp);
        if (len > 1 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
            tmp[len - 1] = '\0';
            len--;
        }
        char *last_sep = strrchr(tmp, '/');
        char *last_back = strrchr(tmp, '\\');
        char *sep = (last_sep > last_back) ? last_sep : last_back;
        char res_buf[512] = "";
        if (strcmp(uname, "BASEDIR$") == 0) {
            if (sep) {
                if (sep == tmp + 2 && tmp[1] == ':') {
                    strncpy(res_buf, tmp, 3);
                    res_buf[3] = '\0';
                } else if (sep == tmp) {
                    strcpy(res_buf, "/");
                } else {
                    size_t parent_len = sep - tmp;
                    strncpy(res_buf, tmp, parent_len);
                    res_buf[parent_len] = '\0';
                }
            } else {
                strcpy(res_buf, ".");
            }
        } else {
            if (sep) {
                strcpy(res_buf, sep + 1);
            } else {
                strcpy(res_buf, tmp);
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), res_buf, strlen(res_buf));
    }
    else if (strcmp(uname, "PATH$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "PATH$ expects 0 or 1 arguments"; return res;
        }
        const char *path_val = NULL;
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "PATH$ expects a string argument"; return res;
            }
            path_val = vfs_get_category_path(vm_get_vfs(vm), str_data(args[0].as.string));
        } else {
            path_val = vfs_get_search_path(vm_get_vfs(vm));
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), path_val ? path_val : "", path_val ? strlen(path_val) : 0);
    }
    else if (strcmp(uname, "VER") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "VER expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 6.4;
    }
    else if (strcmp(uname, "MEM") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MEM expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    }
    else if (strcmp(uname, "SIZE") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "SIZE expects no arguments"; return res;
        }
        MemoryContext *mctx = vm_get_mem(vm);
        size_t limit = mem_get_free_ram(mctx) + mem_get_used_ram(mctx);
        res.type = VAL_NUMBER;
        res.as.number = (double)limit;
    }
    else if (strcmp(uname, "DATE$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DATE$ expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TIMER") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TIMER expects no arguments"; return res;
        }
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0;
        if (lt) {
            val = lt->tm_hour * 3600.0 + lt->tm_min * 60.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ALARM") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "ALARM expects 0 or 1 arguments"; return res;
        }
        extern double vm_get_alarm_countdown(VMContext *vm, double seconds);
        extern double vm_get_closest_alarm_countdown(VMContext *vm);
        double val = 0.0;
        if (arg_count == 1) {
            if (args[0].type != VAL_NUMBER) {
                err->code = 13; err->message = "ALARM expects a numeric argument"; return res;
            }
            val = vm_get_alarm_countdown(vm, args[0].as.number);
        } else {
            val = vm_get_closest_alarm_countdown(vm);
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ALARM$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "ALARM$ expects 0 or 1 arguments"; return res;
        }
        extern double vm_get_alarm_daily_remaining(VMContext *vm, const char *time_str);
        extern double vm_get_closest_alarm_daily_remaining(VMContext *vm);
        double remaining = -1.0;
        if (arg_count == 1) {
            char time_str[128] = "";
            if (args[0].type == VAL_NUMBER) {
                int val_i = (int)args[0].as.number;
                int h = val_i / 10000;
                int m = (val_i % 10000) / 100;
                int s = val_i % 100;
                if (h < 0 || h >= 24 || m < 0 || m >= 60 || s < 0 || s >= 60) {
                    err->code = 5; err->message = "Illegal function call: Invalid time format";
                    return res;
                }
                snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", h, m, s);
            } else {
                size_t tlen = (str_len(args[0].as.string) < 127) ? str_len(args[0].as.string) : 127;
                memcpy(time_str, str_data(args[0].as.string), tlen);
                time_str[tlen] = '\0';
            }
            remaining = vm_get_alarm_daily_remaining(vm, time_str);
        } else {
            remaining = vm_get_closest_alarm_daily_remaining(vm);
        }

        if (remaining < 0.0) {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), "", 0);
        } else {
            int h = (int)(remaining / 3600.0);
            int m = (int)(((int)remaining % 3600) / 60.0);
            int s = (int)((int)remaining % 60);
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
        }
    }
    else if (strcmp(uname, "EXISTS") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "EXISTS expects one string argument"; return res;
        }
        const char *filename = str_data(args[0].as.string);
        char resolved[512] = "";
        bool exists = false;
        if (filename) {
            vfs_resolve(vm_get_vfs(vm), filename, resolved, sizeof(resolved));
            FILE *fp = fopen(resolved, "rb");
            if (fp) {
                exists = true;
                fclose(fp);
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = exists ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "RANDOMIZE") == 0) {
        if (arg_count == 0) {
            err->code = 13; err->message = "RANDOMIZE function expects arguments"; return res;
        }
        if (args[0].type == VAL_STRING) {
            const char *mode = str_data(args[0].as.string);
            if (mode && strcmp(mode, "STRING$") == 0) {
                int len = 8;
                if (arg_count > 1 && args[1].type == VAL_NUMBER) {
                    len = (int)args[1].as.number;
                }
                char *buf = (char *)calloc(1, len + 1);
                const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                for (int i = 0; i < len; i++) {
                    buf[i] = charset[rand() % (sizeof(charset) - 1)];
                }
                buf[len] = '\0';
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, len);
                free(buf);
            } else if (mode && strcmp(mode, "DATE$") == 0) {
                int m = rand() % 12 + 1;
                int d = rand() % 28 + 1;
                int y = rand() % 100;
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d-%02d-%02d", m, d, y);
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
            } else if (mode && strcmp(mode, "DAY$") == 0) {
                const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
                const char *day = days[rand() % 7];
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), day, strlen(day));
            } else if (mode && strcmp(mode, "TIME$") == 0) {
                int h = rand() % 24;
                int m = rand() % 60;
                int s = rand() % 60;
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
            } else {
                size_t len = strlen(mode);
                char *buf = (char *)calloc(1, len + 1);
                strcpy(buf, mode);
                for (size_t i = len - 1; i > 0; i--) {
                    size_t j = rand() % (i + 1);
                    char tmp = buf[i];
                    buf[i] = buf[j];
                    buf[j] = tmp;
                }
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), buf, len);
                free(buf);
            }
        } else {
            err->code = 13; err->message = "RANDOMIZE function expects a string as first argument"; return res;
        }
    }
    else if (strcmp(uname, "GUID$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "GUID$ expects no arguments"; return res;
        }
        char buf[37];
        const char *chars = "0123456789abcdef";
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                buf[i] = '-';
            } else if (i == 14) {
                buf[i] = '4';
            } else if (i == 19) {
                buf[i] = chars[(rand() % 4) + 8];
            } else {
                buf[i] = chars[rand() % 16];
            }
        }
        buf[36] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 36);
    }
    else if (strcmp(uname, "TIM") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "TIM expects one numeric argument"; return res;
        }
        int comp = (int)args[0].as.number;
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            if (comp == 0) val = (double)lt->tm_min;
            else if (comp == 1) val = (double)lt->tm_hour;
            else if (comp == 2) val = (double)(lt->tm_yday + 1);
            else if (comp == 3) val = (double)(lt->tm_year + 1900);
            else {
                err->code = 5; err->message = "Illegal function call: TIM component must be 0, 1, 2, or 3"; return res;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }
    else if (strcmp(uname, "ABS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ABS expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = fabs(args[0].as.number);
    }
    else if (strcmp(uname, "SIN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = sin(args[0].as.number);
    }
    else if (strcmp(uname, "COS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "COS expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = cos(args[0].as.number);
    }
    else if (strcmp(uname, "TAN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "TAN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = tan(args[0].as.number);
    }
    else if (strcmp(uname, "ATN") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "ATN expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = atan(args[0].as.number);
    }
    else if (strcmp(uname, "LOG") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOG expects one numeric argument"; return res;
        }
        if (args[0].as.number <= 0.0) {
            err->code = 5; err->message = "LOG of zero or negative number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = log(args[0].as.number);
    }
    else if (strcmp(uname, "EXP") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EXP expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = exp(args[0].as.number);
    }
    else if (strcmp(uname, "INT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INT expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = floor(args[0].as.number);
    }
    else if (strcmp(uname, "CINT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CINT expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = round(args[0].as.number);
    }
    else if (strcmp(uname, "CSNG") == 0 || strcmp(uname, "CDBL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CSNG/CDBL expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = args[0].as.number;
    }
    else if (strcmp(uname, "FIX") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "FIX expects one numeric argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (args[0].as.number >= 0.0) ? floor(args[0].as.number) : ceil(args[0].as.number);
    }
    else if (strcmp(uname, "RND") == 0) {
        res.type = VAL_NUMBER;
        double arg = 1.0;
        if (arg_count > 0 && args[0].type == VAL_NUMBER) {
            arg = args[0].as.number;
        }
        if (arg < 0.0) {
            srand((unsigned int)(-arg));
            double val = (double)rand() / (double)RAND_MAX;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
        } else if (arg == 0.0) {
            res.as.number = vm_get_last_rnd(vm);
        } else {
            double val = (double)rand() / (double)RAND_MAX;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
        }
    }
    else if (strcmp(uname, "PLAY") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PLAY function expects one numeric argument (e.g. PLAY(0))"; return res;
        }
        res.type = VAL_NUMBER;
#ifdef BPP_LITE_BUILD
        res.as.number = 0.0;
#else
        res.as.number = (double)vdev_music_queue_length();
#endif
    }
    else if (strcmp(uname, "TASK") == 0) {
        if (arg_count != 1) {
            err->code = 13; err->message = "TASK function expects one argument"; return res;
        }
        if (args[0].type == VAL_STRING) {
            const char *filename = str_data(args[0].as.string);
            int pid = task_spawn(vm_get_vdev(vm), filename);
            str_release(vm_get_str(vm), args[0].as.string);
            res.type = VAL_NUMBER;
            res.as.number = (double)pid;
        } else if (args[0].type == VAL_NUMBER) {
            int pid = (int)args[0].as.number;
            int status = task_get_status(pid);
            res.type = VAL_NUMBER;
            res.as.number = (double)status;
        } else {
            err->code = 13; err->message = "Type mismatch: TASK function expects filename string or numeric PID";
        }
    }
    else if (strcmp(uname, "LEN") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LEN expects one string argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)str_len(args[0].as.string);
        str_release(vm_get_str(vm), args[0].as.string);
    }
    else if (strcmp(uname, "ASC") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ASC expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        if (str_len(sr) == 0) {
            err->code = 5; err->message = "ASC of empty string";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)(unsigned char)str_data(sr)[0];
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "CHR$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "CHR$ expects one numeric argument"; return res;
        }
        int code = (int)args[0].as.number;
        if (code < 0 || code > 255) {
            err->code = 5; err->message = "Illegal function call in CHR$"; return res;
        }
        char ch = (char)code;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), &ch, 1);
    }
    else if (strcmp(uname, "VAL") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "VAL expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        res.type = VAL_NUMBER;
        res.as.number = strtod(str_data(sr), NULL);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "REVERSE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "REVERSE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *data = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        for (size_t idx = 0; idx < len; idx++) {
            buf[idx] = data[len - 1 - idx];
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "REPLACE$") == 0) {
        if (arg_count != 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "REPLACE$ expects three string arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;
        BppStringRef sr_repl = args[2].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        const char *repl = str_data(sr_repl);
        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);
        size_t repl_len = str_len(sr_repl);

        if (find_len == 0) {
            res.type = VAL_STRING;
            str_add_ref(sr_orig);
            res.as.string = sr_orig;
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len + count * repl_len;
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                str_release(vm_get_str(vm), sr_repl);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                memcpy(dst, repl, repl_len);
                dst += repl_len;
                src = next + find_len;
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }
        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
        str_release(vm_get_str(vm), sr_repl);
    }
    else if (strcmp(uname, "REMOVE$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "REMOVE$ expects two string arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);

        if (find_len == 0) {
            res.type = VAL_STRING;
            str_add_ref(sr_orig);
            res.as.string = sr_orig;
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len - count * find_len;
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                src = next + find_len;
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }
        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
    }
    else if (strcmp(uname, "REMOVE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "REMOVE function expects two numeric arguments";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        double val = args[0].as.number;
        double find = args[1].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (val == find) ? 0.0 : val;
    }
    else if (strcmp(uname, "HASH") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HASH expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *data = str_data(sr);
        size_t len = str_len(sr);
        unsigned int h = 5381;
        for (size_t idx = 0; idx < len; idx++) {
            h = ((h << 5) + h) + (unsigned char)data[idx];
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)h;
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "HEX$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "HEX$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)args[0].as.number;
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "%lX", uv);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "OCT$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "OCT$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)args[0].as.number;
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%lo", uv);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "BIN$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIN$ expects one numeric argument"; return res;
        }
        unsigned long uv = (unsigned long)args[0].as.number;
        char raw[68];
        int raw_bits = 0;
        if (uv == 0) {
            raw[raw_bits++] = '0';
        } else {
            while (uv > 0 && raw_bits < 64) {
                raw[raw_bits++] = (char)('0' + (int)(uv & 1));
                uv >>= 1;
            }
        }
        int num_bytes = (raw_bits + 7) / 8;
        int total_bits = num_bytes * 8;
        while (raw_bits < total_bits) {
            raw[raw_bits++] = '0';
        }
        char out[80];
        int o = 0;
        for (int idx = total_bits - 1; idx >= 0; idx--) {
            out[o++] = raw[idx];
            if (idx > 0 && (idx % 8) == 0) {
                out[o++] = ' ';
            }
        }
        out[o] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), out, o);
    }
    else if (strcmp(uname, "EDIT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "EDIT$ expects (string$, code)";
            for (int j = 0; j < arg_count; j++) {
                if (args[j].type == VAL_STRING && args[j].as.string) {
                    str_release(vm_get_str(vm), args[j].as.string);
                }
            }
            return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t src_len = str_len(sr);
        long code = (long)args[1].as.number;

        char *dest = (char *)calloc(1, src_len + 1);
        if (!dest) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }

        int dest_len = 0;
        int in_quotes = 0;
        int last_was_space = 0;

        for (size_t idx = 0; idx < src_len; idx++) {
            char c = src[idx];
            if (code & 1) c = (char)(c & 127);
            if ((code & 256) && c == '"') in_quotes = !in_quotes;
            if (in_quotes) {
                dest[dest_len++] = c;
                continue;
            }
            if ((code & 2) && (c == ' ' || c == '\t')) continue;
            if ((code & 4) && (c == '\r' || c == '\n' || c == '\f' || c == '\b' || c == 27 || c == '\0')) continue;
            if (code & 64) {
                if (c == '[') c = '(';
                else if (c == ']') c = ')';
            }
            if (code & 32) {
                if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            }
            if (code & 16) {
                if (c == ' ' || c == '\t') {
                    if (last_was_space) continue;
                    c = ' ';
                    last_was_space = 1;
                } else {
                    last_was_space = 0;
                }
            }
            dest[dest_len++] = c;
        }
        dest[dest_len] = '\0';

        int start_pos = 0;
        int end_pos = dest_len;
        if (code & 8) {
            while (start_pos < end_pos && (dest[start_pos] == ' ' || dest[start_pos] == '\t')) start_pos++;
        }
        if (code & 128) {
            while (end_pos > start_pos && (dest[end_pos - 1] == ' ' || dest[end_pos - 1] == '\t')) end_pos--;
        }

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), dest + start_pos, end_pos - start_pos);
        free(dest);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "NUM$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NUM$ expects one numeric argument"; return res;
        }
        double val = args[0].as.number;
        char tmp[64];
        format_double_clean(tmp, sizeof(tmp), val, false, false);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    }
    else if (strcmp(uname, "TCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "TCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        bool next_upper = true;
        for (size_t idx = 0; idx < len; idx++) {
            char c = src[idx];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                buf[idx] = c;
                next_upper = true;
            } else {
                if (next_upper) {
                    buf[idx] = (char)toupper((unsigned char)c);
                    next_upper = false;
                } else {
                    buf[idx] = (char)tolower((unsigned char)c);
                }
            }
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "ICASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ICASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        for (size_t idx = 0; idx < len; idx++) {
            char c = src[idx];
            if (c >= 'A' && c <= 'Z') {
                buf[idx] = (char)(c + 32);
            } else if (c >= 'a' && c <= 'z') {
                buf[idx] = (char)(c - 32);
            } else {
                buf[idx] = c;
            }
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "DIR$") == 0) {
        if (arg_count > 1) {
            err->code = 13; err->message = "DIR$ expects 0 or 1 argument"; return res;
        }
        char out_name[256];
        int found = 0;
        if (arg_count == 1) {
            if (args[0].type != VAL_STRING) {
                err->code = 13; err->message = "Type mismatch (expected string for DIR$)"; return res;
            }
            if (g_dir_search) platform_find_close(g_dir_search);
            g_dir_search = platform_find_first_file(str_data(args[0].as.string), out_name, sizeof(out_name));
            if (g_dir_search) found = 1;
            str_release(vm_get_str(vm), args[0].as.string);
        } else {
            if (g_dir_search) {
                found = platform_find_next_file(g_dir_search, out_name, sizeof(out_name));
                if (!found) {
                    platform_find_close(g_dir_search);
                    g_dir_search = NULL;
                }
            }
        }
        res.type = VAL_STRING;
        if (found) {
            res.as.string = str_create(vm_get_str(vm), out_name, strlen(out_name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
    else if (strcmp(uname, "ENVIRON$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "ENVIRON$ expects one string argument"; return res;
        }
        char *val = platform_getenv(str_data(args[0].as.string));
        res.type = VAL_STRING;
        if (val) {
            res.as.string = str_create(vm_get_str(vm), val, strlen(val));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
        str_release(vm_get_str(vm), args[0].as.string);
    }
    else if (strcmp(uname, "GETATTR") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "GETATTR expects one string argument"; return res;
        }
        int attr = platform_get_attributes(str_data(args[0].as.string));
        str_release(vm_get_str(vm), args[0].as.string);
        if (attr == -1) {
            err->code = 53; err->message = "File not found"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)attr;
    }
    else if (strcmp(uname, "STR$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "STR$ expects one numeric argument"; return res;
        }
        char buf[64];
        snprintf(buf, sizeof(buf), " %g", args[0].as.number);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "LEFT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "LEFT$ expects a string and a number"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int n = (int)args[1].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative length in LEFT$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        int len = (int)str_len(sr);
        if (n > len) n = len;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str_data(sr), n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "RIGHT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "RIGHT$ expects a string and a number"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int n = (int)args[1].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative length in RIGHT$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        int len = (int)str_len(sr);
        if (n > len) n = len;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), str_data(sr) + (len - n), n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "UCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "UCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        const char *src = str_data(sr);
        for (size_t i = 0; i < len; i++) {
            buf[i] = (char)toupper((unsigned char)src[i]);
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "LCASE$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LCASE$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        size_t len = str_len(sr);
        char *buf = (char *)calloc(1, len + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        const char *src = str_data(sr);
        for (size_t i = 0; i < len; i++) {
            buf[i] = (char)tolower((unsigned char)src[i]);
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        free(buf);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "LTRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "LTRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t start = 0;
        while (start < len && isspace((unsigned char)src[start])) {
            start++;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src + start, len - start);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "RTRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "RTRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t end = len;
        while (end > 0 && isspace((unsigned char)src[end - 1])) {
            end--;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src, end);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "TRIM$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "TRIM$ expects one string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *src = str_data(sr);
        size_t len = str_len(sr);
        size_t start = 0;
        while (start < len && isspace((unsigned char)src[start])) {
            start++;
        }
        size_t end = len;
        while (end > start && isspace((unsigned char)src[end - 1])) {
            end--;
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), src + start, end - start);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "SPACE$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SPACE$ expects one numeric argument"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative count in SPACE$"; return res;
        }
        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory"; return res;
        }
        memset(buf, ' ', n);
        buf[n] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, n);
        free(buf);
    }
    else if (strcmp(uname, "STRING$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "STRING$ expects a number and a character/string"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 0) {
            err->code = 5; err->message = "Negative count in STRING$"; return res;
        }
        char c = ' ';
        if (args[1].type == VAL_NUMBER) {
            c = (char)args[1].as.number;
        } else {
            BppStringRef sr = args[1].as.string;
            const char *s = str_data(sr);
            if (str_len(sr) > 0) c = s[0];
            str_release(vm_get_str(vm), sr);
        }
        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 14; err->message = "Out of memory"; return res;
        }
        memset(buf, c, n);
        buf[n] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, n);
        free(buf);
    }
    else if (strcmp(uname, "REPLACE$") == 0) {
        if (arg_count != 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "REPLACE$ expects three string arguments"; return res;
        }
        BppStringRef sr_orig = args[0].as.string;
        BppStringRef sr_find = args[1].as.string;
        BppStringRef sr_repl = args[2].as.string;

        const char *orig = str_data(sr_orig);
        const char *find = str_data(sr_find);
        const char *repl = str_data(sr_repl);

        size_t orig_len = str_len(sr_orig);
        size_t find_len = str_len(sr_find);
        size_t repl_len = str_len(sr_repl);

        res.type = VAL_STRING;

        if (find_len == 0) {
            res.as.string = str_create(vm_get_str(vm), orig, orig_len);
        } else {
            size_t count = 0;
            const char *p = orig;
            while ((p = strstr(p, find)) != NULL) {
                count++;
                p += find_len;
            }

            size_t new_len = orig_len + count * (repl_len - find_len);
            char *buf = (char *)calloc(1, new_len + 1);
            if (!buf) {
                err->code = 14; err->message = "Out of memory";
                str_release(vm_get_str(vm), sr_orig);
                str_release(vm_get_str(vm), sr_find);
                str_release(vm_get_str(vm), sr_repl);
                return res;
            }

            char *dst = buf;
            const char *src = orig;
            while (true) {
                const char *next = strstr(src, find);
                if (!next) {
                    strcpy(dst, src);
                    break;
                }
                size_t prefix_len = next - src;
                memcpy(dst, src, prefix_len);
                dst += prefix_len;
                memcpy(dst, repl, repl_len);
                dst += repl_len;
                src = next + find_len;
            }
            res.as.string = str_create(vm_get_str(vm), buf, new_len);
            free(buf);
        }

        str_release(vm_get_str(vm), sr_orig);
        str_release(vm_get_str(vm), sr_find);
        str_release(vm_get_str(vm), sr_repl);
    }
    else if (strcmp(uname, "HELP") == 0 || strcmp(uname, "HELP$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HELP expects a string argument"; return res;
        }
        BppStringRef sr = args[0].as.string;
        const char *target = str_data(sr);
        BppMetadataRegistry *reg = vm_get_metadata(vm);
        const char *doc = metadata_get_docstring(reg, target);
        
        res.type = VAL_STRING;
        if (doc) {
            res.as.string = str_create(vm_get_str(vm), doc, strlen(doc));
        } else {
            const BppMetadataBlock *block = metadata_get_block(reg, "KEYWORD", target);
            if (!block) block = metadata_get_block(reg, "SCOPE", target);
            if (!block) block = metadata_get_block(reg, "ALIAS", target);
            if (!block) block = metadata_get_block(reg, "OPTION", target);

            if (block) {
                char temp_buf[2048];
                snprintf(temp_buf, sizeof(temp_buf), "[%s BLOCK: %s]\nDocstring: %s\nBody:\n%s", 
                         block->block_type, block->target_name, block->docstring, block->body);
                res.as.string = str_create(vm_get_str(vm), temp_buf, strlen(temp_buf));
            } else {
                const FunctionEntry *entry = funcreg_find_by_name(target);
                if (entry) {
                    char temp_buf[512];
                    snprintf(temp_buf, sizeof(temp_buf), "Function: %s\nHelp:     %s", entry->name, entry->help_text);
                    res.as.string = str_create(vm_get_str(vm), temp_buf, strlen(temp_buf));
                } else {
                    res.as.string = str_create(vm_get_str(vm), "", 0);
                }
            }
        }
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "MID$") == 0) {
        if ((arg_count != 2 && arg_count != 3) || args[0].type != VAL_STRING || 
            args[1].type == VAL_STRING || (arg_count == 3 && args[2].type == VAL_STRING)) {
            err->code = 13; err->message = "MID$ expects MID$(s$, start[, len])"; return res;
        }
        BppStringRef sr = args[0].as.string;
        int start = (int)args[1].as.number;
        int n = (arg_count == 3) ? (int)args[2].as.number : -1;
        if (start <= 0 || (arg_count == 3 && n < 0)) {
            err->code = 5; err->message = "Invalid bounds in MID$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        res.type = VAL_STRING;
        res.as.string = str_mid(vm_get_str(vm), sr, start, n);
        str_release(vm_get_str(vm), sr);
    }
    else if (strcmp(uname, "INSTR") == 0) {
        int start = 1;
        BppStringRef s1 = NULL;
        BppStringRef s2 = NULL;

        if (arg_count == 2) {
            if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
                err->code = 13; err->message = "INSTR expects string arguments"; return res;
            }
            s1 = args[0].as.string;
            s2 = args[1].as.string;
        } else if (arg_count == 3) {
            if (args[0].type == VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
                err->code = 13; err->message = "INSTR expects start, s1$, s2$"; return res;
            }
            start = (int)args[0].as.number;
            s1 = args[1].as.string;
            s2 = args[2].as.string;
        } else {
            err->code = 13; err->message = "Invalid argument count for INSTR"; return res;
        }

        int len1 = (int)str_len(s1);
        int len2 = (int)str_len(s2);
        double found_idx = 0.0;

        if (start <= 0 || start > len1) {
            found_idx = 0.0;
        } else if (len2 == 0) {
            found_idx = (double)start;
        } else {
            const char *str1 = str_data(s1);
            const char *str2 = str_data(s2);
            const char *ptr = strstr(str1 + (start - 1), str2);
            if (ptr) {
                found_idx = (double)(ptr - str1 + 1);
            }
        }

        res.type = VAL_NUMBER;
        res.as.number = found_idx;
        
        if (s1) str_release(vm_get_str(vm), s1);
        if (s2) str_release(vm_get_str(vm), s2);
    }
    else if (strcmp(uname, "EOF") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EOF expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = file_eof(vm_get_file(vm), ch) ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "LOF") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOF expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_lof(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "LOC") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "LOC expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "SEEK") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SEEK expects numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "FREEFILE") == 0) {
        int free_ch = 1;
        for (int c = 1; c <= 16; ++c) {
            if (!file_is_open(vm_get_file(vm), c)) {
                free_ch = c;
                break;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)free_ch;
    }
    else if (strcmp(uname, "FILEATTR") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "FILEATTR expects two numeric arguments"; return res;
        }
        int ch = (int)args[0].as.number;
        int attr = (int)args[1].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        res.type = VAL_NUMBER;
        if (attr == 1) {
            BppFileMode fmode = file_get_mode(vm_get_file(vm), ch);
            switch (fmode) {
                case FILE_MODE_INPUT: res.as.number = 1; break;
                case FILE_MODE_OUTPUT: res.as.number = 2; break;
                case FILE_MODE_RANDOM: res.as.number = 4; break;
                case FILE_MODE_APPEND: res.as.number = 8; break;
                case FILE_MODE_BINARY: res.as.number = 32; break;
                default: res.as.number = 0; break;
            }
        } else if (attr == 2) {
            res.as.number = (double)ch; /* Map to internal VDev handle ID */
        } else {
            err->code = 5; err->message = "Invalid FILEATTR attribute"; return res;
        }
    }
    else if (strcmp(uname, "INPUT$") == 0) {
        if (arg_count < 1 || arg_count > 2) {
            err->code = 13; err->message = "INPUT$ expects 1 or 2 arguments"; return res;
        }
        if (args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INPUT$ expects numeric count"; return res;
        }
        int n = (int)args[0].as.number;
        if (n < 1) {
            err->code = 5; err->message = "Count must be positive"; return res;
        }

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }

        if (arg_count == 1) {
            for (int i = 0; i < n; ++i) {
                int c = platform_getch();
                buf[i] = (char)c;
            }
            buf[n] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, n);
        } else {
            if (args[1].type == VAL_STRING) {
                free(buf);
                err->code = 13; err->message = "Channel must be numeric"; return res;
            }
            int ch = (int)args[1].as.number;
            if (!file_is_open(vm_get_file(vm), ch)) {
                free(buf);
                err->code = 52; err->message = "Bad file number"; return res;
            }
            int read_res = file_read(vm_get_file(vm), ch, buf, n);
            if (read_res < 0) read_res = 0;
            buf[read_res] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), buf, read_res);
        }
        free(buf);
    }
    else if (strcmp(uname, "SCREEN") == 0) {
        if (arg_count < 2 || arg_count > 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SCREEN expects SCREEN(row, col [, color_flag])"; return res;
        }
        int row = (int)args[0].as.number;
        int col = (int)args[1].as.number;
        int color_flag = 0;
        if (arg_count == 3) {
            if (args[2].type == VAL_STRING) {
                err->code = 13; err->message = "color_flag must be numeric"; return res;
            }
            color_flag = (int)args[2].as.number;
        }
        
        int r = row - 1;
        int c = col - 1;
        int val = -1;

#ifndef NO_SDL2
        if (vdev_get(vm_get_vdev(vm), "CON:") != NULL) {
            if (color_flag == 0) {
                val = gfx_get_char_at(r, c);
            } else {
                val = gfx_get_attr_at(r, c);
            }
        }
#endif

        if (val <= 0) {
            if (color_flag == 0) {
                val = platform_screen_get_char(r, c);
            } else {
                val = platform_screen_get_attr(r, c);
            }
        }

        if (val <= 0) {
            VConContext *vcon = vm_get_vcon(vm);
            if (vcon) {
                int active_idx = vcon_get_active_index(vcon);
                if (color_flag == 0) {
                    val = vcon_get_char_at(vcon, active_idx, r, c);
                } else {
                    val = vcon_get_attr_at(vcon, active_idx, r, c);
                }
            } else {
                val = (color_flag == 0) ? 32 : 7;
            }
        }

        res.type = VAL_NUMBER;
        res.as.number = (double)val;
    }
    else if (strcmp(uname, "IOCTL$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "IOCTL$ expects one numeric channel"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        
        VDev *dev = file_get_vdev(vm_get_file(vm), ch);
        char buf[256] = "";
        if (dev && dev->dev_ioctl) {
            int io_res = dev->dev_ioctl(dev, 0x7FFF, buf);
            if (io_res < 0) {
                err->code = 57; err->message = "Device I/O error"; return res;
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "TXNSTATUS") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "TXNSTATUS expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)file_txn_status(vm_get_file(vm));
    }
    else if (strcmp(uname, "BIOSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIOSTATUS expects BIOSTATUS(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        int status = 0;
        FileContext *fctx = vm_get_file(vm);
        if (file_is_open(fctx, ch)) {
            status |= 1;
            FILE *fp = file_get_handle(fctx, ch);
            if (fp) {
                status |= 2;
                status |= 4;
                status |= 8;
                status |= 16;
                status |= 32;
            } else if (file_get_vdev(fctx, ch) != NULL) {
                status |= 64;
            }
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)status;
    }
    else if (strcmp(uname, "BIOSIZE") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "BIOSIZE expects BIOSIZE(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)file_lof(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "BIOCHECKSUM") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "BIOCHECKSUM expects BIOCHECKSUM(data$)"; return res;
        }
        BppStringRef data_ref = args[0].as.string;
        const unsigned char *data = data_ref ? (const unsigned char *)str_data(data_ref) : (const unsigned char *)"";
        size_t len = data_ref ? str_len(data_ref) : 0;
        res.type = VAL_NUMBER;
        res.as.number = (double)compute_crc16(data, len);
    }
    else if (strcmp(uname, "BIOCOMPARE") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "BIOCOMPARE expects BIOCOMPARE(chan, pos, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        BppStringRef data_ref = args[2].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(fctx, ch, pos + 1);
        unsigned char *buf = (unsigned char *)calloc(1, len > 0 ? len : 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        int read_bytes = file_read(fctx, ch, buf, len);
        if (read_bytes < 0) read_bytes = 0;

        int diff_pos = 0;
        for (int i = 0; i < len; ++i) {
            unsigned char b_val = (i < read_bytes) ? buf[i] : 0;
            if (b_val != (unsigned char)data[i]) {
                diff_pos = i + 1;
                break;
            }
        }
        free(buf);

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_NUMBER;
        res.as.number = (double)diff_pos;
    }
    else if (strcmp(uname, "SIOREAD$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SIOREAD$ expects SIOREAD$(chan, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        int n = (int)args[1].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n < 1 || n > 1024) {
            err->code = 5; err->message = "Invalid read length"; return res;
        }

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }

        int bytes_read = file_read(vm_get_file(vm), ch, buf, n);
        if (bytes_read < 0) bytes_read = 0;
        buf[bytes_read] = '\0';
        
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, bytes_read);
        free(buf);
    }
    else if (strcmp(uname, "SIOREADLN$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOREADLN$ expects SIOREADLN$(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        char buf[1024] = "";
        char *gets_res = file_gets(vm_get_file(vm), ch, buf, sizeof(buf));
        if (!gets_res) {
            buf[0] = '\0';
        } else {
            size_t len = strlen(buf);
            while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
                buf[len - 1] = '\0';
                len--;
            }
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    }
    else if (strcmp(uname, "SIOWRITE") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "SIOWRITE expects SIOWRITE(chan, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        BppStringRef data_ref = args[1].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        if (file_txn_status(fctx) > 0) {
            FILE *fp = file_get_handle(fctx, ch);
            if (fp) {
                long pos = ftell(fp);
                unsigned char *orig = (unsigned char *)calloc(1, len);
                if (orig) {
                    int orig_read = (int)fread(orig, 1, len, fp);
                    if (orig_read < len) {
                        memset(orig + orig_read, 0, len - orig_read);
                    }
                    file_txn_log_write(fctx, ch, pos, orig, len);
                    free(orig);
                }
                fseek(fp, pos, SEEK_SET);
            }
        }

        int written = file_write(fctx, ch, data, len);
        if (written < 0) written = 0;
        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "SIOSEEK") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = 13; err->message = "SIOSEEK expects SIOSEEK(chan, pos)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;

        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        file_seek(vm_get_file(vm), ch, pos + 1);
        res.type = VAL_NUMBER;
        res.as.number = (double)file_loc(vm_get_file(vm), ch);
    }
    else if (strcmp(uname, "SIOFLUSH") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOFLUSH expects SIOFLUSH(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        int fres = file_flush(vm_get_file(vm), ch);
        res.type = VAL_NUMBER;
        res.as.number = (double)fres;
    }
    else if (strcmp(uname, "SIOSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOSTATUS expects SIOSTATUS(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        int status = 0;
        FileContext *fctx = vm_get_file(vm);
        if (file_is_open(fctx, ch)) {
            status |= 1;
            if (file_eof(fctx, ch)) status |= 2;
            if (file_get_vdev(fctx, ch) != NULL) status |= 8;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)status;
    }
    else if (strcmp(uname, "SIOAVAIL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "SIOAVAIL expects SIOAVAIL(chan)"; return res;
        }
        int ch = (int)args[0].as.number;
        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        long lof = file_lof(vm_get_file(vm), ch);
        long loc = file_loc(vm_get_file(vm), ch);
        res.type = VAL_NUMBER;
        res.as.number = (double)(lof - loc >= 0 ? lof - loc : 0);
    }
    else if (strcmp(uname, "BIOREAD$") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
            err->code = 13; err->message = "BIOREAD$ expects BIOREAD$(chan, pos, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        int n = (int)args[2].as.number;

        if (!file_is_open(vm_get_file(vm), ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n < 1 || n > 1024) {
            err->code = 5; err->message = "Invalid read length"; return res;
        }

        FILE *fp = file_get_handle(vm_get_file(vm), ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(vm_get_file(vm), ch, pos + 1);

        char *buf = (char *)calloc(1, n + 1);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        int bytes_read = file_read(vm_get_file(vm), ch, buf, n);
        if (bytes_read < 0) bytes_read = 0;
        buf[bytes_read] = '\0';

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, bytes_read);
        free(buf);
    }
    else if (strcmp(uname, "BIOWRITE") == 0) {
        if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13; err->message = "BIOWRITE expects BIOWRITE(chan, pos, data$)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        BppStringRef data_ref = args[2].as.string;
        const char *data = data_ref ? str_data(data_ref) : "";
        int len = data_ref ? (int)str_len(data_ref) : 0;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        file_seek(fctx, ch, pos + 1);

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, len);
            if (orig) {
                int orig_read = (int)fread(orig, 1, len, fp);
                if (orig_read < len) {
                    memset(orig + orig_read, 0, len - orig_read);
                }
                file_txn_log_write(fctx, ch, pos, orig, len);
                free(orig);
            }
            fseek(fp, pos, SEEK_SET);
        }

        int written = file_write(fctx, ch, data, len);
        if (written < 0) written = 0;

        if (fp) fseek(fp, saved_pos, SEEK_SET);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "BIOCOPY") == 0) {
        if (arg_count != 4 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13; err->message = "BIOCOPY expects BIOCOPY(chan, src_pos, dest_pos, n)"; return res;
        }
        int ch = (int)args[0].as.number;
        long src_pos = (long)args[1].as.number;
        long dest_pos = (long)args[2].as.number;
        int n = (int)args[3].as.number;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n <= 0 || n > 65536) {
            err->code = 5; err->message = "Invalid copy length"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        unsigned char *buf = (unsigned char *)calloc(1, n);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        fseek(fp, src_pos, SEEK_SET);
        int read_bytes = (int)fread(buf, 1, n, fp);
        if (read_bytes < n) {
            memset(buf + read_bytes, 0, n - read_bytes);
        }

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, n);
            if (orig) {
                fseek(fp, dest_pos, SEEK_SET);
                int orig_read = (int)fread(orig, 1, n, fp);
                if (orig_read < n) {
                    memset(orig + orig_read, 0, n - orig_read);
                }
                file_txn_log_write(fctx, ch, dest_pos, orig, n);
                free(orig);
            }
        }

        fseek(fp, dest_pos, SEEK_SET);
        int written = (int)fwrite(buf, 1, n, fp);
        fflush(fp);

        if (fp) fseek(fp, saved_pos, SEEK_SET);
        free(buf);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "BIOFILL") == 0) {
        if (arg_count != 4 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13; err->message = "BIOFILL expects BIOFILL(chan, pos, n, byte_val)"; return res;
        }
        int ch = (int)args[0].as.number;
        long pos = (long)args[1].as.number;
        int n = (int)args[2].as.number;
        unsigned char val = (unsigned char)args[3].as.number;

        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, ch)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        if (n <= 0 || n > 65536) {
            err->code = 5; err->message = "Invalid fill length"; return res;
        }

        FILE *fp = file_get_handle(fctx, ch);
        long saved_pos = fp ? ftell(fp) : 0;

        if (file_txn_status(fctx) > 0 && fp) {
            unsigned char *orig = (unsigned char *)calloc(1, n);
            if (orig) {
                fseek(fp, pos, SEEK_SET);
                int orig_read = (int)fread(orig, 1, n, fp);
                if (orig_read < n) {
                    memset(orig + orig_read, 0, n - orig_read);
                }
                file_txn_log_write(fctx, ch, pos, orig, n);
                free(orig);
            }
        }

        unsigned char *buf = (unsigned char *)calloc(1, n);
        if (!buf) {
            err->code = 7; err->message = "Out of memory"; return res;
        }
        memset(buf, val, n);
        fseek(fp, pos, SEEK_SET);
        int written = (int)fwrite(buf, 1, n, fp);
        fflush(fp);

        if (fp) fseek(fp, saved_pos, SEEK_SET);
        free(buf);

        res.type = VAL_NUMBER;
        res.as.number = (double)written;
    }
    else if (strcmp(uname, "MKI$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKI$ expects numeric argument"; return res;
        }
        short val = (short)args[0].as.number;
        char buf[2];
        memcpy(buf, &val, 2);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 2);
    }
    else if (strcmp(uname, "MKS$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKS$ expects numeric argument"; return res;
        }
        float val = (float)args[0].as.number;
        char buf[4];
        memcpy(buf, &val, 4);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 4);
    }
    else if (strcmp(uname, "MKD$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "MKD$ expects numeric argument"; return res;
        }
        double val = args[0].as.number;
        char buf[8];
        memcpy(buf, &val, 8);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 8);
    }
    else if (strcmp(uname, "CVI") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVI expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 2) {
            short val;
            memcpy(&val, str_data(args[0].as.string), 2);
            res.type = VAL_NUMBER;
            res.as.number = (double)val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVI string too short"; return res;
        }
    }
    else if (strcmp(uname, "CVS") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVS expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 4) {
            float val;
            memcpy(&val, str_data(args[0].as.string), 4);
            res.type = VAL_NUMBER;
            res.as.number = (double)val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVS string too short"; return res;
        }
    }
    else if (strcmp(uname, "CVD") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "CVD expects string argument"; return res;
        }
        if (args[0].as.string && str_len(args[0].as.string) >= 8) {
        double val;
            memcpy(&val, str_data(args[0].as.string), 8);
            res.type = VAL_NUMBER;
            res.as.number = val;
        } else {
            err->code = 5; err->message = "Illegal function call: CVD string too short"; return res;
        }
    }
    else if (strncmp(uname, "USR", 3) == 0) {
        int idx = 0;
        if (strlen(uname) == 4 && isdigit((unsigned char)uname[3])) {
            idx = uname[3] - '0';
        }
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "USR expects one numeric argument"; return res;
        }
        uintptr_t ptr = vm_get_usr_ptr(vm, idx);
        res.type = VAL_NUMBER;
        if (ptr != 0) {
            typedef double (*UsrFunc)(double);
            if (ptr > 0x10000) {
                UsrFunc func = (UsrFunc)(intptr_t)ptr;
                res.as.number = func(args[0].as.number);
            } else {
                res.as.number = (double)ptr + args[0].as.number;
            }
        } else {
            res.as.number = args[0].as.number;
        }
    }
    else if (strcmp(uname, "ERDEV") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "ERDEV$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "DSK1    ", 8);
    }
    else if (strcmp(uname, "EXTERR") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "EXTERR expects one numeric argument (0 to 3)"; return res;
        }
        int mode = (int)args[0].as.number;
        res.type = VAL_NUMBER;
        if (mode == 0) res.as.number = 0.0;
        else if (mode == 1) res.as.number = 1.0;
        else if (mode == 2) res.as.number = 1.0;
        else if (mode == 3) res.as.number = 1.0;
        else res.as.number = 0.0;
    }
    else if (strcmp(uname, "HASH$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "HASH$ expects (algo$, data$)"; return res;
        }
        const char *algo = str_data(args[0].as.string);
        const char *data = str_data(args[1].as.string);
        char digest[128];
        bpp_hash_string(algo, data, digest, sizeof(digest));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), digest, strlen(digest));
    }
    else if (strcmp(uname, "SALT$") == 0) {
        if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "SALT$ expects (data$, salt$)"; return res;
        }
        const char *data = str_data(args[0].as.string);
        const char *salt = str_data(args[1].as.string);
        char combined[1024];
        snprintf(combined, sizeof(combined), "%s:%s", salt, data);
        char digest[128];
        bpp_hash_string("SHA256", combined, digest, sizeof(digest));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), digest, strlen(digest));
    }
    else if (strcmp(uname, "AUDITCRACK") == 0 || strcmp(uname, "AUDITCRACK$") == 0) {
        if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13; err->message = "AUDITCRACK expects (hash$, wordlist_file$ [, algo$])"; return res;
        }
        const char *target_hash = str_data(args[0].as.string);
        const char *wordlist_path = str_data(args[1].as.string);
        const char *algo = (arg_count >= 3 && args[2].type == VAL_STRING) ? str_data(args[2].as.string) : "MD5";
        
        char found_word[256] = {0};
        FILE *fp = fopen(wordlist_path, "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                size_t len = strlen(line);
                while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) line[--len] = '\0';
                char digest[128];
                bpp_hash_string(algo, line, digest, sizeof(digest));
                if (strcasecmp(digest, target_hash) == 0) {
                    snprintf(found_word, sizeof(found_word), "%s", line);
                    break;
                }
            }
            fclose(fp);
        }
        if (uname[strlen(uname)-1] == '$') {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), found_word, strlen(found_word));
        } else {
            res.type = VAL_NUMBER;
            res.as.number = (found_word[0] != '\0') ? 1.0 : 0.0;
        }
    }
    else if (strcmp(uname, "SANDBOXAUDIT") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "VMCHECK") == 0) {
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
    }
    else if (strcmp(uname, "NETHOST$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "localhost", 9);
    }
    else if (strcmp(uname, "NETIP$") == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "127.0.0.1", 9);
    }
    else if (strcmp(uname, "DEVICECOUNT") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "DEVICECOUNT expects no arguments"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vdev_count(vm_get_vdev(vm));
    }
    else if (strcmp(uname, "DEVICE$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "DEVICE$ expects one numeric argument"; return res;
        }
        int idx = (int)args[0].as.number;
        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range"; return res;
        }
        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        if (dev) {
            res.as.string = str_create(vm_get_str(vm), dev->name, (int)strlen(dev->name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
    else if (strcmp(uname, "DEVICECLASS$") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "DEVICECLASS$ expects one numeric argument"; return res;
        }
        int idx = (int)args[0].as.number;
        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range"; return res;
        }
        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        if (dev) {
            const char *cls_name = vdev_class_name(dev->dev_class);
            res.as.string = str_create(vm_get_str(vm), cls_name, (int)strlen(cls_name));
        } else {
            res.as.string = str_create(vm_get_str(vm), "UNKNOWN", 7);
        }
    }
    else if (strcmp(uname, "DEVICEINFO$") == 0) {
        if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type != VAL_STRING) {
            if (arg_count == 2) {
                if (args[0].type == VAL_STRING) str_release(vm_get_str(vm), args[0].as.string);
                if (args[1].type == VAL_STRING) str_release(vm_get_str(vm), args[1].as.string);
            } else if (arg_count == 1 && args[0].type == VAL_STRING) {
                str_release(vm_get_str(vm), args[0].as.string);
            }
            err->code = 13; err->message = "DEVICEINFO$ expects a numeric index and a string key"; return res;
        }
        int idx = (int)args[0].as.number;
        BppStringRef key_sr = args[1].as.string;
        const char *key = str_data(key_sr);

        int count = vdev_count(vm_get_vdev(vm));
        if (idx < 1 || idx > count) {
            err->code = 5; err->message = "Device index out of range";
            str_release(vm_get_str(vm), key_sr);
            return res;
        }

        VDev *dev = vdev_get_by_index(vm_get_vdev(vm), idx - 1);
        res.type = VAL_STRING;
        const char *val_str = "";
        if (dev) {
            val_str = vdev_info(dev, key);
        }
        if (!val_str) val_str = "";
        res.as.string = str_create(vm_get_str(vm), val_str, (int)strlen(val_str));
        str_release(vm_get_str(vm), key_sr);
    }
    else if (strcmp(uname, "POLL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "POLL expects one numeric channel argument"; return res;
        }
        int channel = (int)args[0].as.number;
        FileContext *fctx = vm_get_file(vm);
        if (!file_is_open(fctx, channel)) {
            err->code = 52; err->message = "Bad file number"; return res;
        }
        VDev *dev = file_get_vdev(fctx, channel);
        res.type = VAL_NUMBER;
        if (dev) {
            res.as.number = (vdev_poll(dev) > 0) ? 1.0 : 0.0;
        } else {
            res.as.number = 1.0;
        }
    }
#if BPP_SUPPORT_NET
    else if (strcmp(uname, "NSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NSTATUS expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vnet_status(vm_get_vnet(vm), (int)args[0].as.number);
    }
    else if (strcmp(uname, "NCONNECTED") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NCONNECTED expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = vnet_connected(vm_get_vnet(vm), (int)args[0].as.number) ? -1.0 : 0.0;
    }
    else if (strcmp(uname, "NHTTPSTATUS") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "NHTTPSTATUS expects one numeric channel argument"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vnet_http_status(vm_get_vnet(vm), (int)args[0].as.number);
    }
    else if (strcmp(uname, "HTTP_GET$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "HTTP_GET$ expects one string URL argument"; return res;
        }
        const char *url = str_data(args[0].as.string);
        VDev n_dev = fujinet_create_n_dev(vm);
        if (n_dev.dev_open(&n_dev, url, 1) < 0) {
            err->code = 57; err->message = "HTTP_GET$ failed to open URL";
            return res;
        }
        char *body_buf = (char *)calloc(1, 4096);
        if (!body_buf) {
            n_dev.dev_close(&n_dev);
            err->code = 7; err->message = "Out of memory";
            return res;
        }
        size_t body_cap = 4096;
        size_t body_len = 0;
            char chunk[1024];
            while (true) {
                int read_bytes = n_dev.dev_read(&n_dev, chunk, sizeof(chunk));
                if (read_bytes <= 0) {
                    break;
                }
                if (body_len + read_bytes >= body_cap) {
                    body_cap *= 2;
                    char *new_buf = realloc(body_buf, body_cap);
                    if (!new_buf) {
                        break;
                    }
                    body_buf = new_buf;
                }
                memcpy(body_buf + body_len, chunk, read_bytes);
                body_len += read_bytes;
            }
        n_dev.dev_close(&n_dev);
        res.type = VAL_STRING;
        if (body_buf) {
            res.as.string = str_create(vm_get_str(vm), body_buf, body_len);
            free(body_buf);
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
#endif
    else if (strcmp(uname, "INP") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "INP expects one numeric port argument"; return res;
        }
        /* Sandbox Check */
        if (security_check(SECOP_SYSTEM, 0) != 0) {
            err->code = 70; err->message = "Permission denied: Port I/O is restricted"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)vdev_bus_in((int)args[0].as.number);
    }
#if BPP_SUPPORT_BIOS
    else if (strcmp(uname, "MEMMAP$") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MEMMAP$ expects no arguments"; return res;
        }
        MockBiosModel model = vdev_bus_get_model();
        const char *m_name = mock_bios_model_to_string(model);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), m_name, strlen(m_name));
    }
#endif
    else if (strcmp(uname, "PEEK") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13; err->message = "PEEK expects one numeric address argument"; return res;
        }
        /* Sandbox Check */
        if (security_check(SECOP_MEM_READ, 0) != 0) {
            err->code = 70; err->message = "Permission denied: PEEK is restricted"; return res;
        }
        uint8_t val = 0;
#ifndef BPP_LITE_BUILD
        if (vmem_peek(vm_get_vmem(vm), (uint16_t)args[0].as.number, &val) == 0) {
#endif
            bool intercepted = false;
            val = vdev_bus_peek((unsigned long)args[0].as.number, &intercepted);
#ifndef BPP_LITE_BUILD
        }
#endif
        res.type = VAL_NUMBER;
        res.as.number = (double)val;
    }
    else if (strcmp(uname, "MAP") == 0 || strcmp(uname, "MAP_NEW") == 0) {
        if (arg_count != 0) {
            err->code = 13; err->message = "MAP expects no arguments"; return res;
        }
        BppMap *m = bpp_map_create();
        if (!m) {
            err->code = 14; err->message = "Out of memory creating map"; return res;
        }
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "MAP_SET") == 0) {
        if (arg_count != 3) {
            err->code = 13; err->message = "MAP_SET expects three arguments (map, key$, value)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_SET must be a MAP";
            if (args[1].type == VAL_STRING) str_release(vm_get_str(vm), args[1].as.string);
            if (args[2].type == VAL_STRING) str_release(vm_get_str(vm), args[2].as.string);
            if (args[2].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[2].as.map);
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_SET must be a string key";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            if (args[2].type == VAL_STRING) str_release(vm_get_str(vm), args[2].as.string);
            if (args[2].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[2].as.map);
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool set_ok = bpp_map_set(vm_get_str(vm), args[0].as.map, k, args[2]);
        if (!set_ok) {
            err->code = 14; err->message = "Failed to set map key";
        }
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        str_release(vm_get_str(vm), args[1].as.string);
        if (args[2].type == VAL_STRING) str_release(vm_get_str(vm), args[2].as.string);
        if (args[2].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[2].as.map);

        res.type = VAL_NUMBER;
        res.as.number = set_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "MAP_GET") == 0 || strcmp(uname, "MAP_GET$") == 0) {
        bool expect_str = (strcmp(uname, "MAP_GET$") == 0);
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_GET expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_GET must be a MAP";
            if (args[1].type == VAL_STRING) str_release(vm_get_str(vm), args[1].as.string);
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_GET must be a string key";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            return res;
        }
        const char *k = str_data(args[1].as.string);
        BValue val;
        if (bpp_map_get(args[0].as.map, k, &val)) {
            res = val;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            } else if (res.type == VAL_MAP && res.as.map) {
                bpp_map_add_ref(res.as.map);
            }
        } else {
            if (expect_str) {
                res.type = VAL_STRING;
                res.as.string = str_create(vm_get_str(vm), "", 0);
            } else {
                res.type = VAL_NUMBER;
                res.as.number = 0.0;
            }
        }
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        str_release(vm_get_str(vm), args[1].as.string);

        if (expect_str && res.type != VAL_STRING) {
            char coerce_buf[64];
            if (res.type == VAL_NUMBER) {
                snprintf(coerce_buf, sizeof(coerce_buf), "%g", res.as.number);
            } else {
                strcpy(coerce_buf, "");
            }
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), coerce_buf, strlen(coerce_buf));
        }
    }
    else if (strcmp(uname, "MAP_REMOVE") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_REMOVE expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_REMOVE must be a MAP";
            if (args[1].type == VAL_STRING) str_release(vm_get_str(vm), args[1].as.string);
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_REMOVE must be a string key";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool rem_ok = bpp_map_remove(vm_get_str(vm), args[0].as.map, k);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        str_release(vm_get_str(vm), args[1].as.string);
        res.type = VAL_NUMBER;
        res.as.number = rem_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "MAP_COUNT") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "MAP_COUNT expects one MAP argument";
            if (arg_count == 1 && args[0].type == VAL_STRING) str_release(vm_get_str(vm), args[0].as.string);
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)bpp_map_count(args[0].as.map);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
    }
    else if (strcmp(uname, "MAP_KEY$") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_KEY$ expects two arguments (map, index)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_KEY$ must be a MAP";
            return res;
        }
        int idx = (int)args[1].as.number;
        const char *k = bpp_map_key(args[0].as.map, idx);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), k ? k : "", k ? strlen(k) : 0);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
    }
    else if (strcmp(uname, "MAP_HAS") == 0) {
        if (arg_count != 2) {
            err->code = 13; err->message = "MAP_HAS expects two arguments (map, key$)"; return res;
        }
        if (args[0].type != VAL_MAP) {
            err->code = 13; err->message = "First argument to MAP_HAS must be a MAP";
            if (args[1].type == VAL_STRING) str_release(vm_get_str(vm), args[1].as.string);
            return res;
        }
        if (args[1].type != VAL_STRING) {
            err->code = 13; err->message = "Second argument to MAP_HAS must be a string key";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            return res;
        }
        const char *k = str_data(args[1].as.string);
        bool has_ok = bpp_map_has(args[0].as.map, k);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        str_release(vm_get_str(vm), args[1].as.string);
        res.type = VAL_NUMBER;
        res.as.number = has_ok ? 1.0 : 0.0;
    }
    else if (strcmp(uname, "JSON_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "JSON_PARSE expects one string argument";
            return res;
        }
        const char *json = str_data(args[0].as.string);
        BppMap *m = bpp_map_parse_json(vm_get_str(vm), json);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "JSON_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "JSON_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *json = bpp_map_stringify_json(args[0].as.map);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), json ? json : "", json ? strlen(json) : 0);
        free(json);
    }
    else if (strcmp(uname, "XML_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "XML_PARSE expects one string argument";
            return res;
        }
        const char *xml = str_data(args[0].as.string);
        BppMap *m = bpp_map_parse_xml(vm_get_str(vm), xml);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "XML_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "XML_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *xml = bpp_map_stringify_xml(args[0].as.map);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), xml ? xml : "", xml ? strlen(xml) : 0);
        free(xml);
    }
    else if (strcmp(uname, "YAML_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "YAML_PARSE expects one string argument";
            return res;
        }
        const char *yaml = str_data(args[0].as.string);
        BppMap *m = bpp_map_parse_yaml(vm_get_str(vm), yaml);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "YAML_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "YAML_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *yaml = bpp_map_stringify_yaml(args[0].as.map);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), yaml ? yaml : "", yaml ? strlen(yaml) : 0);
        free(yaml);
    }
    else if (strcmp(uname, "INI_PARSE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_STRING) {
            err->code = 13; err->message = "INI_PARSE expects one string argument";
            return res;
        }
        const char *ini = str_data(args[0].as.string);
        BppMap *m = bpp_map_parse_ini(vm_get_str(vm), ini);
        str_release(vm_get_str(vm), args[0].as.string);
        res.type = VAL_MAP;
        res.as.map = m;
    }
    else if (strcmp(uname, "INI_STRINGIFY$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "INI_STRINGIFY$ expects one MAP argument";
            return res;
        }
        char *ini = bpp_map_stringify_ini(args[0].as.map);
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), ini ? ini : "", ini ? strlen(ini) : 0);
        free(ini);
    }
    else if (strcmp(uname, "DIALECT_LOAD") == 0) {
        if (arg_count != 1 && arg_count != 2) {
            err->code = 13; err->message = "DIALECT_LOAD expects 1 or 2 arguments (filepath$ [, format$])";
            return res;
        }
        if (args[0].type != VAL_STRING) {
            err->code = 13; err->message = "DIALECT_LOAD expects string representation of dialect spec or file path";
            if (arg_count == 2 && args[1].type == VAL_STRING) {
                str_release(vm_get_str(vm), args[1].as.string);
            }
            return res;
        }
        if (arg_count == 2 && args[1].type != VAL_STRING) {
            str_release(vm_get_str(vm), args[0].as.string);
            err->code = 13; err->message = "DIALECT_LOAD expects string format (JSON, XML, YAML, or INI) as the second argument";
            return res;
        }
        
        const char *spec_str = str_data(args[0].as.string);
        char *file_content = NULL;
        const char *format = NULL;
        
        if (arg_count == 2) {
            format = str_data(args[1].as.string);
            FILE *temp_fp = fopen(spec_str, "r");
            if (temp_fp) {
                fclose(temp_fp);
                file_content = eval_read_file_to_string(spec_str);
            }
        } else {
            FILE *temp_fp = fopen(spec_str, "r");
            if (!temp_fp) {
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Dialect spec file not found or invalid format";
                return res;
            }
            fclose(temp_fp);
            file_content = eval_read_file_to_string(spec_str);
            if (!file_content) {
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Failed to read dialect spec file";
                return res;
            }
            
            const char *ext = strrchr(spec_str, '.');
            if (ext) {
                if (strcasecmp(ext, ".json") == 0) {
                    format = "JSON";
                } else if (strcasecmp(ext, ".ini") == 0) {
                    format = "INI";
                } else if (strcasecmp(ext, ".xml") == 0) {
                    format = "XML";
                } else if (strcasecmp(ext, ".yaml") == 0 || strcasecmp(ext, ".yml") == 0) {
                    format = "YAML";
                }
            }
            if (!format) {
                free(file_content);
                str_release(vm_get_str(vm), args[0].as.string);
                err->code = 5; err->message = "Could not infer dialect format from file extension (expected .json, .ini, .xml, .yaml)";
                return res;
            }
        }
        
        const char *parse_source = file_content ? file_content : spec_str;
        BppMap *map = NULL;
        if (strcasecmp(format, "JSON") == 0) {
            map = bpp_map_parse_json(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "XML") == 0) {
            map = bpp_map_parse_xml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "YAML") == 0) {
            map = bpp_map_parse_yaml(vm_get_str(vm), parse_source);
        } else if (strcasecmp(format, "INI") == 0) {
            map = bpp_map_parse_ini(vm_get_str(vm), parse_source);
        } else {
            err->code = 5; err->message = "Unsupported spec format (expected JSON, XML, YAML, or INI)";
        }
        
        if (file_content) free(file_content);
        str_release(vm_get_str(vm), args[0].as.string);
        if (arg_count == 2) {
            str_release(vm_get_str(vm), args[1].as.string);
        }
        
        if (err->code == 0) {
            if (!map) {
                err->code = 5; err->message = "Failed to parse dialect spec content";
            } else {
                res.type = VAL_MAP;
                res.as.map = map;
            }
        }
    }
    else if (strcmp(uname, "DIALECT_VALIDATE") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_VALIDATE expects one MAP argument";
            return res;
        }
        char val_err[512] = "";
        bool ok = dialect_validate_map(vm, args[0].as.map, val_err, sizeof(val_err));
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        if (!ok) {
            err->code = 5;
            static char err_msg_buf[512];
            strncpy(err_msg_buf, val_err, sizeof(err_msg_buf) - 1);
            err_msg_buf[sizeof(err_msg_buf) - 1] = '\0';
            err->message = err_msg_buf;
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = 1.0;
    }
    else if (strcmp(uname, "DIALECT_REGISTER") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_REGISTER expects one MAP argument";
            return res;
        }
        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err->code = 14; err->message = "Out of memory allocating dialect";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            return res;
        }
        if (!dialect_load_from_map(vm, args[0].as.map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            err->code = 5;
            static char err_msg_buf2[512];
            strncpy(err_msg_buf2, val_err, sizeof(err_msg_buf2) - 1);
            err_msg_buf2[sizeof(err_msg_buf2) - 1] = '\0';
            err->message = err_msg_buf2;
            return res;
        }
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        vm_set_active_dialect(vm, d);
        res.type = VAL_NUMBER;
        res.as.number = 1.0;
    }
    else if (strcmp(uname, "DIALECT_DOC$") == 0) {
        if (arg_count != 1 || args[0].type != VAL_MAP) {
            err->code = 13; err->message = "DIALECT_DOC$ expects one MAP argument";
            return res;
        }
        BppDialect *d = dialect_create();
        char val_err[512] = "";
        if (!d) {
            err->code = 14; err->message = "Out of memory allocating dialect";
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            return res;
        }
        if (!dialect_load_from_map(vm, args[0].as.map, d, val_err, sizeof(val_err))) {
            dialect_free(d);
            bpp_map_release(vm_get_str(vm), args[0].as.map);
            err->code = 5;
            static char err_msg_buf3[512];
            strncpy(err_msg_buf3, val_err, sizeof(err_msg_buf3) - 1);
            err_msg_buf3[sizeof(err_msg_buf3) - 1] = '\0';
            err->message = err_msg_buf3;
            return res;
        }
        bpp_map_release(vm_get_str(vm), args[0].as.map);
        
        char *docs = dialect_generate_docs(vm, d);
        dialect_free(d);
        
        if (!docs) {
            err->code = 14; err->message = "Failed to generate dialect documentation";
        } else {
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), docs, strlen(docs));
            free(docs);
        }
    }
    else {
        const FunctionEntry *entry = funcreg_find_by_name(uname);
        if (entry) {
            if (arg_count < entry->min_args) {
                err->code = 13;
                err->message = "Too few arguments for function";
                return res;
            }
            if (arg_count > entry->max_args) {
                err->code = 13;
                err->message = "Too many arguments for function";
                return res;
            }

            /* Check safety classification level gates and keyword restrictions */
            bool blocked = false;
            BppSecLevel current_lvl = security_get_level();

            if (entry->safety == FSAFE_SYSTEM) {
                if (current_lvl >= SEC_SAFE) blocked = true;
            } else if (entry->safety == FSAFE_IO) {
                if (current_lvl >= SEC_STANDARD) blocked = true;
            } else if (entry->safety == FSAFE_STATE) {
                if (current_lvl >= SEC_PARANOID) blocked = true;
            }

            if (entry->keyword != KW_NONE && security_is_keyword_restricted((int)entry->keyword)) {
                blocked = true;
            }

            if (blocked) {
                err->code = 70; /* Permission Denied */
                err->message = "Function blocked by active security level or runtime restriction";
                return res;
            }

            res = entry->handler(args, arg_count, vm);
        } else {
            err->code = 2;
            err->message = "Undefined function called";
        }
    }
 
    return res;
}

BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    memset(&null_val, 0, sizeof(null_val));

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* Allocate stacks from scratch arena */
    BValue *val_stack = (BValue *)mem_scratch_alloc(mem, sizeof(BValue) * MAX_EVAL_DEPTH);
    if (!val_stack) {
        out_err->code = 14;
        out_err->message = "Evaluation stack overflow (scratch exhausted)";
        return null_val;
    }

    size_t val_ptr = 0;
    int open_parens = 0;
    BppToken tok = lex_peek(lex);

    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA &&
           tok.type != TOK_SEMICOLON && (tok.type != TOK_RPAREN || open_parens > 0) &&
           tok.type != TOK_RBRACKET &&
           (tok.type != TOK_KEYWORD || tok.as.keyword == KW_NONE ||
            tok.as.keyword == KW_TASK || tok.as.keyword == KW_PLAY || tok.as.keyword == KW_HELP ||
            tok.as.keyword == KW_SCREEN || tok.as.keyword == KW_SEEK ||
            tok.as.keyword == KW_TIMER || tok.as.keyword == KW_KEY ||
            tok.as.keyword == KW_REMOVE || tok.as.keyword == KW_REMOVE_STR ||
            tok.as.keyword == KW_ALARM || tok.as.keyword == KW_ALARM_STR ||
            tok.as.keyword == KW_RANDOMIZE)) {

        /* Stop parsing if we see 'AT' identifier */
        if (tok.type == TOK_IDENT && tok.length == 2 &&
            (tok.start[0] == 'A' || tok.start[0] == 'a') &&
            (tok.start[1] == 'T' || tok.start[1] == 't')) {
            break;
        }

        /* Read the peeked token */
        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_STRING) {
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_RPN_LITERAL) {
            char *rpn_str = (char *)mem_scratch_alloc(mem, tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14;
                out_err->message = "Scratch memory exhausted";
                return null_val;
            }
            memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(mem, rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) return null_val;

            val_stack[val_ptr++] = res;
        } else if (tok.type == TOK_IDENT) {
            /* Variable or function lookup */
            char name_buf[256];
            size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
            memcpy(name_buf, tok.start, copy_len);
            name_buf[copy_len] = '\0';

            if (strcasecmp(name_buf, "DUP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DUP";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 1];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "DROP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DROP";
                    return null_val;
                }
                val_ptr--;
                BValue val = val_stack[val_ptr];
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
            } else if (strcasecmp(name_buf, "SWAP") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on SWAP";
                    return null_val;
                }
                BValue temp = val_stack[val_ptr - 1];
                val_stack[val_ptr - 1] = val_stack[val_ptr - 2];
                val_stack[val_ptr - 2] = temp;
            } else if (strcasecmp(name_buf, "OVER") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on OVER";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 2];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "ROT") == 0) {
                if (val_ptr < 3) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROT";
                    return null_val;
                }
                BValue a = val_stack[val_ptr - 3];
                BValue b = val_stack[val_ptr - 2];
                BValue c = val_stack[val_ptr - 1];
                val_stack[val_ptr - 3] = b;
                val_stack[val_ptr - 2] = c;
                val_stack[val_ptr - 1] = a;
            } else if (strcasecmp(name_buf, "CLEAR") == 0) {
                while (val_ptr > 0) {
                    val_ptr--;
                    BValue val = val_stack[val_ptr];
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                }
            } else if (strcasecmp(name_buf, "DEPTH") == 0) {
                BValue val;
                val.type = VAL_NUMBER;
                val.as.number = (double)val_ptr;
                val_stack[val_ptr++] = val;
            } else if (strcasecmp(name_buf, "PICK") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on PICK";
                    return null_val;
                }
                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "PICK index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "PICK index out of bounds";
                    return null_val;
                }
                BValue picked = val_stack[val_ptr - 1 - n];
                if (picked.type == VAL_STRING && picked.as.string) {
                    str_add_ref(picked.as.string);
                }
                val_stack[val_ptr++] = picked;
            } else if (strcasecmp(name_buf, "ROLL") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROLL";
                    return null_val;
                }
                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "ROLL index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "ROLL index out of bounds";
                    return null_val;
                }
                if (n > 0) {
                    BValue rolled = val_stack[(int)val_ptr - 1 - n];
                    for (int i = (int)val_ptr - 1 - n; i < (int)val_ptr - 1; ++i) {
                        val_stack[i] = val_stack[i + 1];
                    }
                    val_stack[(int)val_ptr - 1] = rolled;
                }
            } else {
                /* Check if followed by '(' */
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); /* Consume '(' */
                    if (is_builtin_function(name_buf)) {
                        BValue val = eval_builtin_function(vm, name_buf, lex, true, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else if (!arr_exists(vm_get_arr(vm), name_buf)) {
                        BValue args[8];
                        int argc = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (argc >= 8) {
                                out_err->code = 2;
                                out_err->message = "Too many arguments in function call";
                                return null_val;
                            }
                            args[argc++] = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) return null_val;
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue val = invoke_user_function(vm, name_buf, args, argc, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else {
                        /* Array access */
                        int indices[4];
                        int num_indices = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (num_indices >= 4) {
                                out_err->code = 9;
                                out_err->message = "Subscript out of range (max 4 dimensions)";
                                return null_val;
                            }
                            BValue idx_val = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) return null_val;
                            indices[num_indices++] = (int)idx_val.as.number;
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue *elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, indices, out_err);
                        if (out_err->code != 0 || !elem) {
                            return null_val;
                        }
                        BValue val = *elem;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            bpp_map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                } else {
                    /* Simple variable lookup */
                    BValue *v = var_lookup(var, name_buf, false);
                    if (!v) {
                        char base_name[256];
                        char member_chain[8][64];
                        int member_count = 0;
                        split_member_chain(name_buf, strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                        if (member_count > 0) {
                            v = var_lookup(var, base_name, false);
                            if (v) {
                                BValue val = *v;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) bpp_map_add_ref(val.as.map);
                                
                                /* Walk up to the last member */
                                bool walk_err = false;
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        walk_err = true; break;
                                    }
                                    BValue next_val;
                                    if (!bpp_map_get(val.as.map, member_chain[m], &next_val)) {
                                        walk_err = true; break;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) bpp_map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                if (!walk_err) {
                                    /* Check if followed by '(' -> Method call */
                                    if (lex_peek(lex).type == TOK_LPAREN) {
                                        lex_next(lex); /* Consume '(' */
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue type_val;
                                            if (bpp_map_get(val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING) {
                                                char fully_qualified_method[512];
                                                snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                         str_data(type_val.as.string), member_chain[member_count - 1]);
                                                
                                                BValue args[9];
                                                int argc = 0;
                                                args[argc++] = val;
                                                bpp_map_add_ref(val.as.map);
                                                
                                                while (true) {
                                                    BppToken next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    }
                                                    if (argc >= 9) {
                                                        walk_err = true; break;
                                                    }
                                                    args[argc++] = eval_expression(vm, lex, out_err);
                                                    if (out_err->code != 0) {
                                                        walk_err = true; break;
                                                    }
                                                    next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_COMMA) {
                                                        lex_next(lex);
                                                    } else if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    } else {
                                                        walk_err = true; break;
                                                    }
                                                }
                                                
                                                if (!walk_err) {
                                                    BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                                                    for (int i = 0; i < argc; i++) {
                                                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                        else if (args[i].type == VAL_MAP) bpp_map_release(vm_get_str(vm), args[i].as.map);
                                                    }
                                                    if (out_err->code == 0) {
                                                        val_stack[val_ptr++] = ret_val;
                                                        tok = lex_peek(lex);
                                                        continue;
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        /* Standard field lookup on last member */
                                        int m = member_count - 1;
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue next_val;
                                            if (bpp_map_get(val.as.map, member_chain[m], &next_val)) {
                                                BValue copy = next_val;
                                                if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                                else if (copy.type == VAL_MAP && copy.as.map) bpp_map_add_ref(copy.as.map);
                                                
                                                bpp_map_release(vm_get_str(vm), val.as.map);
                                                val_stack[val_ptr++] = copy;
                                                tok = lex_peek(lex);
                                                continue;
                                            }
                                        }
                                    }
                                }
                                
                                if (val.type == VAL_MAP && val.as.map) bpp_map_release(vm_get_str(vm), val.as.map);
                                else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                            }
                        }
                        
                        /* Return uninitialized variable (numeric 0.0) */
                        BValue val;
                        val.type = VAL_NUMBER;
                        val.as.number = 0.0;
                        val_stack[val_ptr++] = val;
                    } else {
                        BValue val = *v;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            bpp_map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                }
            }
        } else if (is_operator(tok.type)) {
            BppTokenType op = tok.type;
            if (val_ptr == 1 && op == TOK_MINUS) {
                op = TOK_UNARY_MINUS;
            }
            if (!execute_op(vm, op, val_stack, &val_ptr, out_err)) {
                return null_val;
            }
        } else if (tok.type == TOK_LPAREN) {
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            open_parens--;
        } else {
            out_err->code = 2;
            out_err->message = "Unexpected token in RPN expression";
            return null_val;
        }

        tok = lex_peek(lex);
    }

    if (val_ptr == 0) {
        BValue zero;
        zero.type = VAL_NUMBER;
        zero.as.number = 0.0;
        return zero;
    }

    return val_stack[val_ptr - 1];
}

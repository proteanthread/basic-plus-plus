#include "infix_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: infix_core.c
 * Subsystem: Standalone Infix Math & Expression Parser
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses and evaluates algebraic infix math expression strings
 *    using operator precedence (Shunting-yard algorithm).
 *
 * 2. WHAT TO EXPECT:
 *    Precedence-based parsing. Pure floating-point math evaluation.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Supported operator tokens, priority weights.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Parsing hierarchy logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check parentheses balance and token validity on parse error.
 * ===================================================================== */

#define MAX_STACK 64

typedef enum {
    OP_LPAREN,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_UNARY_MINUS
} OpType;

static int get_precedence(OpType op) {
    switch (op) {
    case OP_LPAREN: return 0;
    case OP_ADD:
    case OP_SUB: return 1;
    case OP_MUL:
    case OP_DIV: return 2;
    case OP_POW: return 3;
    case OP_UNARY_MINUS: return 4;
    default: return -1;
    }
}

static int is_right_associative(OpType op) {
    return op == OP_POW;
}

static int apply_op(OpType op, double *val_stack, int *val_top) {
    if (op == OP_UNARY_MINUS) {
        if (*val_top < 1) return -1;
        val_stack[*val_top - 1] = -val_stack[*val_top - 1];
        return 0;
    }
    if (*val_top < 2) return -1;
    double b = val_stack[--(*val_top)];
    double a = val_stack[*val_top - 1];
    double res = 0.0;
    switch (op) {
    case OP_ADD: res = a + b; break;
    case OP_SUB: res = a - b; break;
    case OP_MUL: res = a * b; break;
    case OP_DIV: 
        if (b == 0.0) return -1; /* Div by zero */
        res = a / b; 
        break;
    case OP_POW: res = pow(a, b); break;
    default: return -1;
    }
    val_stack[*val_top - 1] = res;
    return 0;
}

int infix_core_evaluate(const char *expr, InfixVarResolver var_cb, InfixFuncResolver func_cb, void *user_data, double *out_val)
{
    double val_stack[MAX_STACK];
    OpType op_stack[MAX_STACK];
    int val_top = 0;
    int op_top = 0;
    
    const char *p = expr;
    int last_was_op = 1; /* For detecting unary operators */
    
    while (*p) {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }
        
        /* Parse numbers */
        if (isdigit((unsigned char)*p) || *p == '.') {
            char *end;
            double val = strtod(p, &end);
            if (end == p) return -1;
            if (val_top >= MAX_STACK) return -1;
            val_stack[val_top++] = val;
            p = end;
            last_was_op = 0;
            continue;
        }
        
        /* Parse variables or functions */
        if (isalpha((unsigned char)*p) || *p == '_') {
            char name[64];
            int len = 0;
            while ((isalnum((unsigned char)*p) || *p == '_') && len < 63) {
                name[len++] = *p++;
            }
            name[len] = '\0';
            
            /* Check if it's a function call */
            while (*p && isspace((unsigned char)*p)) p++;
            if (*p == '(') {
                /* We have a function. Parse the argument inside parentheses. */
                p++; /* skip '(' */
                /* Find matching closing parenthesis */
                int paren_count = 1;
                const char *arg_start = p;
                while (*p && paren_count > 0) {
                    if (*p == '(') paren_count++;
                    else if (*p == ')') paren_count--;
                    p++;
                }
                if (paren_count > 0) return -1; /* mismatched parens */
                
                /* Extract argument string */
                size_t arg_len = (size_t)(p - 1 - arg_start);
                char *arg_str = (char *)malloc(arg_len + 1);
                if (!arg_str) return -1;
                memcpy(arg_str, arg_start, arg_len);
                arg_str[arg_len] = '\0';
                
                double arg_val;
                int eval_res = infix_core_evaluate(arg_str, var_cb, func_cb, user_data, &arg_val);
                free(arg_str);
                if (eval_res != 0) return -1;
                
                /* Resolve function */
                int found = 0;
                double res = 0.0;
                if (func_cb) {
                    res = func_cb(user_data, name, arg_val, &found);
                }
                if (!found) {
                    /* Fallback to standard math functions */
                    if (strcmp(name, "sin") == 0) { res = sin(arg_val); found = 1; }
                    else if (strcmp(name, "cos") == 0) { res = cos(arg_val); found = 1; }
                    else if (strcmp(name, "tan") == 0) { res = tan(arg_val); found = 1; }
                    else if (strcmp(name, "abs") == 0) { res = fabs(arg_val); found = 1; }
                    else if (strcmp(name, "sqr") == 0 || strcmp(name, "sqrt") == 0) { 
                        if (arg_val < 0) return -1;
                        res = sqrt(arg_val); 
                        found = 1; 
                    }
                    else if (strcmp(name, "log") == 0) { 
                        if (arg_val <= 0) return -1;
                        res = log(arg_val); 
                        found = 1; 
                    }
                    else if (strcmp(name, "exp") == 0) { res = exp(arg_val); found = 1; }
                }
                if (!found) return -1; /* Unknown function */
                
                if (val_top >= MAX_STACK) return -1;
                val_stack[val_top++] = res;
            } else {
                /* Resolved as a variable */
                int found = 0;
                double res = 0.0;
                if (var_cb) {
                    res = var_cb(user_data, name, &found);
                }
                if (!found) return -1; /* Unknown variable */
                
                if (val_top >= MAX_STACK) return -1;
                val_stack[val_top++] = res;
            }
            last_was_op = 0;
            continue;
        }
        
        /* Parentheses */
        if (*p == '(') {
            if (op_top >= MAX_STACK) return -1;
            op_stack[op_top++] = OP_LPAREN;
            p++;
            last_was_op = 1;
            continue;
        }
        
        if (*p == ')') {
            while (op_top > 0 && op_stack[op_top - 1] != OP_LPAREN) {
                if (apply_op(op_stack[--op_top], val_stack, &val_top) != 0) return -1;
            }
            if (op_top == 0) return -1; /* mismatched parens */
            op_top--; /* pop '(' */
            p++;
            last_was_op = 0;
            continue;
        }
        
        /* Operators */
        OpType op;
        int is_op = 0;
        if (*p == '+') {
            if (last_was_op) {
                p++; /* Unary plus - ignore */
                continue;
            }
            op = OP_ADD;
            is_op = 1;
        } else if (*p == '-') {
            if (last_was_op) {
                op = OP_UNARY_MINUS;
            } else {
                op = OP_SUB;
            }
            is_op = 1;
        } else if (*p == '*') {
            op = OP_MUL;
            is_op = 1;
        } else if (*p == '/') {
            op = OP_DIV;
            is_op = 1;
        } else if (*p == '^') {
            op = OP_POW;
            is_op = 1;
        }
        
        if (is_op) {
            int prec = get_precedence(op);
            while (op_top > 0) {
                OpType top_op = op_stack[op_top - 1];
                int top_prec = get_precedence(top_op);
                if (top_prec > prec || (top_prec == prec && !is_right_associative(op))) {
                    op_top--;
                    if (apply_op(top_op, val_stack, &val_top) != 0) return -1;
                } else {
                    break;
                }
            }
            if (op_top >= MAX_STACK) return -1;
            op_stack[op_top++] = op;
            p++;
            last_was_op = 1;
            continue;
        }
        
        return -1; /* Invalid character */
    }
    
    while (op_top > 0) {
        OpType op = op_stack[--op_top];
        if (op == OP_LPAREN) return -1; /* mismatched parens */
        if (apply_op(op, val_stack, &val_top) != 0) return -1;
    }
    
    if (val_top != 1) return -1;
    *out_val = val_stack[0];
    return 0;
}

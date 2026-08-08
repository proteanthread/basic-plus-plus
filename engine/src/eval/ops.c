/**
 * @file ops.c
 * @brief Binary, unary, and relational operator handlers for expression evaluation in BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements operator handlers: arithmetic (`+`, `-`, `*`, `/`, `^`, `MOD`), relational (`=`, `<>`, `<`, `>`, `<=`, `>=`), logical (`AND`, `OR`, `XOR`, `NOT`, `EQV`, `IMP`), bitwise operators, and string concatenation.
 *
 * 2. WHY IT EXISTS:
 * Evaluates binary and unary operations across primitive values, string handles, and boolean logic flags.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Operates on two `BValue` inputs, performing type coercion if necessary; string concatenation creates a new refcounted string while releasing input temporaries per Rule #1.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'eval'. Includes "eval/eval_internal.h", "eval/eval.h",
 * <math.h>, <string.h>, <stdlib.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support bitwise matrix operators or custom user-defined operator overloading hooks.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Relational comparison result truth values (-1.0 for true, 0.0 for false) and double precision power calculations.
 *
 * 8. WHAT TO EXPECT:
 * Returns BValue result or ERR_DIVISION_BY_ZERO / ERR_TYPE_MISMATCH.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify zero-division checks before floating-point division or MOD.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Operands are valid initialized BValue structures.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Floating-point comparison safety via math.h helpers.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/eval.h
 * - engine/include/eval/eval_internal.h
 */

/**
 * @file eval_ops.c
 * @brief Operator execution and member access resolution.
 */

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "core/struct.h"
#include <string.h>
#include <math.h>

bool eval_execute_op(VMContext *vm, BppTokenType op, BValue *val_stack, size_t *val_ptr, BppError *err) {
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
            val->as.number = (double)(~(int32_t)(int64_t)val->as.number);
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
        case TOK_POW:
            ans = pow(n1, n2);
            if (isnan(ans) || isinf(ans)) {
                err->code = 5;
                err->message = "Overflow or invalid power operation";
                return false;
            }
            break;
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
        case TOK_AND: ans = (double)((int32_t)(int64_t)n1 & (int32_t)(int64_t)n2); break;
        case TOK_OR:  ans = (double)((int32_t)(int64_t)n1 | (int32_t)(int64_t)n2); break;
        case TOK_XOR: ans = (double)((int32_t)(int64_t)n1 ^ (int32_t)(int64_t)n2); break;
        default:
            err->code = 2; /* Syntax error */
            err->message = "Invalid numeric operator";
            return false;
    }

    if (vm_get_arithmetic_decimal(vm) && (op == TOK_PLUS || op == TOK_MINUS || op == TOK_MUL || op == TOK_DIV)) {
        ans = eval_round_to_decimal(ans, 12);
    }

    res->type = VAL_NUMBER;
    res->as.number = ans;
    return true;
}

BValue eval_resolve_member_access(VMContext *vm, LexerContext *lex, BValue val, BppError *out_err) {
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
            if (!map_get(val.as.map, "__type__", &type_val) || type_val.type != VAL_STRING || !type_val.as.string) {
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
            map_add_ref(val.as.map);

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
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return null_val;
                }
                args[argc++] = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) {
                    for (int i = 0; i < argc - 1; i++) {
                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
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
                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                    }
                    return null_val;
                }
            }

            /* Invoke method */
            BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
            for (int i = 0; i < argc; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
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
            if (!map_get(val.as.map, field_name, &field_val)) {
                out_err->code = 35; out_err->message = "Member field not defined in UDT/Class";
                return null_val;
            }
            BValue old_val = val;
            val = field_val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            } else if (val.type == VAL_MAP && val.as.map) {
                map_add_ref(val.as.map);
            }
            if (old_val.type == VAL_MAP && old_val.as.map) {
                map_release(vm_get_str(vm), old_val.as.map);
            }
        }
    }
    return val;
}

// FILENAME: ops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (struct.h, struct.c, variables.h, variables.c)
// NEEDS: libengine (eval_internal.h, map.h, map.c, sub.h, sub.c)
// Provides core logic and interface definitions for ops within BASIC++.
//
// ---- Includes ----

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "core/struct.h"
#include "statements/oop/sub.h"


bool eval_execute_op(VMContext *vm, BppTokenType op, BValue *val_stack, size_t *val_ptr, BppError *err) {
    StringContext *str = vm_get_str(vm);

    if (op == TOK_UNARY_MINUS || op == TOK_UNARY_PLUS || op == TOK_NOT) {
        if (*val_ptr < 1) {
            err->code = 2; // Syntax error
            err->message = "Missing operand for unary operator";
            return false;
        }
        BValue *val = &val_stack[*val_ptr - 1];
        if (val->type == VAL_STRING) {
            err->code = 13; // Type mismatch
            err->message = "Unary operators not supported on strings";
            return false;
        }
        if (val->type == VAL_COMPLEX) {
            if (op == TOK_UNARY_MINUS) {
                val->as.complex_val.real = -val->as.complex_val.real;
                val->as.complex_val.imag = -val->as.complex_val.imag;
            } else if (op == TOK_NOT) {
                err->code = 13; // Type mismatch
                err->message = "NOT operator not supported on complex numbers";
                return false;
            }
            return true;
        }
        if (op == TOK_UNARY_MINUS) {
            val->as.number = -val->as.number;
        } else if (op == TOK_NOT) {
            val->as.number = (double)(~(int64_t)val->as.number);
        }
        return true;
    }

    // Binary operators
    if (*val_ptr < 2) {
        err->code = 2; // Syntax error
        err->message = "Missing operand for binary operator";
        return false;
    }

    BValue rhs = val_stack[--(*val_ptr)];
    BValue lhs = val_stack[*val_ptr - 1];
    BValue *res = &val_stack[*val_ptr - 1];

    // Fast Path: Standard scalar arithmetic (VAL_NUMBER / VAL_INTEGER)
    if ((lhs.type == VAL_NUMBER || lhs.type == VAL_INTEGER) && (rhs.type == VAL_NUMBER || rhs.type == VAL_INTEGER)) {
        double n1 = lhs.as.number;
        double n2 = rhs.as.number;
        double ans = 0.0;
        switch (op) {
            case TOK_PLUS:  ans = n1 + n2; break;
            case TOK_MINUS: ans = n1 - n2; break;
            case TOK_MUL:   ans = n1 * n2; break;
            case TOK_DIV:
                if (n2 == 0.0) {
                    err->code = 11; // Division by zero
                    err->message = "Division by zero";
                    return false;
                }
                ans = n1 / n2;
                break;
            case TOK_EQ: ans = (n1 == n2) ? -1.0 : 0.0; break;
            case TOK_NE: ans = (n1 != n2) ? -1.0 : 0.0; break;
            case TOK_LT: ans = (n1 < n2)  ? -1.0 : 0.0; break;
            case TOK_GT: ans = (n1 > n2)  ? -1.0 : 0.0; break;
            case TOK_LE: ans = (n1 <= n2) ? -1.0 : 0.0; break;
            case TOK_GE: ans = (n1 >= n2) ? -1.0 : 0.0; break;
            default:
                goto generic_op_path;
        }
        res->type = VAL_NUMBER;
        res->as.number = ans;
        return true;
    }

generic_op_path:
    // Infix String Concatenation: &
    if (op == TOK_AMPERSAND) {
        char s1_buf[64], s2_buf[64];
        const char *s1 = "";
        const char *s2 = "";

        if (lhs.type == VAL_STRING && lhs.as.string) {
            s1 = str_data(lhs.as.string);
        } else if (lhs.type == VAL_NUMBER || lhs.type == VAL_INTEGER) {
            runtime_snprintf(s1_buf, sizeof(s1_buf), "%g", lhs.as.number);
            s1 = s1_buf;
        }

        if (rhs.type == VAL_STRING && rhs.as.string) {
            s2 = str_data(rhs.as.string);
        } else if (rhs.type == VAL_NUMBER || rhs.type == VAL_INTEGER) {
            runtime_snprintf(s2_buf, sizeof(s2_buf), "%g", rhs.as.number);
            s2 = s2_buf;
        }

        size_t l1 = runtime_strlen(s1);
        size_t l2 = runtime_strlen(s2);
        HalContext *hal = hal_get();
        char *comb = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(l1 + l2 + 1) : NULL;
        if (!comb) {
            err->code = 14; err->message = "Out of string space";
            return false;
        }
        runtime_memcpy(comb, s1, l1);
        runtime_memcpy(comb + l1, s2, l2);
        comb[l1 + l2] = '\0';

        if (lhs.type == VAL_STRING && lhs.as.string) str_release(str, lhs.as.string);
        if (rhs.type == VAL_STRING && rhs.as.string) str_release(str, rhs.as.string);

        res->type = VAL_STRING;
        res->as.string = str_create(str, comb, l1 + l2);
        if (hal && hal->mem.free) hal->mem.free(comb);
        return true;
    }

    // Complex arithmetic operations
    if (lhs.type == VAL_COMPLEX || rhs.type == VAL_COMPLEX) {
        if ((lhs.type != VAL_COMPLEX && lhs.type != VAL_NUMBER && lhs.type != VAL_INTEGER) ||
            (rhs.type != VAL_COMPLEX && rhs.type != VAL_NUMBER && rhs.type != VAL_INTEGER)) {
            err->code = 13; // Type mismatch
            err->message = "Type mismatch with complex number";
            return false;
        }

        double z1_r = (lhs.type == VAL_COMPLEX) ? lhs.as.complex_val.real : lhs.as.number;
        double z1_i = (lhs.type == VAL_COMPLEX) ? lhs.as.complex_val.imag : 0.0;
        double z2_r = (rhs.type == VAL_COMPLEX) ? rhs.as.complex_val.real : rhs.as.number;
        double z2_i = (rhs.type == VAL_COMPLEX) ? rhs.as.complex_val.imag : 0.0;

        switch (op) {
            case TOK_PLUS:
                res->type = VAL_COMPLEX;
                res->as.complex_val.real = z1_r + z2_r;
                res->as.complex_val.imag = z1_i + z2_i;
                return true;
            case TOK_MINUS:
                res->type = VAL_COMPLEX;
                res->as.complex_val.real = z1_r - z2_r;
                res->as.complex_val.imag = z1_i - z2_i;
                return true;
            case TOK_MUL:
                res->type = VAL_COMPLEX;
                res->as.complex_val.real = z1_r * z2_r - z1_i * z2_i;
                res->as.complex_val.imag = z1_r * z2_i + z1_i * z2_r;
                return true;
            case TOK_DIV: {
                double denom = z2_r * z2_r + z2_i * z2_i;
                if (denom == 0.0) {
                    err->code = 11; // Division by zero
                    err->message = "Division by zero in complex arithmetic";
                    return false;
                }
                res->type = VAL_COMPLEX;
                res->as.complex_val.real = (z1_r * z2_r + z1_i * z2_i) / denom;
                res->as.complex_val.imag = (z1_i * z2_r - z1_r * z2_i) / denom;
                return true;
            }
            case TOK_POW: {
                double r = runtime_hypot(z1_r, z1_i);
                double theta = runtime_atan2(z1_i, z1_r);
                if (r == 0.0) {
                    res->type = VAL_COMPLEX;
                    if (z2_r == 0.0 && z2_i == 0.0) {
                        res->as.complex_val.real = 1.0;
                        res->as.complex_val.imag = 0.0;
                    } else {
                        res->as.complex_val.real = 0.0;
                        res->as.complex_val.imag = 0.0;
                    }
                    return true;
                }
                double ln_r = runtime_log(r);
                double u = z2_r * ln_r - z2_i * theta;
                double v = z2_r * theta + z2_i * ln_r;
                double exp_u = runtime_exp(u);
                res->type = VAL_COMPLEX;
                res->as.complex_val.real = exp_u * runtime_cos(v);
                res->as.complex_val.imag = exp_u * runtime_sin(v);
                return true;
            }
            case TOK_EQ:
                res->type = VAL_NUMBER;
                res->as.number = (z1_r == z2_r && z1_i == z2_i) ? -1.0 : 0.0;
                return true;
            case TOK_NE:
                res->type = VAL_NUMBER;
                res->as.number = (z1_r != z2_r || z1_i != z2_i) ? -1.0 : 0.0;
                return true;
            default:
                err->code = 13; // Type mismatch
                err->message = "Invalid operator for complex numbers";
                return false;
        }
    }


    // Object operator overloading
    if (lhs.type == VAL_MAP && lhs.as.map) {
        BValue type_val;
        if (map_get(lhs.as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
            const char *op_sym = (op == TOK_PLUS) ? "+" : (op == TOK_MINUS) ? "-" : (op == TOK_MUL) ? "*" : (op == TOK_DIV) ? "/" : (op == TOK_EQ) ? "=" : "";
            if (op_sym[0] != '\0') {
                char op_func[128];
                runtime_snprintf(op_func, sizeof(op_func), "%s.OPERATOR_%s", str_data(type_val.as.string), op_sym);
                BppLineNumber target_line = 0;
                const char *target_text = NULL;
                bool is_lib = false;
                if (!find_procedure_ex(vm, op_func, KW_FUNCTION, &target_line, &target_text, &is_lib) &&
                    !find_procedure_ex(vm, op_func, KW_OPERATOR, &target_line, &target_text, &is_lib)) {
                    runtime_snprintf(op_func, sizeof(op_func), "%s.%s", str_data(type_val.as.string), op_sym);
                }
                if (find_procedure_ex(vm, op_func, KW_FUNCTION, &target_line, &target_text, &is_lib) ||
                    find_procedure_ex(vm, op_func, KW_OPERATOR, &target_line, &target_text, &is_lib)) {
                    BValue args[2];
                    args[0] = lhs;
                    map_add_ref(args[0].as.map);
                    args[1] = rhs;
                    if (rhs.type == VAL_STRING && rhs.as.string) str_add_ref(rhs.as.string);
                    else if (rhs.type == VAL_MAP && rhs.as.map) map_add_ref(rhs.as.map);
                    *res = invoke_user_function(vm, op_func, args, 2, err);
                    if (args[0].type == VAL_MAP && args[0].as.map) map_release(str, args[0].as.map);
                    if (args[1].type == VAL_STRING && args[1].as.string) str_release(str, args[1].as.string);
                    else if (args[1].type == VAL_MAP && args[1].as.map) map_release(str, args[1].as.map);
                    return (err->code == 0);
                }
            }
        }
    }

    // String operators
    if (lhs.type == VAL_STRING || rhs.type == VAL_STRING) {
        if (lhs.type != VAL_STRING || rhs.type != VAL_STRING) {
            err->code = 13; // Type mismatch
            err->message = "Mixed string and numeric arguments";
            return false;
        }

        if (op == TOK_PLUS) {
            // String Concatenation
            BppStringRef concat = str_concat(str, lhs.as.string, rhs.as.string);
            if (!concat) {
                err->code = 14; // Out of string space
                err->message = "String heap limit reached in concatenation";
                return false;
            }
            res->type = VAL_STRING;
            res->as.string = concat;
            // Release temp inputs if needed
            if (lhs.as.string) str_release(str, lhs.as.string);
            if (rhs.as.string) str_release(str, rhs.as.string);
            return true;
        }

        // Relational string comparisons
        const char *s1 = str_data(lhs.as.string);
        const char *s2 = str_data(rhs.as.string);
        int cmp = runtime_strcmp(s1, s2);
        double bool_res = 0.0; // 0.0 is False, -1.0 is True

        switch (op) {
            case TOK_EQ: bool_res = (cmp == 0) ? -1.0 : 0.0; break;
            case TOK_NE: bool_res = (cmp != 0) ? -1.0 : 0.0; break;
            case TOK_LT: bool_res = (cmp < 0)  ? -1.0 : 0.0; break;
            case TOK_GT: bool_res = (cmp > 0)  ? -1.0 : 0.0; break;
            case TOK_LE: bool_res = (cmp <= 0) ? -1.0 : 0.0; break;
            case TOK_GE: bool_res = (cmp >= 0) ? -1.0 : 0.0; break;
            default:
                err->code = 13; // Type mismatch
                err->message = "Invalid operator for strings";
                return false;
        }

        // Release strings
        if (lhs.as.string) str_release(str, lhs.as.string);
        if (rhs.as.string) str_release(str, rhs.as.string);

        res->type = VAL_NUMBER;
        res->as.number = bool_res;
        return true;
    }

    // Numeric operations
    double n1 = lhs.as.number;
    double n2 = rhs.as.number;
    double ans = 0.0;

    switch (op) {
        case TOK_PLUS:  ans = n1 + n2; break;
        case TOK_MINUS: ans = n1 - n2; break;
        case TOK_MUL:   ans = n1 * n2; break;
        case TOK_POW:
            ans = runtime_pow(n1, n2);
            if (runtime_isnan(ans) || runtime_isinf(ans)) {
                err->code = 5;
                err->message = "Overflow or invalid power operation";
                return false;
            }
            break;
        case TOK_DIV:
            if (n2 == 0.0) {
                err->code = 11; // Division by zero
                err->message = "Division by zero";
                return false;
            }
            ans = n1 / n2;
            break;
        case TOK_BACKSLASH:
            if ((int64_t)n2 == 0) {
                err->code = 11; // Division by zero
                err->message = "Division by zero in integer division";
                return false;
            }
            ans = (double)((int64_t)n1 / (int64_t)n2);
            break;
        case TOK_EQ: ans = n1 == n2 ? -1.0 : 0.0; break;
        case TOK_NE: ans = n1 != n2 ? -1.0 : 0.0; break;
        case TOK_LT: ans = n1 < n2  ? -1.0 : 0.0; break;
        case TOK_GT: ans = n1 > n2  ? -1.0 : 0.0; break;
        case TOK_LE: ans = n1 <= n2 ? -1.0 : 0.0; break;
        case TOK_GE: ans = n1 >= n2 ? -1.0 : 0.0; break;
        case TOK_AND: ans = (double)((int64_t)n1 & (int64_t)n2); break;
        case TOK_OR:  ans = (double)((int64_t)n1 | (int64_t)n2); break;
        case TOK_XOR: ans = (double)((int64_t)n1 ^ (int64_t)n2); break;
        case TOK_IMP: ans = (double)(~(int64_t)n1 | (int64_t)n2); break;
        case TOK_EQV: ans = (double)(~((int64_t)n1 ^ (int64_t)n2)); break;
        case TOK_SHL: ans = (double)((uint64_t)(int64_t)n1 << (uint64_t)(int64_t)n2); break;
        case TOK_SHR: ans = (double)((uint64_t)(int64_t)n1 >> (uint64_t)(int64_t)n2); break;
        case TOK_READBIT: ans = (double)(((uint64_t)(int64_t)n1 >> (uint64_t)(int64_t)n2) & 1); break;
        case TOK_SETBIT: ans = (double)((uint64_t)(int64_t)n1 | ((uint64_t)1 << (uint64_t)(int64_t)n2)); break;
        case TOK_RESETBIT: ans = (double)((uint64_t)(int64_t)n1 & ~((uint64_t)1 << (uint64_t)(int64_t)n2)); break;
        case TOK_TOGGLEBIT: ans = (double)((uint64_t)(int64_t)n1 ^ ((uint64_t)1 << (uint64_t)(int64_t)n2)); break;
        case TOK_MIN: ans = (n1 < n2) ? n1 : n2; break;
        case TOK_MAX: ans = (n1 > n2) ? n1 : n2; break;
        case TOK_HYPOT: ans = runtime_sqrt(n1 * n1 + n2 * n2); break;
        case TOK_ATAN2: ans = runtime_atan2(n1, n2); break;
        case TOK_REMAINDER:
            if (n2 == 0.0) {
                err->code = 11; // Division by zero
                err->message = "Division by zero in REMAINDER";
                return false;
            }
            ans = n1 - n2 * runtime_floor(n1 / n2);
            break;
        case TOK_MOD:
            if ((int64_t)n2 == 0) {
                err->code = 11; // Division by zero
                err->message = "Division by zero in MOD";
                return false;
            }
            ans = (double)((int64_t)n1 % (int64_t)n2);
            break;
        default:
            err->code = 2; // Syntax error
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
    runtime_memset(&null_val, 0, sizeof(null_val));

    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex); // Consume '.'
        BppToken field_tok = lex_next(lex);
        if (field_tok.type != TOK_IDENT) {
            out_err->code = 2; out_err->message = "Expected member identifier after '.'";
            return null_val;
        }
        char field_name[256];
        size_t flen = (field_tok.length < sizeof(field_name) - 1) ? field_tok.length : sizeof(field_name) - 1;
        runtime_memcpy(field_name, field_tok.start, flen);
        field_name[flen] = '\0';

        // Check if followed by '(' -> Class method call
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); // Consume '('
            
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
            runtime_snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                             str_data(type_val.as.string), field_name);

            // Parse arguments
            BValue args[9];
            int argc = 0;
            
            // Implicit THIS
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

            // Invoke method
            BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
            for (int i = 0; i < argc; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
            }
            if (out_err->code != 0) return null_val;
            
            val = ret_val;
        } else {
            // Standard field lookup
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


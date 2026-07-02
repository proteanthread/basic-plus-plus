/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_expr.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Statement dispatch and recursive-descent expression parsing.
 *
 * 2. WHAT TO EXPECT:
 *    Highly recursive execution structure. Expression evaluation strictly obeys operator precedence levels.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Syntax parsing details for expressions, parser diagnostic logging, and parsing warnings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Operator precedence hierarchy, recursive call stack structure, keyword parsing dispatch methods.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Increase stack allocation limit if stack overflow occurs. Ensure expression parentheses evaluate in correct recursion.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - parser_expr.c
// ---
//
// Expression parsing engine.
//
// PURPOSE:
//   Implements the recursive-descent expression parser for both
//   integer arithmetic (long) and BValue (polymorphic) expressions.
//   All BASIC built-in functions are evaluated here.
//
// EXPRESSION GRAMMAR (standard BASIC precedence):
//   expression = [+|-|NOT] term ((+|-) term)* ((AND|OR|XOR) expr)*
//   term       = power ((*|/|\|MOD) power)*
//   power      = factor (^ factor)*
//   factor     = number | variable | @(expr) | (expr) | function(args)
//
// TWO EXPRESSION PATHS:
//   1. Integer path: parse_expression() -> long
//      Used by legacy Phase 1-3 code and integer-only dialects.
//   2. BValue path: parse_expression_bval() -> BValue
//      Used by modern code; handles int/float/string/complex.
//
// HOW TO EXTEND:
//   Adding a new built-in function:
//   1. Register it in funcreg.c with funcreg_add().
//   2. The function handler receives BValue args and returns BValue.
//   3. No changes needed here -- the registry dispatch handles it.
//
//   Adding a new operator:
//   1. Add the precedence level (new parse_xxx function if needed).
//   2. Add token recognition in lexer.c.
//   3. Add the operator case in the appropriate parse level below.
//
// TROUBLESHOOTING:
//   - "HOW?" on expressions: usually divide-by-zero or overflow.
//   - "WHAT?" on function calls: function not in registry, or
//     wrong number of arguments, or $ suffix mismatch.
//
// ---

#include "parser_internal.h"
#include "dialect.h"
#include "gw_math_mbf.h"
#include "gw_sdl2.h"

// --- Expression Parsing ---
 //
 // Expression grammar (standard BASIC precedence):
 //
 // expression = [+|-] term ((+|-) term)*
 // term = factor ((*|/) factor)*
 // factor = number
 // | variable
 // | @(expression)
 // | (expression)
 // | ABS(expression)
 // | RND(expression)
 // | SIZE
 //
 // This gives correct precedence: * and / bind tighter than + and -.
 // Parentheses override precedence.
 //
 // Recursion depth: expression -> term -> factor -> expression (via
 // parentheses or function calls). Maximum depth equals the nesting
 // depth of parentheses in the source, which is bounded by line
 // length (255 chars max -> at most ~127 nesting levels, but in
 // practice programs use 3-5 levels).

 // parse_factor - Parse an atomic expression (highest precedence).
long pi_parse_factor(Lexer *lex, RuntimeState *rt, int line_num)
{
 long value = 0;

 if (error_occurred()) return 0;

 switch (lex->current.type) {
 case TOK_NUMBER:
 value = lex->current.value.num_value;
 lexer_next(lex);
 return value;

 case TOK_VARIABLE:
 {
 char vname = lex->current.value.var_name;
 lexer_next(lex);
  // Check for DIM array access: A(idx) -- auto-DIMs
  if (lex->current.type == TOK_LPAREN &&
  dialect_get_config()->has_dim_arrays) {
  int idx1, idx2 = 0, idx3 = 0;
  BValue dval;
  lexer_next(lex); // consume (
  idx1 = (int)parse_expression(lex, rt, line_num);
  if (error_occurred()) return 0;
  if (lex->current.type == TOK_COMMA) {
   lexer_next(lex);
   idx2 = (int)parse_expression(lex, rt, line_num);
   if (error_occurred()) return 0;
   if (lex->current.type == TOK_COMMA) {
   lexer_next(lex);
   idx3 = (int)parse_expression(lex, rt, line_num);
   if (error_occurred()) return 0;
   }
  }
  if (!lexer_expect(lex, TOK_RPAREN)) return 0;
  dval = runtime_get_dim(rt, &vname, 1,
   idx1, idx2, idx3, line_num);
  return bval_to_int(&dval);
  }
  return runtime_get_var(rt, vname);
 }

 case TOK_NAMED_VAR:
 // Extended variable - look up by name
 value = runtime_get_named_var(rt,
 lex->current.str_start, lex->current.str_length);
 lexer_next(lex);
 return value;

 case TOK_AT:
 // @(expression) - array access
 {
 long index;
 lexer_next(lex); // consume @
 if (!lexer_expect(lex, TOK_LPAREN)) return 0;
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 return runtime_get_array(rt, index);
 }

 case TOK_LPAREN:
 // (expression) - parenthesized sub-expression
 lexer_next(lex); // consume (
 value = parse_expression(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 return value;

 case TOK_KEYWORD:
 // User-defined function dispatch.
 // FN<name>(...) calls are handled before the registry.
 if (lex->current.value.keyword == KW_FN) {
 BValue fnr;
 lexer_next(lex); // consume FN
 fnr = pi_eval_user_fn(lex, rt, line_num);
 if (error_occurred()) return 0;
 return bval_to_int(&fnr);
 }

 // Registry-based function dispatch.
 //
 // Look up the keyword in the function registry. If found,
 // parse arguments, call the handler, and convert the
 // BValue result to long for the integer expression path.
 {
 const FunctionEntry *fn;
 fn = funcreg_find_by_keyword(
 lex->current.value.keyword);
 if (fn != NULL) {
 BValue args[16];
 int argc = 0;
 BValue result;

 lexer_next(lex); // consume function name

 if (fn->max_args > 0 &&
  lex->current.type == TOK_LPAREN) {
  // Parse (arg1, arg2, ...)
  lexer_next(lex); // consume (
 args[argc] = bval_int(
 parse_expression(lex, rt, line_num));
 if (error_occurred()) return 0;
 argc++;

 while (argc < fn->max_args &&
 lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 args[argc] = bval_int(
 parse_expression(lex, rt, line_num));
 if (error_occurred()) return 0;
 argc++;
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return 0;
 }
 // else: zero-arg function (SIZE)

 result = fn->handler(args, argc, (void *)rt);
 if (error_occurred()) return 0;
 return bval_to_int(&result);
 }
 }
 // Fall through to error
 error_raise(ERR_WHAT, line_num);
 return 0;

 default:
 error_raise(ERR_WHAT, line_num);
 return 0;
 }
}

 // parse_power - Parse exponentiation (^).
 //
 // power = factor (^ factor)*
 // Note: ^ is right-associative in QBasic, but we implement
 // left-to-right for simplicity (covers 99% of cases).
long pi_parse_power(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 left = pi_parse_factor(lex, rt, line_num);
 if (error_occurred()) return 0;

 while (lex->current.type == TOK_CARET) {
 long right;
 lexer_next(lex); // consume ^
 right = pi_parse_factor(lex, rt, line_num);
 if (error_occurred()) return 0;
 left = (long)pow((double)left, (double)right);
 }

 return left;
}

 // parse_term - Parse a multiplicative expression.
 //
 // term = power ((*|/|\|MOD) power)*
long pi_parse_term(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 TokenType op;

 left = pi_parse_power(lex, rt, line_num);
 if (error_occurred()) return 0;

 for (;;) {
 int is_mod = 0;
 int is_intdiv = 0;

 if (lex->current.type == TOK_STAR ||
 lex->current.type == TOK_SLASH) {
 op = lex->current.type;
 } else if (lex->current.type == TOK_BACKSLASH) {
 is_intdiv = 1;
 op = TOK_SLASH; // placeholder
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_MOD) {
 is_mod = 1;
 op = TOK_SLASH; // placeholder
 } else {
 break;
 }

 lexer_next(lex); // consume operator

 {
 long right;
 right = pi_parse_power(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (is_mod) {
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left % right;
 } else if (is_intdiv) {
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left / right;
 } else if (op == TOK_STAR) {
 left = left * right;
 } else {
 // Division - check for divide by zero
 if (right == 0) {
 error_raise(ERR_HOW, line_num);
 return 0;
 }
 left = left / right;
 }
 }
 }

 return left;
}

 // parse_expression - Parse an additive expression.
 //
 // expression = [+|-|NOT] term ((+|-) term)*
 // ((AND|OR|XOR|EQV|IMP) expr)*
 //
 // The optional leading +/- handles unary plus/minus.
 // NOT is unary prefix (bitwise complement).
 // AND/OR/XOR/EQV/IMP are lowest-precedence binary.

BValue parse_expression_bval_internal(Lexer *lex, RuntimeState *rt, int line_num, int stop_at_comparisons);

long parse_expression(Lexer *lex, RuntimeState *rt, int line_num)
{
    BValue val = parse_expression_bval_internal(lex, rt, line_num, 1);
    return bval_to_int(&val);
}

BValue parse_expression_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
    return parse_expression_bval_internal(lex, rt, line_num, 0);
}

// --- BValue Expression System ---
/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * Subsystem: Expression Parsing & Evaluation
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Iterative expression evaluation utilizing Dijkstra's Shunting-yard
 *    algorithm. Operands and operators are pushed to explicit software
 *    stacks to avoid C runtime call stack recursion.
 *
 * 2. PORTABILITY CONCERNS:
 *    Completely portable ANSI/ISO C17. State structures are stored
 *    entirely within interpreter-managed stack frames rather than the
 *    host operating system stack, making execution fully independent
 *    of the compiler's stack model.
 *
 * 3. MEMORY MANAGEMENT:
 *    Parser stacks are pre-allocated inside local arrays or stored
 *    in the interpreter stack. Scratch pool memory is leveraged
 *    only for string allocations.
 *
 * 4. PARSER & LEXER BEHAVIOR:
 *    Consumes tokens from the Lexer stream. Relies on operator precedence
 *    hierarchies (precedence levels 1-14) to control reduction order.
 *    Stops on statement separators, commas, or right parentheses
 *    if no matching left parentheses exist on the stack.
 *
 * 5. FUTURE EXPANSION POINTS:
 *    New operators or built-in functions can be registered dynamically
 *    in funcreg.c without changing this evaluation engine.
 *
 * 6. WHAT CAN BE CHANGED:
 *    Operator precedence definitions, support for new literal types.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    Iterative loop structure, suspension frame restoration sequence.
 *
 * 8. TROUBLESHOOTING & FAILURE MODES:
 *    Check precedence mapping if mathematical ordering is wrong.
 *    Ensure stack sizes (32 elements) are not exceeded during nested
 *    evaluations.
 * ===================================================================== */

enum OpType {
    OP_EOF = 0,
    OP_IMP,
    OP_EQV,
    OP_XOR,
    OP_OR,
    OP_AND,
    OP_NOT,
    OP_CMP_EQ,
    OP_CMP_NE,
    OP_CMP_LT,
    OP_CMP_GT,
    OP_CMP_LE,
    OP_CMP_GE,
    OP_LIKE,
    OP_ADD,
    OP_SUB,
    OP_INTDIV,
    OP_MOD,
    OP_MUL,
    OP_DIV,
    OP_UNARY_MINUS,
    OP_UNARY_PLUS,
    OP_POW,
    OP_LPAREN,
    OP_FUNC,
    OP_USER_FUNC,
    OP_ARRAY,
    OP_AT
};

static int get_precedence(int op) {
    switch (op) {
        case OP_IMP: return 1;
        case OP_EQV: return 2;
        case OP_XOR: return 3;
        case OP_OR:  return 4;
        case OP_AND: return 5;
        case OP_NOT: return 6;
        case OP_CMP_EQ:
        case OP_CMP_NE:
        case OP_CMP_LT:
        case OP_CMP_GT:
        case OP_CMP_LE:
        case OP_CMP_GE:
        case OP_LIKE: return 7;
        case OP_ADD:
        case OP_SUB: return 8;
        case OP_INTDIV: return 9;
        case OP_MOD: return 10;
        case OP_MUL:
        case OP_DIV: return 11;
        case OP_UNARY_MINUS:
        case OP_UNARY_PLUS: return 12;
        case OP_POW: return 13;
        case OP_FUNC:
        case OP_USER_FUNC:
        case OP_ARRAY:
        case OP_AT: return 14;
        default: return 0;
    }
}

static int is_right_associative(int op) {
    return (op == OP_POW || op == OP_UNARY_MINUS || op == OP_UNARY_PLUS || op == OP_NOT);
}

static BValue evaluate_variable_simple(Lexer *lex, RuntimeState *rt, Token var_tok, int line_num)
{
    char vname;
    const char *nm;
    int nlen;
    
    if (var_tok.type == TOK_VARIABLE) {
        vname = var_tok.value.var_name;
        lexer_next(lex); // consume variable name
        if (lex->current.type == TOK_DOT) {
            TypedVar *tv = runtime_find_typed_var(rt, &vname, 1);
            if (tv != NULL) {
                UserTypeDef *td = &rt->user_types[tv->type_index];
                const char *fname;
                int flen, fi;
                lexer_next(lex); // consume dot
                if (lex->current.type == TOK_NAMED_VAR) {
                    fname = lex->current.str_start;
                    flen = lex->current.str_length;
                } else if (lex->current.type == TOK_KEYWORD) {
                    fname = lexer_keyword_name(lex->current.value.keyword);
                    flen = fname ? (int)strlen(fname) : 0;
                } else if (lex->current.type == TOK_VARIABLE) {
                    fname = &lex->current.value.var_name;
                    flen = 1;
                } else {
                    error_raise(ERR_WHAT, line_num);
                    return bval_int(0);
                }
                fi = runtime_find_field(td, fname, flen);
                if (fi < 0) {
                    error_raise(ERR_WHAT, line_num);
                    return bval_int(0);
                }
                lexer_next(lex); // consume field
                while (td->fields[fi].nested_type_index >= 0 && lex->current.type == TOK_DOT) {
                    int ci = (int)bval_to_int(&tv->fields[fi]);
                    if (ci < 0 || ci >= rt->typed_var_count) {
                        error_raise(ERR_HOW, line_num);
                        return bval_int(0);
                    }
                    tv = &rt->typed_vars[ci];
                    td = &rt->user_types[tv->type_index];
                    lexer_next(lex); // consume dot
                    if (lex->current.type == TOK_NAMED_VAR) {
                        fname = lex->current.str_start;
                        flen = lex->current.str_length;
                    } else if (lex->current.type == TOK_KEYWORD) {
                        fname = lexer_keyword_name(lex->current.value.keyword);
                        flen = fname ? (int)strlen(fname) : 0;
                    } else if (lex->current.type == TOK_VARIABLE) {
                        fname = &lex->current.value.var_name;
                        flen = 1;
                    } else {
                        error_raise(ERR_WHAT, line_num);
                        return bval_int(0);
                    }
                    fi = runtime_find_field(td, fname, flen);
                    if (fi < 0) {
                        error_raise(ERR_WHAT, line_num);
                        return bval_int(0);
                    }
                    lexer_next(lex);
                }
                return tv->fields[fi];
            }
        }
        return runtime_get_var_bval(rt, vname);
    } else if (var_tok.type == TOK_STRING_VAR) {
        vname = var_tok.value.var_name;
        lexer_next(lex); // consume string var name
        if (!dialect_check_feature("string variables", dialect_get_config()->has_string_vars, line_num))
            return bval_int(0);
        return runtime_get_string_var(rt, vname);
    } else if (var_tok.type == TOK_NAMED_VAR) {
        nm = var_tok.str_start;
        nlen = var_tok.str_length;
        lexer_next(lex); // consume named var name
        for (int ci = 0; ci < rt->const_count; ci++) {
            int cl = rt->constants[ci].name_len;
            if (cl == nlen) {
                int j, match = 1;
                for (j = 0; j < nlen; j++) {
                    char a = nm[j];
                    char b = rt->constants[ci].name[j];
                    if (a >= 'a' && a <= 'z') a = (char)(a - 32);
                    if (b >= 'a' && b <= 'z') b = (char)(b - 32);
                    if (a != b) { match = 0; break; }
                }
                if (match) return rt->constants[ci].value;
            }
        }
        return runtime_get_named_var_bval(rt, nm, nlen);
    }
    
    error_raise(ERR_WHAT, line_num);
    return bval_int(0);
}

static int apply_operator(RuntimeState *rt, int op, BValue *val_stack, int *val_top, KeywordId kw, const char *op_name, int op_name_len, int arg_count, int line_num)
{
    if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV ||
        op == OP_INTDIV || op == OP_MOD || op == OP_POW ||
        op == OP_CMP_EQ || op == OP_CMP_NE || op == OP_CMP_LT ||
        op == OP_CMP_GT || op == OP_CMP_LE || op == OP_CMP_GE ||
        op == OP_LIKE || op == OP_AND || op == OP_OR || op == OP_XOR ||
        op == OP_EQV || op == OP_IMP) {
        
        if (*val_top < 2) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        BValue right = val_stack[--(*val_top)];
        BValue left = val_stack[--(*val_top)];
        BValue res = bval_int(0);
        
        switch (op) {
            case OP_ADD:
                if (bval_is_string(&left) && bval_is_string(&right)) {
                    if (dialect_is_strict()) {
                        error_raise(ERR_WHAT, line_num);
                        return 0;
                    }
                    res = bval_concat(&left, &right, line_num, &rt->strpool);
                } else {
                    res = bval_add(&left, &right, line_num);
                }
                break;
            case OP_SUB: res = bval_sub(&left, &right, line_num); break;
            case OP_MUL: res = bval_mul(&left, &right, line_num); break;
            case OP_DIV: res = bval_div(&left, &right, line_num); break;
            case OP_INTDIV: {
                long a = bval_to_int(&left);
                long b = bval_to_int(&right);
                if (b == 0) {
                    error_raise(ERR_HOW, line_num);
                    return 0;
                }
                res = bval_int(a / b);
                break;
            }
            case OP_MOD: res = bval_mod(&left, &right, line_num); break;
            case OP_POW: {
                double base_d = bval_to_float(&left);
                double exp_d = bval_to_float(&right);
                res = bval_float(pow(base_d, exp_d));
                break;
            }
            case OP_AND: res = bval_int(bval_to_int(&left) & bval_to_int(&right)); break;
            case OP_OR:  res = bval_int(bval_to_int(&left) | bval_to_int(&right)); break;
            case OP_XOR: res = bval_int(bval_to_int(&left) ^ bval_to_int(&right)); break;
            case OP_EQV: res = bval_int(~(bval_to_int(&left) ^ bval_to_int(&right))); break;
            case OP_IMP: res = bval_int((~bval_to_int(&left)) | bval_to_int(&right)); break;
            case OP_LIKE: {
                if (!bval_is_string(&left) || !bval_is_string(&right)) {
                    error_raise(ERR_HOW, line_num);
                    return 0;
                }
                const char *s = left.v.sval.data;
                int sl = left.v.sval.length;
                const char *p = right.v.sval.data;
                int pl = right.v.sval.length;
                if (!s) { s = ""; sl = 0; }
                if (!p) { p = ""; pl = 0; }
                int si = 0, pi = 0, star_pi = -1, star_si = -1, match = 0;
                while (si < sl) {
                    if (pi < pl && p[pi] == '*') {
                        star_pi = pi++; star_si = si;
                    } else if (pi < pl && p[pi] == '?') {
                        si++; pi++;
                    } else if (pi < pl && p[pi] == '#') {
                        if (s[si] >= '0' && s[si] <= '9') { si++; pi++; }
                        else if (star_pi >= 0) { pi = star_pi + 1; si = ++star_si; }
                        else break;
                    } else if (pi < pl && p[pi] == '[') {
                        int neg = 0, found = 0;
                        int ci = pi + 1;
                        if (ci < pl && p[ci] == '!') { neg = 1; ci++; }
                        while (ci < pl && p[ci] != ']') {
                            if (p[ci] == s[si]) found = 1;
                            ci++;
                        }
                        if (ci < pl) ci++;
                        if (found != neg) { pi = ci; si++; }
                        else if (star_pi >= 0) { pi = star_pi + 1; si = ++star_si; }
                        else break;
                    } else if (pi < pl && (p[pi] == s[si] || (tolower((unsigned char)p[pi]) == tolower((unsigned char)s[si])))) {
                        si++; pi++;
                    } else if (star_pi >= 0) {
                        pi = star_pi + 1; si = ++star_si;
                    } else break;
                }
                while (pi < pl && p[pi] == '*') pi++;
                if (si == sl && pi == pl) match = 1;
                res = bval_int(match ? -1 : 0);
                break;
            }
            default: {
                // Relational comparisons
                int result = 0;
                if (bval_is_string(&left) && bval_is_string(&right)) {
                    const char *ld = left.v.sval.data;
                    int ll = left.v.sval.length;
                    const char *rd = right.v.sval.data;
                    int rl = right.v.sval.length;
                    if (!ld) { ld = ""; ll = 0; }
                    if (!rd) { rd = ""; rl = 0; }
                    int minlen = ll < rl ? ll : rl;
                    int cmp = memcmp(ld, rd, (size_t)minlen);
                    if (cmp == 0) {
                        if (ll < rl) cmp = -1;
                        else if (ll > rl) cmp = 1;
                    }
                    switch (op) {
                        case OP_CMP_EQ: result = (cmp == 0); break;
                        case OP_CMP_NE: result = (cmp != 0); break;
                        case OP_CMP_LT: result = (cmp < 0); break;
                        case OP_CMP_GT: result = (cmp > 0); break;
                        case OP_CMP_LE: result = (cmp <= 0); break;
                        case OP_CMP_GE: result = (cmp >= 0); break;
                    }
                } else if (left.type == VAL_FLOAT || right.type == VAL_FLOAT) {
                    double lv = bval_to_float(&left);
                    double rv = bval_to_float(&right);
                    switch (op) {
                        case OP_CMP_EQ: result = (lv == rv); break;
                        case OP_CMP_NE: result = (lv != rv); break;
                        case OP_CMP_LT: result = (lv < rv); break;
                        case OP_CMP_GT: result = (lv > rv); break;
                        case OP_CMP_LE: result = (lv <= rv); break;
                        case OP_CMP_GE: result = (lv >= rv); break;
                    }
                } else {
                    long lv = bval_to_int(&left);
                    long rv = bval_to_int(&right);
                    switch (op) {
                        case OP_CMP_EQ: result = (lv==rv); break;
                        case OP_CMP_NE: result = (lv!=rv); break;
                        case OP_CMP_LT: result = (lv<rv); break;
                        case OP_CMP_GT: result = (lv>rv); break;
                        case OP_CMP_LE: result = (lv<=rv); break;
                        case OP_CMP_GE: result = (lv>=rv); break;
                    }
                }
                res = bval_int(result ? -1 : 0);
                break;
            }
        }
        if (error_occurred()) return 0;
        val_stack[(*val_top)++] = res;
    } else if (op == OP_UNARY_MINUS) {
        if (*val_top < 1) { error_raise(ERR_WHAT, line_num); return 0; }
        BValue val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = bval_neg(&val, line_num);
    } else if (op == OP_UNARY_PLUS) {
        // no-op
    } else if (op == OP_NOT) {
        if (*val_top < 1) { error_raise(ERR_WHAT, line_num); return 0; }
        BValue val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = bval_int(~bval_to_int(&val));
    } else if (op == OP_AT) {
        if (*val_top < 1) { error_raise(ERR_WHAT, line_num); return 0; }
        BValue val = val_stack[--(*val_top)];
        val_stack[(*val_top)++] = bval_int(runtime_get_array(rt, bval_to_int(&val)));
    } else if (op == OP_FUNC) {
        BValue args[16];
        if (*val_top < arg_count || arg_count > 16) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        for (int i = arg_count - 1; i >= 0; i--) {
            args[i] = val_stack[--(*val_top)];
        }
        const FunctionEntry *fn = funcreg_find_by_keyword(kw);
        if (fn == NULL) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        if (arg_count < fn->min_args || arg_count > fn->max_args) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        val_stack[(*val_top)++] = fn->handler(args, arg_count, rt);
    } else if (op == OP_ARRAY) {
        BValue args[3];
        if (*val_top < arg_count || arg_count > 3) {
            error_raise(ERR_WHAT, line_num);
            return 0;
        }
        for (int i = arg_count - 1; i >= 0; i--) {
            args[i] = val_stack[--(*val_top)];
        }
        int idx1 = arg_count >= 1 ? (int)bval_to_subscript(&args[0]) : 0;
        int idx2 = arg_count >= 2 ? (int)bval_to_subscript(&args[1]) : 0;
        int idx3 = arg_count >= 3 ? (int)bval_to_subscript(&args[2]) : 0;
        
        char name_buf[MAX_VAR_NAME_LEN + 1];
        int nlen = 0;
        if (op_name_len > 0) {
            nlen = op_name_len < MAX_VAR_NAME_LEN ? op_name_len : MAX_VAR_NAME_LEN;
            memcpy(name_buf, op_name, (size_t)nlen);
            name_buf[nlen] = '\0';
        } else {
            if (kw < 256) {
                name_buf[0] = (char)kw;
                name_buf[1] = '\0';
                nlen = 1;
            } else {
                name_buf[0] = (char)(kw & 0xFF);
                name_buf[1] = (char)((kw >> 8) & 0xFF);
                name_buf[2] = '\0';
                nlen = 2;
            }
        }
        
        val_stack[(*val_top)++] = runtime_get_dim(rt, name_buf, nlen, idx1, idx2, idx3, line_num);
    }
    
    return 1;
}

static int is_rnd_argument_start(Lexer *lex) {
    TokenType t = lex->current.type;
    return (t == TOK_NUMBER || t == TOK_FLOAT_LIT || t == TOK_VARIABLE ||
            t == TOK_NAMED_VAR || t == TOK_LPAREN || t == TOK_MINUS || t == TOK_PLUS);
}

BValue parse_expression_bval_internal(Lexer *lex, RuntimeState *rt, int line_num, int stop_at_comparisons)
{
    BValue val_stack[32];
    int val_top = 0;
    ParseOp op_stack[32];
    int op_top = 0;
    int arg_count_stack[32];
    int arg_count_top = 0;
    int expect_operand = 1;

    if (error_occurred()) return bval_int(0);

    while (!error_occurred()) {
        Token tok = lex->current;

        // Stop conditions when expecting operator
        if (!expect_operand) {
            if (tok.type == TOK_CR || tok.type == TOK_EOF || tok.type == TOK_COLON) {
                break;
            }
            if (tok.type == TOK_KEYWORD) {
                KeywordId kw = tok.value.keyword;
                if (kw != KW_AND && kw != KW_OR && kw != KW_XOR && kw != KW_EQV && kw != KW_IMP && kw != KW_MOD && kw != KW_LIKE) {
                    break;
                }
            }
            if (tok.type == TOK_COMMA || tok.type == TOK_RPAREN) {
                int has_lparen = 0;
                for (int i = 0; i < op_top; i++) {
                    if (op_stack[i].op == OP_LPAREN) {
                        has_lparen = 1; break;
                    }
                }
                if (!has_lparen) break;
            }
            
            // Check for comparison operator stop condition
            int is_cmp = 0;
            if (tok.type == TOK_EQUALS || tok.type == TOK_NOT_EQ || tok.type == TOK_LT ||
                tok.type == TOK_GT || tok.type == TOK_LT_EQ || tok.type == TOK_GT_EQ) {
                is_cmp = 1;
            } else if (tok.type == TOK_KEYWORD && tok.value.keyword == KW_LIKE) {
                is_cmp = 1;
            }
            if (is_cmp && stop_at_comparisons) {
                break;
            }
        }

        if (expect_operand) {
            // Parse Operand or Unary Operator
            if (tok.type == TOK_NUMBER) {
                val_stack[val_top++] = bval_int(tok.value.num_value);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_FLOAT_LIT) {
                if (!dialect_check_feature("floating point", dialect_get_config()->has_float, line_num)) {
                    val_stack[val_top++] = bval_int((long)tok.value.fval);
                } else {
                    val_stack[val_top++] = bval_float(tok.value.fval);
                }
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_IMAGINARY) {
                val_stack[val_top++] = bval_complex(0.0, tok.value.fval);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_STRING) {
                char *ptr = strpool_store(&rt->strpool, tok.str_start, tok.str_length);
                val_stack[val_top++] = bval_string(ptr, tok.str_length);
                lexer_next(lex);
                expect_operand = 0;
            } else if (tok.type == TOK_PLUS) {
                ParseOp op_entry;
                memset(&op_entry, 0, sizeof(op_entry));
                op_entry.op = OP_UNARY_PLUS;
                op_entry.precedence = 12;
                op_entry.assoc = 1;
                op_entry.is_unary = 1;
                op_stack[op_top++] = op_entry;
                lexer_next(lex);
            } else if (tok.type == TOK_MINUS) {
                ParseOp op_entry;
                memset(&op_entry, 0, sizeof(op_entry));
                op_entry.op = OP_UNARY_MINUS;
                op_entry.precedence = 12;
                op_entry.assoc = 1;
                op_entry.is_unary = 1;
                op_stack[op_top++] = op_entry;
                lexer_next(lex);
            } else if (tok.type == TOK_KEYWORD && tok.value.keyword == KW_NOT) {
                ParseOp op_entry;
                memset(&op_entry, 0, sizeof(op_entry));
                op_entry.op = OP_NOT;
                op_entry.precedence = 6;
                op_entry.assoc = 1;
                op_entry.is_unary = 1;
                op_stack[op_top++] = op_entry;
                lexer_next(lex);
            } else if (tok.type == TOK_LPAREN) {
                // Peek for complex literal (real +/- imagi)
                int parsed_complex = 0;
                Lexer peek = *lex;
                lexer_next(&peek);
                if (peek.current.type == TOK_NUMBER || peek.current.type == TOK_FLOAT_LIT) {
                    lexer_next(&peek);
                    if (peek.current.type == TOK_PLUS || peek.current.type == TOK_MINUS) {
                        int neg = (peek.current.type == TOK_MINUS);
                        lexer_next(&peek);
                        if (peek.current.type == TOK_IMAGINARY) {
                            double imag = peek.current.value.fval;
                            if (neg) imag = -imag;
                            lexer_next(&peek);
                            if (peek.current.type == TOK_RPAREN) {
                                lexer_next(lex); // consume (
                                double real = 0.0;
                                if (lex->current.type == TOK_NUMBER) real = (double)lex->current.value.num_value;
                                else real = lex->current.value.fval;
                                lexer_next(lex); // consume real
                                lexer_next(lex); // consume +/-
                                lexer_next(lex); // consume imaginary
                                lexer_next(lex); // consume )
                                val_stack[val_top++] = bval_complex(real, imag);
                                expect_operand = 0;
                                parsed_complex = 1;
                            }
                        }
                    }
                }
                if (!parsed_complex) {
                    ParseOp op_entry;
                    memset(&op_entry, 0, sizeof(op_entry));
                    op_entry.op = OP_LPAREN;
                    op_stack[op_top++] = op_entry;
                    lexer_next(lex);
                }
            } else if (tok.type == TOK_AT) {
                lexer_next(lex);
                if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                ParseOp op_entry_at;
                memset(&op_entry_at, 0, sizeof(op_entry_at));
                op_entry_at.op = OP_AT;
                op_entry_at.precedence = 14;
                op_entry_at.assoc = 1;
                op_entry_at.is_unary = 1;
                op_stack[op_top++] = op_entry_at;
                
                ParseOp op_entry_lp;
                memset(&op_entry_lp, 0, sizeof(op_entry_lp));
                op_entry_lp.op = OP_LPAREN;
                op_stack[op_top++] = op_entry_lp;
            } else if (tok.type == TOK_VARIABLE || tok.type == TOK_STRING_VAR || tok.type == TOK_NAMED_VAR) {
                // Peek if followed by (
                Lexer peek = *lex;
                lexer_next(&peek);
                if (peek.current.type == TOK_LPAREN) {
                    char name_buf[32];
                    int nlen = 0;
                    KeywordId kw = 0;
                    if (tok.type == TOK_VARIABLE) {
                        kw = (KeywordId)tok.value.var_name;
                        name_buf[0] = tok.value.var_name;
                        name_buf[1] = '\0';
                        nlen = 1;
                    } else if (tok.type == TOK_STRING_VAR) {
                        kw = (KeywordId)tok.value.var_name | ('$' << 8);
                        name_buf[0] = tok.value.var_name;
                        name_buf[1] = '$';
                        name_buf[2] = '\0';
                        nlen = 2;
                    } else {
                        nlen = tok.str_length < 31 ? tok.str_length : 31;
                        memcpy(name_buf, tok.str_start, (size_t)nlen);
                        name_buf[nlen] = '\0';
                    }
                    
                    // 1. Check if user-defined FUNCTION
                    SubDef *sd = runtime_find_sub(rt, name_buf, nlen);
                    if (sd != NULL && sd->is_external && sd->body_index == -1) {
                        extern int runtime_load_external_sub(RuntimeState *rt, SubDef *sd);
                        if (runtime_load_external_sub(rt, sd) != 0) {
                            error_raise(ERR_HOW, line_num);
                            return bval_float(0.0);
                        }
                    }
                    if (sd != NULL && sd->is_function) {
                        ParseOp op_entry;
                        memset(&op_entry, 0, sizeof(op_entry));
                        op_entry.op = OP_USER_FUNC;
                        op_entry.precedence = 14;
                        op_entry.kw = 0;
                        memcpy(op_entry.name, name_buf, (size_t)nlen + 1);
                        op_entry.name_len = nlen;
                        op_stack[op_top++] = op_entry;
                        
                        lexer_next(lex); // consume variable name
                        lexer_next(lex); // consume (
                        ParseOp lp_entry;
                        memset(&lp_entry, 0, sizeof(lp_entry));
                        lp_entry.op = OP_LPAREN;
                        op_stack[op_top++] = lp_entry;
                        arg_count_stack[arg_count_top++] = 1;
                    } else {
                        // 2. Check single-line DEF FN
                        int is_def_fn = 0;
                        UserFunction *ufn = NULL;
                        if (nlen >= 3 && (name_buf[0] == 'F' || name_buf[0] == 'f') && (name_buf[1] == 'N' || name_buf[1] == 'n')) {
                            char fn_ch = name_buf[2];
                            char fn_buf[2];
                            if (fn_ch >= 'a' && fn_ch <= 'z') fn_ch = (char)(fn_ch - 32);
                            fn_buf[0] = fn_ch; fn_buf[1] = '\0';
                            ufn = runtime_find_fn(rt, fn_buf, 1);
                            if (ufn != NULL) {
                                is_def_fn = 1;
                            }
                        }
                        
                        if (is_def_fn) {
                            lexer_next(lex); // consume FNx
                            lexer_next(lex); // consume (
                            BValue args[MAX_FN_PARAMS];
                            BValue saved[MAX_FN_PARAMS];
                            int ac = 0, pi;
                            if (ufn->param_count > 0) {
                                args[ac++] = parse_expression_bval_internal(lex, rt, line_num, 0);
                                if (error_occurred()) return bval_int(0);
                                while (ac < ufn->param_count && lex->current.type == TOK_COMMA) {
                                    lexer_next(lex);
                                    args[ac++] = parse_expression_bval_internal(lex, rt, line_num, 0);
                                    if (error_occurred()) return bval_int(0);
                                }
                            }
                            if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                            for (pi = 0; pi < ufn->param_count; pi++) {
                                int vi = ufn->params[pi] - 'A';
                                saved[pi] = rt->variables[vi];
                                if (pi < ac) rt->variables[vi] = args[pi];
                            }
                            Lexer bl;
                            lexer_init(&bl, ufn->body);
                            BValue res = parse_expression_bval_internal(&bl, rt, line_num, 0);
                            for (pi = 0; pi < ufn->param_count; pi++) {
                                int vi = ufn->params[pi] - 'A';
                                rt->variables[vi] = saved[pi];
                            }
                            val_stack[val_top++] = res;
                            expect_operand = 0;
                        } else {
                            // 3. Try library function
                            extern int lib_space_try_call_func(const char *name, int name_len,
                                                               void *lex_ptr, void *rt_ptr,
                                                               int line_num, void *out_result);
                            BValue res;
                            Lexer saved_lex = *lex;
                            lexer_next(lex); // consume variable name (points to '(')
                            if (lib_space_try_call_func(name_buf, nlen, lex, rt, line_num, &res)) {
                                val_stack[val_top++] = res;
                                expect_operand = 0;
                            } else {
                                // 4. Fallback to array read
                                *lex = saved_lex;
                                ParseOp op_entry;
                                memset(&op_entry, 0, sizeof(op_entry));
                                op_entry.op = OP_ARRAY;
                                op_entry.precedence = 14;
                                op_entry.kw = kw;
                                memcpy(op_entry.name, name_buf, (size_t)nlen + 1);
                                op_entry.name_len = nlen;
                                op_stack[op_top++] = op_entry;
                                
                                lexer_next(lex); // consume name
                                lexer_next(lex); // consume (
                                ParseOp lp_entry;
                                memset(&lp_entry, 0, sizeof(lp_entry));
                                lp_entry.op = OP_LPAREN;
                                op_stack[op_top++] = lp_entry;
                                arg_count_stack[arg_count_top++] = 1;
                            }
                        }
                    }
                } else {
                    val_stack[val_top++] = evaluate_variable_simple(lex, rt, tok, line_num);
                    expect_operand = 0;
                }
            } else if (tok.type == TOK_KEYWORD) {
                KeywordId kw = tok.value.keyword;
                if (kw == KW_FN) {
                    Lexer saved = *lex;
                    lexer_next(lex); // consume FN
                    if (lex->current.type != TOK_VARIABLE) {
                        error_raise(ERR_WHAT, line_num);
                        return bval_int(0);
                    }
                    char fn_name = lex->current.value.var_name;
                    lexer_next(lex); // consume name
                    
                    char name_buf[4];
                    name_buf[0] = 'F'; name_buf[1] = 'N'; name_buf[2] = fn_name; name_buf[3] = '\0';
                    SubDef *sd = runtime_find_sub(rt, name_buf, 3);
                    if (sd != NULL && sd->is_function && sd->body_index >= 0) {
                        if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                        ParseOp op_entry;
                        memset(&op_entry, 0, sizeof(op_entry));
                        op_entry.op = OP_USER_FUNC;
                        op_entry.precedence = 14;
                        op_entry.kw = (KeywordId)fn_name;
                        op_entry.name[0] = 'F'; op_entry.name[1] = 'N'; op_entry.name[2] = fn_name; op_entry.name[3] = '\0';
                        op_entry.name_len = 3;
                        op_stack[op_top++] = op_entry;
                        
                        ParseOp lp_entry;
                        memset(&lp_entry, 0, sizeof(lp_entry));
                        lp_entry.op = OP_LPAREN;
                        op_stack[op_top++] = lp_entry;
                        arg_count_stack[arg_count_top++] = 1;
                    } else {
                        *lex = saved; // restore FN
                        val_stack[val_top++] = pi_eval_user_fn(lex, rt, line_num);
                        expect_operand = 0;
                    }
                } else if (kw == KW_LBOUND || kw == KW_UBOUND) {
                    int is_upper = (kw == KW_UBOUND);
                    char aname[MAX_VAR_NAME_LEN + 1];
                    int alen = 0, dim_arg = 1;
                    lexer_next(lex); // consume bounds kw
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    if (lex->current.type == TOK_VARIABLE) {
                        aname[0] = lex->current.value.var_name; aname[1] = '\0'; alen = 1; lexer_next(lex);
                    } else if (lex->current.type == TOK_NAMED_VAR) {
                        alen = lex->current.str_length;
                        if (alen > MAX_VAR_NAME_LEN) alen = MAX_VAR_NAME_LEN;
                        memcpy(aname, lex->current.str_start, (size_t)alen); aname[alen] = '\0'; lexer_next(lex);
                    } else {
                        error_raise(ERR_WHAT, line_num); return bval_int(0);
                    }
                    for (int i = 0; i < alen; i++) {
                        if (aname[i] >= 'a' && aname[i] <= 'z') aname[i] = (char)(aname[i] - 32);
                    }
                    if (lex->current.type == TOK_COMMA) {
                        lexer_next(lex);
                        BValue dim_val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        dim_arg = (int)bval_to_int(&dim_val);
                    }
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = runtime_get_dim(rt, aname, alen, is_upper ? -2 : -1, dim_arg, 0, line_num);
                    expect_operand = 0;
                } else if (kw == KW_DET) {
                    char aname[MAX_VAR_NAME_LEN + 1];
                    int alen = 0, i, n, p, r;
                    lexer_next(lex); // consume DET
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_float(0.0);
                    if (lex->current.type == TOK_VARIABLE) {
                        aname[0] = lex->current.value.var_name; aname[1] = '\0'; alen = 1; lexer_next(lex);
                    } else if (lex->current.type == TOK_NAMED_VAR) {
                        alen = lex->current.str_length;
                        if (alen > MAX_VAR_NAME_LEN) alen = MAX_VAR_NAME_LEN;
                        memcpy(aname, lex->current.str_start, (size_t)alen); aname[alen] = '\0'; lexer_next(lex);
                    } else {
                        error_raise(ERR_WHAT, line_num); return bval_float(0.0);
                    }
                    for (i = 0; i < alen; i++) {
                        if (aname[i] >= 'a' && aname[i] <= 'z') aname[i] = (char)(aname[i] - 32);
                    }
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_float(0.0);
                    DimArray *arr = runtime_find_dim(rt, aname, alen);
                    if (arr == NULL || arr->dims != 2 || arr->size[0] != arr->size[1]) {
                        error_raise(ERR_HOW, line_num); return bval_float(0.0);
                    }
                    n = arr->size[0] - 1;
                    if (n > 15 || n < 1) { error_raise(ERR_SORRY, line_num); return bval_float(0.0); }
                    double work[16][16];
                    for (r = 0; r < n; r++) {
                        for (int c = 0; c < n; c++) {
                            BValue v = arr->elements[(r + 1) * arr->size[1] + (c + 1)];
                            work[r][c] = bval_to_float(&v);
                        }
                    }
                    double det_val = 1.0;
                    int sign = 1;
                    for (p = 0; p < n; p++) {
                        int max_row = p;
                        double max_val = work[p][p] < 0 ? -work[p][p] : work[p][p];
                        for (r = p + 1; r < n; r++) {
                            double v = work[r][p] < 0 ? -work[r][p] : work[r][p];
                            if (v > max_val) { max_val = v; max_row = r; }
                        }
                        if (max_row != p) {
                            for (int c = 0; c < n; c++) {
                                double tmp = work[p][c]; work[p][c] = work[max_row][c]; work[max_row][c] = tmp;
                            }
                            sign = -sign;
                        }
                        double pivot = work[p][p];
                        if (pivot > -1e-12 && pivot < 1e-12) { det_val = 0.0; break; }
                        det_val *= pivot;
                        for (r = p + 1; r < n; r++) {
                            double factor = work[r][p] / pivot;
                            for (int c = p + 1; c < n; c++) work[r][c] -= factor * work[p][c];
                        }
                    }
                    val_stack[val_top++] = bval_float(det_val * sign);
                    expect_operand = 0;
                } else if (kw == KW_VARPTR || kw == KW_VARPTR_STR) {
                    int is_str = (kw == KW_VARPTR_STR);
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        if (is_str) val_stack[val_top++] = bval_string("", 0);
                        else val_stack[val_top++] = bval_int(0);
                    } else {
                        TokenType vt = lex->current.type;
                        if (vt == TOK_VARIABLE || vt == TOK_STRING_VAR || vt == TOK_NAMED_VAR) {
                            char name[MAX_VAR_NAME_LEN + 1];
                            int len = 0;
                            if (vt == TOK_VARIABLE || vt == TOK_STRING_VAR) {
                                name[0] = lex->current.value.var_name;
                                if (vt == TOK_STRING_VAR) { name[1] = '$'; name[2] = '\0'; len = 2; }
                                else { name[1] = '\0'; len = 1; }
                            } else {
                                len = lex->current.str_length < MAX_VAR_NAME_LEN ? lex->current.str_length : MAX_VAR_NAME_LEN;
                                memcpy(name, lex->current.str_start, (size_t)len);
                                name[len] = '\0';
                            }
                            lexer_next(lex); // consume name
                            
                            int is_array = 0;
                            int idx1 = 0, idx2 = 0, idx3 = 0;
                            if (lex->current.type == TOK_LPAREN) {
                                lexer_next(lex); // consume (
                                BValue t1 = parse_expression_bval_internal(lex, rt, line_num, 0);
                                idx1 = (int)bval_to_int(&t1);
                                if (lex->current.type == TOK_COMMA) {
                                    lexer_next(lex);
                                    BValue t2 = parse_expression_bval_internal(lex, rt, line_num, 0);
                                    idx2 = (int)bval_to_int(&t2);
                                    if (lex->current.type == TOK_COMMA) {
                                        lexer_next(lex);
                                        BValue t3 = parse_expression_bval_internal(lex, rt, line_num, 0);
                                        idx3 = (int)bval_to_int(&t3);
                                    }
                                }
                                lexer_expect(lex, TOK_RPAREN);
                                is_array = 1;
                            }
                            lexer_expect(lex, TOK_RPAREN);
                            
                            uint32_t addr = 0;
                            uint8_t type_byte = 2;
                            if (is_array) {
                                DimArray *arr = NULL;
                                for (int i = 0; i < rt->dim_count; i++) {
                                    if (pi_str_case_equal(name, rt->dim_arrays[i].name)) {
                                        arr = &rt->dim_arrays[i]; break;
                                    }
                                }
                                if (arr != NULL) {
                                    int flat_idx = 0;
                                    if (arr->dims == 1) flat_idx = idx1 - rt->option_base;
                                    else if (arr->dims == 2) flat_idx = (idx1 - rt->option_base) * arr->size[1] + (idx2 - rt->option_base);
                                    else if (arr->dims == 3) flat_idx = ((idx1 - rt->option_base) * arr->size[1] + (idx2 - rt->option_base)) * arr->size[2] + (idx3 - rt->option_base);
                                    
                                    if (flat_idx >= 0 && flat_idx < arr->total) {
                                        int element_offset = (int)(arr->elements - rt->dim_elements) + flat_idx;
                                        addr = (uint32_t)(0x10000 + element_offset * 8);
                                    }
                                }
                                int suffix = (len > 0) ? name[len - 1] : 0;
                                if (suffix == '%') type_byte = 2;
                                else if (suffix == '!') type_byte = 4;
                                else if (suffix == '#') type_byte = 8;
                                else if (suffix == '$') type_byte = 3;
                            } else {
                                if (len == 1 && name[0] >= 'A' && name[0] <= 'Z') {
                                    addr = (uint32_t)(0x7000 + (name[0] - 'A') * 8);
                                    unsigned char dtype = rt->deftype_map[name[0] - 'A'];
                                    if (dtype == DEFTYPE_INT) type_byte = 2;
                                    else if (dtype == DEFTYPE_SNG) type_byte = 4;
                                    else if (dtype == DEFTYPE_DBL) type_byte = 8;
                                    else if (dtype == DEFTYPE_STR) type_byte = 3;
                                } else if (len == 2 && name[0] >= 'A' && name[0] <= 'Z' && name[1] == '$') {
                                    addr = (uint32_t)(0x7000 + (26 + (name[0] - 'A')) * 8);
                                    type_byte = 3;
                                } else {
                                    int named_idx = -1;
                                    for (int i = 0; i < rt->named_count; i++) {
                                        if (pi_str_case_equal(name, rt->named_vars[i].name)) {
                                            named_idx = i; break;
                                        }
                                    }
                                    if (named_idx != -1) {
                                        addr = (uint32_t)(0x7000 + (52 + named_idx) * 8);
                                        char last = name[len - 1];
                                        if (last == '%') type_byte = 2;
                                        else if (last == '!') type_byte = 4;
                                        else if (last == '#') type_byte = 8;
                                        else if (last == '$') type_byte = 3;
                                    }
                                }
                            }
                            
                            if (is_str) {
                                char desc[4];
                                desc[0] = (char)type_byte;
                                desc[1] = (char)(addr & 0xFF);
                                desc[2] = (char)((addr >> 8) & 0xFF);
                                desc[3] = '\0';
                                char *pool_str = strpool_store(&rt->strpool, desc, 3);
                                val_stack[val_top++] = bval_string(pool_str, 3);
                            } else {
                                val_stack[val_top++] = bval_int((long)addr);
                            }
                        } else {
                            lexer_skip_to_end(lex);
                            if (is_str) val_stack[val_top++] = bval_string("", 0);
                            else val_stack[val_top++] = bval_int(0);
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_TICKS) {
                    val_stack[val_top++] = bval_float(vdev_get_time());
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_TIMER) {
                    double remain = 0.0;
                    if (rt->timer_interval > 0.0) {
                        double elapsed = vdev_get_time() - rt->timer_last_fire;
                        remain = rt->timer_interval - elapsed;
                        if (remain < 0.0) remain = 0.0;
                    }
                    val_stack[val_top++] = bval_float(remain);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_DATE_FUNC) {
                    char buf[16];
                    time_t t = time(NULL);
                    struct tm *tm = localtime(&t);
                    sprintf(buf, "%02d-%02d-%04d", tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900);
                    char *ptr = strpool_store(&rt->strpool, buf, 10);
                    val_stack[val_top++] = bval_string(ptr, 10);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_TIME_FUNC) {
                    char buf[16];
                    time_t t = time(NULL);
                    struct tm *tm = localtime(&t);
                    sprintf(buf, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
                    char *ptr = strpool_store(&rt->strpool, buf, 8);
                    val_stack[val_top++] = bval_string(ptr, 8);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_CLOCK_FUNC) {
                    char buf[64];
                    time_t t = time(NULL);
                    struct tm *tm = localtime(&t);
                    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    int len = 19;
                    char *ptr = strpool_store(&rt->strpool, buf, len);
                    val_stack[val_top++] = bval_string(ptr, len);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_ALARM_FUNC) {
                    int len = (int)strlen(rt->alarm_str);
                    if (len == 0) {
                        val_stack[val_top++] = bval_string(NULL, 0);
                    } else {
                        char *ptr = strpool_store(&rt->strpool, rt->alarm_str, len);
                        val_stack[val_top++] = bval_string(ptr, len);
                    }
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_DIALECT_FUNC) {
                    const char *dname = dialect_get_short_name();
                    int len = (int)strlen(dname);
                    char *ptr = strpool_store(&rt->strpool, dname, len);
                    val_stack[val_top++] = bval_string(ptr, len);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_MEMMAP_FUNC) {
                    const char *mname = memmap_get_name((MemMapType)rt->memmap_type);
                    int len = (int)strlen(mname);
                    char *ptr = strpool_store(&rt->strpool, mname, len);
                    val_stack[val_top++] = bval_string(ptr, len);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_VPATH_FUNC) {
                    const char *vpath = vfs_get_vpath();
                    if (vpath == NULL) vpath = "";
                    int len = (int)strlen(vpath);
                    char *ptr = strpool_store(&rt->strpool, vpath, len);
                    val_stack[val_top++] = bval_string(ptr, len);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_CWD_FUNC || kw == KW_PWD) {
                    char cwdbuf[512];
#ifdef _WIN32
                    if (_getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#else
                    if (getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#endif
                    if (kw == KW_CWD_FUNC) {
                        char *last_comp = cwdbuf;
                        for (int i = 0; cwdbuf[i] != '\0'; i++) {
                            if (cwdbuf[i] == '/' || cwdbuf[i] == '\\') {
                                if (cwdbuf[i+1] != '\0') last_comp = &cwdbuf[i+1];
                            }
                        }
                        int len = (int)strlen(last_comp);
                        char *ptr = strpool_store(&rt->strpool, last_comp, len);
                        val_stack[val_top++] = bval_string(ptr, len);
                    } else {
                        int len = (int)strlen(cwdbuf);
                        char *ptr = strpool_store(&rt->strpool, cwdbuf, len);
                        val_stack[val_top++] = bval_string(ptr, len);
                    }
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_ALIAS_FUNC) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    BValue arg = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    if (!bval_is_string(&arg)) {
                        val_stack[val_top++] = bval_string(NULL, 0);
                    } else {
                        const char *result = lexer_find_alias_by_name(arg.v.sval.data, arg.v.sval.length);
                        if (result == NULL) {
                            int ki;
                            KeywordId found_kw = KW_COUNT;
                            for (ki = 0; ki < (int)KW_COUNT; ki++) {
                                const char *kname = lexer_keyword_name((KeywordId)ki);
                                if (kname[0] != '\0') {
                                    int klen = (int)strlen(kname);
                                    if (klen == arg.v.sval.length) {
                                        int j, m = 1;
                                        for (j = 0; j < klen; j++) {
                                            char ca = arg.v.sval.data[j];
                                            char cb = kname[j];
                                            if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
                                            if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
                                            if (ca != cb) { m = 0; break; }
                                        }
                                        if (m) { found_kw = (KeywordId)ki; break; }
                                    }
                                }
                            }
                            if (found_kw != KW_COUNT) {
                                result = lexer_find_alias_for_keyword(found_kw);
                            }
                        }
                        if (result == NULL) {
                            val_stack[val_top++] = bval_string(NULL, 0);
                        } else {
                            int len = (int)strlen(result);
                            char *ptr = strpool_store(&rt->strpool, result, len);
                            val_stack[val_top++] = bval_string(ptr, len);
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_CINT) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    double v = bval_to_float(&val);
                    val_stack[val_top++] = bval_int((long)(v >= 0 ? v + 0.5 : v - 0.5));
                    expect_operand = 0;
                } else if (kw == KW_CSNG || kw == KW_CDBL) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_float(bval_to_float(&val));
                    expect_operand = 0;
                } else if (kw == KW_CSRLIN) {
                    val_stack[val_top++] = bval_int((long)rt->cursor_row);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_ERL) {
                    val_stack[val_top++] = bval_int((long)rt->last_err_line);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_ERR_VAR) {
                    val_stack[val_top++] = bval_int((long)rt->last_err_code);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_EXTERR) {
                    lexer_next(lex);
                    if (lex->current.type == TOK_LPAREN) {
                        lexer_next(lex);
                        (void)parse_expression(lex, rt, line_num);
                        if (error_occurred()) return bval_int(0);
                        if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    }
                    val_stack[val_top++] = bval_int(0);
                    expect_operand = 0;
                } else if (kw == KW_ERDEV) {
                    val_stack[val_top++] = bval_int(0);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_FRE) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    long arg = 0;
                    if (lex->current.type == TOK_STRING || lex->current.type == TOK_STRING_VAR) {
                        lexer_next(lex);
                    } else {
                        arg = parse_expression(lex, rt, line_num);
                        if (error_occurred()) return bval_int(0);
                    }
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    long result = 0;
                    if (arg == 0) result = rt->strpool.size - rt->strpool.used;
                    else if (arg == -1) result = mem_pool_available(&rt->memory->scratch);
                    else if (arg == -2 || arg == -3) result = mem_pool_available(&rt->memory->variable);
                    else result = mem_pool_available(&rt->memory->variable) + (rt->strpool.size - rt->strpool.used) + mem_pool_available(&rt->memory->scratch);
                    val_stack[val_top++] = bval_int(result);
                    expect_operand = 0;
                } else if (kw == KW_EXIST_FUNC) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    BValue arg = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    int res = 0;
                    if (bval_is_string(&arg) && arg.v.sval.length > 0 && arg.v.sval.length < 260 && arg.v.sval.data != NULL) {
                        char fname[260];
                        memcpy(fname, arg.v.sval.data, (size_t)arg.v.sval.length);
                        fname[arg.v.sval.length] = '\0';
                        FILE *fp = fopen(fname, "rb");
                        if (fp != NULL) { fclose(fp); res = 1; }
                    }
                    val_stack[val_top++] = bval_int(res);
                    expect_operand = 0;
                } else if (kw == KW_FILELEN_FUNC) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(-1);
                    BValue arg = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(-1);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(-1);
                    long sz = -1;
                    if (bval_is_string(&arg) && arg.v.sval.length > 0 && arg.v.sval.length < 260 && arg.v.sval.data != NULL) {
                        char fname[260];
                        memcpy(fname, arg.v.sval.data, (size_t)arg.v.sval.length);
                        fname[arg.v.sval.length] = '\0';
                        FILE *fp = fopen(fname, "rb");
                        if (fp != NULL) {
                            fseek(fp, 0L, SEEK_END);
                            sz = ftell(fp);
                            fclose(fp);
                        }
                    }
                    val_stack[val_top++] = bval_int(sz);
                    expect_operand = 0;
                } else if (kw == KW_INP) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    long port = parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    int paddr = (int)(port & 0xFFFF);
                    long res = 0;
                    if (paddr >= 0 && paddr < MAX_MEM_SEGMENT) res = (long)rt->mem_segment[paddr];
                    val_stack[val_top++] = bval_int(res);
                    expect_operand = 0;
                } else if (kw == KW_SHELL) {
                    lexer_next(lex);
                    if (lex->current.type != TOK_LPAREN) {
                        error_raise(ERR_WHAT, line_num);
                        return bval_int(0);
                    }
                    lexer_next(lex);
                    BValue sv = parse_expression_bval_internal(lex, rt, line_num, 0);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    if (!bval_is_string(&sv)) {
                        error_raise(ERR_WHAT, line_num);
                        val_stack[val_top++] = bval_string(NULL, 0);
                    } else {
                        char cmd[512];
                        int cl = sv.v.sval.length < 510 ? sv.v.sval.length : 510;
                        if (sv.v.sval.data) memcpy(cmd, sv.v.sval.data, (size_t)cl);
                        cmd[cl] = '\0';
                        FILE *pp = NULL;
#if !defined(__MSDOS__) && !defined(__DOS__) && !defined(MSDOS)
#ifdef _WIN32
                        pp = _popen(cmd, "r");
#else
                        pp = popen(cmd, "r");
#endif
#endif
                        if (pp == NULL) {
                            val_stack[val_top++] = bval_string(NULL, 0);
                        } else {
                            static char outbuf[32768];
                            int outlen = 0;
                            while (outlen < 32760) {
                                int ch = fgetc(pp);
                                if (ch == EOF) break;
                                outbuf[outlen++] = (char)ch;
                            }
#ifdef _WIN32
                            rt->last_shell_exitcode = _pclose(pp);
#else
                            rt->last_shell_exitcode = pclose(pp);
#endif
                            while (outlen > 0 && (outbuf[outlen - 1] == '\n' || outbuf[outlen - 1] == '\r')) outlen--;
                            char *poolbuf = strpool_alloc(&rt->strpool, outlen);
                            if (poolbuf && outlen > 0) memcpy(poolbuf, outbuf, (size_t)outlen);
                            val_stack[val_top++] = bval_string(poolbuf, outlen);
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_ERRORLEVEL) {
                    val_stack[val_top++] = bval_int((long)rt->last_shell_exitcode);
                    lexer_next(lex);
                    expect_operand = 0;
                } else if (kw == KW_LOC) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    long chan = parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    FILE *fp = fileio_get_fp((int)chan);
                    long res = 0;
                    if (fp) {
                        res = (long)ftell(fp);
                        if (res < 0) res = 0;
                    }
                    val_stack[val_top++] = bval_int(res);
                    expect_operand = 0;
                } else if (kw == KW_LPOS) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    (void)parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_int(0);
                    expect_operand = 0;
                } else if (kw == KW_POS_FUNC) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    (void)parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_int((long)rt->cursor_col);
                    expect_operand = 0;
                } else if (kw == KW_PMAP) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    long coord = parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (lex->current.type == TOK_COMMA) lexer_next(lex);
                    (void)parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_int(coord);
                    expect_operand = 0;
                } else if (kw == KW_PLAY) {
                    lexer_next(lex);
                    if (lex->current.type == TOK_LPAREN) {
                        lexer_next(lex);
                        (void)parse_expression(lex, rt, line_num);
                        if (error_occurred()) return bval_int(0);
                        if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    }
                    val_stack[val_top++] = bval_int(0);
                    expect_operand = 0;
                } else if (kw == KW_STICK) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    (void)parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_int(0);
                    expect_operand = 0;
                } else if (kw == KW_USR) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
                    (void)parse_expression(lex, rt, line_num);
                    if (error_occurred()) return bval_int(0);
                    if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
                    val_stack[val_top++] = bval_int(0);
                    expect_operand = 0;
                } else if (kw == KW_SCREEN) {
                    lexer_next(lex);
                    if (lex->current.type == TOK_LPAREN) {
                        lexer_next(lex);
                        BValue t1 = parse_expression_bval_internal(lex, rt, line_num, 0);
                        int row = (int)bval_to_int(&t1);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(32); }
                        else {
                            if (lex->current.type == TOK_COMMA) lexer_next(lex);
                            BValue t2 = parse_expression_bval_internal(lex, rt, line_num, 0);
                            int col = (int)bval_to_int(&t2);
                            if (error_occurred()) { val_stack[val_top++] = bval_int(32); }
                            else {
                                int flag = 0;
                                if (lex->current.type == TOK_COMMA) {
                                    lexer_next(lex);
                                    BValue t3 = parse_expression_bval_internal(lex, rt, line_num, 0);
                                    flag = (int)bval_to_int(&t3);
                                }
                                if (!lexer_expect(lex, TOK_RPAREN)) {
                                    val_stack[val_top++] = bval_int(32);
                                } else {
#ifndef NO_SDL2
                                    char ch = gw_sdl2_get_char(col - 1, row - 1);
                                    if (flag == 1) {
                                        val_stack[val_top++] = bval_int(7);
                                    } else {
                                        val_stack[val_top++] = bval_int((unsigned char)ch);
                                    }
#else
                                    val_stack[val_top++] = bval_int(32);
#endif
                                }
                            }
                        }
                    } else {
                        val_stack[val_top++] = bval_int(0);
                    }
                    expect_operand = 0;
                } else if (kw == KW_MKI_FUNC || kw == KW_MKS_FUNC || kw == KW_MKD_FUNC) {
                    double mkval;
                    char buf[8];
                    int blen;
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_string("", 0);
                    } else {
                        BValue mkarg = parse_expression_bval_internal(lex, rt, line_num, 0);
                        mkval = bval_to_float(&mkarg);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_string("", 0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_string("", 0);
                            } else {
                                if (kw == KW_MKI_FUNC) {
                                    short sv = (short)(long)mkval;
                                    memcpy(buf, &sv, 2);
                                    blen = 2;
                                } else if (kw == KW_MKS_FUNC) {
                                    if (dialect_get_config()->id == DIALECT_GW_BASIC) {
                                        gw_double_to_mbf32(mkval, (uint8_t *)buf);
                                    } else {
                                        float fv = (float)mkval;
                                        memcpy(buf, &fv, 4);
                                    }
                                    blen = 4;
                                } else {
                                    if (dialect_get_config()->id == DIALECT_GW_BASIC) {
                                        gw_double_to_mbf64(mkval, (uint8_t *)buf);
                                    } else {
                                        memcpy(buf, &mkval, 8);
                                    }
                                    blen = 8;
                                }
                                char *ptr = strpool_store(&rt->strpool, buf, blen);
                                if (ptr) val_stack[val_top++] = bval_string(ptr, blen);
                                else val_stack[val_top++] = bval_string("", 0);
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_INPUT_FUNC) {
                    long nchars;
                    int chan = 0;
                    char buf[256];
                    int i;
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_string("", 0);
                    } else {
                        BValue nchars_val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        nchars = bval_to_int(&nchars_val);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_string("", 0);
                        } else {
                            if (nchars < 1) nchars = 1;
                            if (nchars > 255) nchars = 255;
                            if (lex->current.type == TOK_COMMA) {
                                lexer_next(lex);
                                if (lex->current.type == TOK_HASH) lexer_next(lex);
                                BValue chan_val = parse_expression_bval_internal(lex, rt, line_num, 0);
                                chan = (int)bval_to_int(&chan_val);
                            }
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_string("", 0);
                            } else {
                                if (chan > 0) {
                                    FILE *fp = fileio_get_fp(chan);
                                    for (i = 0; i < (int)nchars; i++) {
                                        int ch;
                                        if (!fp) break;
                                        ch = fgetc(fp);
                                        if (ch == EOF) break;
                                        buf[i] = (char)ch;
                                    }
                                } else {
                                    for (i = 0; i < (int)nchars; i++) {
                                        int ch = getchar();
                                        if (ch == EOF) break;
                                        buf[i] = (char)ch;
                                    }
                                }
                                buf[i] = '\0';
                                char *ptr = strpool_store(&rt->strpool, buf, i);
                                if (!ptr) {
                                    error_raise(ERR_SORRY, line_num);
                                    val_stack[val_top++] = bval_string("", 0);
                                } else {
                                    BValue sv;
                                    sv.type = VAL_STRING;
                                    sv.v.sval.data = ptr;
                                    sv.v.sval.length = i;
                                    val_stack[val_top++] = sv;
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_IOCTL_FUNC) {
                    int chan;
                    int cmode;
                    const char *st;
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_string("", 0);
                    } else {
                        if (lex->current.type == TOK_HASH) lexer_next(lex);
                        BValue chan_val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        chan = (int)bval_to_int(&chan_val);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_string("", 0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_string("", 0);
                            } else {
                                cmode = fileio_get_channel_mode(chan);
                                switch (cmode) {
                                    case FCHAN_INPUT: st="I"; break;
                                    case FCHAN_OUTPUT: st="O"; break;
                                    case FCHAN_APPEND: st="A"; break;
                                    case FCHAN_RANDOM: st="R"; break;
                                    case FCHAN_BINARY: st="B"; break;
                                    default: st=""; break;
                                }
                                int sl = (int)strlen(st);
                                char *p = strpool_store(&rt->strpool, st, sl);
                                val_stack[val_top++] = bval_string(p, sl);
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_CVI) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue sv = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_int(0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                if (!bval_is_string(&sv) || sv.v.sval.data == NULL || sv.v.sval.length < 2) {
                                    val_stack[val_top++] = bval_int(0);
                                } else {
                                    unsigned char lo = (unsigned char)sv.v.sval.data[0];
                                    unsigned char hi = (unsigned char)sv.v.sval.data[1];
                                    int cvi_val = (int)(lo | (hi << 8));
                                    if (cvi_val > 32767) cvi_val -= 65536;
                                    val_stack[val_top++] = bval_int((long)cvi_val);
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_CVS) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue sv = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_int(0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                if (!bval_is_string(&sv) || sv.v.sval.data == NULL || sv.v.sval.length < 4) {
                                    val_stack[val_top++] = bval_int(0);
                                } else {
                                    if (dialect_get_config()->id == DIALECT_GW_BASIC) {
                                        double val = gw_mbf32_to_double((const uint8_t *)sv.v.sval.data);
                                        val_stack[val_top++] = bval_float(val);
                                    } else {
                                        float f;
                                        memcpy(&f, sv.v.sval.data, sizeof(float));
                                        val_stack[val_top++] = bval_float((double)f);
                                    }
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_CVD) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue sv = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_int(0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                if (!bval_is_string(&sv) || sv.v.sval.data == NULL || sv.v.sval.length < 8) {
                                    val_stack[val_top++] = bval_int(0);
                                } else {
                                    if (dialect_get_config()->id == DIALECT_GW_BASIC) {
                                        double val = gw_mbf64_to_double((const uint8_t *)sv.v.sval.data);
                                        val_stack[val_top++] = bval_float(val);
                                    } else {
                                        double d;
                                        memcpy(&d, sv.v.sval.data, sizeof(double));
                                        val_stack[val_top++] = bval_float(d);
                                    }
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_INKEY || kw == KW_ONKEY) {
                    lexer_next(lex);
                    int ch = vdev_inkey();
                    if (ch > 0) {
                        char kb[2];
                        kb[0] = (char)ch;
                        kb[1] = '\0';
                        char *ptr = strpool_store(&rt->strpool, kb, 1);
                        val_stack[val_top++] = bval_string(ptr, 1);
                    } else {
                        val_stack[val_top++] = bval_string(NULL, 0);
                    }
                    expect_operand = 0;
                } else if (kw == KW_LCASE || kw == KW_UCASE || kw == KW_TCASE ||
                           kw == KW_LTRIM || kw == KW_RTRIM || kw == KW_TRIM) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) {
                            val_stack[val_top++] = bval_int(0);
                        } else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                const char *s = val.v.sval.data;
                                int slen = val.v.sval.length;
                                if (s == NULL) { s = ""; slen = 0; }
                                if (slen > 255) slen = 255;
                                char buf[256];
                                char *ptr;
                                
                                if (kw == KW_LCASE) {
                                    for (int i = 0; i < slen; i++) buf[i] = (char)tolower((unsigned char)s[i]);
                                    ptr = strpool_store(&rt->strpool, buf, slen);
                                    val_stack[val_top++] = bval_string(ptr, slen);
                                } else if (kw == KW_UCASE) {
                                    for (int i = 0; i < slen; i++) buf[i] = (char)toupper((unsigned char)s[i]);
                                    ptr = strpool_store(&rt->strpool, buf, slen);
                                    val_stack[val_top++] = bval_string(ptr, slen);
                                } else if (kw == KW_TCASE) {
                                    int after_space = 1;
                                    for (int i = 0; i < slen; i++) {
                                        unsigned char c = (unsigned char)s[i];
                                        if (c == ' ' || c == '\t') { buf[i] = (char)c; after_space = 1; }
                                        else if (after_space) { buf[i] = (char)toupper(c); after_space = 0; }
                                        else { buf[i] = (char)tolower(c); }
                                    }
                                    ptr = strpool_store(&rt->strpool, buf, slen);
                                    val_stack[val_top++] = bval_string(ptr, slen);
                                } else if (kw == KW_LTRIM) {
                                    int i = 0;
                                    while (i < slen && s[i] == ' ') i++;
                                    ptr = strpool_store(&rt->strpool, s + i, slen - i);
                                    val_stack[val_top++] = bval_string(ptr, slen - i);
                                } else if (kw == KW_RTRIM) {
                                    int i = slen;
                                    while (i > 0 && s[i-1] == ' ') i--;
                                    ptr = strpool_store(&rt->strpool, s, i);
                                    val_stack[val_top++] = bval_string(ptr, i);
                                } else if (kw == KW_TRIM) {
                                    int left = 0;
                                    int right = slen;
                                    while (left < right && s[left] == ' ') left++;
                                    while (right > left && s[right-1] == ' ') right--;
                                    ptr = strpool_store(&rt->strpool, s + left, right - left);
                                    val_stack[val_top++] = bval_string(ptr, right - left);
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_REPLACE) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue src = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                        else {
                            if (lex->current.type != TOK_COMMA) {
                                error_raise(ERR_WHAT, line_num);
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                lexer_next(lex);
                                BValue old_v = parse_expression_bval_internal(lex, rt, line_num, 0);
                                if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                                else {
                                    if (lex->current.type != TOK_COMMA) {
                                        error_raise(ERR_WHAT, line_num);
                                        val_stack[val_top++] = bval_int(0);
                                    } else {
                                        lexer_next(lex);
                                        BValue new_v = parse_expression_bval_internal(lex, rt, line_num, 0);
                                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                                        else {
                                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                                val_stack[val_top++] = bval_int(0);
                                            } else {
                                                const char *sd = src.v.sval.data; int sl = src.v.sval.length;
                                                const char *od = old_v.v.sval.data; int ol = old_v.v.sval.length;
                                                const char *nd = new_v.v.sval.data; int nl = new_v.v.sval.length;
                                                if (!sd) { sd = ""; sl = 0; }
                                                if (!od || ol == 0) {
                                                    char *ptr = strpool_store(&rt->strpool, sd, sl);
                                                    val_stack[val_top++] = bval_string(ptr, sl);
                                                } else {
                                                    if (!nd) { nd = ""; nl = 0; }
                                                    char buf[MAX_LINE_LENGTH + 1];
                                                    int wi = 0, ri = 0;
                                                    for (ri = 0; ri < sl && wi < MAX_LINE_LENGTH; ) {
                                                        if (ri + ol <= sl && memcmp(sd + ri, od, (size_t)ol) == 0) {
                                                            for (int ci = 0; ci < nl && wi < MAX_LINE_LENGTH; ci++)
                                                                buf[wi++] = nd[ci];
                                                            ri += ol;
                                                        } else {
                                                            buf[wi++] = sd[ri++];
                                                        }
                                                    }
                                                    char *ptr = strpool_store(&rt->strpool, buf, wi);
                                                    val_stack[val_top++] = bval_string(ptr, wi);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_REVERSE) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                        else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                const char *s = val.v.sval.data;
                                int slen = val.v.sval.length;
                                if (s == NULL) { s = ""; slen = 0; }
                                if (slen > 255) slen = 255;
                                char buf[256];
                                for (int i = 0; i < slen; i++) buf[i] = s[slen - 1 - i];
                                char *ptr = strpool_store(&rt->strpool, buf, slen);
                                val_stack[val_top++] = bval_string(ptr, slen);
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_MCASE) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                        else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                const char *s = val.v.sval.data;
                                int slen = val.v.sval.length;
                                if (s == NULL) { s = ""; slen = 0; }
                                if (slen > 255) slen = 255;
                                char buf[256];
                                for (int i = 0; i < slen; i++) {
                                    unsigned char c = (unsigned char)s[i];
                                    uint64_t r = rt->rnd_seed;
                                    rt->rnd_seed = r * 6364136223846793005ULL + (12345ULL | 1);
                                    if ((r >> 17) & 1) buf[i] = (char)toupper(c);
                                    else buf[i] = (char)tolower(c);
                                }
                                char *ptr = strpool_store(&rt->strpool, buf, slen);
                                val_stack[val_top++] = bval_string(ptr, slen);
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_ICASE) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                        else {
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                const char *s = val.v.sval.data;
                                int slen = val.v.sval.length;
                                if (s == NULL) { s = ""; slen = 0; }
                                if (slen > 255) slen = 255;
                                char buf[256];
                                for (int i = 0; i < slen; i++) {
                                    unsigned char c = (unsigned char)s[i];
                                    if (isupper(c)) buf[i] = (char)tolower(c);
                                    else if (islower(c)) buf[i] = (char)toupper(c);
                                    else buf[i] = (char)c;
                                }
                                char *ptr = strpool_store(&rt->strpool, buf, slen);
                                val_stack[val_top++] = bval_string(ptr, slen);
                            }
                        }
                    }
                    expect_operand = 0;
                } else if (kw == KW_HASH) {
                    lexer_next(lex);
                    if (!lexer_expect(lex, TOK_LPAREN)) {
                        val_stack[val_top++] = bval_int(0);
                    } else {
                        BValue val = parse_expression_bval_internal(lex, rt, line_num, 0);
                        if (error_occurred()) { val_stack[val_top++] = bval_int(0); }
                        else {
                            int bits = 32;
                            if (lex->current.type == TOK_COMMA) {
                                lexer_next(lex);
                                bits = (int)parse_expression(lex, rt, line_num);
                            }
                            if (!lexer_expect(lex, TOK_RPAREN)) {
                                val_stack[val_top++] = bval_int(0);
                            } else {
                                const char *s = val.v.sval.data;
                                int slen = val.v.sval.length;
                                if (s == NULL) { s = ""; slen = 0; }
                                if (bits != 8 && bits != 16 && bits != 32 && bits != 64 && bits != 128 && bits != 256)
                                    bits = 32;
                                int rounds = (bits + 63) / 64;
                                if (rounds < 1) rounds = 1;
                                if (rounds > 4) rounds = 4;
                                char hexbuf[65];
                                int hexlen = 0;
                                for (int r = 0; r < rounds; r++) {
                                    unsigned long long h = 14695981039346656037ULL + (unsigned long long)r * 6364136223846793005ULL;
                                    for (int i = 0; i < slen; i++) {
                                        h ^= (unsigned char)s[i];
                                        h *= 1099511628211ULL;
                                    }
                                    if (bits <= 64 && rounds == 1) {
                                        if (bits == 8) {
                                            h = (h ^ (h >> 8) ^ (h >> 16) ^ (h >> 24) ^ (h >> 32) ^ (h >> 40) ^ (h >> 48) ^ (h >> 56)) & 0xFF;
                                            sprintf(hexbuf, "%02X", (unsigned)h);
                                            hexlen = 2;
                                        } else if (bits == 16) {
                                            h = ((h >> 16) ^ h) & 0xFFFF;
                                            sprintf(hexbuf, "%04X", (unsigned)h);
                                            hexlen = 4;
                                        } else if (bits == 32) {
                                            h = ((h >> 32) ^ h) & 0xFFFFFFFFULL;
                                            sprintf(hexbuf, "%08X", (unsigned)h);
                                            hexlen = 8;
                                        } else {
                                            sprintf(hexbuf, "%08X%08X", (unsigned)(h >> 32), (unsigned)(h & 0xFFFFFFFFULL));
                                            hexlen = 16;
                                        }
                                    } else {
                                        sprintf(hexbuf + hexlen, "%08X%08X", (unsigned)(h >> 32), (unsigned)(h & 0xFFFFFFFFULL));
                                        hexlen += 16;
                                    }
                                }
                                int want = bits / 4;
                                if (hexlen > want) hexlen = want;
                                char *hptr = strpool_store(&rt->strpool, hexbuf, hexlen);
                                val_stack[val_top++] = bval_string(hptr, hexlen);
                            }
                        }
                    }
                    expect_operand = 0;
                } else {
                    const FunctionEntry *fn = funcreg_find_by_keyword(kw);
                    if (fn != NULL) {
                        lexer_next(lex);
                        if (lex->current.type == TOK_LPAREN) {
                            lexer_next(lex);
                            ParseOp op_entry;
                            memset(&op_entry, 0, sizeof(op_entry));
                            op_entry.op = OP_FUNC;
                            op_entry.precedence = 14;
                            op_entry.kw = kw;
                            op_stack[op_top++] = op_entry;
                            
                            ParseOp lp_entry;
                            memset(&lp_entry, 0, sizeof(lp_entry));
                            lp_entry.op = OP_LPAREN;
                            op_stack[op_top++] = lp_entry;
                            arg_count_stack[arg_count_top++] = 1;
                        } else {
                            if (fn->max_args == 0) {
                                val_stack[val_top++] = fn->handler(NULL, 0, rt);
                                expect_operand = 0;
                            } else if (is_rnd_argument_start(lex)) {
                                ParseOp op_entry;
                                memset(&op_entry, 0, sizeof(op_entry));
                                op_entry.op = OP_FUNC;
                                op_entry.precedence = 12;
                                op_entry.kw = kw;
                                op_entry.is_unary = 1;
                                op_entry.arg_count = 1;
                                op_stack[op_top++] = op_entry;
                            } else {
                                val_stack[val_top++] = fn->handler(NULL, 0, rt);
                                expect_operand = 0;
                            }
                        }
                    } else {
                        // CONST lookup: check if keyword matches a stored constant name
                        const char *knm = tok.str_start;
                        int knl = tok.str_length;
                        int ci, matched = 0;
                        if (knm != NULL && knl > 0) {
                            for (ci = 0; ci < rt->const_count; ci++) {
                                if (rt->constants[ci].name_len == knl && memcmp(rt->constants[ci].name, knm, (size_t)knl) == 0) {
                                    lexer_next(lex);
                                    val_stack[val_top++] = rt->constants[ci].value;
                                    expect_operand = 0;
                                    matched = 1;
                                    break;
                                }
                            }
                        }
                        if (!matched) {
                            error_raise(ERR_WHAT, line_num);
                            return bval_int(0);
                        }
                    }
                }
            } else {
                error_raise(ERR_WHAT, line_num);
                return bval_int(0);
            }
        } else {
            int op = OP_EOF;
            int precedence = 0;
            
            if (tok.type == TOK_PLUS) op = OP_ADD;
            else if (tok.type == TOK_MINUS) op = OP_SUB;
            else if (tok.type == TOK_STAR) op = OP_MUL;
            else if (tok.type == TOK_SLASH) op = OP_DIV;
            else if (tok.type == TOK_BACKSLASH) op = OP_INTDIV;
            else if (tok.type == TOK_CARET) op = OP_POW;
            else if (tok.type == TOK_EQUALS) op = OP_CMP_EQ;
            else if (tok.type == TOK_NOT_EQ) op = OP_CMP_NE;
            else if (tok.type == TOK_LT) op = OP_CMP_LT;
            else if (tok.type == TOK_GT) op = OP_CMP_GT;
            else if (tok.type == TOK_LT_EQ) op = OP_CMP_LE;
            else if (tok.type == TOK_GT_EQ) op = OP_CMP_GE;
            else if (tok.type == TOK_KEYWORD) {
                KeywordId kw = tok.value.keyword;
                if (kw == KW_AND) op = OP_AND;
                else if (kw == KW_OR) op = OP_OR;
                else if (kw == KW_XOR) op = OP_XOR;
                else if (kw == KW_EQV) op = OP_EQV;
                else if (kw == KW_IMP) op = OP_IMP;
                else if (kw == KW_MOD) op = OP_MOD;
                else if (kw == KW_LIKE) op = OP_LIKE;
            }
            
            if (op != OP_EOF) {
                precedence = get_precedence(op);
                int is_right = is_right_associative(op);
                
                while (op_top > 0) {
                    ParseOp top_op = op_stack[op_top - 1];
                    if (top_op.op == OP_LPAREN) break;
                    int top_prec = get_precedence(top_op.op);
                    if (top_prec > precedence || (!is_right && top_prec == precedence)) {
                        op_top--;
                        if (!apply_operator(rt, top_op.op, val_stack, &val_top, top_op.kw, top_op.name, top_op.name_len, top_op.arg_count, line_num)) {
                            return bval_int(0);
                        }
                    } else {
                        break;
                    }
                }
                ParseOp op_entry;
                memset(&op_entry, 0, sizeof(op_entry));
                op_entry.op = op;
                op_entry.precedence = precedence;
                op_entry.assoc = is_right;
                op_stack[op_top++] = op_entry;
                lexer_next(lex);
                expect_operand = 1;
            } else if (tok.type == TOK_COMMA) {
                while (op_top > 0 && op_stack[op_top - 1].op != OP_LPAREN) {
                    ParseOp top_op = op_stack[--op_top];
                    if (!apply_operator(rt, top_op.op, val_stack, &val_top, top_op.kw, top_op.name, top_op.name_len, top_op.arg_count, line_num)) {
                        return bval_int(0);
                    }
                }
                if (op_top > 0 && op_stack[op_top - 1].op == OP_LPAREN) {
                    if (arg_count_top > 0) {
                        arg_count_stack[arg_count_top - 1]++;
                    }
                }
                lexer_next(lex);
                expect_operand = 1;
            } else if (tok.type == TOK_RPAREN) {
                while (op_top > 0 && op_stack[op_top - 1].op != OP_LPAREN) {
                    ParseOp top_op = op_stack[--op_top];
                    if (!apply_operator(rt, top_op.op, val_stack, &val_top, top_op.kw, top_op.name, top_op.name_len, top_op.arg_count, line_num)) {
                        return bval_int(0);
                    }
                }
                if (op_top > 0 && op_stack[op_top - 1].op == OP_LPAREN) {
                    op_top--; // pop (
                    if (op_top > 0) {
                        ParseOp next_op = op_stack[op_top - 1];
                        if (next_op.op == OP_FUNC || next_op.op == OP_USER_FUNC || next_op.op == OP_ARRAY) {
                            op_top--; // pop function operator
                            int arg_count = arg_count_top > 0 ? arg_count_stack[--arg_count_top] : 0;
                            
                            if (next_op.op == OP_USER_FUNC) {
                                char sub_name[MAX_VAR_NAME_LEN + 1];
                                if (next_op.name_len > 0) {
                                    memcpy(sub_name, next_op.name, (size_t)next_op.name_len);
                                    sub_name[next_op.name_len] = '\0';
                                } else {
                                    sub_name[0] = 'F'; sub_name[1] = 'N'; sub_name[2] = (char)next_op.kw; sub_name[3] = '\0';
                                }
                                SubDef *sd = runtime_find_sub(rt, sub_name, (int)strlen(sub_name));
                                if (sd != NULL && sd->is_function && sd->body_index >= 0) {
                                    BValue args[16];
                                    if (val_top < arg_count || arg_count > 16) {
                                        error_raise(ERR_WHAT, line_num);
                                        return bval_int(0);
                                    }
                                    for (int i = arg_count - 1; i >= 0; i--) {
                                        args[i] = val_stack[--val_top];
                                    }
                                    
                                    // Save runtime state
                                    BValue saved_fn_rv = rt->fn_return_value;
                                    int saved_sub_idx = rt->in_sub_index;
                                    int saved_bif_depth = rt->block_if_depth;
                                    rt->fn_return_value = bval_int(0);
                                    rt->block_if_depth = 0;
                                    rt->in_sub_index = (int)(sd - rt->subs);
                                    
                                    // Push FRAME_SUB
                                    StackFrame frame;
                                    frame.type = FRAME_SUB;
                                    frame.data.sub_call.return_index = rt->current_index;
                                    frame.data.sub_call.sub_index = (int)(sd - rt->subs);
                                    for (int i = 0; i < MAX_VARIABLES; i++)
                                        frame.data.sub_call.saved_vars[i] = rt->variables[i];
                                    for (int i = 0; i < MAX_STRING_VARS; i++)
                                        frame.data.sub_call.saved_strvars[i] = rt->string_vars[i];
                                    if (runtime_push(rt, &frame) != 0) return bval_int(0);
                                    
                                    // Push scope
                                    int smode = SCOPE_FULL;
                                    if (dialect_get_config()->id == DIALECT_QBASIC)
                                        smode = SCOPE_FRESH;
                                    scope_stack_push(&rt->scope_stack, rt, smode, (int)(sd - rt->subs), rt->current_index);
                                    
                                    // Assign parameters
                                    for (int i = 0; i < arg_count && i < sd->param_count; i++) {
                                        pi_set_param_by_name(rt, sd->params[i], args[i]);
                                    }
                                    
                                    // Execute FUNCTION body lines
                                    int save_idx = rt->current_index;
                                    int save_next = rt->next_index;
                                    int fi = sd->body_index;
                                    ProgramStore *pgm = rt->program;
                                    while (fi < pgm->count && !error_occurred()) {
                                        ProgramLine *fline = &pgm->lines[fi];
                                        int fln = fline->line_number;
                                        Lexer fl;
                                        lexer_init(&fl, fline->text);
                                        if (fl.current.type == TOK_NUMBER) lexer_next(&fl);
                                        
                                        rt->current_index = fi;
                                        rt->next_index = -1;
                                        
                                        parser_execute_line(&fl, rt, fln);
                                        
                                        if (error_occurred()) return bval_int(0);
                                        
                                        // Check if END SUB/FUNCTION popped our frame
                                        if (rt->in_sub_index < 0) break;
                                        
                                        if (rt->next_index >= 0) fi = rt->next_index;
                                        else fi++;
                                    }
                                    
                                    rt->current_index = save_idx;
                                    rt->next_index = save_next;
                                    
                                    BValue rv = rt->fn_return_value;
                                    rt->fn_return_value = saved_fn_rv;
                                    rt->in_sub_index = saved_sub_idx;
                                    rt->block_if_depth = saved_bif_depth;
                                    
                                    val_stack[val_top++] = rv;
                                }
                            } else {
                                if (!apply_operator(rt, next_op.op, val_stack, &val_top, next_op.kw, next_op.name, next_op.name_len, arg_count, line_num)) {
                                    return bval_int(0);
                                }
                            }
                        }
                    }
                }
                lexer_next(lex);
                expect_operand = 0;
            } else {
                break;
            }
        }
    }

    if (error_occurred()) {
        return bval_int(0);
    }

    while (op_top > 0) {
        ParseOp top_op = op_stack[--op_top];
        if (top_op.op == OP_LPAREN) {
            error_raise(ERR_WHAT, line_num);
            return bval_int(0);
        }
        if (!apply_operator(rt, top_op.op, val_stack, &val_top, top_op.kw, top_op.name, top_op.name_len, top_op.arg_count, line_num)) {
            return bval_int(0);
        }
    }

    if (val_top != 1) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }

    return val_stack[0];
}

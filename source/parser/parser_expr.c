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
long parse_expression(Lexer *lex, RuntimeState *rt, int line_num)
{
 long left;
 int negate = 0;
 int do_not = 0;

 if (error_occurred()) return 0;

 // Optional leading sign or NOT
 if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 do_not = 1;
 lexer_next(lex);
 // Handle sign after NOT: NOT -1, NOT +5
 if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 }

 left = pi_parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (negate) {
 left = -left;
 }
 if (do_not) {
 left = ~left; // bitwise NOT
 }

 // Additive: + -
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 long right;
 TokenType op = lex->current.type;
 lexer_next(lex); // consume operator

 right = pi_parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;

 if (op == TOK_PLUS) {
 left = left + right;
 } else {
 left = left - right;
 }
 }

 // Logical/bitwise: AND OR XOR EQV IMP
 while (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 long right;
 if (kw != KW_AND && kw != KW_OR &&
 kw != KW_XOR && kw != KW_EQV &&
 kw != KW_IMP) {
 break;
 }
 lexer_next(lex); // consume operator

 // Parse the right side at additive level
 {
 int rn = 0, rn2 = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 rn2 = 1; lexer_next(lex);
 }
 right = pi_parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (rn) right = -right;
 if (rn2) right = ~right;

 // Inner additive loop
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 long r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = pi_parse_term(lex, rt, line_num);
 if (error_occurred()) return 0;
 if (op2 == TOK_PLUS)
 right = right + r2;
 else
 right = right - r2;
 }
 }

 switch (kw) {
 case KW_AND: left = left & right; break;
 case KW_OR: left = left | right; break;
 case KW_XOR: left = left ^ right; break;
 case KW_EQV:
 left = ~(left ^ right); break;
 case KW_IMP:
 left = (~left) | right; break;
 default: break;
 }
 }

 return left;
}

// --- BValue Expression System ---
 // These are the BValue-returning versions of parse_factor, parse_term,
 // and parse_expression. They handle integers, floats, strings, and
 // all functions.
 //
 // The old long-returning versions are preserved for backward
 // compatibility with Phases 1-3 code paths.

 // parse_factor_bval - BValue atom parser.
 //
 // Handles: integers, floats, string literals, variables (A-Z, named),
 // string variables (A$-Z$), @() arrays, DIM array access, parenthesized
 // expressions, and all built-in functions.
BValue pi_parse_factor_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue val;

 if (error_occurred()) return bval_int(0);

 switch (lex->current.type) {
 case TOK_NUMBER:
 val = bval_int(lex->current.value.num_value);
 lexer_next(lex);
 return val;

 case TOK_FLOAT_LIT:
 if (!dialect_check_feature("floating point",
 dialect_get_config()->has_float, line_num)) {
 // Integer-only: truncate to int
 val = bval_int((long)lex->current.value.fval);
 lexer_next(lex);
 return val;
 }
 val = bval_float(lex->current.value.fval);
 lexer_next(lex);
 return val;

 case TOK_IMAGINARY:
 // Pure imaginary literal: 2i -> (0+2i)
 val = bval_complex(0.0, lex->current.value.fval);
 lexer_next(lex);
 return val;

 case TOK_STRING:
 {
 // String literal - store in pool
 char *ptr = strpool_store(&rt->strpool,
 lex->current.str_start, lex->current.str_length);
 int slen = lex->current.str_length;
 lexer_next(lex);
 if (ptr == NULL) {
 error_raise(ERR_SORRY, line_num);
 return bval_string(NULL, 0);
 }
 return bval_string(ptr, slen);
 }

 case TOK_VARIABLE:
 {
 char vname = lex->current.value.var_name;
 lexer_next(lex);
 // Check for typed var dot-read: V.field
 if (lex->current.type == TOK_DOT) {
 TypedVar *tv = runtime_find_typed_var(
  rt, &vname, 1);
 if (tv != NULL) {
  UserTypeDef *td =
  &rt->user_types[tv->type_index];
  const char *fname;
  int flen, fi;
  lexer_next(lex); // consume dot
  if (lex->current.type == TOK_NAMED_VAR) {
  fname = lex->current.str_start;
  flen = lex->current.str_length;
  } else if (lex->current.type ==
   TOK_KEYWORD) {
  fname = lexer_keyword_name(
   lex->current.value.keyword);
  flen = fname ?
   (int)strlen(fname) : 0;
  } else if (lex->current.type ==
   TOK_VARIABLE) {
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
  // Nested type: walk into child
  while (td->fields[fi].nested_type_index >= 0
      && lex->current.type == TOK_DOT) {
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
  // Check if this is a DIM array access: A(...) -- auto-DIMs
  if (lex->current.type == TOK_LPAREN &&
  dialect_get_config()->has_dim_arrays) {
  int idx1, idx2 = 0, idx3 = 0;
  lexer_next(lex); // consume (
  val = parse_expression_bval(lex, rt, line_num);
  idx1 = (int)bval_to_subscript(&val);
  if (error_occurred()) return bval_int(0);
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  val = parse_expression_bval(lex, rt,
  line_num);
  idx2 = (int)bval_to_subscript(&val);
  if (error_occurred()) return bval_int(0);
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  val = parse_expression_bval(lex, rt,
  line_num);
  idx3 = (int)bval_to_subscript(&val);
  if (error_occurred()) return bval_int(0);
  }
  }
  if (!lexer_expect(lex, TOK_RPAREN))
  return bval_int(0);
  return runtime_get_dim(rt, &vname, 1,
  idx1, idx2, idx3, line_num);
  }
 return runtime_get_var_bval(rt, vname);
 }

  case TOK_STRING_VAR:
 {
 char vname = lex->current.value.var_name;
 if (!dialect_check_feature("string variables",
 dialect_get_config()->has_string_vars, line_num))
 return bval_int(0);
 lexer_next(lex);
 // Check for DIM string array access: A$(index)
 // The DIM name for single-char string arrays is
 // stored as "A$" (2 chars).
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 char sname[3];
 sname[0] = vname;
 sname[1] = '$';
 sname[2] = '\0';
 {
 DimArray *arr = runtime_find_dim(
 rt, sname, 2);
 if (arr != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 lexer_next(lex); // consume (
 val = parse_expression_bval(
 lex, rt, line_num);
 idx1 = (int)bval_to_subscript(
 &val);
 if (error_occurred())
 return bval_int(0);
 if (lex->current.type ==
 TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 idx2 = (int)bval_to_subscript(
 &val);
 if (error_occurred())
 return bval_int(0);
 if (lex->current.type ==
 TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 idx3 = (int)bval_to_subscript(
 &val);
 if (error_occurred())
 return bval_int(0);
 }
 }
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 return runtime_get_dim(rt,
 sname, 2, idx1, idx2,
 idx3, line_num);
 }
 }
 }
 return runtime_get_string_var(rt, vname);
 }

   case TOK_NAMED_VAR:
 {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;
 if (!dialect_check_feature("named variables",
 dialect_get_config()->has_extended_vars, line_num))
 return bval_int(0);
 lexer_next(lex);

 // Dot-read for scalar TypedVar: Player.HP
 // Must check before FUNCTION/DIM checks.
 if (lex->current.type == TOK_DOT) {
 TypedVar *tv = runtime_find_typed_var(rt, nm, nlen);
 if (tv != NULL) {
  UserTypeDef *td = &rt->user_types[tv->type_index];
  const char *fname;
  int flen, fi;

  lexer_next(lex); // consume dot
  if (lex->current.type != TOK_NAMED_VAR &&
      lex->current.type != TOK_VARIABLE &&
      lex->current.type != TOK_KEYWORD) {
  error_raise(ERR_WHAT, line_num);
  return bval_int(0);
  }
  if (lex->current.type == TOK_NAMED_VAR) {
  fname = lex->current.str_start;
  flen = lex->current.str_length;
  } else if (lex->current.type == TOK_KEYWORD) {
  fname = lexer_keyword_name(lex->current.value.keyword);
  flen = fname ? (int)strlen(fname) : 0;
  } else {
  fname = &lex->current.value.var_name;
  flen = 1;
  }
  fi = runtime_find_field(td, fname, flen);
  if (fi < 0) {
  error_raise(ERR_WHAT, line_num);
  return bval_int(0);
  }
  lexer_next(lex); // consume field name
  // Nested type: walk into child
  while (td->fields[fi].nested_type_index >= 0
      && lex->current.type == TOK_DOT) {
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

 // Typed array dot-read: Enemies(1).HP
 // Check for array with type_index >= 0, then
 // parse subscript + dot + field.
 if (lex->current.type == TOK_LPAREN &&
     dialect_get_config()->has_dim_arrays) {
 DimArray *arr = runtime_find_dim(rt, nm, nlen);
 if (arr != NULL && arr->type_index >= 0) {
  int idx1, idx2 = 0, idx3 = 0;
  int elem_idx;
  UserTypeDef *td;
  BValue *fval;
  BValue val;

  lexer_next(lex); // consume (
  val = parse_expression_bval(lex, rt, line_num);
  idx1 = (int)bval_to_subscript(&val);
  if (error_occurred()) return bval_int(0);
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  val = parse_expression_bval(lex, rt, line_num);
  idx2 = (int)bval_to_subscript(&val);
  if (error_occurred()) return bval_int(0);
  if (lex->current.type == TOK_COMMA) {
   lexer_next(lex);
   val = parse_expression_bval(lex, rt, line_num);
   idx3 = (int)bval_to_subscript(&val);
   if (error_occurred()) return bval_int(0);
  }
  }
  if (!lexer_expect(lex, TOK_RPAREN))
  return bval_int(0);

  td = &rt->user_types[arr->type_index];
  elem_idx = idx1 - rt->option_base;
  if (arr->dims >= 2)
  elem_idx = elem_idx * arr->size[1]
   + (idx2 - rt->option_base);
  if (arr->dims >= 3)
  elem_idx = elem_idx * arr->size[2]
   + (idx3 - rt->option_base);

  if (lex->current.type == TOK_DOT) {
  const char *fname;
  int flen, fi;
  lexer_next(lex); // consume dot
  if (lex->current.type != TOK_NAMED_VAR &&
      lex->current.type != TOK_VARIABLE &&
      lex->current.type != TOK_KEYWORD) {
   error_raise(ERR_WHAT, line_num);
   return bval_int(0);
  }
  if (lex->current.type == TOK_NAMED_VAR) {
   fname = lex->current.str_start;
   flen = lex->current.str_length;
  } else if (lex->current.type == TOK_KEYWORD) {
   fname = lexer_keyword_name(lex->current.value.keyword);
   flen = fname ? (int)strlen(fname) : 0;
  } else {
   fname = &lex->current.value.var_name;
   flen = 1;
  }
  fi = runtime_find_field(td, fname, flen);
  if (fi < 0) {
   error_raise(ERR_WHAT, line_num);
   return bval_int(0);
  }
  lexer_next(lex); // consume field name
  fval = runtime_get_typed_array_field(
   rt, arr, elem_idx, fi);
  if (fval == NULL) {
   error_raise(ERR_HOW, line_num);
   return bval_int(0);
  }
  return *fval;
  }
  // No dot: return int of element 0? Error - typed array must use dot
  error_raise(ERR_WHAT, line_num);
  return bval_int(0);
 }
 }

 // Check for FUNCTION call.
 // If name(args) matches a FUNCTION def,
 // execute it and return fn_return_value.
 if (lex->current.type == TOK_LPAREN &&
 nm != NULL && nlen > 0) {
 SubDef *sd = runtime_find_sub(
 rt, nm, nlen);
 if (sd != NULL && sd->is_function) {
 StackFrame frame;
 int i;
 int save_idx, save_next;

 // Push FRAME_SUB
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++){
 frame.data.sub_call
 .saved_vars[i] =
 rt->variables[i];
 }
 for (i=0; i<MAX_STRING_VARS; i++){
 frame.data.sub_call
 .saved_strvars[i] =
 rt->string_vars[i];
 }
 if (runtime_push(rt, &frame) != 0)
 return bval_int(0);

 // Push scope stack
 {
 int smode = SCOPE_FULL;
 if (dialect_get_config()->id ==
  DIALECT_QBASIC)
  smode = SCOPE_FRESH;
 scope_stack_push(
  &rt->scope_stack, rt,
  smode,
  (int)(sd - rt->subs),
  rt->current_index);
 }

 // Parse args
 lexer_next(lex); // consume (
 for (i = 0; i < sd->param_count;
 i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 pi_set_param_by_name(rt,
 sd->params[i], av);
 }
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);

 // Execute FUNCTION body inline.
 // Save/restore execution position and
 // fn_return_value (for recursion).
 {
 BValue saved_fn_rv =
  rt->fn_return_value;
 int saved_sub_idx =
  rt->in_sub_index;
 int saved_bif_depth =
  rt->block_if_depth;
 rt->fn_return_value = bval_int(0);
 rt->block_if_depth = 0;
 rt->in_sub_index =
 (int)(sd - rt->subs);
 save_idx = rt->current_index;
 save_next = rt->next_index;

 {
 int fi = sd->body_index;
 ProgramStore *pgm = rt->program;
 while (fi < pgm->count &&
 !error_occurred()) {
 Lexer fl;
 int fln;
 ProgramLine *fline =
 &pgm->lines[fi];
 fln = fline->line_number;
 lexer_init(&fl,
 fline->text);
 if (fl.current.type ==
 TOK_NUMBER)
 lexer_next(&fl);

 rt->current_index = fi;
 rt->next_index = -1;

 parser_execute_line(
 &fl, rt, fln);

 if (error_occurred())
 return bval_int(0);

 // Check if END SUB/FUNCTION
 // popped our frame 
 if (rt->in_sub_index < 0) {
 break;
 }

 if (rt->next_index >= 0)
 fi = rt->next_index;
 else
 fi++;
 }
 }

 rt->current_index = save_idx;
 rt->next_index = save_next;
 {
  BValue rv = rt->fn_return_value;
 rt->fn_return_value = saved_fn_rv;
 rt->in_sub_index = saved_sub_idx;
 rt->block_if_depth = saved_bif_depth;
 return rv;
 }
 }
 }

 // Check for single-line DEF FN
 // called as FNA(x) in extended-vars
 // mode (where FNA is TOK_NAMED_VAR).
 if (nlen >= 3 &&
 (nm[0] == 'F' || nm[0] == 'f') &&
 (nm[1] == 'N' || nm[1] == 'n')) {
 // Extract fn letter(s) after FN
 char fn_ch = nm[2];
 char fn_buf[2];
 UserFunction *ufn;
 if (fn_ch >= 'a' && fn_ch <= 'z')
 fn_ch = (char)(fn_ch - 32);
 fn_buf[0] = fn_ch;
 fn_buf[1] = '\0';
 ufn = runtime_find_fn(rt,
 fn_buf, 1);
 if (ufn != NULL) {
 // Evaluate inline
 BValue args[MAX_FN_PARAMS];
 BValue saved[MAX_FN_PARAMS];
 int ac = 0, pi;
 Lexer bl;
 BValue res;
 lexer_next(lex); // (
 if (ufn->param_count > 0) {
 args[ac] =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 ac++;
 while (ac <
 ufn->param_count &&
 lex->current.type
 == TOK_COMMA) {
 lexer_next(lex);
 args[ac] =
 parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 ac++;
 }
 }
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 // Save & bind params
 for (pi = 0;
 pi < ufn->param_count;
 pi++) {
 int vi =
 ufn->params[pi] - 'A';
 saved[pi] =
 rt->variables[vi];
 if (pi < ac)
 rt->variables[vi] =
 args[pi];
 }
 // Eval body
 lexer_init(&bl, ufn->body);
 res = parse_expression_bval(
 &bl, rt, line_num);
 // Restore
 for (pi = 0;
 pi < ufn->param_count;
 pi++) {
 int vi =
 ufn->params[pi] - 'A';
 rt->variables[vi] =
 saved[pi];
 }
 return res;
 }
 }
 }

 // Check for DIM array access
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 DimArray *arr = runtime_find_dim(rt, nm, nlen);
 if (arr != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 lexer_next(lex);
 val = parse_expression_bval(lex, rt, line_num);
 idx1 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx2 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 val = parse_expression_bval(lex, rt,
 line_num);
 idx3 = (int)bval_to_subscript(&val);
 if (error_occurred()) return bval_int(0);
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return runtime_get_dim(rt, nm, nlen,
 idx1, idx2, idx3, line_num);
 }
 }
 // Check CONST table before named vars
 {
 int ci;
 for (ci = 0; ci < rt->const_count; ci++){
 int cl = rt->constants[ci].name_len;
 if (cl == nlen) {
 // Case-insensitive compare
 int j, match = 1;
 for (j = 0; j < nlen; j++) {
 char a = nm[j];
 char b = rt->constants[ci]
 .name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) {
 match = 0; break;
 }
 }
 if (match) {
 return rt->constants[ci]
 .value;
 }
 }
 }
 }
 return runtime_get_named_var_bval(rt, nm, nlen);
 }

 case TOK_AT:
 {
 long index;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN)) return bval_int(0);
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
 return bval_int(runtime_get_array(rt, index));
 }

 case TOK_LPAREN:
 lexer_next(lex);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 // Check for complex literal: (real +/- coeffai)
 if ((lex->current.type == TOK_PLUS ||
      lex->current.type == TOK_MINUS) &&
     bval_is_numeric(&val) &&
     !bval_is_complex(&val)) {
  int neg = (lex->current.type == TOK_MINUS);
  // Peek ahead a" only convert if next is imaginary
  lexer_next(lex); // consume +/-
  if (lex->current.type == TOK_IMAGINARY) {
   double real_part = bval_to_float(&val);
   double imag_part = lex->current.value.fval;
   if (neg) imag_part = -imag_part;
   lexer_next(lex); // consume imaginary
   if (!lexer_expect(lex, TOK_RPAREN))
    return bval_int(0);
   return bval_complex(real_part, imag_part);
  }
  // Not imaginary a" put back the +/- as part of
   // a normal expression. We can't un-consume the
   // +/-, so evaluate what follows and combine. 
  {
   BValue rhs = pi_parse_term_bval(lex, rt,
    line_num);
   if (error_occurred()) return bval_int(0);
   if (neg)
    val = bval_sub(&val, &rhs, line_num);
   else
    val = bval_add(&val, &rhs, line_num);
   // Continue with remaining +/- terms
   while (lex->current.type == TOK_PLUS ||
          lex->current.type == TOK_MINUS) {
    int s = (lex->current.type == TOK_MINUS);
    lexer_next(lex);
    rhs = pi_parse_term_bval(lex, rt, line_num);
    if (error_occurred()) return bval_int(0);
    if (s)
     val = bval_sub(&val, &rhs, line_num);
    else
     val = bval_add(&val, &rhs, line_num);
   }
  }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) return bval_int(0);
 return val;

 case TOK_KEYWORD:
 // User-defined function dispatch.
 if (lex->current.value.keyword == KW_FN) {
 lexer_next(lex); // consume FN
 return pi_eval_user_fn(lex, rt, line_num);
 }
 // LBOUND(arrayname, dim) / UBOUND(arrayname, dim)
 // Array bound query functions. The first arg is an
 // array name (not an expression), so we parse it
 // specially.
 if (lex->current.value.keyword == KW_LBOUND ||
     lex->current.value.keyword == KW_UBOUND) {
  int is_upper = (lex->current.value.keyword
   == KW_UBOUND);
  char aname[MAX_VAR_NAME_LEN + 1];
  int alen = 0, dim_arg = 1, i;
  DimArray *arr;

  lexer_next(lex); // consume LBOUND/UBOUND
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_int(0);

  // Parse array name
  if (lex->current.type == TOK_VARIABLE) {
   aname[0] = lex->current.value.var_name;
   aname[1] = '\0';
   alen = 1;
   lexer_next(lex);
  } else if (lex->current.type == TOK_NAMED_VAR) {
   alen = lex->current.str_length;
   if (alen > MAX_VAR_NAME_LEN)
    alen = MAX_VAR_NAME_LEN;
   memcpy(aname, lex->current.str_start,
    (size_t)alen);
   aname[alen] = '\0';
   lexer_next(lex);
  } else {
   error_raise(ERR_WHAT, line_num);
   return bval_int(0);
  }
  // Uppercase
  for (i = 0; i < alen; i++) {
   if (aname[i] >= 'a' && aname[i] <= 'z')
    aname[i] = (char)(aname[i] - 32);
  }

  // Optional dimension argument
   if (lex->current.type == TOK_COMMA) {
    BValue dim_val;
    lexer_next(lex);
    dim_val = parse_expression_bval(
     lex, rt, line_num);
    dim_arg = (int)bval_to_int(&dim_val);
    if (error_occurred()) return bval_int(0);
  }
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_int(0);

  arr = runtime_find_dim(rt, aname, alen);
  if (arr == NULL) {
   error_raise(ERR_HOW, line_num);
   return bval_int(0);
  }
  if (dim_arg < 1 || dim_arg > arr->dims) {
   error_raise(ERR_HOW, line_num);
   return bval_int(0);
  }
  if (is_upper) {
   // UBOUND: max valid subscript
   return bval_int(
    arr->size[dim_arg - 1] - 1
    + rt->option_base);
  } else {
   // LBOUND: OPTION BASE value
   return bval_int(rt->option_base);
  }
 }

 // DET(arrayname) - Matrix determinant.
 // Computes determinant of a square 2D array
 // using LU decomposition with partial pivoting.
 if (lex->current.value.keyword == KW_DET) {
  char aname[MAX_VAR_NAME_LEN + 1];
  int alen = 0, i, n, p, r;
  DimArray *arr;
  double work[16][16];
  double det_val = 1.0;
  int sign = 1;

  lexer_next(lex); // consume DET
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_float(0.0);

  // Parse array name
  if (lex->current.type == TOK_VARIABLE) {
   aname[0] = lex->current.value.var_name;
   aname[1] = '\0';
   alen = 1;
   lexer_next(lex);
  } else if (lex->current.type == TOK_NAMED_VAR) {
   alen = lex->current.str_length;
   if (alen > MAX_VAR_NAME_LEN)
    alen = MAX_VAR_NAME_LEN;
   memcpy(aname, lex->current.str_start,
    (size_t)alen);
   aname[alen] = '\0';
   lexer_next(lex);
  } else {
   error_raise(ERR_WHAT, line_num);
   return bval_float(0.0);
  }
  for (i = 0; i < alen; i++) {
   if (aname[i] >= 'a' && aname[i] <= 'z')
    aname[i] = (char)(aname[i] - 32);
  }
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_float(0.0);

  arr = runtime_find_dim(rt, aname, alen);
  if (arr == NULL || arr->dims != 2) {
   error_raise(ERR_HOW, line_num);
   return bval_float(0.0);
  }
  if (arr->size[0] != arr->size[1]) {
   error_raise(ERR_HOW, line_num);
   return bval_float(0.0);
  }
  n = arr->size[0] - 1; // 1-based size
  if (n > 15 || n < 1) {
   error_raise(ERR_SORRY, line_num);
   return bval_float(0.0);
  }

  // Copy matrix to work array (1-based)
  for (r = 0; r < n; r++) {
   int c;
   for (c = 0; c < n; c++) {
    BValue v = arr->elements[
     (r + 1) * arr->size[1] + (c + 1)];
    work[r][c] = bval_to_float(&v);
   }
  }

  // LU decomposition with partial pivoting
  for (p = 0; p < n; p++) {
   int max_row = p;
   double max_val = work[p][p];
   double pivot;
   int c;
   if (max_val < 0) max_val = -max_val;

   for (r = p + 1; r < n; r++) {
    double v = work[r][p];
    if (v < 0) v = -v;
    if (v > max_val) {
     max_val = v;
     max_row = r;
    }
   }
   if (max_row != p) {
    // Swap rows
    for (c = 0; c < n; c++) {
     double tmp = work[p][c];
     work[p][c] = work[max_row][c];
     work[max_row][c] = tmp;
    }
    sign = -sign;
   }
   pivot = work[p][p];
   if (pivot > -1e-12 && pivot < 1e-12) {
    return bval_float(0.0); // singular
   }
   det_val *= pivot;
   // Eliminate below pivot
   for (r = p + 1; r < n; r++) {
    double factor = work[r][p] / pivot;
    for (c = p + 1; c < n; c++) {
     work[r][c] -= factor * work[p][c];
    }
   }
  }
  return bval_float(det_val * sign);
 }

 // Registry-based BValue function dispatch.
 //
 // Look up the keyword in the function registry. Parse
 // arguments into BValue array using the BValue expression
 // parser (preserving float/string types). Call the handler
 // and return the BValue result directly.
 {
 KeywordId kw = lex->current.value.keyword;
 const FunctionEntry *fn;
 fn = funcreg_find_by_keyword(kw);
 if (fn != NULL) {
 BValue args[16];
 int argc = 0;
 BValue result;

 lexer_next(lex); // consume function name

  if (fn->max_args > 0 &&
  lex->current.type == TOK_LPAREN) {
  // Parse (arg1, arg2, ...)
  lexer_next(lex); // consume (

  args[argc] = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;

 while (argc < fn->max_args &&
 lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 args[argc] = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 argc++;
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 }
 // else: zero-arg function (SIZE)

 // Validate argument count
 if (argc < fn->min_args) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 result = fn->handler(args, argc, (void *)rt);
 return result;
 }

 // TIMER - returns seconds since midnight (float).
 if (kw == KW_TIMER) {
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 t = time(NULL);
 tm = localtime(&t);
 return bval_float(
 (double)(tm->tm_hour * 3600 +
 tm->tm_min * 60 +
 tm->tm_sec));
 }

 // DATE$ - returns current date as "MM-DD-YYYY".
 // TIME$ - returns current time as "HH:MM:SS".
 if (kw == KW_DATE_FUNC) {
 char buf[16];
 char *ptr;
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 // $ already consumed by lexer
 t = time(NULL);
 tm = localtime(&t);
 sprintf(buf, "%02d-%02d-%04d",
 tm->tm_mon + 1, tm->tm_mday,
 tm->tm_year + 1900);
 ptr = strpool_store(&rt->strpool, buf, 10);
 return bval_string(ptr, 10);
 }
 if (kw == KW_TIME_FUNC) {
 char buf[16];
 char *ptr;
 time_t t;
 struct tm *tm;
 lexer_next(lex);
 t = time(NULL);
 tm = localtime(&t);
 sprintf(buf, "%02d:%02d:%02d",
 tm->tm_hour, tm->tm_min,
 tm->tm_sec);
 ptr = strpool_store(&rt->strpool, buf, 8);
 return bval_string(ptr, 8);
 }

  // CLOCK$ - returns full timestamp "YYYY-MM-DD HH:MM:SS".
  // More detailed than DATE$ or TIME$ alone.
 if (kw == KW_CLOCK_FUNC) {
 char buf[64];
 char *ptr;
 time_t t;
 struct tm *tm;
 int len;
 lexer_next(lex);
 t = time(NULL);
 tm = localtime(&t);
 sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
 tm->tm_year + 1900, tm->tm_mon + 1,
 tm->tm_mday, tm->tm_hour,
 tm->tm_min, tm->tm_sec);
 len = 19;
 ptr = strpool_store(&rt->strpool, buf, len);
 return bval_string(ptr, len);
 }

  // ALARM$ - get/set the alarm time string.
  // As a function (expression context), returns the
  // current alarm time setting. If no alarm is set,
  // returns empty string.
  //
  // Alarm time is stored in rt->alarm_str[].
  // Setting ALARM$ is done via ALARM$ = "HH:MM:SS"
  // in the statement handler (not here).
 if (kw == KW_ALARM_FUNC) {
 char *ptr;
 int len;
 lexer_next(lex);
 len = (int)strlen(rt->alarm_str);
 if (len == 0)
 return bval_string(NULL, 0);
 ptr = strpool_store(&rt->strpool,
 rt->alarm_str, len);
 return bval_string(ptr, len);
 }
  // DIALECT$ - returns the current dialect name.
  // Read-only introspection; does not change dialect.
  // Example: PRINT DIALECT$  -> "GW-BASIC"
  //          IF DIALECT$ = "GWBS" THEN ...
 if (kw == KW_DIALECT_FUNC) {
 const char *dname;
 char *ptr;
 int len;
 lexer_next(lex);
 dname = dialect_get_short_name();
 len = (int)strlen(dname);
 ptr = strpool_store(&rt->strpool, dname, len);
 return bval_string(ptr, len);
 }

  // MEMMAP$ - returns the current memory map name.
  // Read-only introspection; does not change memmap.
  // Example: PRINT MEMMAP$  -> "Commodore 64"
 if (kw == KW_MEMMAP_FUNC) {
 const char *mname;
 char *ptr;
 int len;
 lexer_next(lex);
 mname = memmap_get_name(
 (MemMapType)rt->memmap_type);
 len = (int)strlen(mname);
 ptr = strpool_store(&rt->strpool, mname, len);
 return bval_string(ptr, len);
 }
  // CWD$ - returns the current working directory.
  // Read-only string pseudo-variable.
  // CURDIR$ is an alias (mapped to KW_CWD_FUNC).
  // Example: PRINT CWD$   -> "C:\GAMES"
  //          A$ = CURDIR$
 if (kw == KW_CWD_FUNC) {
 char cwdbuf[512];
 char *ptr;
 int len;
 lexer_next(lex);
#ifdef _WIN32
 if (_getcwd(cwdbuf, sizeof(cwdbuf))
 == NULL)
#else
 if (getcwd(cwdbuf, sizeof(cwdbuf))
 == NULL)
#endif
 {
 cwdbuf[0] = '\0';
 }
 len = (int)strlen(cwdbuf);
 ptr = strpool_store(&rt->strpool,
 cwdbuf, len);
 return bval_string(ptr, len);
 }
  // ALIAS$(name$) - bidirectional alias lookup.
  // If name$ is an alias, returns the original keyword.
  // If name$ is a keyword, returns its alias (if any).
  // Returns empty string if neither found.
 if (kw == KW_ALIAS_FUNC) {
 BValue arg;
 const char *result;
 char *ptr;
 int len;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string(NULL, 0);
 arg = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_string(NULL, 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string(NULL, 0);

 if (!bval_is_string(&arg))
 return bval_string(NULL, 0);

 // Try forward: alias name -> keyword
 result = lexer_find_alias_by_name(
 arg.v.sval.data, arg.v.sval.length);
 if (result == NULL) {
 // Try reverse: keyword name -> alias
 int ki;
 KeywordId found_kw = KW_COUNT;
 for (ki = 0; ki < (int)KW_COUNT; ki++) {
 const char *kname =
 lexer_keyword_name((KeywordId)ki);
 if (kname[0] != '\0') {
 int klen = (int)strlen(kname);
 if (klen == arg.v.sval.length) {
 int j, m = 1;
 for (j = 0; j < klen; j++) {
 char ca = arg.v.sval.data[j];
 char cb = kname[j];
 if (ca>='a' && ca<='z') ca=(char)(ca-32);
 if (cb>='a' && cb<='z') cb=(char)(cb-32);
 if (ca != cb) { m=0; break; }
 }
 if (m) { found_kw=(KeywordId)ki; break; }
 }
 }
 }
 if (found_kw != KW_COUNT) {
 result = lexer_find_alias_for_keyword(found_kw);
 }
 }

 if (result == NULL)
 return bval_string(NULL, 0);

 len = (int)strlen(result);
 ptr = strpool_store(&rt->strpool, result, len);
 return bval_string(ptr, len);
 }

 // CINT(x) - round to integer.
 if (kw == KW_CINT) {
 double v;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 v = bval_to_float(&val);
 return bval_int((long)(v >= 0 ? v+0.5 : v-0.5));
 }

 // CSNG(x) - convert to single-precision float.
 // CDBL(x) - convert to double-precision float.
 // Both return double since BASIC++ uses double internally.
 if (kw == KW_CSNG || kw == KW_CDBL) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_float(bval_to_float(&val));
 }

 // CSRLIN - return current cursor row.
 // No parentheses needed (variable-like).
 if (kw == KW_CSRLIN) {
 lexer_next(lex);
 return bval_int((long)rt->cursor_row);
 }

 // ERL - last error line number.
 // Variable-like (no parentheses).
 if (kw == KW_ERL) {
 lexer_next(lex);
 return bval_int((long)rt->last_err_line);
 }

 // ERR - last error code.
 // Variable-like (no parentheses).
 if (kw == KW_ERR_VAR) {
 lexer_next(lex);
 return bval_int((long)rt->last_err_code);
 }

 // EXTERR(n) - DOS extended error.
 // Returns 0 (not applicable on modern OS).
 if (kw == KW_EXTERR) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 }
 return bval_int(0);
 }

 // ERDEV - device error code.
 // Returns 0 (not applicable).
 if (kw == KW_ERDEV) {
 lexer_next(lex);
 return bval_int(0);
 }

 // FRE(n) - Free memory query.
 //
 // GW-BASIC compatible:
 // FRE(0) = free string space
 // FRE("") = free string space
 // FRE(x$) = free string space
 // FRE(-1) = free stack space
 // FRE(-2) = free array/variable space
 //
 // BASIC++ extension:
 // FRE(-3) = variable pool free
 // FRE(n) for n>0 = total free (all pools)
 if (kw == KW_FRE) {
 long arg;
 long result;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 // Accept string arg: FRE("") or FRE(x$)
 if (lex->current.type == TOK_STRING ||
 lex->current.type == TOK_STRING_VAR) {
 lexer_next(lex);
 arg = 0;
 } else {
 arg = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 if (arg == 0) {
 // GW-BASIC: free string space
 result = rt->strpool.size
 - rt->strpool.used;
 } else if (arg == -1) {
 // GW-BASIC: free stack space
 result = mem_pool_available(
 &rt->memory->scratch);
 } else if (arg == -2) {
 // GW-BASIC: free array/var space
 result = mem_pool_available(
 &rt->memory->variable);
 } else if (arg == -3) {
 // BASIC++ ext: variable pool
 result = mem_pool_available(
 &rt->memory->variable);
 } else {
 // Total free (all pools)
 result = mem_pool_available(
 &rt->memory->variable)
 + (rt->strpool.size
 - rt->strpool.used)
 + mem_pool_available(
 &rt->memory->scratch);
 }
 return bval_int(result);
 }

 // EXIST("filename") - Check if file exists.
 // Returns 1 if the file can be opened, 0 if not.
 // Pure C89: uses fopen("rb") test.
 // Example: IF EXIST("GAME.BAS") THEN LOAD "GAME.BAS"
 if (kw == KW_EXIST_FUNC) {
 BValue arg;
 char fname[260];
 int flen;
 FILE *fp;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 arg = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&arg))
 return bval_int(0);
 flen = arg.v.sval.length;
 if (flen > 259) flen = 259;
 if (flen < 1 || arg.v.sval.data == NULL)
 return bval_int(0);
 memcpy(fname, arg.v.sval.data,
 (size_t)flen);
 fname[flen] = '\0';
 fp = fopen(fname, "rb");
 if (fp != NULL) {
 fclose(fp);
 return bval_int(1);
 }
 return bval_int(0);
 }

 // FILELEN("filename") - File size in bytes.
 // Opens the file, seeks to end, returns
 // the position (= file size in bytes).
 // Returns -1 if file not found.
 // Pure C89: fopen/fseek/ftell.
 // Example: PRINT FILELEN("DATA.BIN")
 if (kw == KW_FILELEN_FUNC) {
 BValue arg;
 char fname[260];
 int flen;
 FILE *fp;
 long sz;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(-1);
 arg = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(-1);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(-1);
 if (!bval_is_string(&arg))
 return bval_int(-1);
 flen = arg.v.sval.length;
 if (flen > 259) flen = 259;
 if (flen < 1 || arg.v.sval.data == NULL)
 return bval_int(-1);
 memcpy(fname, arg.v.sval.data,
 (size_t)flen);
 fname[flen] = '\0';
 fp = fopen(fname, "rb");
 if (fp == NULL)
 return bval_int(-1);
 fseek(fp, 0L, SEEK_END);
 sz = ftell(fp);
 fclose(fp);
 return bval_int(sz);
 }

 // INP(port) - Read I/O port.
 // No direct port access; returns 0.
 if (kw == KW_INP) {
 long port;
 int paddr;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 port = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 // Read from virtual memory segment.
 // On memory-mapped platforms (C64,
 // Atari, Apple, etc.) I/O ports live
 // in the address space, so INP and
 // PEEK are equivalent. On x86 (MSDOS)
 // the port space is separate, but we
 // map it into the same 64K array for
 // compatibility.
 paddr = (int)(port & 0xFFFF);
 if (paddr >= 0 &&
 paddr < MAX_MEM_SEGMENT)
 return bval_int(
 (long)rt->mem_segment[paddr]);
 return bval_int(0);
 }

 // SHELL$(command$) - Capture command output.
 //
 // Runs the command via popen/_ popen and
 // returns stdout as a string. Max 32K.
 if (kw == KW_SHELL) {
 BValue sv;
 char cmd[512];
 int cl;
 static char outbuf[32768];
 int outlen = 0;
 FILE *pp;
 char *poolbuf;

 lexer_next(lex);

 // Expect $(
 if (lex->current.type != TOK_LPAREN) {
 // No parens = not SHELL$, error
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 lexer_next(lex);

 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 if (!bval_is_string(&sv)) {
 error_raise(ERR_WHAT, line_num);
 return bval_string(NULL, 0);
 }

 cl = sv.v.sval.length;
 if (cl > 510) cl = 510;
 if (sv.v.sval.data)
 memcpy(cmd, sv.v.sval.data,
 (size_t)cl);
 cmd[cl] = '\0';

#if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
 pp = NULL; // popen not available on DOS
#elif defined(_WIN32)
 pp = _popen(cmd, "r");
#else
 pp = popen(cmd, "r");
#endif
 if (pp == NULL)
 return bval_string(NULL, 0);

 while (outlen < 32760) {
 int ch = fgetc(pp);
 if (ch == EOF) break;
 outbuf[outlen++] = (char)ch;
 }

#if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
 rt->last_shell_exitcode = -1;
#elif defined(_WIN32)
 rt->last_shell_exitcode =
 _pclose(pp);
#else
 rt->last_shell_exitcode =
 pclose(pp);
#endif
 // Strip trailing newline
 while (outlen > 0 &&
 (outbuf[outlen - 1] == '\n' ||
 outbuf[outlen - 1] == '\r'))
 outlen--;

 poolbuf = strpool_alloc(
 &rt->strpool, outlen);
 if (poolbuf && outlen > 0)
 memcpy(poolbuf, outbuf,
 (size_t)outlen);
 return bval_string(poolbuf, outlen);
 }

 // ERRORLEVEL - Return last SHELL exit code.
 // Used as a pseudo-variable in expressions.
 if (kw == KW_ERRORLEVEL) {
 lexer_next(lex);
 return bval_int(
 (long)rt->last_shell_exitcode);
 }

 // LOC(n) - File position.
 // Returns current byte position in file.
 if (kw == KW_LOC) {
 long chan;
 long result = 0;
 FILE *fp;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 chan = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 fp = fileio_get_fp((int)chan);
 if (fp) {
 result = (long)ftell(fp);
 if (result < 0) result = 0;
 }
 return bval_int(result);
 }

 // LPOS(n) - Printer head position.
 // Returns column position on printer.
 // No printer support; returns 0.
 if (kw == KW_LPOS) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(0);
 }

 // POS(x) - Return current cursor column.
 // Argument is a dummy (GW-BASIC compat).
 if (kw == KW_POS_FUNC) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 return bval_int(
 (long)rt->cursor_col);
 }

  // PMAP(coordinate, function)
  // Map between physical and view coords.
  // Stub: returns the input coordinate.
 if (kw == KW_PMAP) {
  long coord, pmap_fn;
  lexer_next(lex);
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_int(0);
  coord = parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(0);
  if (lex->current.type == TOK_COMMA)
   lexer_next(lex);
  pmap_fn = parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(0);
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_int(0);
  (void)pmap_fn;
  return bval_int(coord);
 }

 // PLAY(n) - Return number of notes in
 // background music buffer.
 // No sound hardware; always returns 0.
 if (kw == KW_PLAY) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 (void)parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_int(0);
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
  return bval_int(0);
 }
 // If no parens, fall through to
 // statement PLAY handling 
 return bval_int(0);
 }

  // STICK(n) - Return joystick position.
  // n=0: x of joystick A, n=1: y of A
  // n=2: x of joystick B, n=3: y of B
  // No joystick hardware; always returns 0.
 if (kw == KW_STICK) {
  lexer_next(lex);
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_int(0);
  (void)parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(0);
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_int(0);
  return bval_int(0);
 }

  // USR(n) - Call machine language routine.
  // In GW-BASIC, calls a user assembly routine
  // at the DEF USR address. No machine code
  // execution in this interpreter; consume
  // the argument and return 0.
 if (kw == KW_USR) {
  lexer_next(lex);
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_int(0);
  (void)parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(0);
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_int(0);
  return bval_int(0);
 }

 // VARPTR(var) - Return pointer to variable.
 // In GW-BASIC, returns the memory address
 // of a variable. We return a pseudo-index
 // based on the variable name (A=1..Z=26).
 if (kw == KW_VARPTR) {
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 if (lex->current.type == TOK_VARIABLE) {
 long idx = lex->current
 .value.var_name
 - 'A' + 1;
 lexer_next(lex);
 if (!lexer_expect(lex,
 TOK_RPAREN))
 return bval_int(0);
 return bval_int(idx);
 }
 // Named var or string var
 lexer_skip_to_end(lex);
 return bval_int(0);
 }

  // VARPTR$(var) - Return string pointer.
  // Returns a string representation of
  // the variable pointer. Stub: returns
  // empty string.
 if (kw == KW_VARPTR_STR) {
  lexer_next(lex);
  if (!lexer_expect(lex, TOK_LPAREN))
   return bval_string("", 0);
  // Consume variable argument
  (void)parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_string("", 0);
  if (!lexer_expect(lex, TOK_RPAREN))
   return bval_string("", 0);
  return bval_string("", 0);
 }

 // SCREEN(row, col [, flag])
 // Read character or attribute at screen pos.
 // flag=0 or omitted: return ASCII code.
 // flag=1: return color attribute.
  // No screen buffer; returns 32 (space).
 if (kw == KW_SCREEN) {
 lexer_next(lex);
 if (lex->current.type == TOK_LPAREN) {
  lexer_next(lex);
  (void)parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(32);
  if (lex->current.type == TOK_COMMA)
   lexer_next(lex);
  (void)parse_expression(lex, rt,
   line_num);
  if (error_occurred())
   return bval_int(32);
  // Optional 3rd arg (flag)
  if (lex->current.type ==
      TOK_COMMA) {
   lexer_next(lex);
   (void)parse_expression(
    lex, rt, line_num);
   if (error_occurred())
    return bval_int(0);
  }
  if (!lexer_expect(lex,
      TOK_RPAREN))
   return bval_int(32);
  return bval_int(32);
 }
 // No parens = SCREEN statement,
 // fall through 
 return bval_int(0);
 }

 // MKI$(n) - Pack integer into 2-byte string.
 // MKS$(n) - Pack single into 4-byte string.
 // MKD$(n) - Pack double into 8-byte string.
 // Used with FIELD/PUT for random-access files.
 if (kw == KW_MKI_FUNC ||
 kw == KW_MKS_FUNC ||
 kw == KW_MKD_FUNC) {
 double mkval;
 char buf[8];
 int blen;
 char *ptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 {
 BValue mkarg;
 mkarg = parse_expression_bval(
 lex, rt, line_num);
 mkval = bval_to_float(&mkarg);
 }
 if (error_occurred())
 return bval_string("", 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);

  if (kw == KW_MKI_FUNC) {
  // 2-byte integer (little-endian)
  short sv = (short)(long)mkval;
  memcpy(buf, &sv, 2);
  blen = 2;
  } else if (kw == KW_MKS_FUNC) {
  if (dialect_get_config()->id == DIALECT_GW_BASIC) {
      gw_double_to_mbf32(mkval, (uint8_t *)buf);
  } else {
      // 4-byte single float
      float fv = (float)mkval;
      memcpy(buf, &fv, 4);
  }
  blen = 4;
  } else {
  if (dialect_get_config()->id == DIALECT_GW_BASIC) {
      gw_double_to_mbf64(mkval, (uint8_t *)buf);
  } else {
      // 8-byte double
      memcpy(buf, &mkval, 8);
  }
  blen = 8;
  }
 ptr = strpool_store(
 &rt->strpool, buf, blen);
 if (ptr)
 return bval_string(ptr, blen);
 return bval_string("", 0);
 }

 // INPUT$(n [, #channel])
 // Read n characters from keyboard or file.
 // From keyboard: reads n chars without echo.
 // From file: reads n bytes from channel.
 if (kw == KW_INPUT_FUNC) {
 long nchars;
 int chan = 0;
 char buf[256];
 int i;
 char *ptr;
 BValue sv;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 nchars = parse_expression(lex, rt,
 line_num);
 if (error_occurred())
 return bval_string("", 0);
 if (nchars < 1) nchars = 1;
 if (nchars > 255) nchars = 255;
 // Optional channel: , #n
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred())
 return bval_string("", 0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);

 if (chan > 0) {
 // File: read n bytes
 FILE *fp = fileio_get_fp(chan);
 for (i = 0; i < (int)nchars; i++) {
 int ch;
 if (!fp) break;
 ch = fgetc(fp);
 if (ch == EOF) break;
 buf[i] = (char)ch;
 }
 } else {
 // Keyboard: read n chars
 for (i = 0; i < (int)nchars; i++) {
 int ch = getchar();
 if (ch == EOF) break;
 buf[i] = (char)ch;
 }
 }
 buf[i] = '\0';
 ptr = strpool_store(&rt->strpool,
 buf, i);
 if (!ptr) {
 error_raise(ERR_SORRY, line_num);
 return bval_string("", 0);
 }
 sv.type = VAL_STRING;
 sv.v.sval.data = ptr;
 sv.v.sval.length = i;
 return sv;
 }

 // IOCTL$(#n) - Read device control string.
 // Device-specific; returns empty string.
 if (kw == KW_IOCTL_FUNC) {
 // IOCTL$(#n) - Return device status.
 // Returns the channel mode as a string.
 int chan;
 int cmode;
 const char *st;
 char *p;
 int sl;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_string("", 0);
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred())
 return bval_string("", 0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_string("", 0);
 cmode = fileio_get_channel_mode(chan);
 switch (cmode) {
 case FCHAN_INPUT: st="I"; break;
 case FCHAN_OUTPUT: st="O"; break;
 case FCHAN_APPEND: st="A"; break;
 case FCHAN_RANDOM: st="R"; break;
 case FCHAN_BINARY: st="B"; break;
 default: st=""; break;
 }
 sl = (int)strlen(st);
 p = strpool_store(&rt->strpool,
 st, sl);
 return bval_string(p, sl);
 }

 // CVI(string$) - 2-byte string to integer.
 if (kw == KW_CVI) {
 BValue sv;
 int cvi_val;
 unsigned char lo, hi;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 2) {
 return bval_int(0);
 }
 lo = (unsigned char)sv.v.sval.data[0];
 hi = (unsigned char)sv.v.sval.data[1];
 cvi_val = (int)(lo | (hi << 8));
 if (cvi_val > 32767) cvi_val -= 65536;
 return bval_int((long)cvi_val);
 }

 // CVS(string$) - 4-byte string to float.
 if (kw == KW_CVS) {
 BValue sv;
 float f;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 4) {
 return bval_int(0);
 }
 if (dialect_get_config()->id == DIALECT_GW_BASIC) {
     double val = gw_mbf32_to_double((const uint8_t *)sv.v.sval.data);
     return bval_float(val);
 }
 memcpy(&f, sv.v.sval.data,
 sizeof(float));
 return bval_float((double)f);
 }

 // CVD(string$) - 8-byte string to double.
 if (kw == KW_CVD) {
 BValue sv;
 double d;
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);
 if (!bval_is_string(&sv) ||
 sv.v.sval.data == NULL ||
 sv.v.sval.length < 8) {
 return bval_int(0);
 }
 if (dialect_get_config()->id == DIALECT_GW_BASIC) {
     double val = gw_mbf64_to_double((const uint8_t *)sv.v.sval.data);
     return bval_float(val);
 }
 memcpy(&d, sv.v.sval.data,
 sizeof(double));
 return bval_float(d);
 }

 // INKEY$ - non-blocking keyboard read.
 // Returns empty string if no key, or 1-char string.
 if (kw == KW_INKEY) {
 int ch;
 lexer_next(lex);
 ch = vdev_inkey();
 if (ch > 0) {
 char kb[2];
 char *ptr;
 kb[0] = (char)ch;
 kb[1] = '\0';
 ptr = strpool_store(&rt->strpool, kb, 1);
 return bval_string(ptr, 1);
 }
 return bval_string(NULL, 0);
 }

 // ONKEY$ - event-aware keyboard read.
 // Currently behaves like INKEY$ (non-blocking).
 // Returns empty string if no key, or 1-char string.
 // Future: integrate with ON KEY GOSUB event trap.
 if (kw == KW_ONKEY) {
 int ch;
 lexer_next(lex);
 ch = vdev_inkey();
 if (ch > 0) {
 char kb[2];
 char *ptr;
 kb[0] = (char)ch;
 kb[1] = '\0';
 ptr = strpool_store(&rt->strpool, kb, 1);
 return bval_string(ptr, 1);
 }
 return bval_string(NULL, 0);
 }

 // CONST lookup: check if keyword matches a
 // stored constant name.
 {
 const char *knm = lex->current.str_start;
 int knl = lex->current.str_length;
 int ci;
 if (knm != NULL && knl > 0) {
 for (ci = 0; ci < rt->const_count;
 ci++) {
 if (rt->constants[ci].name_len ==
 knl &&
 memcmp(rt->constants[ci].name,
 knm, (size_t)knl)==0) {
 lexer_next(lex);
 return rt->constants[ci].value;
 }
 }
 }
 }

 // LCASE$(s$) - lowercase.
 // UCASE$(s$) - uppercase.
 // TCASE$(s$) - title case.
 // LTRIM$(s$) - trim left spaces.
 // RTRIM$(s$) - trim right spaces.
 // TRIM$(s$) - trim left and right spaces.
 if (kw == KW_LCASE || kw == KW_UCASE ||
 kw == KW_TCASE ||
 kw == KW_LTRIM || kw == KW_RTRIM ||
 kw == KW_TRIM) {
 char buf[256];
 char *ptr;
 const char *s;
 int slen, i;

 lexer_next(lex);
 // $ already consumed by lexer
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }
 if (slen > 255) slen = 255;

 if (kw == KW_LCASE) {
 for (i = 0; i < slen; i++)
 buf[i] = (char)tolower(
 (unsigned char)s[i]);
 ptr = strpool_store(&rt->strpool,
 buf, slen);
 return bval_string(ptr, slen);
 }
 if (kw == KW_UCASE) {
 for (i = 0; i < slen; i++)
 buf[i] = (char)toupper(
 (unsigned char)s[i]);
 ptr = strpool_store(&rt->strpool,
 buf, slen);
 return bval_string(ptr, slen);
 }
 if (kw == KW_TCASE) {
 int after_space = 1;
 for (i = 0; i < slen; i++) {
 unsigned char c = (unsigned char)s[i];
 if (c == ' ' || c == '\t') {
 buf[i] = (char)c;
 after_space = 1;
 } else if (after_space) {
 buf[i] = (char)toupper(c);
 after_space = 0;
 } else {
 buf[i] = (char)tolower(c);
 }
 }
 ptr = strpool_store(&rt->strpool,
 buf, slen);
 return bval_string(ptr, slen);
 }
 if (kw == KW_LTRIM) {
 i = 0;
 while (i < slen && s[i] == ' ') i++;
 ptr = strpool_store(&rt->strpool,
 s + i, slen - i);
 return bval_string(ptr, slen - i);
 }
 if (kw == KW_RTRIM) {
 i = slen;
 while (i > 0 && s[i-1] == ' ') i--;
 ptr = strpool_store(&rt->strpool,
 s, i);
 return bval_string(ptr, i);
 }
 if (kw == KW_TRIM) {
 int left = 0;
 int right = slen;
 while (left < right && s[left] == ' ')
 left++;
 while (right > left && s[right-1] == ' ')
 right--;
 ptr = strpool_store(&rt->strpool,
 s + left, right - left);
 return bval_string(ptr, right - left);
 }
 }

 // REPLACE$(source$, old$, new$) - Replace all occurrences.
 if (kw == KW_REPLACE) {
 BValue src, old_v, new_v;
 const char *sd, *od, *nd;
 int sl, ol, nl, ri, wi;
 char buf[MAX_LINE_LENGTH + 1];

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 src = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 lexer_next(lex);
 old_v = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 lexer_next(lex);
 new_v = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 sd = src.v.sval.data; sl = src.v.sval.length;
 od = old_v.v.sval.data; ol = old_v.v.sval.length;
 nd = new_v.v.sval.data; nl = new_v.v.sval.length;
 if (!sd) { sd = ""; sl = 0; }
 if (!od || ol == 0) {
 // Empty search: return source unchanged
 char *ptr = strpool_store(&rt->strpool, sd, sl);
 return bval_string(ptr, sl);
 }
 if (!nd) { nd = ""; nl = 0; }

 wi = 0;
 for (ri = 0; ri < sl && wi < MAX_LINE_LENGTH; ) {
 if (ri + ol <= sl &&
 memcmp(sd + ri, od, (size_t)ol) == 0) {
 // Match found: copy replacement
 int ci;
 for (ci = 0; ci < nl &&
 wi < MAX_LINE_LENGTH; ci++)
 buf[wi++] = nd[ci];
 ri += ol;
 } else {
 buf[wi++] = sd[ri++];
 }
 }
 {
 char *ptr = strpool_store(&rt->strpool,
 buf, wi);
 return bval_string(ptr, wi);
 }
 }

 // REVERSE$(s$) - Reverse string.
 if (kw == KW_REVERSE) {
 const char *s;
 int slen, i;
 char buf[256];
 char *ptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }
 if (slen > 255) slen = 255;
 for (i = 0; i < slen; i++)
 buf[i] = s[slen - 1 - i];
 ptr = strpool_store(&rt->strpool, buf, slen);
 return bval_string(ptr, slen);
 }

 // MCASE$(s$) - Mixed/random case.
 // Each character is randomly upper or lower case.
 // Uses the runtime's RNG (rnd_seed) for
 // deterministic behavior when seeded.
 if (kw == KW_MCASE) {
 const char *s;
 int slen, i;
 char buf[256];
 char *ptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }
 if (slen > 255) slen = 255;
 for (i = 0; i < slen; i++) {
 unsigned char c = (unsigned char)s[i];
 // Use PCG-derived bit for randomness
 uint64_t r = rt->rnd_seed;
 rt->rnd_seed = r * 6364136223846793005ULL
 + (12345ULL | 1);
 if ((r >> 17) & 1) {
 buf[i] = (char)toupper(c);
 } else {
 buf[i] = (char)tolower(c);
 }
 }
 ptr = strpool_store(&rt->strpool, buf, slen);
 return bval_string(ptr, slen);
 }

 // ICASE$(s$) - Invert case.
 // Swaps upper to lower and lower to upper
 // for every character. Non-alpha chars unchanged.
 if (kw == KW_ICASE) {
 const char *s;
 int slen, i;
 char buf[256];
 char *ptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }
 if (slen > 255) slen = 255;
 for (i = 0; i < slen; i++) {
 unsigned char c = (unsigned char)s[i];
 if (isupper(c))
 buf[i] = (char)tolower(c);
 else if (islower(c))
 buf[i] = (char)toupper(c);
 else
 buf[i] = (char)c;
 }
 ptr = strpool_store(&rt->strpool, buf, slen);
 return bval_string(ptr, slen);
 }

 // HASH$(s$ [, bits]) - Hash string to hex.
 // bits = 8, 16, 32 (default), 64, 128, 256.
 // Uses FNV-1a algorithm. For >64 bits, uses
 // multiple seeded rounds and concatenates.
 if (kw == KW_HASH) {
 const char *s;
 int slen, bits, i;
 unsigned long long h;
 char hexbuf[65]; // max 256 bits = 64 hex chars
 int hexlen;
 char *hptr;

 lexer_next(lex);
 if (!lexer_expect(lex, TOK_LPAREN))
 return bval_int(0);
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 bits = 32; // default
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 bits = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return bval_int(0);

 s = val.v.sval.data;
 slen = val.v.sval.length;
 if (s == NULL) { s = ""; slen = 0; }

 // Validate bit width
 if (bits != 8 && bits != 16 && bits != 32 &&
 bits != 64 && bits != 128 && bits != 256)
 bits = 32;

 hexlen = 0;
 {
 // Number of 64-bit rounds needed
 int rounds = (bits + 63) / 64;
 int r;
 if (rounds < 1) rounds = 1;
 if (rounds > 4) rounds = 4; // 256 max

 for (r = 0; r < rounds; r++) {
 // FNV-1a with per-round seed
 h = 14695981039346656037ULL +
 (unsigned long long)r * 6364136223846793005ULL;
 for (i = 0; i < slen; i++) {
 h ^= (unsigned char)s[i];
 h *= 1099511628211ULL;
 }
 // Fold to required width on last round
 if (bits <= 64 && rounds == 1) {
 if (bits == 8) {
 h = (h ^ (h >> 8) ^ (h >> 16) ^
 (h >> 24) ^ (h >> 32) ^
 (h >> 40) ^ (h >> 48) ^
 (h >> 56)) & 0xFF;
 sprintf(hexbuf, "%02X",
 (unsigned)h);
 hexlen = 2;
 } else if (bits == 16) {
 h = ((h >> 16) ^ h) & 0xFFFF;
 sprintf(hexbuf, "%04X",
 (unsigned)h);
 hexlen = 4;
 } else if (bits == 32) {
 h = ((h >> 32) ^ h) & 0xFFFFFFFFULL;
 sprintf(hexbuf, "%08X",
 (unsigned)h);
 hexlen = 8;
 } else {
 // 64-bit
 sprintf(hexbuf, "%08X%08X",
 (unsigned)(h >> 32),
 (unsigned)(h & 0xFFFFFFFFULL));
 hexlen = 16;
 }
 } else {
 // Multi-round: append 16 hex per round
 sprintf(hexbuf + hexlen, "%08X%08X",
 (unsigned)(h >> 32),
 (unsigned)(h & 0xFFFFFFFFULL));
 hexlen += 16;
 }
 }
 // Truncate to exact bit-width hex chars
 {
 int want = bits / 4;
 if (hexlen > want) hexlen = want;
 }
 }
 hptr = strpool_store(&rt->strpool,
 hexbuf, hexlen);
 return bval_string(hptr, hexlen);
 }

 // Unknown keyword in expression context
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }

 default:
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
}

 // parse_power_bval - BValue exponentiation (^) parser.
BValue pi_parse_power_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 left = pi_parse_factor_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 while (lex->current.type == TOK_CARET) {
 BValue right;
 double base_d, exp_d;
 lexer_next(lex); // consume ^
 right = pi_parse_factor_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 base_d = bval_to_float(&left);
 exp_d = bval_to_float(&right);
 left = bval_float(pow(base_d, exp_d));
 }

 return left;
}

 // parse_term_bval - BValue multiplicative expression parser.
 //
 // term = power ((*|/|\|MOD) power)*
BValue pi_parse_term_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 TokenType op;

 left = pi_parse_power_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 for (;;) {
 int is_mod = 0;
 int is_intdiv = 0;

 if (lex->current.type == TOK_STAR ||
 lex->current.type == TOK_SLASH) {
 op = lex->current.type;
 } else if (lex->current.type == TOK_BACKSLASH) {
 is_intdiv = 1;
 op = TOK_SLASH;
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_MOD) {
 is_mod = 1;
 op = TOK_SLASH; // placeholder
 } else {
 break;
 }

 lexer_next(lex);

 {
 BValue right;
 right = pi_parse_power_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (is_mod) {
 left = bval_mod(&left, &right, line_num);
 } else if (is_intdiv) {
 // Integer division: truncate both to int
 long a = bval_to_int(&left);
 long b = bval_to_int(&right);
 if (b == 0) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 left = bval_int(a / b);
 } else if (op == TOK_STAR) {
 left = bval_mul(&left, &right, line_num);
 } else {
 left = bval_div(&left, &right, line_num);
 }
 if (error_occurred()) return bval_int(0);
 }
 }

 return left;
}

 // parse_expression_bval - BValue additive expression parser.
 //
 // expression = [+|-|NOT] term ((+|-) term)*
 // ((AND|OR|XOR|EQV|IMP) expr)*
 //
 // String concatenation: when both operands are strings and the
 // operator is +, performs string concatenation instead of addition.
BValue parse_expression_bval(Lexer *lex, RuntimeState *rt, int line_num)
{
 BValue left;
 int negate = 0;
 int do_not = 0;

 if (error_occurred()) return bval_int(0);

 // Optional leading sign or NOT
 if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 do_not = 1;
 lexer_next(lex);
 // Handle sign after NOT: NOT -1, NOT +5
 if (lex->current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 }

 left = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (negate) {
 left = bval_neg(&left, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (do_not) {
 long v = bval_to_int(&left);
 left = bval_int(~v);
 }

 // Additive: + -
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue right;
 TokenType op = lex->current.type;
 lexer_next(lex);

 right = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 if (op == TOK_PLUS) {
 // Check for string concatenation
 if (bval_is_string(&left) && bval_is_string(&right)) {
 // ECMA-55 does not support string concatenation.
 // In strict mode, reject it. In union/normal
 // mode, allow it for GW-BASIC/QBasic compat.
 if (dialect_is_strict()) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 left = bval_concat(&left, &right, line_num,
 &rt->strpool);
 } else {
 left = bval_add(&left, &right, line_num);
 }
 } else {
 left = bval_sub(&left, &right, line_num);
 }
 if (error_occurred()) return bval_int(0);
 }

 // Comparison operators: = < > <= >= <>
 // These return -1 for true, 0 for false (QBasic convention).
 // Precedence: between additive and logical.
 if (lex->current.type == TOK_EQUALS ||
 lex->current.type == TOK_LT ||
 lex->current.type == TOK_GT ||
 lex->current.type == TOK_LT_EQ ||
 lex->current.type == TOK_GT_EQ ||
 lex->current.type == TOK_NOT_EQ) {
 TokenType cmp_op = lex->current.type;
 BValue right;
 int result = 0;

 lexer_next(lex); // consume comparison operator

 // Parse right side: unary + additive
 {
 int rn = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 right = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn) {
 right = bval_neg(&right, line_num);
 if (error_occurred()) return bval_int(0);
 }
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op2 == TOK_PLUS)
 right = bval_add(&right, &r2, line_num);
 else
 right = bval_sub(&right, &r2, line_num);
 if (error_occurred()) return bval_int(0);
 }
 }

 // Compare left and right
 if (bval_is_string(&left) && bval_is_string(&right)) {
 const char *ld = left.v.sval.data;
 int ll = left.v.sval.length;
 const char *rd = right.v.sval.data;
 int rl = right.v.sval.length;
 int cmp, minlen;
 if (ld == NULL) { ld = ""; ll = 0; }
 if (rd == NULL) { rd = ""; rl = 0; }
 minlen = ll < rl ? ll : rl;
 cmp = memcmp(ld, rd, (size_t)minlen);
 if (cmp == 0) {
 if (ll < rl) cmp = -1;
 else if (ll > rl) cmp = 1;
 }
 switch (cmp_op) {
 case TOK_EQUALS: result=(cmp==0); break;
 case TOK_LT: result=(cmp<0); break;
 case TOK_GT: result=(cmp>0); break;
 case TOK_LT_EQ: result=(cmp<=0); break;
 case TOK_GT_EQ: result=(cmp>=0); break;
 case TOK_NOT_EQ:result=(cmp!=0); break;
 default: break;
 }
 } else if (left.type == VAL_FLOAT ||
 right.type == VAL_FLOAT) {
 double lv = bval_to_float(&left);
 double rv = bval_to_float(&right);
 switch (cmp_op) {
 case TOK_EQUALS: result=(lv==rv); break;
 case TOK_LT: result=(lv<rv); break;
 case TOK_GT: result=(lv>rv); break;
 case TOK_LT_EQ: result=(lv<=rv); break;
 case TOK_GT_EQ: result=(lv>=rv); break;
 case TOK_NOT_EQ:result=(lv!=rv); break;
 default: break;
 }
 } else {
 long lv = bval_to_int(&left);
 long rv = bval_to_int(&right);
 switch (cmp_op) {
 case TOK_EQUALS: result=(lv==rv); break;
 case TOK_LT: result=(lv<rv); break;
 case TOK_GT: result=(lv>rv); break;
 case TOK_LT_EQ: result=(lv<=rv); break;
 case TOK_GT_EQ: result=(lv>=rv); break;
 case TOK_NOT_EQ:result=(lv!=rv); break;
 default: break;
 }
 }
 left = bval_int(result ? -1 : 0);
 }

 // LIKE operator: string LIKE "pattern"
 // Returns -1 for match, 0 for no match.
 // Pattern metacharacters:
 //   * = match any zero or more characters
 //   ? = match any single character
 //   # = match any single digit (0-9)
 //   [abc] = match any char in set
 //   [!abc] = match any char NOT in set
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_LIKE) {
 BValue right;
 const char *s, *p;
 int sl, pl;
 int match;

 lexer_next(lex); // consume LIKE
 right = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);

 // Both operands must be strings
 if (!bval_is_string(&left) ||
 !bval_is_string(&right)) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 s = left.v.sval.data;
 sl = left.v.sval.length;
 p = right.v.sval.data;
 pl = right.v.sval.length;
 if (!s) { s = ""; sl = 0; }
 if (!p) { p = ""; pl = 0; }

 // Glob match with stack-based backtracking
 {
 int si = 0, pi2 = 0;
 int star_pi = -1, star_si = -1;
 match = 0;
 while (si < sl) {
 if (pi2 < pl && p[pi2] == '*') {
 star_pi = pi2++;
 star_si = si;
 } else if (pi2 < pl && p[pi2] == '?') {
 si++; pi2++;
 } else if (pi2 < pl && p[pi2] == '#') {
 if (s[si] >= '0' && s[si] <= '9') {
 si++; pi2++;
 } else if (star_pi >= 0) {
 pi2 = star_pi + 1;
 si = ++star_si;
 } else break;
 } else if (pi2 < pl && p[pi2] == '[') {
 // Character class
 int neg2 = 0, found2 = 0;
 int ci = pi2 + 1;
 if (ci < pl && p[ci] == '!') {
 neg2 = 1; ci++;
 }
 while (ci < pl && p[ci] != ']') {
 if (p[ci] == s[si]) found2 = 1;
 ci++;
 }
 if (ci < pl) ci++; // skip ]
 if (found2 != neg2) {
 pi2 = ci; si++;
 } else if (star_pi >= 0) {
 pi2 = star_pi + 1;
 si = ++star_si;
 } else break;
 } else if (pi2 < pl &&
 (p[pi2] == s[si] ||
 ((p[pi2] >= 'A' && p[pi2] <= 'Z' ?
 p[pi2] + 32 : p[pi2]) ==
 (s[si] >= 'A' && s[si] <= 'Z' ?
 s[si] + 32 : s[si])))) {
 si++; pi2++;
 } else if (star_pi >= 0) {
 pi2 = star_pi + 1;
 si = ++star_si;
 } else break;
 }
 while (pi2 < pl && p[pi2] == '*') pi2++;
 if (si == sl && pi2 == pl) match = 1;
 }
 left = bval_int(match ? -1 : 0);
 }

 // Logical/bitwise: AND OR XOR EQV IMP
 while (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 long lv, rv;
 if (kw != KW_AND && kw != KW_OR &&
 kw != KW_XOR && kw != KW_EQV &&
 kw != KW_IMP) {
 break;
 }
 lexer_next(lex); // consume operator

 // Parse right side as full additive expr
 {
 BValue right;
 int rn = 0, rn2 = 0;
 if (lex->current.type == TOK_MINUS) {
 rn = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NOT) {
 rn2 = 1; lexer_next(lex);
 }
 right = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn) {
 right = bval_neg(&right, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (rn2) {
 long v = bval_to_int(&right);
 right = bval_int(~v);
 }

 // Inner additive loop
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r2;
 TokenType op2 = lex->current.type;
 lexer_next(lex);
 r2 = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op2 == TOK_PLUS)
 right = bval_add(&right, &r2, line_num);
 else
 right = bval_sub(&right, &r2, line_num);
 if (error_occurred()) return bval_int(0);
 }

 // Inner comparison check
 if (lex->current.type == TOK_EQUALS ||
 lex->current.type == TOK_LT ||
 lex->current.type == TOK_GT ||
 lex->current.type == TOK_LT_EQ ||
 lex->current.type == TOK_GT_EQ ||
 lex->current.type == TOK_NOT_EQ) {
 TokenType cop = lex->current.type;
 BValue rr;
 int cmp_res = 0;
 int rn3 = 0;
 lexer_next(lex);
 if (lex->current.type == TOK_MINUS) {
 rn3 = 1; lexer_next(lex);
 } else if (lex->current.type == TOK_PLUS) {
 lexer_next(lex);
 }
 rr = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (rn3) {
 rr = bval_neg(&rr, line_num);
 if (error_occurred()) return bval_int(0);
 }
 while (lex->current.type == TOK_PLUS ||
 lex->current.type == TOK_MINUS) {
 BValue r3;
 TokenType op3 = lex->current.type;
 lexer_next(lex);
 r3 = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (op3 == TOK_PLUS)
 rr = bval_add(&rr, &r3, line_num);
 else
 rr = bval_sub(&rr, &r3, line_num);
 if (error_occurred()) return bval_int(0);
 }
 if (right.type == VAL_FLOAT ||
 rr.type == VAL_FLOAT) {
 double lf = bval_to_float(&right);
 double rf = bval_to_float(&rr);
 switch (cop) {
 case TOK_EQUALS: cmp_res=(lf==rf); break;
 case TOK_LT: cmp_res=(lf<rf); break;
 case TOK_GT: cmp_res=(lf>rf); break;
 case TOK_LT_EQ: cmp_res=(lf<=rf); break;
 case TOK_GT_EQ: cmp_res=(lf>=rf); break;
 case TOK_NOT_EQ:cmp_res=(lf!=rf); break;
 default: break;
 }
 } else {
 long li = bval_to_int(&right);
 long ri = bval_to_int(&rr);
 switch (cop) {
 case TOK_EQUALS: cmp_res=(li==ri); break;
 case TOK_LT: cmp_res=(li<ri); break;
 case TOK_GT: cmp_res=(li>ri); break;
 case TOK_LT_EQ: cmp_res=(li<=ri); break;
 case TOK_GT_EQ: cmp_res=(li>=ri); break;
 case TOK_NOT_EQ:cmp_res=(li!=ri); break;
 default: break;
 }
 }
 right = bval_int(cmp_res ? -1 : 0);
 }

 // Inner LIKE check
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_LIKE) {
 BValue pat;
 const char *s2, *p2;
 int sl2, pl2, match2;
 lexer_next(lex);
 pat = pi_parse_term_bval(lex, rt, line_num);
 if (error_occurred()) return bval_int(0);
 if (!bval_is_string(&right) ||
 !bval_is_string(&pat)) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 s2 = right.v.sval.data;
 sl2 = right.v.sval.length;
 p2 = pat.v.sval.data;
 pl2 = pat.v.sval.length;
 if (!s2) { s2 = ""; sl2 = 0; }
 if (!p2) { p2 = ""; pl2 = 0; }
 {
 int si2 = 0, pi3 = 0;
 int sp = -1, ss = -1;
 match2 = 0;
 while (si2 < sl2) {
 if (pi3 < pl2 && p2[pi3] == '*') {
 sp = pi3++; ss = si2;
 } else if (pi3 < pl2 && p2[pi3] == '?') {
 si2++; pi3++;
 } else if (pi3 < pl2 && p2[pi3] == '#') {
 if (s2[si2]>='0' && s2[si2]<='9') {
 si2++; pi3++;
 } else if (sp >= 0) {
 pi3 = sp+1; si2 = ++ss;
 } else break;
 } else if (pi3 < pl2 && p2[pi3] == '[') {
 int ng = 0, fd = 0;
 int ci2 = pi3 + 1;
 if (ci2 < pl2 && p2[ci2]=='!') {
 ng=1; ci2++;
 }
 while (ci2<pl2 && p2[ci2]!=']') {
 if (p2[ci2]==s2[si2]) fd=1;
 ci2++;
 }
 if (ci2<pl2) ci2++;
 if (fd!=ng) {
 pi3=ci2; si2++;
 } else if (sp>=0) {
 pi3=sp+1; si2=++ss;
 } else break;
 } else if (pi3 < pl2 &&
 (p2[pi3]==s2[si2] ||
 ((p2[pi3]>='A'&&p2[pi3]<='Z'?
 p2[pi3]+32:p2[pi3])==
 (s2[si2]>='A'&&s2[si2]<='Z'?
 s2[si2]+32:s2[si2])))) {
 si2++; pi3++;
 } else if (sp >= 0) {
 pi3=sp+1; si2=++ss;
 } else break;
 }
 while (pi3<pl2 && p2[pi3]=='*') pi3++;
 if (si2==sl2 && pi3==pl2) match2=1;
 }
 right = bval_int(match2 ? -1 : 0);
 }

 rv = bval_to_int(&right);
 }

 lv = bval_to_int(&left);
 switch (kw) {
 case KW_AND: left = bval_int(lv & rv); break;
 case KW_OR: left = bval_int(lv | rv); break;
 case KW_XOR: left = bval_int(lv ^ rv); break;
 case KW_EQV: left = bval_int(~(lv ^ rv)); break;
 case KW_IMP: left = bval_int((~lv) | rv); break;
 default: break;
 }
 }

 return left;
}



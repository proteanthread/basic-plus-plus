 // ---
 // BASIC++ Interpreter - parser_assign.c
 // ---
 //
 // Assignment and variable declaration handlers: LET, DIM.
 //
 // Handles explicit and implicit variable assignment, string
 // variable assignment, array element assignment, and DIM
 // array declarations.
 //
//
// HOW TO EXTEND:
//   To add a new statement or sub-command:
//   1. Add the keyword to lexer.h (KeywordId enum).
//   2. Add it to the keyword table in lexer.c.
//   3. Add a handler function in this file.
//   4. Wire it into parser.c's dispatch switch.
//
// TROUBLESHOOTING:
//   - 'WHAT?' on valid syntax: check dialect feature flags.
//   - Crash in expression: ensure error_occurred() is checked
//     after every parse_expression call.
 // ---

#include "parser_internal.h"

 // parse_let - Parse and execute LET (or bare assignment).
 //
 // Syntax:
 // LET var = expr
 // var = expr (LET is optional in PATB)
 // LET @(expr) = expr
 // @(expr) = expr
 //
 // The 'has_let' parameter indicates whether the LET keyword was
 // already consumed by the caller.
void pi_parse_let(Lexer *lex, RuntimeState *rt, int line_num,
 int has_let)
{
 // Check for @() array assignment
 if (lex->current.type == TOK_AT) {
 long index, value;
 lexer_next(lex); // consume @
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 index = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;
 value = parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_array(rt, index, value);
 return;
 }

 // Standard variable assignment (or DIM array assignment)
 if (lex->current.type == TOK_VARIABLE) {
 char var_name;
 var_name = lex->current.value.var_name;
 lexer_next(lex); // consume variable
 // Check for typed var dot-access: V.field = expr
 if (lex->current.type == TOK_DOT) {
 TypedVar *tv = runtime_find_typed_var(
  rt, &var_name, 1);
 if (tv != NULL) {
  UserTypeDef *td =
  &rt->user_types[tv->type_index];
  const char *fname;
  int flen, fi;
  BValue bv;
  BValue *target = NULL;

  lexer_next(lex); // consume dot
  if (lex->current.type != TOK_NAMED_VAR &&
      lex->current.type != TOK_VARIABLE &&
      lex->current.type != TOK_KEYWORD) {
  error_raise(ERR_WHAT, line_num);
  return;
  }
  if (lex->current.type == TOK_NAMED_VAR) {
  fname = lex->current.str_start;
  flen = lex->current.str_length;
  } else if (lex->current.type ==
   TOK_KEYWORD) {
  fname = lexer_keyword_name(
   lex->current.value.keyword);
  flen = fname ?
   (int)strlen(fname) : 0;
  } else {
  fname = &lex->current.value.var_name;
  flen = 1;
  }
  fi = runtime_find_field(td, fname, flen);
  if (fi < 0) {
  error_raise(ERR_WHAT, line_num);
  return;
  }
  lexer_next(lex); // consume field
  target = &tv->fields[fi];

  // Nested type: walk into child TypedVar
  while (td->fields[fi].nested_type_index >= 0
      && lex->current.type == TOK_DOT) {
   int ci = (int)bval_to_int(target);
   if (ci < 0 || ci >= rt->typed_var_count) {
    error_raise(ERR_HOW, line_num);
    return;
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
    return;
   }
   fi = runtime_find_field(td, fname, flen);
   if (fi < 0) {
    error_raise(ERR_WHAT, line_num);
    return;
   }
   lexer_next(lex); // consume field
   target = &tv->fields[fi];
  }

  if (!lexer_expect(lex, TOK_EQUALS))
  return;
  bv = parse_expression_bval(lex, rt,
  line_num);
  if (error_occurred()) return;
  if (bval_is_string(&bv) &&
      bv.v.sval.data != NULL) {
  char *p = strpool_store(
   &rt->strpool,
   bv.v.sval.data,
   bv.v.sval.length);
  bv = bval_string(p,
   bv.v.sval.length);
  }
  if (target != NULL) *target = bv;
  return;
 }
 }

 // Check for DIM array assignment: A(i) = expr
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 int idx1, idx2 = 0, idx3 = 0;
 BValue val;
 lexer_next(lex); // consume (
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx3 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;

  // ASC() array unpack: X(n) = ASC(string$)
  // For 1D arrays, unpack each character's ASCII
  // value into consecutive slots starting at idx1.
  // Truncates if string exceeds remaining slots.
 if (idx2 == 0 && idx3 == 0 &&
 lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ASC) {
 BValue str_val;
 lexer_next(lex); // consume ASC
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 str_val = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (bval_is_string(&str_val) &&
 str_val.v.sval.length > 0 &&
 str_val.v.sval.data != NULL) {
 DimArray *arr = runtime_find_dim(rt,
 &var_name, 1);
 if (arr != NULL) {
 int i, max_fill;
 int slen = str_val.v.sval.length;
 max_fill = arr->size[0] - idx1;
 if (max_fill > slen) max_fill = slen;
 if (max_fill < 0) max_fill = 0;
 for (i = 0; i < max_fill; i++) {
 BValue cv = bval_int((long)
 (unsigned char)
 str_val.v.sval.data[i]);
 runtime_set_dim(rt,
 &var_name, 1,
 idx1 + i, 0, 0,
 cv, line_num);
 if (error_occurred()) return;
 }
 }
 }
 return;
 }

 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, &var_name, 1, idx1, idx2, idx3, val, line_num);
 return;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;

 // Whole-record assignment: B = A (single-letter)
 {
  TypedVar *lhs_tv = runtime_find_typed_var(
   rt, &var_name, 1);
  if (lhs_tv != NULL) {
   TypedVar *rhs_tv = NULL;
   if (lex->current.type == TOK_NAMED_VAR) {
    rhs_tv = runtime_find_typed_var(
     rt, lex->current.str_start,
     lex->current.str_length);
   } else if (lex->current.type == TOK_VARIABLE) {
    rhs_tv = runtime_find_typed_var(
     rt, &lex->current.value.var_name, 1);
   } else if (lex->current.type == TOK_KEYWORD) {
    const char *kn = lexer_keyword_name(
     lex->current.value.keyword);
    if (kn) rhs_tv = runtime_find_typed_var(
     rt, kn, (int)strlen(kn));
   }
   if (rhs_tv != NULL) {
    if (lhs_tv->type_index !=
        rhs_tv->type_index) {
     error_raise(ERR_WHAT, line_num);
     return;
    }
    lexer_next(lex);
    runtime_copy_typed_var(rt, lhs_tv,
     rhs_tv);
    return;
   }
  }
 }

 {
 BValue bv;
 bv = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 // FUNCTION / multi-line DEF FN return value:
 // if inside a function, check if var_name
 // matches the function name (or FN suffix).
 if (rt->in_sub_index >= 0 &&
 rt->in_sub_index < rt->sub_count &&
 rt->subs[rt->in_sub_index].is_function) {
 SubDef *cur =
 &rt->subs[rt->in_sub_index];
 char uc = (char)(var_name >= 'a' &&
 var_name <= 'z' ?
 var_name - 32 :
 var_name);
 // Direct match: FUNCTION S -> S = v
 if (cur->name_len == 1 &&
 cur->name[0] == uc) {
 rt->fn_return_value = bv;
 return;
 }
 // DEF FN match: FNS -> S = v
 if (cur->name_len == 3 &&
 cur->name[0] == 'F' &&
 cur->name[1] == 'N' &&
 cur->name[2] == uc) {
 rt->fn_return_value = bv;
 return;
 }
 }

 runtime_set_var_bval(rt, var_name, bv);
 }
 } else if (lex->current.type == TOK_NAMED_VAR) {
 // Extended variable assignment
 const char *name = lex->current.str_start;
 int name_len = lex->current.str_length;
 BValue bv;

 lexer_next(lex); // consume named variable

 // Dot-access for scalar TypedVar: Player.HP = 100
 // Must check before DIM array check.
 if (lex->current.type == TOK_DOT) {
 TypedVar *tv = runtime_find_typed_var(rt, name, name_len);
 if (tv != NULL) {
  UserTypeDef *td = &rt->user_types[tv->type_index];
  BValue *target = NULL;

  // Walk dot chain for nested types
  while (lex->current.type == TOK_DOT) {
  const char *fname;
  int flen, fi;
  lexer_next(lex); // consume dot
  if (lex->current.type != TOK_NAMED_VAR &&
       lex->current.type != TOK_VARIABLE &&
       lex->current.type != TOK_KEYWORD) {
   error_raise(ERR_WHAT, line_num);
   return;
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
   return;
  }
  lexer_next(lex); // consume field name
  target = &tv->fields[fi];

   // If this field is a nested type and more dots follow,
    // walk into the child TypedVar. The parent field
    // stores the child's typed_vars[] index. 
   if (td->fields[fi].nested_type_index >= 0 &&
       lex->current.type == TOK_DOT) {
    int ci = (int)bval_to_int(target);
    if (ci < 0 || ci >= rt->typed_var_count) {
     error_raise(ERR_HOW, line_num);
     return;
    }
    tv = &rt->typed_vars[ci];
    td = &rt->user_types[tv->type_index];
    target = NULL; // will be set next iteration
    continue;
   }
  }

  if (!lexer_expect(lex, TOK_EQUALS)) return;
  bv = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;

  // Store string values in string pool
  if (bval_is_string(&bv) && bv.v.sval.data != NULL) {
  char *p = strpool_store(&rt->strpool,
   bv.v.sval.data, bv.v.sval.length);
  bv = bval_string(p, bv.v.sval.length);
  }
  if (target != NULL)
  *target = bv;
  return;
 }
 }

 // Typed array dot-write: Enemies(1).HP = 100
 // Check if DIM array exists and is typed, then
 // handle subscript + dot + field.
 if (lex->current.type == TOK_LPAREN &&
     dialect_get_config()->has_dim_arrays) {
 DimArray *arr = runtime_find_dim(rt, name, name_len);
 if (arr != NULL && arr->type_index >= 0) {
  // Typed array: parse subscript, expect dot
  int idx1, idx2 = 0, idx3 = 0;
  int elem_idx;
  UserTypeDef *td;

  lexer_next(lex); // consume (
  idx1 = (int)parse_expression(lex, rt, line_num);
  if (error_occurred()) return;
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  idx2 = (int)parse_expression(lex, rt, line_num);
  if (error_occurred()) return;
  if (lex->current.type == TOK_COMMA) {
   lexer_next(lex);
   idx3 = (int)parse_expression(lex, rt, line_num);
   if (error_occurred()) return;
  }
  }
  if (!lexer_expect(lex, TOK_RPAREN)) return;

  td = &rt->user_types[arr->type_index];

  // Calculate element index
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
  BValue *fval;

  lexer_next(lex); // consume dot
  if (lex->current.type != TOK_NAMED_VAR &&
       lex->current.type != TOK_VARIABLE &&
       lex->current.type != TOK_KEYWORD) {
   error_raise(ERR_WHAT, line_num);
   return;
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
   return;
  }
  lexer_next(lex); // consume field name

  if (!lexer_expect(lex, TOK_EQUALS)) return;
  bv = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;

  // Store strings in pool
  if (bval_is_string(&bv) && bv.v.sval.data != NULL) {
   char *p = strpool_store(&rt->strpool,
   bv.v.sval.data, bv.v.sval.length);
   bv = bval_string(p, bv.v.sval.length);
  }

  fval = runtime_get_typed_array_field(
   rt, arr, elem_idx, fi);
  if (fval == NULL) {
   error_raise(ERR_HOW, line_num);
   return;
  }
  *fval = bv;
  return;
  }
  // No dot: fall through to normal array assign below
 }
 }

 // Check for DIM array assignment: Arr(i) = expr
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays &&
 runtime_find_dim(rt, name, name_len) != NULL) {
 int idx1, idx2 = 0, idx3 = 0;
 BValue dval;
 lexer_next(lex); // consume (
 idx1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx2 = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 idx3 = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (!lexer_expect(lex, TOK_EQUALS)) return;

  // ASC() array unpack: Arr(n) = ASC(string$)
  // Same as single-letter path above.
 if (idx2 == 0 && idx3 == 0 &&
 lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ASC) {
 BValue str_val;
 lexer_next(lex); // consume ASC
 if (!lexer_expect(lex, TOK_LPAREN)) return;
 str_val = parse_expression_bval(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN)) return;
 if (bval_is_string(&str_val) &&
 str_val.v.sval.length > 0 &&
 str_val.v.sval.data != NULL) {
 DimArray *arr = runtime_find_dim(rt,
 name, name_len);
 if (arr != NULL) {
 int i, max_fill;
 int slen = str_val.v.sval.length;
 max_fill = arr->size[0] - idx1;
 if (max_fill > slen) max_fill = slen;
 if (max_fill < 0) max_fill = 0;
 for (i = 0; i < max_fill; i++) {
 BValue cv = bval_int((long)
 (unsigned char)
 str_val.v.sval.data[i]);
 runtime_set_dim(rt,
 name, name_len,
 idx1 + i, 0, 0,
 cv, line_num);
 if (error_occurred()) return;
 }
 }
 }
 return;
 }

 dval = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_dim(rt, name, name_len,
 idx1, idx2, idx3, dval, line_num);
 return;
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;

 // Whole-record assignment: B = A
 // If LHS is a typed var and RHS is also a typed var
 // of the same type, copy all fields.
 {
  TypedVar *lhs_tv = runtime_find_typed_var(
   rt, name, name_len);
  if (lhs_tv != NULL) {
   // RHS must be a typed var name
   TypedVar *rhs_tv = NULL;
   if (lex->current.type == TOK_NAMED_VAR) {
    rhs_tv = runtime_find_typed_var(
     rt, lex->current.str_start,
     lex->current.str_length);
   } else if (lex->current.type == TOK_VARIABLE) {
    rhs_tv = runtime_find_typed_var(
     rt, &lex->current.value.var_name, 1);
   } else if (lex->current.type == TOK_KEYWORD) {
    const char *kn = lexer_keyword_name(
     lex->current.value.keyword);
    if (kn) rhs_tv = runtime_find_typed_var(
     rt, kn, (int)strlen(kn));
   }
   if (rhs_tv != NULL) {
    if (lhs_tv->type_index !=
        rhs_tv->type_index) {
     error_raise(ERR_WHAT, line_num);
     return;
    }
    lexer_next(lex); // consume RHS name
    runtime_copy_typed_var(rt, lhs_tv,
     rhs_tv);
    return;
   }
  }
 }

 bv = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 // FUNCTION return value: if we are inside a
 // FUNCTION and the LHS matches the function name,
 // set fn_return_value instead of a variable.
 if (rt->in_sub_index >= 0 &&
 rt->in_sub_index < rt->sub_count &&
 rt->subs[rt->in_sub_index].is_function) {
 SubDef *cur = &rt->subs[rt->in_sub_index];
 int fn_match = 0;
 if (cur->name_len == name_len) {
 // Case-insensitive compare
 int j, match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = cur->name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 }
 if (match) fn_match = 1;
 }
 // Multi-line DEF FN: function name is
 // "FN<x>", but LET uses just "<x>".
 // Match suffix after "FN" prefix.
 if (!fn_match && cur->name_len > 2 &&
 cur->name[0] == 'F' &&
 cur->name[1] == 'N' &&
 name_len == cur->name_len - 2) {
 int j, match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = cur->name[j + 2];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 }
 if (match) fn_match = 1;
 }
 if (fn_match) {
 rt->fn_return_value = bv;
 return;
 }
 }

 runtime_set_named_var_bval(rt, name, name_len, bv);
 } else if (lex->current.type == TOK_KEYWORD &&
 dialect_get_config()->has_extended_vars) {
  // GW-BASIC keyword-as-variable: RUNNING, DELAY, etc.
  // The lexer split it into a keyword prefix. Reconstruct
  // the full variable name from keyword + trailing chars.
 const char *kn = lexer_keyword_name(
  lex->current.value.keyword);
 if (kn != NULL) {
  char fullname[MAX_VAR_NAME_LEN + 1];
  int klen = (int)strlen(kn);
  int flen = klen;
  int saved_pos;
  BValue bv;

  if (klen > MAX_VAR_NAME_LEN) klen = MAX_VAR_NAME_LEN;
  memcpy(fullname, kn, (size_t)klen);

  lexer_next(lex); // consume keyword

  // Absorb trailing alpha/digit characters (NING from RUNNING)
  saved_pos = lex->pos;
  if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_start != NULL) {
  int tlen = lex->current.str_length;
  if (flen + tlen <= MAX_VAR_NAME_LEN) {
   memcpy(fullname + flen, lex->current.str_start,
   (size_t)tlen);
   flen += tlen;
  }
  lexer_next(lex); // consume trailing
  }
  fullname[flen] = '\0';

  if (!lexer_expect(lex, TOK_EQUALS)) return;
  bv = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;
  runtime_set_named_var_bval(rt, fullname, flen, bv);
  return;
 }
 error_raise(ERR_WHAT, line_num);
 } else if (lex->current.type == TOK_STRING_VAR) {
 // String variable or string array assignment
 char var_name = lex->current.value.var_name;
 BValue val;
 lexer_next(lex); // consume string variable

 // Check for DIM string array assignment: A$(idx) = expr
 // The DIM name is "A$" (2 chars).
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 char sname[3];
 sname[0] = var_name;
 sname[1] = '$';
 sname[2] = '\0';
 {
  int idx1, idx2 = 0, idx3 = 0;
  lexer_next(lex); // consume (
  val = parse_expression_bval(lex, rt, line_num);
  idx1 = (int)bval_to_subscript(&val);
  if (error_occurred()) return;
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  val = parse_expression_bval(lex, rt,
  line_num);
  idx2 = (int)bval_to_subscript(&val);
  if (error_occurred()) return;
  if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  val = parse_expression_bval(lex, rt,
  line_num);
  idx3 = (int)bval_to_subscript(&val);
  if (error_occurred()) return;
  }
  }
  if (!lexer_expect(lex, TOK_RPAREN)) return;
  if (!lexer_expect(lex, TOK_EQUALS)) return;
  val = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;
  runtime_set_dim(rt, sname, 2, idx1, idx2, idx3,
  val, line_num);
  return;
 }
 }

 if (!lexer_expect(lex, TOK_EQUALS)) return;
 val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;
 runtime_set_string_var(rt, var_name, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }

 (void)has_let; // suppress unused parameter warning
}

// --- DIM Statement Handler ---
 // Syntax:
 // DIM name(size) - 1D array
 // DIM name(size1,size2) - 2D array
void pi_parse_dim(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (!dialect_get_config()->has_dim_arrays) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // DIM supports comma-separated declarations:
 // DIM A(10), B(20), RM$(255)
 // DIM Player AS Enemy         (scalar typed var)
 // DIM Enemies(5) AS Enemy     (typed array)
 //
 // Each declaration is: name[(subscripts)] [AS TypeName]
 // Name can be TOK_VARIABLE (A), TOK_NAMED_VAR (RM, RM$),
 // or TOK_STRING_VAR (A$).
 do {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 int dim1;
 int dim2 = 0;
 int dim3 = 0;
 int has_subscripts = 0;

 // Get array name
 if (lex->current.type == TOK_VARIABLE) {
 name[0] = lex->current.value.var_name;
 name[1] = '\0';
 name_len = 1;
 lexer_next(lex);
 } else if (lex->current.type == TOK_STRING_VAR) {
 name[0] = lex->current.value.var_name;
 name[1] = '$';
 name[2] = '\0';
 name_len = 2;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR) {
 name_len = lex->current.str_length;
 if (name_len > MAX_VAR_NAME_LEN)
 name_len = MAX_VAR_NAME_LEN;
 memcpy(name, lex->current.str_start, (size_t)name_len);
 name[name_len] = '\0';
 lexer_next(lex);
 } else if (lex->current.type == TOK_KEYWORD) {
 // Keyword as var name (e.g., DIM Pos where POS
  // is a builtin function keyword) 
 const char *kn = lexer_keyword_name(
  lex->current.value.keyword);
 if (kn != NULL) {
 name_len = (int)strlen(kn);
 if (name_len > MAX_VAR_NAME_LEN)
 name_len = MAX_VAR_NAME_LEN;
 memcpy(name, kn, (size_t)name_len);
 name[name_len] = '\0';
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Check for AS TypeName (scalar typed var, no parens).
 // e.g. DIM Boss AS Enemy
 if (lex->current.type == TOK_KEYWORD &&
     lex->current.value.keyword == KW_AS) {
 // Scalar typed variable: DIM Name AS TypeName
 char tname[MAX_VAR_NAME_LEN + 1];
 int tlen;
 UserTypeDef *td;

 lexer_next(lex); // consume AS
 if (lex->current.type != TOK_NAMED_VAR &&
       lex->current.type != TOK_VARIABLE &&
       lex->current.type != TOK_KEYWORD) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 if (lex->current.type == TOK_NAMED_VAR) {
  tlen = lex->current.str_length;
  if (tlen > MAX_VAR_NAME_LEN) tlen = MAX_VAR_NAME_LEN;
  memcpy(tname, lex->current.str_start, (size_t)tlen);
 } else if (lex->current.type == TOK_KEYWORD) {
  const char *kn = lexer_keyword_name(lex->current.value.keyword);
  tlen = kn ? (int)strlen(kn) : 0;
  if (tlen > MAX_VAR_NAME_LEN) tlen = MAX_VAR_NAME_LEN;
  if (kn) memcpy(tname, kn, (size_t)tlen);
 } else {
  tname[0] = lex->current.value.var_name;
  tlen = 1;
 }
 tname[tlen] = '\0';
 lexer_next(lex); // consume type name

 td = runtime_find_type(rt, tname, tlen);
 if (td == NULL) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 {
  int tidx = (int)(td - rt->user_types);
  if (runtime_create_typed_var(
   rt, name, name_len, tidx) < 0) {
  error_raise(ERR_SORRY, line_num);
  return;
  }
 }
 goto dim_next;
 }

 if (!lexer_expect(lex, TOK_LPAREN)) return;
 has_subscripts = 1;

 dim1 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume comma
 dim2 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume comma
 dim3 = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 }
 }

 if (!lexer_expect(lex, TOK_RPAREN)) return;

 // Check for AS TypeName after subscripts.
 // e.g. DIM Enemies(5) AS Enemy
 // Creates a typed array with field-stride storage.
 if (has_subscripts &&
     lex->current.type == TOK_KEYWORD &&
     lex->current.value.keyword == KW_AS) {
 char tname[MAX_VAR_NAME_LEN + 1];
 int tlen;
 UserTypeDef *td;
 DimArray *arr;
 int tidx, total, i;
 int copy_len;

 lexer_next(lex); // consume AS
 if (lex->current.type != TOK_NAMED_VAR &&
       lex->current.type != TOK_VARIABLE &&
       lex->current.type != TOK_KEYWORD) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 if (lex->current.type == TOK_NAMED_VAR) {
  tlen = lex->current.str_length;
  if (tlen > MAX_VAR_NAME_LEN) tlen = MAX_VAR_NAME_LEN;
  memcpy(tname, lex->current.str_start, (size_t)tlen);
 } else if (lex->current.type == TOK_KEYWORD) {
  const char *kn = lexer_keyword_name(lex->current.value.keyword);
  tlen = kn ? (int)strlen(kn) : 0;
  if (tlen > MAX_VAR_NAME_LEN) tlen = MAX_VAR_NAME_LEN;
  if (kn) memcpy(tname, kn, (size_t)tlen);
 } else {
  tname[0] = lex->current.value.var_name;
  tlen = 1;
 }
 tname[tlen] = '\0';
 lexer_next(lex); // consume type name

 td = runtime_find_type(rt, tname, tlen);
 if (td == NULL) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 tidx = (int)(td - rt->user_types);

 // Calculate total BValues needed:
  // num_elements * field_count
 total = dim1 + 1 - rt->option_base;
 if (dim2 > 0) total *= (dim2 + 1 - rt->option_base);
 if (dim3 > 0) total *= (dim3 + 1 - rt->option_base);
 total *= td->field_count;

 if (rt->dim_count >= MAX_DIM_ARRAYS ||
     rt->dim_elements_used + total > MAX_ARRAY_ELEMENTS) {
  error_raise(ERR_SORRY, line_num);
  return;
 }

 // Check for re-DIM
 if (runtime_find_dim(rt, name, name_len) != NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 arr = &rt->dim_arrays[rt->dim_count];
 copy_len = name_len;
 if (copy_len > MAX_VAR_NAME_LEN) copy_len = MAX_VAR_NAME_LEN;
 memcpy(arr->name, name, (size_t)copy_len);
 arr->name[copy_len] = '\0';
 for (i = 0; i < copy_len; i++) {
  if (arr->name[i] >= 'a' && arr->name[i] <= 'z')
  arr->name[i] = (char)(arr->name[i] - 32);
 }

 if (dim3 > 0) arr->dims = 3;
 else if (dim2 > 0) arr->dims = 2;
 else arr->dims = 1;
 arr->size[0] = dim1 + 1 - rt->option_base;
 arr->size[1] = (dim2 > 0) ? dim2 + 1 - rt->option_base : 0;
 arr->size[2] = (dim3 > 0) ? dim3 + 1 - rt->option_base : 0;
 arr->elements = &rt->dim_elements[rt->dim_elements_used];
 arr->total = total;
 arr->type_index = tidx;

 // Initialize: string fields empty, numeric zero
 for (i = 0; i < total; i++) {
  int fi = i % td->field_count;
  if (td->fields[fi].is_string)
  arr->elements[i] = bval_string(NULL, 0);
  else
  arr->elements[i] = bval_int(0);
 }

 rt->dim_elements_used += total;
 rt->dim_count++;
 goto dim_next;
 }

 runtime_dim(rt, name, name_len, dim1, dim2, dim3, line_num);
 if (error_occurred()) return;

dim_next:
 // Check for comma -> more arrays to DIM
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume comma, loop
 } else {
 break;
 }
 } while (1);
}


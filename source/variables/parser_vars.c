 // ---
 // BASIC++ Interpreter - parser_vars.c
 // ---
 //
 // Variable, constant & type declaration commands.
 //
 // CONST, SWAP, REDIM, SHARED, STATIC, COMMON,
 // TYPE, DEFINT, DEFDBL, DEFSNG, DEFSTR, VARS.
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

 // pi_parse_const_stmt - Handle CONST_KW command.
void pi_parse_const_stmt(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CONST name = value
 // Store named constant in runtime table.
 {
 const char *nm;
 int nlen, ci;
 BValue cv;

 if (lex->current.type != TOK_KEYWORD &&
 lex->current.type != TOK_NAMED_VAR &&
 lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 if (nm == NULL || nlen == 0) {
 // Single letter variable
 nm = &lex->current.value.var_name;
 nlen = 1;
 }
 lexer_next(lex);
 if (!lexer_expect(lex, TOK_EQUALS))
 return;
 cv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (rt->const_count >= MAX_CONSTANTS) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 ci = rt->const_count++;
 if (nlen > 31) nlen = 31;
 memcpy(rt->constants[ci].name, nm,
 (size_t)nlen);
 rt->constants[ci].name[nlen] = '\0';
 rt->constants[ci].name_len = nlen;
 rt->constants[ci].value = cv;
 }
 return;
}

 // pi_parse_swap - Handle SWAP command.
void pi_parse_swap(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SWAP A,B - exchange two variable values.
 // Supports:
 // - Numeric scalars: SWAP A, B
 // - String scalars:  SWAP A$, B$
 // - Named variables: SWAP Score, Total
 // - Array elements:  SWAP A(1), A(2)
 // - String arrays:   SWAP A$(1), A$(2)
 // - Mixed named:     SWAP Names$(1), Names$(2)
 {
 // SwapRef describes one side of the SWAP. It captures
 // enough information to get/set the value.
 // kind: 0=var, 1=strvar, 2=named, 3=dim
 struct SwapRef {
 int kind;
 char var_name; // kind 0,1
 const char *nv_name; // kind 2
 int nv_len; // kind 2
 char dim_name[34]; // kind 3: array name
 int dim_nlen; // kind 3
 int idx1, idx2, idx3; // kind 3
 };
 struct SwapRef ref[2];
 BValue val[2];
 int side;

 for (side = 0; side < 2; side++) {
 if (side == 1) {
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume comma
 }

 if (lex->current.type == TOK_VARIABLE) {
 char vn = lex->current.value.var_name;
 lexer_next(lex);
 // Check for array element: A(i)
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays &&
 runtime_find_dim(rt, &vn, 1) != NULL) {
 int i1, i2 = 0, i3 = 0;
 lexer_next(lex); // consume (
 i1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 i2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 i3 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 ref[side].kind = 3;
 ref[side].dim_name[0] = vn;
 ref[side].dim_name[1] = '\0';
 ref[side].dim_nlen = 1;
 ref[side].idx1 = i1;
 ref[side].idx2 = i2;
 ref[side].idx3 = i3;
 } else {
 ref[side].kind = 0;
 ref[side].var_name = vn;
 }
 } else if (lex->current.type ==
 TOK_STRING_VAR) {
 char vn = lex->current.value.var_name;
 lexer_next(lex);
 // Check for string array: A$(i)
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays) {
 char sn[3];
 sn[0] = vn; sn[1] = '$'; sn[2] = '\0';
 if (runtime_find_dim(rt, sn, 2) != NULL) {
 int i1, i2 = 0, i3 = 0;
 BValue sv;
 lexer_next(lex); // (
 sv = parse_expression_bval(
 lex, rt, line_num);
 i1 = (int)bval_to_subscript(&sv);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 sv = parse_expression_bval(
 lex, rt, line_num);
 i2 = (int)bval_to_subscript(&sv);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 sv = parse_expression_bval(
 lex, rt, line_num);
 i3 = (int)bval_to_subscript(&sv);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 ref[side].kind = 3;
 ref[side].dim_name[0] = vn;
 ref[side].dim_name[1] = '$';
 ref[side].dim_name[2] = '\0';
 ref[side].dim_nlen = 2;
 ref[side].idx1 = i1;
 ref[side].idx2 = i2;
 ref[side].idx3 = i3;
 } else {
 ref[side].kind = 1;
 ref[side].var_name = vn;
 }
 } else {
 ref[side].kind = 1;
 ref[side].var_name = vn;
 }
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *nm = lex->current.str_start;
 int nl = lex->current.str_length;
 lexer_next(lex);
 // Check for named array: Arr(i)
 if (lex->current.type == TOK_LPAREN &&
 dialect_get_config()->has_dim_arrays &&
 runtime_find_dim(rt, nm, nl) != NULL) {
 int i1, i2 = 0, i3 = 0;
 lexer_next(lex); // (
 i1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 i2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 i3 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 }
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 ref[side].kind = 3;
 {
 int cl = nl;
 if (cl > 33) cl = 33;
 memcpy(ref[side].dim_name, nm,
 (size_t)cl);
 ref[side].dim_name[cl] = '\0';
 ref[side].dim_nlen = cl;
 }
 ref[side].idx1 = i1;
 ref[side].idx2 = i2;
 ref[side].idx3 = i3;
 } else {
 ref[side].kind = 2;
 ref[side].nv_name = nm;
 ref[side].nv_len = nl;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } // end for(side)

 // Get values from both sides
 for (side = 0; side < 2; side++) {
 switch (ref[side].kind) {
 case 0:
 val[side] = runtime_get_var_bval(
 rt, ref[side].var_name);
 break;
 case 1:
 val[side] = runtime_get_string_var(
 rt, ref[side].var_name);
 break;
 case 2:
 val[side] = runtime_get_named_var_bval(
 rt, ref[side].nv_name,
 ref[side].nv_len);
 break;
 case 3:
 val[side] = runtime_get_dim(
 rt, ref[side].dim_name,
 ref[side].dim_nlen,
 ref[side].idx1,
 ref[side].idx2,
 ref[side].idx3, line_num);
 if (error_occurred()) return;
 break;
 }
 }

 // Set swapped values
 switch (ref[0].kind) {
 case 0:
 runtime_set_var_bval(rt,
 ref[0].var_name, val[1]);
 break;
 case 1:
 runtime_set_string_var(rt,
 ref[0].var_name, val[1]);
 break;
 case 2:
 runtime_set_named_var_bval(rt,
 ref[0].nv_name,
 ref[0].nv_len, val[1]);
 break;
 case 3:
 runtime_set_dim(rt,
 ref[0].dim_name,
 ref[0].dim_nlen,
 ref[0].idx1, ref[0].idx2,
 ref[0].idx3,
 val[1], line_num);
 break;
 }
 switch (ref[1].kind) {
 case 0:
 runtime_set_var_bval(rt,
 ref[1].var_name, val[0]);
 break;
 case 1:
 runtime_set_string_var(rt,
 ref[1].var_name, val[0]);
 break;
 case 2:
 runtime_set_named_var_bval(rt,
 ref[1].nv_name,
 ref[1].nv_len, val[0]);
 break;
 case 3:
 runtime_set_dim(rt,
 ref[1].dim_name,
 ref[1].dim_nlen,
 ref[1].idx1, ref[1].idx2,
 ref[1].idx3,
 val[0], line_num);
 break;
 }
 }
 return;
}

 // pi_parse_redim - Handle REDIM and REDIM PRESERVE.
void pi_parse_redim(Lexer *lex, RuntimeState *rt, int line_num)
{
 // REDIM array(size)          -- erase and re-DIM
 // REDIM PRESERVE array(size) -- resize keeping data
 int preserve = 0;
 char rname[MAX_VAR_NAME_LEN + 1];
 int rlen = 0, ri;
 DimArray *existing;
 // Static buffer for PRESERVE: max 4096 elements
 static BValue saved[4096];
 int saved_count = 0;

 // Check for PRESERVE keyword
 if (lex->current.type == TOK_NAMED_VAR) {
  const char *s = lex->current.str_start;
  int slen = lex->current.str_length;
  if (slen == 8 &&
   (s[0]=='P'||s[0]=='p') &&
   (s[1]=='R'||s[1]=='r') &&
   (s[2]=='E'||s[2]=='e') &&
   (s[3]=='S'||s[3]=='s') &&
   (s[4]=='E'||s[4]=='e') &&
   (s[5]=='R'||s[5]=='r') &&
   (s[6]=='V'||s[6]=='v') &&
   (s[7]=='E'||s[7]=='e')) {
   preserve = 1;
   lexer_next(lex); // consume PRESERVE
  }
 }

 // Peek at array name
 if (lex->current.type == TOK_VARIABLE) {
  rname[0] = lex->current.value.var_name;
  rname[1] = '\0';
  rlen = 1;
 } else if (lex->current.type == TOK_NAMED_VAR) {
  rlen = lex->current.str_length;
  if (rlen > MAX_VAR_NAME_LEN)
   rlen = MAX_VAR_NAME_LEN;
  memcpy(rname, lex->current.str_start,
   (size_t)rlen);
  rname[rlen] = '\0';
 }
 // Uppercase for matching
 for (ri = 0; ri < rlen; ri++) {
  if (rname[ri] >= 'a' && rname[ri] <= 'z')
   rname[ri] = (char)(rname[ri] - 32);
 }

 // Find existing array
 existing = runtime_find_dim(rt, rname, rlen);
 if (existing != NULL) {
  if (preserve) {
   // Save existing data
   saved_count = existing->total;
   if (saved_count > 4096)
    saved_count = 4096;
   memcpy(saved, existing->elements,
    (size_t)saved_count * sizeof(BValue));
  }
  // Erase the old array
  existing->name[0] = '\0';
  existing->total = 0;
 }

 // Re-DIM with new size
 pi_parse_dim(lex, rt, line_num);
 if (error_occurred()) return;

 // Restore preserved data
 if (preserve && saved_count > 0) {
  DimArray *new_arr =
   runtime_find_dim(rt, rname, rlen);
  if (new_arr != NULL) {
   int copy_count = saved_count;
   if (copy_count > new_arr->total)
    copy_count = new_arr->total;
   memcpy(new_arr->elements, saved,
    (size_t)copy_count * sizeof(BValue));
  }
 }
 return;
}

 // scope_parse_varlist - Parse a variable name list.
 //
 // Consumes comma-separated variable names:
 //   SHARED A, B$, Counter, Arr()
 // Handles single-letter vars (TOK_VARIABLE),
 // string vars (TOK_STRING_VAR), named vars
 // (TOK_NAMED_VAR), and array indicators ().
 //
 // Returns the count of variables parsed.
 // Used by SHARED, STATIC, and COMMON.
static int scope_parse_varlist(Lexer *lex)
{
 int count = 0;

 for (;;) {
  if (lex->current.type == TOK_VARIABLE ||
   lex->current.type == TOK_STRING_VAR ||
   lex->current.type == TOK_NAMED_VAR) {
   lexer_next(lex);
   count++;

   // Skip optional () for arrays
   if (lex->current.type == TOK_LPAREN) {
    lexer_next(lex);
    if (lex->current.type == TOK_RPAREN)
     lexer_next(lex);
   }

   // Check for AS type clause
   if (lex->current.type == TOK_KEYWORD &&
    lex->current.value.keyword == KW_AS) {
    lexer_next(lex);
    // Skip type name
    if (lex->current.type == TOK_KEYWORD ||
     lex->current.type ==
      TOK_NAMED_VAR) {
     lexer_next(lex);
    }
   }

   // Comma continues the list
   if (lex->current.type == TOK_COMMA) {
    lexer_next(lex);
   } else {
    break;
   }
  } else {
   break;
  }
 }
 return count;
}

 // pi_parse_shared - Handle SHARED command.
void pi_parse_shared(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SHARED var1 [AS type], var2, ...
 //
 // QBasic: makes procedure-local variables refer to
 // the module-level copy instead. Changes propagate
 // back to the caller on scope exit.
 //
 // Register each variable with scope_stack_add_shared
 // so scope_stack_pop skips restoring them.
 while (lex->current.type != TOK_EOF &&
  lex->current.type != TOK_CR &&
  lex->current.type != TOK_COLON) {
  if (lex->current.type == TOK_NAMED_VAR) {
   const char *vn = lex->current.str_start;
   int vn_len = lex->current.str_length;
   scope_stack_add_shared(
    &rt->scope_stack, vn, vn_len);
   lexer_next(lex);
  } else if (lex->current.type ==
   TOK_VARIABLE) {
   // Single-letter: build 1-char name
   char buf[2];
   buf[0] = lex->current.value.var_name;
   buf[1] = '\0';
   scope_stack_add_shared(
    &rt->scope_stack, buf, 1);
   lexer_next(lex);
  } else if (lex->current.type ==
   TOK_STRING_VAR) {
   // String var: build "X$" name
   char buf[3];
   buf[0] = lex->current.value.var_name;
   buf[1] = '$'; buf[2] = '\0';
   scope_stack_add_shared(
    &rt->scope_stack, buf, 2);
   lexer_next(lex);
  } else {
   break;
  }
  // Skip optional () for arrays
  if (lex->current.type == TOK_LPAREN) {
   lexer_next(lex);
   if (lex->current.type == TOK_RPAREN)
    lexer_next(lex);
  }
  // Skip optional AS type
  if (lex->current.type == TOK_KEYWORD &&
   lex->current.value.keyword == KW_AS) {
   lexer_next(lex);
   if (lex->current.type == TOK_KEYWORD ||
    lex->current.type == TOK_NAMED_VAR)
    lexer_next(lex);
  }
  if (lex->current.type == TOK_COMMA)
   lexer_next(lex);
  else
   break;
 }
 (void)line_num;
 return;
}

 // pi_parse_static - Handle STATIC command.
void pi_parse_static(Lexer *lex, RuntimeState *rt, int line_num)
{
 // STATIC var1 [AS type], var2, ...
 //
 // QBasic: preserves variable values between calls
 // to the same SUB/FUNCTION. Without STATIC, local
 // variables are re-initialized on each entry.
 //
 // Register each variable with scope_stack_add_static
 // so it's saved back to SubDef on scope exit and
 // restored on the next entry.
 while (lex->current.type != TOK_EOF &&
  lex->current.type != TOK_CR &&
  lex->current.type != TOK_COLON) {
  if (lex->current.type == TOK_NAMED_VAR) {
   const char *vn = lex->current.str_start;
   int vn_len = lex->current.str_length;
   scope_stack_add_static(
    &rt->scope_stack, rt,
    vn, vn_len, 0, 0);
   lexer_next(lex);
  } else if (lex->current.type ==
   TOK_VARIABLE) {
   char vl = lex->current.value.var_name;
   scope_stack_add_static(
    &rt->scope_stack, rt,
    NULL, 0, vl, 0);
   lexer_next(lex);
  } else if (lex->current.type ==
   TOK_STRING_VAR) {
   char vl = lex->current.value.var_name;
   scope_stack_add_static(
    &rt->scope_stack, rt,
    NULL, 0, vl, 1);
   lexer_next(lex);
  } else {
   break;
  }
  // Skip optional () for arrays
  if (lex->current.type == TOK_LPAREN) {
   lexer_next(lex);
   if (lex->current.type == TOK_RPAREN)
    lexer_next(lex);
  }
  // Skip optional AS type
  if (lex->current.type == TOK_KEYWORD &&
   lex->current.value.keyword == KW_AS) {
   lexer_next(lex);
   if (lex->current.type == TOK_KEYWORD ||
    lex->current.type == TOK_NAMED_VAR)
    lexer_next(lex);
  }
  if (lex->current.type == TOK_COMMA)
   lexer_next(lex);
  else
   break;
 }
 (void)line_num;
 return;
}

 // pi_parse_common - Handle COMMON command.
void pi_parse_common(Lexer *lex, RuntimeState *rt, int line_num)
{
 // COMMON [SHARED] var1, var2, ...
 //
 // GW-BASIC/QBasic: declares variables that
 // persist across CHAIN to another program.
 // Without COMMON, variables are cleared
 // when CHAINing.
 //
 // BASIC++ architecture: CHAIN runs in the
 // same process with the same RuntimeState,
 // so all variables naturally survive CHAIN.
 // This makes COMMON a semantic no-op.
 //
 // Hybrid mode: parse optional SHARED keyword
 // and the variable list correctly.
 // Consume optional SHARED keyword
 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword == KW_SHARED) {
  lexer_next(lex);
 }
 (void)scope_parse_varlist(lex);
 return;

}

 // pi_parse_type - Handle TYPE command.
void pi_parse_type(Lexer *lex, RuntimeState *rt, int line_num)
{
 // TYPE name
 // field AS STRING/INTEGER/...
 // END TYPE
 //
 // Define a user type. Scan lines until
 // END TYPE is found.
 {
 char tname[MAX_VAR_NAME_LEN + 1];
 int tlen = 0, ti;
 int tidx;
 UserTypeDef *td;
 ProgramStore *pgm = rt->program;

 // Get type name
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 tlen = lex->current.str_length;
 if (tlen > MAX_VAR_NAME_LEN)
 tlen = MAX_VAR_NAME_LEN;
 memcpy(tname,
 lex->current.str_start,
 (size_t)tlen);
 } else if (lex->current.type ==
 TOK_VARIABLE) {
 tname[0] = lex->current.value
 .var_name;
 tlen = 1;
 } else if (lex->current.type ==
 TOK_KEYWORD) {
 // Allow keyword names as type names
 // (e.g., TYPE Point where POINT is
 // a builtin function keyword) 
 const char *kn =
 lexer_keyword_name(
 lex->current.value.keyword);
 if (kn != NULL) {
 tlen = (int)strlen(kn);
 if (tlen > MAX_VAR_NAME_LEN)
 tlen = MAX_VAR_NAME_LEN;
 memcpy(tname, kn, (size_t)tlen);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 tname[tlen] = '\0';
 lexer_next(lex);

 if (rt->type_count >=
 MAX_USER_TYPES) {
 error_raise(ERR_SORRY, line_num);
 return;
 }
 tidx = rt->type_count;
 td = &rt->user_types[tidx];
 memcpy(td->name, tname,
 (size_t)(tlen + 1));
 td->field_count = 0;

 // Scan forward for fields
 ti = rt->current_index + 1;
 while (ti < pgm->count) {
 Lexer fl;
 const char *src;

 src = pgm->lines[ti].text;
 lexer_init(&fl, src);

 // Skip line number
 if (fl.current.type == TOK_NUMBER)
 lexer_next(&fl);

 // Check for END TYPE
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value.keyword ==
 KW_END) {
 lexer_next(&fl);
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value
 .keyword ==
 KW_TYPE) {
 break;
 }
 }

 // Parse field: name AS type
 if (fl.current.type ==
 TOK_NAMED_VAR ||
 fl.current.type ==
 TOK_VARIABLE ||
 fl.current.type ==
 TOK_KEYWORD) {
 int fi = td->field_count;
 if (fi < MAX_TYPE_FIELDS) {
 int nl;
 if (fl.current.type ==
 TOK_NAMED_VAR) {
 nl = fl.current
 .str_length;
 if (nl >
 MAX_VAR_NAME_LEN)
 nl =
 MAX_VAR_NAME_LEN;
 memcpy(
 td->fields[fi]
 .name,
 fl.current
 .str_start,
 (size_t)nl);
 td->fields[fi]
 .name[nl] =
 '\0';
 } else if (fl.current.type ==
 TOK_KEYWORD) {
 // Keyword as field name
 // (e.g., Name, Color) 
 const char *kn =
 lexer_keyword_name(
 fl.current.value
 .keyword);
 if (kn != NULL) {
 nl = (int)strlen(kn);
 if (nl >
 MAX_VAR_NAME_LEN)
 nl =
 MAX_VAR_NAME_LEN;
 memcpy(
 td->fields[fi]
 .name,
 kn, (size_t)nl);
 td->fields[fi]
 .name[nl] =
 '\0';
 } else {
 td->fields[fi]
 .name[0] = '?';
 td->fields[fi]
 .name[1] = '\0';
 }
 } else {
 td->fields[fi]
 .name[0] =
 fl.current.value
 .var_name;
 td->fields[fi]
 .name[1] =
 '\0';
 }
 td->fields[fi]
 .is_string = 0;
 td->fields[fi]
 .nested_type_index = -1;
 lexer_next(&fl);
 // Skip AS keyword
 if (fl.current.type ==
 TOK_KEYWORD &&
 fl.current.value
 .keyword ==
 KW_AS) {
 lexer_next(&fl);
 // Check STRING
 if (fl.current.type
 == TOK_KEYWORD
 &&
 fl.current.value
 .keyword ==
 KW_STRING_FUNC) {
 td->fields[fi]
 .is_string =
 1;
 } else if (
 fl.current.type ==
 TOK_NAMED_VAR) {
 // Check for nested type
 UserTypeDef *nt =
 runtime_find_type(
 rt,
 fl.current.str_start,
 fl.current
 .str_length);
 if (nt != NULL) {
 int nti = (int)(nt -
 rt->user_types);
 td->fields[fi]
 .nested_type_index
 = nti;
 }
 }
 }
 td->field_count++;
 }
 }
 ti++;
 }

 // Skip past END TYPE
 rt->current_index = ti;
 rt->type_count++;
 }
 return;
}

 // deftype_parse_range - Parse letter range and set type.
 //
 // Parses GW-BASIC letter range syntax:
 //   DEFINT A-C, X-Z
 //   DEFDBL I
 //   DEFSNG A-Z
 //
 // For each letter in the range, sets
 // rt->deftype_map[letter_index] to the given
 // type code. Supports comma-separated ranges.
 //
 // Hybrid mode: the deftype_map is populated and
 // enforced at runtime - assigning a float to an
 // integer-declared variable silently truncates.
static void deftype_parse_range(Lexer *lex,
 RuntimeState *rt, int line_num,
 unsigned char dtype)
{
 for (;;) {
  char start_ch, end_ch;
  int si, ei;

  // Get start letter
  if (lex->current.type == TOK_VARIABLE) {
   start_ch = lex->current.value.var_name;
  } else if (lex->current.type ==
   TOK_NAMED_VAR &&
   lex->current.str_length >= 1) {
   start_ch = lex->current.str_start[0];
   if (start_ch >= 'a' && start_ch <= 'z')
    start_ch = (char)(start_ch - 32);
  } else {
   // No more range specs
   break;
  }
  lexer_next(lex);

  // Default: single letter range
  end_ch = start_ch;

  // Check for '-' range separator
  if (lex->current.type == TOK_MINUS) {
   lexer_next(lex);
   if (lex->current.type == TOK_VARIABLE) {
    end_ch = lex->current.value.var_name;
   } else if (lex->current.type ==
    TOK_NAMED_VAR &&
    lex->current.str_length >= 1) {
    end_ch = lex->current.str_start[0];
    if (end_ch >= 'a' && end_ch <= 'z')
     end_ch = (char)(end_ch - 32);
   }
   lexer_next(lex);
  }

  // Validate range
  if (start_ch >= 'A' && start_ch <= 'Z' &&
   end_ch >= 'A' && end_ch <= 'Z') {
   si = start_ch - 'A';
   ei = end_ch - 'A';
   if (si > ei) { int t = si; si = ei; ei = t; }
   while (si <= ei) {
    rt->deftype_map[si] = dtype;
    si++;
   }
  }

  // Comma for next range, or done
  if (lex->current.type == TOK_COMMA) {
   lexer_next(lex);
  } else {
   break;
  }
 }
 return;
}

 // pi_parse_defint - Handle DEFINT command.
void pi_parse_defint(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DEFINT letter-range [, letter-range ...]
 // Declare variables starting with the given
 // letters as INTEGER type. Enforced at runtime:
 // populates deftype_map for coercion on assignment.
 deftype_parse_range(lex, rt, line_num,
  DEFTYPE_INT);
 return;
}

 // pi_parse_defdbl - Handle DEFDBL command.
void pi_parse_defdbl(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DEFDBL letter-range [, letter-range ...]
 // Declare variables starting with the given
 // letters as DOUBLE type. Enforced at runtime:
 // populates deftype_map for coercion on assignment.
 deftype_parse_range(lex, rt, line_num,
  DEFTYPE_DBL);
 return;
}

 // pi_parse_defsng - Handle DEFSNG command.
void pi_parse_defsng(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DEFSNG letter-range [, letter-range ...]
 // Declare variables starting with the given
 // letters as SINGLE type. Enforced at runtime:
 // populates deftype_map for coercion on assignment.
 deftype_parse_range(lex, rt, line_num,
  DEFTYPE_SNG);
 return;
}

 // pi_parse_defstr - Handle DEFSTR command.
void pi_parse_defstr(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DEFSTR letter-range [, letter-range ...]
 // Declare variables starting with the given
 // letters as STRING type. Enforced at runtime:
 // populates deftype_map for coercion on assignment.
 deftype_parse_range(lex, rt, line_num,
  DEFTYPE_STR);
 return;
}


 // pi_parse_vars - Handle VARS command.
void pi_parse_vars(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
  // VARS [ENV | SYSTEM | ALL]
  //
  // With no argument: show BASIC++ program variables
  // (A-Z, A$-Z$, named vars, DIM arrays, CONST).
  //
  // VARS ENV: List OS environment variables (the
  // process environment block). Read-only listing.
  // This is NOT the same as ENVIRON$ which gets/sets
  // individual env vars by name -- VARS ENV dumps
  // the entire environment table.
  //
  // VARS SYSTEM: List BASIC++ interpreter internal
  // environment variables (dialect, version, security
  // level, platform, build info, etc.) as key=value.
  //
  // VARS ALL: Show all three sections.
 int show_prog = 0;
 int show_env = 0;
 int show_sys = 0;
 int show_user = 0;
 int v;
 int printed;

 // Parse optional sub-keyword.
  //
  // VARS is a meta-command -- its argument (ENV, USER,
  // SYSTEM, ALL) is not a BASIC expression. In dialects
  // with single-letter variables (PATB), the lexer has
  // already tokenized e.g. "USER" as variable 'U' with
  // the remaining "SER" still in lex->source after the
  // current token. We reconstruct the full word by
  // combining the current token with what follows.
  //
  // In dialects with named variables (QBasic), the
  // lexer gives us the full word as TOK_NAMED_VAR.
 {
     char word[16];
     int wlen = 0;
     const char *rest;

     if (lex->current.type == TOK_NAMED_VAR) {
         // Multi-char var: copy name directly
         int n = lex->current.str_length;
         if (n > 15) n = 15;
         memcpy(word, lex->current.str_start,
             (size_t)n);
         wlen = n;
         word[wlen] = '\0';
         lexer_next(lex); // consume it
     } else if (lex->current.type ==
                TOK_KEYWORD) {
         // SYSTEM is a keyword
         if (lex->current.value.keyword ==
             KW_SYSTEM) {
             word[0] = 'S'; word[1] = 'Y';
             word[2] = 'S'; word[3] = 'T';
             word[4] = 'E'; word[5] = 'M';
             wlen = 6;
             word[wlen] = '\0';
             lexer_next(lex);
         }
     } else if (lex->current.type ==
                TOK_VARIABLE) {
         // Single-letter var: reconstruct word.
          // The lexer consumed one letter; remaining
          // alphabetic chars are still in source. 
         word[0] = lex->current.value.var_name;
         wlen = 1;
         rest = lex->source + lex->pos;
         while (wlen < 15 &&
                ((rest[0] >= 'A' &&
                  rest[0] <= 'Z') ||
                 (rest[0] >= 'a' &&
                  rest[0] <= 'z'))) {
             word[wlen++] = *rest++;
         }
         word[wlen] = '\0';
         // Advance lexer past the extra chars
         lex->pos = (int)(rest - lex->source);
         lexer_next(lex);
     } else if (lex->current.type == TOK_CR ||
                lex->current.type == TOK_EOF) {
         // No argument: show program vars
         wlen = 0;
         word[0] = '\0';
     }

     // Match the reconstructed word
     if (wlen == 3 &&
         (word[0] == 'E' || word[0] == 'e') &&
         (word[1] == 'N' || word[1] == 'n') &&
         (word[2] == 'V' || word[2] == 'v')) {
         show_env = 1;
     } else if (wlen == 4 &&
         (word[0] == 'U' || word[0] == 'u') &&
         (word[1] == 'S' || word[1] == 's') &&
         (word[2] == 'E' || word[2] == 'e') &&
         (word[3] == 'R' || word[3] == 'r')) {
         show_user = 1;
     } else if (wlen == 6 &&
         (word[0] == 'S' || word[0] == 's') &&
         (word[1] == 'Y' || word[1] == 'y') &&
         (word[2] == 'S' || word[2] == 's') &&
         (word[3] == 'T' || word[3] == 't') &&
         (word[4] == 'E' || word[4] == 'e') &&
         (word[5] == 'M' || word[5] == 'm')) {
         show_sys = 1;
     } else if (wlen == 3 &&
         (word[0] == 'A' || word[0] == 'a') &&
         (word[1] == 'L' || word[1] == 'l') &&
         (word[2] == 'L' || word[2] == 'l')) {
         show_prog = 1;
         show_env = 1;
         show_sys = 1;
         show_user = 1;
     } else {
         // No match or no argument: program vars
         show_prog = 1;
     }
 }

 // ---- Section 1: BASIC++ Program Variables ----
 if (show_prog) {
     printed = 0;

     printf("=== PROGRAM VARIABLES ===\n\n");

     // Numeric variables A-Z
     for (v = 0; v < MAX_VARIABLES; v++) {
         if (rt->variables[v].type ==
             VAL_INTEGER &&
             rt->variables[v].v.ival != 0) {
             printf(" %c = %ld\n",
                 'A' + v,
                 rt->variables[v].v.ival);
             printed = 1;
         } else if (rt->variables[v].type ==
             VAL_FLOAT &&
             rt->variables[v].v.fval
             != 0.0) {
             printf(" %c = %g\n",
                 'A' + v,
                 rt->variables[v].v.fval);
             printed = 1;
         }
     }

     // String variables A$-Z$
     for (v = 0; v < MAX_STRING_VARS; v++) {
         if (rt->string_vars[v].type ==
             VAL_STRING &&
             rt->string_vars[v].v.sval.data !=
             NULL &&
             rt->string_vars[v].v.sval.length
             > 0) {
             printf(" %c$ = \"%.*s\"\n",
                 'A' + v,
                 rt->string_vars[v].v.sval
                 .length,
                 rt->string_vars[v].v.sval
                 .data);
             printed = 1;
         }
     }

     // Named variables
     for (v = 0; v < rt->named_count; v++) {
         if (rt->named_vars[v].value.type ==
             VAL_INTEGER &&
             rt->named_vars[v].value.v.ival
             != 0) {
             printf(" %s = %ld\n",
                 rt->named_vars[v].name,
                 rt->named_vars[v]
                 .value.v.ival);
             printed = 1;
         } else if (
             rt->named_vars[v].value.type ==
             VAL_FLOAT &&
             rt->named_vars[v].value.v.fval
             != 0.0) {
             printf(" %s = %g\n",
                 rt->named_vars[v].name,
                 rt->named_vars[v]
                 .value.v.fval);
             printed = 1;
         } else if (
             rt->named_vars[v].value.type ==
             VAL_STRING &&
             rt->named_vars[v].value.v.sval
             .data != NULL &&
             rt->named_vars[v].value.v.sval
             .length > 0) {
             printf(" %s = \"%.*s\"\n",
                 rt->named_vars[v].name,
                 rt->named_vars[v].value
                 .v.sval.length,
                 rt->named_vars[v].value
                 .v.sval.data);
             printed = 1;
         }
     }

     // DIM arrays (skip typed arrays, shown separately)
     for (v = 0; v < rt->dim_count; v++) {
         if (rt->dim_arrays[v].type_index >= 0) continue;
         printf(" DIM %s(",
             rt->dim_arrays[v].name);
         if (rt->dim_arrays[v].dims == 1) {
             printf("%d",
                 rt->dim_arrays[v].size[0]);
         } else {
             printf("%d,%d",
                 rt->dim_arrays[v].size[0],
                 rt->dim_arrays[v].size[1]);
         }
         printf(") [%d elements]\n",
             rt->dim_arrays[v].total);
         printed = 1;
     }

     // CONST table
     for (v = 0; v < rt->const_count; v++) {
         printf(" CONST %s = ",
             rt->constants[v].name);
         if (rt->constants[v].value.type ==
             VAL_STRING) {
             printf("\"%.*s\"\n",
                 rt->constants[v].value
                 .v.sval.length,
                 rt->constants[v].value
                 .v.sval.data);
         } else if (rt->constants[v].value.type
             == VAL_FLOAT) {
             printf("%g\n",
                 rt->constants[v].value
                 .v.fval);
         } else {
             printf("%ld\n",
                 rt->constants[v].value
                 .v.ival);
         }
         printed = 1;
     }

     // User-Defined Type instances
     for (v = 0; v < rt->typed_var_count; v++) {
         TypedVar *tv = &rt->typed_vars[v];
         UserTypeDef *td = &rt->user_types[tv->type_index];
         int fi;
         printf(" %s AS %s",
             tv->name, td->name);
         printf(" {");
         for (fi = 0; fi < td->field_count; fi++) {
             if (fi > 0) printf(",");
             printf(" .%s=", td->fields[fi].name);
             if (tv->fields[fi].type == VAL_STRING &&
                 tv->fields[fi].v.sval.data != NULL) {
                 printf("\"%.*s\"",
                     tv->fields[fi].v.sval.length,
                     tv->fields[fi].v.sval.data);
             } else if (tv->fields[fi].type ==
                 VAL_FLOAT) {
                 printf("%g",
                     tv->fields[fi].v.fval);
             } else {
                 printf("%ld",
                     tv->fields[fi].v.ival);
             }
         }
         printf(" }\n");
         printed = 1;
     }

     // Typed DIM arrays
     for (v = 0; v < rt->dim_count; v++) {
         if (rt->dim_arrays[v].type_index >= 0) {
             UserTypeDef *td = &rt->user_types[
                 rt->dim_arrays[v].type_index];
             int elem_count = rt->dim_arrays[v].total
                 / td->field_count;
             printf(" DIM %s(%d) AS %s"
                 " [%d elements x %d fields]\n",
                 rt->dim_arrays[v].name,
                 elem_count,
                 td->name,
                 elem_count,
                 td->field_count);
             printed = 1;
         }
     }

     if (!printed)
         printf(" (all variables clear)\n");
     printf("\n");
 }

 // ---- Section 2: User Environment Variables ----
 if (show_user) {
     int ecount;
     printf("=== USER ENVIRONMENT ===\n\n");
     ecount = platform_list_env_user();
     printf("\n %d variable(s).\n\n", ecount);
 }

 // ---- Section 3: OS Environment Variables ----
 if (show_env) {
     int ecount;
     printf("=== OS ENVIRONMENT ===\n\n");
     ecount = platform_list_env_all();
     printf("\n %d variable(s).\n\n", ecount);
 }

 // ---- Section 4: BASIC++ System Environment ----
 if (show_sys) {
     printf("=== BASIC++ ENVIRONMENT ===\n\n");
     printf(" VERSION=%s\n", BASICPP_VERSION);
     printf(" NAME=%s\n", BASICPP_NAME);
     printf(" DIALECT=%s\n",
         dialect_get_name());
     printf(" PLATFORM=%s\n",
         platform_name());
     printf(" PLATFORM_SHORT=%s\n",
         platform_short_name());
     printf(" COMPILER=%s\n",
         platform_get_info()->compiler);
     printf(" COMPILER_VER=%s\n",
         platform_get_info()->compiler_ver);
     printf(" WORDSIZE=%d\n",
         platform_word_size());
     printf(" BUILD=%s %s\n",
         __DATE__, __TIME__);
     printf(" STANDARD=ANSI C89/C90\n");
     printf(" SECURITY=%d\n",
         (int)security_get_level());
     printf(" OPTION_BASE=%d\n",
         rt->option_base);
     printf(" ANGLE=%s\n",
         rt->angle_degrees ?
             "DEGREES" : "RADIANS");
     printf(" SCREEN_MODE=%d\n",
         rt->screen_mode);
     printf(" SCREEN_WIDTH=%d\n",
         rt->screen_width > 0 ?
             rt->screen_width : 80);
     printf(" SCREEN_LINES=%d\n",
         rt->screen_lines > 0 ?
             rt->screen_lines : 25);
     printf(" TRACE=%s\n",
         rt->trace_on ? "ON" : "OFF");
     printf(" MAX_VARIABLES=%d\n",
         MAX_VARIABLES);
     printf(" MAX_NAMED_VARS=%d\n",
         MAX_NAMED_VARS);
     printf(" MAX_STACK_DEPTH=%d\n",
         MAX_STACK_DEPTH);
     printf(" MAX_PROGRAM_LINES=%d\n",
         MAX_PROGRAM_LINES);
     printf(" MAX_LINE_LENGTH=%d\n",
         MAX_LINE_LENGTH);
     printf(" MAX_DIM_ARRAYS=%d\n",
         MAX_DIM_ARRAYS);
     printf(" MAX_ARRAY_ELEMENTS=%d\n",
         MAX_ARRAY_ELEMENTS);
     printf(" MAX_STRING_LENGTH=%d\n",
         MAX_STRING_LENGTH);
     printf(" MAX_STRING_POOL=%ld\n",
         (long)MAX_STRING_POOL);
     printf(" MAX_USER_FUNCS=%d\n",
         MAX_USER_FUNCS);
     printf(" MAX_MODULES=%d\n",
         MAX_MODULES);
     printf(" MAX_BREAKPOINTS=%d\n",
         MAX_BREAKPOINTS);
     printf(" MAX_MEM_SEGMENT=%d\n",
         MAX_MEM_SEGMENT);
     printf(" GFX_WIDTH=%d\n", GFX_WIDTH);
     printf(" GFX_HEIGHT=%d\n", GFX_HEIGHT);
     printf(" GFX_MAX_COLORS=%d\n",
         GFX_MAX_COLORS);
     printf("\n");
 }

 return;
 }

 // ===== Self-test framework =====
}


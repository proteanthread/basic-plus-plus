/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser.c
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
// BASIC++ Interpreter - parser.c
// ---
//
// Statement dispatcher and line executor.
//
// PURPOSE:
//   This file contains:
//     pi_parse_statement() -- The main keyword dispatch switch.
//     parser_execute_line() -- Multi-statement line executor.
//     Helper functions shared across parser sub-modules.
//
//   Statement handler implementations are in parser_*.c sub-modules.
//   Expression parsing is in parser_expr.c.
//
// HOW IT WORKS:
//   1. parser_execute_line() tokenizes a line and calls
//      pi_parse_statement() for each colon-separated statement.
//   2. pi_parse_statement() reads the keyword token and dispatches
//      to the appropriate pi_parse_XXX() handler function.
//   3. Each handler consumes its tokens and calls back into the
//      expression evaluator (parser_expr.c) as needed.
//
// HOW TO EXTEND:
//   Adding a new statement:
//   1. Add KW_YOURNAME to the KeywordId enum (lexer.h).
//   2. Add the keyword to the keyword table (lexer.c).
//   3. Add a case KW_YOURNAME to the switch below.
//   4. Implement pi_parse_yourname() in the appropriate
//      parser_*.c sub-module.
//   5. Declare it in parser_internal.h.
//
// TROUBLESHOOTING:
//   - "WHAT?" on a valid keyword:
//     Check dialect_keyword_allowed() -- OPTION STRICT may be
//     blocking keywords not in the active dialect.
//   - Missing handler crash:
//     Every switch case must return. Never fall through.
//
// ---

#include "parser_internal.h"

// --- Forward Declarations ---
 // Internal parsing functions. These are not exposed in the header
 // because they are implementation details of the parse-and-execute
 // architecture.
// Case-insensitive string compare (for property values)
int pi_prop_eq_ci(const char *a, const char *b)
{
 while (*a && *b) {
 char ca = *a, cb = *b;
 if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
 if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
 if (ca != cb) return 0;
 a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}


// Case-insensitive string comparison (C89-safe)
int pi_str_case_equal(const char *a, const char *b)
{
 if (!a || !b) return 0;
 while (*a && *b) {
 if (toupper((unsigned char)*a) !=
 toupper((unsigned char)*b))
 return 0;
 a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}

 // set_param_by_name - Set a SUB/FUNCTION parameter.
 //
 // If the name is a single letter A-Z, sets the corresponding
 // single-letter variable (A-Z). Otherwise sets a named variable.
 // This is needed because the lexer treats single letters as
 // TOK_VARIABLE (using rt->variables[]), not named vars.
void pi_set_param_by_name(RuntimeState *rt,
 const char *name, BValue val)
{
 int len = (int)strlen(name);
 if (len == 1 && name[0] >= 'A' && name[0] <= 'Z') {
 runtime_set_var_bval(rt, name[0], val);
 } else {
 runtime_set_named_var_bval(rt, name, len, val);
 }
}

void pi_parse_statement(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (error_occurred()) return;

 // Keyword-based dispatch
 if (lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 lexer_next(lex); // consume keyword

 // Strict mode gate: reject keywords that don't
 // belong to the active dialect's bitmask.
 // In union mode this always passes.
 if (!dialect_keyword_allowed(kw)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 switch (kw) {
 case KW_PRINT:
 pi_parse_print(lex, rt, line_num);
 return;
 case KW_LET:
 pi_parse_let(lex, rt, line_num, 1);
 return;
 case KW_INPUT:
 pi_parse_input(lex, rt, line_num);
 return;
 case KW_IF:
 pi_parse_if(lex, rt, line_num);
 return;
 case KW_UNLESS:
 pi_parse_unless(lex, rt, line_num);
 return;
 case KW_GOTO:
 pi_parse_goto(lex, rt, line_num);
 return;
 case KW_GOSUB:
 pi_parse_gosub(lex, rt, line_num);
 return;
 case KW_RETURN:
 pi_parse_return(lex, rt, line_num);
 return;
 case KW_END:
 pi_parse_end(lex, rt, line_num);
 return;
 case KW_STOP:
 pi_parse_stop(lex, rt, line_num);
 return;
 case KW_REM:
 pi_parse_rem(lex, rt, line_num);
 return;
 case KW_LIST:
 pi_parse_list_cmd(lex, rt, line_num);
 return;
 case KW_RUN:
 pi_parse_run_cmd(lex, rt, line_num);
 return;
 case KW_NEW:
 pi_parse_new_cmd(lex, rt, line_num);
 return;
 case KW_SAVE:
 if (security_check(SECOP_FILE_WRITE, line_num))
 return;
 pi_parse_save_cmd(lex, rt, line_num);
 return;
 case KW_LOAD:
 if (security_check(SECOP_FILE_READ, line_num))
 return;
 pi_parse_load_cmd(lex, rt, line_num);
 return;
 case KW_FOR:
  if (!dialect_check_feature("FOR/NEXT",
   dialect_get_config()->has_for_next, line_num))
   return;
  pi_parse_for(lex, rt, line_num);
  return;
 case KW_NEXT:
 pi_parse_next(lex, rt, line_num);
 return;
 case KW_WHILE:
  if (!dialect_check_feature("WHILE/WEND",
   dialect_get_config()->has_while_wend, line_num))
   return;
  pi_parse_while(lex, rt, line_num);
  return;
 case KW_WEND:
 pi_parse_wend(lex, rt, line_num);
 return;
 case KW_DO:
  if (!dialect_check_feature("DO/LOOP",
   dialect_get_config()->has_do_loop, line_num))
   return;
  pi_parse_do(lex, rt, line_num);
  return;
 case KW_LOOP:
 pi_parse_loop(lex, rt, line_num);
 return;
 case KW_DATA:
  if (!dialect_check_feature("DATA/READ",
   dialect_get_config()->has_data_read, line_num))
   return;
  pi_parse_data(lex, rt, line_num);
  return;
  case KW_READ:
  if (!dialect_check_feature("DATA/READ",
   dialect_get_config()->has_data_read, line_num))
   return;
  pi_parse_read(lex, rt, line_num);
  return;
  case KW_RESTORE:
  if (!dialect_check_feature("DATA/READ",
   dialect_get_config()->has_data_read, line_num))
   return;
  pi_parse_restore(lex, rt, line_num);
  return;
  case KW_MERGE:
 if (!dialect_check_feature("MERGE/CHAIN",
 dialect_get_config()->has_merge_chain, line_num))
 return;
 if (security_check(SECOP_FILE_READ, line_num))
 return;
 pi_parse_merge_cmd(lex, rt, line_num);
 return;
  case KW_CHAIN:
 if (!dialect_check_feature("MERGE/CHAIN",
 dialect_get_config()->has_merge_chain, line_num))
 return;
 if (security_check(SECOP_CHAIN, line_num))
 return;
 pi_parse_chain_cmd(lex, rt, line_num);
 return;
 case KW_DIALECT:
 pi_parse_dialect_cmd(lex, rt, line_num);
 return;
 // DEF FN - user-defined functions
 case KW_DEF:
 pi_parse_def_fn(lex, rt, line_num);
 return;
 // MAT - matrix operations
 case KW_MAT:
 pi_parse_mat_cmd(lex, rt, line_num);
 return;
 // IMAGE - format string (consumed by PRINT USING)
 case KW_IMAGE:
  pi_parse_image(lex, rt, line_num);
  return;
 case KW_OPEN:
 if (security_check(SECOP_FILE_WRITE, line_num))
 return;
 pi_parse_open(lex, rt, line_num);
 return;
 case KW_CLOSE:
 pi_parse_close(lex, rt, line_num);
 return;
 // ECMA-116 Enhanced Files
 case KW_SET:
 pi_parse_set_file(lex, rt, line_num);
 return;
 case KW_ASK:
 pi_parse_ask_file(lex, rt, line_num);
 return;
 case KW_REWRITE:
 if (security_check(SECOP_FILE_WRITE,
 line_num))
 return;
 pi_parse_rewrite(lex, rt, line_num);
 return;
 case KW_DIM:
  if (!dialect_check_feature("DIM arrays",
   dialect_get_config()->has_dim_arrays, line_num))
   return;
  pi_parse_dim(lex, rt, line_num);
  return;
 case KW_COMPILE:
  if (security_check(SECOP_COMPILE, line_num))
   return;
  pi_parse_compile(lex, rt, line_num);
  return;
 case KW_BSAVE:
  if (security_check(SECOP_FILE_WRITE, line_num))
   return;
  pi_parse_bsave(lex, rt, line_num);
  return;
 case KW_BLOAD:
  if (security_check(SECOP_FILE_READ, line_num))
   return;
  pi_parse_bload(lex, rt, line_num);
  return;
 case KW_BRUN:
  exec_brun(rt);
  return;
 case KW_MODULE:
  pi_parse_module(lex, rt, line_num);
  return;
 case KW_SECURITY:
  pi_parse_security(lex, rt, line_num);
  return;
 case KW_SYSTEM:
  pi_parse_system(lex, rt, line_num);
  return;
 case KW_BREAK:
  pi_parse_break(lex, rt, line_num);
  return;
 case KW_CONT:
  pi_parse_cont(lex, rt, line_num);
  return;
 case KW_VARS:
  pi_parse_vars(lex, rt, line_num);
  return;
 case KW_ASSERT:
  pi_parse_assert(lex, rt, line_num);
  return;
 case KW_TEST:
  pi_parse_test(lex, rt, line_num);
  return;
 case KW_ENDTEST:
  pi_parse_endtest(lex, rt, line_num);
  return;
 case KW_SELFTEST:
  pi_parse_selftest(lex, rt, line_num);
  return;
 case KW_CHECK:
  pi_parse_check(lex, rt, line_num);
  return;
 case KW_VERIFY:
  pi_parse_verify(lex, rt, line_num);
  return;
 case KW_HELP:
  pi_parse_help(lex, rt, line_num);
  return;
 case KW_INFO:
  pi_parse_info(lex, rt, line_num);
  return;
 case KW_CATALOG:
  pi_parse_catalog(lex, rt, line_num);
  return;
 case KW_VDEV:
  pi_parse_vdev(lex, rt, line_num);
  return;
 case KW_VMEM:
  pi_parse_vmem(lex, rt, line_num);
  return;
 case KW_VNET:
  pi_parse_vnet(lex, rt, line_num);
  return;
 case KW_VCON:
  pi_parse_vcon(lex, rt, line_num);
  return;
 case KW_VTERM:
  pi_parse_vterm(lex, rt, line_num);
  return;
 case KW_VMACH:
  pi_parse_vmach(lex, rt, line_num);
  return;
  case KW_DEVMAP:
  pi_parse_devmap(lex, rt, line_num);
  return;
 case KW_ATOMIC:
  pi_parse_atomic(lex, rt, line_num);
  return;
 case KW_TXN:
  pi_parse_txn(lex, rt, line_num);
  return;
 case KW_COMMIT:
  pi_parse_commit(lex, rt, line_num);
  return;
 case KW_ROLLBACK:
  pi_parse_rollback(lex, rt, line_num);
  return;
 case KW_RENUM:
  pi_parse_renum(lex, rt, line_num);
  return;
 case KW_DELETE:
  pi_parse_delete(lex, rt, line_num);
  return;
 case KW_VER:
  pi_parse_ver(lex, rt, line_num);
  return;
 case KW_BYE:
 // BYE - Exit the interpreter.
 //
 // Prints a farewell message and terminates the
 // process. This is the clean way to exit BASIC++
 // back to the command prompt.
 printf("Goodbye.\n");
 exit(0);
 return; // unreachable, but satisfies compilers

 // ===== Core commands =====
 case KW_CLS:
   if (!dialect_check_feature("CLS",
    dialect_get_config()->has_cls, line_num))
    return;
   pi_parse_cls(lex, rt, line_num);
   return;
 case KW_HOME:
  pi_parse_home(lex, rt, line_num);
  return;
 case KW_CLEAR:
  pi_parse_clear(lex, rt, line_num);
  return;
 case KW_CLR:
  pi_parse_clr(lex, rt, line_num);
  return;
 case KW_TRON:
   if (!dialect_check_feature("TRON/TROFF",
    dialect_get_config()->has_tron_troff, line_num))
    return;
   pi_parse_tron(lex, rt, line_num);
   return;
  case KW_TROFF:
   if (!dialect_check_feature("TRON/TROFF",
    dialect_get_config()->has_tron_troff, line_num))
    return;
   pi_parse_troff(lex, rt, line_num);
   return;
 case KW_ON:
  pi_parse_on(lex, rt, line_num);
  return;
 case KW_SWAP:
  pi_parse_swap(lex, rt, line_num);
  return;
 case KW_RANDOMIZE:
  pi_parse_randomize(lex, rt, line_num);
  return;
 case KW_ELSE:
  pi_parse_else(lex, rt, line_num);
  return;
 case KW_ELSEIF:
  pi_parse_elseif(lex, rt, line_num);
  return;
 case KW_ENDIF:
  pi_parse_endif(lex, rt, line_num);
  return;
 case KW_FILES:
  pi_parse_files(lex, rt, line_num);
  return;
 case KW_DIR:
  pi_parse_dir(lex, rt, line_num);
  return;
 case KW_BEEP:
  pi_parse_beep(lex, rt, line_num);
  return;
 case KW_COLOR:
  pi_parse_color(lex, rt, line_num);
  return;
 case KW_INK:
  pi_parse_ink(lex, rt, line_num);
  return;
 case KW_PAPER:
  pi_parse_paper(lex, rt, line_num);
  return;
 case KW_BORDER:
  pi_parse_border(lex, rt, line_num);
  return;
 case KW_BRIGHT:
  pi_parse_bright(lex, rt, line_num);
  return;
 case KW_FLASH:
  pi_parse_flash(lex, rt, line_num);
  return;
 case KW_INVERSE:
  pi_parse_inverse(lex, rt, line_num);
  return;
 case KW_OVER:
  pi_parse_over(lex, rt, line_num);
  return;
 case KW_PAUSE:
  pi_parse_pause(lex, rt, line_num);
  return;
 case KW_DELAY:
  pi_parse_delay(lex, rt, line_num);
  return;
 case KW_REPEAT:
  pi_parse_repeat(lex, rt, line_num);
  return;
 case KW_ENDREPEAT:
  pi_parse_endrepeat(lex, rt, line_num);
  return;
 case KW_ENDFOR:
  pi_parse_endfor(lex, rt, line_num);
  return;
 case KW_DEFINE:
  pi_parse_define(lex, rt, line_num);
  return;
 case KW_ENDDEFINE:
  pi_parse_enddefine(lex, rt, line_num);
  return;
 case KW_PROCEDURE:
  pi_parse_procedure(lex, rt, line_num);
  return;
 case KW_LOCAL:
  pi_parse_local(lex, rt, line_num);
  return;
 case KW_REMAINDER:
  // REMAINDER - SELect ON default clause.
  // Only meaningful inside SELect ON parsing.
  // If encountered standalone, skip to
  // END SELect.
 error_raise(ERR_WHAT, line_num);
 return;

 case KW_AUTO:
  pi_parse_auto(lex, rt, line_num);
  return;
 case KW_SOUND:
  pi_parse_sound(lex, rt, line_num);
  return;
 case KW_PLAY:
  pi_parse_play(lex, rt, line_num);
  return;
 case KW_SCREEN:
  pi_parse_screen(lex, rt, line_num);
  return;
 case KW_CONSOLE:
  pi_parse_console(lex, rt, line_num);
  return;
 case KW_WIDTH:
  pi_parse_width(lex, rt, line_num);
  return;
 case KW_DRAW:
  pi_parse_draw(lex, rt, line_num);
  return;
 case KW_SLEEP:
  pi_parse_sleep(lex, rt, line_num);
  return;
 case KW_LOCATE:
  pi_parse_locate(lex, rt, line_num);
  return;
 case KW_LINE:
  // LINE INPUT takes priority over LINE graphics
  if (lex->current.type == TOK_KEYWORD &&
      lex->current.value.keyword == KW_INPUT) {
   lexer_next(lex); // consume INPUT
   pi_parse_line_input(lex, rt, line_num);
   return;
  }
  pi_parse_line(lex, rt, line_num);
  return;
 case KW_SELECT:
  pi_parse_select(lex, rt, line_num);
  return;
 case KW_CASE:
  pi_parse_case(lex, rt, line_num);
  return;
 case KW_EXIT:
  pi_parse_exit(lex, rt, line_num);
  return;
 case KW_CONST_KW:
  pi_parse_const_stmt(lex, rt, line_num);
  return;
 case KW_ERASE:
  if (security_check(SECOP_FILE_WRITE, line_num))
   return;
  pi_parse_erase(lex, rt, line_num);
  return;
 case KW_LPRINT:
  pi_parse_lprint(lex, rt, line_num);
  return;
 case KW_DISPLAY:
  pi_parse_display(lex, rt, line_num);
  return;
 case KW_DECLARE:
  pi_parse_declare(lex, rt, line_num);
  return;
 case KW_SUB:
  pi_parse_sub(lex, rt, line_num);
  return;
 case KW_FUNCTION:
  pi_parse_function(lex, rt, line_num);
  return;
 case KW_CALL:
  pi_parse_call(lex, rt, line_num);
  return;
 case KW_SHELL:
  if (security_check(SECOP_SYSTEM, line_num))
   return;
  pi_parse_shell(lex, rt, line_num);
  return;
 case KW_REDIM:
  pi_parse_redim(lex, rt, line_num);
  return;
 case KW_SHARED:
  pi_parse_shared(lex, rt, line_num);
  return;
 case KW_STATIC:
  pi_parse_static(lex, rt, line_num);
  return;
 case KW_RESUME:
  pi_parse_resume(lex, rt, line_num);
  return;
 case KW_OPTION:
  pi_parse_option(lex, rt, line_num);
  return;
 case KW_COMMON:
  pi_parse_common(lex, rt, line_num);
  return;
 case KW_FIELD:
  pi_parse_field(lex, rt, line_num);
  return;
 case KW_GET:
  pi_parse_get(lex, rt, line_num);
  return;
 case KW_PUT:
  pi_parse_put(lex, rt, line_num);
  return;
 case KW_IOCTL:
  pi_parse_ioctl(lex, rt, line_num);
  return;
 case KW_KEY:
  pi_parse_key(lex, rt, line_num);
  return;
 case KW_KILL:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_kill(lex, rt, line_num);
  return;
 case KW_SCRATCH:
  pi_parse_scratch(lex, rt, line_num);
  return;
 case KW_UNSAVE:
  pi_parse_unsave(lex, rt, line_num);
  return;
 case KW_COPY:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_copy(lex, rt, line_num);
  return;
 case KW_MOVE:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_move(lex, rt, line_num);
  return;
 case KW_PWD:
  pi_parse_pwd(lex, rt, line_num);
  return;
 case KW_LLIST:
  pi_parse_llist(lex, rt, line_num);
  return;
 case KW_LOCK:
  pi_parse_lock(lex, rt, line_num);
  return;
 case KW_UNLOCK:
  pi_parse_unlock(lex, rt, line_num);
  return;
 case KW_VIEW:
  pi_parse_view(lex, rt, line_num);
  return;
 case KW_WAIT:
  pi_parse_wait(lex, rt, line_num);
  return;
 case KW_WINDOW:
  pi_parse_window(lex, rt, line_num);
  return;
 case KW_MEMMAP:
  pi_parse_memmap(lex, rt, line_num);
  return;
 case KW_SYS:
  pi_parse_sys(lex, rt, line_num);
  return;
 case KW_EXEC:
  if (security_check(SECOP_SYSTEM, line_num))
   return;
  pi_parse_exec(lex, rt, line_num);
  return;
 case KW_WRITE:
  pi_parse_write(lex, rt, line_num);
  return;
 case KW_LSET:
  pi_parse_lset(lex, rt, line_num);
  return;
 case KW_RSET:
  pi_parse_rset(lex, rt, line_num);
  return;
 case KW_RESET:
  pi_parse_reset(lex, rt, line_num);
  return;
 case KW_RMDIR:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_rmdir(lex, rt, line_num);
  return;
 case KW_ALIAS:
  pi_parse_alias(lex, rt, line_num);
  return;
 case KW_SCOPE:
  pi_parse_scope(lex, rt, line_num);
  return;
 case KW_KEYWORD:
  pi_parse_keyword(lex, rt, line_num);
  return;
 case KW_OVERRIDE:
  pi_parse_override(lex, rt, line_num);
  return;
 case KW_OUT:
  pi_parse_out(lex, rt, line_num);
  return;
 case KW_COM:
  pi_parse_com(lex, rt, line_num);
  return;
 case KW_PEN:
  pi_parse_pen(lex, rt, line_num);
  return;
 case KW_STRIG:
  pi_parse_strig(lex, rt, line_num);
  return;
 case KW_TIMER:
  pi_parse_timer(lex, rt, line_num);
  return;
 case KW_NAME:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_name(lex, rt, line_num);
  return;
 case KW_RENAME:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_rename(lex, rt, line_num);
  return;
 case KW_MKDIR:
  if (security_check(SECOP_FILE_MGMT, line_num))
   return;
  pi_parse_mkdir(lex, rt, line_num);
  return;
 case KW_MID:
  pi_parse_mid(lex, rt, line_num);
  return;
 case KW_DEFINT:
  pi_parse_defint(lex, rt, line_num);
  return;
 case KW_DEFDBL:
  pi_parse_defdbl(lex, rt, line_num);
  return;
 case KW_DEFSNG:
  pi_parse_defsng(lex, rt, line_num);
  return;
 case KW_DEFSTR:
  pi_parse_defstr(lex, rt, line_num);
  return;
 case KW_EDIT:
  pi_parse_edit(lex, rt, line_num);
  return;
 case KW_ERDEV:
 case KW_EXTERR:
 // ERDEV / EXTERR
 // Device and DOS extended error. As
   // statements these are no-ops.
  lexer_skip_to_end(lex);
 return;

 case KW_ENVIRON:
  if (security_check(SECOP_SYSTEM, line_num))
   return;
  pi_parse_environ(lex, rt, line_num);
  return;
 case KW_SEEK:
  pi_parse_seek(lex, rt, line_num);
  return;
 case KW_CHDIR:
  pi_parse_chdir(lex, rt, line_num);
  return;
 case KW_ERROR:
  pi_parse_error(lex, rt, line_num);
  return;
 case KW_CAUSE:
  pi_parse_cause(lex, rt, line_num);
  return;
 case KW_WHEN:
  pi_parse_when(lex, rt, line_num);
  return;
 case KW_USE:
  pi_parse_use(lex, rt, line_num);
  return;
 case KW_RETRY:
  pi_parse_retry(lex, rt, line_num);
  return;
 case KW_CONTINUE:
  pi_parse_continue(lex, rt, line_num);
  return;
 case KW_TRAP:
  pi_parse_trap(lex, rt, line_num);
  return;
 case KW_POKE:
  pi_parse_poke(lex, rt, line_num);
  return;
 case KW_PSET:
  pi_parse_pset(lex, rt, line_num);
  return;
 case KW_CIRCLE:
  pi_parse_circle(lex, rt, line_num);
  return;
 case KW_PAINT:
  pi_parse_paint(lex, rt, line_num);
  return;
 case KW_PALETTE:
  pi_parse_palette(lex, rt, line_num);
  return;
 case KW_PCOPY:
  pi_parse_pcopy(lex, rt, line_num);
  return;
 case KW_PRESET:
  pi_parse_preset(lex, rt, line_num);
  return;
 case KW_TYPE:
  // TYPE USING or TYPE "file" = formatted file output
  if (lex->current.type == TOK_KEYWORD &&
      lex->current.value.keyword == KW_USING) {
   pi_parse_type_cmd(lex, rt, line_num);
   return;
  }
  if (lex->current.type == TOK_STRING) {
   pi_parse_type_cmd(lex, rt, line_num);
   return;
  }
  // Otherwise: user-defined TYPE
  pi_parse_type(lex, rt, line_num);
  return;
 case KW_ACCESS:
  pi_parse_access(lex, rt, line_num);
  return;
 // Enhanced debugger commands
 case KW_DEBUG:
  pi_parse_debug(lex, rt, line_num);
  return;
 case KW_DUMP:
  pi_parse_dump(lex, rt, line_num);
  return;
 case KW_BACKTRACE:
  pi_parse_backtrace(lex, rt, line_num);
  return;
 case KW_TRACE:
  pi_parse_trace(lex, rt, line_num);
  return;
 default:
  // Check if it's a dynamically registered specification statement
  if (lex->current.value.keyword >= KW_CUSTOM_START) {
   extern void pi_parse_custom_statement(Lexer *lex, RuntimeState *rt, int line_num, int kw_id);
   pi_parse_custom_statement(lex, rt, line_num, lex->current.value.keyword);
   return;
  }
  error_raise(ERR_WHAT, line_num);
  return;
 }
 }

 // If the current token is a variable and LET is optional,
 // treat as a bare assignment (e.g., "A=5").
  if (lex->current.type == TOK_VARIABLE &&
 dialect_get_config()->has_let_optional) {
 pi_parse_let(lex, rt, line_num, 0);
 return;
 }

 // Variable without LET in LET-required dialect
 if (lex->current.type == TOK_VARIABLE &&
 !dialect_get_config()->has_let_optional &&
 dialect_is_strict()) {
 printf("SORRY? LET is required in this dialect.\n");
 return;
 }

 // Named variable bare assignment (e.g., "SCORE=100")
 // when both LET-optional and extended vars are active.
 // Also check for label definitions and implicit SUB calls.
 if (lex->current.type == TOK_NAMED_VAR &&
 dialect_get_config()->has_let_optional) {
 const char *nm = lex->current.str_start;
 int nlen = lex->current.str_length;

 // Check for label definition (identifier:).
 // If the next token after the name is ':', skip
 // the label and continue with any statements
 // after the colon.
 if (nm != NULL && nlen > 0) {
 // Peek ahead: save lexer state
 Lexer saved = *lex;
 lexer_next(lex);
 if (lex->current.type == TOK_COLON) {
 lexer_next(lex); // consume colon
 // Execute rest of line if any
 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 pi_parse_statement(lex, rt, line_num);
 }
 return;
 }
 // Not a label - restore and continue
 *lex = saved;
 }

 // Check for implicit SUB call (SubName args).
 // If the name matches a defined SUB, treat as CALL.
 if (nm != NULL && nlen > 0) {
 SubDef *sd = runtime_find_sub(rt, nm, nlen);
 if (sd != NULL && !sd->is_function) {
 StackFrame frame;
 int i;
 lexer_next(lex); // consume name

 // Push FRAME_SUB
 frame.type = FRAME_SUB;
 frame.data.sub_call.return_index =
 rt->current_index + 1;
 frame.data.sub_call.sub_index =
 (int)(sd - rt->subs);
 for (i = 0; i < MAX_VARIABLES; i++) {
 frame.data.sub_call.saved_vars[i] =
 rt->variables[i];
 }
 for (i = 0; i < MAX_STRING_VARS; i++) {
 frame.data.sub_call.saved_strvars[i] =
 rt->string_vars[i];
 }
 if (runtime_push(rt, &frame) != 0)
 return;

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
  rt->current_index + 1);
 }

 // Parse args (with or without parens)
 if (lex->current.type == TOK_LPAREN) {
 lexer_next(lex);
 for (i = 0; i < sd->param_count; i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 pi_set_param_by_name(rt,
 sd->params[i], av);
 }
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);
 } else {
 for (i = 0; i < sd->param_count; i++) {
 BValue av;
 if (i > 0) {
 if (lex->current.type !=
 TOK_COMMA) break;
 lexer_next(lex);
 }
 av = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 pi_set_param_by_name(rt,
 sd->params[i], av);
 }
 }

 rt->in_sub_index =
 (int)(sd - rt->subs);
 rt->next_index = sd->body_index;
 lexer_skip_to_end(lex);
 return;
 }
 }

 // Regular assignment
 pi_parse_let(lex, rt, line_num, 0);
 return;
 }

 // If the current token is @, treat as array assignment
 // (e.g., "@(0)=5") when LET is optional.
 if (lex->current.type == TOK_AT &&
 dialect_get_config()->has_let_optional) {
 pi_parse_let(lex, rt, line_num, 0);
 return;
 }

 // String variable bare assignment (e.g., "A$=\"HELLO\"")
 if (lex->current.type == TOK_STRING_VAR &&
 dialect_get_config()->has_let_optional) {
 pi_parse_let(lex, rt, line_num, 0);
 return;
 }

 // If we reach here, it's a syntax error
 if (lex->current.type != TOK_EOF && lex->current.type != TOK_CR) {
 error_raise(ERR_WHAT, line_num);
 }
}

// --- Line Execution ---
 // parser_execute_line - Parse and execute a complete BASIC line.
 //
 // Handles multiple statements separated by the dialect's separator
 // character (';' for PATB). Stops on:
 // - End of line (TOK_EOF)
 // - Error
 // - Flow control that changes execution (GOTO, GOSUB, etc.)
 // set rt->next_index, which signals the caller.
 //
 // The lexer should be initialized on the line text, positioned
 // AFTER the line number (if any). The caller is responsible for
 // skipping the line number.
 //
 // SUPER BASIC postfix modifiers:
 // After each statement, checks for postfix IF/UNLESS/FOR.
 // These are JOSS-style modifiers that qualify the preceding
 // statement:
 //   PRINT X IF X>0       - conditional execution
 //   PRINT X UNLESS X<0   - negated conditional
 //   PRINT I FOR I=1 TO 5 - inline loop

 // postfix_scan - Scan for a postfix IF/UNLESS/FOR modifier.
 //
 // Scans forward from the current lexer position through the
 // source text, looking for IF/UNLESS/FOR at paren depth 0.
 // Does not modify the lexer state (saves and restores).
 //
 // Returns:
 //   KW_IF     - found postfix IF
 //   KW_UNLESS - found postfix UNLESS
 //   KW_FOR    - found postfix FOR
 //   KW_COUNT  - no postfix modifier found (sentinel)
 //
 // Sets *mod_pos to the lexer position of the modifier keyword.
static KeywordId postfix_scan(Lexer *lex, int *mod_pos)
{
 // Save lexer state
 int save_pos = lex->pos;
 Token save_tok = lex->current;
 int depth = 0;
 KeywordId result = KW_COUNT;

  // Save error state. The pre-scan calls lexer_next()
  // which may raise ERR_WHAT on unrecognized characters
  // (e.g. '.' in IMAGE format strings). These spurious
  // errors must not prevent statement execution.
  // Suppress error output during the scan to avoid
  // printing stray error messages.
 int had_error = error_occurred();
 int was_suppressed = error_get_suppress();
 error_set_suppress(1);

 {
 // Track previous keyword to detect compound
 // keywords like END IF, END FOR, END SUB.
 // When prev_kw is KW_END, the next IF/FOR
 // is part of the compound keyword, not a
 // postfix modifier.
 KeywordId prev_kw = KW_COUNT;

 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {

 if (lex->current.type == TOK_LPAREN) {
 depth++;
 } else if (lex->current.type == TOK_RPAREN) {
 if (depth > 0) depth--;
 } else if (depth == 0 &&
 lex->current.type == TOK_KEYWORD) {
 KeywordId kw = lex->current.value.keyword;
 if ((kw == KW_IF || kw == KW_UNLESS ||
 kw == KW_FOR) && prev_kw != KW_END) {
  // Don't match prefix IF/UNLESS/FOR -
  // they appear at the START of a statement.
  // A postfix modifier always has some
  // tokens before it.
 if (lex->pos != save_pos ||
 lex->current.type != save_tok.type ||
 (save_tok.type == TOK_KEYWORD &&
  save_tok.value.keyword != kw)) {
 *mod_pos = lex->pos;
 result = kw;
 // Found - stop scanning
 break;
 }
 }
 // Stop scanning at statement separator
 if (kw == KW_THEN) break;
 prev_kw = kw;
 } else {
 prev_kw = KW_COUNT; // reset on non-keyword
 }

 // Stop at statement separators
 if (lex->current.type == TOK_COLON ||
 lex->current.type == TOK_SEMICOLON) {
 break;
 }

 lexer_next(lex);
 }
 } // end prev_kw scope

 // Restore lexer state
 lex->pos = save_pos;
 lex->current = save_tok;

  // Restore error state: always restore suppress flag.
  // If no error existed before the scan, clear any
  // error raised during scanning.
 error_set_suppress(was_suppressed);
 if (!had_error && error_occurred()) {
 error_clear();
 }

 return result;
}

 // skip_statement_body - Advance the lexer past a statement body
 // up to (but not consuming) a postfix modifier keyword at
 // the given lexer position.
 //
 // After this call, the lexer's current token is the modifier
 // keyword (IF/UNLESS/FOR).
void pi_skip_to_pos(Lexer *lex, int target_pos)
{
 while (lex->pos < target_pos &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 lexer_next(lex);
 }
}

void parser_execute_line(Lexer *lex, RuntimeState *rt, int line_num)
{
 char sep = dialect_get_separator();

 while (!error_occurred() && lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR) {
 int old_next = rt->next_index;
 int mod_pos = 0;
 KeywordId modifier;

  // Pre-scan for JOSS-style postfix modifiers.
  // This must be done BEFORE executing the statement
  // to allow conditional execution.
  //
  // SKIP for REM and DATA: these consume the rest
  // of the line as raw text. Scanning through them
  // would incorrectly match keywords embedded in
  // comment text or data literals (e.g., "for" in
  // "REM look for bugs" triggers false KW_FOR).
 if (lex->current.type == TOK_KEYWORD &&
  (lex->current.value.keyword == KW_REM ||
   lex->current.value.keyword == KW_DATA)) {
  modifier = KW_COUNT; // sentinel: no modifier
 } else {
  modifier = postfix_scan(lex, &mod_pos);
 }

 if (modifier == KW_IF || modifier == KW_UNLESS) {
  // Postfix IF/UNLESS:
  // <statement> IF <condition>
  // <statement> UNLESS <condition>
  //
  // Save statement start position, skip to the
  // modifier, evaluate condition, then either
  // re-parse and execute the statement or skip.
 int stmt_start_pos = lex->pos;
 Token stmt_start_tok = lex->current;
 BValue cond_val;
 int condition;

 // Skip to the modifier keyword
 pi_skip_to_pos(lex, mod_pos);
 // lex->current should now be IF/UNLESS
 lexer_next(lex); // consume IF/UNLESS

 // Evaluate condition
 cond_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (modifier == KW_IF) {
 condition = (bval_to_int(&cond_val) != 0);
 } else {
 // UNLESS = negated IF
 condition = (bval_to_int(&cond_val) == 0);
 }

 if (condition) {
  // Condition is true: re-parse and execute
  // the statement body. Save current position
  // (past the condition), restore to statement
  // start, execute, then skip to saved end.
 int end_pos = lex->pos;
 Token end_tok = lex->current;

 lex->pos = stmt_start_pos;
 lex->current = stmt_start_tok;

 pi_parse_statement(lex, rt, line_num);
 if (error_occurred()) return;

 // Skip past the postfix modifier we
  // already evaluated 
 lex->pos = end_pos;
 lex->current = end_tok;
 }
 // else: condition false, skip (already past)

 } else if (modifier == KW_FOR) {
  // Postfix FOR:
  // <statement> FOR <var>=<start> TO <limit>
  //              [STEP <n>] [BY <n>]
  //
  // Execute the statement body repeatedly for
  // each iteration of the loop variable.
 int stmt_start_pos = lex->pos;
 Token stmt_start_tok = lex->current;
 char loop_var;
 long start_val, limit_val, step_val;
 long loop_i;
 int for_end_pos;
 Token for_end_tok;

 // Skip to the FOR keyword
 pi_skip_to_pos(lex, mod_pos);
 lexer_next(lex); // consume FOR

 // Parse: <var> = <start> TO <limit> [STEP n]
 if (lex->current.type != TOK_VARIABLE) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 loop_var = lex->current.value.var_name;
 lexer_next(lex); // consume variable

 if (!lexer_expect(lex, TOK_EQUALS)) return;

 start_val = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (!lexer_match_keyword(lex, KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume TO

 limit_val = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 step_val = 1;
 if (lex->current.type == TOK_KEYWORD &&
 (lex->current.value.keyword == KW_STEP ||
  lex->current.value.keyword == KW_BY)) {
 lexer_next(lex); // consume STEP/BY
 step_val = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 // Save position after FOR clause
 for_end_pos = lex->pos;
 for_end_tok = lex->current;

 // Execute the loop
 if (step_val == 0) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 for (loop_i = start_val;
 (step_val > 0) ? (loop_i <= limit_val)
 : (loop_i >= limit_val);
 loop_i += step_val) {

 // Set loop variable
 runtime_set_var(
 rt, loop_var, loop_i);

 // Re-parse and execute statement body
 lex->pos = stmt_start_pos;
 lex->current = stmt_start_tok;

 pi_parse_statement(lex, rt, line_num);
 if (error_occurred()) return;

 // Check for flow control
 if (rt->next_index != old_next) return;
 }

 // Restore to after FOR clause
 lex->pos = for_end_pos;
 lex->current = for_end_tok;

 } else {
  // No postfix modifier: execute statement
  // normally.
 pi_parse_statement(lex, rt, line_num);
 }

 // If flow control changed next_index, stop processing
  // this line - the caller will jump to the new target 
 if (rt->next_index != old_next) {
 return;
 }

 // If program was stopped, don't continue
 if (!rt->running && rt->stopped) {
 return;
 }

 // Check for statement separator (dialect-specific)
 if (lex->current.type == TOK_SEMICOLON && sep == ';') {
 lexer_next(lex); // consume separator
 continue;
 }
 if (lex->current.type == TOK_COLON && sep == ':') {
 lexer_next(lex); // consume separator
 continue;
 }

 // Any other token at this point is the end of statements
 break;
 }
}


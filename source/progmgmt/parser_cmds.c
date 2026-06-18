 // ---
 // BASIC++ Interpreter - parser_cmds.c
 // ---
 //
 // System command handlers: LIST, RUN, NEW, SAVE, LOAD,
 // MERGE, CHAIN, DIALECT.
 //
 // Handles program management commands that operate on the
 // stored program rather than executing BASIC logic.
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

void pi_parse_list_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 // LIST "filename" - Display file from disk
 // without loading it into memory.
 // Auto-appends .BAS if no extension.
 if (lex->current.type == TOK_STRING) {
 char filename[MAX_LINE_LENGTH + 1];
 int flen = lex->current.str_length;
 char linebuf[MAX_LINE_LENGTH + 2];
 FILE *fp;
 int ll;

 if (flen >= MAX_LINE_LENGTH) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 memcpy(filename, lex->current.str_start,
  (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen,
  MAX_LINE_LENGTH);

 fp = fopen(filename, "r");
 if (fp == NULL) {
  printf("File not found: %s\n",
  filename);
  return;
 }

 printf("\n--- %s ---\n", filename);
 while (fgets(linebuf, sizeof(linebuf),
  fp) != NULL) {
  // Strip trailing newline
  ll = (int)strlen(linebuf);
  while (ll > 0 &&
  (linebuf[ll-1] == '\n' ||
  linebuf[ll-1] == '\r')) {
  linebuf[--ll] = '\0';
  }
  printf("%s\n", linebuf);
 }
 printf("--- end ---\n\n");
 fclose(fp);
 return;
 }

 // No arguments: list everything
 if (lex->current.type != TOK_NUMBER &&
 lex->current.type != TOK_MINUS) {
 program_list(&rt->memory->program, 0, 0);
 return;
 }

 // Parse comma-separated segments.
 // Each segment is one of:
 // n -> single line (from=n, to=n)
 // n- -> from n to end (from=n, to=0)
 // n-m -> range (from=n, to=m)
 // -n -> from start to n (from=0, to=n)
 for (;;) {
 int from = 0;
 int to = 0;

 // Case: -n (start to line n)
 if (lex->current.type == TOK_MINUS) {
 lexer_next(lex); // consume '-'
 if (lex->current.type == TOK_NUMBER) {
 to = (int)lex->current.value.num_value;
 lexer_next(lex);
 }
 program_list(&rt->memory->program, from, to);
 }
 // Case: starts with a number
 else if (lex->current.type == TOK_NUMBER) {
 from = (int)lex->current.value.num_value;
 lexer_next(lex);

 if (lex->current.type == TOK_MINUS) {
 // n- or n-m
 lexer_next(lex); // consume '-'
 if (lex->current.type == TOK_NUMBER) {
 to = (int)lex->current.value.num_value;
 lexer_next(lex);
 }
 // else to=0 means "to end"
 program_list(&rt->memory->program, from, to);
 } else {
 // Single line: n
 to = from;
 program_list(&rt->memory->program, from, to);
 }
 } else {
 // Unexpected token, stop
 break;
 }

 // Check for comma separator to continue
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume ','
 } else {
 break;
 }
 }
}

int pi_ensure_bas_ext(char *fname, int len, int maxlen);

 // parse_run_cmd - Parse RUN command.
 //
 // RUN "filename" - Load file then execute.
 // RUN            - Execute the program in memory.
 //
 // If filename has no extension, ".BAS" is appended.
void pi_parse_run_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 if (lex->current.type == TOK_STRING) {
 // RUN "filename" - load then execute
 char filename[MAX_LINE_LENGTH + 1];
 int flen = lex->current.str_length;

 if (security_check(SECOP_FILE_READ, line_num))
  return;

 if (flen >= MAX_LINE_LENGTH) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 memcpy(filename, lex->current.str_start,
  (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen,
  MAX_LINE_LENGTH);

 // Clear and load
 program_clear(&rt->memory->program);
 runtime_reset(rt);
 if (fileio_load(&rt->memory->program,
  filename) != 0)
  return; // load failed
 }

 if (rt->memory->program.count == 0) {
 // No program to run
 return;
 }

 // ECMA-55: END must be the last line of the program.
 // Check in strict mode only. Warn but still execute.
 if (dialect_is_strict()) {
 ProgramStore *ps = &rt->memory->program;
 int last = ps->count - 1;
 if (last >= 0) {
  Lexer chk;
  lexer_init(&chk, ps->lines[last].text);
  if (chk.current.type == TOK_NUMBER)
  lexer_next(&chk);
  if (!(chk.current.type == TOK_KEYWORD &&
  chk.current.value.keyword == KW_END)) {
  printf("Warning: END is not the "
   "last statement (ECMA-55)\n");
  }
 }
 }

 exec_run(rt);
}

 // parse_new_cmd - Parse NEW command.
 //
 // Clears the program store and resets runtime state.
void pi_parse_new_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;

 program_clear(&rt->memory->program);
 runtime_reset(rt);
 lexer_clear_scope(ASCOPE_PROGRAM);
}

 // parse_save_cmd - Parse SAVE command.
 //
 // Syntax: SAVE "filename"
void pi_parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num);

 // parse_load_cmd - Parse LOAD command.
 //
 // Syntax: LOAD "filename"
void pi_parse_load_cmd(Lexer *lex, RuntimeState *rt, int line_num);

// --- Loop Statement Handlers ---
 // All loops use the unified stack frame system. Each loop type
 // pushes a typed frame when entering the loop and pops it when
 // exiting. The type-checking in runtime_pop() prevents mismatched
 // NEXT/WEND/LOOP from corrupting execution.
 //
 // Loop execution model:
 // FOR: push frame, body executes, NEXT checks/increments/jumps
 // WHILE: evaluate condition, push frame if true, skip to WEND if false
 // DO: push frame (optionally evaluate pre-condition), LOOP checks

 // parse_for - Parse and execute FOR statement.
 //
 // Syntax:
 // FOR var = start TO limit [STEP step]
 //
 // Behavior:
 // 1. Evaluate start, limit, and optional step (default 1).
 // 2. Set the variable to start.
 // 3. Check initial condition: if step > 0, var must be <= limit;
 // if step < 0, var must be >= limit. If false, skip to NEXT.
 // 4. Push a FRAME_FOR with variable, limit, step, and body index.
 // 5. Continue executing the loop body.

void pi_parse_merge_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];
 int flen;

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 flen = lex->current.str_length;
 if (flen >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen, MAX_LINE_LENGTH);

 fileio_merge(&rt->memory->program, filename);

 // MERGE may shift line positions in the program store.
 // Invalidate SubDef body_index entries so that SUB/FUNCTION
 // definitions are re-scanned on next encounter. Free static
 // storage since SUB bodies may have changed.
 //
 // This matches GW-BASIC/QBasic behavior: MERGE during
 // execution is rare but valid; SUB definitions are
 // re-registered when encountered at the new line positions.
 {
  int si;
  for (si = 0; si < rt->sub_count; si++) {
   SubDef *sd = &rt->subs[si];
   if (sd->has_static_data) {
    free(sd->static_vars);
    free(sd->static_strvars);
    free(sd->static_named);
    sd->static_vars = NULL;
    sd->static_strvars = NULL;
    sd->static_named = NULL;
    sd->has_static_data = 0;
   }
  }
  rt->sub_count = 0;
 }
}

 // parse_chain_cmd - Parse CHAIN "filename"
 //
 // Loads the file and triggers execution (like RUN after LOAD).
void pi_parse_chain_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];
 int flen;

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 flen = lex->current.str_length;
 if (flen >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen, MAX_LINE_LENGTH);

 // CHAIN semantics (GW-BASIC/QBasic compatible):
 // - Variables are preserved (COMMON variables
 //   survive across CHAIN in real BASIC; we keep
 //   ALL variables since they share the same
 //   RuntimeState).
 // - Program code is replaced.
 // - Execution state must be fully reset:
 //   scope stack, SUB/FUNCTION table, call stack,
 //   execution pointers, DATA position.
 //
 // Milestone 9 cleanup: free SubDef static storage,
 // unwind scope stack, reset fn_return_value. Without
 // this, stale scope frames and orphaned heap
 // allocations would leak/corrupt execution.

 // Unwind scope stack (Milestone 9)
 scope_stack_free(&rt->scope_stack);
 scope_stack_init(&rt->scope_stack);

 // Free SubDef static storage and clear SUB table
 {
  int si;
  for (si = 0; si < rt->sub_count; si++) {
   SubDef *sd = &rt->subs[si];
   if (sd->has_static_data) {
    free(sd->static_vars);
    free(sd->static_strvars);
    free(sd->static_named);
    sd->static_vars = NULL;
    sd->static_strvars = NULL;
    sd->static_named = NULL;
    sd->has_static_data = 0;
   }
  }
 }
 rt->sub_count = 0;
 rt->fn_return_value = bval_int(0);
 rt->in_sub_index = -1;

 // Reset execution state (but NOT variables)
 rt->stack_top = 0; // clear FOR/WHILE/GOSUB stack
 rt->current_index = 0;
 rt->next_index = -1;
 rt->data_ptr = 0; // reset DATA pointer
 rt->label_count = 0; // labels need re-scan

 if (fileio_chain(&rt->memory->program, filename) == 0) {
  // Trigger execution preserving variables
  exec_chain_run(rt);
 }
}

 // parse_dialect_cmd - Parse DIALECT command.
 //
 // Syntax:
 // DIALECT (list available dialects)
 // DIALECT "name" (switch by name or short code)
 // DIALECT number (switch by ID)
 //
 // Calls dialect_apply() after switching to reconfigure
 // the function registry and runtime for the new dialect.
void pi_parse_dialect_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)rt;

 if (lex->current.type == TOK_EOF || lex->current.type == TOK_CR) {
 // No argument - list all dialects
 dialect_list_all();
 return;
 }

 if (lex->current.type == TOK_NUMBER) {
 // Switch by numeric ID
 int id = (int)lex->current.value.num_value;
 lexer_next(lex);
 if (id < 0 || id >= DIALECT_COUNT) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)id);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(), dialect_get_short_name());
 return;
 }

 if (lex->current.type == TOK_STRING) {
 // Switch by name or short code (substring match)
 char name[MAX_LINE_LENGTH + 1];
 int found;

 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(name, lex->current.str_start,
 (size_t)lex->current.str_length);
 name[lex->current.str_length] = '\0';
 lexer_next(lex);

 found = dialect_find_by_name(name);
 if (found < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)found);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(), dialect_get_short_name());
 return;
 }
 // Handle DIALECT LIST (LIST is a keyword, not a string).
 // Also handle named-var form in extended-var dialects.
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_LIST) {
 lexer_next(lex);
 dialect_list_all();
 return;
 }
 if (lex->current.type == TOK_NAMED_VAR) {
 // Try as dialect name (e.g. DIALECT GWBS)
 char name[MAX_LINE_LENGTH + 1];
 int found;
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 memcpy(name, lex->current.str_start,
 (size_t)lex->current.str_length);
 name[lex->current.str_length] = '\0';
 lexer_next(lex);
 found = dialect_find_by_name(name);
 if (found < 0) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 dialect_init((DialectId)found);
 dialect_apply();
 printf("Dialect: %s [%s]\n",
 dialect_get_name(),
 dialect_get_short_name());
 return;
 }

 error_raise(ERR_WHAT, line_num);
}

// --- DEF FN - User-Defined Functions ---
 //
 // SYNTAX:
 // DEF FNA(X) = X*X+1
 // DEF FNA(X,Y) = X*Y+1
 //
 // SEMANTICS:
 // 1. Parses function name (single letter after FN prefix)
 // 2. Parses parameter list (single-letter variables in parens)
 // 3. Expects '=' followed by an expression
 // 4. Stores the ENTIRE expression text for later evaluation
 //
 // INVOCATION:
 // FNA(5) -> saves X, sets X=5, evaluates body, restores X
 //
 // WHY TEXT-BASED:
 // Classic BASIC stored DEF FN bodies as text and re-parsed
 // them at each invocation. This approach:
 // - Matches historical behavior exactly
 // - Requires no AST/bytecode storage
 // - Supports all expression features naturally
 // - Is memory-efficient (just a string per function)

 // parse_def_fn - Parse a DEF FN statement.
 //
 // DEF FN<name>(<params>) = <expression>
 //
 // Stores the function definition in the runtime's user_funcs table.

// --- SAVE/LOAD Command Implementations ---
 // These are here rather than in fileio.c because they need access
 // to the parser's token stream for the filename argument. The
 // actual file operations delegate to fileio.c functions.

 // parse_save_cmd - SAVE "filename"
 // ensure_bas_ext - Append ".BAS" if filename has no extension.
 //
 // Scans for a '.' after the last path separator. If none is
 // found, appends ".BAS". Buffer must have room for 4 extra
 // bytes. Returns the new length.
int pi_ensure_bas_ext(char *fname, int len, int maxlen)
{
 int i;
 int has_dot = 0;

 // Scan backward from end for '.' or path separator
 for (i = len - 1; i >= 0; i--) {
 if (fname[i] == '.') { has_dot = 1; break; }
 if (fname[i] == '/' || fname[i] == '\\') break;
 }

 if (!has_dot && len + 4 < maxlen) {
 fname[len]   = '.';
 fname[len+1] = 'B';
 fname[len+2] = 'A';
 fname[len+3] = 'S';
 fname[len+4] = '\0';
 len += 4;
 }
 return len;
}

void pi_parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type == TOK_STRING) {
 // SAVE "filename"
 int flen = lex->current.str_length;
 if (flen >= MAX_LINE_LENGTH) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 memcpy(filename, lex->current.str_start,
  (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen,
  MAX_LINE_LENGTH);
 } else {
  // Bare SAVE - generate YYYYMMDD_HHMMSS.BAS
  // from current date/time.
 time_t t;
 struct tm *tm;
 t = time(NULL);
 tm = localtime(&t);
 sprintf(filename,
  "%04d%02d%02d_%02d%02d%02d.BAS",
  tm->tm_year + 1900, tm->tm_mon + 1,
  tm->tm_mday, tm->tm_hour,
  tm->tm_min, tm->tm_sec);
 printf("Saving to %s\n", filename);
 }

 fileio_save(&rt->memory->program, filename);

 // Track for UNSAVE
 {
 int sl = (int)strlen(filename);
 if (sl > 259) sl = 259;
 memcpy(rt->last_save_file, filename,
  (size_t)sl);
 rt->last_save_file[sl] = '\0';
 }
}

 // parse_load_cmd - LOAD "filename"
 //
 // If no filename given (bare LOAD), auto-discover a startup
 // file in this priority order:
 //   MENU.BAS, HELLO.BAS, RUNME.BAS, MAIN.BAS, AUTORUN.BAS
 //
 // If filename has no extension, ".BAS" is appended.
void pi_parse_load_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 char filename[MAX_LINE_LENGTH + 1];

 if (lex->current.type == TOK_STRING) {
 // LOAD "filename"
 int flen = lex->current.str_length;
 if (flen >= MAX_LINE_LENGTH) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 memcpy(filename, lex->current.str_start,
  (size_t)flen);
 filename[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(filename, flen,
  MAX_LINE_LENGTH);
 } else {
  // Bare LOAD - try common startup filenames.
 static const char *try_names[] = {
  "MENU.BAS", "HELLO.BAS", "RUNME.BAS",
  "MAIN.BAS", "AUTORUN.BAS", NULL
 };
 int found = 0;
 int ti;
 for (ti = 0; try_names[ti] != NULL; ti++) {
  FILE *fp = fopen(try_names[ti], "r");
  if (fp != NULL) {
  fclose(fp);
  strcpy(filename, try_names[ti]);
  found = 1;
  break;
  }
 }
 if (!found) {
  printf("No program file found.\n");
  return;
 }
 printf("Loading %s\n", filename);
 }

 // LOAD clears the current program first
 program_clear(&rt->memory->program);
 runtime_reset(rt);

 fileio_load(&rt->memory->program, filename);
}



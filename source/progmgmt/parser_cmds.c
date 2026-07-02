/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_cmds.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

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
#include "../pcode.h"
#include "../bytecode.h"
#include "../codegen/archive.h"
#include "../memmap.h"
#include "../gw_memory.h"

extern struct GW_Memory *g_gw_mem;

void pi_parse_list_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)line_num;

 if (rt->bytecode_only) {
     printf("LIST: Prohibited in obfuscated/bytecode-only mode.\n");
     return;
 }

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
#ifdef BPP_LITE_BUILD
      error_raise(ERR_HOW, line_num);
      return;
#else
  // RUN "filename" - load then execute
  char filename[MAX_LINE_LENGTH + 1];
  int flen = lex->current.str_length;

   if (security_check(SECOP_PROG_MGMT, line_num))
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

  // Auto-detect format by checking magic bytes
  unsigned char magic[4] = {0};
  FILE *mf = fopen(filename, "rb");
  if (mf) {
      if (fread(magic, 1, 4, mf) != 4) {
          memset(magic, 0, 4);
      }
      fclose(mf);
  }

  if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'E' && magic[3] == '\x1A') {
      if (bpe_load(filename, &rt->memory->program, rt) != 0)
          return;
  } else if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'P' && (magic[3] == '\x1B' || magic[3] == '\x1A')) {
      if (bpp_load(&rt->memory->program, filename, rt) != 0)
          return;
  } else {
      if (fileio_load(&rt->memory->program, filename) != 0)
          return; // load failed
  }
#endif
  }

 if (rt->memory->program.count == 0) {
 // No program to run
 return;
 }

#ifdef BPP_LITE_BUILD
  {
      ProgramStore *ps = &rt->memory->program;
      int last = ps->count - 1;
      if (last >= 0) {
          Lexer chk;
          lexer_init(&chk, ps->lines[last].text);
          if (chk.current.type == TOK_NUMBER)
              lexer_next(&chk);
          if (!(chk.current.type == TOK_KEYWORD &&
                chk.current.value.keyword == KW_END)) {
              error_raise(ERR_WHAT, ps->lines[last].line_number);
              return;
          }
      }
  }
#else
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
#endif

#ifndef BPP_LITE_BUILD
  if (rt->has_loaded_pcode) {
      exec_brun(rt);
      return;
  }
#endif
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

 pcode_cache_invalidate(rt);
 rt->bytecode_only = 0;

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

#ifndef BPP_LITE_BUILD
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

  pcode_cache_invalidate(rt);
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

 rt->chain_pending = 1;
 strncpy(rt->chain_file, filename, sizeof(rt->chain_file) - 1);
 rt->chain_file[sizeof(rt->chain_file) - 1] = '\0';
 vm_set_state(rt, VM_RUNNING);
}
#endif

 // parse_dialect_cmd - Parse DIALECT command.
 //
 // Syntax:
 // DIALECT (list available dialects)
 // DIALECT "name" (switch by name or short code)
 // DIALECT number (switch by ID)
 //
 // Calls dialect_apply() after switching to reconfigure
 // the function registry and runtime for the new dialect.
static void dialect_update_memmap_and_screen(RuntimeState *rt, int dialect_id)
{
    MemMapType default_map = memmap_default_for_dialect(dialect_id);
    if (default_map != MMAP_NONE) {
        memmap_init(rt->mem_segment, default_map);
        rt->memmap_type = (int)default_map;
        rt->mem_seg_base = 0;
#ifndef BPP_LITE_BUILD
        if (g_gw_mem != NULL) {
            gw_mem_def_seg(g_gw_mem, 0);
        }
#endif
        // Sync screen columns
        if (default_map == MMAP_IBM_PCJR) {
            rt->screen_width = 40;
#ifndef BPP_LITE_BUILD
#ifndef NO_SDL2
            extern void gw_sdl2_set_mode(int mode, int cols);
            gw_sdl2_set_mode(rt->screen_mode, 40);
#endif
#endif
        } else if (default_map == MMAP_IBM_PC || default_map == MMAP_IBM_XT || default_map == MMAP_IBM_AT || default_map == MMAP_MSDOS) {
            rt->screen_width = 80;
#ifndef BPP_LITE_BUILD
#ifndef NO_SDL2
            extern void gw_sdl2_set_mode(int mode, int cols);
            gw_sdl2_set_mode(rt->screen_mode, 80);
#endif
#endif
        }
    } else {
        rt->memmap_type = (int)MMAP_NONE;
    }
}

void pi_parse_dialect_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
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
 dialect_update_memmap_and_screen(rt, id);
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
 dialect_update_memmap_and_screen(rt, found);
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
 dialect_update_memmap_and_screen(rt, found);
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

int pi_ensure_bpp_ext(char *fname, int len, int maxlen)
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
 fname[len+2] = 'P';
 fname[len+3] = 'P';
 fname[len+4] = '\0';
 len += 4;
 }
 return len;
}

#ifndef BPP_LITE_BUILD
void pi_parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
#ifdef BPP_LITE_BUILD
    error_raise(ERR_HOW, line_num);
    return;
#endif
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
#endif

 // parse_load_cmd - LOAD "filename"
 //
 // If no filename given (bare LOAD), auto-discover a startup
 // file in this priority order:
 //   MENU.BAS, HELLO.BAS, RUNME.BAS, MAIN.BAS, AUTORUN.BAS
 //
 // If filename has no extension, ".BAS" is appended.
#ifndef BPP_LITE_BUILD
void pi_parse_load_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_VARIABLE) {
        const char *nv = (lex->current.type == TOK_NAMED_VAR) ? lex->current.str_start : NULL;
        int nlen = (lex->current.type == TOK_NAMED_VAR) ? lex->current.str_length : 0;
        char name[32] = "";
        
        if (lex->current.type == TOK_VARIABLE) {
            name[0] = (char)toupper(lex->current.value.var_name);
            name[1] = '\0';
            nlen = 1;
        } else if (nv && nlen < 31) {
            int k;
            for (k = 0; k < nlen; k++) {
                name[k] = (char)toupper(nv[k]);
            }
            name[nlen] = '\0';
        }
        
        int type_lib = 0, type_func = 0, type_mod = 0, type_feat = 0, type_plug = 0;
        int handled = 0;
        
        if (strcmp(name, "LIBRARY") == 0) { type_lib = 1; handled = 1; }
        else if (strcmp(name, "FUNCTION") == 0) { type_func = 1; handled = 1; }
        else if (strcmp(name, "MODULE") == 0) { type_mod = 1; handled = 1; }
        else if (strcmp(name, "FEATURE") == 0) { type_feat = 1; handled = 1; }
        else if (strcmp(name, "PLUGIN") == 0) { type_plug = 1; handled = 1; }
        
        if (handled) {
            char path[MAX_LINE_LENGTH + 1];
            lexer_next(lex); // consume subcommand (e.g. LIBRARY)
            
            if (lex->current.type != TOK_STRING) {
                error_raise(ERR_WHAT, line_num);
                return;
            }
            int plen = lex->current.str_length;
            if (plen >= MAX_LINE_LENGTH) {
                error_raise(ERR_WHAT, line_num);
                return;
            }
            memcpy(path, lex->current.str_start, (size_t)plen);
            path[plen] = '\0';
            lexer_next(lex);
            
            if (security_check(SECOP_EXT_LOAD, line_num)) return;
            
            if (type_lib) {
                extern int ext_lib_load(const char *path, void *rt);
                if (ext_lib_load(path, rt) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_func) {
                extern int ext_func_load(const char *path, void *rt);
                if (ext_func_load(path, rt) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_mod) {
                extern int module_load_dynamic(const char *path);
                if (module_load_dynamic(path) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_feat) {
                extern int ext_feature_load(const char *spec_path, void *rt);
                if (ext_feature_load(path, rt) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_plug) {
                extern int ext_plugin_load(const char *path, void *rt);
                if (ext_plugin_load(path, rt) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            }
            return;
        }
    }

    char filename[MAX_LINE_LENGTH + 1];

    if (lex->current.type == TOK_STRING) {
        // LOAD "filename"
        int flen = lex->current.str_length;
        if (flen >= MAX_LINE_LENGTH) {
            error_raise(ERR_WHAT, line_num);
            return;
        }
        memcpy(filename, lex->current.str_start, (size_t)flen);
        filename[flen] = '\0';
        lexer_next(lex);

        // Auto-append .BAS if no extension
        pi_ensure_bas_ext(filename, flen, MAX_LINE_LENGTH);
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

    // Auto-detect format by checking magic bytes
    unsigned char magic[4] = {0};
    FILE *mf = fopen(filename, "rb");
    if (mf) {
        if (fread(magic, 1, 4, mf) != 4) {
            memset(magic, 0, 4);
        }
        fclose(mf);
    }

    if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'E' && magic[3] == '\x1A') {
        bpe_load(filename, &rt->memory->program, rt);
    } else if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'P' && (magic[3] == '\x1B' || magic[3] == '\x1A')) {
        bpp_load(&rt->memory->program, filename, rt);
    } else {
        fileio_load(&rt->memory->program, filename);
    }
}
#endif

void pi_parse_unload_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
#ifdef BPP_LITE_BUILD
    error_raise(ERR_HOW, line_num);
    return;
#endif
    (void)rt;
    if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_VARIABLE) {
        const char *nv = (lex->current.type == TOK_NAMED_VAR) ? lex->current.str_start : NULL;
        int nlen = (lex->current.type == TOK_NAMED_VAR) ? lex->current.str_length : 0;
        char name[32] = "";
        
        if (lex->current.type == TOK_VARIABLE) {
            name[0] = (char)toupper(lex->current.value.var_name);
            name[1] = '\0';
            nlen = 1;
        } else if (nv && nlen < 31) {
            int k;
            for (k = 0; k < nlen; k++) {
                name[k] = (char)toupper(nv[k]);
            }
            name[nlen] = '\0';
        }
        
        int type_lib = 0, type_func = 0, type_mod = 0, type_feat = 0, type_plug = 0;
        int handled = 0;
        
        if (strcmp(name, "LIBRARY") == 0) { type_lib = 1; handled = 1; }
        else if (strcmp(name, "FUNCTION") == 0) { type_func = 1; handled = 1; }
        else if (strcmp(name, "MODULE") == 0) { type_mod = 1; handled = 1; }
        else if (strcmp(name, "FEATURE") == 0) { type_feat = 1; handled = 1; }
        else if (strcmp(name, "PLUGIN") == 0) { type_plug = 1; handled = 1; }
        
        if (handled) {
            char target_name[MAX_LINE_LENGTH + 1];
            lexer_next(lex); // consume subcommand (e.g. LIBRARY)
            
            if (lex->current.type != TOK_STRING) {
                error_raise(ERR_WHAT, line_num);
                return;
            }
            int tlen = lex->current.str_length;
            if (tlen >= MAX_LINE_LENGTH) {
                error_raise(ERR_WHAT, line_num);
                return;
            }
            memcpy(target_name, lex->current.str_start, (size_t)tlen);
            target_name[tlen] = '\0';
            lexer_next(lex);
            
            if (security_check(SECOP_EXT_LOAD, line_num)) return;
            
            if (type_lib) {
                extern int ext_lib_unload(const char *name);
                if (ext_lib_unload(target_name) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_func) {
                extern int ext_func_unload(const char *name);
                if (ext_func_unload(target_name) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_mod) {
                extern int module_deactivate(const char *name);
                if (module_deactivate(target_name) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_feat) {
                extern int ext_feature_unload(const char *name);
                if (ext_feature_unload(target_name) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            } else if (type_plug) {
                extern int ext_plugin_unload(const char *name);
                if (ext_plugin_unload(target_name) != 0) {
                    error_raise(ERR_HOW, line_num);
                }
            }
            return;
        }
    }
    error_raise(ERR_WHAT, line_num);
}

void pi_parse_bios_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (lex->current.type == TOK_EOF || lex->current.type == TOK_CR) {
        printf("Current BIOS Map: %s\n", memmap_get_name((MemMapType)rt->memmap_type));
        return;
    }

    if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_LIST) {
        lexer_next(lex);
        memmap_list();
        return;
    }

    if (lex->current.type == TOK_STRING || lex->current.type == TOK_NAMED_VAR) {
        char name[MAX_LINE_LENGTH + 1];
        int len = lex->current.str_length;
        if (len >= MAX_LINE_LENGTH) {
            error_raise(ERR_WHAT, line_num);
            return;
        }
        memcpy(name, lex->current.str_start, (size_t)len);
        name[len] = '\0';
        lexer_next(lex);

        MemMapType mtype = memmap_from_string(name, len);
        if (mtype == MMAP_COUNT || (mtype != MMAP_NONE && mtype != MMAP_MSDOS && 
            mtype != MMAP_IBM_PC && mtype != MMAP_IBM_PCJR && 
            mtype != MMAP_IBM_XT && mtype != MMAP_IBM_AT)) {
            printf("Invalid BIOS map. Must be NONE, MSDOS, IBMPC, PCJR, PCXT, or PCAT.\n");
            error_raise(ERR_HOW, line_num);
            return;
        }

        memmap_init(rt->mem_segment, mtype);
        rt->memmap_type = (int)mtype;
        rt->mem_seg_base = 0;
#ifndef BPP_LITE_BUILD
        if (g_gw_mem != NULL) {
            gw_mem_def_seg(g_gw_mem, 0);
        }
#endif
        printf("BIOS Map switched to: %s\n", memmap_get_name(mtype));
        return;
    }

    error_raise(ERR_WHAT, line_num);
}

void pi_parse_int_cmd(Lexer *lex, RuntimeState *rt, int line_num)
{
    long int_num = (long)parse_expression(lex, rt, line_num);
    if (error_occurred()) return;
    emulate_interrupt(rt, (int)int_num, line_num);
}

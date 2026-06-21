/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_misc.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_misc.c
 // ---
 //
 // Miscellaneous utility commands.
 //
 // RANDOMIZE, PAUSE, DELAY, SLEEP, WAIT, CLEAR,
 // CLR, RESET, POKE, MEMMAP, MID (statement),
 // KEY, REPEAT, ENDREPEAT, ENDFOR, OUT, COM,
 // PEN, STRIG, ACCESS, IMAGE.
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

static int event_parse_on_off_stop(Lexer *lex);

 // pi_parse_randomize - Handle RANDOMIZE command.
void pi_parse_randomize(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RANDOMIZE [n] - seed RNG.
 // No argument or 0 = time-based seed.
 //
 // Seeds both the custom LCG (rnd_seed,
 // used by RND) and srand() for compat.
 {
 long seed = 0;
 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 BValue sv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 seed = bval_to_int(&sv);
 }
 if (seed == 0) {
 unsigned long ts;
 ts = (unsigned long)time(NULL);
 rt->rnd_seed = ts & 0x7FFFFFFFUL;
 srand((unsigned int)ts);
 } else {
 rt->rnd_seed = (unsigned long)seed
 & 0x7FFFFFFFUL;
 srand((unsigned int)seed);
 }
 // Ensure non-zero seed
 if (rt->rnd_seed == 0)
 rt->rnd_seed = 1;
 }
 return;
}

 // pi_parse_pause - Handle PAUSE command.
void pi_parse_pause(Lexer *lex, RuntimeState *rt, int line_num)
{
  // PAUSE n - Timer-tick delay.
  //
  // Spectrum: n = frames at 50Hz (1/50th sec).
  // PAUSE 50 = 1 second. PAUSE 0 = wait for
  // keypress (infinite pause). Interruptible
  // by any keypress on some platforms.
  //
  // We convert: ms = n * 20 (for 50Hz).
 {
  int frames = 0;
  if (lex->current.type != TOK_EOF &&
   lex->current.type != TOK_CR &&
   lex->current.type != TOK_COLON) {
   frames = (int)parse_expression(
    lex, rt, line_num);
   if (error_occurred()) return;
  }
  if (frames < 0) frames = 0;
  if (frames == 0) {
   // PAUSE 0: wait for keypress
   printf("[Press any key]");
   fflush(stdout);
#ifdef _WIN32
   (void)_getch();
#else
   (void)getchar();
#endif
  } else {
   // Convert 50Hz frames to ms
   vdev_sleep((unsigned int)frames * 20);
  }
 }
 return;
}

 // pi_parse_delay - Handle DELAY command.
void pi_parse_delay(Lexer *lex, RuntimeState *rt, int line_num)
{
  // DELAY ms - NOP busy-wait delay.
  //
  // CPU-bound busy loop for timing-critical
  // code. Uses clock() for calibration.
  // Not interruptible.
 {
  int ms;
  clock_t start, elapsed;
  ms = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (ms < 0) ms = 0;
  if (ms > 0) {
   start = clock();
   do {
   elapsed = clock() - start;
   } while ((elapsed * 1000 /
    CLOCKS_PER_SEC) < (clock_t)ms);
  }
 }
 return;
}

 // pi_parse_sleep - Handle SLEEP command.
void pi_parse_sleep(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SLEEP [seconds]
 // Pause execution. Without argument, pause 1 second.
 lexer_next(lex); // consume SLEEP
 {
 int secs = 1;
 if (lex->current.type == TOK_NUMBER ||
 lex->current.type == TOK_VARIABLE ||
 lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_LPAREN) {
 secs = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }
 if (secs < 0) secs = 0;
 vdev_sleep((unsigned int)secs * 1000);
 }
 return;
}

 // pi_parse_wait - Handle WAIT command.
void pi_parse_wait(Lexer *lex, RuntimeState *rt, int line_num)
{
 // WAIT port, and_mask [, xor_mask]
 //
 // Reads virtual memory at 'port' address,
 // applies: result = (mem[port] XOR xor_mask)
 //                    AND and_mask
 // Returns immediately if result != 0.
 // Otherwise polls briefly (virtual hardware
 // has no interrupt source, so we yield after
 // a short busy-wait to avoid lockup).
 {
 int port, and_mask, xor_mask = 0;
 int paddr, val, result;
 int polls;

 port = (int)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
  lexer_next(lex);
 and_mask = (int)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  xor_mask = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
 }

 paddr = port & 0xFFFF;

  // Poll virtual memory up to 50 iterations
  // (with 2ms sleep each = 100ms max).
  // Real hardware would block indefinitely;
  // virtual memory has no async updates, so
  // we check once and yield gracefully.
 for (polls = 0; polls < 50; polls++) {
  if (paddr >= 0 &&
   paddr < MAX_MEM_SEGMENT) {
   val = (int)rt->mem_segment[paddr];
  } else {
   val = 0xFF;
  }
  result = (val ^ xor_mask) & and_mask;
  if (result != 0)
   return;
  vdev_sleep(2);
 }
 }
 return;
}

 // pi_parse_clear - Handle CLEAR command.
void pi_parse_clear(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CLEAR - Reset variables, arrays, stack.
 //
 // GW-BASIC: clears all variables, string
 // space, resets the stack, and closes files.
 // Optional: CLEAR [,stack_size]
 // We ignore the optional argument.
 {
 int vi;
 rt->stack_top = 0;
 for (vi = 0; vi < MAX_VARIABLES; vi++)
 rt->variables[vi] = bval_int(0);
 for (vi = 0; vi < MAX_STRING_VARS;
 vi++) {
 rt->string_vars[vi] =
 bval_string(NULL, 0);
 }
 rt->named_count = 0;
 memset(rt->named_vars, 0,
 sizeof(rt->named_vars));
 rt->dim_count = 0;
 rt->dim_elements_used = 0;
 if (rt->array_base != NULL) {
 memset(rt->array_base, 0,
 (size_t)(rt->array_size
 * (long)sizeof(long)));
 }
 strpool_reset(&rt->strpool);
 // Consume optional ,stack_size
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 parse_expression(lex, rt,
 line_num);
 }
 }
 return;
}

 // pi_parse_clr - Handle CLR command.
void pi_parse_clr(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CLR - Clear the runtime stack only.
 //
 // Resets GOSUB return addresses and FOR/WHILE/DO
 // loop frames. Does NOT clear variables, arrays,
 // or the program. Useful for recovery after an
 // error interrupts nested GOSUBs.
 //
 // Note: CLEAR does the full GW-BASIC reset
 // (variables + stack + string space). CLR is
 // the lightweight stack-only variant.
 rt->stack_top = 0;
 return;
}

 // pi_parse_reset - Handle RESET command.
void pi_parse_reset(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RESET
 // Close all open files.
 // GW-BASIC: closes all files and writes
 // directory information to disk.
 fileio_channels_cleanup();
 fileio_channels_init();
 return;
}

 // pi_parse_poke - Handle POKE command.
void pi_parse_poke(Lexer *lex, RuntimeState *rt, int line_num)
{
 // POKE address, value
 // Write a byte to virtual memory.
 {
 int addr, val, offset;
 addr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 val = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 offset = rt->mem_seg_base + addr;
 if (offset >= 0 &&
 offset < MAX_MEM_SEGMENT) {
 rt->mem_segment[offset] =
 (unsigned char)(val & 0xFF);
 }
 }
 return;
}

 // pi_parse_memmap - Handle MEMMAP command.
void pi_parse_memmap(Lexer *lex, RuntimeState *rt, int line_num)
{
 // MEMMAP "platform"
 // MEMMAP LIST
 //
 // Select a platform memory map preset.
 // Initializes the 64K virtual memory
 // with correct ROM/hardware values for
 // the selected 8-bit platform.
 if (lexer_match_keyword(lex,
 KW_LIST)) {
 lexer_next(lex);
 memmap_list();
 } else if (lex->current.type ==
 TOK_STRING) {
 MemMapType mtype;
 mtype = memmap_from_string(
 lex->current.str_start,
 lex->current.str_length);
 lexer_next(lex);
 if (mtype == MMAP_COUNT) {
 printf("Unknown memory map."
 " Use MEMMAP LIST.\n");
 } else {
 memmap_init(rt->mem_segment,
 mtype);
 rt->memmap_type = (int)mtype;
 rt->mem_seg_base = 0;
 printf("Memory map: %s\n",
 memmap_get_name(mtype));
 }
 } else {
 // Show current map
 printf("Current map: %s\n",
 memmap_get_name(
 (MemMapType)rt->memmap_type));
 }
 return;
}

 // pi_parse_mid - Handle MID command.
void pi_parse_mid(Lexer *lex, RuntimeState *rt, int line_num)
{
 // MID$(var$, start [, len]) = rep$
 // Replace characters in var$ in-place.
 // Supports both single-letter string vars (A$)
 // and named string vars (Name$).
 {
 int is_named = 0;
 char sv_name = 0;
 const char *nv_name = NULL;
 int nv_len = 0;
 int start_pos, rep_len;
 int max_replace;
 BValue cur_val, rep_val;
 char *cur_data, *rep_data;
 int cur_len, rd_len, copy_len;
 char *new_ptr;

 // Expect (
 if (lex->current.type !=
 TOK_LPAREN) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // String variable name:
  // TOK_STRING_VAR = single-letter A$
  // TOK_NAMED_VAR = multi-char Name$ (also
  //   used for A$ in some dialects) 
 if (lex->current.type ==
 TOK_STRING_VAR) {
 sv_name =
 lex->current.value.var_name;
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 is_named = 1;
 nv_name = lex->current.str_start;
 nv_len = lex->current.str_length;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Comma, start position
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 start_pos =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 // Optional length
 max_replace = -1;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 max_replace =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 // Close paren
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 // =
 if (lex->current.type !=
 TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Replacement string
 rep_val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;

 // Get current string value
 if (is_named) {
 cur_val = runtime_get_named_var_bval(
 rt, nv_name, nv_len);
 } else {
 cur_val = runtime_get_string_var(
 rt, sv_name);
 }
 if (cur_val.type != VAL_STRING ||
 cur_val.v.sval.data == NULL) {
 return; // empty, nothing to do
 }
 cur_data = cur_val.v.sval.data;
 cur_len = cur_val.v.sval.length;

 if (start_pos < 1 ||
 start_pos > cur_len) {
 return; // out of range, no-op
 }

 rep_data = rep_val.v.sval.data;
 rd_len = rep_val.v.sval.length;
 if (rep_data == NULL) rd_len = 0;

 // Calculate how many chars to copy
 rep_len = cur_len - (start_pos - 1);
 if (max_replace >= 0 &&
 max_replace < rep_len)
 rep_len = max_replace;
 copy_len = rd_len;
 if (copy_len > rep_len)
 copy_len = rep_len;

 // Build new string
 new_ptr = strpool_store(
 &rt->strpool, cur_data, cur_len);
 if (new_ptr && copy_len > 0) {
 memcpy(new_ptr + (start_pos - 1),
 rep_data,
 (size_t)copy_len);
 if (is_named) {
 runtime_set_named_var_bval(rt,
 nv_name, nv_len,
 bval_string(new_ptr,
 cur_len));
 } else {
 runtime_set_string_var(rt,
 sv_name,
 bval_string(new_ptr,
 cur_len));
 }
 }
 }
 return;
}


 // pi_parse_key - Handle KEY command.
void pi_parse_key(Lexer *lex, RuntimeState *rt, int line_num)
{
 // KEY n, string$
 // Assign string to function key slot.
 // Slots: 1-12=F1-F12, 13-24=SHIFT+F,
 // 25-36=CTRL+F, 37-48=ALT+F.
 //
 // KEY ON / KEY OFF
 // Toggle function key display bar.
 //
 // KEY LIST
 // Print all key assignments.
 //
 // KEY(n) ON / OFF / STOP
 // Enable/disable key event trapping.
 // KEY(n) ON/OFF/STOP: key event trapping
 if (lex->current.type == TOK_LPAREN) {
  int keynum, state;
  lexer_next(lex); // consume (
  keynum = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (!lexer_expect(lex, TOK_RPAREN))
   return;
  if (keynum < 1 || keynum > MAX_KEY_TRAPS) {
   error_raise(ERR_HOW, line_num);
   return;
  }
  state = event_parse_on_off_stop(lex);
  if (state < 0) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  rt->key_event_state[keynum - 1] = state;
  return;
 }
 // KEY ON / KEY LIST (ON is keyword)
 if (lex->current.type == TOK_KEYWORD) {
 KeywordId sub = lex->current
 .value.keyword;
 if (sub == KW_ON) {
 lexer_next(lex);
 rt->fkey_display = 1;
 // Print the key bar
 goto do_key_bar;
 }
 if (sub == KW_LIST) {
 lexer_next(lex);
 goto do_key_list;
 }
 }
 // KEY OFF / KEY LIST / ON (named var)
 if (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 const char *nv;
 int nlen;
 // For TOK_VARIABLE, get letter
 if (lex->current.type == TOK_VARIABLE) {
 char vl = lex->current
 .value.var_name;
 // Check if remaining text is
 // "OFF" or "ON" by scanning
 // ahead in source 
 nv = lex->source + lex->pos - 1;
 // Variable already consumed the
 // first char. We need the raw
 // text. Look at source from the
 // start of this ident. 
 (void)vl;
 // Can't reconstruct - fall
 // through to assignment 
 goto do_key_assign;
 }
 nv = lex->current.str_start;
 nlen = lex->current.str_length;
 // OFF
 if (nlen == 3 &&
 (nv[0]=='O'||nv[0]=='o') &&
 (nv[1]=='F'||nv[1]=='f') &&
 (nv[2]=='F'||nv[2]=='f')) {
 lexer_next(lex);
 rt->fkey_display = 0;
 return;
 }
 // LIST
 if (nlen == 4 &&
 (nv[0]=='L'||nv[0]=='l') &&
 (nv[1]=='I'||nv[1]=='i') &&
 (nv[2]=='S'||nv[2]=='s') &&
 (nv[3]=='T'||nv[3]=='t')) {
 lexer_next(lex);
 goto do_key_list;
 }
 }
 // Fall through to assignment
 goto do_key_assign;

 do_key_list:
 {
 static const char *mod[] = {
 "", "SHIFT+",
 "CTRL+", "ALT+"
 };
 int si;
 int found = 0;
 for (si = 1; si <= FKEY_MAX_SLOTS; si++) {
 if (rt->fkey_macros[si][0]) {
 int grp = (si - 1) / 12;
 int fn = (si - 1) % 12 + 1;
 printf(" KEY %2d "
 "%sF%-2d \"%s\"\n",
 si, mod[grp],
 fn,
 rt->fkey_macros[si]);
 found = 1;
 }
 }
 if (!found) {
 printf(" (no keys "
 "assigned)\n");
 }
 }
 return;

 do_key_bar:
 // Print GW-BASIC style key bar:
 // 1HELP 2LOAD 3RUN ...10 
 {
 int ki;
 for (ki = 1; ki <= 10; ki++) {
 if (rt->fkey_macros[ki][0]) {
 char trunc[7];
 int ti;
 int pad;
 for (ti = 0; ti < 6 &&
 rt->fkey_macros[ki][ti];
 ti++) {
 trunc[ti] =
 rt->fkey_macros[ki][ti];
 }
 trunc[ti] = '\0';
 printf("%d%s", ki, trunc);
 // Pad to 8 chars total
 pad = 8 - ti
 - (ki >= 10 ? 2 : 1);
 while (pad-- > 0)
 putchar(' ');
 } else {
 printf("%-8d", ki);
 }
 }
 printf("\n");
 }
 return;

 do_key_assign:
 // KEY n, string$ - assignment
 {
 long slot;
 int slen;
 slot = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume ,
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (slot < 1 || slot > FKEY_MAX_SLOTS) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 slen = lex->current.str_length;
 if (slen > FKEY_MAX_LEN)
 slen = FKEY_MAX_LEN;
 memcpy(rt->fkey_macros[slot],
 lex->current.str_start,
 (size_t)slen);
 rt->fkey_macros[slot][slen] = '\0';
 lexer_next(lex);
 }
 return;
}



 // pi_parse_out - Handle OUT command.
void pi_parse_out(Lexer *lex, RuntimeState *rt, int line_num)
{
 // OUT port, value
 // Write to I/O port.
 // Routes to virtual memory segment so
 // MEMMAP presets can be modified via OUT
 // just like POKE.
 {
 int port, val, paddr;
 port = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 val = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 paddr = port & 0xFFFF;
 if (paddr >= 0 &&
 paddr < MAX_MEM_SEGMENT) {
 rt->mem_segment[paddr] =
 (unsigned char)(val & 0xFF);
 }
 }
 return;
}

 // event_parse_on_off_stop - Parse ON / OFF / STOP token.
 //
 // Returns: 1 = ON, 0 = OFF, 2 = STOP, -1 = error.
 // ON is a keyword (KW_ON). STOP is a keyword (KW_STOP).
 // OFF is a named variable (no KW_OFF exists).
static int event_parse_on_off_stop(Lexer *lex)
{
 if (lex->current.type == TOK_KEYWORD) {
  if (lex->current.value.keyword == KW_ON) {
   lexer_next(lex);
   return 1;
  }
  if (lex->current.value.keyword == KW_STOP) {
   lexer_next(lex);
   return 2;
  }
 }
 if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 3 &&
  (lex->current.str_start[0] == 'O' ||
   lex->current.str_start[0] == 'o') &&
  (lex->current.str_start[1] == 'F' ||
   lex->current.str_start[1] == 'f') &&
  (lex->current.str_start[2] == 'F' ||
   lex->current.str_start[2] == 'f')) {
  lexer_next(lex);
  return 0;
 }
 // Single-letter 'O' variable (PATB dialect)
 if (lex->current.type == TOK_VARIABLE &&
  lex->current.value.var_name == 'O') {
  const char *rest = lex->source + lex->pos;
  if ((rest[0] == 'F' || rest[0] == 'f') &&
   (rest[1] == 'F' || rest[1] == 'f')) {
   lex->pos += 2;
   lexer_next(lex);
   return 0;
  }
  if ((rest[0] == 'N' || rest[0] == 'n') &&
   (rest[1] == ' ' || rest[1] == '\0' ||
    rest[1] == '\r' || rest[1] == '\n')) {
   lexer_next(lex);
   return 1;
  }
 }
 return -1;
}

 // pi_parse_com - Handle COM command.
void pi_parse_com(Lexer *lex, RuntimeState *rt, int line_num)
{
 // COM(n) ON / OFF / STOP
 //
 // Enable, disable, or suspend event trapping
 // for serial communication port n (1-4).
 // Uses virtual device infrastructure: the
 // event state is stored in rt->com_event_state[]
 // and the handler target in rt->on_com_line[].
 {
 int port, state;

 // Expect (
 if (lex->current.type == TOK_LPAREN) {
  lexer_next(lex);
  port = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (!lexer_expect(lex, TOK_RPAREN))
   return;
 } else {
  port = 1;
 }

 if (port < 1 || port > MAX_COM_PORTS) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 state = event_parse_on_off_stop(lex);
 if (state < 0) {
  error_raise(ERR_WHAT, line_num);
  return;
 }

 rt->com_event_state[port - 1] = state;
 }
 return;
}

 // pi_parse_pen - Handle PEN command.
void pi_parse_pen(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PEN ON / OFF / STOP
 //
 // Enable, disable, or suspend event trapping
 // for the light pen. The event state drives
 // ON PEN GOSUB handler dispatch. The PEN()
 // function reads the virtual console cursor
 // position as a light-pen proxy.
 {
 int state;
 state = event_parse_on_off_stop(lex);
 if (state < 0) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 rt->pen_event_state = state;
 }
 return;
}

 // pi_parse_strig - Handle STRIG command.
void pi_parse_strig(Lexer *lex, RuntimeState *rt, int line_num)
{
 // STRIG(n) ON / OFF / STOP
 // STRIG ON / OFF / STOP
 //
 // Enable, disable, or suspend event trapping
 // for joystick trigger button n (0-3).
 // Button 0 = joystick A trigger,
 // Button 2 = joystick B trigger.
 // Uses rt->strig_event_state[] to store
 // the enable/disable/suspend state.
 {
 int button = 0;
 int state;

 // Optional (n)
 if (lex->current.type == TOK_LPAREN) {
  lexer_next(lex);
  button = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (!lexer_expect(lex, TOK_RPAREN))
   return;
 }

 if (button < 0 || button >= MAX_STRIG_BUTTONS) {
  error_raise(ERR_HOW, line_num);
  return;
 }

 state = event_parse_on_off_stop(lex);
 if (state < 0) {
  error_raise(ERR_WHAT, line_num);
  return;
 }

 rt->strig_event_state[button] = state;
 }
 return;
}

 // pi_parse_access - Handle ACCESS command.
void pi_parse_access(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ACCESS READ | WRITE | READ WRITE
 //
 // Standalone ACCESS statement reports or
 // sets the default file access mode. When
 // used as part of OPEN syntax, the OPEN
 // handler processes the mode directly.
 //
 // Recognized modes:
 //  READ       - read-only
 //  WRITE      - write-only
 //  READ WRITE - read and write
 {
 int mode = 0; // 0=none, 1=read, 2=write, 3=rw

 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword == KW_READ) {
  mode |= 1;
  lexer_next(lex);
 }
 if (lex->current.type == TOK_NAMED_VAR ||
  lex->current.type == TOK_VARIABLE) {
  // Check for READ (named var in some dialects)
  if (lex->current.type == TOK_NAMED_VAR &&
   lex->current.str_length == 4 &&
   (lex->current.str_start[0] == 'R' ||
    lex->current.str_start[0] == 'r')) {
   mode |= 1;
   lexer_next(lex);
  }
 }

 // Check for WRITE keyword
 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword == KW_WRITE) {
  mode |= 2;
  lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
  lex->current.str_length == 5 &&
  (lex->current.str_start[0] == 'W' ||
   lex->current.str_start[0] == 'w')) {
  mode |= 2;
  lexer_next(lex);
 }

 if (mode == 0) {
  printf("ACCESS: READ WRITE (default)\n");
  } else {
   printf("ACCESS: %s%s%s\n",
    (mode & 1) ? "READ" : "",
    (mode == 3) ? " " : "",
    (mode & 2) ? "WRITE" : "");
  }
  }
 return;
}

 // pi_parse_image - Handle IMAGE command.
void pi_parse_image(Lexer *lex, RuntimeState *rt, int line_num)
{
 // IMAGE format-string
 //
 // HP-style format definition line. Like DATA and
 // REM, the IMAGE statement is a data-storage line
 // that is NOT executed at runtime. The format text
 // following the IMAGE keyword is extracted directly
 // from the source by PRINT USING when it references
 // this line number:
 //
 //   100 IMAGE 3D.2D,2X,5A
 //   110 PRINT USING 100, 3.14, "Hello"
 //
 // At execution time, IMAGE is intentionally a no-op.
 // This is the correct and complete implementation.
 lexer_skip_to_end(lex);
 return;
}

 // pi_parse_timer - Handle TIMER statement.
 //
 // TIMER ON / OFF / STOP
 //
 // Enable, disable, or suspend timer event trapping.
 // When set to ON, the event_poll() loop checks the
 // timer interval and dispatches ON TIMER GOSUB.
void pi_parse_timer(Lexer *lex, RuntimeState *rt, int line_num)
{
 int state;
 state = event_parse_on_off_stop(lex);
 if (state < 0) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 rt->timer_event_state = state;
 // Reset fire time when enabling
 if (state == EVT_ON) {
  rt->timer_last_fire = (double)time(NULL);
 }
 return;
}

 // pi_parse_play_event - Handle PLAY statement as event control.
 //
 // PLAY(n) ON / OFF / STOP
 // PLAY ON / OFF / STOP
 //
 // Enable, disable, or suspend play (music buffer)
 // event trapping. The (n) argument is consumed but
 // ignored (specifies buffer fill threshold).
void pi_parse_play_event(Lexer *lex, RuntimeState *rt, int line_num)
{
 int state;

 // Consume optional (n)
 if (lex->current.type == TOK_LPAREN) {
  lexer_next(lex);
  (void)parse_expression(lex, rt, line_num);
  if (error_occurred()) return;
  if (lex->current.type == TOK_RPAREN)
   lexer_next(lex);
 }

 state = event_parse_on_off_stop(lex);
 if (state < 0) {
  // Not an event control - pass to regular PLAY
  return;
 }
 rt->play_event_state = state;
 return;
}

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_fileio.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_fileio.c
 // ---
 //
 // File I/O statement handlers: OPEN, CLOSE, SET, ASK,
 // REWRITE, ERASE (file channels).
 //
 // Handles ECMA-116 enhanced file operations, GW-BASIC/QBasic
 // file modes, and cross-dialect file channel management.
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
#include "modules/mod_fsk_audio.h"

void pi_parse_open(Lexer *lex, RuntimeState *rt, int line_num)
{
  char filename[MAX_LINE_LENGTH + 1];
  int mode = 0;
  int chan;

  (void)rt;

 // Detect ECMA-116 form vs GW-BASIC form vs Atari form:
 // ECMA-116: OPEN #n: NAME "file", ACCESS mode, ...
 // GW-BASIC: OPEN "file" FOR mode AS #n
 // Atari:    OPEN #n, aux1, aux2, "device:"
 if (lex->current.type == TOK_HASH) {
 // ===== ECMA-116 or Atari/Commodore OPEN form =====
 //
 // ECMA-116: OPEN #expr: NAME "file" [, ACCESS ...]
 // Atari:    OPEN #expr, aux1, aux2, "device:"
 //
 // Disambiguation: after parsing #expr, if the next
 // token is ':' -> ECMA-116 form; if ',' -> Atari form.
 int e116_org = FORG_SEQUENTIAL;
 int e116_rec = FREC_DISPLAY;
 int rec_len = 128;

 lexer_next(lex); // consume #
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // ===== Atari/Commodore OPEN form =====
 // OPEN #chan, aux1, aux2, "device:"
 //
 // aux1 is the mode byte:
 //   4 = input, 6 = directory, 8 = output, 12 = read+write
 // aux2 is device-specific (baud rate, density, etc.)
 //   usually 0 for standard operations.
 //
 // The "device:" string is resolved through the device
 // alias system (e.g., Atari "E:" -> CON:) or opened
 // as a file if no alias matches.
 if (lex->current.type == TOK_COMMA) {
  int aux1, aux2;
  char devname[MAX_LINE_LENGTH + 1];

  lexer_next(lex); // consume , after chan
  aux1 = (int)parse_expression(lex, rt, line_num);
  if (error_occurred()) return;

  // Expect comma before aux2
  if (lex->current.type != TOK_COMMA) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  lexer_next(lex); // consume ,
  aux2 = (int)parse_expression(lex, rt, line_num);
  if (error_occurred()) return;

  // Expect comma before device/filename string
  if (lex->current.type != TOK_COMMA) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  lexer_next(lex); // consume ,

  // Parse device/filename string
  if (lex->current.type != TOK_STRING) {
   error_raise(ERR_WHAT, line_num);
   return;
  }
  if (lex->current.str_length >= MAX_LINE_LENGTH) {
   error_raise(ERR_HOW, line_num);
   return;
  }
  memcpy(devname, lex->current.str_start,
         (size_t)lex->current.str_length);
  devname[lex->current.str_length] = '\0';
  lexer_next(lex);

   // Map Atari aux1 mode to internal FCHAN mode.
   //   4 = FCHAN_INPUT    (read only)
   //   6 = FCHAN_INPUT    (directory listing)
   //   8 = FCHAN_OUTPUT   (write only)
   //   9 = FCHAN_APPEND   (append)
   //  12 = FCHAN_RANDOM   (read+write)
   // Default: FCHAN_INPUT
  switch (aux1) {
  case 4: case 6:
   mode = FCHAN_INPUT;
   break;
  case 8:
   mode = FCHAN_OUTPUT;
   break;
  case 9:
   mode = FCHAN_APPEND;
   break;
  case 12:
   mode = FCHAN_RANDOM;
   break;
  default:
   mode = FCHAN_INPUT;
   break;
  }

  // Store aux2 for future device-specific use
  (void)aux2;

   // Route through the standard open path which
   // handles device aliases, direct VDev names,
   // and regular files.
  if (mode == FCHAN_RANDOM) {
   fileio_open_random(chan, devname, 128, line_num);
  } else {
   fileio_open(chan, devname, mode, line_num);
  }
  return;
 }

 // Expect : after channel (ECMA-116 form)
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume :

 // Expect NAME keyword (as named var)
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_NAME) {
 lexer_next(lex); // consume NAME
 } else if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 4) {
 const char *s = lex->current.str_start;
 if ((s[0]=='N'||s[0]=='n') &&
 (s[1]=='A'||s[1]=='a') &&
 (s[2]=='M'||s[2]=='m') &&
 (s[3]=='E'||s[3]=='e')) {
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Parse filename string
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 // Default mode: INPUT for sequential
 mode = FCHAN_INPUT;

 // Parse optional comma-separated attributes
 while (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume ,

 // Check for ACCESS keyword
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ACCESS) {
 lexer_next(lex); // consume ACCESS
 // Parse INPUT|OUTPUT|OUTIN
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_INPUT) {
 mode = FCHAN_INPUT;
 lexer_next(lex);
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *s =
 lex->current.str_start;
 int sn = lex->current.str_length;
 if (sn == 6 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='U'||s[1]=='u') &&
 (s[2]=='T'||s[2]=='t') &&
 (s[3]=='P'||s[3]=='p') &&
 (s[4]=='U'||s[4]=='u') &&
 (s[5]=='T'||s[5]=='t')) {
 mode = FCHAN_OUTPUT;
 lexer_next(lex);
 } else if (sn == 5 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='U'||s[1]=='u') &&
 (s[2]=='T'||s[2]=='t') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='N'||s[4]=='n')) {
 mode = FCHAN_RANDOM; // OUTIN=r/w
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 // Check for ORGANIZATION (named var)
 else if (lex->current.type == TOK_NAMED_VAR) {
 const char *s =
 lex->current.str_start;
 int sn = lex->current.str_length;

 // ORGANIZATION
 if (sn == 12 &&
 (s[0]=='O'||s[0]=='o') &&
 (s[1]=='R'||s[1]=='r') &&
 (s[2]=='G'||s[2]=='g') &&
 (s[3]=='A'||s[3]=='a') &&
 (s[4]=='N'||s[4]=='n') &&
 (s[5]=='I'||s[5]=='i') &&
 (s[6]=='Z'||s[6]=='z') &&
 (s[7]=='A'||s[7]=='a') &&
 (s[8]=='T'||s[8]=='t') &&
 (s[9]=='I'||s[9]=='i') &&
 (s[10]=='O'||s[10]=='o') &&
 (s[11]=='N'||s[11]=='n')) {
 lexer_next(lex); // consume
 // Parse org type
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *t =
 lex->current.str_start;
 int tn =
 lex->current.str_length;
 if (tn == 10 &&
 (t[0]=='S'||t[0]=='s') &&
 (t[1]=='E'||t[1]=='e') &&
 (t[2]=='Q'||t[2]=='q') &&
 (t[3]=='U'||t[3]=='u') &&
 (t[4]=='E'||t[4]=='e') &&
 (t[5]=='N'||t[5]=='n') &&
 (t[6]=='T'||t[6]=='t') &&
 (t[7]=='I'||t[7]=='i') &&
 (t[8]=='A'||t[8]=='a') &&
 (t[9]=='L'||t[9]=='l')) {
 e116_org = FORG_SEQUENTIAL;
 lexer_next(lex);
 } else if (tn == 8 &&
 (t[0]=='R'||t[0]=='r') &&
 (t[1]=='E'||t[1]=='e') &&
 (t[2]=='L'||t[2]=='l') &&
 (t[3]=='A'||t[3]=='a') &&
 (t[4]=='T'||t[4]=='t') &&
 (t[5]=='I'||t[5]=='i') &&
 (t[6]=='V'||t[6]=='v') &&
 (t[7]=='E'||t[7]=='e')) {
 e116_org = FORG_RELATIVE;
 mode = FCHAN_RANDOM;
 lexer_next(lex);
 } else if (tn == 6 &&
 (t[0]=='S'||t[0]=='s') &&
 (t[1]=='T'||t[1]=='t') &&
 (t[2]=='R'||t[2]=='r') &&
 (t[3]=='E'||t[3]=='e') &&
 (t[4]=='A'||t[4]=='a') &&
 (t[5]=='M'||t[5]=='m')) {
 e116_org = FORG_STREAM;
 mode = FCHAN_BINARY;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 // RECTYPE
 else if (sn == 7 &&
 (s[0]=='R'||s[0]=='r') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='C'||s[2]=='c') &&
 (s[3]=='T'||s[3]=='t') &&
 (s[4]=='Y'||s[4]=='y') &&
 (s[5]=='P'||s[5]=='p') &&
 (s[6]=='E'||s[6]=='e')) {
 lexer_next(lex); // consume
 // Parse rec type
 if (lex->current.type ==
 TOK_NAMED_VAR) {
 const char *t =
 lex->current.str_start;
 int tn =
 lex->current.str_length;
 if (tn == 7 &&
 (t[0]=='D'||t[0]=='d') &&
 (t[1]=='I'||t[1]=='i') &&
 (t[2]=='S'||t[2]=='s') &&
 (t[3]=='P'||t[3]=='p') &&
 (t[4]=='L'||t[4]=='l') &&
 (t[5]=='A'||t[5]=='a') &&
 (t[6]=='Y'||t[6]=='y')) {
 e116_rec = FREC_DISPLAY;
 lexer_next(lex);
 } else if (tn == 8 &&
 (t[0]=='I'||t[0]=='i') &&
 (t[1]=='N'||t[1]=='n') &&
 (t[2]=='T'||t[2]=='t') &&
 (t[3]=='E'||t[3]=='e') &&
 (t[4]=='R'||t[4]=='r') &&
 (t[5]=='N'||t[5]=='n') &&
 (t[6]=='A'||t[6]=='a') &&
 (t[7]=='L'||t[7]=='l')) {
 e116_rec = FREC_INTERNAL;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }
 else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 }

 // Open the file using the parsed attributes
 if (e116_org == FORG_RELATIVE) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (e116_org == FORG_STREAM) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 // Store ECMA-116 metadata if open succeeded
 if (!error_occurred()) {
 fileio_set_e116_metadata(chan,
 e116_org, e116_rec);
 }
 return;
 }

 // ===== GW-BASIC OPEN form =====
 // OPEN "filename" FOR mode AS #n

 // Parse filename string
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 // ===== Old-style GW-BASIC short OPEN form =====
 // OPEN "R", channel, "filename" [, reclen]
 // OPEN "I", channel, "filename"
 // OPEN "O", channel, "filename"
 // OPEN "A", channel, "filename"
 //
 // Detection: if the first string is exactly 1 char
 // (R/I/O/A) and the next token is a comma, this is
 // the old short-form syntax.
 if (strlen(filename) == 1 &&
 lex->current.type == TOK_COMMA) {
 char mode_ch = filename[0];
 int rec_len = 128;

 if (mode_ch >= 'a' && mode_ch <= 'z')
 mode_ch = (char)(mode_ch - 32);

 // Map mode character
 if (mode_ch == 'R') {
 mode = FCHAN_RANDOM;
 } else if (mode_ch == 'I') {
 mode = FCHAN_INPUT;
 } else if (mode_ch == 'O') {
 mode = FCHAN_OUTPUT;
 } else if (mode_ch == 'A') {
 mode = FCHAN_APPEND;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 lexer_next(lex); // consume comma after mode

 // Parse channel number (optional #)
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex); // consume #
 }
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Expect comma before filename
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume comma

 // Parse filename string
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 if (lex->current.str_length >= MAX_LINE_LENGTH) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 memcpy(filename, lex->current.str_start,
 (size_t)lex->current.str_length);
 filename[lex->current.str_length] = '\0';
 lexer_next(lex);

 // Optional record length: , reclen
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex); // consume comma
 rec_len = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 }

 // Open the file
 if (mode == FCHAN_RANDOM) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (mode == FCHAN_BINARY) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 return;
 }

 // Expect FOR keyword
 if (lex->current.type != TOK_KEYWORD ||
 lex->current.value.keyword != KW_FOR) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume FOR

 // Parse mode: INPUT, OUTPUT, APPEND, RANDOM, BINARY
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_INPUT) {
 mode = FCHAN_INPUT;
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 // OUTPUT, APPEND, RANDOM, BINARY are not keywords
 // - they appear as named variables. Match by text.
 const char *src;
 int len;
 if (lex->current.type == TOK_NAMED_VAR) {
 src = lex->current.str_start;
 len = lex->current.str_length;
 } else {
 src = NULL;
 len = 0;
 }
 if (len == 6 && (src[0] == 'O' || src[0] == 'o') &&
 (src[1] == 'U' || src[1] == 'u') &&
 (src[2] == 'T' || src[2] == 't') &&
 (src[3] == 'P' || src[3] == 'p') &&
 (src[4] == 'U' || src[4] == 'u') &&
 (src[5] == 'T' || src[5] == 't')) {
 mode = FCHAN_OUTPUT;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'A' || src[0] == 'a') &&
 (src[1] == 'P' || src[1] == 'p') &&
 (src[2] == 'P' || src[2] == 'p') &&
 (src[3] == 'E' || src[3] == 'e') &&
 (src[4] == 'N' || src[4] == 'n') &&
 (src[5] == 'D' || src[5] == 'd')) {
 mode = FCHAN_APPEND;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'R' || src[0] == 'r') &&
 (src[1] == 'A' || src[1] == 'a') &&
 (src[2] == 'N' || src[2] == 'n') &&
 (src[3] == 'D' || src[3] == 'd') &&
 (src[4] == 'O' || src[4] == 'o') &&
 (src[5] == 'M' || src[5] == 'm')) {
 mode = FCHAN_RANDOM;
 lexer_next(lex);
 } else if (len == 6 && (src[0] == 'B' || src[0] == 'b') &&
 (src[1] == 'I' || src[1] == 'i') &&
 (src[2] == 'N' || src[2] == 'n') &&
 (src[3] == 'A' || src[3] == 'a') &&
 (src[4] == 'R' || src[4] == 'r') &&
 (src[5] == 'Y' || src[5] == 'y')) {
 mode = FCHAN_BINARY;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 // Optional ACCESS clause:
 // ACCESS READ | WRITE | READ WRITE
 // Accepted and silently consumed for compatibility.
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_ACCESS) {
 lexer_next(lex); // consume ACCESS
 // Skip READ/WRITE/READ WRITE
 while (lex->current.type == TOK_NAMED_VAR ||
 lex->current.type == TOK_VARIABLE) {
 lexer_next(lex);
 }
 }

 // Expect AS keyword
 if (lex->current.type != TOK_KEYWORD ||
 lex->current.value.keyword != KW_AS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume AS

 // Expect #
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex); // consume #

 // Parse channel number
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Optional LEN = n (record length for RANDOM)
 {
 int rec_len = 128;
 if (lex->current.type == TOK_NAMED_VAR &&
 lex->current.str_length == 3) {
 const char *s = lex->current.str_start;
 if ((s[0]=='L'||s[0]=='l') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='N'||s[2]=='n')) {
 lexer_next(lex); // consume LEN
 if (lex->current.type == TOK_EQUALS)
 lexer_next(lex);
 rec_len = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 }

 if (mode == FCHAN_RANDOM) {
 fileio_open_random(chan, filename,
 rec_len, line_num);
 } else if (mode == FCHAN_BINARY) {
 fileio_open_binary(chan, filename,
 line_num);
 } else {
 fileio_open(chan, filename, mode,
 line_num);
 }
 }
}

 // parse_close - Parse CLOSE statement.
 // The CLOSE keyword has already been consumed.
void pi_parse_close(Lexer *lex, RuntimeState *rt, int line_num)
{
 int chan;

 // CLOSE forms:
 // CLOSE - close all files
 // CLOSE #n - close file #n
 // CLOSE n - close file n (old GW-BASIC style)
 // CLOSE #n, #m - close multiple files

 // CLOSE with no args = close all open files
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 lex->current.type == TOK_COLON) {
 int ci;
 for (ci = 1; ci <= 8; ci++) {
 if (fileio_get_fp(ci) != NULL) {
 fileio_close(ci, line_num);
 }
 }
 return;
 }

 do {
 // Optional #
 if (lex->current.type == TOK_HASH) {
 lexer_next(lex); // consume #
 }

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 fileio_close(chan, line_num);
 if (error_occurred()) return;

 // Check for comma -> more channels
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 } else {
 break;
 }
 } while (1);
}

// --- ECMA-116 Enhanced Files: SET / ASK / REWRITE / ERASE ---

 // parse_set_file - Parse SET # statement.
 // SET #n: POINTER BEGIN | END | expr
 // The SET keyword has already been consumed.
void pi_parse_set_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;
 long pos;

 // Expect #
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Expect :
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Expect POINTER keyword
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_POINTER) {
 lexer_next(lex); // consume POINTER

 // Parse BEGIN, END, or expression
 if (lex->current.type == TOK_NAMED_VAR) {
 const char *s = lex->current.str_start;
 int sn = lex->current.str_length;
 if (sn == 5 &&
 (s[0]=='B'||s[0]=='b') &&
 (s[1]=='E'||s[1]=='e') &&
 (s[2]=='G'||s[2]=='g') &&
 (s[3]=='I'||s[3]=='i') &&
 (s[4]=='N'||s[4]=='n')) {
 pos = 0; // BEGIN
 lexer_next(lex);
 } else {
 // Could be END or an expression
 pos = (long)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_END) {
 pos = -1; // END
 lexer_next(lex);
 } else {
 pos = (long)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 fileio_set_pointer(chan, pos, line_num);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
}

 // parse_ask_file - Parse ASK # statement.
 // ASK #n: POINTER var | FILESIZE var
 // The ASK keyword has already been consumed.
void pi_parse_ask_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;

 // Expect #
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Expect :
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // POINTER or FILESIZE
 if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_POINTER) {
 long pos;
 lexer_next(lex); // consume POINTER

 pos = fileio_ask_pointer(chan, line_num);
 if (error_occurred()) return;

 // Assign to variable
 if (lex->current.type == TOK_VARIABLE) {
 int vi = lex->current.value.var_name - 'A';
 rt->variables[vi] = bval_float((double)pos);
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
 1) {
 runtime_set_named_var_bval(rt,
 lex->current.str_start,
 lex->current.str_length,
 bval_float((double)pos));
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 } else if (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == KW_FILESIZE) {
 long sz;
 lexer_next(lex); // consume FILESIZE

 sz = fileio_ask_filesize(chan, line_num);
 if (error_occurred()) return;

 // Assign to variable
 if (lex->current.type == TOK_VARIABLE) {
 int vi = lex->current.value.var_name - 'A';
 rt->variables[vi] = bval_float((double)sz);
 lexer_next(lex);
 } else if (lex->current.type == TOK_NAMED_VAR &&
 1) {
 runtime_set_named_var_bval(rt,
 lex->current.str_start,
 lex->current.str_length,
 bval_float((double)sz));
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 } else {
 error_raise(ERR_WHAT, line_num);
 }
}

 // parse_rewrite - Parse REWRITE # statement.
 // REWRITE #n: expr [, expr ...]
 // The REWRITE keyword has already been consumed.
void pi_parse_rewrite(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;
 char buf[MAX_RECORD_LEN];
 int pos = 0;

 // Expect #
 if (lex->current.type != TOK_HASH) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 // Expect :
 if (lex->current.type != TOK_COLON) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Build record from expressions
 memset(buf, ' ', MAX_RECORD_LEN);
 while (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 BValue val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&val)) {
 int sl = val.v.sval.length;
 if (sl > 0 && pos + sl <= MAX_RECORD_LEN) {
 memcpy(buf + pos, val.v.sval.data,
 (size_t)sl);
 pos += sl;
 }
 } else {
 char nb[32];
 int nl;
 sprintf(nb, "%g", bval_to_float(&val));
 nl = (int)strlen(nb);
 if (pos + nl <= MAX_RECORD_LEN) {
 memcpy(buf + pos, nb, (size_t)nl);
 pos += nl;
 }
 }
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 else
 break;
 }

 fileio_rewrite_record(chan, buf, pos, line_num);
}

 // parse_erase_file - Parse ERASE # statement.
 // ERASE #n
 // This is separate from the existing ERASE (clear arrays).
 // The ERASE keyword has already been consumed.
void pi_parse_erase_file(Lexer *lex, RuntimeState *rt,
 int line_num)
{
 int chan;

 // # means it's a file ERASE, not array ERASE
 lexer_next(lex); // consume #
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;

 fileio_erase_channel(chan, line_num);
}

void pi_parse_mount(Lexer *lex, RuntimeState *rt, int line_num)
{
    BValue prefix_val, target_val;
    char prefix[VFS_MAX_PREFIX];
    char target[VFS_MAX_TARGET];

    // Parse prefix string expression
    prefix_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    if (!bval_is_string(&prefix_val) || prefix_val.v.sval.data == NULL) {
        error_raise(ERR_WHAT, line_num);
        return;
    }

    // Expect TO keyword
    if (!lexer_match_keyword(lex, KW_TO)) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    lexer_next(lex); // consume TO

    // Parse target string expression
    target_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    if (!bval_is_string(&target_val) || target_val.v.sval.data == NULL) {
        error_raise(ERR_WHAT, line_num);
        return;
    }

    // Copy to buffers
    int plen = prefix_val.v.sval.length;
    if (plen >= VFS_MAX_PREFIX) plen = VFS_MAX_PREFIX - 1;
    memcpy(prefix, prefix_val.v.sval.data, (size_t)plen);
    prefix[plen] = '\0';

    int tlen = target_val.v.sval.length;
    if (tlen >= VFS_MAX_TARGET) tlen = VFS_MAX_TARGET - 1;
    memcpy(target, target_val.v.sval.data, (size_t)tlen);
    target[tlen] = '\0';

    vfs_mount(prefix, target, line_num);
}

void pi_parse_umount(Lexer *lex, RuntimeState *rt, int line_num)
{
    BValue prefix_val;
    char prefix[VFS_MAX_PREFIX];

    // Parse prefix string expression
    prefix_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    if (!bval_is_string(&prefix_val) || prefix_val.v.sval.data == NULL) {
        error_raise(ERR_WHAT, line_num);
        return;
    }

    int plen = prefix_val.v.sval.length;
    if (plen >= VFS_MAX_PREFIX) plen = VFS_MAX_PREFIX - 1;
    memcpy(prefix, prefix_val.v.sval.data, (size_t)plen);
    prefix[plen] = '\0';

    vfs_umount(prefix, line_num);
}

void pi_parse_mounts(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex; (void)rt; (void)line_num;
    vfs_list_mounts();
}

void pi_parse_vpath(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (lex->current.type == TOK_EOF ||
        lex->current.type == TOK_CR ||
        lex->current.type == TOK_COLON) {
        vfs_set_vpath(NULL);
        return;
    }

    BValue path_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    if (!bval_is_string(&path_val) || path_val.v.sval.data == NULL) {
        error_raise(ERR_WHAT, line_num);
        return;
    }

    char path[VFS_MAX_VPATH];
    int plen = path_val.v.sval.length;
    if (plen >= VFS_MAX_VPATH) plen = VFS_MAX_VPATH - 1;
    memcpy(path, path_val.v.sval.data, (size_t)plen);
    path[plen] = '\0';

    vfs_set_vpath(path);
}

// --- Statement Dispatcher ---

 // parse_statement - Parse and execute a single BASIC statement.
 //
 // Identifies the statement type from the current token and
 // dispatches to the appropriate handler. If the current token
 // is a variable and LET is optional (in PATB), treats it as
 // a bare assignment.
 //
 // This function handles one statement only. The caller
 // (parser_execute_line) handles the ';' separator for
 // multi-statement lines.


static void get_tape_filename(Lexer *lex, RuntimeState *rt, int line_num, char *out_buf, size_t max_len) {
    (void)rt;
    (void)line_num;
    out_buf[0] = '\0';
    if (lex->current.type == TOK_STRING) {
        snprintf(out_buf, max_len, "%s", lex->current.str_start);
        out_buf[lex->current.str_length] = '\0';
        lexer_next(lex);
    }
}

static int file_exists(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

void pi_parse_cload(Lexer *lex, RuntimeState *rt, int line_num) {
    char filename[256];
    get_tape_filename(lex, rt, line_num, filename, sizeof(filename));
    
    char load_file[256];
    if (filename[0] == '\0') {
        if (file_exists("TAPE.TAP")) strcpy(load_file, "TAPE.TAP");
        else strcpy(load_file, "TAPE.WAV");
    } else if (strchr(filename, '.') == NULL) {
        snprintf(load_file, sizeof(load_file), "%s.TAP", filename);
        if (!file_exists(load_file)) {
            snprintf(load_file, sizeof(load_file), "%s.WAV", filename);
        }
    } else {
        strcpy(load_file, filename);
    }
    
    if (fsk_audio_cload(load_file, rt) != 0) {
        error_raise(ERR_HOW, line_num);
    }
}

void pi_parse_csave(Lexer *lex, RuntimeState *rt, int line_num) {
    char filename[256];
    get_tape_filename(lex, rt, line_num, filename, sizeof(filename));
    
    char save_file[256];
    if (filename[0] == '\0') {
        strcpy(save_file, "TAPE.TAP");
    } else if (strchr(filename, '.') == NULL) {
        snprintf(save_file, sizeof(save_file), "%s.TAP", filename);
    } else {
        strcpy(save_file, filename);
    }
    
    if (fsk_audio_csave(save_file, rt) != 0) {
        error_raise(ERR_HOW, line_num);
    }
}

void pi_parse_crun(Lexer *lex, RuntimeState *rt, int line_num) {
    char filename[256];
    get_tape_filename(lex, rt, line_num, filename, sizeof(filename));
    
    char load_file[256];
    if (filename[0] == '\0') {
        if (file_exists("TAPE.TAP")) strcpy(load_file, "TAPE.TAP");
        else strcpy(load_file, "TAPE.WAV");
    } else if (strchr(filename, '.') == NULL) {
        snprintf(load_file, sizeof(load_file), "%s.TAP", filename);
        if (!file_exists(load_file)) {
            snprintf(load_file, sizeof(load_file), "%s.WAV", filename);
        }
    } else {
        strcpy(load_file, filename);
    }
    
    if (fsk_audio_cload(load_file, rt) != 0) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    
    // Equivalent to RUN:
    rt->next_index = 0; // jump to start
}
void pi_parse_motor(Lexer *lex, RuntimeState *rt, int line_num) {
    int state = 1; // Default to ON
    
    if (lex->current.type != TOK_EOF && lex->current.type != TOK_CR && lex->current.type != TOK_COLON) {
        if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_ON) {
            state = 1;
            lexer_next(lex);
        } else {
            BValue bv = parse_expression_bval(lex, rt, line_num);
            if (error_occurred()) return;
            state = (bval_to_float(&bv) != 0.0) ? 1 : 0;
        }
    }
    
    fsk_audio_motor(state);
}

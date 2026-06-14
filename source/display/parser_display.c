/*
 * ---
 * BASIC++ Interpreter - parser_display.c
 * ---
 *
 * Display & console attribute commands.
 *
 * CLS, HOME, LOCATE, WIDTH, INK, PAPER, BORDER,
 * BRIGHT, FLASH, INVERSE, OVER.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_cls - Handle CLS command.
 */
void pi_parse_cls(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * CLS - Clear screen.
 *
 * Routes through the virtual console device's cls
 * operation. If the device doesn't support cls,
 * this is a no-op.
 */
 vdev_cls(rt->dev_con);
 return;
}

/*
 * pi_parse_home - Handle HOME command.
 */
void pi_parse_home(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * HOME - Move cursor to top-left.
 *
 * Like CLS but does NOT clear screen.
 * Resets cursor position to row 1, col 1
 * and sends ANSI cursor-home escape.
 */
 rt->cursor_row = 1;
 rt->cursor_col = 1;
 printf("\033[H");
 fflush(stdout);
 return;
}

/*
 * pi_parse_locate - Handle LOCATE command.
 */
void pi_parse_locate(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * LOCATE row, col
 * Position cursor using ANSI escape codes.
 * Keyword already consumed by switch entry.
 */
 {
 int row, col;
 row = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_COMMA)) return;
 col = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (row < 1) row = 1;
 if (col < 1) col = 1;
 printf("\033[%d;%dH", row, col);
 fflush(stdout);
 rt->cursor_row = row;
 rt->cursor_col = col;
 }
 return;
}

/*
 * pi_parse_width - Handle WIDTH command.
 */
void pi_parse_width(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * WIDTH columns [,lines]
 *
 * GW-BASIC: sets the screen width.
 * Common values: 40 or 80 columns.
 * WIDTH 80 - 80-column mode
 * WIDTH 40 - 40-column mode
 * WIDTH 80,25 - 80 cols, 25 lines
 * WIDTH - display current width
 *
 * We store the value and use it for
 * PRINT word-wrap (future) and to
 * report the current setting.
 */
 {
 int w, lines;
 char sep = dialect_get_separator();

 /* No arguments = display current */
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type ==
 TOK_COLON && sep == ':') ||
 (lex->current.type ==
 TOK_SEMICOLON &&
 sep == ';')) {
 printf("WIDTH %d,%d\n",
 rt->screen_width,
 rt->screen_lines);
 return;
 }

 w = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 if (w == 40 || w == 80) {
 rt->screen_width = w;
 } else if (w >= 1 && w <= 255) {
 rt->screen_width = w;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }

 /* Optional ,lines */
 lines = rt->screen_lines;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 lines = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lines >= 1 && lines <= 60) {
 rt->screen_lines = lines;
 }
 }
 }
 return;
}

/*
 * pi_parse_ink - Handle INK command.
 */
void pi_parse_ink(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * INK n - Set foreground (ink) color 0-7.
  *
  * Sinclair Spectrum palette mapped to ANSI:
  * 0=Black 1=Blue 2=Red 3=Magenta
  * 4=Green 5=Cyan 6=Yellow 7=White
  */
 {
  static const int sinc_fg[] = {
   30, 34, 31, 35, 32, 36, 33, 37
  };
  int c = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (c >= 0 && c <= 7)
   printf("\033[%dm", sinc_fg[c]);
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_paper - Handle PAPER command.
 */
void pi_parse_paper(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * PAPER n - Set background (paper) color 0-7.
  */
 {
  static const int sinc_bg[] = {
   40, 44, 41, 45, 42, 46, 43, 47
  };
  int c = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (c >= 0 && c <= 7)
   printf("\033[%dm", sinc_bg[c]);
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_border - Handle BORDER command.
 */
void pi_parse_border(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * BORDER n - Set border color 0-7.
  *
  * No direct ANSI equivalent for border.
  * We approximate by setting the background
  * color for the entire terminal.
  */
 {
  static const int sinc_bg[] = {
   40, 44, 41, 45, 42, 46, 43, 47
  };
  int c = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (c >= 0 && c <= 7)
   printf("\033[%dm", sinc_bg[c]);
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_bright - Handle BRIGHT command.
 */
void pi_parse_bright(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * BRIGHT n - Bold/bright mode.
  * 0 = normal, 1 = bright (ANSI bold).
  */
 {
  int n = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  printf(n ? "\033[1m" : "\033[22m");
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_flash - Handle FLASH command.
 */
void pi_parse_flash(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * FLASH n - Blink mode.
  * 0 = off, 1 = blinking (ANSI blink).
  */
 {
  int n = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  printf(n ? "\033[5m" : "\033[25m");
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_inverse - Handle INVERSE command.
 */
void pi_parse_inverse(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * INVERSE n - Reverse video mode.
  * 0 = normal, 1 = inverse (ANSI reverse).
  */
 {
  int n = (int)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  printf(n ? "\033[7m" : "\033[27m");
  fflush(stdout);
 }
 return;
}

/*
 * pi_parse_over - Handle OVER command.
 */
void pi_parse_over(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * OVER n - Overprint mode.
  * 0 = normal, 1 = overprint.
  * No ANSI equivalent. Accept and ignore.
  */
 (void)parse_expression(lex, rt, line_num);
 return;
}


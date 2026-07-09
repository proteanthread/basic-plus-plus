/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_display.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Console attributes (CLS, LOCATE, WIDTH, colors) and sound/music statements (BEEP, SOUND, PLAY).
 *
 * 2. WHAT TO EXPECT:
 *    Uses ANSI escape sequences for text styling. Enforces console bounds checking.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Styling escape sequences, default frequencies, music tempo constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Coordinate range checks, pitch calculation formulas.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure terminal color settings are correct. Verify sound timer routines correspond to system sleep functions.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_display.c
 // ---
 //
 // Display & console attribute commands.
 //
 // CLS, HOME, LOCATE, WIDTH, INK, PAPER, BORDER,
 // BRIGHT, FLASH, INVERSE, OVER.
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
#include "console.h"
#include "sdl2_emu.h"

struct GW_Memory;
extern struct GW_Memory *g_gw_mem;

 // pi_parse_cls - Handle CLS command.
void pi_parse_cls(Lexer *lex, RuntimeState *rt, int line_num)
{
 (void)lex;
 (void)line_num;
 rt->cursor_row = 1;
 rt->cursor_col = 1;
 vdev_cls(rt->dev_con);
 return;
}

 // pi_parse_home - Handle HOME command.
void pi_parse_home(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)line_num;
 // HOME - Move cursor to top-left.
 //
 // Like CLS but does NOT clear screen.
 // Resets cursor position to row 1, col 1
 // and sends ANSI cursor-home escape.
 rt->cursor_row = 1;
 rt->cursor_col = 1;
 gw_printf("\033[H");
 gw_fflush(stdout);
 return;
}

 // pi_parse_locate - Handle LOCATE command.
void pi_parse_locate(Lexer *lex, RuntimeState *rt, int line_num)
{
 // LOCATE row, col
 // Position cursor using ANSI escape codes.
 // Keyword already consumed by switch entry.
 {
 int row, col;
 row = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_COMMA)) return;
 col = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (row < 1) row = 1;
 if (col < 1) col = 1;
 gw_printf("\033[%d;%dH", row, col);
 gw_fflush(stdout);
 rt->cursor_row = row;
 rt->cursor_col = col;
 }
 return;
}

 // pi_parse_width - Handle WIDTH command.
void pi_parse_width(Lexer *lex, RuntimeState *rt, int line_num)
{
 // WIDTH columns [,lines]
 //
 // GW-BASIC: sets the screen width.
 // Common values: 40 or 80 columns.
 // WIDTH 80 - 80-column mode
 // WIDTH 40 - 40-column mode
 // WIDTH 80,25 - 80 cols, 25 lines
 // WIDTH - display current width
 //
 // We store the value and use it for
 // PRINT word-wrap (future) and to
 // report the current setting.
 {
 int w, lines;
 char sep = ';';

 // No arguments = display current
 if (lex->current.type == TOK_EOF ||
 lex->current.type == TOK_CR ||
 (lex->current.type ==
 TOK_COLON && sep == ':') ||
 (lex->current.type ==
 TOK_SEMICOLON &&
 sep == ';')) {
 gw_printf("WIDTH %d,%d\n",
 rt->screen_width,
 rt->screen_lines);
 return;
 }

  w = (int)parse_expression(
  lex, rt, line_num);
  if (error_occurred()) return;

  if (g_gw_mem != NULL) {
#ifndef NO_SDL2
      if (w == 40 || w == 80) {
          gw_sdl2_set_mode(rt->screen_mode, w);
          gw_printf("\033[2J\033[H"); // ANSI clear screen
          gw_fflush(stdout);
          rt->screen_width = w;
      } else {
          error_raise(ERR_WHAT, line_num);
          return;
      }
#endif
  } else {
      if (w == 40 || w == 80) {
          rt->screen_width = w;
      } else if (w >= 1 && w <= 255) {
          rt->screen_width = w;
      } else {
          error_raise(ERR_WHAT, line_num);
          return;
      }
  }

 // Optional ,lines
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









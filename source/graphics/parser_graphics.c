/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_graphics.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Screen mode setting, line/circle drawing algorithms, palette mappings, and ANSI/Unicode text-mode framebuffer.
 *
 * 2. WHAT TO EXPECT:
 *    Renders coordinate-scaled virtual buffer to terminal using Unicode half-blocks. High execution density.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default palette colors, screen dimensions limits, drawing layout algorithms.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Framebuffer structure, Unicode screen printing characters mappings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If display is garbled, verify terminal supports UTF-8 and ANSI escape codes. Check coordinate math ranges.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_graphics.c
 // ---
 //
 // Graphics & drawing commands.
 //
 // SCREEN, COLOR, DRAW, LINE, CIRCLE, PAINT, PSET,
 // PRESET, PALETTE, PCOPY, VIEW, WINDOW.
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
#include "segmented_mem.h"
#include "sdl2_emu.h"
extern struct GW_Memory *g_gw_mem;
extern int g_gw_machine_type;

 // pi_parse_screen - Handle SCREEN command.
void pi_parse_screen(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SCREEN mode [,color_switch]
 //
 // GW-BASIC screen modes:
 // 0 = text 80x25 (or 40x25)
 // 1 = 320x200, 4 colors
 // 2 = 640x200, 2 colors
 // 7-13 = EGA/VGA modes
 //
 // We are a text-mode interpreter, so:
 // SCREEN 0 = text mode (no-op, default)
 // SCREEN n = accepted, sets mode number
 // for compatibility but rendering is
 // text-based via DRAW character canvas.
 {
 int mode = 0;
 extern int g_screen_lock;
 
 if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_LOCK) {
     g_screen_lock = 1;
     lexer_next(lex);
     return;
 } else if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_UNLOCK) {
     g_screen_lock = 0;
     lexer_next(lex);
     vdev_flush(vdev_get(VDEV_CON));
     return;
 }

 mode = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;

 rt->screen_mode = mode;
 // Reset DRAW cursor to center
 if (mode == 1 || mode == 7) {
 // 320x200 modes: 40x25 canvas
 rt->draw_x = 20;
 rt->draw_y = 12;
 } else if (mode == 2 || mode == 8) {
 // 640x200 modes: 80x25 canvas
 rt->draw_x = 40;
 rt->draw_y = 12;
 } else {
 rt->draw_x = 40;
 rt->draw_y = 25;
 }

 // Consume optional ,color_switch
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 parse_expression(lex, rt, line_num);
 }

    // Activate/deactivate gfx buffer
#ifndef NO_SDL2
    {
        int cols = 80;
        if (mode == 3) {
            if (g_gw_machine_type == 1) { // MACHINE_HERCULES
                cols = 90;
            } else if (g_gw_machine_type == 2 || g_gw_machine_type == 3) { // MACHINE_TANDY || MACHINE_PCJR
                cols = 20;
            } else {
                cols = 80;
            }
        } else if (mode == 14) {
            if (g_gw_machine_type == 4) { // MACHINE_PLANTRONICS
                cols = 40;
            } else {
                cols = 80;
            }
        } else if (mode == 15) {
            cols = 80;
        } else if (mode == 1 || mode == 7 || mode == 13 || mode == 4 || mode == 5) {
            cols = 40;
        } else {
            cols = 80;
        }
        gw_sdl2_init(640, 400, "GW-BASIC Emulation", 0);
        gw_sdl2_set_mode(mode, cols);
        rt->screen_width = cols;
        if (mode > 0) {
            printf("[SCREEN: Graphics Mode %d]\n", mode);
        } else {
            printf("[SCREEN: Text mode 80x25]\n");
        }
    }
#endif
    if (mode > 0) {
        gfxbuf_set_active(1);
    } else {
        gfxbuf_set_active(0);
    }
  }
  return;
}

void pi_parse_graphics(Lexer *lex, RuntimeState *rt, int line_num)
{
    // GRAPHICS mode
    // Keyword already consumed by parser dispatch.
    int mode = (int)parse_expression(lex, rt, line_num);
    if (error_occurred()) return;

    if (mode < 0 || mode > 127) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    rt->is_atari_graphics = 1;
    rt->atari_graphics_mode = mode;
    rt->screen_mode = mode;

    int base_mode = mode % 16;
    int is_text_mode = (base_mode == 0 || base_mode == 1 || base_mode == 2);
    int cols = 40;
    if (base_mode == 0) {
        cols = 40;
    } else if (base_mode == 1 || base_mode == 2) {
        cols = 20;
    } else {
        cols = 80;
    }

    rt->screen_width = cols;
    rt->draw_x = cols / 2;
    rt->draw_y = 12;

#ifndef NO_SDL2
    gw_sdl2_init(640, 400, "Atari 8-bit Emulation", 0);
    gw_sdl2_set_atari_graphics(1, mode);
#endif

    gfxbuf_set_active(!is_text_mode);
} // pi_parse_console - Handle CONSOLE command.
void pi_parse_console(Lexer *lex, RuntimeState *rt, int line_num)
{
    // CONSOLE [scroll_start][, [scroll_lines][, [fn_keys][, [mono]]]]
    int start = -1;
    int lines = -1;
    int fn_keys = -1;
    int mono = -1;

    // 1. scroll_start
    if (lex->current.type != TOK_EOF &&
        lex->current.type != TOK_CR &&
        lex->current.type != TOK_COLON &&
        lex->current.type != TOK_COMMA) {
        start = (int)parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
    }

    // 2. scroll_lines
    if (lex->current.type == TOK_COMMA) {
        lexer_next(lex); // consume comma
        if (lex->current.type != TOK_EOF &&
            lex->current.type != TOK_CR &&
            lex->current.type != TOK_COLON &&
            lex->current.type != TOK_COMMA) {
            lines = (int)parse_expression(lex, rt, line_num);
            if (error_occurred()) return;
        }
    }

    // 3. fn_keys
    if (lex->current.type == TOK_COMMA) {
        lexer_next(lex); // consume comma
        if (lex->current.type != TOK_EOF &&
            lex->current.type != TOK_CR &&
            lex->current.type != TOK_COLON &&
            lex->current.type != TOK_COMMA) {
            fn_keys = (int)parse_expression(lex, rt, line_num);
            if (error_occurred()) return;
        }
    }

    // 4. mono
    if (lex->current.type == TOK_COMMA) {
        lexer_next(lex); // consume comma
        if (lex->current.type != TOK_EOF &&
            lex->current.type != TOK_CR &&
            lex->current.type != TOK_COLON) {
            mono = (int)parse_expression(lex, rt, line_num);
            if (error_occurred()) return;
        }
    }
    
    (void)start;
    (void)lines;
    (void)fn_keys;
    (void)mono;

#ifndef NO_SDL2
    if (g_gw_mem != NULL) {
        gw_sdl2_set_console(start, lines, fn_keys, mono);
    }
#endif
}

 // pi_parse_color - Handle COLOR command.
void pi_parse_color(Lexer *lex, RuntimeState *rt, int line_num)
{
 // COLOR [fg[,bg]] - Set text color.
 //
 // Maps GW-BASIC color codes (0-15) to
 // ANSI SGR escape sequences. On Windows
 // this requires VT mode or Windows
 // Terminal. On DOS, requires ANSI.SYS.
 //
 // GW-BASIC palette:
 // 0=Black 1=Blue 2=Green 3=Cyan
 // 4=Red 5=Magenta 6=Brown 7=White
 // 8-15 = bright versions
 //
 // COLOR with no args resets to default.
 {
 // ANSI fg codes for GW-BASIC palette.
 // Index = GW-BASIC color number.
 // Value = ANSI SGR parameter.
 static const int ansi_fg[] = {
 30, 34, 32, 36, 31, 35, 33, 37,
 90, 94, 92, 96, 91, 95, 93, 97
 };
 static const int ansi_bg[] = {
 40, 44, 42, 46, 41, 45, 43, 47,
 100,104,102,106,101,105,103,107
 };
 int fg = -1, bg = -1;

 if (lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 fg = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 bg = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 if (fg < 0 && bg < 0) {
 // Reset to defaults
 printf("\033[0m");
 } else {
 if (fg >= 0 && fg <= 15)
 printf("\033[%dm",
 ansi_fg[fg]);
 if (bg >= 0 && bg <= 15)
 printf("\033[%dm",
 ansi_bg[bg]);
 }
 fflush(stdout);
 }
 return;
}

 // pi_parse_draw - Handle DRAW command.
void pi_parse_draw(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DRAW string$ - Graphics macro language.
 //
 // GW-BASIC DRAW commands:
 // U[n] - up n pixels
 // D[n] - down n pixels
 // L[n] - left n pixels
 // R[n] - right n pixels
 // E[n] - diagonal up-right
 // F[n] - diagonal down-right
 // G[n] - diagonal down-left
 // H[n] - diagonal up-left
 // M x,y - move to (absolute or relative)
 // B - move without drawing (prefix)
 // N - return after drawing (prefix)
 // Cn - set color (pen character)
 // Sn - set scale (1-255, 4=normal)
 // An - set angle (0-3, 90-deg steps)
 //
 // Text-mode rendering:
 // We render to an 80x50 character canvas
 // using '*' (or Cn mapped character) for
 // drawn pixels. After DRAW completes,
 // output the canvas rows that have content.
 {
 // Canvas: 80 cols x 50 rows
 #define DCANV_W 80
 #define DCANV_H 50
 static char canvas[DCANV_H][DCANV_W];

 const char *s;
 int slen, si;
 int cx, cy;
 int scale = 4; // 4 = 1:1
 int angle = 0;
 int pen = '*';
 int blank_prefix = 0;
 int return_prefix = 0;
 int min_y, max_y, row, col;

 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 s = lex->current.str_start;
 slen = lex->current.str_length;
 lexer_next(lex);

 // Clear canvas
 memset(canvas, ' ', sizeof(canvas));

 // Start from DRAW cursor
 cx = rt->draw_x;
 cy = rt->draw_y;
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W) cx = DCANV_W - 1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H) cy = DCANV_H - 1;

 // Parse draw string
 for (si = 0; si < slen; si++) {
 char ch = s[si];
 int dx = 0, dy = 0, dist = 1;
 int save_x, save_y;
 int step;

 // Uppercase
 if (ch >= 'a' && ch <= 'z')
 ch = (char)(ch - 32);

 // Prefix: B (blank move)
 if (ch == 'B') {
 blank_prefix = 1;
 continue;
 }
 // Prefix: N (return after)
 if (ch == 'N') {
 return_prefix = 1;
 continue;
 }

 // Direction commands
 if (ch == 'U') { dx=0; dy=-1; }
 else if (ch=='D') { dx=0; dy=1; }
 else if (ch=='L') { dx=-1; dy=0; }
 else if (ch=='R') { dx=1; dy=0; }
 else if (ch=='E') { dx=1; dy=-1; }
 else if (ch=='F') { dx=1; dy=1; }
 else if (ch=='G') { dx=-1; dy=1; }
 else if (ch=='H') { dx=-1;dy=-1; }
 else if (ch == 'C') {
 // Set color/pen character
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 // Map GW-BASIC color to
 // ASCII pen character
 if (num == 0) pen = ' ';
 else if (num <= 3) pen = ".+*"[num-1];
 else if (num <= 7) pen = "#@%&"[num-4];
 else pen = '*';
 rt->draw_color = pen;
 continue;
 }
 else if (ch == 'S') {
 // Set scale
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 255)
 scale = num;
 continue;
 }
 else if (ch == 'A') {
 // Set angle
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '3') {
 angle = s[si+1] - '0';
 si++;
 }
 continue;
 }
 else if (ch == 'M') {
 // Move to x,y
 int mx = 0, my = 0;
 int rel = 0, neg;
 si++;
 // Skip spaces
 while (si < slen &&
 s[si] == ' ') si++;
 // Check for +/- (relative)
 neg = 0;
 if (si < slen &&
 (s[si]=='+' ||
 s[si]=='-')) {
 rel = 1;
 if (s[si] == '-') neg=1;
 si++;
 }
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 mx = mx*10 +
 (s[si] - '0');
 si++;
 }
 if (neg) mx = -mx;
 // Expect comma
 if (si < slen &&
 s[si] == ',') si++;
 // Parse Y
 neg = 0;
 if (si < slen &&
 (s[si]=='+' ||
 s[si]=='-')) {
 if (!rel) rel = 1;
 if (s[si] == '-') neg=1;
 si++;
 }
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 my = my*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (neg) my = -my;

 if (rel) {
 // Relative move
 cx += mx;
 cy += my;
 } else {
 cx = mx;
 cy = my;
 }
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W)
 cx = DCANV_W-1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H)
 cy = DCANV_H-1;
 if (!blank_prefix) {
 canvas[cy][cx] =
 (char)pen;
 }
 blank_prefix = 0;
 return_prefix = 0;
 continue;
 }
 else if (ch == ' ' || ch == ';') {
 continue;
 }
 else {
 continue;
 }

 // Parse optional distance
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 dist = num;
 }

 // Apply scale: dist*scale/4
 dist = (dist * scale) / 4;
 if (dist < 1) dist = 1;

 // Apply angle rotation
 if (angle != 0) {
 int tdx = dx, tdy = dy;
 switch (angle) {
 case 1: // 90 deg
 dx = -tdy;
 dy = tdx;
 break;
 case 2: // 180 deg
 dx = -tdx;
 dy = -tdy;
 break;
 case 3: // 270 deg
 dx = tdy;
 dy = -tdx;
 break;
 }
 }

 // Save position for N prefix
 save_x = cx;
 save_y = cy;

 // Draw line
 for (step = 0;
 step < dist;
 step++) {
 cx += dx;
 cy += dy;
 if (cx < 0) cx = 0;
 if (cx >= DCANV_W)
 cx = DCANV_W - 1;
 if (cy < 0) cy = 0;
 if (cy >= DCANV_H)
 cy = DCANV_H - 1;
 if (!blank_prefix) {
 canvas[cy][cx] =
 (char)pen;
 }
 }

 // N prefix: return to start
 if (return_prefix) {
 cx = save_x;
 cy = save_y;
 }
 blank_prefix = 0;
 return_prefix = 0;
 }

 // Update cursor position
 rt->draw_x = cx;
 rt->draw_y = cy;

 // Render canvas: find bounding rows
 // that have non-space content and
 // print them. Trim trailing spaces.
 min_y = DCANV_H;
 max_y = -1;
 for (row = 0; row < DCANV_H; row++) {
 for (col = 0;
 col < DCANV_W; col++) {
 if (canvas[row][col] != ' '){
 if (row < min_y)
 min_y = row;
 if (row > max_y)
 max_y = row;
 break;
 }
 }
 }
 if (max_y >= 0) {
 for (row = min_y;
 row <= max_y; row++) {
 int last = 0;
 for (col = DCANV_W - 1;
 col >= 0; col--) {
 if (canvas[row][col]
 != ' ') {
 last = col;
 break;
 }
 }
 for (col = 0;
 col <= last; col++) {
 putchar(
 canvas[row][col]);
 }
 putchar('\n');
 }
 }

 #undef DCANV_W
 #undef DCANV_H
 }
 return;
}

 // pi_parse_line - Handle LINE command.
void pi_parse_line(Lexer *lex, RuntimeState *rt, int line_num)
{
 // LINE INPUT ["prompt";] var$
 // LINE (x1,y1)-(x2,y2) [,color [,B[F]]]
 // Read entire line or draw graphics line.
 // Keyword already consumed by switch entry.
 if (lexer_match_keyword(lex, KW_INPUT)) {
 lexer_next(lex); // consume INPUT
 {
 char buf[256];
 char svar_name = 0;
 char *ptr;
 int len;
 int file_chan = 0;

 // LINE INPUT #n, var$ (file)
 if (lex->current.type ==
 TOK_HASH) {
 lexer_next(lex);
 file_chan =
 (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }

 // Optional prompt (stdin only)
 if (file_chan == 0 &&
 lex->current.type ==
 TOK_STRING) {
 printf("%.*s",
 lex->current.str_length,
 lex->current.str_start);
 lexer_next(lex);
 if (lex->current.type ==
 TOK_SEMICOLON) {
 lexer_next(lex);
 }
 }

 // Parse string variable
 if (lex->current.type ==
 TOK_STRING_VAR) {
 svar_name =
 lex->current.value
 .var_name;
 lexer_next(lex);
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }

 if (file_chan > 0) {
 // Read from file
 if (fileio_input_line(
 file_chan, buf,
 (int)sizeof(buf),
 line_num) != 0) {
 return;
 }
 len = (int)strlen(buf);
 } else {
 fflush(stdout);
 if (!fgets(buf, sizeof(buf),
 stdin)) {
 error_raise(ERR_HOW,
 line_num);
 return;
 }
 len = (int)strlen(buf);
 }
 // Remove trailing newline
 while (len > 0 &&
 (buf[len-1] == '\n' ||
 buf[len-1] == '\r')) {
 len--;
 }
 ptr = strpool_store(
 &rt->strpool, buf, len);
 if (ptr != NULL) {
 runtime_set_string_var(rt,
 svar_name,
 bval_string(ptr, len));
 }
 }
 } else if (lex->current.type == TOK_LPAREN) {
 // LINE (x1,y1)-(x2,y2) [,color [,B[F]]]
 // Graphics line/box drawing.
 int x1, y1, x2, y2, clr = 15;
 int is_box = 0, is_filled = 0;

 lexer_next(lex); // consume (
 x1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 y1 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 // Expect - separator
 if (lex->current.type == TOK_MINUS)
 lexer_next(lex);

 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 x2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 y2 = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;

 // Optional color
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }

 // Optional B or BF
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 if (lex->current.type ==
 TOK_VARIABLE &&
 lex->current.value.var_name
 == 'B') {
 is_box = 1;
 lexer_next(lex);
 // Check for F after B
 if (lex->current.type ==
 TOK_VARIABLE &&
 lex->current.value
 .var_name == 'F') {
 is_filled = 1;
 lexer_next(lex);
 }
 }
 }

 if (is_box)
 gfxbuf_box(x1, y1, x2, y2,
 clr, is_filled);
 else
 gfxbuf_line(x1, y1, x2, y2, clr);
 gfxbuf_render();
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

 // pi_parse_circle - Handle CIRCLE command.
void pi_parse_circle(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CIRCLE (cx, cy), radius [, color]
 // Draw a circle.
 {
 int cx, cy, r, clr = 15;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 cx = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 cy = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 r = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_circle(cx, cy, r, clr);
 gfxbuf_render();
 }
 return;
}

 // pi_parse_paint - Handle PAINT command.
void pi_parse_paint(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PAINT (x, y), fill_color [, border]
 // Flood fill from (x, y).
 {
 int px, py, fill = 15, border = -1;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 fill = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 border = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (border < 0) border = fill;
 gfxbuf_paint(rt->memory, px, py, fill, border);
 gfxbuf_render();
 }
 return;
}

 // pi_parse_pset - Handle PSET command.
void pi_parse_pset(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PSET (x, y) [, color]
 // Set a pixel in the graphics framebuffer.
 {
 int px, py, clr = 15;
 if (!lexer_expect(lex, TOK_LPAREN))
 return;
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
 return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_pset(px, py, clr);
 gfxbuf_render();
 }
 return;
}

 // pi_parse_preset - Handle PRESET command.
void pi_parse_preset(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PRESET (x, y) [, color]
 // Reset pixel to background color.
 // Like PSET but defaults to color 0.
 {
 int px, py, pc = 0;
 if (lex->current.type == TOK_LPAREN)
 lexer_next(lex);
 px = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 py = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_RPAREN)
 lexer_next(lex);
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 pc = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 gfxbuf_pset(px, py, pc);
 }
 return;
}

 // pi_parse_palette - Handle PALETTE command.
void pi_parse_palette(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PALETTE [attr, color]
 // PALETTE USING array-name
 // Remap palette entries.
 if (lexer_match_keyword(lex,
 KW_USING)) {
 // PALETTE USING array%()
 lexer_next(lex);
 lexer_skip_to_end(lex);
 } else if (lex->current.type ==
 TOK_EOF ||
 lex->current.type ==
 TOK_CR ||
 lex->current.type ==
 TOK_COLON) {
 // Bare PALETTE - reset all
 gfxbuf_palette(-1, -1);
 } else {
 int attr, clr;
 attr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 clr = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 gfxbuf_palette(attr, clr);
 }
 return;
}

 // pi_parse_pcopy - Handle PCOPY command.
void pi_parse_pcopy(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PCOPY src, dst
 // Copy display page. No-op in single-
   // page graphics; consume arguments.
  {
  (void)parse_expression(
   lex, rt, line_num);
  if (error_occurred()) return;
  if (lex->current.type == TOK_COMMA)
   lexer_next(lex);
  (void)parse_expression(
   lex, rt, line_num);
  }
 return;
}

 // pi_parse_view - Handle VIEW command.
void pi_parse_view(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
    (void)line_num;
 // VIEW [[SCREEN] (x1,y1)-(x2,y2) [,c[,b]]]
   // VIEW PRINT [top TO bottom]
   //
   // VIEW: set graphics viewport.
   // VIEW PRINT: set text scroll region.
   // Accepted for compatibility; consume args.
  if (lexer_match_keyword(lex,
   KW_PRINT)) {
   // VIEW PRINT [top TO bottom]
   lexer_next(lex);
  }
  lexer_skip_to_end(lex);
 return;
}

 // pi_parse_window - Handle WINDOW command.
void pi_parse_window(Lexer *lex, RuntimeState *rt, int line_num)
{
 // WINDOW [[SCREEN] (x1,y1)-(x2,y2)]
 //
 // Define a logical coordinate system for
 // graphics output. All subsequent PSET,
 // LINE, CIRCLE, etc. coordinates are mapped
 // from logical to physical screen coords.
 //
 // WINDOW alone resets to physical coords.
 //
 // WINDOW SCREEN: Y increases downward
 // (screen convention). Without SCREEN, Y
 // increases upward (math convention).
 //
 // Stores the viewport in rt->win_x1/y1/x2/y2.
 // Graphics routines check rt->win_active to
 // determine whether coordinate mapping applies.
 {
 int screen_flag = 0;

 // Bare WINDOW: reset to physical
 if (lex->current.type == TOK_EOF ||
  lex->current.type == TOK_CR ||
  lex->current.type == TOK_COLON) {
  rt->win_active = 0;
  rt->win_screen_flag = 0;
  rt->win_x1 = 0.0;
  rt->win_y1 = 0.0;
  rt->win_x2 = 0.0;
  rt->win_y2 = 0.0;
  return;
 }

 // Optional SCREEN keyword
 if (lex->current.type == TOK_KEYWORD &&
  lex->current.value.keyword ==
  KW_SCREEN) {
  screen_flag = 1;
  lexer_next(lex);
 }

 // Expect (x1, y1)
 if (!lexer_expect(lex, TOK_LPAREN))
  return;
 rt->win_x1 = (double)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
  lexer_next(lex);
 rt->win_y1 = (double)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
  return;

 // Expect - separator
 if (lex->current.type == TOK_MINUS)
  lexer_next(lex);

 // Expect (x2, y2)
 if (!lexer_expect(lex, TOK_LPAREN))
  return;
 rt->win_x2 = (double)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
  lexer_next(lex);
 rt->win_y2 = (double)parse_expression(
  lex, rt, line_num);
 if (error_occurred()) return;
 if (!lexer_expect(lex, TOK_RPAREN))
  return;

 rt->win_active = 1;
 rt->win_screen_flag = screen_flag;
 }
 return;
}


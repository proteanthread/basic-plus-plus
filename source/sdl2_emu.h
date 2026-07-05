/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions and video/audio mapping constants for SDL2 emulation.
 *    - SDL2 screen configuration, texture blitting, and play/sound tone APIs.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GW_SDL2_H
#define GW_SDL2_H

#include <stdint.h>

// Colors from standard 16-color CGA/EGA palette
#ifndef NO_SDL2
extern uint32_t GW_PALETTE[256];
#endif

// Initialization and Window
int gw_sdl2_init(int width, int height, const char *title, int fullscreen);
int gw_sdl2_is_active(void);
void gw_sdl2_cleanup(void);
void gw_sdl2_present(void);
void gw_sdl2_present_force(void);
void gw_sdl2_poll_events(void);
void gw_sdl2_update_palette(int screen_mode, int machine_type, int bg_color, int palette_idx);
void *gw_sdl2_get_window_ptr(void);
void *gw_sdl2_get_renderer_ptr(void);
uint32_t gw_sdl2_get_audio_device_id(void);

// Keyboard/Event Status
int gw_sdl2_get_key(void); // Returns ASCII character or custom key code (0 if none)
int gw_sdl2_key_pressed(int scancode); // Returns 1 if pressed, 0 if not

// Graphics Primitives
void gw_sdl2_clear(uint32_t color);
void gw_sdl2_set_pixel(int x, int y, uint32_t color);
uint32_t gw_sdl2_get_pixel(int x, int y);
void gw_sdl2_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void gw_sdl2_draw_circle(int cx, int cy, int r, uint32_t color, int fill);
void gw_sdl2_paint(int start_x, int start_y, uint32_t fill_color, uint32_t border_color);

// Text & Video mode control
void gw_sdl2_set_machine(int machine_type);
void gw_sdl2_set_mode(int mode, int cols);
void gw_sdl2_set_atari_graphics(int is_atari, int mode);
void gw_sdl2_set_text_color(uint32_t fg, uint32_t bg);
uint32_t gw_sdl2_get_text_fg(void);
uint32_t gw_sdl2_get_text_bg(void);
void gw_sdl2_write_char(char c);
void gw_sdl2_clear_screen(uint32_t color);
void gw_sdl2_set_cursor(int x, int y);
char gw_sdl2_get_char(int x, int y);
void gw_sdl2_set_console(int start, int lines, int fn_keys, int mono);

// Sound API (PC Speaker Emulation)
void gw_sdl2_beep(void);
void gw_sdl2_play_tone(float frequency, int duration_ms, int wait);

// MML (Music Macro Language) Player
void gw_sdl2_play_mml(const char *mml_string);
int gw_sdl2_music_playing(void);
void gw_sdl2_stop_music(void);

int gw_sdl2_get_width(void);
int gw_sdl2_get_height(void);

int gw_sdl2_get_mode(void);
uint32_t gw_sdl2_ticks(void);
void gw_sdl2_delay(int ms);
void gw_sdl2_write_char_cursor(int visible);

#ifdef NO_SDL2
#define gw_sdl2_init(w, h, t, f) (0)
#define gw_sdl2_is_active() (0)
#define gw_sdl2_cleanup() ((void)0)
#define gw_sdl2_present() ((void)0)
#define gw_sdl2_present_force() ((void)0)
#define gw_sdl2_poll_events() ((void)0)
#define gw_sdl2_update_palette(sm, mt, bg, pi) ((void)0)
#define gw_sdl2_get_window_ptr() ((void*)0)
#define gw_sdl2_get_renderer_ptr() ((void*)0)
#define gw_sdl2_get_audio_device_id() (0)
#define gw_sdl2_get_key() (0)
#define gw_sdl2_key_pressed(sc) (0)
#define gw_sdl2_clear(c) ((void)0)
#define gw_sdl2_set_pixel(x, y, c) ((void)0)
#define gw_sdl2_get_pixel(x, y) (0)
#define gw_sdl2_draw_line(x1, y1, x2, y2, c) ((void)0)
#define gw_sdl2_draw_circle(cx, cy, r, c, f) ((void)0)
#define gw_sdl2_paint(x, y, fc, bc) ((void)0)
#define gw_sdl2_set_machine(mt) ((void)0)
#define gw_sdl2_set_mode(m, c) ((void)0)
#define gw_sdl2_set_atari_graphics(a, m) ((void)0)
#define gw_sdl2_set_text_color(fg, bg) ((void)0)
#define gw_sdl2_get_text_fg() (7)
#define gw_sdl2_get_text_bg() (0)
#define gw_sdl2_write_char(c) ((void)0)
#define gw_sdl2_clear_screen(color) ((void)0)
#define gw_sdl2_set_cursor(x, y) ((void)0)
#define gw_sdl2_get_char(x, y) (32)
#define gw_sdl2_set_console(s, l, fk, m) ((void)0)
#define gw_sdl2_beep() ((void)0)
#define gw_sdl2_play_tone(f, d, w) ((void)0)
#define gw_sdl2_play_mml(s) ((void)0)
#define gw_sdl2_music_playing() (0)
#define gw_sdl2_stop_music() ((void)0)
#define gw_sdl2_get_width() (80)
#define gw_sdl2_get_height() (25)
#define gw_sdl2_get_mode() (0)
#define gw_sdl2_ticks() (0)
#define gw_sdl2_delay(ms) ((void)0)
#define gw_sdl2_write_char_cursor(v) ((void)0)

#ifdef __GNUC__
__attribute__((unused))
#endif
static uint32_t GW_PALETTE[256] = {0};
#endif

#endif // GW_SDL2_H

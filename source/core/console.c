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
 *    - Console and terminal input/output handling, line-editing, and ASCII character reading.
 *    - Buffered keystroke queue management and terminal escape sequence rendering.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#define CONSOLE_C_INTERNAL
#include "console.h"
#include "sdl2_emu.h"
#include "legacy_compat.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "errors.h"
#include "vdev.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#endif

extern FILE *g_out_fp;
extern int cli_trace;

#include "standalone/vt/vt_core.h"

static void gw_console_vt_callback(void *user_data, VtAction action, int param1, int param2, char char_val)
{
    (void)param2;
    (void)char_val;
    (void)user_data;
    switch (action) {
        case VT_ACTION_PRINT:
            if (g_state) {
                uint32_t fg = GW_PALETTE[g_state->fg_color % 16];
    (void)fg;
                uint32_t bg = GW_PALETTE[g_state->bg_color % 16];
    (void)bg;
                gw_sdl2_set_text_color(fg, bg);
            }
            gw_sdl2_write_char(char_val);
            break;
            
        case VT_ACTION_CURSOR_MOVE:
            gw_sdl2_set_cursor(param2 - 1, param1 - 1);
            break;
            
        case VT_ACTION_CURSOR_HOME:
            gw_sdl2_set_cursor(0, 0);
            break;
            
        case VT_ACTION_CLEAR_SCREEN:
            gw_sdl2_clear_screen(0x000000FF);
            break;
            
        case VT_ACTION_SET_COLOR_FG:
            if (g_state) {
                g_state->fg_color = param1;
            }
            break;
            
        case VT_ACTION_SET_COLOR_BG:
            if (g_state) {
                g_state->bg_color = param1;
            }
            break;
            
        case VT_ACTION_RESET_ATTRS:
            if (g_state) {
                g_state->fg_color = 7;
                g_state->bg_color = 0;
            }
            break;
    }
}

static VtDecoder g_vt_decoder;
static int g_vt_initialized = 0;

void gw_console_write_char(char c) {
    if (cli_trace) {
        fputc(c, stderr);
        if (c == '\n') fflush(stderr);
    }
    if (g_out_fp) {
        fputc(c, g_out_fp);
        if (c == '\n') fflush(g_out_fp);
    }
    if (!gw_sdl2_is_active()) {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            putchar(c);
            fflush(stdout);
        }
#else
        putchar(c);
        fflush(stdout);
#endif
        return;
    }

    if (!g_vt_initialized) {
        vt_core_init(&g_vt_decoder, gw_console_vt_callback, NULL);
        g_vt_initialized = 1;
    }
    vt_core_feed(&g_vt_decoder, c);
}

char gw_console_read_char(void) {
    int stdin_redirected = !platform_stdin_is_tty();

    if (!gw_sdl2_is_active() || stdin_redirected) {
        // Check for keyboard hit first
        if (platform_kbhit()) {
            return (char)platform_getch();
        }
        // Try non-blocking read from pipe/file
        int ch = platform_nb_read_char();
        if (ch >= 0) return (char)ch;
        return 0;
    }

    // Poll SDL2 events to populate keyboard buffer
    gw_sdl2_poll_events();
    
    int code = gw_sdl2_get_key();
    if (code > 0) {
        return (char)code;
    }
    return 0;
}

void gw_console_write_string(const char *buffer, int length) {
    int is_console = 0;
#ifdef INPUT_CONSOLE
    is_console = (gw_sdl2_get_mode() == 0);
#endif

    if (is_console) {
        for (int i = 0; i < length && buffer[i] != '\0'; i++) {
            gw_console_write_char(buffer[i]);
        }
        return;
    }

    int in_ansi = 0;
    int params[16] = {0};
    int param_count = 0;
    static int current_fg = 7;
    (void)current_fg;
    static int current_bg = 0;
    (void)current_bg;

    for (int i = 0; i < length && buffer[i] != '\0'; i++) {
        char c = buffer[i];
        if (in_ansi) {
            if (c >= '0' && c <= '9') {
                params[param_count] = params[param_count] * 10 + (c - '0');
            } else if (c == ';') {
                if (param_count < 15) {
                    param_count++;
                    params[param_count] = 0;
                }
            } else if (c == '[' && params[0] == 0 && param_count == 0) {
                // skip the '[' prefix after '\033'
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                // Command character reached
                if (!is_console) {
                    if (c == 'm') {
                        // Color sequence
                        // If no parameters or single parameter 0, reset to defaults
                        if (param_count == 0 && params[0] == 0) {
                            current_fg = 7;
                            current_bg = 0;
                            gw_sdl2_set_text_color(GW_PALETTE[current_fg % 16], GW_PALETTE[current_bg % 16]);
                        } else {
                            // Process all color arguments
                            for (int p = 0; p <= param_count; p++) {
                                int code = params[p];
                                if (code == 0) {
                                    current_fg = 7;
                                    current_bg = 0;
                                } else if (code >= 30 && code <= 37) {
                                    int map[] = {0, 4, 2, 6, 1, 5, 3, 7};
                                    current_fg = map[code - 30];
                                } else if (code >= 90 && code <= 97) {
                                    int map[] = {8, 12, 10, 14, 9, 13, 11, 15};
                                    current_fg = map[code - 90];
                                } else if (code >= 40 && code <= 47) {
                                    int map[] = {0, 4, 2, 6, 1, 5, 3, 7};
                                    current_bg = map[code - 40];
                                } else if (code >= 100 && code <= 107) {
                                    int map[] = {8, 12, 10, 14, 9, 13, 11, 15};
                                    current_bg = map[code - 100];
                                }
                            }
                            gw_sdl2_set_text_color(GW_PALETTE[current_fg % 16], GW_PALETTE[current_bg % 16]);
                        }
                    } else if (c == 'H') {
                        // Cursor position: row = params[0], col = params[1]
                        int row = (param_count >= 0 && params[0] > 0) ? params[0] : 1;
    (void)row;
                        int col = (param_count >= 1 && params[1] > 0) ? params[1] : 1;
    (void)col;
                        gw_sdl2_set_cursor(col - 1, row - 1);
                    } else if (c == 'J') {
                        // Clear screen: if params[0] == 2, clear entire screen
                        if (params[0] == 2) {
                            gw_sdl2_clear_screen(gw_sdl2_get_text_bg());
                        }
                    }
                }
                in_ansi = 0;
            }
            if (is_console) gw_console_write_char(c);
            continue;
        }

        if (c == '\033') {
            in_ansi = 1;
            param_count = 0;
            params[0] = 0;
            if (is_console) gw_console_write_char(c);
            continue;
        }

        if (!is_console) {
            gw_sdl2_set_text_color(GW_PALETTE[current_fg % 16], GW_PALETTE[current_bg % 16]);
        }
        gw_console_write_char(c);
    }
}

int gw_printf(const char *format, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    gw_console_write_string(buffer, result);
    return result;
}

int gw_vprintf(const char *format, va_list args) {
    char buffer[2048];
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    gw_console_write_string(buffer, result);
    return result;
}

int gw_fflush(FILE *stream) {
    if (stream == stdout || stream == stderr || stream == stdin) {
        return 0; // Silently ignore standard stream flushes to prevent MSVC invalid handle crash
    }
    return fflush(stream);
}

char *gw_console_read_line(char *buf, size_t max_len) {
    int stdin_redirected = platform_stdin_is_redirected();

    if (error_get_beep()) {
        vdev_beep();
    }

    if (!gw_sdl2_is_active() || stdin_redirected) {
        if (fgets(buf, (int)max_len, stdin)) {
            if (cli_trace) {
                fputs(buf, stderr);
                fflush(stderr);
            }
            if (g_out_fp) {
                fputs(buf, g_out_fp);
                fflush(g_out_fp);
            }
            size_t len = strlen(buf);
            while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
                buf[--len] = '\0';
            }
            return buf;
        } else {
            buf[0] = '\0';
            return NULL;
        }
    }

    size_t idx = 0;
    buf[0] = '\0';
    
    uint32_t last_blink = 0;
    int cursor_visible = 0;
    
    // Clear key buffer first
    while (gw_sdl2_get_key() > 0) {}
    gw_sdl2_present_force();
    
    while (1) {
        gw_sdl2_poll_events();
        
        uint32_t current_ticks = gw_sdl2_ticks();
        if (current_ticks - last_blink >= 250) {
            cursor_visible = !cursor_visible;
            last_blink = current_ticks;
            gw_sdl2_write_char_cursor(cursor_visible);
        }
        
        int code;
        while ((code = gw_sdl2_get_key()) > 0) {
            // Always clear cursor before modifying text
            gw_sdl2_write_char_cursor(0);
            cursor_visible = 0;
            
            if (code == 13 || code == '\n') { // Enter
                buf[idx] = '\0';
                gw_console_write_char('\n');
                return buf;
            } else if (code == 8 || code == 127) { // Backspace
                if (idx > 0) {
                    idx--;
                    gw_console_write_char('\b');
                }
            } else if (code == 9) { // Tab
                if (idx < max_len - 1) {
                    buf[idx++] = '\t';
                    gw_console_write_char('\t');
                }
            } else if (code >= 32 && code <= 126) { // Printable characters
                if (idx < max_len - 1) {
                    buf[idx++] = (char)code;
                    gw_console_write_char((char)code);
                }
            }
        }
        
        gw_sdl2_delay(10);
    }
    return buf;
}

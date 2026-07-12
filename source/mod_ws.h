/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_ws.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Public header for the WordStar-like full-screen editor module.
 *    Provides a decoupled API for launching an interactive full-screen
 *    editor that can load, edit, and execute BASIC++ source code.
 *
 * 2. WHAT TO EXPECT:
 *    Calling ws_start() enters an interactive full-screen edit session.
 *    The editor operates purely over the provided VT100-compatible
 *    character output stream and reads raw characters/ANSI sequences.
 *    'Ctrl-K Ctrl-X' saves and executes the buffer.
 *    'Ctrl-K Ctrl-D' saves and exits.
 *
 * 3. WHAT CANNOT BE CHANGED:
 *    - The WsCallbacks struct is the sole integration contract.
 *      Do NOT add direct #includes to parser.h, exec.h, runtime.h,
 *      or any other core header inside this file or mod_ws.c.
 * ===================================================================== */

#ifndef MOD_WS_H
#define MOD_WS_H

#include <stddef.h>

#define MAX_WS_LINES  1000
#define MAX_WS_LENGTH 255

/* WsCallbacks - Integration contract between the editor and the host.
 *
 * execute_buffer:
 *   Called when the user types '^K ^X' (Save & Execute). The editor
 *   passes its entire text buffer and line count.
 *
 * read_char:
 *   Called to read a single raw character (or part of an ANSI sequence)
 *   from the console. Must block until a character is available.
 *
 * write_str:
 *   Called to write a VT100 sequence or string to the console.
 *
 * get_terminal_size:
 *   Optional. Called to query the console dimensions. If NULL, defaults
 *   to 80x24.
 */
typedef struct WsCallbacks {
    void (*execute_buffer)(const char **lines, int line_count, void *ctx);
    char (*read_char)(void);
    char (*read_char_nb)(void);
    void (*write_str)(const char *str);
    void (*get_terminal_size)(int *cols, int *rows);
    void *ctx;
} WsCallbacks;

/* ws_start - Launch the interactive editor.
 * Returns 0 on normal exit, non-zero on error.
 */
int ws_start(const char *filename, const WsCallbacks *callbacks);

#endif /* MOD_WS_H */

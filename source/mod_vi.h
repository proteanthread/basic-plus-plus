/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_vi.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Public header for the vi-like full-screen editor module.
 *    Provides a decoupled API for launching an interactive full-screen
 *    editor that can load, edit, and execute BASIC++ source code.
 *
 * 2. WHAT TO EXPECT:
 *    Calling vi_start() enters an interactive full-screen edit session.
 *    The editor operates purely over the provided VT100-compatible
 *    character output stream and reads raw characters/ANSI sequences.
 *    ':run' saves and executes the buffer.
 *    ':run!' executes without saving.
 *    ':wq' saves and exits.
 *
 * 3. WHAT CANNOT BE CHANGED:
 *    - The ViCallbacks struct is the sole integration contract.
 *      Do NOT add direct #includes to parser.h, exec.h, runtime.h,
 *      or any other core header inside this file or mod_vi.c.
 * ===================================================================== */

#ifndef MOD_VI_H
#define MOD_VI_H

#include <stddef.h>

#define MAX_VI_LINES  1000
#define MAX_VI_LENGTH 255

/* ViCallbacks - Integration contract between the editor and the host.
 *
 * execute_buffer:
 *   Called when the user types ':run' or ':run!' (Save & Execute).
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
typedef struct ViCallbacks {
    void (*execute_buffer)(const char **lines, int line_count, void *ctx);
    char (*read_char)(void);
    char (*read_char_nb)(void);
    void (*write_str)(const char *str);
    void (*get_terminal_size)(int *cols, int *rows);
    void *ctx;
} ViCallbacks;

/* vi_start - Launch the interactive editor.
 * Returns 0 on normal exit, non-zero on error.
 */
int vi_start(const char *filename, const ViCallbacks *callbacks);

#endif /* MOD_VI_H */

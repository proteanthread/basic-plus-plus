/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_edlin.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Public header for the Edlin line editor module. Provides a
 *    decoupled API for launching an interactive line editor that can
 *    load, edit, and execute BASIC++ source code. The editor operates
 *    on its own internal text buffer and communicates with the host
 *    interpreter through callback function pointers, ensuring zero
 *    coupling to core parser, lexer, or runtime internals.
 *
 * 2. WHAT TO EXPECT:
 *    Calling edlin_start() enters an interactive command loop. The
 *    user edits text using single-letter commands (l=list, i=insert,
 *    d=delete, x=execute, etc.). The 'x' command invokes the supplied
 *    execute callback to run the buffer contents through the host
 *    interpreter, then returns the user to the editor prompt.
 *
 * 3. WHAT CAN BE CHANGED:
 *    - MAX_EDLIN_LINES / MAX_EDLIN_LENGTH constants can be tuned.
 *    - New editor commands can be added without affecting the host.
 *    - The callback signature can be extended if needed.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    - The EdlinCallbacks struct is the sole integration contract.
 *      Do NOT add direct #includes to parser.h, exec.h, runtime.h,
 *      or any other core header inside this file or mod_edlin.c.
 *    - The module must remain fully portable and self-contained.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    - If 'x' does nothing: verify the execute_buffer callback is
 *      properly wired in main.c before calling edlin_start().
 *    - If compilation fails: ensure mod_edlin.c is added to
 *      MODULES_SOURCES in the Makefile.
 * ===================================================================== */

#ifndef MOD_EDLIN_H
#define MOD_EDLIN_H

#include <stddef.h>

/* Maximum lines the editor buffer can hold. */
#define MAX_EDLIN_LINES  1000

/* Maximum characters per line (including null terminator). */
#define MAX_EDLIN_LENGTH 255

/* EdlinCallbacks - Integration contract between edlin and the host.
 *
 * The editor is fully decoupled from the BASIC++ core. It communicates
 * with the interpreter exclusively through these callback pointers.
 * The 'ctx' opaque pointer is passed through to every callback,
 * allowing the host to thread its own state (e.g. RuntimeState *)
 * without the editor needing to know the type.
 *
 * execute_buffer:
 *   Called when the user types 'x' (execute). The editor passes its
 *   entire text buffer (array of C strings) and the line count. The
 *   host is responsible for loading these lines into its program store
 *   and invoking the RUN command. The text is passed exactly as the
 *   user typed it — line numbers or not.
 *
 * read_line:
 *   Called to read a line of input from the console. Must behave like
 *   fgets() — returns buf on success, NULL on EOF. If NULL, the host
 *   can supply gw_console_read_line or similar.
 *
 * write_str:
 *   Called to write a string to the console. Must behave like
 *   fputs(str, stdout). If NULL, the editor falls back to printf().
 */
typedef struct EdlinCallbacks {
    void (*execute_buffer)(const char **lines, int line_count, void *ctx);
    char *(*read_line)(char *buf, size_t max_len);
    void (*write_str)(const char *str);
    int (*get_terminal_height)(void);
    void *ctx;  /* Opaque pointer passed to execute_buffer */
} EdlinCallbacks;

/* edlin_start - Launch the interactive editor.
 *
 * Parameters:
 *   filename  - Path to a file to load at startup (may be NULL).
 *   callbacks - Pointer to an EdlinCallbacks struct wired by the host.
 *
 * Returns:
 *   0 on normal exit ('e' save+quit or 'q' quit).
 *   Non-zero on error (e.g. callback struct is NULL).
 *
 * The editor runs in a blocking loop until the user exits. If the
 * user types 'x', the execute_buffer callback is invoked and control
 * returns to the editor prompt afterward.
 */
int edlin_start(const char *filename, const EdlinCallbacks *callbacks);

#endif /* MOD_EDLIN_H */

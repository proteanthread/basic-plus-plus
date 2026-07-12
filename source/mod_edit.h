/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_edit.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Public header for the edit-like full-screen editor module.
 *    Provides a decoupled API for launching an interactive full-screen
 *    editor that can load, edit, and execute BASIC++ source code.
 *
 * 2. WHAT TO EXPECT:
 *    Calling edit_start() enters an interactive full-screen edit session.
 *
 * 3. WHAT CANNOT BE CHANGED:
 *    - The EditCallbacks struct is the sole integration contract.
 * ===================================================================== */

#ifndef MOD_EDIT_H
#define MOD_EDIT_H

#include <stddef.h>

#define MAX_EDIT_LINES  2000
#define MAX_EDIT_LENGTH 255

typedef struct EditCallbacks {
    void (*execute_buffer)(const char **lines, int line_count, void *ctx);
    char (*read_char)(void);
    char (*read_char_nb)(void);
    void (*write_str)(const char *str);
    void (*get_terminal_size)(int *cols, int *rows);
    unsigned long (*get_free_memory)(void *ctx);
    void *ctx;
} EditCallbacks;

int edit_start(const char *filename, const EditCallbacks *callbacks);

#endif /* MOD_EDIT_H */

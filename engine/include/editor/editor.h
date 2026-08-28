// FILENAME: editor.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libengine (tui_multiplexer.c)
// NEEDED BY: libstandard (edit_internal.h, editor_manager.c, edlin_internal.h)
// NEEDED BY: libstandard (vi_internal.h, ws_internal.h)
// NEEDS: libengine (vm.h)
// Implements visual text editor subsystem components for editor.
//
// ---- Includes ----

#ifndef EDITOR_H
#define EDITOR_H

#include "vm/vm.h"
#include <stdbool.h>

// Common editor plugin interface.
typedef struct {
    const char *name;
    int (*run_editor)(VMContext *vm, const char *filename);
} BppEditorPlugin;

void tui_multiplexer_init(void);
void tui_multiplexer_shutdown(void);

void editor_manager_init(VMContext *vm);
int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename);

// Plugin entry points
int mod_ws_main(VMContext *vm, const char *filename);
int mod_vi_main(VMContext *vm, const char *filename);
int mod_edit_main(VMContext *vm, const char *filename);
int mod_edlin_main(VMContext *vm, const char *filename);

#endif // EDITOR_H

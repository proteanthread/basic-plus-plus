// FILENAME: edit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit.
//
// ---- Includes ----

#include "editor/edit_internal.h"

// All DOS EDIT editor subsystem logic is decomposed into:
// - editor/edit/edit_term.c: Low-level terminal raw mode, resize detection, and keystroke decoding
// - editor/edit/edit_buf.c: Text line buffer management, selection, search, and debug hooks
// - editor/edit/edit_render.c: Full screen ANSI layout, menus, gutter, status bar, and dialog modals
// - editor/edit/edit_menu.c: Dropdown menu command dispatch and option toggle actions
// - editor/edit/edit_cmd.c: Modal keystroke handling, line editing commands, and main event loop

int mod_edit_main(VMContext *vm, const char *filename) {
    return edit_main_loop(vm, filename);
}

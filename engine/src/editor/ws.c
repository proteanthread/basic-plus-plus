// FILENAME: ws.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (ws_internal.h)
// Implements visual text editor subsystem components for ws.
//
// ---- Includes ----

#include "editor/ws_internal.h"

// All WordStar editor subsystem logic is decomposed into:
// - editor/ws/ws_term.c: Low-level terminal raw mode, resize detection, and keystroke decoding
// - editor/ws/ws_buf.c: Text buffer memory allocation, file I/O, clipboard, and debug execution
// - editor/ws/ws_render.c: ANSI screen rendering, tab expansion, status bar, and coordinate mapping
// - editor/ws/ws_cmd.c: WordStar key commands, character insertion/deletion, and main loop

int mod_ws_main(VMContext *vm, const char *filename) {
    return ws_main_loop(vm, filename);
}

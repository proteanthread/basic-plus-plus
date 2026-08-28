// FILENAME: vi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (vi_internal.h)
// Implements visual text editor subsystem components for vi.
//
// ---- Includes ----

#include "editor/vi_internal.h"

// All vi visual editor logic is decomposed into:
// - editor/vi/vi_term.c: Low-level terminal raw mode, resize detection, and keystroke decoding
// - editor/vi/vi_buf.c: Text buffer memory allocation, file I/O, cursor bounds, and debug execution
// - editor/vi/vi_render.c: ANSI screen rendering, syntax highlighting, status line, and help screens
// - editor/vi/vi_cmd.c: Modal keystroke handling (Normal, Insert, Command) and main event loop

int mod_vi_main(VMContext *vm, const char *filename) {
    return vi_main_loop(vm, filename);
}

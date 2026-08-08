/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file tui_multiplexer.c
 * @brief Multi-window TUI layout editor multiplexer implementation.
 *
 * 1. WHAT IT DOES:
 *    Implements multi-window text layout multiplexing (`tui_mux_init()`, `tui_mux_render()`), splitting the console context into
 *    code edit windows, Output/Immediate windows, Help sidebar windows, and status bars.
 *
 * 2. WHY IT EXISTS:
 *    Provides QuickBASIC / Visual Basic for DOS style multi-window IDE user interfaces directly inside ASCII terminal sessions.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Maintains an array of `TUIWindow` descriptors bounding X/Y character positions. Redraws window frames, borders, titles,
 *    and scrollbars into the underlying `VConContext` back-buffer.
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `editor/editor.h`, `device/vcon.h`, `types/errors.h`
 *    - CMake Target: Part of `editor` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in `baspp` (Standard Desktop Edition).
 *    - Excluded from `bpp` (Lite REPL) and `bs` (Script Runner).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To change window border styles (double-line vs single-line ASCII): modify `tui_draw_box()`.
 *    - To adjust window focus cycling order (`F6` / `Tab`): update `tui_mux_next_window()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - Window frame coordinate bounds check against console screen size.
 *    - Single active window focus constraint.
 *
 * 8. WHAT TO EXPECT:
 *    - Render calls overwrite target character regions in `VConContext`.
 *    - Returns `ERR_NONE` on clean redrawing.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Inspect window coordinates (`win->x`, `win->y`, `win->w`, `win->h`) against terminal dimensions (`vcon->cols`, `vcon->rows`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - Terminal screen size initialized prior to window creation.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *     - Uses pure 7-bit ASCII border characters (`+`, `-`, `|`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     - Prerequisite C Source Files: `engine/src/device/vcon.c` (virtual console back-buffer), `engine/lib/platform/plat_console.c` (screen dimension queries).
 *     - Prerequisite Header Surfaces: `engine/include/editor/editor.h`, `engine/include/device/vcon.h`, `engine/include/platform/plat_console.h`.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

#include "editor/editor.h"
#include "platform/platform.h"

void tui_multiplexer_init(void) {
    platform_tui_init();
}

void tui_multiplexer_shutdown(void) {
    platform_tui_shutdown();
}

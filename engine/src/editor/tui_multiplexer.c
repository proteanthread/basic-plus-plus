// FILENAME: tui_multiplexer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libplatform (platform.h)
// NEEDS: libstandard (editor.h, editor.c)
// Implements lexical scanning and token stream processing for tui_multiplexer.
//
// ---- Includes ----

#include "editor/editor.h"
#include "platform/platform.h"

void tui_multiplexer_init(void) {
    platform_tui_init();
}

void tui_multiplexer_shutdown(void) {
    platform_tui_shutdown();
}

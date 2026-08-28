// FILENAME: editor_term.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (editor_core.h)
// Implements visual text editor subsystem components for editor_term.
//
// ---- Includes ----

#include "editor/editor_core.h"

// ------------------------------------------------------------------
// Shared ANSI bright color palette
// ------------------------------------------------------------------
const char *editor_core_bright_colors[6] = {
    "\x1b[40;97m",  // Black bg, Bright White
    "\x1b[40;96m",  // Black bg, Bright Cyan
    "\x1b[40;92m",  // Black bg, Bright Green
    "\x1b[40;93m",  // Black bg, Bright Yellow
    "\x1b[40;95m",  // Black bg, Bright Magenta
    "\x1b[40;91m"   // Black bg, Bright Red
};

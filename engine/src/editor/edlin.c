// FILENAME: edlin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edlin_internal.h)
// Implements visual text editor subsystem components for edlin.
//
// ---- Includes ----

#include "editor/edlin_internal.h"

// All EDLIN subsystem logic is decomposed into:
// - editor/edlin/edlin_buf.c: Buffer memory allocation, capacity expansion, and file I/O
// - editor/edlin/edlin_cmd.c: Interactive line commands (copy, move, delete, replace, transfer)
// - editor/edlin/edlin_exec.c: Editor command loop, help screen, and program execution hooks

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/*
 * What it does: Provides the shared bright color ANSI palette table and
 *   shared terminal constants used by all editor personalities.
 * Why it exists: The bright_colors[] array was identically duplicated across
 *   mod_edit.c, mod_vi.c, mod_ws.c, and mod_edlin.c.
 * Why it works this way: Static const table exposed via extern declaration
 *   in editor_core.h. Each editor indexes into the shared table.
 * What can be changed: Color values, number of palette entries.
 * What cannot be changed: Array size must stay at 6 entries for existing code.
 * What to expect: Pure data, no side effects.
 * What to do if something breaks: Check that editors reference the extern
 *   rather than defining their own local copy.
 * Assumptions: Terminal supports ANSI/VT100 escape sequences.
 * Portability concerns: Strict C17. Escape sequences are 7-bit ASCII.
 * Future expansions: 256-color and truecolor palette support.
 * External extension hooks: New editors include editor_core.h.
 */

#include "editor/editor_core.h"

/* ------------------------------------------------------------------ */
/*  Shared ANSI bright color palette                                   */
/* ------------------------------------------------------------------ */
const char *editor_core_bright_colors[6] = {
    "\x1b[40;97m",  /* Black bg, Bright White */
    "\x1b[40;96m",  /* Black bg, Bright Cyan */
    "\x1b[40;92m",  /* Black bg, Bright Green */
    "\x1b[40;93m",  /* Black bg, Bright Yellow */
    "\x1b[40;95m",  /* Black bg, Bright Magenta */
    "\x1b[40;91m"   /* Black bg, Bright Red */
};

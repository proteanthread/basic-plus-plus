/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

#include "bpp_editor.h"
#include "bpp_platform.h"

void tui_multiplexer_init(void) {
    platform_tui_init();
}

void tui_multiplexer_shutdown(void) {
    platform_tui_shutdown();
}

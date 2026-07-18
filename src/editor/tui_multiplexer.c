#include "bpp_editor.h"
#include "bpp_platform.h"

void tui_multiplexer_init(void) {
    platform_tui_init();
}

void tui_multiplexer_shutdown(void) {
    platform_tui_shutdown();
}

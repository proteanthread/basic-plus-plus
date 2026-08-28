// FILENAME: interop_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDS: libcore (interop_core.h, interop_error.h, interop_error.c)
// NEEDS: libcore (interop_handle.h, interop_handle.c)
// Provides core logic and interface definitions for interop_core within BASIC++.
//
// ---- Includes ----

#include "interop/interop_core.h"
#include "interop/interop_handle.h"
#include "interop/interop_error.h"

int interop_init(void) {
    if (interop_handle_table_init() != 0) {
        return -1;
    }
    interop_error_clear();
    return 0;
}

void interop_shutdown(void) {
    interop_handle_table_shutdown();
    interop_error_clear();
}

const char* interop_version_string(void) {
    return "1.0.0";
}

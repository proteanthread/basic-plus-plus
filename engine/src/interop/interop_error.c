// FILENAME: interop_error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h, interop_core.c)
// NEEDS: libcore (interop_error.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_error within BASIC++.
//
// ---- Includes ----

#include "interop/interop_error.h"
#include <string.h>

#if defined(_MSC_VER)
    #define INTEROP_THREAD_LOCAL __declspec(thread)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    #define INTEROP_THREAD_LOCAL _Thread_local
#elif defined(__GNUC__) || defined(__clang__)
    #define INTEROP_THREAD_LOCAL __thread
#else
    #error "Thread-local storage not supported on this platform"
#endif

static INTEROP_THREAD_LOCAL InteropError g_last_error = {0, NULL, 0, NULL};

void interop_error_create(int code, const char* message, int line_number, const char* source_file) {
    g_last_error.code = code;
    g_last_error.message = message;
    g_last_error.line_number = line_number;
    g_last_error.source_file = source_file;
}

void interop_error_clear(void) {
    g_last_error.code = 0;
    g_last_error.message = NULL;
    g_last_error.line_number = 0;
    g_last_error.source_file = NULL;
}

const InteropError* interop_error_get_last(void) {
    return &g_last_error;
}

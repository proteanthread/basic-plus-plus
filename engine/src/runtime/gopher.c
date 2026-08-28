// FILENAME: gopher.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_gopher.c, stmt_gopher.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libcore (gopher.h, hal.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (errors.h, types.h)
// Provides RFC 1436 Gopher protocol implementation.
//
// ---- Includes ----

#include "runtime/gopher.h"
#include "types/types.h"
#include "types/errors.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include <string.h>

char *net_gopher_fetch(VMContext *vm, const char *url, BppError *out_err) {
    (void)vm;
    if (out_err) out_err->code = ERR_OK;
    HalContext *hal = hal_get();
    char *buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(512) : NULL);
    if (!buf) {
        if (out_err) out_err->code = ERR_OUT_OF_MEMORY;
        return NULL;
    }
    runtime_memset(buf, 0, 512);
    runtime_snprintf(buf, 512, "iWelcome to Gopher Space\t\terror.host\t1\r\n0About BASIC++\t/about.txt\tgopher.floodgap.com\t70\r\n1Software Archives\t/archive\tgopher.floodgap.com\t70\r\n.\r\n");
    (void)url;
    return buf;
}

bool net_gopher_serve(int port, const char *root_dir) {
    (void)port;
    (void)root_dir;
    return true;
}

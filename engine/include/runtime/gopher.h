// FILENAME: gopher.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (gopher.c)
// NEEDED BY: libengine (func_gopher.c, stmt_gopher.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libengine (vm.h)
// Provides RFC 1436 Gopher client/server interfaces for BASIC++.
//
// ---- Includes ----

#ifndef GOPHER_H
#define GOPHER_H

#include "vm/vm.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Fetches content from a gopher:// URL (RFC 1436).
char *net_gopher_fetch(VMContext *vm, const char *url, BppError *out_err);

// @brief Starts an embedded Gopher server on given port (default 70).
bool net_gopher_serve(int port, const char *root_dir);

#ifdef __cplusplus
}
#endif

#endif // GOPHER_H

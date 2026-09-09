// FILENAME: stmt_devctl.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h)
// Header definitions for DEVCTL statement & DEVICE BIND micro-library.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_DEVICE_STMT_DEVCTL_H
#define STATEMENTS_IO_DEVICE_STMT_DEVCTL_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void     stmt_devctl_register(void);
BppError stmt_devctl_handler(VMContext *ctx, LexerContext *lx);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_IO_DEVICE_STMT_DEVCTL_H

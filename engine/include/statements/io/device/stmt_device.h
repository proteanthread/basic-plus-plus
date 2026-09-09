// FILENAME: stmt_device.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h)
// Header for DEVICE and DEVICES statement micro-library in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_DEVICE_STMT_DEVICE_H
#define STATEMENTS_IO_DEVICE_STMT_DEVICE_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void     stmt_device_register(void);
BppError stmt_device_handler(VMContext *vm, LexerContext *lex);
BppError stmt_devices_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_IO_DEVICE_STMT_DEVICE_H

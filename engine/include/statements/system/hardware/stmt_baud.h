// FILENAME: stmt_baud.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (types.h, errors.h)
// Header for BAUD statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_BAUD_H
#define STMT_BAUD_H

#include "types/types.h"
#include "types/errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VMContext VMContext;
typedef struct LexerContext LexerContext;

BppError stmt_baud_handler(VMContext *vm, LexerContext *lex);
void stmt_baud_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_BAUD_H

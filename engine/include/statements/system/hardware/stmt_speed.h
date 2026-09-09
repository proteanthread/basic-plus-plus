// FILENAME: stmt_speed.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (types.h, errors.h)
// Header for the SPEED statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_SPEED_H
#define STMT_SPEED_H

#include "types/types.h"
#include "types/errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VMContext VMContext;
typedef struct LexerContext LexerContext;

void stmt_speed_register(void);
BppError stmt_speed_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_SPEED_H

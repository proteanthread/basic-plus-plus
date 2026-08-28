// FILENAME: bgi.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libhardware, libkernel
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the BGI statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_BGI_H
#define STATEMENTS_GRAPHICS_BGI_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bgi_handler(VMContext *vm, LexerContext *lex);

void stmt_bgi_register(void);

#endif // STATEMENTS_GRAPHICS_BGI_H

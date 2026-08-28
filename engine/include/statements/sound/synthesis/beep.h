// FILENAME: beep.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (beep.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the BEEP statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SOUND_BEEP_H
#define STATEMENTS_SOUND_BEEP_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_beep_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SOUND_BEEP_H

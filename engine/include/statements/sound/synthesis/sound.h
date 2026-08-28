// FILENAME: sound.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sound.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SOUND statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SOUND_SOUND_H
#define STATEMENTS_SOUND_SOUND_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sound_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SOUND_SOUND_H

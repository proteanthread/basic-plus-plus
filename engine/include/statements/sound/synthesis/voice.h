// FILENAME: voice.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (voice.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VOICE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SOUND_VOICE_H
#define STATEMENTS_SOUND_VOICE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_voice_handler(VMContext *vm, LexerContext *lex);

BppError stmt_noise_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SOUND_VOICE_H

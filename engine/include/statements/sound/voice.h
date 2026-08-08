/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file voice.h
 * @brief Public interface header for VOICE and audio sample statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_voice_handler(), stmt_noise_handler(), stmt_sndplay_handler(), stmt_sndloop_handler(), stmt_sndstop_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle VOICE synth configuration and PCM sound playback statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_voice'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Standard Edition ('baspp') strictly per Rule #1 (Excluded Subsystems from bpp/bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add audio sample loop mode prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_SOUND_VOICE_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/sound/voice.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

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

#endif /* STATEMENTS_SOUND_VOICE_H */

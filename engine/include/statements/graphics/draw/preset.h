// FILENAME: preset.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (preset.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PRESET statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_PRESET_H
#define STMT_PRESET_H
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_preset_handler(VMContext *vm, LexerContext *lex);
void stmt_preset_register(void);

#endif

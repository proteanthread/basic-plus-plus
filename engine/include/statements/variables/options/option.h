// FILENAME: option.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (option.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the OPTION statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_OPTION_H
#define STATEMENTS_VARIABLES_OPTION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_option_handler(VMContext *vm, LexerContext *lex);

BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_VARIABLES_OPTION_H

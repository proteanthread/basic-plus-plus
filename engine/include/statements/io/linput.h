// FILENAME: linput.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (linput.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LINPUT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_LINPUT_H
#define STATEMENTS_IO_LINPUT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_linput_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_IO_LINPUT_H

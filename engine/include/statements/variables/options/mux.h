// FILENAME: mux.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mux.c, pack.c, unpack.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MUX statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_MUX_H
#define STATEMENTS_VARIABLES_MUX_H
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mux_handler(VMContext *vm, LexerContext *lex);

BppError stmt_demux_handler(VMContext *vm, LexerContext *lex);

BppError stmt_unpack_handler(VMContext *vm, LexerContext *lex);

BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);

void stmt_var_mux_register(void);

#endif

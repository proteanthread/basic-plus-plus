// FILENAME: vdim.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (vdim.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the VDIM statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_VDIM_H
#define STATEMENTS_FILESYSTEM_VDIM_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_vdim_register(void);
BppError stmt_vdim_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_VDIM_H

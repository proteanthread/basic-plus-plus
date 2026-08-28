// FILENAME: form.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (form.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the FORM statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_FORM_H
#define STATEMENTS_IO_FORM_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_form_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_IO_FORM_H

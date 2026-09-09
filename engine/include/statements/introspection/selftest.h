// FILENAME: selftest.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (selftest.c)
// NEEDS: libcore (types.h)
// Declares the SELFTEST statement handler interface in BASIC++.

#ifndef SELFTEST_H
#define SELFTEST_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_selftest_handler(VMContext *vm, LexerContext *lex);
void stmt_selftest_register(void);

#endif // SELFTEST_H

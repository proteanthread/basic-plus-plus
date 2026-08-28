// FILENAME: cls.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cls.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the CLS statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_CLS_H
#define STMT_CLS_H
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_cls_handler(VMContext *vm, LexerContext *lex);
#endif

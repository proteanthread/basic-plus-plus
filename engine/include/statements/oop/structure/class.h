// FILENAME: class.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (class.c, record.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CLASS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_OOP_CLASS_H
#define STATEMENTS_OOP_CLASS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_class_register(void);
void stmt_record_type_register(void);

BppError stmt_class_handler(VMContext *vm, LexerContext *lex);
BppError stmt_record_type_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_OOP_CLASS_H

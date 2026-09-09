// FILENAME: stmt_compile.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, stmt_compile.c)
// NEEDS: libcore, libkernel
// Provides header declarations for the COMPILE statement.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_STMT_COMPILE_H
#define STATEMENTS_PROGRAM_STMT_COMPILE_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"

BppError stmt_compile_handler(VMContext *vm, LexerContext *lex);
void     stmt_compile_register(void);

#endif // STATEMENTS_PROGRAM_STMT_COMPILE_H

// FILENAME: print_file.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (print_file.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PRINT_FILE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_PRINT_FILE_H
#define STATEMENTS_FILESYSTEM_PRINT_FILE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_print_file_handler(VMContext *vm, LexerContext *lex);

BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_PRINT_FILE_H

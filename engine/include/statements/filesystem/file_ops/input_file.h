// FILENAME: input_file.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (input_file.c, line.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the INPUT_FILE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_INPUT_FILE_H
#define STATEMENTS_FILESYSTEM_INPUT_FILE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_input_file_handler(VMContext *vm, LexerContext *lex);

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex);

BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_INPUT_FILE_H

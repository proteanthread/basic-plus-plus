// FILENAME: files.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (files.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the FILES statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_FILES_H
#define STATEMENTS_FILESYSTEM_FILES_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_files_handler(VMContext *vm, LexerContext *lex);

BppError stmt_dir_handler(VMContext *vm, LexerContext *lex);

BppError stmt_pwd_handler(VMContext *vm, LexerContext *lex);

BppError stmt_path_handler(VMContext *vm, LexerContext *lex);

BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex);

BppError stmt_scratch_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_FILES_H

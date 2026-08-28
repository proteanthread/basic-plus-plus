// FILENAME: version.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, libcore, libengine, libkernel, libstandard
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VERSION statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_VERSION_H
#define STATEMENTS_SYSTEM_VERSION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_version_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ver_handler(VMContext *vm, LexerContext *lex);
BValue func_ver_str_eval(BValue *args, int arg_count, void *rt);
void stmt_version_register(void);

#endif // STATEMENTS_SYSTEM_VERSION_H

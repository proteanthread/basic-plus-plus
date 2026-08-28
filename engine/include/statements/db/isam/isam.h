// FILENAME: isam.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (delete.c, exec_internal.h, isam.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ISAM statement in BASIC++.
//
// ---- Includes ----

#ifndef ISAM_H
#define ISAM_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Statement handlers
BppError stmt_createindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_deleteindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_setindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_insert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_update_handler(VMContext *vm, LexerContext *lex);
BppError stmt_delete_rec_handler(VMContext *vm, LexerContext *lex);
BppError stmt_retrieve_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekeq_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekge_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekgt_handler(VMContext *vm, LexerContext *lex);

void stmt_isam_register(void);
void isam_system_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // ISAM_H

// FILENAME: stmt_mutex.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, stmt_mutex.c)
// NEEDS: libengine (lexer.h, vm.h)
// Provides runtime statement and function prototypes for MUTEX concurrency.
//
// ---- Includes ----

#ifndef STMT_MUTEX_H
#define STMT_MUTEX_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_mutex_register(void);
BppError stmt_mutex_handler(VMContext *vm, LexerContext *lex);

// Virtual Mutex API
int mutex_create_or_get(const char *name);
bool mutex_lock(int id);
bool mutex_unlock(int id);
bool mutex_is_locked(int id);

// Builtin functions
BValue func_mutex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mutex_lock_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mutex_unlock_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // STMT_MUTEX_H

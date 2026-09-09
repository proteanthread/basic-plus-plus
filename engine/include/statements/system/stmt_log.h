// FILENAME: stmt_log.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h), libcore (debug/logger.h)
// Interface definitions for Structured and Flat Logging Statements (LOG.INFO, LOG.WARN, LOG.ERROR, LOG.DEBUG, LOG.TRACE, LOGINFO, etc.).
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_STMT_LOG_H
#define STATEMENTS_SYSTEM_STMT_LOG_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Statement handler for structured LOG statement (LOG.INFO, LOG.WARN, LOG.ERROR, LOG.DEBUG, LOG.TRACE, LOG LEVEL, LOG DUMP, LOG CLEAR)
BppError stmt_log_handler(VMContext *vm, LexerContext *lex);

// Direct flat statement handlers
BppError stmt_loginfo_handler(VMContext *vm, LexerContext *lex);
BppError stmt_logwarn_handler(VMContext *vm, LexerContext *lex);
BppError stmt_logerror_handler(VMContext *vm, LexerContext *lex);
BppError stmt_logdebug_handler(VMContext *vm, LexerContext *lex);
BppError stmt_logtrace_handler(VMContext *vm, LexerContext *lex);

// Registers microlib metadata for logging statements
void stmt_log_register(void);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_SYSTEM_STMT_LOG_H

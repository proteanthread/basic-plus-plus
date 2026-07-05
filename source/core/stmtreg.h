/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stmtreg.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Dynamic Statement Registry subsystem. Maps keyword IDs and named
 *    identifiers to statement parsing callbacks.
 * ===================================================================== */

#ifndef BASICPP_STMTREG_H
#define BASICPP_STMTREG_H

#include "lexer.h"
#include "runtime.h"

typedef void (*StmtParserCallback)(Lexer *lex, RuntimeState *rt, int line_num);

typedef struct {
    char name[32];
    KeywordId kw;
    StmtParserCallback handler;
} StmtEntry;

#define MAX_STATEMENT_ENTRIES 256

void stmtreg_init(void);
void stmtreg_cleanup(void);
void stmtreg_register_builtins(void);
int stmtreg_register(const char *name, KeywordId kw, StmtParserCallback handler);
StmtParserCallback stmtreg_find_by_keyword(KeywordId kw);
StmtParserCallback stmtreg_find_by_name(const char *name, int len);

void cmd_alias_clear(void);
int cmd_alias_register(const char *name, int name_len, const char *expansion, int exp_len);
const char *cmd_alias_find(const char *name, int len);

#endif // BASICPP_STMTREG_H

// FILENAME: line_page.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, line_stmt.c, page_stmt.c)
// NEEDS: libcore, libengine
// Declarations for LINE and PAGE output statements (JOSS / RAND P-2922).

#ifndef LINE_PAGE_H
#define LINE_PAGE_H

#include "eval/eval.h"

BppError stmt_line_output_handler(VMContext *vm, LexerContext *lex);
BppError stmt_page_output_handler(VMContext *vm, LexerContext *lex);

#endif // LINE_PAGE_H

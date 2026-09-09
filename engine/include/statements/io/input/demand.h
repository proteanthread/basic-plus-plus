// FILENAME: demand.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, demand.c)
// NEEDS: libcore, libengine
// Declarations for DEMAND statement (JOSS / RAND P-2922).

#ifndef DEMAND_H
#define DEMAND_H

#include "eval/eval.h"

void stmt_demand_register(void);
BppError stmt_demand_handler(VMContext *vm, LexerContext *lex);

#endif // DEMAND_H

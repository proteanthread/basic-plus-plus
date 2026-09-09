// FILENAME: drivelistbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (drivelistbox.c)
// NEEDS: libcore (types.h)
// Declares the DRIVELISTBOX UI control statement handler interface in BASIC++.

#ifndef DRIVELISTBOX_H
#define DRIVELISTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_drivelistbox_handler(VMContext *vm, LexerContext *lex);
void stmt_drivelistbox_register(void);

#endif // DRIVELISTBOX_H

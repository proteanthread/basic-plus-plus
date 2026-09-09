// FILENAME: filelistbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (filelistbox.c)
// NEEDS: libcore (types.h)
// Declares the FILELISTBOX UI control statement handler interface in BASIC++.

#ifndef FILELISTBOX_H
#define FILELISTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_filelistbox_handler(VMContext *vm, LexerContext *lex);
void stmt_filelistbox_register(void);

#endif // FILELISTBOX_H

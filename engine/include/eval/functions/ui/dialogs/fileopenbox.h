// FILENAME: fileopenbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (fileopenbox.c)
// NEEDS: libcore (types.h)
// Declares the FILEOPENBOX$ built-in function interface in BASIC++.

#ifndef FILEOPENBOX_H
#define FILEOPENBOX_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_fileopenbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_fileopenbox_register(void);

#endif // FILEOPENBOX_H

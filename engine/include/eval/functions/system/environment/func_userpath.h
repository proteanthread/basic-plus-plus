// FILENAME: func_userpath.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_userpath.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares HOMEDRIVE$, HOMEPATH$, USERPATH$, HOME$ interface in BASIC++.

#ifndef FUNC_USERPATH_H
#define FUNC_USERPATH_H

#include "types/types.h"
#include "vm/vm.h"

void func_userpath_register(void);
BValue func_userpath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_USERPATH_H

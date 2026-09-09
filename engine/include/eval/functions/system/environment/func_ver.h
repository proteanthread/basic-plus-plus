// FILENAME: func_ver.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_ver.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares VER, TODAY$, ERRORLEVEL, TXN, TXNSTATUS, EXTERR, VARPTR$, IOCTL$ interface in BASIC++.

#ifndef FUNC_VER_H
#define FUNC_VER_H

#include "types/types.h"
#include "vm/vm.h"

void func_ver_register(void);
BValue func_ver_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_VER_H

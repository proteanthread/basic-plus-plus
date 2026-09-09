// FILENAME: func_workdir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_workdir.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares WORKDIR$, CWD$, PWD$, CURDIR$, DRIVE$ interface in BASIC++.

#ifndef FUNC_WORKDIR_H
#define FUNC_WORKDIR_H

#include "types/types.h"
#include "vm/vm.h"

void func_workdir_register(void);
BValue func_workdir_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_WORKDIR_H

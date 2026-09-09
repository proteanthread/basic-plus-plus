// FILENAME: func_vintage_vars.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_vintage_vars.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares ST, CONSOL, USER, BDOS, BIOS, SWAP$, JOB, JOB$, PPN$, SYS$, MAXRAM, HIMEM, LOMEM interface in BASIC++.

#ifndef FUNC_VINTAGE_VARS_H
#define FUNC_VINTAGE_VARS_H

#include "types/types.h"
#include "vm/vm.h"

void func_vintage_vars_register(void);
BValue func_vintage_vars_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_VINTAGE_VARS_H

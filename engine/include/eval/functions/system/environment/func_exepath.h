// FILENAME: func_exepath.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_exepath.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares EXEPATH$, EXEDIR$, EXENAME$, APPDIR$, DATADIR$, DOCSDIR$, HELPDIR$ interface in BASIC++.

#ifndef FUNC_EXEPATH_H
#define FUNC_EXEPATH_H

#include "types/types.h"
#include "vm/vm.h"

void func_exepath_register(void);
BValue func_exepath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_EXEPATH_H

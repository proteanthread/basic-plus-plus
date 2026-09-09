// FILENAME: func_scriptpath.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_scriptpath.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares SCRIPTPATH$, SCRIPTDIR$, SCRIPTNAME$, FILENAME$, DIRPATH$, BASENAME$, BASEDIR$, PROGNAME$, EXT$ interface in BASIC++.

#ifndef FUNC_SCRIPTPATH_H
#define FUNC_SCRIPTPATH_H

#include "types/types.h"
#include "vm/vm.h"

void func_scriptpath_register(void);
BValue func_scriptpath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_SCRIPTPATH_H

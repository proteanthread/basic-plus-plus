// FILENAME: func_tempdir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_tempdir.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares TEMPDIR$, TMPDIR$, TEMP$, TMP$ interface in BASIC++.

#ifndef FUNC_TEMPDIR_H
#define FUNC_TEMPDIR_H

#include "types/types.h"
#include "vm/vm.h"

void func_tempdir_register(void);
BValue func_tempdir_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_TEMPDIR_H

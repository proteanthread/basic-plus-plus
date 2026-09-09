// FILENAME: func_program.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_program.c, common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (types.h), libkernel (vm.h)
// Provides declarations for the PROGRAM$ program introspection function.

#ifndef FUNC_PROGRAM_H
#define FUNC_PROGRAM_H

#include "types/types.h"
#include "vm/vm.h"

void func_program_register(void);
BValue func_program_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_PROGRAM_H

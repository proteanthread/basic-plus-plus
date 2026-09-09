// FILENAME: hebrew.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (hebrew.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares the HEBREW$ function interface and LanguageDescriptor in BASIC++.

#ifndef HEBREW_H
#define HEBREW_H

#include "types/types.h"
#include "vm/vm.h"

void func_hebrew_register(void);
BValue func_hebrew_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // HEBREW_H

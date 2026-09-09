// FILENAME: place.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (place.c)
// NEEDS: libcore (types.h)
// Declares the PLACE$ built-in function interface in BASIC++.

#ifndef PLACE_H
#define PLACE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_place_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_place_register(void);

#endif // PLACE_H

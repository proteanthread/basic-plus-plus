// FILENAME: isarray.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (types.h), libengine (vm.h)
// Declares runtime evaluation for the ISARRAY built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_TYPES_ISARRAY_H
#define EVAL_FUNCTIONS_TYPES_ISARRAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

void   func_isarray_register(void);
BValue func_isarray_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_TYPES_ISARRAY_H

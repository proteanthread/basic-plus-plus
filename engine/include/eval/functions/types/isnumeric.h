// FILENAME: isnumeric.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (types.h), libengine (vm.h)
// Declares runtime evaluation for the ISNUMERIC built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_TYPES_ISNUMERIC_H
#define EVAL_FUNCTIONS_TYPES_ISNUMERIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

void   func_isnumeric_register(void);
BValue func_isnumeric_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_TYPES_ISNUMERIC_H

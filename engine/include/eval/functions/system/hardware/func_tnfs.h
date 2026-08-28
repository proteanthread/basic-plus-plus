// FILENAME: func_tnfs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_tnfs.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares TNFS.DIR$ built-in function.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_HARDWARE_FUNC_TNFS_H
#define EVAL_FUNCTIONS_HARDWARE_FUNC_TNFS_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_tnfs_dir(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_HARDWARE_FUNC_TNFS_H

// FILENAME: func_nil_bead.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_nil_bead.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides prototypes for NET.PACK$() and NIL.PACK$().
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_NIL_BEAD_H
#define EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_NIL_BEAD_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_net_pack(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_NIL_BEAD_H

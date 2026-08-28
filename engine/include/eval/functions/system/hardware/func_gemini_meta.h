// FILENAME: func_gemini_meta.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_gemini_meta.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares GEMINI.STATUS%() and GEMINI.META$() built-in functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_HARDWARE_FUNC_GEMINI_META_H
#define EVAL_FUNCTIONS_HARDWARE_FUNC_GEMINI_META_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_gemini_status(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_gemini_meta(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_HARDWARE_FUNC_GEMINI_META_H

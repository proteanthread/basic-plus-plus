// FILENAME: func_systems_types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c, func_systems_types.c)
// NEEDS: libkernel (types.h)
// Provides runtime function prototypes for systems types, pointers, and masking.
//
// ---- Includes ----

#ifndef FUNC_SYSTEMS_TYPES_H
#define FUNC_SYSTEMS_TYPES_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_systems_types_register(void);

BValue func_byte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_word_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_dword_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ptr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_deref_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mask_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_set_bitfield_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_SYSTEMS_TYPES_H

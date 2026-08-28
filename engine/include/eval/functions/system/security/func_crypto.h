// FILENAME: func_crypto.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDED BY: libkernel (func_crypto.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides prototypes for CRYPTO built-in functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_SECURITY_FUNC_CRYPTO_H
#define EVAL_FUNCTIONS_SYSTEM_SECURITY_FUNC_CRYPTO_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_crypto_encrypt(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_crypto_decrypt(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_crypto_hash(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_crypto_hmac(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_crypto_key(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_SECURITY_FUNC_CRYPTO_H

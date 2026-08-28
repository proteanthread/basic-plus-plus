// FILENAME: func_sock.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_sock.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides prototypes for SOCK built-in functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_SOCK_H
#define EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_SOCK_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_sock_open(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_sock_accept(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_sock_recv(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_sock_poll(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_sock_status(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_SOCK_H

// FILENAME: func_ip.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides interface definitions for the IP$ function in BASIC++.

#ifndef FUNC_IP_H
#define FUNC_IP_H

#include "eval/eval.h"
#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

void func_ip_register(void);
BValue func_ip_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_IP_H

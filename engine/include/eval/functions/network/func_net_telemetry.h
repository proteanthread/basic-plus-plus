// FILENAME: func_net_telemetry.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_net_telemetry.c, common_reg_funcs.c)
// NEEDS: libcore (types.h)
// Declares NSTATUS, NHTTPSTATUS, NEOF, NBYTESWAITING, NCONNECTED, NERROR, SIOSTATUS, SIOAVAIL, BIOSTATUS, BIOSIZE, BIOCHECKSUM interface in BASIC++.

#ifndef FUNC_NET_TELEMETRY_H
#define FUNC_NET_TELEMETRY_H

#include "types/types.h"
#include "vm/vm.h"

void func_net_telemetry_register(void);
BValue func_net_telemetry_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_NET_TELEMETRY_H

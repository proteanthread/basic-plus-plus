// FILENAME: func_packet.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_packet.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares built-in PACKET.* metadata inspection functions.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_PACKET_H
#define EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_PACKET_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_packet_mac(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_rssi(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_payload(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_len(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_src(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_port(VMContext *vm, int argc, BValue *argv, BppError *err);
BValue func_packet_type(VMContext *vm, int argc, BValue *argv, BppError *err);

#endif // EVAL_FUNCTIONS_SYSTEM_HARDWARE_FUNC_PACKET_H

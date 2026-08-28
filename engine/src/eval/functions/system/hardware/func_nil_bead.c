// FILENAME: func_nil_bead.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_bead.h, nil_bead.c, strings.h, strings.c)
// NEEDS: libengine (func_nil_bead.h)
// Implements NET.PACK$() and NIL.PACK$() RFC 51 serialization.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_nil_bead.h"
#include "runtime/nil_bead.h"
#include "runtime/strings.h"
#include <stdlib.h>

BValue func_net_pack(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 1) {
        err->code = 13;
        err->message = "NET.PACK$ expects value or array argument";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    uint8_t buffer[4096];
    size_t written = nil_bead_serialize_value(vm, argv[0], buffer, sizeof(buffer));

    BValue res = {
        .type = VAL_STRING,
        .as.string = str_create(vm_get_str(vm), (const char *)buffer, written)
    };
    return res;
}

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
#include "runtime/language_descriptor.h"
#include "memory/memory.h"
#include "vm/vm.h"

static const LangDesc g_net_pack_desc = {
    .name = "NET.PACK$",
    .category = "Network & Protocols",
    .syntax = "NET.PACK$(var)",
    .description = "Serializes variable or array into RFC 51 compact bead byte stream.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_nil_bead_register(void) {
    lang_desc_register(&g_net_pack_desc);
}

BValue func_net_pack(VMContext *vm, int argc, BValue *argv, BppError *err) {
    if (argc < 1) {
        err->code = 13;
        err->message = "NET.PACK$ expects value or array argument";
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    uint8_t stack_buf[1024];
    uint8_t *buffer = stack_buf;
    size_t buf_cap = sizeof(stack_buf);
    size_t written = nil_bead_serialize_value(vm, argv[0], buffer, buf_cap);

    BValue res = {
        .type = VAL_STRING,
        .as.string = str_create(vm_get_str(vm), (const char *)buffer, written)
    };
    return res;
}

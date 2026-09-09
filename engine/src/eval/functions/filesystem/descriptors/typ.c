// FILENAME: typ.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (file.h, file.c, funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (typ.h, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the TYP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/filesystem/descriptors/typ.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/file.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "runtime/memory.h"
#include "runtime/string/memops.h"

static const LangDesc g_typ_desc = {
    .name = "TYP",
    .category = "Filesystem",
    .syntax = "type_code% = TYP(#channel) | TYP(channel)",
    .description = "Apple /// Business BASIC channel record type introspection (0=EOF, 1=Integer, 2=Real, 3=String).",
    .error_summary = "Error 13: Type Mismatch, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_typ_register(void) {
    lang_desc_register(&g_typ_desc);
}

BValue func_typ_eval(VMContext *vm, const char *name, int argc, BValue *args, BppError *err) {
    (void)name;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;


    if (argc != 1 || args[0].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "TYP expects numeric channel";
        return res;
    }

    int ch = (int)args[0].as.number;
    if (!file_is_open(vm_get_file(vm), ch) || file_eof(vm_get_file(vm), ch)) {
        res.as.number = 0.0; // 0 = EOF / Empty
    } else {
        res.as.number = 3.0; // 1=Integer, 2=Real, 3=String/Text record
    }

    return res;
}

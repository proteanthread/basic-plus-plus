// FILENAME: unpack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mux.h, mux.c, string.c, unpack.h)
// Provides runtime implementation for the UNPACK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/unpack.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/mux.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_unpack_register(void) {
    MicroLibMetadata meta = {
        .name = "UNPACK",
        .category = "String Functions",
        .syntax = "UNPACK(fmt$, bin_str$)",
        .help_text = "Unpacks binary data from bin_str$ according to format template fmt$.",
        .error_codes = "Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string arguments)"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "UNPACK$",
        .keyword = KW_NONE,
        .category = FCAT_STRING,
        .ret_type = FRET_ANY,
        .min_args = 2,
        .max_args = 2,
        .safety = FSAFE_PURE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Unpack a serialized field from a record string",
        .module_name = "String"
    };
    funcreg_register(&entry);
}

BValue func_unpack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "UNPACK$ requires a string buffer and a field index";
        return res;
    }

    size_t idx = (size_t)args[1].as.number;
    size_t str_len_val = str_len(args[0].as.string);
    const char *data = str_data(args[0].as.string);

    BValue decoded;
    runtime_memset(&decoded, 0, sizeof(decoded));

    if (!unpack_fields(vm_get_str(vm), data, str_len_val, &decoded, idx + 1)) {
        err->code = 5;
        err->message = "UNPACK$ deserialization index out of bounds or corrupt buffer";
        return res;
    }

    return decoded;
}

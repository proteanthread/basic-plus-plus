/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file pack.c
 * @brief PACK$ binary structure packing function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements PACK$ built-in string function, serializing multiple data fields into a packed binary or formatted record string.
 *
 * 2. WHY IT EXISTS:
 * Provides binary data packing and IPC struct serialization matching QBASIC, BASIC++, and binary file protocol requirements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates format string parameter `args[0]` and value fields `args[1..N]`. Delegates packing to runtime helper, allocates a `BppString`, releases transient string arguments, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_pack'. Includes "eval/functions/string/pack.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support extended struct packing directives (endianness markers, bitfields).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input format string and transient string values.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing packed binary bytes or ERR_TYPE_MISMATCH (error 13) or ERR_ILLEGAL_FUNCTION_CALL (error 5).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify pack field serialization in runtime modules and string pool allocations.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `VMContext` pointer and format string expression.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 8-bit byte output passthrough.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/pack.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/pack.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/mux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func_pack_register(void) {
    MicroLibMetadata meta = {
        .name = "PACK$",
        .category = "String Functions",
        .syntax = "PACK$(fmt$, val1 [, val2, ...])",
        .help_text = "Packs binary data values into a binary string according to format template fmt$.",
        .error_codes = "Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string format)"
    };
    microlib_register(&meta);
}

BValue func_pack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count == 0) {
        err->code = 13;
        err->message = "PACK$ requires at least one argument";
        return res;
    }

    size_t out_len = 0;
    char *buf = pack_fields(args, (size_t)arg_count, &out_len);
    if (!buf) {
        err->code = 5;
        err->message = "PACK$ field serialization failed";
        return res;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, out_len);
    free(buf);
    return res;
}

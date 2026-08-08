/**
 * @file bin.c
 * @brief BIN$ binary string conversion function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements BIN$ built-in string function, converting an integer numeric argument to a binary string representation of '0' and '1' digits.
 *
 * 2. WHY IT EXISTS:
 * Provides binary string formatting for bitwise inspection, hardware register display, and low-level debugging.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts to unsigned integer, shifts and masks bits to construct a heap-allocated `BppString`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_bin'. Includes "eval/functions/string/bin.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <stdint.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support optional bit-width specifiers (e.g. BIN$(x, 8), BIN$(x, 16)).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * BValue string allocation refcount rules (caller owns returned string reference).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing binary string representation or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify heap string allocation and buffer sizing for raw binary string formatting.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard 64-bit integer bitwise operations.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/bin.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/bin.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <stdint.h>

void func_bin_register(void) {
    MicroLibMetadata meta = {
        .name = "BIN$",
        .category = "String Functions",
        .syntax = "BIN$(x)",
        .help_text = "Returns the binary string representation of integer x.",
        .error_codes = "Error 13: Type Mismatch (BIN$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_bin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "BIN$ expects one numeric argument";
        return res;
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char raw[68];
    int raw_bits = 0;
    if (uv == 0) {
        raw[raw_bits++] = '0';
    } else {
        while (uv > 0 && raw_bits < 64) {
            raw[raw_bits++] = (char)('0' + (int)(uv & 1));
            uv >>= 1;
        }
    }
    int num_bytes = (raw_bits + 7) / 8;
    int total_bits = num_bytes * 8;
    while (raw_bits < total_bits) {
        raw[raw_bits++] = '0';
    }
    char out[80];
    int o = 0;
    for (int idx = total_bits - 1; idx >= 0; idx--) {
        out[o++] = raw[idx];
        if (idx > 0 && (idx % 8) == 0) {
            out[o++] = ' ';
        }
    }
    out[o] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out, o);
    return res;
}

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file func_unpack.c
 * @brief UNPACK$(fmt$, data$) binary data decoding built-in function evaluator implementation.
 *
 * 1. WHAT IT DOES:
 *    Implements `UNPACK$(format$, binary_string$)` (`func_unpack()`), parsing binary buffers according to Perl/Python style
 *    format specifiers (`c` char, `s` int16, `i` int32, `f` float, `d` double, `H` hex), extracting structured fields into array or delimited values.
 *
 * 2. WHY IT EXISTS:
 *    Enables high-performance binary protocol parsing (such as network packets, telemetry buffers, and binary files) directly in BASIC++.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Processes format specifiers left-to-right, unpacking 8-bit, 16-bit, 32-bit, or 64-bit binary elements with explicit endianness handling.
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `eval/functions/str_funcs.h`, `core/string.h`, `types/types.h`, `types/errors.h`
 *    - CMake Target: Part of `unpack` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in `libbasicpp` (`baspp`) and `libbasicpp_lite` (`bpp`/`bs`).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To add new format specifiers (e.g., `q` for 64-bit int, `p` for Pascal string): extend `unpack_parse_token()`.
 *    - To toggle big-endian vs little-endian mode (`>` vs `<`): update byte swap flags in `unpack_extract_number()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - Format specifier syntax rules (`c`, `s`, `i`, `f`, `d`, `H`).
 *    - Bounded memory read protection preventing buffer overruns.
 *
 * 8. WHAT TO EXPECT:
 *    - Returns string/array `BValue` or `ERR_OUT_OF_RANGE` if format string exceeds binary input buffer size.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check input data string length against required format specifier bytes.
 *    - Verify host endianness detection logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - `args` array contains format string and binary data string.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *     - Explicit memcpy byte copying to avoid unaligned integer access faults on ARM architecture targets.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     - Prerequisite C Source Files: `engine/src/core/string.c` (BppString memory manager), `engine/src/eval/eval.c` (argument evaluator).
 *     - Prerequisite Header Surfaces: `engine/include/eval/functions/str_funcs.h`, `engine/include/core/string.h`, `engine/include/types/types.h`, `engine/include/types/errors.h`.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file func_unpack.c
 * @brief UNPACK$ Built-in Function Micro-Library.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Evaluates the UNPACK$ function to deserialise packed record fields.
 * - Why it exists: Isolated micro-library for UNPACK$ record deserialization.
 * - Why it works this way: Uses unpack_fields from runtime/mux to decode values.
 *
 * SECTION 2: SAFE ZONES FOR EDITING
 * - Safe to update parameter bounds or register metadata text.
 *
 * SECTION 3: CRITICAL INVARIANTS
 * - Must validate argument types.
 *
 * SECTION 4: EXPECTED SIDE-EFFECTS & BEHAVIORS
 * - Deserializes record data buffer.
 *
 * SECTION 5: WHAT TO DO IF SOMETHING BREAKS
 * - Check unpack_fields in runtime/mod_mux.c.
 *
 * SECTION 6: ASSUMPTIONS
 * - Host VM context is non-NULL.
 *
 * SECTION 7: PORTABILITY CONCERNS
 * - Strictly portable C17.
 *
 * SECTION 8: FUTURE EXPANSIONS
 * - Advanced schema-based unpacking.
 */

#include "eval/functions/string/unpack.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/mux.h"
#include "runtime/funcreg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    memset(&decoded, 0, sizeof(decoded));

    if (!unpack_fields(vm_get_str(vm), data, str_len_val, &decoded, idx + 1)) {
        err->code = 5;
        err->message = "UNPACK$ deserialization index out of bounds or corrupt buffer";
        return res;
    }

    return decoded;
}

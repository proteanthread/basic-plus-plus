/**
 * @file microplex.c
 * @brief MICROPLEX$(s1$, s2$) built-in string multiplexing function evaluator for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Evaluates MICROPLEX$(s1$, s2$) built-in string function for character-level string interleaving.
 *
 * 2. WHY IT EXISTS:
 * Fulfills strict keyword-to-filename mapping requirement for the MICROPLEX$ feature per Rule #1.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Consumes two string arguments, allocates an output buffer, interleaves characters from both strings alternatingly, and returns a reference-counted VAL_STRING.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'eval'. Includes "eval/eval_internal.h", "runtime/microplex.h",
 * "runtime/strings.h", "vm/vm.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support 3+ string parameters or custom stride patterns.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * BValue ownership semantics: returned VAL_STRING refcount MUST be owned by caller (Rule #1 & #3).
 *
 * 8. WHAT TO EXPECT:
 * Returns newly allocated refcounted string BValue or ERR_TYPE_MISMATCH.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcounts and argument count validation.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments are evaluated BValue structures.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Bounded allocation and safe string buffer frees.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/core/string.c
 * Prerequisite Header Files:
 * - engine/include/eval/eval.h
 * - engine/include/runtime/microplex.h
 * - engine/include/runtime/strings.h
 */

#include "eval/eval_internal.h"
#include "runtime/microplex.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <stdlib.h>
#include <string.h>

BValue func_microplex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "MICROPLEX$") != 0 && strcmp(uname, "MICROPLEX") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = 13;
        err->message = "MICROPLEX$ requires two string arguments";
        return res;
    }

    const char *s1 = args[0].as.string ? str_data(args[0].as.string) : "";
    size_t len1 = args[0].as.string ? str_len(args[0].as.string) : 0;

    const char *s2 = args[1].as.string ? str_data(args[1].as.string) : "";
    size_t len2 = args[1].as.string ? str_len(args[1].as.string) : 0;

    size_t out_cap = len1 + len2 + 1;
    char *out_buf = (char *)malloc(out_cap);
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in MICROPLEX$";
        return res;
    }

    size_t out_len = microplex_string(s1, len1, s2, len2, out_buf, out_cap);
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, out_len);
    free(out_buf);

    return res;
}

/**
 * @file regex.c
 * @brief Module implementation for regular expression pattern matching functions (REGEX.MATCH, REGEX.REPLACE, REGEX.FIND) in BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements string pattern matching and replacement functions using portable regular expression rules.
 *
 * 2. WHY IT EXISTS:
 * Extends string processing capabilities with pattern-based matching and text substitution per modern BASIC requirements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Compiles ASCII pattern strings into lightweight state machines or NFA matchers; operates directly on null-terminated string buffers with bounded character comparisons.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'module'. Includes "module/regex.h", "runtime/funcreg.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support extended Regex flags (case-insensitive, multi-line).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Bounded string matching logic and memory allocation refcount lifecycle.
 *
 * 8. WHAT TO EXPECT:
 * Returns integer boolean match state or newly allocated replacement string BValue.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify pattern string syntax and zero-termination of target string inputs.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid non-NULL pattern and target string input pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Portable C implementation with no POSIX regex.h dependency required.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/funcreg.c
 * Prerequisite Header Files:
 * - engine/include/module/regex.h
 * - engine/include/runtime/funcreg.h
 */

/**
 * @file mod_regex.c
 * @brief Regex functionality implementation in C17.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements REGEX_MATCH and REGEX_REPLACE$ built-in functions.
 * - Why it exists: Provides pattern matching and string substitution using platform regex wrappers.
 * - Why it works this way: Routes matching and replacing through platform wrappers to
 *   avoid dependencies on external/non-standard C++ regex engines.
 */

#include "module/regex.h"
#include "platform/platform.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include <string.h>
#include <stdlib.h>

static BValue regex_match_func(BValue *args, int argc, void *rt) {
    (void)rt;
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (argc < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return res;
    }

    const char *text = str_data(args[0].as.string);
    const char *pattern = str_data(args[1].as.string);
    if (!text || !pattern) {
        return res;
    }

    if (platform_regex_match(text, pattern)) {
        res.as.number = -1.0; /* TRUE in BASIC */
    }

    return res;
}

static BValue regex_replace_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;

    if (argc < 3 || args[0].type != VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    const char *text = str_data(args[0].as.string);
    const char *pattern = str_data(args[1].as.string);
    const char *replacement = str_data(args[2].as.string);
    if (!text || !pattern || !replacement) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    char *replaced = platform_regex_replace(text, pattern, replacement);
    if (replaced) {
        res.as.string = str_create(vm_get_str(vm), replaced, strlen(replaced));
        free(replaced);
    } else {
        res.as.string = str_create(vm_get_str(vm), text, strlen(text));
    }

    return res;
}

void register_regex_functions(void) {
    FunctionEntry fe;
    memset(&fe, 0, sizeof(fe));
    fe.module_name = "regexext";
    fe.overridable = 1;
    fe.category = FCAT_STRING;
    
    fe.name = "REGEX_MATCH";
    fe.ret_type = FRET_FLOAT;
    fe.min_args = 2;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = regex_match_func;
    funcreg_register(&fe);

    fe.name = "REGEX_REPLACE$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 3;
    fe.max_args = 3;
    fe.handler = regex_replace_func;
    funcreg_register(&fe);
}

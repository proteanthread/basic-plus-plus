/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mod_regex.c
 * @brief Regex functionality implementation in C17.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements REGEX_MATCH and REGEX_REPLACE$ built-in functions.
 * - Why it exists: Provides pattern matching and string substitution using platform regex wrappers.
 * - Why it works this way: Routes matching and replacing through bpp_platform wrappers to
 *   avoid dependencies on external/non-standard C++ regex engines.
 */

#include "mod_regex.h"
#include "bpp_platform.h"
#include "bpp_funcreg.h"
#include "bpp_vm.h"
#include "bpp_strings.h"
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

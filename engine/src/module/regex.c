// FILENAME: regex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, hal.h, memops.h, memops.c, regex.h)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (vm.h)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for regex within BASIC++.
//
// ---- Includes ----

#include "module/regex.h"
#include "platform/platform.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static BValue regex_match_func(BValue *args, int argc, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
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
        res.as.number = -1.0; // TRUE in BASIC
    }

    return res;
}

static BValue regex_replace_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
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
        res.as.string = str_create(vm_get_str(vm), replaced, runtime_strlen(replaced));
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) hal->mem.free(replaced);
    } else {
        res.as.string = str_create(vm_get_str(vm), text, runtime_strlen(text));
    }

    return res;
}

void register_regex_functions(void) {
    FunctionEntry fe;
    runtime_memset(&fe, 0, sizeof(fe));
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


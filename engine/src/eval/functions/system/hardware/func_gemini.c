// FILENAME: func_gemini.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, gemini.h, gemini.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements the GEMINI.GET$ built-in function for Gemini protocol requests.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "runtime/gemini.h"
#include "vm/vm.h"
#include <stdlib.h>
#include <string.h>

BValue func_gemini_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count < 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "GEMINI.GET$ expects string URL argument";
        return res;
    }

    const char *url = str_data(args[0].as.string);
    char *resp = net_gemini_fetch(vm, url, err);
    if (resp) {
        res.as.string = str_create(vm_get_str(vm), resp, strlen(resp));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

void func_gemini_register(void) {
    MicroLibMetadata meta = {
        .name = "GEMINI.GET$",
        .category = "Network & Cloud",
        .syntax = "GEMINI.GET$(url$)",
        .help_text = "Fetches a text/gemini document from a gemini:// URI capsule over TLS.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "GEMINI.GET$",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Fetch Gemini protocol URL",
        .module_name = "Net"
    };
    funcreg_register(&entry);
}

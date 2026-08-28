// FILENAME: func_gopher.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, gopher.h, gopher.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements the GOPHER.GET$ built-in function for RFC 1436 Gopher requests.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "runtime/gopher.h"
#include "vm/vm.h"
#include <stdlib.h>
#include <string.h>

BValue func_gopher_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count < 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "GOPHER.GET$ expects string URL argument";
        return res;
    }

    const char *url = str_data(args[0].as.string);
    char *resp = net_gopher_fetch(vm, url, err);
    if (resp) {
        res.as.string = str_create(vm_get_str(vm), resp, strlen(resp));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

void func_gopher_register(void) {
    MicroLibMetadata meta = {
        .name = "GOPHER.GET$",
        .category = "Network & Cloud",
        .syntax = "GOPHER.GET$(url$)",
        .help_text = "Fetches a Gopher item or menu from a gopher:// URL over RFC 1436 port 70.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "GOPHER.GET$",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Fetch Gopher protocol URL",
        .module_name = "Net"
    };
    funcreg_register(&entry);
}

// FILENAME: func_http.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements the HTTP.GET$ built-in function for REST web client requests.
//
// ---- Includes ----

#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "vm/vm.h"
#include "iot_net.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/strops.h"

static const LangDesc g_http_get_desc = {
    .name = "HTTP.GET$",
    .category = "Wireless & IoT",
    .syntax = "HTTP.GET$(url$)",
    .description = "Performs an HTTP GET request to a remote web server and returns response body.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

BValue func_http_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count < 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "HTTP.GET$ expects string URL argument";
        return res;
    }

    const char *url = str_data(args[0].as.string);
    char *resp = iot_http_get(url);
    if (resp) {
        res.as.string = str_create(vm_get_str(vm), resp, runtime_strlen(resp));
        runtime_free(resp);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

void func_http_register(void) {
    lang_desc_register(&g_http_get_desc);

    FunctionEntry entry_get = {
        .name = "HTTP.GET$",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Perform HTTP GET request",
        .module_name = "Net"
    };
    funcreg_register(&entry_get);
}

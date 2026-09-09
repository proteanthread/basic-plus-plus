// FILENAME: curdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (curdir.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the CURDIR$ function in BASIC++.

#include "eval/functions/system/environment/curdir.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_curdir_desc = {
    .name = "CURDIR$",
    .category = "File Functions",
    .syntax = "CURDIR$([drive$])",
    .description = "Returns the current working directory path.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_curdir_register(void) {
    lang_desc_register(&g_curdir_desc);
}

BValue func_curdir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    BValue res;
    res.type = VAL_STRING;
    char buf[1024];
    runtime_memset(buf, 0, sizeof(buf));
    if (platform_getcwd(buf, sizeof(buf) - 1)) {
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        res.as.string = str_create(vm_get_str(vm), "C:\\", 3);
    }
    return res;
}

// FILENAME: func_workdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_workdir.h)
// Provides runtime implementation and LanguageDescriptors for working directory and drive variables.

#include "eval/functions/system/environment/func_workdir.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_workdir_desc = {
    .name = "WORKDIR$", .category = "Environment", .syntax = "WORKDIR$ or CWD$ or PWD$ or CURDIR$",
    .description = "Returns the current working directory path string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_drive_desc = {
    .name = "DRIVE$", .category = "Environment", .syntax = "DRIVE$ or SYS.DRIVE$",
    .description = "Returns the current disk drive letter (e.g. 'C:').",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_workdir_register(void) {
    lang_desc_register(&g_workdir_desc);
    lang_desc_register(&g_drive_desc);
}

BValue func_workdir_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));
    char pbuf[1024] = {0};
    platform_getcwd(pbuf, sizeof(pbuf) - 1);

    if (uname && (runtime_strcasecmp(uname, "DRIVE$") == 0 || runtime_strcasecmp(uname, "DRIVE") == 0 ||
                  runtime_strcasecmp(uname, "SYS.DRIVE$") == 0)) {
        char dbuf[4] = {0};
        if (pbuf[0] && pbuf[1] == ':') {
            dbuf[0] = pbuf[0]; dbuf[1] = ':'; dbuf[2] = '\0';
        } else {
            dbuf[0] = '/'; dbuf[1] = '\0';
        }
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), dbuf, runtime_strlen(dbuf));
        return res;
    }

    res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), pbuf, runtime_strlen(pbuf));
    return res;
}

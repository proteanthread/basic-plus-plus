// FILENAME: dir_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (dir_fn.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DIR$ function in BASIC++.

#include "eval/functions/system/environment/dir_fn.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_dir_desc = {
    .name = "DIR$",
    .category = "File Functions",
    .syntax = "DIR$([filespec$])",
    .description = "Returns the next matching filename matching the filespec filter.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static BppDirSearch *g_dir_search = NULL;

void func_dir_fn_register(void) {
    lang_desc_register(&g_dir_desc);
}

BValue func_dir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    const char *spec = NULL;
    if (arg_count >= 1 && args[0].type == VAL_STRING && args[0].as.string) {
        spec = str_data(args[0].as.string);
    }

    char name_buf[260];
    name_buf[0] = '\0';

    if (spec && spec[0] != '\0') {
        if (g_dir_search) {
            platform_find_close(g_dir_search);
            g_dir_search = NULL;
        }
        g_dir_search = platform_find_first_file(spec, name_buf, sizeof(name_buf));
        if (!g_dir_search) {
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
        res.as.string = str_create(vm_get_str(vm), name_buf, runtime_strlen(name_buf));
        return res;
    } else {
        if (!g_dir_search) {
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
        if (platform_find_next_file(g_dir_search, name_buf, sizeof(name_buf))) {
            res.as.string = str_create(vm_get_str(vm), name_buf, runtime_strlen(name_buf));
            return res;
        } else {
            platform_find_close(g_dir_search);
            g_dir_search = NULL;
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
    }
}

// FILENAME: func_tempdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_tempdir.h)
// Provides runtime implementation and LanguageDescriptors for temporary directory variables.

#include "eval/functions/system/environment/func_tempdir.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_tempdir_desc = {
    .name = "TEMPDIR$", .category = "Environment", .syntax = "TEMPDIR$ or TMPDIR$ or TEMP$ or TMP$",
    .description = "Returns the host operating system temporary directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_tempdir_register(void) {
    lang_desc_register(&g_tempdir_desc);
}

BValue func_tempdir_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));
    char tbuf[1024] = {0};
#if defined(_WIN32)
    const char *tmp = platform_getenv("TEMP");
    if (!tmp) tmp = platform_getenv("TMP");
    if (!tmp) tmp = "C:\\TEMP";
#else
    const char *tmp = platform_getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
#endif
    runtime_strncpy(tbuf, tmp, sizeof(tbuf) - 1);
    res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), tbuf, runtime_strlen(tbuf));
    return res;
}

// FILENAME: func_userpath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_userpath.h)
// Provides runtime implementation and LanguageDescriptors for user profile and home path variables.

#include "eval/functions/system/environment/func_userpath.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_homedrive_desc = {
    .name = "HOMEDRIVE$", .category = "Environment", .syntax = "HOMEDRIVE$",
    .description = "Returns the user home drive letter (e.g. 'C:').",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_homepath_desc = {
    .name = "HOMEPATH$", .category = "Environment", .syntax = "HOMEPATH$",
    .description = "Returns the user home directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_userpath_desc = {
    .name = "USERPATH$", .category = "Environment", .syntax = "USERPATH$ or HOME$",
    .description = "Returns the complete user profile directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_userpath_register(void) {
    lang_desc_register(&g_homedrive_desc);
    lang_desc_register(&g_homepath_desc);
    lang_desc_register(&g_userpath_desc);
}

BValue func_userpath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));

    if (uname && (runtime_strcasecmp(uname, "HOMEDRIVE$") == 0 || runtime_strcasecmp(uname, "HOMEDRIVE") == 0)) {
#if defined(_WIN32)
        const char *hd = platform_getenv("HOMEDRIVE");
        if (!hd) hd = "C:";
#else
        const char *hd = "/";
#endif
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), hd, runtime_strlen(hd));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "HOMEPATH$") == 0 || runtime_strcasecmp(uname, "HOMEPATH") == 0)) {
#if defined(_WIN32)
        const char *hp = platform_getenv("HOMEPATH");
        if (!hp) hp = "\\Users";
#else
        const char *hp = platform_getenv("HOME");
        if (!hp) hp = "/home";
#endif
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), hp, runtime_strlen(hp));
        return res;
    }

    // USERPATH$ / HOME$
#if defined(_WIN32)
    const char *up = platform_getenv("USERPROFILE");
    if (!up) up = "C:\\Users";
#else
    const char *up = platform_getenv("HOME");
    if (!up) up = "/home";
#endif
    res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), up, runtime_strlen(up));
    return res;
}

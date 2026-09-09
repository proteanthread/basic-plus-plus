// FILENAME: func_exepath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_exepath.h)
// Provides runtime implementation and LanguageDescriptors for executable and app path variables.

#include "eval/functions/system/environment/func_exepath.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_exepath_desc = {
    .name = "EXEPATH$", .category = "Environment", .syntax = "EXEPATH$",
    .description = "Returns the absolute filesystem path of the running executable.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_exedir_desc = {
    .name = "EXEDIR$", .category = "Environment", .syntax = "EXEDIR$ or APPDIR$",
    .description = "Returns the directory containing the running executable binary.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_exename_desc = {
    .name = "EXENAME$", .category = "Environment", .syntax = "EXENAME$",
    .description = "Returns the filename of the running executable binary.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_datadir_desc = {
    .name = "DATADIR$", .category = "Environment", .syntax = "DATADIR$",
    .description = "Returns the application data directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_docsdir_desc = {
    .name = "DOCSDIR$", .category = "Environment", .syntax = "DOCSDIR$",
    .description = "Returns the documentation directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_helpdir_desc = {
    .name = "HELPDIR$", .category = "Environment", .syntax = "HELPDIR$",
    .description = "Returns the help database directory path.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_exepath_register(void) {
    lang_desc_register(&g_exepath_desc);
    lang_desc_register(&g_exedir_desc);
    lang_desc_register(&g_exename_desc);
    lang_desc_register(&g_datadir_desc);
    lang_desc_register(&g_docsdir_desc);
    lang_desc_register(&g_helpdir_desc);
}

static void get_dir_part(const char *path, char *out, size_t out_size) {
    if (!path || !out || out_size == 0) return;
    runtime_strncpy(out, path, out_size - 1);
    char *p1 = runtime_strrchr(out, '/');
    char *p2 = runtime_strrchr(out, '\\');
    char *last_slash = (p1 > p2) ? p1 : p2;
    if (last_slash) {
        *last_slash = '\0';
    }
}

static const char *get_name_part(const char *path) {
    if (!path) return "";
    const char *p1 = runtime_strrchr(path, '/');
    const char *p2 = runtime_strrchr(path, '\\');
    const char *last_slash = (p1 > p2) ? p1 : p2;
    return last_slash ? (last_slash + 1) : path;
}

BValue func_exepath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));
    char exepath[1024] = {0};
    platform_get_executable_path(exepath, sizeof(exepath));
    if (!exepath[0]) {
        platform_getcwd(exepath, sizeof(exepath) - 1);
    }

    if (uname && (runtime_strcasecmp(uname, "EXENAME$") == 0 || runtime_strcasecmp(uname, "EXENAME") == 0)) {
        const char *name = get_name_part(exepath);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), name, runtime_strlen(name));
        return res;
    }
    if (uname && (runtime_strcasecmp(uname, "EXEDIR$") == 0 || runtime_strcasecmp(uname, "EXEDIR") == 0 ||
                  runtime_strcasecmp(uname, "APPDIR$") == 0 || runtime_strcasecmp(uname, "APPDIR") == 0)) {
        char dir[1024] = {0}; get_dir_part(exepath, dir, sizeof(dir));
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), dir, runtime_strlen(dir));
        return res;
    }
    if (uname && (runtime_strcasecmp(uname, "DATADIR$") == 0 || runtime_strcasecmp(uname, "DATADIR") == 0)) {
        char dir[1024] = {0}; get_dir_part(exepath, dir, sizeof(dir));
        char data_path[1024] = {0};
        runtime_snprintf(data_path, sizeof(data_path), "%s/openrgb", dir);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), data_path, runtime_strlen(data_path));
        return res;
    }
    if (uname && (runtime_strcasecmp(uname, "DOCSDIR$") == 0 || runtime_strcasecmp(uname, "DOCSDIR") == 0)) {
        char dir[1024] = {0}; get_dir_part(exepath, dir, sizeof(dir));
        char doc_path[1024] = {0};
        runtime_snprintf(doc_path, sizeof(doc_path), "%s/docs", dir);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), doc_path, runtime_strlen(doc_path));
        return res;
    }
    if (uname && (runtime_strcasecmp(uname, "HELPDIR$") == 0 || runtime_strcasecmp(uname, "HELPDIR") == 0)) {
        char dir[1024] = {0}; get_dir_part(exepath, dir, sizeof(dir));
        char help_path[1024] = {0};
        runtime_snprintf(help_path, sizeof(help_path), "%s/help", dir);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), help_path, runtime_strlen(help_path));
        return res;
    }

    res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), exepath, runtime_strlen(exepath));
    return res;
}

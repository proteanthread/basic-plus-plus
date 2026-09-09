// FILENAME: func_scriptpath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_scriptpath.h)
// Provides runtime implementation and LanguageDescriptors for script and file introspection variables.

#include "eval/functions/system/environment/func_scriptpath.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_scriptpath_desc = {
    .name = "SCRIPTPATH$", .category = "Environment", .syntax = "SCRIPTPATH$",
    .description = "Returns the full filesystem path of the loaded BASIC++ script.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_scriptdir_desc = {
    .name = "SCRIPTDIR$", .category = "Environment", .syntax = "SCRIPTDIR$ or DIRPATH$",
    .description = "Returns the directory containing the loaded script.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_scriptname_desc = {
    .name = "SCRIPTNAME$", .category = "Environment", .syntax = "SCRIPTNAME$ or FILENAME$",
    .description = "Returns the filename of the loaded script.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_basename_desc = {
    .name = "BASENAME$", .category = "Environment", .syntax = "BASENAME$ or BASNAME$",
    .description = "Returns the base name of the running script without directory path or extension.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_basedir_desc = {
    .name = "BASEDIR$", .category = "Environment", .syntax = "BASEDIR$ or BASEPATH$",
    .description = "Returns the project root or workspace base directory.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_progname_desc = {
    .name = "PROGNAME$", .category = "Environment", .syntax = "PROGNAME$",
    .description = "Returns the active program name or interpreter identifier.",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

static const LangDesc g_ext_desc = {
    .name = "EXT$", .category = "Environment", .syntax = "EXT$ or EXTNAME$",
    .description = "Returns the file extension of the loaded script (e.g. 'bas').",
    .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
};

void func_scriptpath_register(void) {
    lang_desc_register(&g_scriptpath_desc);
    lang_desc_register(&g_scriptdir_desc);
    lang_desc_register(&g_scriptname_desc);
    lang_desc_register(&g_basename_desc);
    lang_desc_register(&g_basedir_desc);
    lang_desc_register(&g_progname_desc);
    lang_desc_register(&g_ext_desc);
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

BValue func_scriptpath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err; (void)arg_count;
    BValue res; runtime_memset(&res, 0, sizeof(res));
    const char *script_file = vm ? vm_get_current_filename(vm) : NULL;
    if (!script_file || !*script_file) {
        script_file = "interactive.bas";
    }

    if (uname && (runtime_strcasecmp(uname, "SCRIPTDIR$") == 0 || runtime_strcasecmp(uname, "SCRIPTDIR") == 0 ||
                  runtime_strcasecmp(uname, "FILEDIR$") == 0 || runtime_strcasecmp(uname, "FILEDIR") == 0 ||
                  runtime_strcasecmp(uname, "DIRPATH$") == 0 || runtime_strcasecmp(uname, "DIRPATH") == 0)) {
        char dir[1024] = {0};
        get_dir_part(script_file, dir, sizeof(dir));
        if (!dir[0]) {
            platform_getcwd(dir, sizeof(dir) - 1);
        }
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), dir, runtime_strlen(dir));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "SCRIPTNAME$") == 0 || runtime_strcasecmp(uname, "SCRIPTNAME") == 0 ||
                  runtime_strcasecmp(uname, "FILENAME$") == 0 || runtime_strcasecmp(uname, "FILENAME") == 0)) {
        const char *name = get_name_part(script_file);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), name, runtime_strlen(name));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "BASENAME$") == 0 || runtime_strcasecmp(uname, "BASENAME") == 0 ||
                  runtime_strcasecmp(uname, "BASNAME$") == 0 || runtime_strcasecmp(uname, "BASNAME") == 0)) {
        const char *name = get_name_part(script_file);
        char base[256] = {0};
        runtime_strncpy(base, name, sizeof(base) - 1);
        char *dot = runtime_strrchr(base, '.');
        if (dot) *dot = '\0';
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), base, runtime_strlen(base));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "BASEDIR$") == 0 || runtime_strcasecmp(uname, "BASEDIR") == 0 ||
                  runtime_strcasecmp(uname, "BASEPATH$") == 0 || runtime_strcasecmp(uname, "BASEPATH") == 0)) {
        char cwd[1024] = {0};
        platform_getcwd(cwd, sizeof(cwd) - 1);
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), cwd, runtime_strlen(cwd));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "PROGNAME$") == 0 || runtime_strcasecmp(uname, "PROGNAME") == 0)) {
        const char *prog = "baspp";
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), prog, runtime_strlen(prog));
        return res;
    }

    if (uname && (runtime_strcasecmp(uname, "EXT$") == 0 || runtime_strcasecmp(uname, "EXT") == 0 ||
                  runtime_strcasecmp(uname, "EXTNAME$") == 0 || runtime_strcasecmp(uname, "EXTNAME") == 0)) {
        const char *name = get_name_part(script_file);
        const char *dot = runtime_strrchr(name, '.');
        const char *ext = dot ? (dot + 1) : "BAS";
        res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), ext, runtime_strlen(ext));
        return res;
    }

    res.type = VAL_STRING; res.as.string = str_create(vm_get_str(vm), script_file, runtime_strlen(script_file));
    return res;
}

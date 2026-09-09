// FILENAME: fileopenbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (fileopenbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the FILEOPENBOX$ function in BASIC++.

#include "eval/functions/ui/dialogs/fileopenbox.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "device/vdev.h"

static const LangDesc g_fileopenbox_desc = {
    .name = "FILEOPENBOX$",
    .category = "User Interface",
    .syntax = "f$ = FILEOPENBOX$([pattern$ [, title$ [, default$]]])",
    .description = "Displays a file open dialog and returns the chosen filename.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_fileopenbox_register(void) {
    lang_desc_register(&g_fileopenbox_desc);
}

BValue func_fileopenbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    char pattern[128] = "*.*";
    char title[128] = "Open File";
    char def_file[256] = "";

    if (arg_count > 0 && args[0].type == VAL_STRING && args[0].as.string) {
        const char *s = str_data(args[0].as.string);
        if (s && *s) runtime_strncpy(pattern, s, sizeof(pattern) - 1);
    }
    if (arg_count > 1 && args[1].type == VAL_STRING && args[1].as.string) {
        const char *s = str_data(args[1].as.string);
        if (s && *s) runtime_strncpy(title, s, sizeof(title) - 1);
    }
    if (arg_count > 2 && args[2].type == VAL_STRING && args[2].as.string) {
        const char *s = str_data(args[2].as.string);
        if (s && *s) runtime_strncpy(def_file, s, sizeof(def_file) - 1);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[512];
        runtime_snprintf(buf, sizeof(buf), "[ %s ]\nFile Pattern: %s\nSelected: %s\n", title, pattern, def_file[0] ? def_file : pattern);
        vdev_puts(vdev, buf);
    }

    const char *ret_str = (def_file[0] != '\0') ? def_file : (pattern[0] != '\0' ? pattern : "");
    res.as.string = str_create(vm_get_str(vm), ret_str, runtime_strlen(ret_str));
    return res;
}

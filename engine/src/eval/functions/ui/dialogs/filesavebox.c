// FILENAME: filesavebox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (filesavebox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the FILESAVEBOX$ function in BASIC++.

#include "eval/functions/ui/dialogs/filesavebox.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "device/vdev.h"

static const LangDesc g_filesavebox_desc = {
    .name = "FILESAVEBOX$",
    .category = "User Interface",
    .syntax = "f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]])",
    .description = "Displays a file save dialog and returns the chosen filename.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_filesavebox_register(void) {
    lang_desc_register(&g_filesavebox_desc);
}

BValue func_filesavebox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    char pattern[128] = "*.*";
    char title[128] = "Save File";
    char def_file[256] = "UNTITLED.TXT";

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
        runtime_snprintf(buf, sizeof(buf), "[ %s ]\nFile Pattern: %s\nSaving to: %s\n", title, pattern, def_file);
        vdev_puts(vdev, buf);
    }

    res.as.string = str_create(vm_get_str(vm), def_file, runtime_strlen(def_file));
    return res;
}

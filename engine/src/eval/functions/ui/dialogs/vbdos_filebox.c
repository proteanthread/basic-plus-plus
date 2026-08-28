// FILENAME: vbdos_filebox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, vbdos_filebox.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the VBDOS_FILEBOX built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/ui/dialogs/vbdos_filebox.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"

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

    // Output dialog header to virtual device
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

    // Output dialog header to virtual device
    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[512];
        runtime_snprintf(buf, sizeof(buf), "[ %s ]\nFile Pattern: %s\nSaving to: %s\n", title, pattern, def_file);
        vdev_puts(vdev, buf);
    }

    res.as.string = str_create(vm_get_str(vm), def_file, runtime_strlen(def_file));
    return res;
}

void func_vbdos_filebox_register(void) {
    static const MicroLibMetadata meta_open = {
        .name = "FILEOPENBOX$",
        .category = "User Interface",
        .syntax = "f$ = FILEOPENBOX$([pattern$ [, title$ [, default$]]])",
        .help_text = "Displays a file open dialog and returns the chosen filename.",
        .error_codes = "None"
    };
    static const MicroLibMetadata meta_save = {
        .name = "FILESAVEBOX$",
        .category = "User Interface",
        .syntax = "f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]])",
        .help_text = "Displays a file save dialog and returns the chosen filename.",
        .error_codes = "None"
    };
    microlib_register(&meta_open);
    microlib_register(&meta_save);
}

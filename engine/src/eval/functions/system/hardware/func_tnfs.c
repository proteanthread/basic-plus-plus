// FILENAME: func_tnfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (string.h, strings.h, strings.c, tnfs.h, tnfs.c)
// NEEDS: libengine (func_tnfs.h, string.c)
// Implements TNFS.DIR$ built-in function.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_tnfs.h"
#include "runtime/tnfs.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_tnfs_dir_desc = {
    .name = "TNFS.DIR$", .category = "Hardware & Network", .syntax = "TNFS.DIR$([path$ [, pattern$]])",
    .description = "Retrieves directory listing from a mounted TNFS network file system.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};

void func_tnfs_register(void) {
    lang_desc_register(&g_tnfs_dir_desc);
}

BValue func_tnfs_dir(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    const char *path = NULL;
    const char *pattern = NULL;

    if (argc >= 1 && argv[0].type == VAL_STRING) {
        path = str_data(argv[0].as.string);
    }
    if (argc >= 2 && argv[1].type == VAL_STRING) {
        pattern = str_data(argv[1].as.string);
    }

    char *listing = tnfs_list_directory(path, pattern);
    if (!listing) {
        return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), "", 0)};
    }

    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), listing, runtime_strlen(listing))};
}

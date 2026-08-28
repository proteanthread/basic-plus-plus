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

#include <stdlib.h>
#include <string.h>

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

    BValue res = (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), listing, strlen(listing))};
    free(listing);
    return res;
}

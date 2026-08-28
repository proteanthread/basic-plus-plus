// FILENAME: rmdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, rmdir.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the RMDIR statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/rmdir.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_rmdir_register(void) {
    MicroLibMetadata meta = {
        .name = "RMDIR",
        .category = "Filesystem I/O",
        .syntax = "RMDIR pathname$",
        .help_text = "Removes an empty directory from disk.",
        .error_codes = "Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found"
    };
    microlib_register(&meta);
}

BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (path_val.type != VAL_STRING || !path_val.as.string) {
        err.code = 13;
        err.message = "Type Mismatch in RMDIR (expected string path)";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    if (platform_rmdir(path) != 0) {
        err.code = 75;
        err.message = "Path/File access error";
    }

    str_release(vm_get_str(vm), path_val.as.string);
    return err;
}

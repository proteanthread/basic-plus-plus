// FILENAME: chdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (prefix.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (chdir.h, eval.h, eval.c, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the CHDIR statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/chdir.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_chdir_register(void) {
    MicroLibMetadata meta = {
        .name = "CHDIR",
        .category = "Filesystem I/O",
        .syntax = "CHDIR pathname$",
        .help_text = "Changes the current working directory to the specified path.",
        .error_codes = "Error 2: Syntax Error, Error 76: Path Not Found"
    };
    microlib_register(&meta);
}

BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (path_val.type != VAL_STRING || !path_val.as.string) {
        err.code = 13;
        err.message = "Type Mismatch in CHDIR (expected string path)";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    if (platform_chdir(path) != 0) {
        err.code = 76;
        err.message = "Path not found";
    }

    str_release(vm_get_str(vm), path_val.as.string);
    return err;
}

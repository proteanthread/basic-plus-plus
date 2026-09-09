// FILENAME: mkdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, mkdir.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the MKDIR statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/mkdir.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_mkdir_desc = {
    .name = "MKDIR",
    .category = "Filesystem I/O",
    .syntax = "MKDIR pathname$",
    .description = "Creates a new directory on disk.",
    .error_summary = "Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_mkdir_register(void) {
    lang_desc_register(&g_mkdir_desc);
}

BppError stmt_mkdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (path_val.type != VAL_STRING || !path_val.as.string) {
        err.code = 13;
        err.message = "Type Mismatch in MKDIR (expected string path)";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    if (platform_mkdir(path) != 0) {
        err.code = 75;
        err.message = "Path/File access error";
    }

    str_release(vm_get_str(vm), path_val.as.string);
    return err;
}

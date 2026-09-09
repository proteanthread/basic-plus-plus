// FILENAME: kill.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (destroy.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, kill.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the KILL statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/kill.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"


#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/strings.h"
#include "platform/platform.h"

static const LangDesc g_kill_desc = {
    .name = "KILL",
    .category = "Filesystem I/O",
    .syntax = "KILL filespec",
    .description = "Deletes specified file from disk storage.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_kill_register(void) {
    lang_desc_register(&g_kill_desc);
}
BppError stmt_kill_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (path_val.type != VAL_STRING || !path_val.as.string) {
        if (path_val.type == VAL_STRING && path_val.as.string) {
            str_release((StringContext *)vm_get_str(vm), (BppString *)path_val.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }

    BppString *sref = (BppString *)path_val.as.string;
    const char *path = str_data(sref);
    if (path && platform_remove(path) != 0) {
        err.code = 53;
        err.message = "File not found";
    }

    str_release((StringContext *)vm_get_str(vm), sref);
    return err;
}

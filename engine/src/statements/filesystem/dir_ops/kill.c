// FILENAME: kill.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (destroy.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
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
#include "runtime/micro_lib_metadata.h"

void stmt_kill_register(void) {
    MicroLibMetadata meta = {
        .name = "KILL",
        .category = "Filesystem I/O",
        .syntax = "KILL filespec",
        .help_text = "Deletes specified file from disk storage.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}
#include <stdio.h>
#include <string.h>
#include "runtime/strings.h"

BppError stmt_kill_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

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
    if (path && remove(path) != 0) {
        err.code = 53;
        err.message = "File not found";
    }

    str_release((StringContext *)vm_get_str(vm), sref);
    return err;
}

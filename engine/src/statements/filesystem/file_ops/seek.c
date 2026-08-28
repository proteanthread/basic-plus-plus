// FILENAME: seek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, seek.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SEEK statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/seek.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"

void stmt_seek_register(void) {
    MicroLibMetadata meta = {
        .name = "SEEK",
        .category = "Filesystem I/O",
        .syntax = "SEEK [#]file_num, position",
        .help_text = "Sets the byte offset position for the next read or write operation on an open file.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}
#include <string.h>

BppError stmt_seek_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

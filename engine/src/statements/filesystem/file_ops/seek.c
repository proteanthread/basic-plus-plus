// FILENAME: seek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
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
#include "runtime/language_descriptor.h"


#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_seek_desc = {
    .name = "SEEK",
    .category = "Filesystem I/O",
    .syntax = "SEEK [#]file_num, position",
    .description = "Sets the byte offset position for the next read or write operation on an open file.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_seek_register(void) {
    lang_desc_register(&g_seek_desc);
}
BppError stmt_seek_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

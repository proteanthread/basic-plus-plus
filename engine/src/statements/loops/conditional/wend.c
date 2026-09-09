// FILENAME: wend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, wend.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the WEND statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/wend.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_wend_desc = {
    .name = "WEND",
    .category = "Looping / Control Flow",
    .syntax = "WEND",
    .description = "Marks the end of a WHILE loop block and jumps back to re-evaluate the WHILE condition.",
    .error_summary = "Error 29: WEND Without WHILE",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_wend_register(void) {
    lang_desc_register(&g_wend_desc);
}

BppError stmt_wend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppLineNumber line = 0;
    const char *pos = NULL;
    if (!vm_while_peek(vm, &line, &pos)) {
        err.code = 29; err.message = "WEND Without WHILE";
        return err;
    }

    vm_jump(vm, line, pos);
    return err;
}


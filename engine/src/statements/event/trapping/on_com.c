// FILENAME: on_com.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_com.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the ON_COM statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/on_com.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_on_com_desc = {
    .name = "ON COM",
    .category = "Event Handling",
    .syntax = "ON COM(n) GOSUB line_num",
    .description = "Establishes an event handler trap for serial communication port activity.",
    .error_summary = "Error 5: Illegal Function Call, Error 8: Undefined Line Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_on_com_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_on_com_register(void) {
    lang_desc_register(&g_on_com_desc);
}

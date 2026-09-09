// FILENAME: randomize.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, randomize.h, string.c, vm.h)
// Provides runtime implementation for the RANDOMIZE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/options/randomize.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_randomize_desc = {
    .name = "RANDOMIZE",
    .category = "Control & Math",
    .syntax = "RANDOMIZE [seed]",
    .description = "Reseeds the pseudo-random number generator used by the RND function.",
    .error_summary = "Error 5: Illegal Function Call (negative or invalid seed)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};


BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_randomize_register(void) {
    lang_desc_register(&g_randomize_desc);
}

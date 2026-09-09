// FILENAME: def_usr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (def_usr.h, string.c)
// Provides runtime implementation for the DEF_USR statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/def_usr.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_def_usr_desc = {
    .name = "DEF USR",
    .category = "System & Memory",
    .syntax = "DEF USR[digit%] = address%",
    .description = "Specifies the starting address of an assembly language subroutine called by USR.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_def_usr_register(void) {
    lang_desc_register(&g_def_usr_desc);
}

BppError stmt_def_usr_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}

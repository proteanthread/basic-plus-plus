// FILENAME: with.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, with.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the WITH statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/structure/with.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_with_desc = {
    .name = "WITH",
    .category = "Variables & Memory",
    .syntax = "WITH object \n .member = expression \n ... \n END WITH",
    .description = "Executes a series of statements that reference the members of a single object or UDT.",
    .error_summary = "Error 2: Syntax Error, Error 41: WITH Without END WITH",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_with_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected object or UDT variable name after WITH";
        return err;
    }

    char path[256] = {0};
    size_t len = (tok.length < sizeof(path) - 1) ? tok.length : sizeof(path) - 1;
    runtime_memcpy(path, tok.start, len);

    while (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex);
        size_t plen = runtime_strlen(path);
        if (plen < sizeof(path) - 2) {
            path[plen] = '.';
            path[plen + 1] = '\0';
        }
        BppToken mtok = lex_next(lex);
        if (mtok.type == TOK_IDENT || mtok.type == TOK_KEYWORD) {
            size_t mlen = mtok.length;
            plen = runtime_strlen(path);
            if (plen + mlen < sizeof(path) - 1) {
                runtime_memcpy(path + plen, mtok.start, mlen);
                path[plen + mlen] = '\0';
            }
        }
    }

    vm_with_stack_push(vm, path);
    return err;
}

void stmt_with_register(void) {
    lang_desc_register(&g_with_desc);
}


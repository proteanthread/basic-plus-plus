// FILENAME: shared.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (share.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, shared.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SHARED statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/shared.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "runtime/variables.h"

BppError stmt_shared_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in SHARED statement";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        var_set_shared(var, name);

        BppToken next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_local_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in LOCAL statement";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        var_declare(var, name);

        BppToken next = lex_peek(lex);
        if ((next.type == TOK_KEYWORD && next.as.keyword == KW_AS) ||
            (next.type == TOK_IDENT && next.length == 2 && (next.start[0] == 'A' || next.start[0] == 'a') && (next.start[1] == 'S' || next.start[1] == 's'))) {
            lex_next(lex); // Consume AS
            if (lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_KEYWORD) {
                lex_next(lex); // Consume type
            }
        }

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_static_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    VariableContext *var = vm_get_var(vm);

    while (lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in STATIC statement";
            return err;
        }

        char name[64] = {0};
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);

        var_declare(var, name);

        BppToken next = lex_peek(lex);
        if ((next.type == TOK_KEYWORD && next.as.keyword == KW_AS) ||
            (next.type == TOK_IDENT && next.length == 2 && (next.start[0] == 'A' || next.start[0] == 'a') && (next.start[1] == 'S' || next.start[1] == 's'))) {
            lex_next(lex); // Consume AS
            if (lex_peek(lex).type == TOK_IDENT || lex_peek(lex).type == TOK_KEYWORD) {
                lex_next(lex); // Consume type
            }
        }

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

void stmt_shared_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SHARED",
        .category = "Variables & Memory",
        .syntax = "SHARED variable [, variable...]",
        .help_text = "Grants SUB or FUNCTION procedures access to module-level global variables.",
        .error_codes = "Error 2: Syntax Error, Error 33: Illegal Outside SUB/FUNCTION"
    };
    microlib_register(&meta);
}


// FILENAME: clr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (clr.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CLR statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/clr.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_clr_desc = {
    .name = "CLR",
    .category = "Variables & Memory",
    .syntax = "CLR [var1, var2, ...]",
    .description = "Clears specified variables or all variables to default zero/empty values.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_clr_register(void) {
    lang_desc_register(&g_clr_desc);
}

BppError stmt_clr_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    VariableContext *vc = vm_get_var(vm);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
        // CLR with no args clears all scalar variables
        var_clear_all(vc);
        return err;
    }

    while (1) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in CLR";
            return err;
        }

        char var_name[64];
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, var_tok.start, len);
        var_name[len] = '\0';

        BValue *var = var_lookup(vc, var_name, false);
        if (var) {
            if (var->type == VAL_STRING) {
                if (var->as.string) {
                    str_release(vm_get_str(vm), var->as.string);
                }
                var->as.string = str_create(vm_get_str(vm), "", 0);
            } else if (var->type == VAL_INTEGER || var->type == VAL_NUMBER) {
                var->as.number = 0.0;
            }
        }

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex);
            continue;
        }
        break;
    }

    return err;
}

// FILENAME: clear.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (clear.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CLEAR statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/clear.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "runtime/variables.h"
#include "runtime/file.h"

static const LangDesc g_clear_desc = {
    .name = "CLEAR",
    .category = "Variables & Memory",
    .syntax = "CLEAR [string_space%] | CLEAR [, [mem_limit%] [, stack_size%]]",
    .description = "Frees variable and array memory, closes open files, and sets optional memory limits.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_clear_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        // GW-BASIC style: CLEAR , [mem_limit] [, stack_size]
        lex_next(lex); // Consume first comma
        tok = lex_peek(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_EOL && tok.type != TOK_EOF) {
            eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
        }
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // Consume second comma
            tok = lex_peek(lex);
            if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
                eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
            }
        }
    } else if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        // TRS-80 style: CLEAR string_space
        eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
    }

    VariableContext *vc = vm_get_var(vm);
    if (vc) var_clear_all(vc);

    ArrayContext *ac = vm_get_arr(vm);
    if (ac) arr_clear_all(ac);

    FileContext *fc = vm_get_file(vm);
    if (fc) file_close_all(fc);

    return err;
}

void stmt_clear_register(void) {
    lang_desc_register(&g_clear_desc);
}


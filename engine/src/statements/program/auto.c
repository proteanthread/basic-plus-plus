// FILENAME: auto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (auto.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the AUTO statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/auto.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "memory/memory.h"

static const LangDesc g_auto_desc = {
    .name = "AUTO",
    .category = "Program Mgmt & Editing",
    .syntax = "AUTO [start_line] [,step]",
    .description = "Enables automatic line number generation starting at start_line with step interval.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_auto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BppLineNumber start_line = 0;
    BppLineNumber step = 10;
    bool start_specified = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        start_line = (BppLineNumber)tok.as.number;
        start_specified = true;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            tok = lex_peek(lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(lex);
                step = (BppLineNumber)tok.as.number;
            }
        }
    } else if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(lex);
            step = (BppLineNumber)tok.as.number;
        }
    }

    if (step <= 0) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    if (!start_specified) {
        MemoryContext *mem = vm_get_mem(vm);
        size_t count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &count);
        if (count > 0) {
            start_line = lines[count - 1].line_number + step;
        } else {
            start_line = 10;
        }
    }

    // Store auto line state on VMContext context
    vm_set_auto_line(vm, start_line, step);
    return err;
}

void stmt_auto_register(void) {
    lang_desc_register(&g_auto_desc);
}

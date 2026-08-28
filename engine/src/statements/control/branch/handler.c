// FILENAME: handler.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (handler.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the HANDLER statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/branch/handler.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_handler_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) { start_idx = i; break; }
    }

    // Skip HANDLER block in linear execution
    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan = lex_init(mem, lines[i].text);
        if (!scan) continue;
        BppToken tok = lex_next(scan);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
            BppToken ntok = lex_next(scan);
            if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_HANDLER) {
                vm_jump(vm, lines[i].line_number, lines[i].text);
                lex_shutdown(scan);
                return err;
            }
        }
        lex_shutdown(scan);
    }

    return err;
}

void stmt_handler_register(void) {
    static const MicroLibMetadata meta = {
        .name = "HANDLER",
        .category = "Control Flow",
        .syntax = "HANDLER name$ ... END HANDLER",
        .help_text = "ECMA-116 standard detached exception handler definition.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

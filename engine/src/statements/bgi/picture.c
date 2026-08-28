// FILENAME: picture.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, picture.h, string.c, vm.h)
// Provides runtime implementation for the PICTURE statement in BASIC++.
//
// ---- Includes ----

#include "statements/bgi/picture.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_picture_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    // Consume remaining picture declaration tokens on current line
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) { start_idx = i; found = true; break; }
    }

    if (!found || count == 0) return err;

    // Skip PICTURE definition body in linear execution if END PICTURE block exists
    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan = lex_init(mem, lines[i].text);
        if (!scan) continue;
        BppToken tok = lex_next(scan);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
            BppToken ntok = lex_next(scan);
            if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_PICTURE) {
                vm_jump(vm, lines[i].line_number, lines[i].text);
                lex_shutdown(scan);
                return err;
            }
        }
        lex_shutdown(scan);
    }

    return err;
}

void stmt_picture_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PICTURE",
        .category = "Graphics & Sound",
        .syntax = "PICTURE name [(parameter_list)] ... END PICTURE",
        .help_text = "ECMA-116 standard statement to define a parameterized vector graphic picture macro block.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

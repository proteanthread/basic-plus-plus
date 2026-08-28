// FILENAME: llist.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, llist.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vprinter.h, vprinter.c)
// Provides runtime implementation for the LLIST statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/llist.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "device/vprinter.h"
#include "security/security.h"
#include <string.h>
#include <stdio.h>

BppError stmt_llist_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    BppLineNumber start_line = 0;
    BppLineNumber end_line = 999999999;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_MINUS) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(lex);
            end_line = (BppLineNumber)tok.as.number;
        }
    } else if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        start_line = (BppLineNumber)tok.as.number;
        end_line = start_line;

        tok = lex_peek(lex);
        if (tok.type == TOK_MINUS) {
            lex_next(lex);
            end_line = 999999999;
            tok = lex_peek(lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(lex);
                end_line = (BppLineNumber)tok.as.number;
            }
        }
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    char line_buf[2048];
    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number >= start_line && lines[i].line_number <= end_line) {
            const char *text = lines[i].text ? lines[i].text : "";
            snprintf(line_buf, sizeof(line_buf), "%lld %s\n", (long long)lines[i].line_number, text);
            vprinter_write_str(line_buf);
        }
    }

    return err;
}

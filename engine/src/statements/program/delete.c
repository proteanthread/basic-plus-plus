// FILENAME: delete.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (delete.h, isam.h, isam.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the DELETE statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/delete.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "runtime/micro_lib_metadata.h"
#include "statements/db/isam/isam.h"
#include <string.h>

BppError stmt_delete_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        return stmt_delete_rec_handler(vm, lex);
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    BppLineNumber start_line = 0;
    BppLineNumber end_line = 0;
    bool is_range = false;
    if (tok.type == TOK_MINUS) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(lex);
            end_line = (BppLineNumber)tok.as.number;
            start_line = 0;
            is_range = true;
        } else {
            err.code = ERR_SYNTAX;
            return err;
        }
    } else if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        start_line = (BppLineNumber)tok.as.number;
        end_line = start_line;

        tok = lex_peek(lex);
        if (tok.type == TOK_MINUS) {
            lex_next(lex);
            is_range = true;
            end_line = 999999999;
            tok = lex_peek(lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(lex);
                end_line = (BppLineNumber)tok.as.number;
            }
        }
    } else {
        err.code = ERR_SYNTAX;
        return err;
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (count == 0) {
        err.code = ERR_UNDEFINED_LINE;
        return err;
    }

    if (!is_range) {
        const char *existing = mem_program_get(mem, start_line);
        if (!existing) {
            err.code = ERR_UNDEFINED_LINE;
            return err;
        }
        mem_program_remove(mem, start_line);
    } else {
        // Range deletion
        for (size_t i = 0; i < count; i++) {
            if (lines[i].line_number >= start_line && lines[i].line_number <= end_line) {
                mem_program_remove(mem, lines[i].line_number);
            }
        }
    }

    return err;
}

void stmt_delete_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DELETE",
        .category = "Program Mgmt & Editing",
        .syntax = "DELETE [start_line] [- [end_line]]",
        .help_text = "Deletes specified line numbers or ranges of lines from program memory.",
        .error_codes = "Error 2: Syntax Error, Error 8: Undefined Line Number"
    };
    microlib_register(&meta);
}

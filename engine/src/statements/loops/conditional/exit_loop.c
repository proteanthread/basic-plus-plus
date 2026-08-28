// FILENAME: exit_loop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, exit_loop.h, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the EXIT_LOOP statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/exit_loop.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"

void stmt_exit_loop_register(void) {
    MicroLibMetadata meta = {
        .name = "EXIT",
        .category = "Looping / Control Flow",
        .syntax = "EXIT {FOR|DO|WHILE|SUB|FUNCTION}",
        .help_text = "Prematurely exits an active loop, SUB, or FUNCTION block.",
        .error_codes = "Error 2: Syntax Error, Error 33: Invalid EXIT Scope"
    };
    microlib_register(&meta);
}
#include "device/vdev.h"
#include "security/security.h"
#include "memory/memory.h"
#include <string.h>
#include <ctype.h>

static BppError skip_to_matching_token(VMContext *vm, BppKeywordId end_kw) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    BppLineNumber cur_ln = vm_get_current_line(vm);
    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "Execution state corruption during EXIT scan";
        return err;
    }

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        BppToken tok = lex_next(scan_lex);

        if (tok.type == TOK_KEYWORD && tok.as.keyword == end_kw) {
            lex_shutdown(scan_lex);
            if (i + 1 < count) {
                vm_jump(vm, lines[i + 1].line_number, NULL);
            } else {
                vm_jump(vm, lines[i].line_number + 1, NULL);
            }
            return err;
        }
        lex_shutdown(scan_lex);
    }

    err.code = 33; err.message = "Matching block end not found for EXIT";
    return err;
}

BppError stmt_exit_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_next(lex);
    char name[64] = {0};
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_DO) {
            BppLineNumber line = 0; const char *pos = NULL;
            if (!vm_do_pop(vm, &line, &pos)) {
                err.code = 33; err.message = "EXIT DO without DO";
                return err;
            }
            return skip_to_matching_token(vm, KW_LOOP);
        } else if (tok.as.keyword == KW_FOR) {
            BppForFrame frame;
            if (!vm_for_pop(vm, NULL, &frame)) {
                err.code = 33; err.message = "EXIT FOR without FOR";
                return err;
            }
            return skip_to_matching_token(vm, KW_NEXT);
        } else if (tok.as.keyword == KW_SUB) {
            BppSubFrame frame;
            if (!vm_sub_pop(vm, &frame)) {
                err.code = 33; err.message = "EXIT SUB without SUB";
                return err;
            }
            vm_jump(vm, frame.line, frame.pos);
            return err;
        } else if (tok.as.keyword == KW_FUNCTION) {
            BppSubFrame frame;
            if (!vm_sub_pop(vm, &frame)) {
                err.code = 33; err.message = "EXIT FUNCTION without FUNCTION";
                return err;
            }
            vm_jump(vm, frame.line, frame.pos);
            return err;
        }
    } else if (tok.type == TOK_IDENT) {
        size_t len = (tok.length < sizeof(name) - 1) ? tok.length : sizeof(name) - 1;
        memcpy(name, tok.start, len);
        for (size_t i = 0; i < len; i++) name[i] = (char)toupper((unsigned char)name[i]);

        if (strcmp(name, "DO") == 0) {
            BppLineNumber line = 0; const char *pos = NULL;
            if (!vm_do_pop(vm, &line, &pos)) {
                err.code = 33; err.message = "EXIT DO without DO";
                return err;
            }
            return skip_to_matching_token(vm, KW_LOOP);
        } else if (strcmp(name, "FOR") == 0) {
            BppForFrame frame;
            if (!vm_for_pop(vm, NULL, &frame)) {
                err.code = 33; err.message = "EXIT FOR without FOR";
                return err;
            }
            return skip_to_matching_token(vm, KW_NEXT);
        } else if (strcmp(name, "SUB") == 0) {
            BppSubFrame frame;
            if (!vm_sub_pop(vm, &frame)) {
                err.code = 33; err.message = "EXIT SUB without SUB";
                return err;
            }
            vm_jump(vm, frame.line, frame.pos);
            return err;
        } else if (strcmp(name, "FUNCTION") == 0) {
            BppSubFrame frame;
            if (!vm_sub_pop(vm, &frame)) {
                err.code = 33; err.message = "EXIT FUNCTION without FUNCTION";
                return err;
            }
            vm_jump(vm, frame.line, frame.pos);
            return err;
        }
    }

    err.code = 2; err.message = "Syntax error in EXIT statement";
    return err;
}

BppError stmt_exit_loop_handler(VMContext *vm, LexerContext *lex) {
    return stmt_exit_handler(vm, lex);
}


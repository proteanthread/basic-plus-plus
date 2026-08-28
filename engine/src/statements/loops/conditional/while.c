// FILENAME: while.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the WHILE statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/while.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include <string.h>

void stmt_while_register(void) {
    MicroLibMetadata meta = {
        .name = "WHILE",
        .category = "Looping / Control Flow",
        .syntax = "WHILE condition",
        .help_text = "Executes a series of statements in a loop as long as condition evaluates to non-zero (true).",
        .error_codes = "Error 2: Syntax Error, Error 30: WHILE Without WEND"
    };
    microlib_register(&meta);
}

static BppError skip_to_matching_wend(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int nesting = 0;

    // 1. Try to find matching WEND on the current line first
    BppToken tok = lex_peek(lex);
    while (tok.type != TOK_EOF) {
        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_WHILE) {
                nesting++;
            } else if (tok.as.keyword == KW_WEND) {
                if (nesting > 0) {
                    nesting--;
                } else {
                    // Found matching WEND on the current line!
                    lex_next(lex); // Consume WEND
                    return err;
                }
            }
        }
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // 2. Scan subsequent lines
    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber cur_line = vm_get_current_line(vm);

    size_t cur_idx = 0;
    bool found_cur = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_line) {
            cur_idx = i;
            found_cur = true;
            break;
        }
    }

    if (!found_cur) {
        err.code = 30;
        err.message = "WHILE Without WEND";
        return err;
    }

    for (size_t i = cur_idx + 1; i < count; ++i) {
        LexerContext *scan = lex_init(mem, lines[i].text);
        if (!scan) continue;

        BppToken stok = lex_peek(scan);
        while (stok.type != TOK_EOF) {
            if (stok.type == TOK_KEYWORD) {
                if (stok.as.keyword == KW_WHILE) {
                    nesting++;
                } else if (stok.as.keyword == KW_WEND) {
                    if (nesting > 0) {
                        nesting--;
                    } else {
                        // Found matching WEND on line i
                        lex_next(scan); // Consume WEND
                        BppToken after_tok = lex_peek(scan);
                        if (after_tok.type == TOK_EOL || after_tok.type == TOK_BACKSLASH) {
                            lex_next(scan);
                            after_tok = lex_peek(scan);
                        }
                        if (after_tok.type != TOK_EOF) {
                            ptrdiff_t off = after_tok.start - lines[i].text;
                            vm_jump(vm, lines[i].line_number, lines[i].text + off);
                        } else {
                            if (i + 1 < count) {
                                vm_jump(vm, lines[i + 1].line_number, NULL);
                            } else {
                                vm_jump(vm, lines[i].line_number + 1, NULL);
                            }
                        }
                        lex_shutdown(scan);
                        return err;
                    }
                }
            }
            lex_next(scan);
            stok = lex_peek(scan);
        }
        lex_shutdown(scan);
    }

    err.code = 30;
    err.message = "WHILE Without WEND";
    return err;
}

BppError stmt_while_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppLineNumber cur_line = vm_get_current_line(vm);
    const char *stmt_pos = vm_get_current_stmt_pos(vm);

    BValue cond_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    bool is_true = false;
    if (cond_val.type == VAL_NUMBER) {
        is_true = (cond_val.as.number != 0.0);
    } else if (cond_val.type == VAL_STRING) {
        if (cond_val.as.string) {
            is_true = (str_len(cond_val.as.string) > 0);
            str_release(vm_get_str(vm), cond_val.as.string);
        }
    }

    if (is_true) {
        BppLineNumber top_line = 0;
        const char *top_pos = NULL;
        if (!vm_while_peek(vm, &top_line, &top_pos) || top_line != cur_line) {
            if (!vm_while_push(vm, cur_line, stmt_pos)) {
                err.code = 30; err.message = "WHILE stack overflow";
                return err;
            }
        }
    } else {
        BppLineNumber top_line = 0;
        const char *top_pos = NULL;
        if (vm_while_peek(vm, &top_line, &top_pos) && top_line == cur_line) {
            vm_while_pop(vm, NULL, NULL);
        }

        return skip_to_matching_wend(vm, lex);
    }


    return err;
}


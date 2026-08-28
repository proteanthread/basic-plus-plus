// FILENAME: do.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (do.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the DO statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/do.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"

void stmt_do_register(void) {
    MicroLibMetadata meta = {
        .name = "DO",
        .category = "Looping / Control Flow",
        .syntax = "DO [{WHILE|UNTIL} condition]",
        .help_text = "Initiates a structured DO...LOOP block, optionally evaluating a WHILE or UNTIL pre-condition.",
        .error_codes = "Error 2: Syntax Error, Error 31: DO Without LOOP"
    };
    microlib_register(&meta);
}
#include "device/vdev.h"
#include "security/security.h"
#include "memory/memory.h"
#include <string.h>
#include <ctype.h>

static bool val_is_truthy(BValue val) {
    if (val.type == VAL_STRING) {
        return (val.as.string && str_len(val.as.string) > 0);
    }
    return val.as.number != 0.0;
}static bool is_token_do(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DO) ||
           (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "DO", 2) == 0);
}

static bool is_token_loop(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LOOP) ||
           (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "LOOP", 4) == 0);
}

static bool is_token_while(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHILE) ||
           (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "WHILE", 5) == 0);
}

static bool is_token_until(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_UNTIL) ||
           (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "UNTIL", 5) == 0);
}

static BppError skip_to_matching_loop(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int do_nesting = 0;

    // 1. Try to find matching LOOP on the current line first
    BppToken tok = lex_peek(lex);
    while (tok.type != TOK_EOF) {
        if (is_token_do(tok)) {
            do_nesting++;
        } else if (is_token_loop(tok)) {
            if (do_nesting > 0) {
                do_nesting--;
            } else {
                // Consume LOOP and any optional post-condition
                lex_next(lex);
                BppToken post_kw = lex_peek(lex);
                if (is_token_while(post_kw) || is_token_until(post_kw)) {
                    lex_next(lex);
                    BValue dummy = eval_expression(vm, lex, &err);
                    if (dummy.type == VAL_STRING) {
                        str_release(vm_get_str(vm), dummy.as.string);
                    }
                    memset(&err, 0, sizeof(err));
                }
                return err;
            }
        }
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // 2. Scan subsequent lines
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
        err.code = 31; err.message = "DO without LOOP";
        return err;
    }

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;

        BppToken stok = lex_peek(scan_lex);
        while (stok.type != TOK_EOF) {
            if (is_token_do(stok)) {
                do_nesting++;
            } else if (is_token_loop(stok)) {
                if (do_nesting > 0) {
                    do_nesting--;
                } else {
                    lex_next(scan_lex);
                    BppToken post_kw = lex_peek(scan_lex);
                    if (is_token_while(post_kw) || is_token_until(post_kw)) {
                        lex_next(scan_lex);
                        BValue dummy = eval_expression(vm, scan_lex, &err);
                        if (dummy.type == VAL_STRING) {
                            str_release(vm_get_str(vm), dummy.as.string);
                        }
                        memset(&err, 0, sizeof(err));
                    }
                    BppToken after_tok = lex_peek(scan_lex);
                    if (after_tok.type == TOK_EOL || after_tok.type == TOK_BACKSLASH) {
                        lex_next(scan_lex);
                        after_tok = lex_peek(scan_lex);
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
                    lex_shutdown(scan_lex);
                    return err;
                }
            }
            lex_next(scan_lex);
            stok = lex_peek(scan_lex);
        }
        lex_shutdown(scan_lex);
    }

    err.code = 31; err.message = "DO without LOOP";
    return err;
}

BppError stmt_do_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppLineNumber cur_line = vm_get_current_line(vm);
    const char *stmt_pos = vm_get_current_stmt_pos(vm);

    BppToken tok = lex_peek(lex);
    bool is_while = is_token_while(tok);
    bool is_until = is_token_until(tok);

    if (is_while || is_until) {
        lex_next(lex); // Consume WHILE / UNTIL

        BValue cond = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        bool truthy = val_is_truthy(cond);
        if (cond.type == VAL_STRING) {
            str_release(vm_get_str(vm), cond.as.string);
        }

        bool execute_body = is_while ? truthy : !truthy;
        if (!execute_body) {
            BppLineNumber top_line = 0;
            const char *top_pos = NULL;
            if (vm_do_peek(vm, &top_line, &top_pos) && top_line == cur_line) {
                vm_do_pop(vm, NULL, NULL);
            }
            return skip_to_matching_loop(vm, lex);
        }
    }

    BppLineNumber top_line = 0;
    const char *top_pos = NULL;
    if (!vm_do_peek(vm, &top_line, &top_pos) || top_line != cur_line) {
        if (!vm_do_push(vm, cur_line, stmt_pos)) {
            err.code = 7; err.message = "DO loop stack overflow";
            return err;
        }
    }

    return err;
}



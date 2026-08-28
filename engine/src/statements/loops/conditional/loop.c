// FILENAME: loop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, loop.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LOOP statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/loop.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

void stmt_loop_register(void) {
    MicroLibMetadata meta = {
        .name = "LOOP",
        .category = "Looping / Control Flow",
        .syntax = "LOOP [{WHILE|UNTIL} condition] | LOOP (BASIC09 block opener)",
        .help_text = "Terminates a DO...LOOP block, or opens a BASIC09 structured LOOP...ENDLOOP block.",
        .error_codes = "Error 32: LOOP Without DO, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

static bool val_is_truthy(BValue val) {
    if (val.type == VAL_STRING) {
        return (val.as.string && str_len(val.as.string) > 0);
    }
    return val.as.number != 0.0;
}

static bool has_matching_endloop(VMContext *vm) {
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
    if (!found) return false;

    int loop_nesting = 0;
    for (size_t i = start_idx; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;
        BppToken tok = lex_peek(scan_lex);
        while (tok.type != TOK_EOF) {
            bool is_loop = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LOOP) ||
                           (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "LOOP", 4) == 0);
            bool is_endloop = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ENDLOOP) ||
                             (tok.type == TOK_IDENT && tok.length == 7 && strncasecmp(tok.start, "ENDLOOP", 7) == 0);
            if (is_loop && i > start_idx) {
                loop_nesting++;
            } else if (is_endloop) {
                if (loop_nesting > 0) {
                    loop_nesting--;
                } else {
                    lex_shutdown(scan_lex);
                    return true;
                }
            }
            lex_next(scan_lex);
            tok = lex_peek(scan_lex);
        }
        lex_shutdown(scan_lex);
    }

    return false;
}

BppError stmt_loop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    bool is_while = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHILE) ||
                    (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "WHILE", 5) == 0);
    bool is_until = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_UNTIL) ||
                    (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "UNTIL", 5) == 0);

    // 1. If followed by WHILE or UNTIL, it is strictly a closing DO...LOOP condition
    if (is_while || is_until) {
        BppLineNumber do_line = 0;
        const char *do_pos = NULL;
        if (!vm_do_peek(vm, &do_line, &do_pos)) {
            err.code = 32; err.message = "LOOP without DO";
            return err;
        }

        lex_next(lex); // Consume WHILE / UNTIL

        BValue cond = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        bool truthy = val_is_truthy(cond);
        if (cond.type == VAL_STRING) {
            str_release(vm_get_str(vm), cond.as.string);
        }

        bool continue_loop = is_while ? truthy : !truthy;
        if (continue_loop) {
            vm_jump(vm, do_line, do_pos);
        } else {
            vm_do_pop(vm, NULL, NULL);
        }
        return err;
    }

    // 2. Check if this is the opening of a BASIC09 LOOP...ENDLOOP block
    if (has_matching_endloop(vm)) {
        BppLineNumber top_line = 0;
        const char *top_pos = NULL;
        bool has_do = vm_do_peek(vm, &top_line, &top_pos);
        if (has_do && top_line == vm_get_current_line(vm)) {
            // We looped back to the start of this BASIC09 LOOP block: continue execution
            return err;
        }
        // Opening of a BASIC09 LOOP...ENDLOOP block: push start location onto loop stack
        if (!vm_do_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
            err.code = 32; err.message = "Out of loop stack space";
            return err;
        }
        return err;
    }

    // 3. Closing statement of an unconditional DO...LOOP: jump back to DO line
    BppLineNumber top_line = 0;
    const char *top_pos = NULL;
    bool has_do = vm_do_peek(vm, &top_line, &top_pos);

    if (has_do && top_line != vm_get_current_line(vm)) {
        vm_jump(vm, top_line, top_pos);
        return err;
    }

    err.code = 32;
    err.message = "LOOP without DO";
    return err;
}

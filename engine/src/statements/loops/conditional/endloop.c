// FILENAME: endloop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (endloop.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the ENDLOOP statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/endloop.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "memory/memory.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_endloop_desc = {
    .name = "ENDLOOP",
    .category = "Looping / Control Flow",
    .syntax = "ENDLOOP",
    .description = "Terminates a BASIC09 structured LOOP...ENDLOOP block and loops back to LOOP.",
    .error_summary = "Error 2: Syntax Error, Error 32: ENDLOOP Without LOOP",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_exitif_desc = {
    .name = "EXITIF",
    .category = "Looping / Control Flow",
    .syntax = "EXITIF condition [THEN statement(s)]",
    .description = "Evaluates condition in a BASIC09 LOOP...ENDLOOP block and exits the loop if true.",
    .error_summary = "Error 2: Syntax Error, Error 33: Invalid EXIT Scope",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

#if defined(_WIN32)
#define runtime_strncasecmp runtime_strncasecmp
#endif

void stmt_endloop_register(void) {
    lang_desc_register(&g_endloop_desc);
}

void stmt_exitif_register(void) {
    lang_desc_register(&g_endloop_desc);
}

static bool exitif_val_is_truthy(BValue val) {
    if (val.type == VAL_STRING) {
        return (val.as.string && str_len(val.as.string) > 0);
    }
    return val.as.number != 0.0;
}

BppError stmt_endloop_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppLineNumber loop_line = 0;
    const char *loop_pos = NULL;
    if (!vm_do_peek(vm, &loop_line, &loop_pos)) {
        err.code = 32;
        err.message = "ENDLOOP without LOOP";
        return err;
    }

    // Jump back to loop start
    vm_jump(vm, loop_line, loop_pos);
    return err;
}

BppError stmt_exitif_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppLineNumber cur_ln = vm_get_current_line(vm);

    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    bool truthy = exitif_val_is_truthy(cond);
    if (cond.type == VAL_STRING) {
        str_release(vm_get_str(vm), cond.as.string);
    }

    // Check for optional THEN keyword
    BppToken tok = lex_peek(lex);
    bool has_then = false;
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_THEN) ||
        (tok.type == TOK_IDENT && tok.length == 4 && runtime_strncasecmp(tok.start, "THEN", 4) == 0)) {
        lex_next(lex); // Consume THEN
        has_then = true;
    }

    if (!truthy) {
        // Condition not met: if THEN block present, skip the rest of the line
        if (has_then) {
            while (tok.type != TOK_EOF && tok.type != TOK_EOL) {
                tok = lex_next(lex);
            }
        }
        return err;
    }

    // If condition met and THEN present, execute trailing statement on the line
    if (has_then) {
        tok = lex_peek(lex);
        if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
            // Execute remainder of line
            err = vm_execute_line(vm, lex_get_pos(lex));
            // Advance lexer to EOL
            while (tok.type != TOK_EOF && tok.type != TOK_EOL) {
                tok = lex_next(lex);
            }
        }
    }

    // Pop loop and jump past ENDLOOP
    vm_do_pop(vm, NULL, NULL);

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == cur_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (found) {
        int nest = 0;
        for (size_t i = start_idx + 1; i < count; ++i) {
            LexerContext *scan_lex = lex_init(mem, lines[i].text);
            if (!scan_lex) continue;
            BppToken stok = lex_next(scan_lex);
            while (stok.type != TOK_EOF && stok.type != TOK_EOL) {
                bool is_loop = (stok.type == TOK_KEYWORD && stok.as.keyword == KW_LOOP) ||
                               (stok.type == TOK_IDENT && stok.length == 4 && runtime_strncasecmp(stok.start, "LOOP", 4) == 0);
                bool is_endloop = (stok.type == TOK_KEYWORD && stok.as.keyword == KW_ENDLOOP) ||
                                 (stok.type == TOK_IDENT && stok.length == 7 && runtime_strncasecmp(stok.start, "ENDLOOP", 7) == 0);
                if (is_loop) {
                    nest++;
                } else if (is_endloop) {
                    if (nest > 0) {
                        nest--;
                    } else {
                        lex_shutdown(scan_lex);
                        if (i + 1 < count) {
                            vm_jump(vm, lines[i + 1].line_number, NULL);
                        } else {
                            vm_jump(vm, lines[i].line_number + 1, NULL);
                        }
                        return err;
                    }
                }
                stok = lex_next(scan_lex);
            }
            lex_shutdown(scan_lex);
        }
    }

    return err;
}

// FILENAME: when.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, vm.h, when.h)
// Provides runtime implementation for the WHEN statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/branch/when.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_when_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    // Expect optional ERROR IN or EXCEPTION IN after WHEN
    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) ||
        (tok.type == TOK_IDENT && ((tok.length == 5 && strncasecmp(tok.start, "ERROR", 5) == 0) ||
                                   (tok.length == 9 && strncasecmp(tok.start, "EXCEPTION", 9) == 0)))) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "IN", 2) == 0) {
            lex_next(lex);
        }
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    BppLineNumber current_ln = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (lines[i].line_number == current_ln) {
            start_idx = i;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 11; err.message = "Execution state corruption during WHEN scan";
        return err;
    }

    // Scan forward for USE and END WHEN
    int nest = 0;
    BppLineNumber use_ln = 0;
    const char *use_pos = NULL;
    BppLineNumber end_when_ln = 0;
    const char *end_when_pos = NULL;

    for (size_t i = start_idx + 1; i < count; ++i) {
        LexerContext *scan_lex = lex_init(mem, lines[i].text);
        if (!scan_lex) continue;
        BppToken t = lex_next(scan_lex);
        if (t.type == TOK_NUMBER) {
            t = lex_next(scan_lex);
        }
        bool is_when = (t.type == TOK_KEYWORD && t.as.keyword == KW_WHEN) ||
                       (t.type == TOK_IDENT && t.length == 4 && strncasecmp(t.start, "WHEN", 4) == 0);
        bool is_use  = (t.type == TOK_KEYWORD && t.as.keyword == KW_USE) ||
                       (t.type == TOK_IDENT && t.length == 3 && strncasecmp(t.start, "USE", 3) == 0);
        bool is_end  = (t.type == TOK_KEYWORD && t.as.keyword == KW_END) ||
                       (t.type == TOK_IDENT && t.length == 3 && strncasecmp(t.start, "END", 3) == 0);

        if (is_when) {
            nest++;
        } else if (is_use && nest == 0) {
            use_ln = lines[i].line_number;
            use_pos = lines[i].text;
        } else if (is_end) {
            BppToken nt = lex_next(scan_lex);
            bool is_nt_when = (nt.type == TOK_KEYWORD && nt.as.keyword == KW_WHEN) ||
                              (nt.type == TOK_IDENT && nt.length == 4 && strncasecmp(nt.start, "WHEN", 4) == 0);
            if (is_nt_when) {
                if (nest > 0) {
                    nest--;
                } else {
                    end_when_ln = lines[i].line_number;
                    end_when_pos = lines[i].text;
                    lex_shutdown(scan_lex);
                    break;
                }
            }
        }
        lex_shutdown(scan_lex);
    }

    if (end_when_ln == 0) {
        err.code = 2; err.message = "WHEN ERROR IN without matching END WHEN";
        return err;
    }

    BppTryFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.catch_line = use_ln;
    frame.catch_pos = use_pos;
    frame.end_try_line = end_when_ln;
    frame.end_try_pos = end_when_pos;

    if (!try_stack_push(vm_get_try_stack(vm), frame)) {
        err.code = 24; err.message = "Exception stack overflow";
        return err;
    }

    return err;
}

BppError stmt_use_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    if (!vm) return err;

    if (vm_get_err_code(vm) != 0) {
        vm_clear_err_code(vm);
        return err;
    }

    BppTryFrame frame;
    if (try_stack_pop(vm_get_try_stack(vm), &frame)) {
        if (frame.end_try_line > 0) {
            vm_jump(vm, frame.end_try_line, frame.end_try_pos);
        }
    }
    return err;
}

BppError stmt_end_when_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_when_register(void) {
    static const MicroLibMetadata meta = {
        .name = "WHEN",
        .category = "Control Flow",
        .syntax = "WHEN ERROR IN ... USE ... END WHEN",
        .help_text = "ECMA-116 standard exception handling protection block.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

// FILENAME: on_timer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the ON_TIMER statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/on_timer.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"

void stmt_on_timer_register(void) {
    MicroLibMetadata meta = {
        .name = "ON TIMER",
        .category = "Event Trapping",
        .syntax = "ON TIMER(seconds) GOSUB line_label | TIMER {ON|OFF|STOP}",
        .help_text = "Establishes a periodic timer interrupt subroutine trigger.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
#include <string.h>

BppError stmt_on_timer_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

#include "statements/event/trapping/on_error.h"
#include "statements/event/trapping/on_key.h"
#include "statements/event/trapping/on_com.h"
#include "vm/events_net.h"
#include "memory/memory.h"
#include "runtime/metadata.h"
#include "runtime/strings.h"
#include <string.h>
#include <math.h>

BppError stmt_on_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) {
            lex_next(lex); // Consume ERROR
            return stmt_on_error_handler(vm, lex);
        } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TIMER) {
            lex_next(lex); // Consume TIMER
            return stmt_on_timer_handler(vm, lex);
        } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_KEY) {
            lex_next(lex); // Consume KEY
            return stmt_on_key_handler(vm, lex);
        } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_COM) {
            lex_next(lex); // Consume COM
            return stmt_on_com_handler(vm, lex);
        } else if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_NET || tok.as.keyword == KW_PEER || tok.as.keyword == KW_SNIFF || tok.as.keyword == KW_PACKET || tok.as.keyword == KW_PORT)) {
            BppKeywordId kw = tok.as.keyword;
            lex_next(lex); // Consume keyword
            int port = 0;
            if (kw == KW_PORT && lex_peek(lex).type == TOK_LPAREN) {
                lex_next(lex);
                BValue p_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (p_val.type == VAL_NUMBER) port = (int)p_val.as.number;
                if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
            }
            BppToken branch_tok = lex_next(lex);
            if (branch_tok.type == TOK_KEYWORD && (branch_tok.as.keyword == KW_GOTO || branch_tok.as.keyword == KW_GOSUB)) {
                BValue val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (val.type != VAL_NUMBER) {
                    if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                    err.code = 13; err.message = "Type mismatch in ON line number"; return err;
                }
                if (kw == KW_NET) vm_set_net_trap(vm, val.as.number, 1);
                else if (kw == KW_PEER) vm_set_peer_trap(vm, val.as.number, 1);
                else if (kw == KW_SNIFF || kw == KW_PACKET) vm_set_sniff_trap(vm, val.as.number, 1);
                else if (kw == KW_PORT) vm_set_port_trap(vm, port, val.as.number, 1);
                return err;
            } else {
                err.code = 2; err.message = "Expected GOTO or GOSUB in ON handler"; return err;
            }
        } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_BREAK) {
            lex_next(lex); // Consume BREAK
            BppToken branch_tok = lex_next(lex);
            if (branch_tok.type == TOK_KEYWORD && (branch_tok.as.keyword == KW_GOTO || branch_tok.as.keyword == KW_GOSUB)) {
                BValue val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (val.type != VAL_NUMBER) {
                    if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                    err.code = 13; err.message = "Type mismatch in ON BREAK line number"; return err;
                }
                vm_set_break_trap(vm, val.as.number);
                return err;
            } else {
                err.code = 2; err.message = "Expected GOTO or GOSUB in ON BREAK"; return err;
            }
        }
    }

    // Computed ON expr GOTO | GOSUB line1, line2, ...
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_NUMBER && val.type != VAL_INTEGER) {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        err.code = 13; // Type mismatch
        err.message = "Type mismatch in ON expression";
        return err;
    }

    int index = (int)val.as.number;
    if (val.as.number < 0.0 || val.as.number > 255.0) {
        err.code = 5; // Illegal function call
        err.message = "Illegal function call in ON statement (expression out of 0..255 range)";
        return err;
    }

    BppToken branch_tok = lex_next(lex);
    if (branch_tok.type != TOK_KEYWORD || (branch_tok.as.keyword != KW_GOTO && branch_tok.as.keyword != KW_GOSUB)) {
        err.code = 2; // Syntax error
        err.message = "Expected GOTO or GOSUB in ON statement";
        return err;
    }
    bool is_gosub = (branch_tok.as.keyword == KW_GOSUB);

    int target_idx = 1;
    BppLineNumber target_line = 0.0;
    bool target_found = false;

    while (true) {
        BppToken line_tok = lex_next(lex);
        if (line_tok.type == TOK_NUMBER) {
            if (target_idx == index) {
                target_line = line_tok.as.number;
                target_found = true;
            }
        } else if (line_tok.type == TOK_GLOBAL_LABEL) {
            if (target_idx == index) {
                char label_name[64];
                int l_len = (int)(line_tok.length < sizeof(label_name) - 1 ? line_tok.length : sizeof(label_name) - 1);
                memcpy(label_name, line_tok.as.string, l_len);
                label_name[l_len] = '\0';
                char filename[256];
                if (!metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
                    err.code = 8;
                    err.message = "Undefined global label in ON statement";
                    return err;
                }
                target_found = true;
            }
        } else {
            err.code = 2;
            err.message = "Syntax error in ON line list (expected line number or label)";
            return err;
        }

        target_idx++;
        BppToken peek = lex_peek(lex);
        if (peek.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    if (target_found) {
        if (!mem_program_get(vm_get_mem(vm), target_line)) {
            err.code = 8; // Undefined line number
            err.message = "Undefined line number in ON statement";
            return err;
        }

        if (is_gosub) {
            BppLineNumber current = vm_get_current_line(vm);
            const char *resume_pos = lex_get_pos(lex);
            if (!vm_gosub_push(vm, current, resume_pos)) {
                err.code = 14; // Stack overflow
                err.message = "GOSUB stack overflow";
                return err;
            }
        }

        vm_jump(vm, target_line, NULL);
    }

    return err;
}

BppError stmt_timer_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

extern BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_let_handler(VMContext *vm, LexerContext *lex);

BppError stmt_set_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_SCREEN) {
            lex_next(lex); // consume SCREEN
            return stmt_screen_handler(vm, lex);
        } else if (tok.as.keyword == KW_GRAPHICS) {
            lex_next(lex); // consume GRAPHICS
            return stmt_graphics_handler(vm, lex);
        } else if (tok.as.keyword == KW_MODE) {
            lex_next(lex); // consume MODE
            return stmt_screen_handler(vm, lex);
        }
    }

    return stmt_let_handler(vm, lex);
}

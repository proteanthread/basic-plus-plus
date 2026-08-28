// FILENAME: next.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the NEXT statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/counter/next.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/variables.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include "memory/memory.h"
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

void stmt_next_register(void) {
    static const MicroLibMetadata meta = {
        .name = "NEXT",
        .category = "Looping / Control Flow",
        .syntax = "NEXT [var1[, var2...]] | NEXT [var1 [var2...]]",
        .help_text = "Increments the FOR loop counter variable(s) and loops back if target bound has not been exceeded.",
        .error_codes = "Error 1: NEXT Without FOR, Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

static bool is_same_frame_var(const BppForFrame *frame, const char *name) {
    if (!frame || !name) return false;
    if (strcasecmp(frame->var_name, name) == 0) return true;
    for (int k = 0; k < frame->var_count - 1 && k < 7; k++) {
        if (strcasecmp(frame->extra_vars[k], name) == 0) return true;
    }
    return false;
}

static void consume_same_frame_vars(VMContext *vm, LexerContext *lex, const BppForFrame *frame) {
    while (true) {
        BppToken nxt = lex_peek(lex);
        if (nxt.type == TOK_COMMA) {
            LexerContext *look_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (!look_lex) break;
            lex_next(look_lex); // Consume comma
            BppToken var_tok = lex_next(look_lex);
            bool same = false;
            if (var_tok.type == TOK_IDENT || var_tok.type == TOK_KEYWORD) {
                char nvar[64] = {0};
                size_t nvlen = (var_tok.length < sizeof(nvar) - 1) ? var_tok.length : sizeof(nvar) - 1;
                memcpy(nvar, var_tok.start, nvlen);
                same = is_same_frame_var(frame, nvar);
            }
            lex_shutdown(look_lex);
            if (same) {
                lex_next(lex); // Consume comma
                lex_next(lex); // Consume identifier
            } else {
                break;
            }
        } else if (nxt.type == TOK_IDENT || nxt.type == TOK_KEYWORD) {
            char nvar[64] = {0};
            size_t nvlen = (nxt.length < sizeof(nvar) - 1) ? nxt.length : sizeof(nvar) - 1;
            memcpy(nvar, nxt.start, nvlen);
            if (is_same_frame_var(frame, nvar)) {
                lex_next(lex); // Consume space-separated identifier
            } else {
                break; // Belongs to outer loop
            }
        } else {
            break;
        }
    }
}

BppError stmt_next_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    while (true) {
        char target_var[64] = {0};
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            lex_next(lex);
            size_t tlen = (tok.length < sizeof(target_var) - 1) ? tok.length : sizeof(target_var) - 1;
            memcpy(target_var, tok.start, tlen);
        }

        BppForFrame frame;
        if (!vm_for_peek(vm, target_var[0] ? target_var : NULL, &frame)) {
            err.code = 26; err.message = "NEXT Without FOR";
            return err;
        }

        double cur_val;
        if (frame.cached_var_ptr && (frame.cached_var_ptr->type == VAL_NUMBER || frame.cached_var_ptr->type == VAL_INTEGER)) {
            frame.cached_var_ptr->as.number += frame.step;
            cur_val = frame.cached_var_ptr->as.number;
            for (int i = 0; i < frame.var_count - 1 && i < 7; i++) {
                BValue new_val = { .type = VAL_NUMBER, .as.number = cur_val };
                var_assign(vm_get_var(vm), frame.extra_vars[i], new_val);
            }
        } else {
            VariableContext *vc = vm_get_var(vm);
            BValue *val = var_lookup(vc, frame.var_name, true);
            cur_val = (val && (val->type == VAL_NUMBER || val->type == VAL_INTEGER)) ? val->as.number : 0.0;
            cur_val += frame.step;
            BValue new_val = { .type = VAL_NUMBER, .as.number = cur_val };
            var_assign(vc, frame.var_name, new_val);
            for (int i = 0; i < frame.var_count - 1 && i < 7; i++) {
                var_assign(vc, frame.extra_vars[i], new_val);
            }
        }

        bool done = (frame.step >= 0.0) ? (cur_val > frame.target) : (cur_val < frame.target);
        if (done) {
            vm_for_pop(vm, frame.var_name, NULL);
            consume_same_frame_vars(vm, lex, &frame);

            BppToken sep = lex_peek(lex);
            if (sep.type == TOK_COMMA) {
                lex_next(lex); // Consume comma and advance to outer loop counter
                continue;
            } else if (sep.type == TOK_IDENT || sep.type == TOK_KEYWORD) {
                // Space-separated next variable for outer loop (e.g. NEXT J I)
                continue;
            }
        } else {
            consume_same_frame_vars(vm, lex, &frame);
            vm_jump(vm, frame.line, frame.pos);
        }
        break;
    }

    return err;
}

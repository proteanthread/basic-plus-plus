// FILENAME: ctrl_common.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (checkbox.c, optionbutton.c, label.c, frame.c, combobox.c, hscrollbar.c, vscrollbar.c, drivelistbox.c, dirlistbox.c, filelistbox.c, timercontrol.c)
// NEEDS: libcore (types.h)
// Provides shared argument parser for UI widget controls in BASIC++.

#ifndef CTRL_COMMON_H
#define CTRL_COMMON_H

#include "types/types.h"
#include "eval/eval.h"

static inline int parse_ctrl_args(VMContext *vm, LexerContext *lex, BValue *args, int max_args, BppError *err) {
    int count = 0;
    while (count < max_args) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) break;
        args[count] = eval_expression(vm, lex, err);
        if (err->code != 0) break;
        count++;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }
    return count;
}

#endif // CTRL_COMMON_H

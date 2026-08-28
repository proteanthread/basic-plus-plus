// FILENAME: vbdos_controls.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, vbdos_controls.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the VBDOS_CONTROLS statement in BASIC++.
//
// ---- Includes ----

#ifndef VBDOS_CONTROLS_H
#define VBDOS_CONTROLS_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_checkbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_optionbutton_handler(VMContext *vm, LexerContext *lex);
BppError stmt_label_handler(VMContext *vm, LexerContext *lex);
BppError stmt_frame_handler(VMContext *vm, LexerContext *lex);
BppError stmt_combobox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hscrollbar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vscrollbar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drivelistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dirlistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_filelistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_timercontrol_handler(VMContext *vm, LexerContext *lex);

void stmt_vbdos_controls_register(void);

#ifdef __cplusplus
}
#endif

#endif // VBDOS_CONTROLS_H

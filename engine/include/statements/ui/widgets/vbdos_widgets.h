// FILENAME: vbdos_widgets.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, vbdos_widgets.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VBDOS_WIDGETS statement in BASIC++.
//
// ---- Includes ----

#ifndef VBDOS_WIDGETS_H
#define VBDOS_WIDGETS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_button_handler(VMContext *vm, LexerContext *lex);
BppError stmt_menu_handler(VMContext *vm, LexerContext *lex);
BppError stmt_textbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_listbox_handler(VMContext *vm, LexerContext *lex);

void stmt_vbdos_widgets_register(void);

#ifdef __cplusplus
}
#endif

#endif // VBDOS_WIDGETS_H

// FILENAME: stmt_translate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, stmt_translate.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h)
// Declares TRANSLATE statement handler for in-place string translation (Wang 2200).
//
// ---- Includes ----

#ifndef BPP_STMT_TRANSLATE_H
#define BPP_STMT_TRANSLATE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "lexer/lexer.h"

void stmt_translate_register(void);
BppError stmt_translate_handler(VMContext *vm, LexerContext *lex);

#endif // BPP_STMT_TRANSLATE_H

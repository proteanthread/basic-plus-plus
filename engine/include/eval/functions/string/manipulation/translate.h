// FILENAME: translate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, translate.c, dispatch_call.c, stmt_translate.c)
// NEEDS: libkernel (vm.h, eval.h)
// Declares TRANSLATE$ character mapping function and helper for Wang 2200.
//
// ---- Includes ----

#ifndef BPP_TRANSLATE_H
#define BPP_TRANSLATE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"

void func_translate_register(void);
BValue func_translate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void translate_string_in_place(char *dest, size_t max_len, const char *src, size_t src_len, const char *from_str, size_t from_len, const char *to_str, size_t to_len, size_t *out_len);

#endif // BPP_TRANSLATE_H

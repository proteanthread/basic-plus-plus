// FILENAME: dispatch_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch.c, dispatch_call.c, dispatch_check.c)
// NEEDED BY: libengine (dispatch_special.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides core logic and interface definitions for dispatch_internal within BASIC++.
//
// ---- Includes ----

#ifndef DISPATCH_INTERNAL_H
#define DISPATCH_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "hal/hal.h"


#include "eval/eval.h"
#include "eval/eval_internal.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "runtime/funcreg.h"
#include "runtime/map.h"
#include "runtime/variables.h"
#include "security/security.h"
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif

//
// ---- Internal Helpers & Prototypes ----

void normalize_func_name(const char *in, char *out, size_t max_len);
bool dispatch_handle_special(VMContext *vm, const char *uname, LexerContext *lex, bool has_parens, BValue *out_res, BppError *err);

#endif // DISPATCH_INTERNAL_H

// FILENAME: sub_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sub.c, sub_exec.c, sub_invoke.c, sub_lookup.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the SUB_INTERNAL statement in BASIC++.
//
// ---- Includes ----

#ifndef SUB_INTERNAL_H
#define SUB_INTERNAL_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/struct.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/arrays.h"
#include "runtime/map.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "scope/scope.h"
#include "security/security.h"
#include "statements/oop/sub.h"
#include "statements/variables/declaration/def.h"
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Internal Data Structures ----

typedef struct FormalParam {
    char name[64];
    bool is_byref;
    bool is_optional;
    bool is_array;
    char type_name[64];
    char default_expr[128];
} FormalParam;

//
// ---- Internal Function Prototypes ----

int parse_formal_params(MemoryContext *mem, const char *line_text, FormalParam *params, int max_params);

int parse_call_args(VMContext *vm, LexerContext *lex, FormalParam *formal_params, int param_count,
                    BValue *out_args, char out_arg_names[][64], bool *out_is_byref, int max_args, BppError *out_err);

#endif // SUB_INTERNAL_H

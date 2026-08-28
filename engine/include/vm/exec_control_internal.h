// FILENAME: exec_control_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control.c, exec_line.c, exec_postfix.c)
// NEEDED BY: libengine (exec_stmt.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements bytecode virtual machine execution and state for exec_control_internal.
//
// ---- Includes ----

#ifndef EXEC_CONTROL_INTERNAL_H
#define EXEC_CONTROL_INTERNAL_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device/vprinter.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/collections.h"
#include "runtime/file.h"
#include "runtime/override.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "scope/scope.h"
#include "security/security.h"
#include "statements/oop/sub.h"
#include "stmt/stmt.h"
#include "stmt/stmt_handlers.h"
#include "vm/exec_internal.h"
#include "vm/vm.h"
#include "vm/vm_internal.h"

//
// ---- Internal Declarations ----

bool is_postfix_exempt_keyword(BppKeywordId kw);
BppError dispatch_gosub_target(VMContext *vm, LexerContext *lex, BppLineNumber line);
BppError execute_single_statement(VMContext *vm, LexerContext *lex);

#endif // EXEC_CONTROL_INTERNAL_H

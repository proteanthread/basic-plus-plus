// FILENAME: exec_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_interrupt.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for exec_internal.
//
// ---- Includes ----

#ifndef VM_EXEC_INTERNAL_H
#define VM_EXEC_INTERNAL_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bios/bios.h"
#include "core/struct.h"
#include "debug/logger.h"
#include "device/bus.h"
#include "device/usb.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "device/vprinter.h"
#include "eval/ast.h"
#include "eval/eval.h"
#include "eval/eval_internal.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "module/module.h"
#include "platform/platform.h"
#include "runtime/collections.h"
#include "runtime/file.h"
#include "runtime/keyword_props.h"
#include "runtime/metadata.h"
#include "runtime/override.h"
#include "runtime/spec.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#include "scope/scope.h"
#include "security/security.h"
#include "statements/db/isam/isam.h"
#include "statements/dialect/meta/alias.h"
#include "statements/dialect/meta/keyword.h"
#include "statements/dialect/module_stmt.h"
#include "statements/dialect/meta/override.h"
#include "statements/dialect/meta/scope.h"
#include "statements/oop/sub.h"
#include "statements/system/hardware/interrupt.h"
#include "statements/ui/widgets/vbdos_controls.h"
#include "statements/ui/widgets/vbdos_widgets.h"
#include "statements/variables/declaration/def.h"
#include "stmt/stmt.h"
#include "stmt/stmt_handlers.h"
#include "types/config.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Constants & Definitions ----

#define MAX_PARAMS 8

//
// ---- VM Function Declarations ----

void        vm_register_alias(VMContext *vm, const char *name, const char *expansion);
const char *vm_lookup_alias(VMContext *vm, const char *name);
void        vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg);
void        vm_trigger_error_trap(VMContext *vm, int err_code, BppLineNumber line, const char *curr_pos, const char *next_pos);
void        vm_trigger_event_polling(VMContext *vm);
bool        vm_check_watchdog(VMContext *vm, BppError *err);
void        vm_build_data_table(VMContext *vm);
BValue      eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);

void gosub_stack_clear(GosubStack *stack);
void for_stack_clear(ForStack *stack);
void while_stack_clear(WhileStack *stack);
void do_stack_clear(DoStack *stack);
size_t try_stack_count(TryStack *stack);

//
// ---- Internal Prototypes Across Exec Sub-Modules ----

void register_core_statements(VMContext *vm);
BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok);
BppError skip_metadata_block(VMContext *vm, LexerContext *lex, const char *block_type);
BppError execute_custom_keyword_statement(VMContext *vm, LexerContext *lex, BppKeywordId kw);
BppError dispatch_gosub_target(VMContext *vm, LexerContext *lex, BppLineNumber line);
bool is_postfix_exempt_keyword(BppKeywordId kw);
BppError execute_single_statement(VMContext *vm, LexerContext *lex);

#endif // VM_EXEC_INTERNAL_H

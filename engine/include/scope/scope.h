/**
 * @file scope.h
 * @brief Public interface header for SCOPE subsystem and language governance definitions.
 *
 * 1. WHAT IT DOES:
 * Declares data structures (BppScopeState, BppHookInfo, BppScopeBlock) and public API functions for SCOPE statement handling, keyword isolation, execution hooks, symbol privacy, and namespace rules.
 *
 * 2. WHY IT EXISTS:
 * Provides modular language governance, sandboxing, and namespace isolation across dialects and modules.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Scope structures are managed per VMContext or frame stack with zero-initialized memory layouts.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'scope'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Increase capacity constants (MAX_DISABLED_KEYWORDS, MAX_HOOKS, MAX_SCOPE_BLOCKS).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Hook enum types (HOOK_BEFORE, HOOK_AFTER, HOOK_OVERRIDE) and function signatures.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard SCOPE_H and direct header include dependencies.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/scope/scope.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef SCOPE_H
#define SCOPE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#define MAX_DISABLED_KEYWORDS 128
#define MAX_HOOKS 64
#define MAX_PRIVATE_SYMBOLS 128
#define MAX_PROTECTED_SYMBOLS 128
#define MAX_SCOPE_BLOCKS 32
#define MAX_NAMESPACE_DEPTH 16

typedef enum {
    HOOK_BEFORE = 1,
    HOOK_AFTER,
    HOOK_OVERRIDE
} BppHookType;

typedef struct {
    char        target_cmd[64];
    BppHookType type;
    char        target_label[64];
    BppLineNumber target_line;
} BppScopeHook;

typedef struct {
    char disabled_keywords[MAX_DISABLED_KEYWORDS][64];
    int  disabled_count;

    BppScopeHook hooks[MAX_HOOKS];
    int          hook_count;

    char private_symbols[MAX_PRIVATE_SYMBOLS][64];
    int  private_count;

    char protected_symbols[MAX_PROTECTED_SYMBOLS][64];
    int  protected_count;

    char namespace_stack[MAX_NAMESPACE_DEPTH][64];
    int  namespace_depth;

    int  block_depth;
} BppScopeState;

/**
 * @brief Initialize scope state inside VMContext.
 */
void scope_init(VMContext *vm);

/**
 * @brief Reset/clear all scope rules.
 */
void scope_clear(VMContext *vm);

/* Keyword Disable/Enable Capability */
bool scope_keyword_disable(VMContext *vm, const char *keyword);
bool scope_keyword_enable(VMContext *vm, const char *keyword);
bool scope_is_keyword_disabled(VMContext *vm, const char *keyword);

/* Execution Hooks Capability */
bool scope_register_hook(VMContext *vm, const char *cmd, BppHookType type, const char *target_label, BppLineNumber line);
const BppScopeHook *scope_lookup_hook(VMContext *vm, const char *cmd, BppHookType type);
void scope_clear_hooks(VMContext *vm);

/* Visibility & Symbol Protection */
bool scope_set_symbol_private(VMContext *vm, const char *symbol);
bool scope_is_symbol_private(VMContext *vm, const char *symbol);
bool scope_protect_symbol(VMContext *vm, const char *symbol);
bool scope_is_symbol_protected(VMContext *vm, const char *symbol);

/* Namespace Scoping */
bool scope_namespace_enter(VMContext *vm, const char *ns_name);
bool scope_namespace_exit(VMContext *vm);
const char *scope_get_current_namespace(VMContext *vm);

/* Block Scoping (SCOPE BEGIN / END) */
bool scope_block_push(VMContext *vm);
bool scope_block_pop(VMContext *vm);
int  scope_get_block_depth(VMContext *vm);

#endif /* SCOPE_H */

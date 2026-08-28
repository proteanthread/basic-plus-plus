// FILENAME: variables.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext
// NEEDS: libcore (memory.h, memory.c, strings.h, strings.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for variables within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_VARIABLES_H
#define RUNTIME_VARIABLES_H

#include <stdbool.h>
#include "types/types.h"
#include "memory/memory.h"
#include "runtime/strings.h"

// Opaque Variable Context
typedef struct VariableContext VariableContext;

// @brief Initialize the variable context.
VariableContext *var_init(MemoryContext *mem, StringContext *str);

// @brief Shutdown the variable context and free all stored variables.
void var_shutdown(VariableContext *ctx);

// @brief Look up a variable by name.
// @param name The variable name (e.g. "A%", "MYVAR$").
// @param create_if_missing If true, implicitly declare variable if not found.
// @return Pointer to the BValue, or NULL if not found/error.
BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing);

// @brief Explicitly declare a variable (bypasses OPTION EXPLICIT check).
BValue *var_declare(VariableContext *ctx, const char *name);

// @brief Set the value of a variable.
// @return true on success, false on type mismatch or explicit check failure.
bool var_assign(VariableContext *ctx, const char *name, BValue val);

// @brief Clear all variables (implements CLEAR / CLR / NEW).
void var_clear_all(VariableContext *ctx);
void var_mark_common(VariableContext *ctx, const char *name);
bool var_is_common(VariableContext *ctx, const char *name);
void var_clear_for_chain(VariableContext *ctx);

// @brief Clear all local variables belonging to a specific prefix scope.
void var_clear_scope(VariableContext *ctx, const char *prefix);
void var_set_scope(VariableContext *ctx, const char *scope);
void var_set_shared(VariableContext *ctx, const char *name);

// @brief Enable or disable strict variable declaration (OPTION EXPLICIT).
void var_set_explicit(VariableContext *ctx, bool enable);
bool var_is_explicit(VariableContext *ctx);
void var_set_namespace(VariableContext *ctx, const char *ns);
void var_set_case_sensitive(VariableContext *ctx, bool enable);

// @brief Configure implicit type overrides for a range of letters.
void var_set_def_type(VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type);

// @brief Get the active implicit type for a specific letter.
ValueType var_get_def_type(VariableContext *ctx, const char *scope, char letter);

void var_print_all(VariableContext *ctx, void *vdev_ptr);

bool var_serialize(VariableContext *ctx, void *fp);
bool var_deserialize(VariableContext *ctx, void *fp);

#endif // RUNTIME_VARIABLES_H

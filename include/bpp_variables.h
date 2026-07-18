/**
 * @file bpp_variables.h
 * @brief Variable storage, scoping, and lookup API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures and lookup functions for BASIC variables (A-Z variables,
 *   typed variables, and dynamically named variables).
 * - Why it exists: Manages the lifetime, scope, and values of program variables during execution,
 *   supporting implicit declarations, explicit definitions, and suffix-based typing.
 * - Why it works this way: It uses an opaque VariableContext containing fast-path arrays for
 *   single-letter variables (A-Z) and a hash/search structure for named variables. Suffixes
 *   (%, &, !, #, $) determine the dynamic BValue type.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal lookup structures (hash map vs sorted array).
 * - What cannot be changed: Opaque VariableContext handle.
 * - What to expect: Requesting a variable that does not exist will implicitly create it with default values,
 *   unless OPTION EXPLICIT is enabled.
 * - What to do if something breaks: If variables return wrong types or are not found, check the suffix
 *   parsing logic and verify scoping stack states.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Variable names are case-insensitive. Suffixes are stripped or mapped to ValueType.
 * - Portability concerns: Case-insensitive comparisons must conform to basic ASCII rules.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add array variables, structured type fields, or local scopes.
 * - How to write external extensions: External plugins read and write variables through the VM context using these lookup APIs.
 */

#ifndef BPP_VARIABLES_H
#define BPP_VARIABLES_H

#include <stdbool.h>
#include "bpp_types.h"
#include "bpp_memory.h"
#include "bpp_strings.h"

/* Opaque Variable Context */
typedef struct VariableContext VariableContext;

/**
 * @brief Initialize the variable context.
 */
VariableContext *var_init(MemoryContext *mem, StringContext *str);

/**
 * @brief Shutdown the variable context and free all stored variables.
 */
void var_shutdown(VariableContext *ctx);

/**
 * @brief Look up a variable by name.
 * @param name The variable name (e.g. "A%", "MYVAR$").
 * @param create_if_missing If true, implicitly declare variable if not found.
 * @return Pointer to the BValue, or NULL if not found/error.
 */
BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing);

/**
 * @brief Explicitly declare a variable (bypasses OPTION EXPLICIT check).
 */
BValue *var_declare(VariableContext *ctx, const char *name);

/**
 * @brief Set the value of a variable.
 * @return true on success, false on type mismatch or explicit check failure.
 */
bool var_assign(VariableContext *ctx, const char *name, BValue val);

/**
 * @brief Clear all variables (implements CLEAR / CLR / NEW).
 */
void var_clear_all(VariableContext *ctx);

/**
 * @brief Clear all local variables belonging to a specific prefix scope.
 */
void var_clear_scope(VariableContext *ctx, const char *prefix);
void var_set_scope(VariableContext *ctx, const char *scope);
void var_set_shared(VariableContext *ctx, const char *name);

/**
 * @brief Enable or disable strict variable declaration (OPTION EXPLICIT).
 */
void var_set_explicit(VariableContext *ctx, bool enable);
bool var_is_explicit(VariableContext *ctx);
void var_set_namespace(VariableContext *ctx, const char *ns);
void var_set_case_sensitive(VariableContext *ctx, bool enable);

/**
 * @brief Configure implicit type overrides for a range of letters.
 */
void var_set_def_type(VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type);

/**
 * @brief Get the active implicit type for a specific letter.
 */
ValueType var_get_def_type(VariableContext *ctx, const char *scope, char letter);

void var_print_all(VariableContext *ctx, void *vdev_ptr);

bool var_serialize(VariableContext *ctx, void *fp);
bool var_deserialize(VariableContext *ctx, void *fp);

#endif /* BPP_VARIABLES_H */

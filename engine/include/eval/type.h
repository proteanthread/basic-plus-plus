/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_type.h
 * @brief Expression Evaluator Type Checking & Coercion Micro-Library Header.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares type inspection and boolean coercion routines for BASIC values.
 * - Why it exists: Decouples type assertion helpers out of monolithic expression evaluators.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional coercion helpers.
 * - What cannot be changed: Truthiness conventions (-1.0 for true, 0.0 for false).
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - None.
 */

#ifndef EVAL_EVAL_TYPE_H
#define EVAL_EVAL_TYPE_H

#include "types/types.h"
#include <stdbool.h>

bool eval_type_is_numeric(const BValue *val);
bool eval_type_is_string(const BValue *val);
bool eval_type_is_truthy(const BValue *val);
double eval_bool_to_basic(bool condition);

#endif /* EVAL_EVAL_TYPE_H */

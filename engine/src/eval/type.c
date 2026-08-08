/**
 * @file type.c
 * @brief BValue type checking, type matching, and automatic implicit coercion helpers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements type validation helpers (`eval_check_type()`, `eval_coerce_type()`, `eval_type_name()`).
 *
 * 2. WHY IT EXISTS:
 * Enforces strict type safety rules and handles implicit conversions between numbers and strings when allowed by dialect rules.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates BValue type tags; emits ERR_TYPE_MISMATCH (error 13) when incompatible types are passed to handlers.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'eval'. Includes "eval/eval.h", "types/types.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support strict typing flags or custom object type coercion.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Type mismatch error code (ERR_TYPE_MISMATCH = 13) per BASIC standards.
 *
 * 8. WHAT TO EXPECT:
 * Returns BppError code or type string name.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check type enum tag values in BValue definition.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized BValue pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Bounded string name lookups.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/eval.h
 * - engine/include/types/types.h
 */

/**
 * @file eval_type.c
 * @brief Expression Evaluator Type Checking & Coercion Micro-Library.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements value type verification and truthiness coercion for BASIC++.
 * - Why it exists: Isolated micro-library for expression type management.
 * - Why it works this way: BASIC boolean truth is represented as double -1.0 (true) and 0.0 (false).
 *
 * SECTION 2: SAFE ZONES FOR EDITING
 * - Safe to add new value type conversion functions.
 *
 * SECTION 3: CRITICAL INVARIANTS
 * - eval_bool_to_basic must strictly return -1.0 for true and 0.0 for false.
 *
 * SECTION 4: EXPECTED SIDE-EFFECTS & BEHAVIORS
 * - Pure functions with no state mutations.
 *
 * SECTION 5: WHAT TO DO IF SOMETHING BREAKS
 * - Check value type enum definitions in types/types.h.
 *
 * SECTION 6: ASSUMPTIONS
 * - Pointer inputs are non-NULL.
 *
 * SECTION 7: PORTABILITY CONCERNS
 * - Strictly portable C17.
 *
 * SECTION 8: FUTURE EXPANSIONS
 * - Automatic string-to-number coercions.
 */

#include "eval/type.h"

bool eval_type_is_numeric(const BValue *val) {
    return val && val->type == VAL_NUMBER;
}

bool eval_type_is_string(const BValue *val) {
    return val && val->type == VAL_STRING;
}

bool eval_type_is_truthy(const BValue *val) {
    if (!val) return false;
    if (val->type == VAL_NUMBER) {
        return val->as.number != 0.0;
    }
    return false;
}

double eval_bool_to_basic(bool condition) {
    return condition ? -1.0 : 0.0;
}

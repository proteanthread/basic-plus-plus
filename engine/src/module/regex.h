/**
 * @file regex.h
 * @brief Public interface header for REGEX module function registration and pattern matching.
 *
 * 1. WHAT IT DOES:
 * Declares public initialization function mod_regex_init() for registering REGEX.MATCH, REGEX.REPLACE, and REGEX.FIND functions.
 *
 * 2. WHY IT EXISTS:
 * Exposes API surface for activating the regular expression module inside the BASIC++ runtime.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to module initialization signature int mod_*_init(void *rt).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'module'. Includes "types/types.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Declare additional regex match state query routines.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature mod_regex_init(void *rt).
 *
 * 8. WHAT TO EXPECT:
 * Returns 0 on successful registration.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard MODULE_REGEX_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid runtime context pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/module/regex.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 */
#ifndef MOD_REGEX_H
#define MOD_REGEX_H

#ifdef __cplusplus
extern "C" {
#endif

void register_regex_functions(void);

#ifdef __cplusplus
}
#endif

#endif /* MOD_REGEX_H */

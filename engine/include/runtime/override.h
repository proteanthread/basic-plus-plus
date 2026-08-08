/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file override.h
 * @brief Header for OVERRIDE method and statement behavior replacement subsystem.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Registers statement and procedure override hooks allowing dynamic behavior redirection.
 * - Why it exists: Enables aspect-oriented dynamic routing and custom statement replacement semantics.
 * - Why it works this way: Maintains an in-memory lookup table of target overrides per VM context.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal structure of BppOverrideEntry and max table capacities.
 * - What cannot be changed: Public lookup and registration API signatures.
 * - What to expect: Fast bounded lookup array zero-initialized by default.
 * - What to do if something breaks: Verify string copy bounds and zero-initialization.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 standard compliance across MSVC and GCC targets.
 * - Portability concerns: Thread-safe within single VMContext execution threads.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add user function/closure override callbacks.
 */

#ifndef RUNTIME_OVERRIDE_H
#define RUNTIME_OVERRIDE_H

#include "types/types.h"
#include "vm/vm.h"
#include <stdbool.h>

#define MAX_OVERRIDE_ENTRIES 64

typedef struct {
    char target_name[64];
    long target_line;
    char replacement_sub[64];
    bool is_active;
} BppOverrideEntry;

typedef struct {
    BppOverrideEntry entries[MAX_OVERRIDE_ENTRIES];
    int count;
} BppOverrideTable;

void override_init(VMContext *vm);
void override_clear(VMContext *vm);
bool override_register(VMContext *vm, const char *target_name, long target_line, const char *replacement_sub);
const BppOverrideEntry *override_lookup(VMContext *vm, const char *target_name);

#endif /* RUNTIME_OVERRIDE_H */

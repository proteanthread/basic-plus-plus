/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file host.h
 * @brief Public interface header for VM host interface abstraction layer (BppHostInterface).
 *
 * 1. WHAT IT DOES:
 * Declares `BppHostInterface` function-pointer structure and helper creation function `vm_create_host_interface()`.
 *
 * 2. WHY IT EXISTS:
 * Decouples execution statements, plugins, and embedded scripting runtimes from internal VM structures.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Groups I/O, variable, and error callback function pointers together with an opaque `user_data` pointer.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Included in `vm/vm.h` and statement handlers. Includes "types/errors.h", <stddef.h>, <stdbool.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add optional callback function pointers with NULL checks in caller implementations.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Existing function pointer parameter types and return signatures.
 *
 * 8. WHAT TO EXPECT:
 * Declares `BppHostInterface` struct and `vm_create_host_interface()` API prototype.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard VM_HOST_H and include prerequisites (`<stddef.h>`, `<stdbool.h>`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * `user_data` carries host-specific context pointer (typically `VMContext*`).
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit pointer safe (`void* user_data`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/host.c
 * Prerequisite Header Files:
 * - engine/include/types/errors.h
 */

#ifndef VM_HOST_H
#define VM_HOST_H

#include <stddef.h>
#include <stdbool.h>
#include "types/errors.h"

typedef struct BppHostInterface {
    void *user_data;  /* Opaque host context (VMContext* for BASIC++) */

    /* Output Callbacks */
    void (*output)(void *user_data, const char *text);
    void (*output_char)(void *user_data, char c);

    /* Input Callbacks */
    int (*input_line)(void *user_data, char *buf, int max_len);
    int (*input_char)(void *user_data);

    /* Variable Access Callbacks */
    double (*get_numeric_var)(void *user_data, const char *name);
    void (*set_numeric_var)(void *user_data, const char *name, double val);
    const char *(*get_string_var)(void *user_data, const char *name);
    void (*set_string_var)(void *user_data, const char *name, const char *val);

    /* Error Reporting Callback */
    void (*report_error)(void *user_data, BppErrorCode code, const char *msg);

    /* Virtual Device Access */
    void *(*get_device)(void *user_data, const char *device_name);
} BppHostInterface;

#endif /* VM_HOST_H */

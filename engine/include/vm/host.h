/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Defines the BppHostInterface function-pointer contract for decoupling.
 * Why it exists: Enables VM, Python, Lua, or embedded hosts to drive statement & expression execution.
 * Why it works this way: Abstracts I/O, variables, error reporting, and devices into callback pointers.
 * What can be changed: Add optional function pointers to the interface with default NULL checks.
 * What cannot be changed: Core input/output/error callback signatures.
 * What to expect: Clean decoupling of execution logic from interpreter VM internals.
 * What to do if something breaks: Ensure host callbacks handle NULL inputs gracefully.
 * Assumptions: user_data carries host-specific context pointer (e.g. VMContext*).
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add asynchronous event dispatch callbacks.
 * External extension hooks: Embedded hosts populate BppHostInterface to run BASIC++ logic.
 */

#ifndef BPP_HOST_H
#define BPP_HOST_H

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

#endif /* BPP_HOST_H */

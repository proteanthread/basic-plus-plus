// FILENAME: host.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (host.c)
// NEEDS: libkernel (errors.h)
// Implements bytecode virtual machine execution and state for host.
//
// ---- Includes ----

#ifndef VM_HOST_H
#define VM_HOST_H

#include <stddef.h>
#include <stdbool.h>
#include "types/errors.h"

typedef struct BppHostInterface {
    void *user_data;  // Opaque host context (VMContext* for BASIC++)

    // Output Callbacks
    void (*output)(void *user_data, const char *text);
    void (*output_char)(void *user_data, char c);

    // Input Callbacks
    int (*input_line)(void *user_data, char *buf, int max_len);
    int (*input_char)(void *user_data);

    // Variable Access Callbacks
    double (*get_numeric_var)(void *user_data, const char *name);
    void (*set_numeric_var)(void *user_data, const char *name, double val);
    const char *(*get_string_var)(void *user_data, const char *name);
    void (*set_string_var)(void *user_data, const char *name, const char *val);

    // Error Reporting Callback
    void (*report_error)(void *user_data, BppErrorCode code, const char *msg);

    // Virtual Device Access
    void *(*get_device)(void *user_data, const char *device_name);
} BppHostInterface;

#endif // VM_HOST_H

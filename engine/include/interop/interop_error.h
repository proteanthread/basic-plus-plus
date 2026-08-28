// FILENAME: interop_error.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h, interop_core.c, interop_error.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_error within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_ERROR_H
#define INTEROP_ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Standard BASIC++ error codes mapped for interop.
#define INTEROP_ERROR_OK             0
#define INTEROP_ERROR_SYNTAX         2
#define INTEROP_ERROR_ILLEGAL        5
#define INTEROP_ERROR_OVERFLOW       6
#define INTEROP_ERROR_DIVZERO        11
#define INTEROP_ERROR_TYPEMISMATCH   13

// Structured error details.
typedef struct InteropError {
    int code;
    const char* message;
    int line_number;
    const char* source_file;
} InteropError;

// @brief Create and record a new thread-local error.
// @param code The error code.
// @param message The error message.
// @param line_number The line number where the error occurred.
// @param source_file The source file where the error occurred.
void interop_error_create(int code, const char* message, int line_number, const char* source_file);

// @brief Clear the thread-local error state.
void interop_error_clear(void);

// @brief Get the last thread-local error.
// @return Pointer to the thread-local InteropError struct.
const InteropError* interop_error_get_last(void);

#endif // INTEROP_ERROR_H

// FILENAME: snprintf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libserver, libstandard
// NEEDS: platform, memory
// Freestanding snprintf formatting engine for integers and floats.
//
// ---- Includes ----

#ifndef RUNTIME_FORMAT_SNPRINTF_H
#define RUNTIME_FORMAT_SNPRINTF_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Formats data into a bounded string buffer. Returns total characters formatted (excluding terminating null byte).
int runtime_snprintf(char *str, size_t size, const char *format, ...);

// @brief Variadic list version of runtime_snprintf.
int runtime_vsnprintf(char *str, size_t size, const char *format, va_list ap);

// @brief Unbounded formatted output (use with caution; runtime_snprintf is preferred).
int runtime_sprintf(char *str, const char *format, ...);

// @brief Variadic list version of runtime_sprintf.
int runtime_vsprintf(char *str, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_FORMAT_SNPRINTF_H



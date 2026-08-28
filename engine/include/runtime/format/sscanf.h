// FILENAME: sscanf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (sscanf.c)
// NEEDS: platform, memory
// Freestanding sscanf string scanning and parsing engine.
//
// ---- Includes ----

#ifndef RUNTIME_FORMAT_SSCANF_H
#define RUNTIME_FORMAT_SSCANF_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses formatted input from a string buffer. Returns number of input items assigned.
int runtime_sscanf(const char *str, const char *format, ...);

// @brief Variadic list version of runtime_sscanf.
int runtime_vsscanf(const char *str, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_FORMAT_SSCANF_H

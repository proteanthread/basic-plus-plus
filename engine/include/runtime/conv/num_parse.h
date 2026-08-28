// FILENAME: num_parse.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libstandard
// NEEDS: platform, memory
// Freestanding numeric parsing and conversion routines.
//
// ---- Includes ----

#ifndef RUNTIME_CONV_NUM_PARSE_H
#define RUNTIME_CONV_NUM_PARSE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Converts initial portion of str to int.
int runtime_atoi(const char *str);

// @brief Converts initial portion of str to long.
long runtime_atol(const char *str);

// @brief Converts initial portion of str to long long (int64_t).
int64_t runtime_atoll(const char *str);

// @brief Parses string to long integer with base (2..36 or 0 for auto 0x/0b/0/decimal).
long runtime_strtol(const char *nptr, char **endptr, int base);

// @brief Parses string to unsigned long integer with base.
unsigned long runtime_strtoul(const char *nptr, char **endptr, int base);

// @brief Parses string to int64_t with base.
int64_t runtime_strtoll(const char *nptr, char **endptr, int base);

// @brief Parses string to uint64_t with base.
uint64_t runtime_strtoull(const char *nptr, char **endptr, int base);

// @brief Formats integer into string buffer in given base (2..36). Returns length written.
size_t runtime_int_to_str(int64_t value, char *buf, size_t buf_size, int base);

// @brief Formats unsigned integer into string buffer in given base (2..36). Returns length written.
size_t runtime_uint_to_str(uint64_t value, char *buf, size_t buf_size, int base, bool uppercase);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_CONV_NUM_PARSE_H

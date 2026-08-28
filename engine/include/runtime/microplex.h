// FILENAME: microplex.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (microplex.c)
// NEEDED BY: libengine (string_fn.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for microplex within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_MICROPLEX_H
#define RUNTIME_MICROPLEX_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// @brief Interleaves characters of two null-terminated strings into out_buf.
// @param s1 First input string.
// @param len1 Length of first string.
// @param s2 Second input string.
// @param len2 Length of second string.
// @param out_buf Output buffer to receive interleaved string.
// @param out_size Total capacity of out_buf (including null terminator).
// @return Number of characters written (excluding null terminator), or 0 on failure.
size_t microplex_string(const char *s1, size_t len1, const char *s2, size_t len2, char *out_buf, size_t out_size);

// @brief Interleaves bit-fields from two 32-bit unsigned integer arrays into a destination array.
// @param src_a First source array.
// @param len_a Length of first source array.
// @param src_b Second source array.
// @param len_b Length of second source array.
// @param dst Destination array buffer.
// @param dst_len Maximum elements in dst.
// @param bit_width Width of bitfield per slice (1 to 32 bits).
// @return Number of elements populated in dst, or 0 on error.
size_t microplex_bits(const uint32_t *src_a, size_t len_a, const uint32_t *src_b, size_t len_b, uint32_t *dst, size_t dst_len, int bit_width);

#endif // RUNTIME_MICROPLEX_H

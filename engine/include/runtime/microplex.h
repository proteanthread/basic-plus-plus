/* =====================================================================
 * What it does: Declarations for the character-level and bit-level microplexing runtime engine (MICROPLEX$ and BITMUX).
 * Why it exists: Provides low-level interleaving of string characters and bitfields for IoT telemetry, packed byte-streams, and binary protocols.
 * Why it works this way: Operates directly on byte buffers and 32-bit integers with explicit boundary checks and C17 memory safety.
 * What can be changed: Additional stride or mask variants for bit-level microplexing.
 * What cannot be changed: Memory safety invariants, standard C17 type signatures (uint32_t, size_t).
 * What to expect: Pure ASCII/binary microplexing output without host recursion or memory leaks.
 * What to do if something breaks: Verify buffer allocations and bounds checks in rt_microplex.c.
 * Assumptions: Pointers passed to microplex helpers are valid for the specified lengths.
 * Portability concerns: Pure C17 compliant, safe on 32-bit and 64-bit platforms.
 * Future expansions: Multi-stream character microplexing (3+ strings).
 * ===================================================================== */

#ifndef RUNTIME_MICROPLEX_H
#define RUNTIME_MICROPLEX_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Interleaves characters of two null-terminated strings into out_buf.
 * @param s1 First input string.
 * @param len1 Length of first string.
 * @param s2 Second input string.
 * @param len2 Length of second string.
 * @param out_buf Output buffer to receive interleaved string.
 * @param out_size Total capacity of out_buf (including null terminator).
 * @return Number of characters written (excluding null terminator), or 0 on failure.
 */
size_t microplex_string(const char *s1, size_t len1, const char *s2, size_t len2, char *out_buf, size_t out_size);

/**
 * @brief Interleaves bit-fields from two 32-bit unsigned integer arrays into a destination array.
 * @param src_a First source array.
 * @param len_a Length of first source array.
 * @param src_b Second source array.
 * @param len_b Length of second source array.
 * @param dst Destination array buffer.
 * @param dst_len Maximum elements in dst.
 * @param bit_width Width of bitfield per slice (1 to 32 bits).
 * @return Number of elements populated in dst, or 0 on error.
 */
size_t microplex_bits(const uint32_t *src_a, size_t len_a, const uint32_t *src_b, size_t len_b, uint32_t *dst, size_t dst_len, int bit_width);

#endif /* RUNTIME_MICROPLEX_H */

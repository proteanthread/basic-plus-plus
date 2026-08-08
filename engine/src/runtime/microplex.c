/* =====================================================================
 * What it does: Implements character-level string interleaving (MICROPLEX$) and bitfield array interleaving (BITMUX).
 * Why it exists: Enables ultra-dense binary protocol packing, hardware telemetry multiplexing, and byte-stream transformation in resource-restricted environments.
 * Why it works this way: Character interleaving alternates single bytes between input strings; bit-level interleaving shifts and masks n-bit fields sequentially.
 * What can be changed: Internal bit manipulation algorithms and buffer sizing limits.
 * What cannot be changed: Memory boundary checks, C17 standard compliance, pointer safety.
 * What to expect: Fast, non-allocating execution into caller-supplied storage buffers.
 * What to do if something breaks: Check bit_width bounds (1 to 32) and out_size capacity limits.
 * Assumptions: Caller provides valid target memory buffers.
 * Portability concerns: Platform-independent bitwise logic; safe for 32-bit and 64-bit systems.
 * Future expansions: Custom bit masks and multi-array bit multiplexing.
 * ===================================================================== */

#include "runtime/microplex.h"
#include <string.h>

size_t microplex_string(const char *s1, size_t len1, const char *s2, size_t len2, char *out_buf, size_t out_size) {
    if (!s1 || !s2 || !out_buf || out_size == 0) return 0;

    size_t i = 0, j = 0, k = 0;
    size_t max_chars = out_size - 1; /* Reserve for null terminator */

    while ((i < len1 || j < len2) && k < max_chars) {
        if (i < len1 && k < max_chars) {
            out_buf[k++] = s1[i++];
        }
        if (j < len2 && k < max_chars) {
            out_buf[k++] = s2[j++];
        }
    }

    out_buf[k] = '\0';
    return k;
}

size_t microplex_bits(const uint32_t *src_a, size_t len_a, const uint32_t *src_b, size_t len_b, uint32_t *dst, size_t dst_len, int bit_width) {
    if (!src_a || !src_b || !dst || dst_len == 0) return 0;
    if (bit_width < 1 || bit_width > 32) bit_width = 16; /* Default to 16-bit half-words */

    uint32_t mask = (bit_width >= 32) ? 0xFFFFFFFFu : ((1u << bit_width) - 1u);
    size_t min_len = (len_a < len_b) ? len_a : len_b;
    size_t count = (min_len < dst_len) ? min_len : dst_len;

    for (size_t i = 0; i < count; i++) {
        uint32_t val_a = src_a[i] & mask;
        uint32_t val_b = src_b[i] & mask;

        if (bit_width <= 16) {
            dst[i] = val_a | (val_b << bit_width);
        } else {
            dst[i] = val_a ^ val_b; /* Bitwise combining for full 32-bit width */
        }
    }

    return count;
}

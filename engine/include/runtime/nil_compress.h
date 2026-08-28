// FILENAME: nil_compress.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nil_compress.c)
// NEEDED BY: libengine (func_nil_compress.c)
// NEEDS: platform, memory
// Implements RFC 51 COMSTRING and COMPVECTOR run-length/token stream compression.
//
// ---- Includes ----

#ifndef RUNTIME_NIL_COMPRESS_H
#define RUNTIME_NIL_COMPRESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// COMSTRING: Encodes whitespace runs, line terminators and control characters
size_t nil_comstring_encode(const char *in, size_t in_len, char *out, size_t max_out);
size_t nil_comstring_decode(const char *in, size_t in_len, char *out, size_t max_out);

// COMPVECTOR: Run-length compression for byte and numeric vector streams
size_t nil_compvector_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t max_out);
size_t nil_compvector_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t max_out);

#endif // RUNTIME_NIL_COMPRESS_H

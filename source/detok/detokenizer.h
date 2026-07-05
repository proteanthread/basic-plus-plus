/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: detokenizer.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Generic API for detokenizing binary proprietary BASIC formats
 *    (e.g., GW-BASIC, QBASIC) into plaintext .BAS files.
 * ===================================================================== */

#ifndef BASICPP_DETOKENIZER_H
#define BASICPP_DETOKENIZER_H

#include <stdio.h>

typedef enum DetokFormat {
    DETOK_FMT_GWBASIC,
    DETOK_FMT_QBASIC,
    DETOK_FMT_AUTO
} DetokFormat;

// Main API function for detokenizing a binary file stream to a text stream.
// Returns 0 on success, non-zero on failure.
int detokenize_stream(FILE *in, FILE *out, DetokFormat format);

#endif // BASICPP_DETOKENIZER_H

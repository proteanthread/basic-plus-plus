/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Defines the centralized BppErrorCode enum mapping GW-BASIC / QBASIC error codes.
 * Why it exists: Eliminates scattered magic numbers in statement handlers, VM, and runtime.
 * Why it works this way: Provides type-safe error codes for VM and external library consumers.
 * What can be changed: Add new error codes while preserving standard numbers (1-76+).
 * What cannot be changed: Numerical values for standard GW-BASIC/QBASIC error codes.
 * What to expect: Clean programmatic error inspection across all BASIC++ libraries.
 * What to do if something breaks: Check error code mapping against documentation.
 * Assumptions: Values fit within standard signed int.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add extended subsystem-specific error code ranges.
 * External extension hooks: Plugins return BppError populated with BppErrorCode values.
 */

#ifndef BPP_ERRORS_H
#define BPP_ERRORS_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    BPP_ERR_OK                       = 0,
    BPP_ERR_NEXT_WITHOUT_FOR         = 1,
    BPP_ERR_SYNTAX                   = 2,
    BPP_ERR_RETURN_WITHOUT_GOSUB     = 3,
    BPP_ERR_OUT_OF_DATA              = 4,
    BPP_ERR_ILLEGAL_FUNCTION_CALL    = 5,
    BPP_ERR_OVERFLOW                 = 6,
    BPP_ERR_OUT_OF_MEMORY            = 7,
    BPP_ERR_UNDEFINED_LINE           = 8,
    BPP_ERR_SUBSCRIPT_OUT_OF_RANGE   = 9,
    BPP_ERR_REDIM_ARRAY              = 10,
    BPP_ERR_DIVISION_BY_ZERO         = 11,
    BPP_ERR_ILLEGAL_DIRECT           = 12,
    BPP_ERR_TYPE_MISMATCH            = 13,
    BPP_ERR_OUT_OF_STRING_SPACE      = 14,
    BPP_ERR_STRING_TOO_LONG          = 15,
    BPP_ERR_STRING_FORMULA_TOO_COMPLEX = 16,
    BPP_ERR_CANNOT_CONTINUE          = 17,
    BPP_ERR_UNDEFINED_USER_FUNCTION  = 18,
    BPP_ERR_NO_RESUME                = 19,
    BPP_ERR_RESUME_WITHOUT_ERROR     = 20,
    BPP_ERR_DEVICE_TIMEOUT           = 24,
    BPP_ERR_DEVICE_FAULT             = 25,
    BPP_ERR_OUT_OF_PAPER             = 27,
    BPP_ERR_WHILE_WITHOUT_WEND       = 29,
    BPP_ERR_WEND_WITHOUT_WHILE       = 29,
    BPP_ERR_FOR_WITHOUT_NEXT         = 30,
    BPP_ERR_FIELD_NOT_DEFINED        = 35,
    BPP_ERR_BAD_FILE_NUMBER          = 52,
    BPP_ERR_FILE_NOT_FOUND           = 53,
    BPP_ERR_BAD_FILE_MODE            = 54,
    BPP_ERR_FILE_ALREADY_OPEN        = 55,
    BPP_ERR_FIELD_OVERFLOW           = 56,
    BPP_ERR_DEVICE_IO_ERROR          = 57,
    BPP_ERR_FILE_ALREADY_EXISTS      = 58,
    BPP_ERR_BAD_RECORD_LENGTH        = 59,
    BPP_ERR_DISK_FULL                = 61,
    BPP_ERR_INPUT_PAST_END           = 62,
    BPP_ERR_BAD_RECORD_NUMBER        = 63,
    BPP_ERR_BAD_FILE_NAME            = 64,
    BPP_ERR_TOO_MANY_FILES           = 67,
    BPP_ERR_DEVICE_UNAVAILABLE       = 68,
    BPP_ERR_PERMISSION_DENIED        = 70,
    BPP_ERR_DISK_NOT_READY           = 71,
    BPP_ERR_DISK_MEDIA_ERROR         = 72,
    BPP_ERR_ADVANCED_FEATURE_DISABLED = 73,
    BPP_ERR_RENAME_ACROSS_DISKS      = 74,
    BPP_ERR_PATH_FILE_ACCESS_ERROR   = 75,
    BPP_ERR_PATH_NOT_FOUND           = 76
} BppErrorCode;

#endif /* BPP_ERRORS_H */

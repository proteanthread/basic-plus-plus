// FILENAME: errors.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: platform, memory
// Provides core logic and interface definitions for errors within BASIC++.
//
// ---- Includes ----

#ifndef TYPES_ERRORS_H
#define TYPES_ERRORS_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    ERR_OK                       = 0,
    ERR_NEXT_WITHOUT_FOR         = 1,
    ERR_SYNTAX                   = 2,
    ERR_RETURN_WITHOUT_GOSUB     = 3,
    ERR_OUT_OF_DATA              = 4,
    ERR_ILLEGAL_FUNCTION_CALL    = 5,
    ERR_OVERFLOW                 = 6,
    ERR_OUT_OF_MEMORY            = 7,
    ERR_UNDEFINED_LINE           = 8,
    ERR_SUBSCRIPT_OUT_OF_RANGE   = 9,
    ERR_REDIM_ARRAY              = 10,
    ERR_DIVISION_BY_ZERO         = 11,
    ERR_ILLEGAL_DIRECT           = 12,
    ERR_TYPE_MISMATCH            = 13,
    ERR_OUT_OF_STRING_SPACE      = 14,
    ERR_STRING_TOO_LONG          = 15,
    ERR_STRING_FORMULA_TOO_COMPLEX = 16,
    ERR_CANNOT_CONTINUE          = 17,
    ERR_UNDEFINED_USER_FUNCTION  = 18,
    ERR_NO_RESUME                = 19,
    ERR_RESUME_WITHOUT_ERROR     = 20,
    ERR_DEVICE_TIMEOUT           = 24,
    ERR_DEVICE_FAULT             = 25,
    ERR_OUT_OF_PAPER             = 27,
    ERR_WHILE_WITHOUT_WEND       = 29,
    ERR_WEND_WITHOUT_WHILE       = 29,
    ERR_FOR_WITHOUT_NEXT         = 30,
    ERR_FIELD_NOT_DEFINED        = 35,
    ERR_BAD_FILE_NUMBER          = 52,
    ERR_FILE_NOT_FOUND           = 53,
    ERR_BAD_FILE_MODE            = 54,
    ERR_FILE_ALREADY_OPEN        = 55,
    ERR_FIELD_OVERFLOW           = 56,
    ERR_DEVICE_IO_ERROR          = 57,
    ERR_FILE_ALREADY_EXISTS      = 58,
    ERR_BAD_RECORD_LENGTH        = 59,
    ERR_DISK_FULL                = 61,
    ERR_INPUT_PAST_END           = 62,
    ERR_BAD_RECORD_NUMBER        = 63,
    ERR_BAD_FILE_NAME            = 64,
    ERR_TOO_MANY_FILES           = 67,
    ERR_DEVICE_UNAVAILABLE       = 68,
    ERR_PERMISSION_DENIED        = 70,
    ERR_DISK_NOT_READY           = 71,
    ERR_DISK_MEDIA_ERROR         = 72,
    ERR_ADVANCED_FEATURE_DISABLED = 73,
    ERR_RENAME_ACROSS_DISKS      = 74,
    ERR_PATH_FILE_ACCESS_ERROR   = 75,
    ERR_PATH_NOT_FOUND           = 76
} BppErrorCode;

#endif // TYPES_ERRORS_H

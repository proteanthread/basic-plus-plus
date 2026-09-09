// FILENAME: builtin_descriptors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libboot
// NEEDS: libcore (language_descriptor.h, builtin_descriptors.h)
// Implements authoritative descriptor definitions for built-in statements and functions.

#include "runtime/descriptors/builtin_descriptors.h"
#include <stddef.h>

static const LangDesc g_builtin_descriptors[] = {
    // -----------------------------------------------------------------------
    // Built-in Functions
    // -----------------------------------------------------------------------
    {
        .name = "ASC", .category = "String Functions", .syntax = "ASC(string_expr$)",
        .description = "Returns the numeric ASCII / byte value (0 to 255) of the first character of string_expr$.",
        .error_summary = "Error 5: Illegal Function Call (empty string)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "CBOOL", .category = "Type Conversion", .syntax = "CBOOL(expr)",
        .description = "Converts numeric or string expression to a boolean value (-1 for true, 0 for false).",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CCUR", .category = "Type Conversion", .syntax = "CCUR(expr)",
        .description = "Converts numeric expression to fixed-point currency value (scaled 64-bit integer with 4 decimal digits).",
        .error_summary = "Error 6: Overflow, Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "QBASIC, VB for DOS, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CDBL", .category = "Type Conversion", .syntax = "CDBL(expr)",
        .description = "Converts numeric expression to double-precision 64-bit floating-point value.",
        .error_summary = "Error 6: Overflow, Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CINT", .category = "Type Conversion", .syntax = "CINT(expr)",
        .description = "Converts numeric expression to 16-bit or 32-bit integer, rounding fractional part (banker's rounding).",
        .error_summary = "Error 6: Overflow, Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CSNG", .category = "Type Conversion", .syntax = "CSNG(expr)",
        .description = "Converts numeric expression to single-precision 32-bit floating-point value.",
        .error_summary = "Error 6: Overflow, Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CSTR", .category = "Type Conversion", .syntax = "CSTR(expr)",
        .description = "Converts expression of any type to its standard string representation.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "VB for DOS, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "CVD", .category = "Binary & Byte Conversion", .syntax = "CVD(8byte_str$)",
        .description = "Decodes an 8-byte fielded binary string into a double-precision floating-point number.",
        .error_summary = "Error 5: Illegal Function Call (length != 8)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "CVI", .category = "Binary & Byte Conversion", .syntax = "CVI(2byte_str$)",
        .description = "Decodes a 2-byte fielded binary string into a 16-bit signed integer.",
        .error_summary = "Error 5: Illegal Function Call (length != 2)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "CVS", .category = "Binary & Byte Conversion", .syntax = "CVS(4byte_str$)",
        .description = "Decodes a 4-byte fielded binary string into a single-precision floating-point number.",
        .error_summary = "Error 5: Illegal Function Call (length != 4)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "MKDSTR", .category = "Binary & Byte Conversion", .syntax = "MKDSTR(dbl_val#) | MKD$(dbl_val#)",
        .description = "Encodes a double-precision floating-point number into an 8-byte fielded binary string.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "MKISTR", .category = "Binary & Byte Conversion", .syntax = "MKISTR(int_val%) | MKI$(int_val%)",
        .description = "Encodes a 16-bit signed integer into a 2-byte fielded binary string.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "MKSSTR", .category = "Binary & Byte Conversion", .syntax = "MKSSTR(sng_val!) | MKS$(sng_val!)",
        .description = "Encodes a single-precision floating-point number into a 4-byte fielded binary string.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/builtins/string_fn.c"
    },
    {
        .name = "CABS", .category = "Math & Complex Numbers", .syntax = "CABS(z)",
        .description = "Returns the magnitude / absolute value of complex number z (sqrt(re^2 + im^2)).",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CARG", .category = "Math & Complex Numbers", .syntax = "CARG(z)",
        .description = "Returns the phase angle / argument in radians of complex number z (atan2(im, re)).",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CEXP", .category = "Math & Complex Numbers", .syntax = "CEXP(z)",
        .description = "Computes the complex exponential e^z of complex number z.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CLOG", .category = "Math & Complex Numbers", .syntax = "CLOG(z)",
        .description = "Computes the complex natural logarithm ln(z) of complex number z.",
        .error_summary = "Error 5: Illegal Function Call (z=0), Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CONJ", .category = "Math & Complex Numbers", .syntax = "CONJ(z)",
        .description = "Returns the complex conjugate of complex number z (re - i*im).",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CPOW", .category = "Math & Complex Numbers", .syntax = "CPOW(base, exponent)",
        .description = "Raises complex number base to complex or scalar power exponent.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "CSQR", .category = "Math & Complex Numbers", .syntax = "CSQR(z)",
        .description = "Computes the principal square root of complex number z.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "REAL", .category = "Math & Complex Numbers", .syntax = "REAL(z)",
        .description = "Returns the real component of complex number or scalar z.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "IMAG", .category = "Math & Complex Numbers", .syntax = "IMAG(z)",
        .description = "Returns the imaginary component of complex number or scalar z.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "HI", .category = "Bitwise & Binary", .syntax = "HI(val%)",
        .description = "Extracts the high-order byte or word of numeric expression val%.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "LO", .category = "Bitwise & Binary", .syntax = "LO(val%)",
        .description = "Extracts the low-order byte or word of numeric expression val%.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "LGT", .category = "Math Functions", .syntax = "LGT(x)",
        .description = "Computes the common logarithm (base-10 logarithm) of positive number x.",
        .error_summary = "Error 5: Illegal Function Call (x <= 0)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/math/math.c"
    },
    {
        .name = "MED", .category = "Statistical Functions", .syntax = "MED(val1, val2, ...)",
        .description = "Returns the median (middle) value among the supplied numeric arguments.",
        .error_summary = "Error 5: Illegal Function Call (no arguments)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "NUM", .category = "Introspection & Conversion", .syntax = "NUM(expr)",
        .description = "Extracts or counts the numeric elements in expression or collection.",
        .error_summary = "Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/type.c"
    },
    {
        .name = "PEEKB", .category = "Hardware & Memory", .syntax = "PEEKB(address)",
        .description = "Reads an unsigned 8-bit byte from physical or virtual memory address.",
        .error_summary = "Error 5: Illegal Function Call, Error 70: Permission Denied",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_UNSAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/memory/alloc.c"
    },
    {
        .name = "ICASE$", .category = "String Functions", .syntax = "ICASE$(str$)",
        .description = "Inverts the case of each alphabetic character in string str$ (lower to upper, upper to lower).",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/string/strops.c"
    },
    {
        .name = "MCASE$", .category = "String Functions", .syntax = "MCASE$(str$)",
        .description = "Converts string str$ to mixed/title case with first letter of each word capitalized.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/string/strops.c"
    },
    {
        .name = "TCASE$", .category = "String Functions", .syntax = "TCASE$(str$)",
        .description = "Converts string str$ to Title Case following standard publication casing rules.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/string/strops.c"
    },
    {
        .name = "REVERSE$", .category = "String Functions", .syntax = "REVERSE$(str$)",
        .description = "Returns string str$ with all characters in reverse order.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/string/strops.c"
    },
    {
        .name = "EDIT$", .category = "String Functions", .syntax = "EDIT$(str$, action_mask%)",
        .description = "Transforms string str$ according to bitmask actions (trim, compress spaces, lowercase, etc.).",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/string/strops.c"
    },
    {
        .name = "SIZE", .category = "Introspection", .syntax = "SIZE(var_or_type)",
        .description = "Returns the memory size in bytes occupied by the specified variable, array, or user-defined type.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "DATE", .category = "Date & Time", .syntax = "DATE or DATE()",
        .description = "Returns the current calendar date formatted according to system locale or integer epoch day.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/dateserial.c"
    },
    {
        .name = "DAY$", .category = "Date & Time", .syntax = "DAY$ [(date_expr)]",
        .description = "Returns the weekday name string (e.g. 'Monday') for current or specified date.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/day.c"
    },
    {
        .name = "HOURS", .category = "Date & Time", .syntax = "HOURS [(time_expr)]",
        .description = "Returns the hour component (0 to 23) of the current or specified time.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/hour.c"
    },
    {
        .name = "MINUTES", .category = "Date & Time", .syntax = "MINUTES [(time_expr)]",
        .description = "Returns the minute component (0 to 59) of the current or specified time.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/minute.c"
    },
    {
        .name = "SECONDS", .category = "Date & Time", .syntax = "SECONDS [(time_expr)]",
        .description = "Returns the second component (0 to 59) of the current or specified time.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/second.c"
    },
    {
        .name = "MONTH$", .category = "Date & Time", .syntax = "MONTH$ [(date_expr)]",
        .description = "Returns the full calendar month name string (e.g. 'September') for current or specified date.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/functions/datetime/month.c"
    },
    {
        .name = "TIMEZONE$", .category = "Date & Time", .syntax = "TIMEZONE$",
        .description = "Returns the host system timezone descriptive name string (e.g. 'Mountain Daylight Time').",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/time/plat_time.c"
    },
    {
        .name = "TZ$", .category = "Date & Time", .syntax = "TZ$",
        .description = "Returns the short timezone abbreviation string (e.g. 'MDT', 'UTC', 'EST').",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/time/plat_time.c"
    },
    {
        .name = "TZ", .category = "Date & Time", .syntax = "TZ",
        .description = "Returns the numeric timezone UTC offset in minutes.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/time/plat_time.c"
    },
    {
        .name = "MAT_IDN", .category = "Matrix Operations", .syntax = "MAT_IDN(rows [, cols])",
        .description = "Generates an identity matrix of specified dimensions with ones on diagonal and zeros elsewhere.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "MAT_INV", .category = "Matrix Operations", .syntax = "MAT_INV(matrix)",
        .description = "Computes the multiplicative inverse of square matrix using Gaussian elimination.",
        .error_summary = "Error 5: Illegal Function Call (singular matrix)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "MAT_RND", .category = "Matrix Operations", .syntax = "MAT_RND(rows, cols)",
        .description = "Generates a matrix of specified dimensions filled with pseudorandom numbers between 0 and 1.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "MAT_TRN", .category = "Matrix Operations", .syntax = "MAT_TRN(matrix)",
        .description = "Computes the matrix transpose by swapping rows and columns.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/module/mathext.c"
    },
    {
        .name = "GUID$", .category = "System & OS", .syntax = "GUID$",
        .description = "Generates a canonical RFC 4122 formatted Version 4 UUID string (e.g. 'c4a760a8-dbcf-5254-a0d9-6a4474dbb14e').",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "GUID", .category = "System & OS", .syntax = "GUID",
        .description = "Generates a raw 16-byte binary UUID / GUID buffer representation.",
        .error_summary = "Error 14: Out of String Space",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/eval.c"
    },
    {
        .name = "MEMMAP", .category = "Hardware & Memory", .syntax = "MEMMAP(region_idx%)",
        .description = "Queries memory map descriptors for the specified physical or virtual memory region index.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/memory/alloc.c"
    },

    // -----------------------------------------------------------------------
    // Built-in Statements
    // -----------------------------------------------------------------------
    {
        .name = "ASSERT", .category = "Program Control & Debugging", .syntax = "ASSERT condition [, message$]",
        .description = "Evaluates boolean condition; raises Error 255 (Assertion Failure) and halts if condition evaluates to false.",
        .error_summary = "Error 255: Assertion Failure",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/ast/eval_ast.c"
    },
    {
        .name = "BREAK", .category = "Control Flow", .syntax = "BREAK",
        .description = "Immediately exits the innermost active loop (FOR, WHILE, DO), or invokes the interactive debugger breakpoint.",
        .error_summary = "Error 5: Illegal Function Call (outside loop)",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/ast/eval_ast.c"
    },
    {
        .name = "DEF_FN", .category = "Function Definitions", .syntax = "DEF FNname[(args)] = expr",
        .description = "Defines a single-line user function formula adhering to classical Dartmouth / GW-BASIC DEF FN conventions.",
        .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Dartmouth BASIC, GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/ast/eval_ast.c"
    },
    {
        .name = "IF_POSTFIX", .category = "Control Flow", .syntax = "statement IF condition [ELSE statement]",
        .description = "Postfix conditional modifier executing the preceding statement only when condition is true.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Perl, Ruby, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/eval/ast/eval_ast.c"
    },
    {
        .name = "ON_ERROR_GOTO", .category = "Event Trapping", .syntax = "ON ERROR GOTO {line_number | label | 0}",
        .description = "Enables runtime error trapping and specifies handler location, or disables trapping when target is 0.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/events/events.c"
    },
    {
        .name = "ON_GOSUB", .category = "Control Flow", .syntax = "ON expr GOSUB line1 [, line2, ...]",
        .description = "Branches to one of several subroutine target lines based on the 1-based integer evaluation of expr.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/exec/exec_dispatch.c"
    },
    {
        .name = "ON_GOTO", .category = "Control Flow", .syntax = "ON expr GOTO line1 [, line2, ...]",
        .description = "Branches to one of several jump target lines based on the 1-based integer evaluation of expr.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/exec/exec_dispatch.c"
    },
    {
        .name = "ON_KEY", .category = "Event Trapping", .syntax = "ON KEY(n) GOSUB line | KEY(n) {ON|OFF|STOP}",
        .description = "Configures event trapping for specified function key or user-defined key combination.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/events/events.c"
    },
    {
        .name = "ON_PLAY", .category = "Event Trapping", .syntax = "ON PLAY(n) GOSUB line | PLAY {ON|OFF|STOP}",
        .description = "Configures continuous background sound queue threshold trapping for music playback.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/events/events.c"
    },
    {
        .name = "ON_TIMER", .category = "Event Trapping", .syntax = "ON TIMER(seconds) GOSUB line | TIMER {ON|OFF|STOP}",
        .description = "Configures periodic interval timer trapping to trigger subroutine callbacks at specified duration.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/events/events.c"
    },
    {
        .name = "STATIC", .category = "Variables & Memory", .syntax = "STATIC var1 [AS type] [, var2 ...]",
        .description = "Declares procedure-local variables whose storage and values persist across successive invocations.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "QBASIC, VB for DOS, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/variable/var.c"
    },
    {
        .name = "VAR", .category = "Variables & Memory", .syntax = "VAR [DIM] varname [AS type] [= expr]",
        .description = "Declares a block-scoped typed variable with optional immediate initialization expression.",
        .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/variable/var.c"
    },
    {
        .name = "VARS", .category = "Program Management & Introspection", .syntax = "VARS [prefix$]",
        .description = "Lists all currently allocated variables, their scopes, data types, and values to the console.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/variable/var.c"
    },
    {
        .name = "BORDER", .category = "Graphics & Display", .syntax = "BORDER color_index",
        .description = "Sets the color of the overscan screen border area surrounding active display text/graphics.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Commodore BASIC, Amstrad CPC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "BRIGHT", .category = "Graphics & Display", .syntax = "BRIGHT state%",
        .description = "Controls ZX Spectrum text foreground and background highlight attribute (0=normal, 1=bright).",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Sinclair ZX Spectrum, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "DISPLAY", .category = "Console I/O", .syntax = "DISPLAY [exprlist]",
        .description = "Outputs formatted expression list to console screen buffer adhering to ECMA-116 standard.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "ECMA-116, Super BASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/statements/io/display.c"
    },
    {
        .name = "DRAWTO", .category = "Graphics & Display", .syntax = "DRAWTO x, y [, color]",
        .description = "Draws a straight line from current graphics cursor position to destination point (x, y).",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "FLASH", .category = "Graphics & Display", .syntax = "FLASH state%",
        .description = "Controls blinking text attribute on vintage video displays (0=off, 1=on).",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Sinclair ZX Spectrum, Apple II, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "GRAPHICS", .category = "Graphics & Display", .syntax = "GRAPHICS mode_index",
        .description = "Initializes retro or Atari 8-bit graphics display mode with specified resolution and colors.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Atari BASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "INK", .category = "Graphics & Display", .syntax = "INK color_index",
        .description = "Sets the active foreground drawing and printing color index.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Sinclair ZX Spectrum, Amstrad CPC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "INVERSE", .category = "Graphics & Display", .syntax = "INVERSE [state%]",
        .description = "Enables or disables inverse video text mode swapping foreground and background colors.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Apple II Applesoft, Sinclair ZX Spectrum, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "LOCATE", .category = "Console I/O", .syntax = "LOCATE [row] [, [col] [, [cursor] [, [start] [, stop]]]]",
        .description = "Positions the console text cursor at row and column (1-indexed), and configures cursor visibility.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx_tui.c"
    },
    {
        .name = "OVER", .category = "Graphics & Display", .syntax = "OVER state%",
        .description = "Controls overstrike transparent printing mode (0=overwrite, 1=XOR/overstrike).",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Sinclair ZX Spectrum, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "PAPER", .category = "Graphics & Display", .syntax = "PAPER color_index",
        .description = "Sets the active background fill and character background color index.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Sinclair ZX Spectrum, Amstrad CPC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "PCOPY", .category = "Graphics & Display", .syntax = "PCOPY source_page, dest_page",
        .description = "Copies an entire video display buffer page from source_page to dest_page.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "QuickBASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "PMAP", .category = "Graphics & Display", .syntax = "PMAP(coord, map_type)",
        .description = "Maps coordinate between physical window viewport space and mathematical world coordinates.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "QuickBASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "POINTER", .category = "Hardware & Systems", .syntax = "POINTER ptr_name AS type",
        .description = "Declares a typed memory pointer variable for systems and bare-metal programming.",
        .error_summary = "Error 2: Syntax Error",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_UNSAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Systems Standard", .since_version = "6.5.0",
        .source_file = "engine/src/variable/var.c"
    },
    {
        .name = "POKEB", .category = "Hardware & Memory", .syntax = "POKEB address, byte_val",
        .description = "Writes an unsigned 8-bit byte directly to physical or virtual memory address.",
        .error_summary = "Error 5: Illegal Function Call, Error 70: Permission Denied",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_UNSAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/memory/alloc.c"
    },
    {
        .name = "ALLOC", .category = "Hardware & Memory", .syntax = "ALLOC(size_bytes) | ALLOC var, size",
        .description = "Allocates a contiguous raw byte buffer from the system memory arena and returns handle/pointer.",
        .error_summary = "Error 7: Out of Memory",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Systems Standard", .since_version = "6.5.0",
        .source_file = "engine/src/runtime/memory/alloc.c"
    },
    {
        .name = "BANK", .category = "Hardware & Memory", .syntax = "BANK bank_number%",
        .description = "Selects active expanded memory bank (EMS/XMS or retro banked RAM) for segmented addressing.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Commodore 128 BASIC 7.0, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/vdev.c"
    },
    {
        .name = "RAMBANKS", .category = "Hardware & Memory", .syntax = "RAMBANKS count%",
        .description = "Configures or reserves the total number of banked memory pages available to the runtime.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/vdev.c"
    },
    {
        .name = "SEG", .category = "Hardware & Memory", .syntax = "SEG = segment_address% | DEF SEG = segment_address%",
        .description = "Sets the base segment address for subsequent PEEK, POKE, BLOAD, and BSAVE operations.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/vdev.c"
    },
    {
        .name = "ACCESS", .category = "Filesystem I/O", .syntax = "ACCESS filepath$ [, mode%]",
        .description = "Checks file access permissions or configures file stream access mode (read, write, lock).",
        .error_summary = "Error 53: File Not Found, Error 70: Permission Denied",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "DIR", .category = "Filesystem I/O", .syntax = "DIR [filespec$] [, attributes%]",
        .description = "Displays directory entries matching filespec$ to console, or iterates folder contents.",
        .error_summary = "Error 76: Path Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "DUMP", .category = "Debugging & Memory", .syntax = "DUMP [var_or_address [, length%]]",
        .description = "Outputs a formatted hexadecimal and ASCII memory dump of variable contents or address range.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/debug/logger.c"
    },
    {
        .name = "EDIT", .category = "Program Management & Editing", .syntax = "EDIT [filename$ | line_number]",
        .description = "Launches the built-in full-screen TUI text editor or positions cursor at specified line.",
        .error_summary = "Error 53: File Not Found",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/editor/editor_manager.c"
    },
    {
        .name = "EXISTS", .category = "Filesystem I/O", .syntax = "EXISTS(path$) | EXISTS path$",
        .description = "Tests whether the specified file or directory path exists on the mounted filesystem.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "FILEMOD$", .category = "Filesystem I/O", .syntax = "FILEMOD$(path$)",
        .description = "Returns the file last-modified timestamp as an ISO-8601 formatted date/time string.",
        .error_summary = "Error 53: File Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "FILEMOD", .category = "Filesystem I/O", .syntax = "FILEMOD(path$)",
        .description = "Returns the file last-modified timestamp as a 64-bit Unix epoch integer timestamp.",
        .error_summary = "Error 53: File Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "FILESIZE", .category = "Filesystem I/O", .syntax = "FILESIZE(path$)",
        .description = "Returns the total size of the specified file in bytes as a 64-bit integer.",
        .error_summary = "Error 53: File Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "MOUNT", .category = "Virtual Filesystems", .syntax = "MOUNT drive_letter$, device_uri$",
        .description = "Mounts a local directory, virtual ramdisk, archive, or network URI to a virtual drive letter.",
        .error_summary = "Error 76: Path Not Found, Error 70: Permission Denied",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "MOUNTS", .category = "Virtual Filesystems", .syntax = "MOUNTS",
        .description = "Lists all currently active mounted virtual filesystem mount points and device types.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "PWD", .category = "Filesystem I/O", .syntax = "PWD",
        .description = "Prints the current working directory path of the active process to the console.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "RESET", .category = "Filesystem I/O", .syntax = "RESET",
        .description = "Flushes and closes all open file streams, returning file handles to the operating system pool.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "REVERSE", .category = "Graphics & Display", .syntax = "REVERSE [state%]",
        .description = "Sets or inverts character drawing and printing direction.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ADVANCED, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx.c"
    },
    {
        .name = "SCRATCH", .category = "Program Management", .syntax = "SCRATCH",
        .description = "Erases all program lines, resets symbol tables, and frees allocated variable heap memory.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Dartmouth BASIC, HP BASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/control/vm_control.c"
    },
    {
        .name = "SET", .category = "System & Environment", .syntax = "SET option$ = value",
        .description = "Configures engine tunables, dialect switches, or environment configuration variables.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/keyword_props.c"
    },
    {
        .name = "TRACE", .category = "Debugging", .syntax = "TRACE {ON | OFF | line_num}",
        .description = "Configures execution line tracing, printing line numbers to console as they execute.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "Vintage BASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/debug/logger.c"
    },
    {
        .name = "TROFF", .category = "Debugging", .syntax = "TROFF",
        .description = "Turns trace mode off, stopping execution line number printing to console.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/debug/logger.c"
    },
    {
        .name = "TRON", .category = "Debugging", .syntax = "TRON",
        .description = "Turns trace mode on, printing line numbers in brackets [n] to console as each statement executes.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/debug/logger.c"
    },
    {
        .name = "UMOUNT", .category = "Virtual Filesystems", .syntax = "UMOUNT drive_letter$",
        .description = "Unmounts the specified virtual drive letter and releases associated filesystem resources.",
        .error_summary = "Error 76: Path Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "UNLOAD", .category = "Dynamic Loading", .syntax = "UNLOAD module_name$",
        .description = "Unloads a dynamically linked library or loaded module extension from active memory.",
        .error_summary = "Error 53: File Not Found",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "VB for DOS, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/dl/plat_dl.c"
    },
    {
        .name = "UNLOCK", .category = "File & Resource Locking", .syntax = "UNLOCK [#]file_num [, [record_start] [TO record_end]]",
        .description = "Releases active file or record lock previously acquired with LOCK.",
        .error_summary = "Error 52: Bad File Number",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "VPATH", .category = "Virtual Filesystems", .syntax = "VPATH [search_path$]",
        .description = "Configures or displays the active search path list used for file resolution and includes.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/runtime/vfs.c"
    },
    {
        .name = "WIDTH", .category = "Console & Printer I/O", .syntax = "WIDTH [columns] [, rows] | WIDTH [#file_num,] columns",
        .description = "Sets the text output column width for the console screen, active window, or printer channel.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/device/gfx_tui.c"
    },
    {
        .name = "WRITE", .category = "File & Console I/O", .syntax = "WRITE [#file_num,] [exprlist]",
        .description = "Outputs comma-delimited, quote-encapsulated data values suitable for subsequent INPUT reading.",
        .error_summary = "Error 52: Bad File Number",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_IO, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/statements/io/file_ops/write.c"
    },
    {
        .name = "CONSOLE", .category = "Console I/O", .syntax = "CONSOLE [device$ | mode%]",
        .description = "Redirects standard console terminal input and output to an alternate virtual device or port.",
        .error_summary = "Error 52: Bad File Number",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/console/plat_console.c"
    },
    {
        .name = "ERR$", .category = "Error Handling", .syntax = "ERR$ [(error_code%)]",
        .description = "Returns the authoritative textual error description string for the last error or specified code.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_PURE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/error/error.c"
    },
    {
        .name = "ERROR", .category = "Error Handling", .syntax = "ERROR error_code%",
        .description = "Simulates a runtime error with the specified numeric code, triggering active ON ERROR traps.",
        .error_summary = "Simulates runtime error code",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "GW-BASIC, QBASIC, BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/error/error.c"
    },
    {
        .name = "INKEY", .category = "Console Input", .syntax = "INKEY [(timeout_ms)]",
        .description = "Reads a single character from the console keyboard buffer without waiting or with optional timeout.",
        .error_summary = "None",
        .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/platform/console/plat_console.c"
    },
    {
        .name = "ONKEY$", .category = "Event Trapping", .syntax = "ONKEY$(key_code$) GOSUB line",
        .description = "Configures event trapping for specific character keypress sequences.",
        .error_summary = "Error 5: Illegal Function Call",
        .subsystem = SUBSYSTEM_ENGINE, .safety = SAFETY_SAFE, .type = FEATURE_STATEMENT,
        .compat = "BASIC++ Standard", .since_version = "6.0.0",
        .source_file = "engine/src/vm/events/events.c"
    }
};

void builtin_descriptors_register(void) {
    size_t count = sizeof(g_builtin_descriptors) / sizeof(g_builtin_descriptors[0]);
    for (size_t i = 0; i < count; i++) {
        lang_desc_register(&g_builtin_descriptors[i]);
    }
}

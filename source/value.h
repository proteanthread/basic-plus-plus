/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: value.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - value.h
 // ---
 //
 // Tagged value type for the BASIC++ interpreter.
 //
 // PURPOSE:
 // Provides a unified value type (BValue) that can hold integers,
 // floating-point numbers, or strings. This replaces the raw 'long'
 // values used in Phases 1-3 and enables mixed-type expressions,
 // type coercion, and string operations.
 //
 // DESIGN RATIONALE:
 // A tagged union is the classic approach for dynamically-typed
 // languages. Each BValue carries a ValueType tag that indicates
 // which union member is active. The tag is checked at runtime
 // for type-safe operations and automatic coercion.
 //
 // In integer-only mode (PATB dialect), all values are VAL_INTEGER
 // and the tag checks compile to trivial comparisons that the
 // optimizer can often eliminate.
 //
 // STRING OWNERSHIP:
 // String values point into a string pool (see stringpool.h).
 // BValue does NOT own the string data - it holds a borrowed
 // pointer. The string pool manages lifetime. This avoids
 // per-value malloc/free and keeps the memory model simple.
 //
 // ---

#ifndef BASICPP_VALUE_H
#define BASICPP_VALUE_H

#include "config.h"

// --- Value Type Tag ---
typedef enum ValueType {
 VAL_INTEGER = 0, // long integer (always available)
 VAL_FLOAT, // double precision floating point
 VAL_STRING, // string (pointer into string pool)
 VAL_COMPLEX // complex number (real + imaginary)
} ValueType;

// --- BValue - Tagged Union Value ---
 // The fundamental value type for all expressions, variables,
 // and DATA items in the interpreter.
 //
 // Fields:
 // type - discriminator tag
 // v - union holding the actual value
 // .ival - integer value (for VAL_INTEGER)
 // .fval - floating-point value (for VAL_FLOAT)
 // .sval - string value (for VAL_STRING)
 // .cval - complex value (for VAL_COMPLEX)
 // .data - pointer to string data (NOT null-terminated in pool)
 // .length - length of the string in bytes
typedef struct BValue {
 ValueType type;
 union {
 long ival; // VAL_INTEGER
 double fval; // VAL_FLOAT
 struct {
 char *data; // pointer into string pool (borrowed)
 int length; // string length
 } sval; // VAL_STRING
 struct {
 double real; // real part
 double imag; // imaginary part
 } cval; // VAL_COMPLEX
 } v;
} BValue;

// --- Constructors - Create BValue from native types ---

// Create an integer value
BValue bval_int(long val);

// Create a floating-point value
BValue bval_float(double val);

// Create a string value (borrows pointer - does NOT copy)
BValue bval_string(char *data, int length);

// Create a complex value
BValue bval_complex(double real, double imag);

// --- Type Queries ---
int bval_is_int(const BValue *v);
int bval_is_float(const BValue *v);
int bval_is_string(const BValue *v);
int bval_is_complex(const BValue *v);
int bval_is_numeric(const BValue *v);

// --- Type Coercion ---
 // These convert a BValue to the requested type. Numeric types
 // promote freely (int -> float). String -> number uses strtol/strtod.
 // Number -> string is done via bval_to_string_buf (formats into buffer).

// Convert to integer (truncates floats, parses strings)
long bval_to_int(const BValue *v);
long bval_to_subscript(const BValue *v); // round for array subscripts

// Convert to float (promotes ints, parses strings)
double bval_to_float(const BValue *v);

 // Format value as string into a caller-provided buffer.
 // Returns the number of characters written.
 // Buffer must be at least 32 bytes for numeric values.
int bval_to_string_buf(const BValue *v, char *buf, int bufsize);

// --- Arithmetic Operations ---
 // All arithmetic auto-promotes: if either operand is float,
 // the result is float. String operands in arithmetic raise errors.
 //
 // Returns a new BValue with the result.
 // Sets error flag (via error_raise) on type mismatch or div-by-zero.
BValue bval_add(const BValue *a, const BValue *b, int line_num);
BValue bval_sub(const BValue *a, const BValue *b, int line_num);
BValue bval_mul(const BValue *a, const BValue *b, int line_num);
BValue bval_div(const BValue *a, const BValue *b, int line_num);
BValue bval_neg(const BValue *v, int line_num);
BValue bval_mod(const BValue *a, const BValue *b, int line_num);

// --- Comparison ---
 // Compares two BValues. Returns:
 // -1 if a < b
 // 0 if a == b
 // +1 if a > b
 //
 // Numeric comparisons auto-promote to float if mixed.
 // String comparisons are lexicographic (case-sensitive).
 // Mixed string/numeric raises ERR_WHAT.
int bval_compare(const BValue *a, const BValue *b, int line_num);

// --- Float Math Functions ---
 // All take a numeric BValue, return VAL_FLOAT (or VAL_INTEGER for INT).
 // String arguments raise ERR_WHAT.
BValue bval_abs(const BValue *v, int line_num);
BValue bval_sgn(const BValue *v, int line_num);
BValue bval_int_func(const BValue *v, int line_num); // INT()
BValue bval_sqr(const BValue *v, int line_num); // SQR = sqrt
BValue bval_sin(const BValue *v, int line_num);
BValue bval_cos(const BValue *v, int line_num);
BValue bval_tan(const BValue *v, int line_num);
BValue bval_atn(const BValue *v, int line_num); // ATN = atan
BValue bval_log(const BValue *v, int line_num); // natural log
BValue bval_exp(const BValue *v, int line_num); // e^x

// --- String Functions ---
 // These require a string pool pointer for allocating result strings.
 // The pool parameter is passed as void* to avoid circular includes.

// LEN(s$) - returns integer length of string
BValue bval_len(const BValue *v, int line_num);

// ASC(s$) - returns ASCII code of first character
BValue bval_asc(const BValue *v, int line_num);

// VAL(s$) - parse string as number
BValue bval_val(const BValue *v, int line_num);

// CHR$(n) - character from ASCII code (needs string pool)
BValue bval_chr(const BValue *v, int line_num, void *pool);

// STR$(n) - number to string (needs string pool)
BValue bval_str(const BValue *v, int line_num, void *pool);

// LEFT$(s$, n) - left substring (needs string pool)
BValue bval_left(const BValue *s, const BValue *n, int line_num,
 void *pool);

// RIGHT$(s$, n) - right substring (needs string pool)
BValue bval_right(const BValue *s, const BValue *n, int line_num,
 void *pool);

// MID$(s$, start, len) - middle substring (needs string pool)
BValue bval_mid(const BValue *s, const BValue *start,
 const BValue *len, int line_num, void *pool);

// String concatenation: s1$ + s2$ (needs string pool)
BValue bval_concat(const BValue *a, const BValue *b, int line_num,
 void *pool);

#endif // BASICPP_VALUE_H

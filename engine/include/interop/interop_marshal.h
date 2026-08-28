// FILENAME: interop_marshal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h, interop_marshal.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_marshal within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_MARSHAL_H
#define INTEROP_MARSHAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum InteropValueType {
    INTEROP_TYPE_NULL = 0,
    INTEROP_TYPE_NUMBER,
    INTEROP_TYPE_STRING,
    INTEROP_TYPE_INTEGER,
    INTEROP_TYPE_BOOLEAN,
    INTEROP_TYPE_ARRAY,
    INTEROP_TYPE_ERROR
} InteropValueType;

typedef struct InteropValue {
    InteropValueType type;
    union {
        double number_val;
        char* string_val;
        int64_t integer_val;
        bool boolean_val;
        void* array_val;
        void* error_val;
    } as;
} InteropValue;

// @brief Create an InteropValue from a number (double).
// @param val The double value.
// @return The created InteropValue.
InteropValue interop_value_from_number(double val);

// @brief Create an InteropValue from a string.
// @param val The string value (will be copied).
// @return The created InteropValue.
InteropValue interop_value_from_string(const char* val);

// @brief Create an InteropValue from an integer.
// @param val The integer value.
// @return The created InteropValue.
InteropValue interop_value_from_integer(int64_t val);

// @brief Create an InteropValue from a boolean.
// @param val The boolean value.
// @return The created InteropValue.
InteropValue interop_value_from_boolean(bool val);

// @brief Coerce an InteropValue to a number.
// @param val Pointer to the InteropValue.
// @param out_val Pointer to the output double.
// @return 0 on success, non-zero on failure.
int interop_value_to_number(const InteropValue* val, double* out_val);

// @brief Coerce an InteropValue to a string.
// @param val Pointer to the InteropValue.
// @param out_val Pointer to the output string (must be freed by caller).
// @return 0 on success, non-zero on failure.
int interop_value_to_string(const InteropValue* val, char** out_val);

// @brief Coerce an InteropValue to an integer.
// @param val Pointer to the InteropValue.
// @param out_val Pointer to the output integer.
// @return 0 on success, non-zero on failure.
int interop_value_to_integer(const InteropValue* val, int64_t* out_val);

// @brief Coerce an InteropValue to a boolean.
// @param val Pointer to the InteropValue.
// @param out_val Pointer to the output boolean.
// @return 0 on success, non-zero on failure.
int interop_value_to_boolean(const InteropValue* val, bool* out_val);

// @brief Release resources held by an InteropValue.
// @param val Pointer to the InteropValue.
void interop_value_release(InteropValue* val);

// @brief Deep copy an InteropValue.
// @param src Pointer to the source InteropValue.
// @param dst Pointer to the destination InteropValue.
// @return 0 on success, non-zero on failure.
int interop_value_clone(const InteropValue* src, InteropValue* dst);

#endif // INTEROP_MARSHAL_H

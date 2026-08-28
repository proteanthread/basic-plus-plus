// FILENAME: bpp_api.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.c)
// NEEDS: libcore (interop_error.h, interop_error.c)
// NEEDS: libcore (interop_handle.h, interop_handle.c)
// NEEDS: libcore (interop_marshal.h, interop_marshal.c)
// Provides core logic and interface definitions for bpp_api within BASIC++.
//
// ---- Includes ----

#ifndef BPP_API_H
#define BPP_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(BASICPP_STATIC)
    #define BPP_API
  #else
    #define BPP_API __declspec(dllexport)
  #endif
#else
  #define BPP_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Opaque Handle for BASIC++ Engine Context
typedef struct BppEngineContext BppEngineContext;

// Value Types returned by bpp_eval_expr
typedef enum {
    BPP_VAL_NULL = 0,
    BPP_VAL_NUMBER,
    BPP_VAL_STRING,
    BPP_VAL_ERROR
} BppValueType;

typedef struct {
    BppValueType type;
    union {
        double number;
        char *string; // Reference-counted or heap string
        int error_code;
    } as;
} BppValue;

// Host Function Pointer Signature for C17 / Python 3 callbacks
typedef BppValue (*BppHostFn)(BppEngineContext *ctx, const char *name, int argc, const BppValue *args, void *userdata);

// Console Output Callback Pointer Signature
typedef void (*BppConsoleOutputCb)(const char *text, size_t length, void *userdata);

// @brief Initialize a BASIC++ engine context with specified RAM pool size.
// @param ram_bytes Memory allocation in bytes (e.g. 671088640L for 640MB, 0 for default).
// @return Opaque pointer to initialized BppEngineContext, or NULL on allocation failure.
BPP_API BppEngineContext* bpp_init(size_t ram_bytes);

// @brief Shuts down the BASIC++ engine context and frees all associated heaps and devices.
// @param ctx Valid engine context handle.
BPP_API void bpp_shutdown(BppEngineContext *ctx);

// @brief Executes a single line or statement of BASIC++ code.
// @param ctx Engine context handle.
// @param code Null-terminated BASIC++ code line string.
// @return 0 on success, or BASIC++ error code (e.g. 2 for Syntax Error, 13 for Type Mismatch).
BPP_API int bpp_exec_string(BppEngineContext *ctx, const char *code);

// @brief Loads a BASIC++ source file and executes it.
// @param ctx Engine context handle.
// @param filepath Path to .bas or .bpp file.
// @return 0 on success, or non-zero error code.
BPP_API int bpp_load_and_run(BppEngineContext *ctx, const char *filepath);

// @brief Evaluates an expression and returns its typed BppValue result.
// @param ctx Engine context handle.
// @param expression Expression string to evaluate (e.g., "SQR(16) + 5").
// @return BppValue structure containing the result type and value.
BPP_API BppValue bpp_eval_expr(BppEngineContext *ctx, const char *expression);

// @brief Frees resources held by a returned BppValue (e.g. string values).
// @param val Pointer to BppValue to release.
BPP_API void bpp_value_release(BppValue *val);

// @brief Register a custom host C/Python callback function into the engine.
// @param ctx Engine context handle.
// @param name Function name as callable in BASIC++ (e.g. "MYFUNC").
// @param fn Pointer to BppHostFn callback function.
// @param userdata Optional user context pointer passed to callback.
// @return 0 on success, non-zero error code on failure.
BPP_API int bpp_register_func(BppEngineContext *ctx, const char *name, BppHostFn fn, void *userdata);

// @brief Get double numeric value of a BASIC++ variable.
// @param ctx Engine context handle.
// @param var_name Variable name (e.g. "X", "TOTAL%").
// @return Double value of variable, or 0.0 if not found.
BPP_API double bpp_get_var_num(BppEngineContext *ctx, const char *var_name);

// @brief Set double numeric value of a BASIC++ variable.
// @param ctx Engine context handle.
// @param var_name Variable name (e.g. "X", "TOTAL%").
// @param value Double value to assign.
// @return 0 on success, non-zero on failure.
BPP_API int bpp_set_var_num(BppEngineContext *ctx, const char *var_name, double value);

// @brief Get string value of a BASIC++ variable.
// @param ctx Engine context handle.
// @param var_name Variable name (e.g. "NAME$").
// @param out_buf Output buffer to receive string value.
// @param buf_size Maximum capacity of out_buf.
// @return True on success, false on failure or missing variable.
BPP_API bool bpp_get_var_str(BppEngineContext *ctx, const char *var_name, char *out_buf, size_t buf_size);

// @brief Set string value of a BASIC++ variable.
// @param ctx Engine context handle.
// @param var_name Variable name (e.g. "NAME$").
// @param value Null-terminated string value to assign.
// @return 0 on success, non-zero on failure.
BPP_API int bpp_set_var_str(BppEngineContext *ctx, const char *var_name, const char *value);

// @brief Intercept virtual console text output with a custom host callback.
// @param ctx Engine context handle.
// @param cb Host callback function.
// @param userdata Optional context pointer passed to callback.
BPP_API void bpp_set_console_output_cb(BppEngineContext *ctx, BppConsoleOutputCb cb, void *userdata);

// @brief Queries the engine version string.
// @return Version string (e.g., "6.5.2").
BPP_API const char* bpp_version_string(void);

// ============================================
// Cross-Language Interop API Extensions v1.0
// ============================================

#include "interop/interop_handle.h"
#include "interop/interop_error.h"
#include "interop/interop_marshal.h"

// --- Version Query ---
BPP_API const char *basicpp_version_string(void);
BPP_API int basicpp_version_major(void);
BPP_API int basicpp_version_minor(void);
BPP_API int basicpp_version_patch(void);

// --- Error Query ---
BPP_API const InteropError *basicpp_get_last_error(void);
BPP_API void basicpp_clear_error(void);


#ifdef __cplusplus
}
#endif

#endif // BPP_API_H

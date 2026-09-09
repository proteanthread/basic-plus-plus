// FILENAME: basicpp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: C/C++ Host Applications, External Embedding Clients, Bindings
// High-level single-header C17 embedding API and runtime interface for BASIC++.

#ifndef BASICPP_H
#define BASICPP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// ---- Dynamic Export / Import Declarations ----
//

#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(BASICPP_EXPORTS)
        #define BPP_API __declspec(dllexport)
    #elif defined(BASICPP_SHARED)
        #define BPP_API __declspec(dllimport)
    #else
        #define BPP_API
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define BPP_API __attribute__((visibility("default")))
    #else
        #define BPP_API
    #endif
#endif

//
// ---- Version & Constant Declarations ----
//

#define BPP_VERSION_MAJOR 6
#define BPP_VERSION_MINOR 5
#define BPP_VERSION_PATCH 2
#define BPP_VERSION_STRING "6.5.2"

//
// ---- Value Types ----
//

#ifndef BPP_VALUE_DEFINED
#define BPP_VALUE_DEFINED
typedef enum {
    BPP_VAL_NULL = 0,
    BPP_VAL_NUMBER,
    BPP_VAL_INTEGER,
    BPP_VAL_STRING,
    BPP_VAL_ERROR
} BppValueType;

typedef struct {
    BppValueType type;
    union {
        double number;
        char *string;
        int error_code;
    } as;
} BppValue;
#endif

#ifndef BPP_LOG_LEVEL_DEFINED
#define BPP_LOG_LEVEL_DEFINED
typedef enum {
    BPP_LOG_TRACE = 0,
    BPP_LOG_DEBUG = 1,
    BPP_LOG_INFO  = 2,
    BPP_LOG_WARN  = 3,
    BPP_LOG_ERROR = 4,
    BPP_LOG_FATAL = 5
} BppLogLevel;

typedef void (*BppLogSinkFn)(BppLogLevel level, const char *tag, const char *message, const char *timestamp, void *userdata);
#endif

typedef void (*BppBusCallbackFn)(const char *topic, const char *payload, size_t len, void *userdata);

//
// ---- Opaque Engine Context Handle ----
//

typedef struct BppEngineContext BppEngineContext;

//
// ---- Core Engine Lifecycle ----
//

BPP_API BppEngineContext *bpp_init(size_t ram_bytes);
BPP_API void              bpp_shutdown(BppEngineContext *ctx);
BPP_API void              bpp_reset(BppEngineContext *ctx);
BPP_API const char       *bpp_version(void);

//
// ---- Program Execution & Evaluation ----
//

BPP_API int      bpp_exec_string(BppEngineContext *ctx, const char *source);
BPP_API int      bpp_exec_file(BppEngineContext *ctx, const char *filepath);
BPP_API BppValue bpp_eval_expr(BppEngineContext *ctx, const char *expr);
BPP_API void     bpp_value_free(BppEngineContext *ctx, BppValue *val);

//
// ---- Variable Access ----
//

BPP_API int         bpp_set_number(BppEngineContext *ctx, const char *name, double val);
BPP_API int         bpp_set_string(BppEngineContext *ctx, const char *name, const char *val);
BPP_API double      bpp_get_number(BppEngineContext *ctx, const char *name);
BPP_API const char *bpp_get_string(BppEngineContext *ctx, const char *name);

//
// ---- Zero-Copy Buffer & Array Views ----
//

BPP_API int bpp_array_bind_view(BppEngineContext *ctx, const char *arr_name, int elem_type, void *data_ptr, size_t count);

//
// ---- Message Bus & IPC Bridge ----
//

BPP_API void bpp_bus_publish(BppEngineContext *ctx, const char *topic, const char *payload, size_t len);
BPP_API void bpp_bus_subscribe(BppEngineContext *ctx, const char *topic, BppBusCallbackFn callback_fn, void *userdata);
BPP_API void bpp_bus_unsubscribe(BppEngineContext *ctx, const char *topic, BppBusCallbackFn callback_fn);
BPP_API int  bpp_ipc_send(BppEngineContext *ctx, const char *endpoint, const char *payload, size_t len);
BPP_API int  bpp_ipc_recv(BppEngineContext *ctx, const char *endpoint, char *out_buf, size_t max_len);

//
// ---- Logging & Diagnostics Bridge ----
//

BPP_API void        bpp_log_add_sink(BppEngineContext *ctx, BppLogSinkFn sink_fn, int min_level, void *userdata);
BPP_API void        bpp_log_remove_sink(BppEngineContext *ctx, BppLogSinkFn sink_fn);
BPP_API void        bpp_log_msg(BppEngineContext *ctx, int level, const char *tag, const char *msg);
BPP_API const char *bpp_log_level_name(int level);

#ifdef __cplusplus
}
#endif

#endif // BASICPP_H

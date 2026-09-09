// FILENAME: bpp_api.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime, basicpp.dll, libbasicpp, C/C++ Host Applications
// NEEDS: libboot, libcore, libengine, libkernel, libplatform
// Provides complete single-header C17 embedding API implementation for BASIC++.
//
// ---- Includes ----

#define BASICPP_EXPORTS
#include "bpp_api.h"
#include "basicpp.h"

#include "core/boot.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "runtime/funcreg.h"
#include "runtime/arrays.h"
#include "debug/logger.h"
#include "device/msg_broker.h"
#include "device/vdev.h"
#include "platform/platform.h"
#include "types/version.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

#include "runtime/format/snprintf.h"

// Internal Host Registry Callback Entry
typedef struct HostFuncWrapper {
    BppHostFn fn;
    void *userdata;
    BppEngineContext *ctx;
} HostFuncWrapper;

static HostFuncWrapper g_host_funcs[64];
static int g_host_func_count = 0;

static char *api_strdup(const char *s) {
    if (!s) return NULL;
    HalContext *hal = hal_get();
    size_t len = runtime_strlen(s);
    char *copy = NULL;
    if (hal && hal->mem.alloc) {
        copy = (char *)hal->mem.alloc(len + 1);
    }
    if (copy) {
        runtime_memcpy(copy, s, len + 1);
    }
    return copy;
}

static void api_strfree(char *s) {
    if (!s) return;
    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(s);
    }
}

static BValue host_func_bridge(BValue *args, int argc, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NONE;

    if (g_host_func_count > 0 && g_host_funcs[0].fn) {
        BppValue bargs[16];
        int count = (argc > 16) ? 16 : argc;
        for (int i = 0; i < count; i++) {
            runtime_memset(&bargs[i], 0, sizeof(bargs[i]));
            if (args[i].type == VAL_NUMBER || args[i].type == VAL_INTEGER) {
                bargs[i].type = BPP_VAL_NUMBER;
                bargs[i].as.number = args[i].as.number;
            } else if (args[i].type == VAL_STRING && args[i].as.string) {
                bargs[i].type = BPP_VAL_STRING;
                const char *s = str_data(args[i].as.string);
                bargs[i].as.string = s ? api_strdup(s) : NULL;
            }
        }

        BppValue bres = g_host_funcs[0].fn(g_host_funcs[0].ctx, "HOST", count, bargs, g_host_funcs[0].userdata);
        if (bres.type == BPP_VAL_NUMBER) {
            res.type = VAL_NUMBER;
            res.as.number = bres.as.number;
        }

        for (int i = 0; i < count; i++) {
            if (bargs[i].type == BPP_VAL_STRING && bargs[i].as.string) {
                api_strfree(bargs[i].as.string);
            }
        }
    }
    return res;
}

BPP_API BppEngineContext* bpp_init(size_t ram_bytes) {
    platform_init();
    logger_init(NULL, NULL);
    size_t alloc_size = (ram_bytes > 0) ? ram_bytes : 671088640L; // Default 640MB
    VMContext *vm = boot_system(alloc_size);
    return (BppEngineContext*)vm;
}

BPP_API void bpp_shutdown(BppEngineContext *ctx) {
    if (!ctx) return;
    VMContext *vm = (VMContext*)ctx;
    boot_shutdown_vm(vm);
    platform_shutdown();
}

BPP_API void bpp_reset(BppEngineContext *ctx) {
    if (!ctx) return;
    VMContext *vm = (VMContext*)ctx;
    vm_reset_for_run(vm);
    VariableContext *vc = vm_get_var(vm);
    if (vc) var_clear_all(vc);
}

BPP_API int bpp_exec_string(BppEngineContext *ctx, const char *code) {
    if (!ctx || !code) return -1;
    VMContext *vm = (VMContext*)ctx;
    BppError err = vm_execute_line(vm, code);
    return err.code;
}

BPP_API int bpp_load_and_run(BppEngineContext *ctx, const char *filepath) {
    if (!ctx || !filepath) return -1;
    VMContext *vm = (VMContext*)ctx;
    BppError err = vm_load_program_file(vm, filepath);
    if (err.code != 0) return err.code;
    err = vm_execute_line(vm, "RUN");
    return err.code;
}

BPP_API int bpp_exec_file(BppEngineContext *ctx, const char *filepath) {
    return bpp_load_and_run(ctx, filepath);
}

BPP_API BppValue bpp_eval_expr(BppEngineContext *ctx, const char *expression) {
    BppValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = BPP_VAL_NULL;

    if (!ctx || !expression) {
        res.type = BPP_VAL_ERROR;
        res.as.error_code = 5; // Illegal Function Call
        return res;
    }

    VMContext *vm = (VMContext*)ctx;
    MemoryContext *mem = vm_get_mem(vm);
    LexerContext *lex = lex_init(mem, expression);
    if (!lex) {
        res.type = BPP_VAL_ERROR;
        res.as.error_code = 7; // Out of Memory
        return res;
    }

    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue bval = eval_expression(vm, lex, &err);
    lex_shutdown(lex);

    if (err.code != 0) {
        res.type = BPP_VAL_ERROR;
        res.as.error_code = err.code;
    } else if (bval.type == VAL_NUMBER || bval.type == VAL_INTEGER) {
        res.type = BPP_VAL_NUMBER;
        res.as.number = bval.as.number;
    } else if (bval.type == VAL_STRING && bval.as.string) {
        res.type = BPP_VAL_STRING;
        const char *raw_str = str_data(bval.as.string);
        res.as.string = raw_str ? api_strdup(raw_str) : NULL;
    }

    return res;
}

BPP_API void bpp_value_release(BppValue *val) {
    if (!val) return;
    if (val->type == BPP_VAL_STRING && val->as.string) {
        api_strfree(val->as.string);
        val->as.string = NULL;
    }
    val->type = BPP_VAL_NULL;
}

BPP_API void bpp_value_free(BppEngineContext *ctx, BppValue *val) {
    (void)ctx;
    bpp_value_release(val);
}

BPP_API int bpp_register_func(BppEngineContext *ctx, const char *name, BppHostFn fn, void *userdata) {
    if (!ctx || !name || !fn) return -1;

    if (g_host_func_count < 64) {
        g_host_funcs[g_host_func_count].fn = fn;
        g_host_funcs[g_host_func_count].userdata = userdata;
        g_host_funcs[g_host_func_count].ctx = ctx;
        g_host_func_count++;
    }

    FunctionEntry entry;
    runtime_memset(&entry, 0, sizeof(entry));
    entry.name = name;
    entry.keyword = KW_NONE;
    entry.category = FCAT_USER;
    entry.ret_type = FRET_ANY;
    entry.min_args = 0;
    entry.max_args = 16;
    entry.safety = FSAFE_PURE;
    entry.overridable = 1;
    entry.handler = host_func_bridge;
    entry.help_text = "Registered C17/Python host function callback.";
    entry.module_name = "host_c17";

    return funcreg_register(&entry);
}

BPP_API double bpp_get_var_num(BppEngineContext *ctx, const char *var_name) {
    if (!ctx || !var_name) return 0.0;
    VMContext *vm = (VMContext*)ctx;
    VariableContext *vars = vm_get_var(vm);
    BValue *val = var_lookup(vars, var_name, false);
    if (val && (val->type == VAL_NUMBER || val->type == VAL_INTEGER)) {
        return val->as.number;
    }
    return 0.0;
}

BPP_API double bpp_get_number(BppEngineContext *ctx, const char *name) {
    return bpp_get_var_num(ctx, name);
}

BPP_API int bpp_set_var_num(BppEngineContext *ctx, const char *var_name, double value) {
    if (!ctx || !var_name) return -1;
    VMContext *vm = (VMContext*)ctx;
    VariableContext *vars = vm_get_var(vm);
    BValue val;
    runtime_memset(&val, 0, sizeof(val));
    val.type = VAL_NUMBER;
    val.as.number = value;
    return var_assign(vars, var_name, val) ? 0 : -1;
}

BPP_API int bpp_set_number(BppEngineContext *ctx, const char *name, double val) {
    return bpp_set_var_num(ctx, name, val);
}

BPP_API bool bpp_get_var_str(BppEngineContext *ctx, const char *var_name, char *out_buf, size_t buf_size) {
    if (!ctx || !var_name || !out_buf || buf_size == 0) return false;
    VMContext *vm = (VMContext*)ctx;
    VariableContext *vars = vm_get_var(vm);
    BValue *val = var_lookup(vars, var_name, false);
    if (val && val->type == VAL_STRING && val->as.string) {
        const char *s = str_data(val->as.string);
        if (s) {
            size_t len = runtime_strlen(s);
            if (len >= buf_size) len = buf_size - 1;
            runtime_memcpy(out_buf, s, len);
            out_buf[len] = '\0';
            return true;
        }
    }
    out_buf[0] = '\0';
    return false;
}

BPP_API const char *bpp_get_string(BppEngineContext *ctx, const char *name) {
    if (!ctx || !name) return "";
    VMContext *vm = (VMContext*)ctx;
    VariableContext *vc = vm_get_var(vm);
    if (!vc) return "";
    BValue *v = var_lookup(vc, name, false);
    if (!v || v->type != VAL_STRING || !v->as.string) return "";
    return str_data(v->as.string);
}

BPP_API int bpp_set_var_str(BppEngineContext *ctx, const char *var_name, const char *value) {
    if (!ctx || !var_name) return -1;
    VMContext *vm = (VMContext*)ctx;
    VariableContext *vars = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);

    BValue val;
    runtime_memset(&val, 0, sizeof(val));
    val.type = VAL_STRING;
    val.as.string = str_create(str_ctx, value ? value : "", value ? runtime_strlen(value) : 0);

    bool ok = var_assign(vars, var_name, val);
    str_release(str_ctx, val.as.string);
    return ok ? 0 : -1;
}

BPP_API int bpp_set_string(BppEngineContext *ctx, const char *name, const char *val) {
    return bpp_set_var_str(ctx, name, val);
}

BPP_API int bpp_array_bind_view(BppEngineContext *ctx, const char *arr_name, int elem_type, void *data_ptr, size_t count) {
    if (!ctx || !arr_name || !data_ptr || count == 0) return -1;
    VMContext *vm = (VMContext*)ctx;
    ArrayContext *ac = vm_get_arr(vm);
    if (!ac) return -1;

    int bounds[1] = { (int)count - 1 };
    arr_dim(ac, arr_name, 1, bounds);
    BppError err = {0};
    if (elem_type == BPP_VAL_NUMBER) {
        arr_set_type(ac, arr_name, VAL_NUMBER);
        double *darr = (double *)data_ptr;
        for (size_t i = 0; i < count; i++) {
            int idx[1] = { (int)i };
            BValue *el = arr_get_element(ac, arr_name, 1, idx, &err);
            if (el) {
                el->type = VAL_NUMBER;
                el->as.number = darr[i];
            }
        }
    } else if (elem_type == BPP_VAL_INTEGER) {
        arr_set_type(ac, arr_name, VAL_INTEGER);
        int64_t *iarr = (int64_t *)data_ptr;
        for (size_t i = 0; i < count; i++) {
            int idx[1] = { (int)i };
            BValue *el = arr_get_element(ac, arr_name, 1, idx, &err);
            if (el) {
                el->type = VAL_INTEGER;
                el->as.number = (double)iarr[i];
            }
        }
    }
    return 0;
}

BPP_API void bpp_bus_publish(BppEngineContext *ctx, const char *topic, const char *payload, size_t len) {
    (void)ctx;
    msg_broker_publish(topic, payload, len);
}

BPP_API void bpp_bus_subscribe(BppEngineContext *ctx, const char *topic, BppBusCallbackFn callback_fn, void *userdata) {
    (void)ctx;
    msg_broker_subscribe(topic, (MsgBusCallback)callback_fn, userdata);
}

BPP_API void bpp_bus_unsubscribe(BppEngineContext *ctx, const char *topic, BppBusCallbackFn callback_fn) {
    (void)ctx;
    msg_broker_unsubscribe(topic, (MsgBusCallback)callback_fn);
}

BPP_API int bpp_ipc_send(BppEngineContext *ctx, const char *endpoint, const char *payload, size_t len) {
    (void)ctx;
    return msg_broker_send_ipc(endpoint, payload, len) ? 0 : -1;
}

BPP_API int bpp_ipc_recv(BppEngineContext *ctx, const char *endpoint, char *out_buf, size_t max_len) {
    (void)ctx;
    return msg_broker_recv_ipc(endpoint, out_buf, max_len, 0) ? 0 : -1;
}

BPP_API void bpp_log_add_sink(BppEngineContext *ctx, BppLogSinkFn sink_fn, int min_level, void *userdata) {
    (void)ctx;
    logger_add_sink((BppLogSinkFn)sink_fn, (BppLogLevel)min_level, userdata);
}

BPP_API void bpp_log_remove_sink(BppEngineContext *ctx, BppLogSinkFn sink_fn) {
    (void)ctx;
    logger_remove_sink((BppLogSinkFn)sink_fn);
}

BPP_API void bpp_log_msg(BppEngineContext *ctx, int level, const char *tag, const char *msg) {
    (void)ctx;
    log_emit((BppLogLevel)level, tag, "%s", msg ? msg : "");
}

BPP_API const char *bpp_log_level_name(int level) {
    return logger_level_to_str((BppLogLevel)level);
}

BPP_API void bpp_set_console_output_cb(BppEngineContext *ctx, BppConsoleOutputCb cb, void *userdata) {
    (void)ctx;
    (void)cb;
    (void)userdata;
}

BPP_API const char* bpp_version_string(void) {
    return BASIC_VERSION_STRING;
}

BPP_API const char* bpp_version(void) {
    return BASIC_VERSION_STRING;
}

BPP_API const char *basicpp_version_string(void) {
    return "6.5.2";
}

BPP_API int basicpp_version_major(void) {
    return 6;
}

BPP_API int basicpp_version_minor(void) {
    return 5;
}

BPP_API int basicpp_version_patch(void) {
    return 2;
}

BPP_API const InteropError *basicpp_get_last_error(void) {
    return interop_error_get_last();
}

BPP_API void basicpp_clear_error(void) {
    interop_error_clear();
}

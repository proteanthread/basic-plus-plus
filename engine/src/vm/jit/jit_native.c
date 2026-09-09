// FILENAME: jit_native.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (jit_manager.c)
// NEEDS: libcore, libengine, libplatform
// Implements Tier 2 Native C JIT compilation and dynamic symbol binding.
//
// ---- Includes ----

#include "vm/jit.h"
#include "vm/vm.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

typedef struct {
    void *dl_handle;
    char temp_lib_path[260];
} NativeJitEntry;

static NativeJitEntry s_native_entries[16];
static int s_native_count = 0;

void jit_native_shutdown(void) {
    for (int i = 0; i < s_native_count; i++) {
        if (s_native_entries[i].dl_handle) {
            platform_free_library(s_native_entries[i].dl_handle);
            s_native_entries[i].dl_handle = NULL;
        }
        if (s_native_entries[i].temp_lib_path[0] != '\0') {
            platform_remove(s_native_entries[i].temp_lib_path);
            s_native_entries[i].temp_lib_path[0] = '\0';
        }
    }
    s_native_count = 0;
}

bool jit_native_compile_and_load(VMContext *vm, const char *c_source, const char *func_name, JitNativeFn *out_fn) {
    if (!vm || !c_source || !func_name || !out_fn) return false;
    *out_fn = NULL;

    if (s_native_count >= 16) return false;

    char temp_c[260];
    char temp_dll[260];
    static int s_gen_id = 0;
    s_gen_id++;

#ifdef _WIN32
    runtime_snprintf(temp_c, sizeof(temp_c), "build_win\\jit_tmp_%d.c", s_gen_id);
    runtime_snprintf(temp_dll, sizeof(temp_dll), "build_win\\jit_tmp_%d.dll", s_gen_id);
#else
    runtime_snprintf(temp_c, sizeof(temp_c), "/tmp/jit_tmp_%d.c", s_gen_id);
    runtime_snprintf(temp_dll, sizeof(temp_dll), "/tmp/jit_tmp_%d.so", s_gen_id);
#endif

    void *f = platform_file_open(temp_c, "w");
    if (!f) return false;

    size_t slen = runtime_strlen(c_source);
    platform_file_write(f, c_source, slen);
    platform_file_close(f);

    char cmd[512];
#ifdef _WIN32
    runtime_snprintf(cmd, sizeof(cmd), "cl /O2 /LD /nologo /Fe:\"%s\" \"%s\" > nul 2>&1", temp_dll, temp_c);
#else
    runtime_snprintf(cmd, sizeof(cmd), "gcc -O3 -shared -fPIC -o \"%s\" \"%s\" > /dev/null 2>&1", temp_dll, temp_c);
#endif

    platform_execute_command(cmd);
    platform_remove(temp_c);

    void *handle = platform_load_library(temp_dll);
    if (!handle) {
        platform_remove(temp_dll);
        return false;
    }

    void *sym = platform_get_proc_address(handle, func_name);
    if (!sym) {
        platform_free_library(handle);
        platform_remove(temp_dll);
        return false;
    }

    int idx = s_native_count++;
    s_native_entries[idx].dl_handle = handle;
    runtime_strncpy(s_native_entries[idx].temp_lib_path, temp_dll, sizeof(s_native_entries[idx].temp_lib_path) - 1);

    *out_fn = (JitNativeFn)sym;
    return true;
}

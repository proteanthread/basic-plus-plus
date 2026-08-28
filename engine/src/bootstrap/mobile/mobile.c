// FILENAME: mobile.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: 
// NEEDS: libboot (boot.h)
// NEEDS: libcore (hal.h, logger.h, logger.c)
// NEEDS: libkernel (config.h, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for mobile within BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"
#include "hal/hal.h"

static VMContext *g_mobile_vm = NULL;

int mobile_engine_init(size_t heap_mb) {
    platform_init();
    logger_init(NULL, NULL);

    size_t mem_size = (heap_mb > 0 ? heap_mb : 128L) * 1024L * 1024L; // Default 128 MB Mobile allocation
    g_mobile_vm = boot_system(mem_size);
    if (!g_mobile_vm) {
        return -1;
    }
    return 0;
}

int mobile_engine_exec(const char *code) {
    if (!g_mobile_vm || !code) return -1;
    BppError err = vm_execute_line(g_mobile_vm, code);
    return err.code;
}

void mobile_engine_pause(void) {
    // Mobile OS pause lifecycle hook
}

void mobile_engine_resume(void) {
    // Mobile OS resume lifecycle hook
}

void mobile_engine_shutdown(void) {
    if (g_mobile_vm) {
        boot_shutdown_vm(g_mobile_vm);
        g_mobile_vm = NULL;
    }
    platform_shutdown();
}

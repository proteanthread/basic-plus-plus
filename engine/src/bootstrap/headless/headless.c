// FILENAME: headless.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: 
// NEEDS: libboot (boot.h)
// NEEDS: libcore (logger.h, logger.c, string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (config.h, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for headless within BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"

#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

int main(int argc, char **argv) {
    platform_init();
    bool enable_logging = false;
    for (int i = 1; i < argc; i++) {
        if (runtime_strcmp(argv[i], "--runtime_log") == 0 || runtime_strncmp(argv[i], "--runtime_log=", 6) == 0 ||
            runtime_strcmp(argv[i], "--debug") == 0 || runtime_strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            break;
        }
    }
    if (enable_logging) {
        logger_init("headless.log", "headless.out");
    } else {
        logger_init(NULL, NULL);
    }

    size_t mem_size = 256L * 1024L * 1024L; // 256 MB Headless allocation
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: Headless Engine boot failed.\n");
        return 1;
    }

    if (argc > 1) {
        vm_set_running(vm, true);
        BppError err = vm_load_program_file(vm, argv[1]);
        if (err.code != 0) {
            fprintf(stderr, "Error %d loading %s: %s\n", err.code, argv[1], err.message);
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        size_t prog_count = 0;
        mem_program_get_all(vm_get_mem(vm), &prog_count);
        if (prog_count > 0) {
            err = vm_execute_line(vm, "RUN");
            if (err.code != 0) {
                fprintf(stderr, "Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
            }
        }
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    fprintf(stderr, "Usage: baspp_headless <script.bas>\n");
    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

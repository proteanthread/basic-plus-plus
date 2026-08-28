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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    platform_init();
    bool enable_logging = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0 || strncmp(argv[i], "--log=", 6) == 0 ||
            strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "--trace") == 0) {
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
        BppError err = vm_load_program_file(vm, argv[1]);
        if (err.code != 0) {
            fprintf(stderr, "Error %d loading %s: %s\n", err.code, argv[1], err.message);
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        err = vm_execute_line(vm, "RUN");
        if (err.code != 0) {
            fprintf(stderr, "Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
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

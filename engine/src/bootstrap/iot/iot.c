/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file iot.c
 * @brief BASIC++ Lite REPL Target (`bpp` / `bpp.exe`) entry point implementation.
 *
 * 1. WHAT IT DOES:
 * Implements `main()` for the Lite IoT edition (`bpp`/`bpp.exe`), initializing 384 MB default memory pool (`402653184L` bytes) and Apple II / Commodore style (`]` prompt, `Ready.`).
 *
 * 2. WHY IT EXISTS:
 * Serves as the lightweight, headless REPL executable for embedded microcontrollers, retro terminals, and IoT devices. Excludes SDL2 graphics, BGI, SDL audio, TUI multiplexer, and segmented memory.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Configures `libbpp` runtime, boots 9-phase engine, displays Lite banner (`BASIC++ Lite v6.5.2\n384 MB RAM Available.\n\nReady.\n`), and loops on `]` prompt.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into executable target 'bpp' (linking 'libbpp'). Includes "bootstrap/boot.h", "vm/vm.h", "types/version.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Target entry point specifically for Lite Edition (`bpp`). Excluded from `baspp` and `bs`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Adjust default memory pool size or custom CLI flags for IoT serial connections.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory banner (`BASIC++ Lite v6.5.2\n384 MB RAM Available.\n\nReady.\n`) and prompt (`] `).
 *
 * 8. WHAT TO EXPECT:
 * Command-line startup opens interactive terminal prompt without heavy GUI or SDL dependencies.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `libbpp` symbol linkages and console input read loops.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Terminal stdin/stdout stream available.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero platform-specific GUI dependencies.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bootstrap/common/boot.c
 * Prerequisite Header Files:
 * - engine/include/bootstrap/boot.h
 * - engine/include/vm/vm.h
 * - engine/include/types/version.h
 */

#include "core/boot.h"
#include "types/config.h"
#include "types/version.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    platform_init();
    logger_init("bpp.log", "bpp.out");

    size_t mem_size = 384L * 1024L * 1024L; /* 384 MB IoT allocation */
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: IoT Engine boot failed.\n");
        return 1;
    }

    if (argc > 1) {
        if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            BppError err = vm_execute_line(vm, argv[2]);
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        } else if (argv[1][0] != '-') {
            BppError err = vm_load_program_file(vm, argv[1]);
            if (err.code != 0) {
                fprintf(stderr, "Error %d: %s\n", err.code, err.message);
                boot_shutdown_vm(vm);
                platform_shutdown();
                return err.code;
            }
            err = vm_execute_line(vm, "RUN");
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "BASIC++ IoT Edition v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    vdev_printf(vdev, "%zu MB RAM Available. Ready.\n\n", mem_size / (1024 * 1024));

    vm_set_running(vm, true);

    char input_buf[1024];
    while (vm_is_running(vm) && !vm_exit_requested(vm)) {
        vdev_puts(vdev, "iot> ");
        if (!vdev_gets(vdev, input_buf, sizeof(input_buf))) {
            break;
        }

        char *p = input_buf;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        if (isdigit((unsigned char)*p)) {
            BppLineNumber line_num = (BppLineNumber)atof(p);
            while (isdigit((unsigned char)*p) || *p == '.') p++;
            while (isspace((unsigned char)*p)) p++;

            if (*p == '\0') {
                mem_program_remove(vm_get_mem(vm), line_num);
            } else {
                mem_program_store(vm_get_mem(vm), line_num, p);
            }
        } else {
            BppError err = vm_execute_line(vm, p);
            if (err.code != 0) {
                vdev_printf(vdev, "Error %d: %s\n", err.code, err.message);
            }
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

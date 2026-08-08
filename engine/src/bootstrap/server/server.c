/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file server.c
 * @brief BASIC++ Batch Script Runner Target (`bs` / `bs.exe`) entry point implementation.
 *
 * 1. WHAT IT DOES:
 * Implements `main()` for the non-interactive batch script runner (`bs`/`bs.exe`), initializing 64 MB default memory pool (`67108864L` bytes).
 *
 * 2. WHY IT EXISTS:
 * Optimized for headless automation, PowerShell/Bash scripting, CGI web pipelines, and cron jobs with ZERO interactive REPL prompts or startup banners.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Configures `libbpp` runtime, boots engine, loads BASIC script from CLI arguments, executes non-interactively, and exits with script return code.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into executable target 'bs' (linking 'libbpp'). Includes "bootstrap/boot.h", "types/config.h", "vm/vm.h", "types/version.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Target entry point specifically for Script Runner Edition (`bs`). Excluded from `baspp` and `bpp`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support CLI shebang parsing (`#!/usr/bin/env bs`) or piped stdin script execution.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Zero banner, zero prompt, zero REPL iteration invariant — batch script runner must execute non-interactively and terminate.
 *
 * 8. WHAT TO EXPECT:
 * Executes target BASIC script silently, returning exit code 0 on clean completion or non-zero error code on runtime error.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect stderr output for script line number tracebacks.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid path to BASIC source script provided as CLI argument.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance across all operating systems.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bootstrap/common/boot.c
 * Prerequisite Header Files:
 * - engine/include/bootstrap/boot.h
 * - engine/include/vm/vm.h
 * - engine/include/types/config.h
 * - engine/include/types/version.h
 */

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
    logger_init("bs.log", "bs.out");

    size_t mem_size = 64L * 1024L * 1024L; /* 64 MB Batch allocation */
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: Batch Script Engine boot failed.\n");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: bs <script.bas> [args...]\n");
        boot_shutdown_vm(vm);
        platform_shutdown();
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0 && argc > 2) {
        BppError err = vm_execute_line(vm, argv[2]);
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

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

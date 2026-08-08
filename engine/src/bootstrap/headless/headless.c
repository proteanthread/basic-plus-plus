/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file headless.c
 * @brief Headless Server Node bootstrap entry point.
 *
 * WHAT IT DOES:
 * Implements a non-interactive headless engine node runner for background daemon processes,
 * socket listeners, and headless cloud instances.
 *
 * WHY IT EXISTS:
 * Dedicated micro-BASIC bootstrap profile for server instances with no display or keyboard input.
 *
 * WHY IT WORKS THIS WAY:
 * Initializes platform services, loads specified background scripts or listens on virtual net socket streams.
 *
 * WHAT CAN BE CHANGED:
 * Daemon startup logs and default heap sizing.
 *
 * WHAT CANNOT BE CHANGED:
 * Headless virtual device binding invariants.
 *
 * WHAT TO EXPECT:
 * Executes program in background without opening interactive terminal prompts.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Inspect daemon log files or stdout redirection logs.
 *
 * ASSUMPTIONS:
 * Script path or network socket configuration provided via CLI.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance.
 *
 * FUTURE EXPANSIONS:
 * Systemd daemon service integration.
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
    logger_init("headless.log", "headless.out");

    size_t mem_size = 256L * 1024L * 1024L; /* 256 MB Headless allocation */
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

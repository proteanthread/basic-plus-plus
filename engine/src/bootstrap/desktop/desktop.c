/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file desktop.c
 * @brief Desktop Edition (`baspp.exe` / `baspp`) bootstrap entry point for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements main entry point `main()` for `baspp.exe` (Standard Desktop Edition REPL & TUI Editor with 640 MB default memory pool).
 *
 * 2. WHY IT EXISTS:
 * Serves as the flagship desktop executable entry point for interactive programming, script execution, and TUI editing.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Executes 9-phase boot sequence (`boot_initialize`), parses CLI flags (`-c`, `-v`, `-h`), enters interactive REPL prompt (`> `), and handles program lifecycle.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Main file for executable target 'baspp'. Includes "core/boot.h", "vm/vm.h", "platform/platform.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Compiled exclusively into the standard flagship desktop executable `baspp` / `baspp.exe`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add desktop-specific CLI arguments or banner customization.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard prompt (`> `) and banner (`BASIC++ Standard Edition v6.5.2\n640 MB RAM Available.\n\nOk\n`).
 *
 * 8. WHAT TO EXPECT:
 * Enters interactive REPL prompt on boot or executes command string passed via `-c`.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify 9-phase boot sequence in `bootstrap/common/boot.c`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Active interactive console terminal session.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance (`main(int argc, char **argv)` signature).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bootstrap/common/boot.c
 * - engine/src/vm/context.c
 * - engine/src/platform/platform.c
 * Prerequisite Header Files:
 * - engine/include/core/boot.h
 * - engine/include/vm/vm.h
 * - engine/include/platform/platform.h
 */

#include "core/boot.h"
#include "types/config.h"
#include "types/version.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "runtime/vfs.h"
#include "editor/editor.h"
#include "debug/logger.h"
#include "docgen/docgen.h"

extern void tui_multiplexer_init(void);
extern void tui_multiplexer_shutdown(void);
extern void vdev_gfx_force_flush(void);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    platform_init();
    logger_init("baspp.log", "baspp.out");

    size_t mem_size = 671088640L; /* 640 MB Default Allocation */
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: Engine boot failed.\n");
        return 1;
    }

    tui_multiplexer_init();

    if (argc > 1) {
        if (strcmp(argv[1], "--edit") == 0 || strcmp(argv[1], "-e") == 0) {
            const char *fname = (argc > 2) ? argv[2] : NULL;
            editor_manager_run(vm, "edit", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[1], "--vi") == 0) {
            const char *fname = (argc > 2) ? argv[2] : NULL;
            editor_manager_run(vm, "vi", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[1], "--ws") == 0) {
            const char *fname = (argc > 2) ? argv[2] : NULL;
            editor_manager_run(vm, "ws", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[1], "--edlin") == 0) {
            const char *fname = (argc > 2) ? argv[2] : NULL;
            editor_manager_run(vm, "edlin", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            BppError err = vm_execute_line(vm, argv[2]);
            if (err.code != 0) {
                fprintf(stderr, "Error %d: %s\n", err.code, err.message);
            }
            vdev_gfx_force_flush();
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        } else if (strcmp(argv[1], "--export-docs") == 0 || strcmp(argv[1], "--export-api-json") == 0) {
            const char *format = (argc > 2) ? argv[2] : "json";
            const char *target = (argc > 3) ? argv[3] : "docs/api";
            bool success = false;

            if (strcmp(format, "json") == 0 || strcmp(argv[1], "--export-api-json") == 0) {
                const char *outfile = (argc > 2 && argv[2][0] != '-') ? argv[2] : "api_schema.json";
                success = docgen_export_json(outfile);
                if (success) printf("Successfully exported API JSON schema to %s\n", outfile);
            } else if (strcmp(format, "markdown") == 0 || strcmp(format, "md") == 0) {
                success = docgen_export_markdown(target);
                if (success) printf("Successfully exported Markdown documentation to %s/\n", target);
            } else if (strcmp(format, "html") == 0) {
                const char *outfile = (argc > 3) ? argv[3] : "api_reference.html";
                success = docgen_export_html(outfile);
                if (success) printf("Successfully exported HTML documentation manual to %s\n", outfile);
            } else {
                success = docgen_export_all(target);
                if (success) printf("Successfully exported all documentation packages to %s/\n", target);
            }

            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return success ? 0 : 1;
        } else if (argv[1][0] != '-') {
            BppError err = vm_load_program_file(vm, argv[1]);
            if (err.code != 0) {
                fprintf(stderr, "Error %d: %s\n", err.code, err.message);
                tui_multiplexer_shutdown();
                boot_shutdown_vm(vm);
                platform_shutdown();
                return err.code;
            }
            err = vm_execute_line(vm, "RUN");
            if (err.code != 0) {
                fprintf(stderr, "Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
            }
            vdev_gfx_force_flush();
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
    }

    /* Interactive REPL Loop */
    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "BASIC++ Standard Edition v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    vdev_printf(vdev, "%zu MB RAM Available.\n\nOk\n", mem_size / (1024 * 1024));

    vm_set_running(vm, true);

    char input_buf[1024];
    while (vm_is_running(vm) && !vm_exit_requested(vm)) {
        vdev_puts(vdev, "> ");
        if (!vdev_gets(vdev, input_buf, sizeof(input_buf))) {
            break;
        }

        /* Trim leading whitespace */
        char *p = input_buf;
        while (isspace((unsigned char)*p)) p++;

        if (*p == '\0') {
            vdev_puts(vdev, "Ok\n");
            continue;
        }

        /* Check if line starts with a number */
        if (isdigit((unsigned char)*p)) {
            BppLineNumber line_num = (BppLineNumber)atof(p);
            while (isdigit((unsigned char)*p) || *p == '.') p++;
            while (isspace((unsigned char)*p)) p++;

            if (*p == '\0') {
                /* Delete line */
                mem_program_remove(vm_get_mem(vm), line_num);
            } else {
                /* Store line */
                mem_program_store(vm_get_mem(vm), line_num, p);
            }
            vdev_puts(vdev, "Ok\n");
        } else {
            /* Execute immediate statement */
            BppError err = vm_execute_line(vm, p);
            if (err.code != 0) {
                vdev_printf(vdev, "Error %d: %s\n", err.code, err.message);
            }
            vdev_gfx_force_flush();
            if (!vm_exit_requested(vm)) {
                vdev_puts(vdev, "Ok\n");
            }
        }
    }

    tui_multiplexer_shutdown();
    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

// FILENAME: desktop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c), standalone binary entry point
// NEEDS: libboot, libcore, libengine, libkernel, libplatform, libstandard
// Provides core logic and interface definitions for desktop within BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "types/version.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "runtime/vfs.h"
#include "editor/editor.h"
#include "debug/logger.h"
#include "docgen/docgen.h"
#include "interop/interop_core.h"
#include "interop/interop_ipc.h"
#include "interop/interop_jsonrpc.h"
#include "eval/functions/system/environment/command_fn.h"

extern void tui_multiplexer_init(void);
extern void tui_multiplexer_shutdown(void);
extern void vdev_gfx_force_flush(void);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void prompt_pause_if_needed(bool pause_on_error) {
    if (!pause_on_error) return;
    printf("\nPress Enter to continue . . .\n");
    fflush(stdout);
    char buf[256];
    if (!fgets(buf, sizeof(buf), stdin)) {
        // EOF on stdin
    }
}

int main(int argc, char **argv) {
    platform_init();
    bool enable_logging = false;
    bool pause_on_error = true;
    bool batch_mode = false;
    bool show_timer = false;
    double execution_timeout_ms = 0.0;
    const char *custom_log = NULL;
    const char *script_file = NULL;
    const char *cmd_expr = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0) {
            enable_logging = true;
            custom_log = "baspp.log";
        } else if (strncmp(argv[i], "--log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (strncmp(argv[i], "--timeout=", 10) == 0) {
            execution_timeout_ms = atof(argv[i] + 10);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            execution_timeout_ms = atof(argv[++i]);
        } else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            custom_log = "baspp.log";
        } else if (strcmp(argv[i], "--timer") == 0 || strcmp(argv[i], "-t") == 0) {
            show_timer = true;
        } else if (strcmp(argv[i], "--no-pause") == 0 || strcmp(argv[i], "-np") == 0 || strcmp(argv[i], "--batch") == 0) {
            pause_on_error = false;
            batch_mode = true;
        } else if (strcmp(argv[i], "--pause") == 0 || strcmp(argv[i], "-p") == 0) {
            pause_on_error = true;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cmd_expr = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--edit") == 0 || strcmp(argv[i], "-e") == 0) {
            const char *fname = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : NULL;
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            editor_manager_run(vm, "edit", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[i], "--vi") == 0) {
            const char *fname = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : NULL;
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            editor_manager_run(vm, "vi", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[i], "--ws") == 0) {
            const char *fname = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : NULL;
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            editor_manager_run(vm, "ws", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[i], "--edlin") == 0) {
            const char *fname = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : NULL;
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            editor_manager_run(vm, "edlin", fname);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 0;
        } else if (strcmp(argv[i], "--export-docs") == 0 || strcmp(argv[i], "--export-api-json") == 0) {
            const char *format = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : "json";
            const char *target = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : "docs/api";
            bool success = false;
            if (strcmp(format, "json") == 0 || strcmp(argv[i], "--export-api-json") == 0) {
                const char *outfile = (i < argc && argv[i][0] != '-') ? argv[i] : "api_schema.json";
                success = docgen_export_json(outfile);
                if (success) printf("Successfully exported API JSON schema to %s\n", outfile);
            } else if (strcmp(format, "markdown") == 0 || strcmp(format, "md") == 0) {
                success = docgen_export_markdown(target);
                if (success) printf("Successfully exported Markdown documentation to %s/\n", target);
            } else if (strcmp(format, "html") == 0) {
                const char *outfile = (i < argc && argv[i][0] != '-') ? argv[i] : "api_reference.html";
                success = docgen_export_html(outfile);
                if (success) printf("Successfully exported HTML documentation manual to %s\n", outfile);
            } else {
                success = docgen_export_all(target);
                if (success) printf("Successfully exported all documentation packages to %s/\n", target);
            }
            platform_shutdown();
            return success ? 0 : 1;
        } else if (strcmp(argv[i], "--server") == 0) {
            interop_init();
            int port = 0;
            if (i + 1 < argc && argv[i + 1][0] != '-') port = atoi(argv[++i]);
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            int result = interop_ipc_serve(vm, port);
            interop_shutdown();
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return result;
        } else if (strcmp(argv[i], "--jsonrpc") == 0) {
            interop_init();
            int port = (i + 1 < argc && argv[i + 1][0] != '-') ? atoi(argv[++i]) : 9100;
            VMContext *vm = boot_system(671088640L);
            tui_multiplexer_init();
            int result = interop_ipc_serve(vm, port);
            interop_shutdown();
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return result;
        } else if (argv[i][0] != '-' && !script_file) {
            script_file = argv[i];
            char cmd_line[2048] = {0};
            for (int j = i + 1; j < argc; j++) {
                if (j > i + 1) strncat(cmd_line, " ", sizeof(cmd_line) - strlen(cmd_line) - 1);
                strncat(cmd_line, argv[j], sizeof(cmd_line) - strlen(cmd_line) - 1);
            }
            runtime_set_command_line(cmd_line);
            break;
        }
    }

    if (enable_logging) {
        logger_init(custom_log ? custom_log : "baspp.log", "baspp.out");
    } else {
        logger_init(NULL, NULL);
    }

    size_t mem_size = 671088640L; // 640 MB Default Allocation
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: Engine boot failed.\n");
        return 1;
    }

    if (execution_timeout_ms > 0.0) {
        vm_set_timeout(vm, execution_timeout_ms);
    }

    tui_multiplexer_init();

    if (cmd_expr) {
        double t_start = platform_get_highres_time();
        BppError err = vm_execute_line(vm, cmd_expr);
        double t_end = platform_get_highres_time();
        if (show_timer) {
            fprintf(stderr, "[Timer: %.3f ms]\n", (t_end - t_start) * 1000.0);
        }
        if (err.code != 0 || vm_has_error(vm)) {
            if (err.code != 0) {
                fprintf(stderr, "Error %d: %s\n", err.code, err.message);
            } else {
                BppError vm_err = vm_get_error(vm);
                fprintf(stderr, "Error %d: %s\n", vm_err.code, vm_err.message);
                err.code = vm_err.code;
            }
            prompt_pause_if_needed(pause_on_error);
        }
        vdev_gfx_force_flush();
        tui_multiplexer_shutdown();
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    if (script_file) {
        BppError err = vm_load_program_file(vm, script_file);
        if (err.code != 0) {
            fprintf(stderr, "Error %d: %s\n", err.code, err.message);
            prompt_pause_if_needed(pause_on_error);
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        double t_start = platform_get_highres_time();
        err = vm_execute_line(vm, "RUN");
        double t_end = platform_get_highres_time();
        if (show_timer) {
            fprintf(stderr, "[Timer: %.3f ms]\n", (t_end - t_start) * 1000.0);
        }
        if (err.code != 0) {
            fprintf(stderr, "Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
        }
        vdev_gfx_force_flush();
        if (batch_mode) {
            if (err.code != 0 || vm_has_error(vm)) {
                if (err.code == 0) {
                    BppError vm_err = vm_get_error(vm);
                    err.code = vm_err.code;
                }
                prompt_pause_if_needed(pause_on_error);
            }
            tui_multiplexer_shutdown();
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        // Enter REPL loop after program completes so window stays open
    }

    // Interactive REPL Loop
    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "BASIC++ Standard Edition v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    vdev_printf(vdev, "%zu MB RAM Available.\n\nOk\n", mem_size / (1024 * 1024));

    vm_set_running(vm, true);

    char input_buf[1024];
    while (!vm_exit_requested(vm)) {
        vdev_puts(vdev, "> ");
        if (!vdev_gets(vdev, input_buf, sizeof(input_buf))) {
            if (platform_stdin_is_console()) {
                vm_reset_break(vm);
                vdev_puts(vdev, "\n");
                continue;
            }
            break;
        }

        // Trim leading whitespace
        char *p = input_buf;
        while (isspace((unsigned char)*p)) p++;

        if (*p == '\0') {
            continue;
        }

        // Check if line starts with a number
        if (isdigit((unsigned char)*p)) {
            BppLineNumber line_num = (BppLineNumber)atof(p);
            while (isdigit((unsigned char)*p) || *p == '.') p++;
            while (isspace((unsigned char)*p)) p++;

            if (*p == '\0') {
                // Delete line
                mem_program_remove(vm_get_mem(vm), line_num);
            } else {
                // Store line
                mem_program_store(vm_get_mem(vm), line_num, p);
            }
        } else {
            // Execute immediate statement
            BppError err = vm_execute_line(vm, p);
            if (err.code != 0) {
                if (strcmp(err.message, "Break") == 0) {
                    BppLineNumber cur_line = vm_get_current_line(vm);
                    if (cur_line > 0.0) {
                        vdev_printf(vdev, "Break in line %lld\n", (long long)cur_line);
                    } else {
                        vdev_puts(vdev, "Break\n");
                    }
                } else {
                    vdev_printf(vdev, "Error %d: %s\n", err.code, err.message);
                }
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

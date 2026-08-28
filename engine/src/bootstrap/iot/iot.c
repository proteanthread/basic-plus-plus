// FILENAME: iot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: bpp.exe (iot.c), standalone binary entry point
// NEEDS: libboot (boot.h)
// NEEDS: libcore (ctype.h, ctype.c, logger.h, logger.c, string.h)
// NEEDS: libengine (command_fn.h, command_fn.c, string.c, version.c)
// NEEDS: libkernel (config.h, vdev.h, vdev.c, version.h)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for iot within BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "types/version.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"
#include "eval/functions/system/environment/command_fn.h"

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
            custom_log = "bpp.log";
        } else if (strncmp(argv[i], "--log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (strncmp(argv[i], "--timeout=", 10) == 0) {
            execution_timeout_ms = atof(argv[i] + 10);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            execution_timeout_ms = atof(argv[++i]);
        } else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            custom_log = "bpp.log";
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
        logger_init(custom_log ? custom_log : "bpp.log", "bpp.out");
    } else {
        logger_init(NULL, NULL);
    }

    size_t mem_size = 384L * 1024L * 1024L; // 384 MB IoT allocation
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: IoT Engine boot failed.\n");
        return 1;
    }

    if (execution_timeout_ms > 0.0) {
        vm_set_timeout(vm, execution_timeout_ms);
    }

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
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    if (script_file) {
        BppError err = vm_load_program_file(vm, script_file);
        if (err.code != 0) {
            fprintf(stderr, "Error %d: %s\n", err.code, err.message);
            prompt_pause_if_needed(pause_on_error);
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
        if (batch_mode) {
            if (err.code != 0 || vm_has_error(vm)) {
                if (err.code == 0) {
                    BppError vm_err = vm_get_error(vm);
                    err.code = vm_err.code;
                }
                prompt_pause_if_needed(pause_on_error);
            }
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        // Enter REPL loop after program completes so window stays open
    }

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "BASIC++ Lite Edition v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    vdev_printf(vdev, "%zu MB RAM Available.\n\nReady.\n", mem_size / (1024 * 1024));

    vm_set_running(vm, true);

    char input_buf[1024];
    while (!vm_exit_requested(vm)) {
        vdev_puts(vdev, "] ");
        if (!vdev_gets(vdev, input_buf, sizeof(input_buf))) {
            if (platform_stdin_is_console()) {
                vm_reset_break(vm);
                vdev_puts(vdev, "\n");
                continue;
            }
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
            if (!vm_exit_requested(vm)) {
                vdev_puts(vdev, "Ready.\n");
            }
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

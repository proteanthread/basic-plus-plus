// FILENAME: server.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: bs.exe (server.c), standalone binary entry point
// NEEDS: libboot (boot.h)
// NEEDS: libcore (logger.h, logger.c, string.h)
// NEEDS: libengine (command_fn.h, command_fn.c, string.c)
// NEEDS: libkernel (config.h, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for server within BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"

#include "eval/functions/system/environment/command_fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    bool pause_on_error = false;
    bool show_timer = false;
    const char *custom_log = NULL;
    const char *script_file = NULL;
    const char *cmd_expr = NULL;

    double timeout_sec = 10.0; // 10-second default watchdog timeout for non-interactive batch runner
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0) {
            enable_logging = true;
            custom_log = "bs.log";
        } else if (strncmp(argv[i], "--log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            custom_log = "bs.log";
        } else if (strcmp(argv[i], "--timer") == 0 || strcmp(argv[i], "-t") == 0) {
            show_timer = true;
        } else if (strcmp(argv[i], "--no-pause") == 0 || strcmp(argv[i], "-np") == 0 || strcmp(argv[i], "--batch") == 0) {
            pause_on_error = false;
        } else if (strcmp(argv[i], "--pause") == 0 || strcmp(argv[i], "-p") == 0) {
            pause_on_error = true;
        } else if (strncmp(argv[i], "--timeout=", 10) == 0) {
            timeout_sec = atof(argv[i] + 10);
        } else if (strcmp(argv[i], "--no-timeout") == 0) {
            timeout_sec = 0.0;
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
        logger_init(custom_log ? custom_log : "bs.log", "bs.out");
    } else {
        logger_init(NULL, NULL);
    }

    size_t mem_size = 64L * 1024L * 1024L; // 64 MB Batch allocation
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        fprintf(stderr, "FATAL: Batch Script Engine boot failed.\n");
        return 1;
    }

    if (timeout_sec > 0.0) {
        vm_set_timeout(vm, timeout_sec * 1000.0);
    }

    if (cmd_expr) {
        double t_start = platform_get_highres_time();
        BppError err = vm_execute_line(vm, cmd_expr);
        double t_end = platform_get_highres_time();
        if (show_timer) {
            fprintf(stderr, "[Timer: %.3f ms]\n", (t_end - t_start) * 1000.0);
        }
        if (err.code != 0) {
            fprintf(stderr, "Error %d: %s\n", err.code, err.message);
        } else if (vm_has_error(vm)) {
            BppError vm_err = vm_get_error(vm);
            if (vm_err.code != 0) {
                fprintf(stderr, "Error %d: %s\n", vm_err.code, vm_err.message);
            }
        }
        if (err.code != 0 || vm_has_error(vm)) {
            prompt_pause_if_needed(pause_on_error);
        }
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    if (!script_file) {
        fprintf(stderr, "Usage: bs <script.bas> [args...] or bs -c \"<command>\"\n");
        boot_shutdown_vm(vm);
        platform_shutdown();
        return 1;
    }

    BppError err = vm_load_program_file(vm, script_file);
    if (err.code != 0) {
        fprintf(stderr, "Error %d loading %s: %s\n", err.code, script_file, err.message);
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
        prompt_pause_if_needed(pause_on_error);
    } else if (vm_has_error(vm)) {
        BppError vm_err = vm_get_error(vm);
        if (vm_err.code != 0) {
            fprintf(stderr, "Error %d in line %lld: %s\n", vm_err.code, (long long)vm_get_current_line(vm), vm_err.message);
            err.code = vm_err.code;
            prompt_pause_if_needed(pause_on_error);
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return err.code;
}

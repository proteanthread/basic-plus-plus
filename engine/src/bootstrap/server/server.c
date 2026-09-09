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
#include "vm/jit.h"

#include "eval/functions/system/environment/command_fn.h"

#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "runtime/conv/float_parse.h"

static void prompt_pause_if_needed(bool pause_on_error) {
    if (!pause_on_error) return;
    platform_console_puts("\nPress Enter to continue . . .\n");
    platform_console_flush();
    platform_getch();
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
    int jit_mode_val = -1;
    bool jit_fast = false;
    bool is_aot = false;

    for (int i = 1; i < argc; i++) {
        if (runtime_strcmp(argv[i], "--runtime_log") == 0) {
            enable_logging = true;
            custom_log = "bs.log";
        } else if (runtime_strncmp(argv[i], "--runtime_log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (runtime_strcmp(argv[i], "--debug") == 0 || runtime_strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            custom_log = "bs.log";
        } else if (runtime_strcmp(argv[i], "--timer") == 0 || runtime_strcmp(argv[i], "-t") == 0) {
            show_timer = true;
        } else if (runtime_strcmp(argv[i], "--no-pause") == 0 || runtime_strcmp(argv[i], "-np") == 0 || runtime_strcmp(argv[i], "--batch") == 0) {
            pause_on_error = false;
        } else if (runtime_strcmp(argv[i], "--pause") == 0 || runtime_strcmp(argv[i], "-p") == 0) {
            pause_on_error = true;
        } else if (runtime_strncmp(argv[i], "--timeout=", 10) == 0) {
            timeout_sec = runtime_atof(argv[i] + 10);
        } else if (runtime_strcmp(argv[i], "--no-timeout") == 0) {
            timeout_sec = 0.0;
        } else if (runtime_strcmp(argv[i], "--jit") == 0 || runtime_strcmp(argv[i], "--jit=auto") == 0) {
            jit_mode_val = 1;
        } else if (runtime_strcmp(argv[i], "--jit=bytecode") == 0) {
            jit_mode_val = 2;
        } else if (runtime_strcmp(argv[i], "--jit=native") == 0) {
            jit_mode_val = 3;
        } else if (runtime_strcmp(argv[i], "--jit=off") == 0 || runtime_strcmp(argv[i], "--no-jit") == 0) {
            jit_mode_val = 0;
        } else if (runtime_strcmp(argv[i], "--fast") == 0 || runtime_strcmp(argv[i], "-fno-bounds-check") == 0) {
            jit_fast = true;
        } else if (runtime_strcmp(argv[i], "--aot") == 0) {
            is_aot = true;
            jit_mode_val = 2;
        } else if (runtime_strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cmd_expr = argv[i + 1];
            i++;
        } else if (argv[i][0] != '-' && !script_file) {
            script_file = argv[i];
            char cmd_line[2048] = {0};
            for (int j = i + 1; j < argc; j++) {
                if (j > i + 1) runtime_strncat(cmd_line, " ", sizeof(cmd_line) - runtime_strlen(cmd_line) - 1);
                runtime_strncat(cmd_line, argv[j], sizeof(cmd_line) - runtime_strlen(cmd_line) - 1);
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
        platform_console_eputs("FATAL: Batch Script Engine boot failed.\n");
        return 1;
    }

    if (timeout_sec > 0.0) {
        vm_set_timeout(vm, timeout_sec * 1000.0);
    }
    if (jit_mode_val >= 0) {
        jit_set_mode(vm, jit_mode_val);
    }
    if (jit_fast) {
        jit_set_fast_mode(vm, true);
    }

    if (cmd_expr) {
        vm_set_running(vm, true);
        double t_start = platform_get_highres_time();
        BppError err = vm_execute_line(vm, cmd_expr);
        double t_end = platform_get_highres_time();
        if (show_timer) {
            platform_console_eprintf("[Timer: %.3f ms]\n", (t_end - t_start) * 1000.0);
        }
        if (err.code != 0) {
            BppLineNumber cur_line = vm_get_current_line(vm);
            if (cur_line > 0.0) {
                platform_console_eprintf("Error %d in line %lld: %s\n", err.code, (long long)cur_line, err.message);
            } else {
                platform_console_eprintf("Error %d: %s\n", err.code, err.message);
            }
        } else if (vm_has_error(vm)) {
            BppError vm_err = vm_get_error(vm);
            if (vm_err.code != 0) {
                BppLineNumber cur_line = vm_get_current_line(vm);
                if (cur_line > 0.0) {
                    platform_console_eprintf("Error %d in line %lld: %s\n", vm_err.code, (long long)cur_line, vm_err.message);
                } else {
                    platform_console_eprintf("Error %d: %s\n", vm_err.code, vm_err.message);
                }
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
        platform_console_eputs("Usage: bs <script.bas> [args...] or bs -c \"<command>\"\n");
        boot_shutdown_vm(vm);
        platform_shutdown();
        return 1;
    }

    vm_set_running(vm, true);
    double t_start = platform_get_highres_time();
    BppError err = vm_load_program_file(vm, script_file);
    if (err.code != 0) {
        platform_console_eprintf("Error %d loading %s: %s\n", err.code, script_file, err.message);
        prompt_pause_if_needed(pause_on_error);
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    size_t prog_count = 0;
    mem_program_get_all(vm_get_mem(vm), &prog_count);
    if (prog_count > 0) {
        err = is_aot ? jit_compile_and_run_aot(vm) : vm_execute_line(vm, "RUN");
    }
    double t_end = platform_get_highres_time();
    if (show_timer) {
        platform_console_eprintf("[Timer: %.3f ms]\n", (t_end - t_start) * 1000.0);
    }
    if (err.code != 0) {
        if (runtime_strcmp(err.message, "Break") == 0 || vm_break_triggered(vm)) {
            boot_shutdown_vm(vm);
            platform_shutdown();
            return 130;
        }
        platform_console_eprintf("Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
        prompt_pause_if_needed(pause_on_error);
    } else if (vm_has_error(vm)) {
        BppError vm_err = vm_get_error(vm);
        if (vm_err.code != 0) {
            if (runtime_strcmp(vm_err.message, "Break") == 0 || vm_break_triggered(vm)) {
                boot_shutdown_vm(vm);
                platform_shutdown();
                return 130;
            }
            platform_console_eprintf("Error %d in line %lld: %s\n", vm_err.code, (long long)vm_get_current_line(vm), vm_err.message);
            err.code = vm_err.code;
            prompt_pause_if_needed(pause_on_error);
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return err.code;
}

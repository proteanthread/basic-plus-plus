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
#include "vm/jit.h"
#include "eval/functions/system/environment/command_fn.h"

#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
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
    bool pause_on_error = true;
    bool batch_mode = false;
    bool show_timer = false;
    double execution_timeout_ms = 0.0;
    int jit_mode_val = -1;
    bool jit_fast = false;
    bool is_aot = false;
    const char *custom_log = NULL;
    const char *script_file = NULL;
    const char *cmd_expr = NULL;

    for (int i = 1; i < argc; i++) {
        if (runtime_strcmp(argv[i], "--runtime_log") == 0) {
            enable_logging = true;
            custom_log = "bpp.log";
        } else if (runtime_strncmp(argv[i], "--runtime_log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (runtime_strncmp(argv[i], "--timeout=", 10) == 0) {
            execution_timeout_ms = runtime_atof(argv[i] + 10);
        } else if (runtime_strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            execution_timeout_ms = runtime_atof(argv[++i]);
        } else if (runtime_strcmp(argv[i], "--debug") == 0 || runtime_strcmp(argv[i], "--trace") == 0) {
            enable_logging = true;
            custom_log = "bpp.log";
        } else if (runtime_strcmp(argv[i], "--timer") == 0 || runtime_strcmp(argv[i], "-t") == 0) {
            show_timer = true;
        } else if (runtime_strcmp(argv[i], "--no-pause") == 0 || runtime_strcmp(argv[i], "-np") == 0 || runtime_strcmp(argv[i], "--batch") == 0) {
            pause_on_error = false;
            batch_mode = true;
        } else if (runtime_strcmp(argv[i], "--pause") == 0 || runtime_strcmp(argv[i], "-p") == 0) {
            pause_on_error = true;
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
        logger_init(custom_log ? custom_log : "bpp.log", "bpp.out");
    } else {
        logger_init(NULL, NULL);
    }

    size_t mem_size = 384L * 1024L * 1024L; // 384 MB IoT allocation
    VMContext *vm = boot_system(mem_size);
    if (!vm) {
        platform_console_eputs("FATAL: IoT Engine boot failed.\n");
        return 1;
    }

    if (execution_timeout_ms > 0.0) {
        vm_set_timeout(vm, execution_timeout_ms);
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
        if (err.code != 0 || vm_has_error(vm)) {
            if (err.code != 0) {
                platform_console_eprintf("Error %d: %s\n", err.code, err.message);
            } else {
                BppError vm_err = vm_get_error(vm);
                platform_console_eprintf("Error %d: %s\n", vm_err.code, vm_err.message);
                err.code = vm_err.code;
            }
            prompt_pause_if_needed(pause_on_error);
        }
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    if (script_file) {
        vm_set_running(vm, true);
        double t_start = platform_get_highres_time();
        BppError err = vm_load_program_file(vm, script_file);
        if (err.code != 0) {
            platform_console_eprintf("Error %d: %s\n", err.code, err.message);
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
            platform_console_eprintf("Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
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

    int ctrl_c_count = 0;
    double last_ctrl_c_time = 0.0;
    char input_buf[1024];

    while (!vm_exit_requested(vm)) {
        vdev_puts(vdev, "] ");
        if (!vdev_gets(vdev, input_buf, sizeof(input_buf))) {
            if (platform_stdin_is_console()) {
                double now = platform_get_uptime();
                if (now - last_ctrl_c_time <= 1.0) {
                    ctrl_c_count++;
                } else {
                    ctrl_c_count = 1;
                }
                last_ctrl_c_time = now;
                if (ctrl_c_count >= 3) {
                    break;
                }
                vm_reset_break(vm);
                vdev_puts(vdev, "\n");
                continue;
            }
            break;
        }
        ctrl_c_count = 0;

        char *p = input_buf;
        while (runtime_isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        if (runtime_isdigit((unsigned char)*p)) {
            BppLineNumber line_num = (BppLineNumber)runtime_atof(p);
            while (runtime_isdigit((unsigned char)*p) || *p == '.') p++;
            while (runtime_isspace((unsigned char)*p)) p++;

            if (*p == '\0') {
                mem_program_remove(vm_get_mem(vm), line_num);
            } else {
                mem_program_store(vm_get_mem(vm), line_num, p);
            }
        } else {
            BppError err = vm_execute_line(vm, p);
            if (err.code != 0) {
                const char *msg = err.message ? err.message : "Runtime error";
                if (runtime_strcmp(msg, "Break") == 0) {
                    BppLineNumber cur_line = vm_get_current_line(vm);
                    if (cur_line > 0.0) {
                        vdev_printf(vdev, "Break in line %lld\n", (long long)cur_line);
                    } else {
                        vdev_puts(vdev, "Break\n");
                    }
                } else {
                    BppLineNumber cur_line = vm_get_current_line(vm);
                    if (cur_line > 0.0) {
                        vdev_printf(vdev, "Error %d in line %lld: %s\n", err.code, (long long)cur_line, msg);
                    } else {
                        vdev_printf(vdev, "Error %d: %s\n", err.code, msg);
                    }
                }
            } else {
                vdev_puts(vdev, "Ready.\n");
            }
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

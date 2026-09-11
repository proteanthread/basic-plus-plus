// FILENAME: iot_main.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libboot, libcore, libengine, libkernel, libplatform, libserver
// Implements the dedicated IoT and microcontroller edition entry point for BASIC++.
//
// ---- Includes ----

#include "core/boot.h"
#include "types/config.h"
#include "types/version.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"
#include "eval/functions/system/environment/command_fn.h"
#include "esp32_serial.h"
#include "esp32_hal.h"
#include "esp32_regs.h"
#include "iot_sensors.h"
#include "iot_net.h"
#include "iot_runtime.h"
#include "vm/vm.h"
#include "memory/memory.h"

#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/math/math.h"
#include "runtime/conv/float_parse.h"

static IotProfileConfig s_active_config = {
    .device_name = "Generic",
    .ram_size_bytes = 2097152L, // 2 MB Default
    .is_target_configured = false,
    .auto_run_enabled = true
};

const char *iot_get_active_device_name(void) {
    return s_active_config.device_name;
}

bool iot_is_target_active(void) {
    return s_active_config.is_target_configured;
}

static bool check_and_run_autorun(VMContext *vm) {
    void *f = platform_file_open("MAIN.BAS", "r");
    if (f) {
        platform_file_close(f);
        BppError err = vm_load_program_file(vm, "MAIN.BAS");
        if (err.code == 0) {
            vm_execute_line(vm, "RUN");
            return true;
        }
    }
    f = platform_file_open("AUTORUN.BAS", "r");
    if (f) {
        platform_file_close(f);
        BppError err = vm_load_program_file(vm, "AUTORUN.BAS");
        if (err.code == 0) {
            vm_execute_line(vm, "RUN");
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv) {
    platform_init();
    esp32_hal_init();
    esp32_regs_init();
    iot_sensors_init();
    iot_net_init();

    bool enable_logging = false;
    double execution_timeout_ms = 0.0;
    const char *custom_log = NULL;
    const char *script_file = NULL;
    const char *cmd_expr = NULL;
    const char *serial_port = NULL;

    for (int i = 1; i < argc; i++) {
        if (runtime_strcmp(argv[i], "--esp32") == 0) {
            s_active_config.device_name = "ESP32";
            s_active_config.ram_size_bytes = 4194304L; // 4 MB Target
            s_active_config.is_target_configured = true;
        } else if (runtime_strcmp(argv[i], "--pi400") == 0) {
            s_active_config.device_name = "PI400";
            s_active_config.ram_size_bytes = 4194304L;
            s_active_config.is_target_configured = true;
        } else if (runtime_strcmp(argv[i], "--pico") == 0 || runtime_strcmp(argv[i], "--rp2040") == 0) {
            s_active_config.device_name = "PICO";
            s_active_config.ram_size_bytes = 4194304L;
            s_active_config.is_target_configured = true;
        } else if (runtime_strcmp(argv[i], "--esp8266") == 0) {
            s_active_config.device_name = "ESP8266";
            s_active_config.ram_size_bytes = 2097152L;
            s_active_config.is_target_configured = true;
        } else if (runtime_strncmp(argv[i], "--port=", 7) == 0) {
            serial_port = argv[i] + 7;
        } else if ((runtime_strcmp(argv[i], "--port") == 0 || runtime_strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
            serial_port = argv[++i];
        } else if (runtime_strcmp(argv[i], "--no-autorun") == 0) {
            s_active_config.auto_run_enabled = false;
        } else if (runtime_strcmp(argv[i], "--runtime_log") == 0) {
            enable_logging = true;
            custom_log = "iot.log";
        } else if (runtime_strncmp(argv[i], "--runtime_log=", 6) == 0) {
            enable_logging = true;
            custom_log = argv[i] + 6;
        } else if (runtime_strncmp(argv[i], "--timeout=", 10) == 0) {
            execution_timeout_ms = runtime_atof(argv[i] + 10);
        } else if (runtime_strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            execution_timeout_ms = runtime_atof(argv[++i]);
        } else if (runtime_strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cmd_expr = argv[++i];
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

    if (serial_port) {
        esp32_serial_open(serial_port, 115200);
        s_active_config.device_name = "ESP32-HW";
        s_active_config.is_target_configured = true;
    }

    if (enable_logging) {
        logger_init(custom_log ? custom_log : "iot.log", "iot.out");
    } else {
        logger_init(NULL, NULL);
    }

    VMContext *vm = boot_system(s_active_config.ram_size_bytes);
    if (!vm) {
        platform_console_eputs("FATAL: IoT Engine boot failed.\n");
        return 1;
    }

    platform_setup_signals(vm);

    if (execution_timeout_ms > 0.0) {
        vm_set_timeout(vm, execution_timeout_ms);
    }

    // Direct statement execution (-c)
    if (cmd_expr) {
        BppError err = vm_execute_line(vm, cmd_expr);
        if (err.code != 0 || vm_has_error(vm)) {
            if (err.code != 0) {
                platform_console_eprintf("Error %d: %s\n", err.code, err.message);
            } else {
                BppError vm_err = vm_get_error(vm);
                platform_console_eprintf("Error %d: %s\n", vm_err.code, vm_err.message);
                err.code = vm_err.code;
            }
        }
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    // Script file execution
    if (script_file) {
        BppError err = vm_load_program_file(vm, script_file);
        if (err.code != 0) {
            platform_console_eprintf("Error %d: %s\n", err.code, err.message);
            boot_shutdown_vm(vm);
            platform_shutdown();
            return err.code;
        }
        err = vm_execute_line(vm, "RUN");
        if (err.code != 0) {
            platform_console_eprintf("Error %d in line %lld: %s\n", err.code, (long long)vm_get_current_line(vm), err.message);
        }
        boot_shutdown_vm(vm);
        platform_shutdown();
        return err.code;
    }

    // Auto-run MAIN.BAS or AUTORUN.BAS if present
    if (s_active_config.auto_run_enabled) {
        if (check_and_run_autorun(vm)) {
            // Program executed
        }
    }

    // Interactive REPL Banner
    if (s_active_config.is_target_configured) {
        platform_console_printf("BASIC++ IoT Edition - %s v%s\n", s_active_config.device_name, BASIC_VERSION_STRING);
        platform_console_printf("%u MB RAM Available.\n\nReady.\n", s_active_config.ram_size_bytes / (1024 * 1024));
    } else {
        platform_console_printf("BASIC++ IoT Edition v%s\n", BASIC_VERSION_STRING);
        platform_console_printf("2 MB RAM Available.\n\nReady.\n");
    }

    char line_buf[1024];
    while (!vm_exit_requested(vm)) {
        if (vm_break_triggered(vm)) {
            vm_reset_break(vm);
            platform_console_puts("\n");
        }
        platform_console_puts(": ");
        platform_console_flush();

        if (!platform_console_gets(line_buf, sizeof(line_buf))) {
            break;
        }

        // Strip trailing newline
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
            line_buf[--len] = '\0';
        }

        char *trimmed = line_buf;
        while (*trimmed && runtime_isspace((unsigned char)*trimmed)) trimmed++;
        if (*trimmed == '\0') continue;

        // Check if line begins with a line number
        if (runtime_isdigit((unsigned char)*trimmed)) {
            BppLineNumber line_num = (BppLineNumber)runtime_atof(trimmed);
            while (runtime_isdigit((unsigned char)*trimmed) || *trimmed == '.') trimmed++;
            while (runtime_isspace((unsigned char)*trimmed)) trimmed++;

            if (*trimmed == '\0') {
                mem_program_remove(vm_get_mem(vm), line_num);
            } else {
                mem_program_store(vm_get_mem(vm), line_num, trimmed);
            }
        } else {
            BppError err = vm_execute_line(vm, trimmed);
            if (err.code != 0) {
                platform_console_printf("Error %d: %s\n", err.code, err.message);
            } else if (!vm_exit_requested(vm)) {
                platform_console_printf("Ready.\n");
            }
        }
    }

    boot_shutdown_vm(vm);
    platform_shutdown();
    return 0;
}

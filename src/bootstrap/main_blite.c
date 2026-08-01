/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file main_console.c
 * @brief Standard Console/REPL bootstrap entry point.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the main console interactive prompt (REPL) loop for BASIC++.
 *   Parses input line numbers for insertion/deletion, dispatches immediate execution lines,
 *   displays standard banners with RAM statistics, and handles error reporting.
 * - Why it exists: Serves as the primary console frontend executable (basicpp-console).
 * - Why it works this way: It triggers the 9-phase boot sequence. Immediate statements run
 *   directly via vm_execute_line, while numbered statements are sorted in program memory.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Welcome banner messages, custom command line parser logic.
 * - What cannot be changed: Obligation to route terminal printing through the Virtual Device CON: puts/printf APIs.
 * - What to expect: Entering a statement like "10 PRINT A" stores it. Typing "RUN" executes stored statements.
 * - What to do if something breaks: If input gets blocked or echoes incorrectly, check terminal properties
 *   in platform_init or VDev gets.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: CON: device is active. Standard keyboard input streams are non-blocking where expected.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add command history, line editing features, or auto-completions.
 * - How to write external extensions: Custom console interfaces wrap this boot loader framework.
 */

#include "bpp_boot.h"
#include "bpp_config.h"
#include "bpp_platform.h"
#include "bpp_task.h"
#include "bpp_vdev.h"
#include "bpp_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Helper: parse line number from string */
static bool parse_line_number(const char *str, BppLineNumber *out_line, const char **out_text) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    if (!isdigit((unsigned char)*str)) {
        return false;
    }

    /* Parse integer major part */
    double major = 0.0;
    const char *p = str;
    while (isdigit((unsigned char)*p)) {
        major = major * 10.0 + (*p - '0');
        p++;
    }

    double fractional = 0.0;
    double multiplier = 0.01;

    /* Parse subsequent fractional parts separated by dots, e.g. .01.02 */
    while (*p == '.') {
        /* Peek if the character after the dot is a digit */
        if (!isdigit((unsigned char)*(p + 1))) {
            break; /* Trailing dot or start of a directive, stop parsing */
        }
        p++; /* skip '.' */

        int digits_val = 0;
        int digit_count = 0;
        while (isdigit((unsigned char)*p) && digit_count < 2) {
            digits_val = digits_val * 10 + (*p - '0');
            p++;
            digit_count++;
        }
        while (isdigit((unsigned char)*p)) {
            p++;
        }
        if (digit_count == 1) {
            digits_val *= 10; /* Pad single digit to 2 digits, e.g. .5 -> .50 */
        }
        fractional += digits_val * multiplier;
        multiplier *= 0.01;
    }

    *out_line = major + fractional;
    *out_text = p;
    return true;
}

int main(int argc, char **argv) {
    const char *cmd = NULL;
    const char *script_file = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            cmd = argv[++i];
        } else if (argv[i][0] != '-') {
            script_file = argv[i];
        }
    }

    BootConfig config;
    memset(&config, 0, sizeof(config));
    config.is_repl = true;
    config.prog_mem = 67108864L;
    config.var_mem  = 67108864L;
    config.str_mem  = 201326592L;
    config.scratch_mem = 67108864L;

    BootContext *boot = boot_execute(&config);
    if (!boot) {
        fprintf(stderr, "FATAL: BASIC++ blite failed to boot phases.\n");
        return 1;
    }

    VDevContext *vdev = vm_get_vdev(boot->vm);
    platform_setup_signals(boot->vm);

    if (cmd) {
        BppError err = vm_execute_line(boot->vm, cmd);
        if (err.code != 0) {
            vdev_printf(vdev, "?Error %d: %s in line %g\n", err.code, err.message, err.line);
            boot_shutdown(boot);
            return 1;
        }
        boot_shutdown(boot);
        return 0;
    }

    if (script_file) {
        BppError err = vm_load_program_file(boot->vm, script_file);
        if (err.code != 0) {
            vdev_printf(vdev, "?Error %d: %s during LOAD in line %g\n", err.code, err.message, err.line);
            boot_shutdown(boot);
            return 1;
        }
        vm_run_program(boot->vm);
        if (vm_has_error(boot->vm)) {
            BppError run_err = vm_get_error(boot->vm);
            vdev_printf(vdev, "?Error %d: %s in line %g\n", run_err.code, run_err.message, run_err.line);
            boot_shutdown(boot);
            return 1;
        }
        boot_shutdown(boot);
        return 0;
    }

    /* Print welcome banner with RAM stats */
    vdev_printf(vdev, "BASIC++ v%s \"%s\" [%s]\n", BPP_VERSION_STRING, BPP_VERSION_CODENAME, BPP_NAME);
    vdev_printf(vdev, "Platform: %s (Profile: %s)\n", platform_name(), BPP_PROFILE_NAME);
    
    size_t free_ram = mem_get_free_ram(boot->mem);
    char mem_buf[64];
    mem_format_size(free_ram, mem_buf, sizeof(mem_buf));
    vdev_printf(vdev, "%s free\n\n", mem_buf);
    vdev_puts(vdev, BPP_READY "\n");
    VDev *con_dev = vdev_get(vdev, "CON:");
    if (con_dev && con_dev->ops.flush) {
        con_dev->ops.flush(con_dev);
    }

    char input_line[1024];
    VDev *con = vdev_get(vdev, "CON:");

    while (true) {
        vdev_puts(vdev, BPP_PROMPT);
        if (con && con->ops.flush) {
            con->ops.flush(con);
        }

        if (!con->ops.gets || !con->ops.gets(con, input_line, sizeof(input_line))) {
            if (vm_break_triggered(boot->vm)) {
                vdev_puts(vdev, "\n" BPP_READY "\n");
                vm_reset_break(boot->vm);
                continue;
            }
            break; /* EOF */
        }
        if (input_line[0] == 4) break; /* Ctrl+D to exit */

        /* Strip trailing newline and carriage return */
        size_t len = strlen(input_line);
        while (len > 0 && (input_line[len - 1] == '\n' || input_line[len - 1] == '\r')) {
            input_line[--len] = '\0';
        }

        /* Skip leading whitespace for check empty/command */
        char *ptr = input_line;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }

        if (*ptr == '\0') {
            continue; /* Empty line */
        }

        /* Parse line number */
        BppLineNumber line = 0.0;
        const char *stmt_text = NULL;
        bool has_line_num = parse_line_number(ptr, &line, &stmt_text);

        if (has_line_num) {
            /* Skip spaces after line number */
            while (*stmt_text && isspace((unsigned char)*stmt_text)) {
                stmt_text++;
            }

            if (*stmt_text == '\0') {
                /* Empty line text: delete line */
                mem_program_delete(boot->mem, line);
            } else {
                /* Store line */
                if (!mem_program_insert(boot->mem, line, stmt_text)) {
                    vdev_puts(vdev, "?Out of memory storing line\n");
                }
            }
        } else {
            /* Execute statement immediately */
            BppError err = vm_execute_line(boot->vm, ptr);
            if (err.code != 0) {
                if (err.line != 0.0) {
                    vdev_printf(vdev, "?%s in line %g\n", err.message, err.line);
                } else {
                    vdev_printf(vdev, "?%s\n", err.message);
                }
            }

            if (vm_is_running(boot->vm)) {
                vm_run_program(boot->vm);
            }

            /* Check if SYSTEM or EXIT was invoked */
            if (vm_exit_requested(boot->vm)) {
                break;
            }
        }

        /* Reclaim ephemeral scratch memory */
        mem_scratch_reset(boot->mem);
    }

    bool force = !vm_exit_requested(boot->vm);
    if (!force) {
        if (task_mgr_has_active_tasks()) {
            vdev_puts(vdev, "Waiting for background tasks to complete...\n");
        }
    }
    boot_shutdown_ex(boot, force);
    return 0;
}

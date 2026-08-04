/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file main_lite.c
 * @brief Headless/Lite Edition entry point.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the minimal, headless entry point (blite) for executing BASIC scripts
 *   directly from file paths passed as command-line arguments.
 * - Why it exists: Serves as the minimal footprint runtime executable for embedded systems
 *   or lightweight server scripting.
 * - Why it works this way: It loads lines from the target file, parses line numbers, inserts them
 *   sequentially into the program store, executes vm_run_program, and exits immediately.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: File read buffer size configurations, CLI flag defaults.
 * - What cannot be changed: Obligation to run headless (no graphics or graphical console edits).
 * - What to expect: Executing a script from file prints only its outputs and terminates on halt/error.
 * - What to do if something breaks: If file reads fail, check the file path string format and file permissions.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: File systems support standard fopen/fgets/fclose calls.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add command arguments (COMMAND$) or redirect stdout to logs.
 * - How to write external extensions: Plugins link against the headless core using standard static targets.
 */

#include "bpp_boot.h"
#include "bpp_config.h"
#include "bpp_platform.h"
#include "bpp_metadata.h"
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
    if (argc < 2) {
        printf("BASIC++ Script Runner (bscript) v%s \"%s\" (Profile: %s)\n", BPP_VERSION_STRING, BPP_VERSION_CODENAME, BPP_PROFILE_NAME);
        printf("Usage: bscript <filename.bas>\n");
        return 1;
    }

    const char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return 1;
    }

    BootConfig config;
    memset(&config, 0, sizeof(config));
    config.is_repl = false;
    config.prog_mem = 16777216L;
    config.var_mem  = 16777216L;
    config.str_mem  = 16777216L;
    config.scratch_mem = 16777216L;

    BootContext *boot = boot_execute(&config);
    if (!boot) {
        fclose(fp);
        fprintf(stderr, "Error: Subsystems failed to boot.\n");
        return 1;
    }

    platform_setup_signals(boot->vm);

    char line_buf[1024];
    BppLineNumber last_line_num = 0.0;

    /* Load lines from file */
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        /* Strip trailing newlines/spaces */
        size_t len = strlen(line_buf);
        while (len > 0 && isspace((unsigned char)line_buf[len - 1])) {
            line_buf[len - 1] = '\0';
            len--;
        }

        char *ptr = line_buf;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }

        if (*ptr == '\0') {
            continue; /* Skip empty line */
        }

        BppLineNumber line_num = 0.0;
        const char *stmt_text = NULL;
        bool has_line_num = parse_line_number(ptr, &line_num, &stmt_text);

        if (has_line_num) {
            while (*stmt_text && isspace((unsigned char)*stmt_text)) {
                stmt_text++;
            }
            if (*stmt_text != '\0') {
                mem_program_insert(boot->mem, line_num, stmt_text);
                last_line_num = line_num;
            }
        } else {
            last_line_num += 0.0001;
            mem_program_insert(boot->mem, last_line_num, ptr);
        }
    }
    fclose(fp);

    /* Pre-scan program metadata */
    metadata_pre_scan_program(boot->vm, filename);

    /* Run program */
    vm_run_program(boot->vm);

    /* Check error status */
    if (vm_has_error(boot->vm)) {
        BppError err = vm_get_error(boot->vm);
        VDevContext *vdev = vm_get_vdev(boot->vm);
        vdev_printf(vdev, "?Error %d: %s in line %lld\n", err.code, err.message, (long long)err.line);
        boot_shutdown(boot);
        return 1;
    }

    boot_shutdown(boot);
    return 0;
}

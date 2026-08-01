/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file editor.c
 * @brief Built-in interactive console text editor.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements a simple interactive line-editor command (EDIT).
 * - Why it exists: Allows users to add, edit, list, and delete program lines without
 *   leaving the interpreter or relying on host system editors.
 * - Why it works this way: It prints current lines, prompts for a line number, displays
 *   existing text if present, and updates program storage. Operates using standard console I/O
 *   for complete cross-platform portability.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Custom command strings, prompt descriptions, listing format.
 * - What cannot be changed: Program storage insert/update mechanics.
 * - What to expect: Exiting the editor returns back to the interactive REPL immediately.
 * - What to do if something breaks: Verify stdin reads and check line number bounds checks.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Keyboard input and text output streams are redirected through the registered VDevs.
 * - Portability concerns: None. C17 standard compliant.
 */

#ifndef BPP_LITE_BUILD

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_vm.h"
#include "bpp_vdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Helper to trim whitespace from a string.
 */
static char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

/**
 * @brief Interactive EDIT command handler.
 */
BppError stmt_edit_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    vdev_printf(vdev, "\n--- BASIC++ Built-in Line Editor ---\n");
    vdev_printf(vdev, "Commands: L = List, DELETE <num> = Delete line, 0 or EXIT = Exit REPL\n\n");

    /* Main edit loop */
    char input_buf[1024];
    while (true) {
        vdev_printf(vdev, "Edit: ");
        VDev *con = vdev_get(vdev, "CON:");
        if (!con || !con->ops.gets) {
            err.code = 5; err.message = "Console input device unavailable";
            return err;
        }

        if (!con->ops.gets(con, input_buf, sizeof(input_buf))) {
            break; /* EOF */
        }

        char *trimmed = trim_whitespace(input_buf);
        if (trimmed[0] == '\0') continue;

        /* Check for Exit */
        char upper[128];
        size_t len = strlen(trimmed);
        if (len >= sizeof(upper)) len = sizeof(upper) - 1;
        for (size_t i = 0; i < len; ++i) upper[i] = (char)toupper((unsigned char)trimmed[i]);
        upper[len] = '\0';

        if (strcmp(upper, "0") == 0 || strcmp(upper, "EXIT") == 0 || strcmp(upper, "QUIT") == 0) {
            break;
        }

        /* Check for List */
        if (strcmp(upper, "L") == 0 || strcmp(upper, "LIST") == 0) {
            size_t count = 0;
            BppProgramLine *lines = mem_program_get_all(mem, &count);
            vdev_printf(vdev, "\n--- Current Program ---\n");
            for (size_t i = 0; i < count; ++i) {
                vdev_printf(vdev, "%g %s\n", lines[i].line_number, lines[i].text);
            }
            vdev_printf(vdev, "-----------------------\n\n");
            continue;
        }

        /* Check for Delete */
        if (strncmp(upper, "DELETE ", 7) == 0) {
            double ln = atof(trimmed + 7);
            if (ln > 0.0) {
                mem_program_delete(mem, ln);
                vdev_printf(vdev, "Line %g deleted.\n", ln);
            } else {
                vdev_printf(vdev, "Invalid line number.\n");
            }
            continue;
        }

        /* If numeric, edit line */
        if (isdigit((unsigned char)trimmed[0])) {
            double ln = atof(trimmed);
            if (ln <= 0.0) {
                vdev_printf(vdev, "Line numbers must be positive.\n");
                continue;
            }

            /* Find existing line text if any */
            size_t count = 0;
            BppProgramLine *lines = mem_program_get_all(mem, &count);
            const char *existing = "";
            for (size_t i = 0; i < count; ++i) {
                if (lines[i].line_number == ln) {
                    existing = lines[i].text;
                    break;
                }
            }

            if (existing[0] != '\0') {
                vdev_printf(vdev, "Line %g: %s\n", ln, existing);
            }
            vdev_printf(vdev, "New text: ");
            if (!con->ops.gets(con, input_buf, sizeof(input_buf))) {
                break;
            }

            char *new_text = trim_whitespace(input_buf);
            if (new_text[0] == '\0') {
                vdev_printf(vdev, "Preserved.\n");
            } else {
                mem_program_insert(mem, ln, new_text);
                vdev_printf(vdev, "Line %g updated.\n", ln);
            }
        } else {
            vdev_printf(vdev, "Unknown editor command. Use L to list, 0 to exit, or enter a line number.\n");
        }
    }

    vdev_printf(vdev, "Exiting editor. Ready.\n\n");
    return err;
}

#endif /* BPP_LITE_BUILD */


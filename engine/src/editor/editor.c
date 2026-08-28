// FILENAME: editor.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libengine (tui_multiplexer.c)
// NEEDED BY: libstandard (edit_internal.h, editor_manager.c, edlin_internal.h)
// NEEDED BY: libstandard (vi_internal.h, ws_internal.h)
// NEEDS: libcore (ctype.h, ctype.c, float_parse.h, float_parse.c)
// NEEDS: libcore (memops.h, memops.c, num_parse.h, num_parse.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, stmt.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Implements visual text editor subsystem components for editor.
//
// ---- Includes ----

#ifndef BASIC_LITE_BUILD

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"

// @brief Helper to trim whitespace from a string.
static char *trim_whitespace(char *str) {
    if (!str) return "";
    char *end;
    while (runtime_isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + runtime_strlen(str) - 1;
    while (end > str && runtime_isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// @brief Interactive EDIT command handler.
BppError stmt_edit_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    vdev_printf(vdev, "\n--- BASIC++ Built-in Line Editor ---\n");
    vdev_printf(vdev, "Commands: L = List, DELETE <num> = Delete line, 0 or EXIT = Exit REPL\n\n");

    // Main edit loop
    char input_buf[1024];
    while (true) {
        vdev_printf(vdev, "Edit: ");
        VDev *con = vdev_get(vdev, "CON:");
        if (!con || !con->ops.gets) {
            err.code = 5; err.message = "Console input device unavailable";
            return err;
        }

        if (!con->ops.gets(con, input_buf, sizeof(input_buf))) {
            break; // EOF
        }

        char *trimmed = trim_whitespace(input_buf);
        if (trimmed[0] == '\0') continue;

        // Check for Exit
        char upper[128];
        size_t len = runtime_strlen(trimmed);
        if (len >= sizeof(upper)) len = sizeof(upper) - 1;
        for (size_t i = 0; i < len; ++i) upper[i] = (char)runtime_toupper((unsigned char)trimmed[i]);
        upper[len] = '\0';

        if (runtime_strcmp(upper, "0") == 0 || runtime_strcmp(upper, "EXIT") == 0 || runtime_strcmp(upper, "QUIT") == 0) {
            break;
        }

        // Check for List
        if (runtime_strcmp(upper, "L") == 0 || runtime_strcmp(upper, "LIST") == 0) {
            size_t count = 0;
            BppProgramLine *lines = mem_program_get_all(mem, &count);
            vdev_printf(vdev, "\n--- Current Program ---\n");
            for (size_t i = 0; i < count; ++i) {
                vdev_printf(vdev, "%lld %s\n", (long long)lines[i].line_number, lines[i].text);
            }
            vdev_printf(vdev, "-----------------------\n\n");
            continue;
        }

        // Check for Delete
        if (runtime_strncmp(upper, "DELETE ", 7) == 0) {
            double ln = runtime_strtod(trimmed + 7, NULL);
            if (ln > 0.0) {
                mem_program_delete(mem, ln);
                vdev_printf(vdev, "Line %lld deleted.\n", (long long)ln);
            } else {
                vdev_printf(vdev, "Invalid line number.\n");
            }
            continue;
        }

        // If numeric, edit line
        if (runtime_isdigit((unsigned char)trimmed[0])) {
            double ln = runtime_strtod(trimmed, NULL);
            if (ln <= 0.0) {
                vdev_printf(vdev, "Line numbers must be positive.\n");
                continue;
            }

            // Find existing line text if any
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
                vdev_printf(vdev, "Line %lld: %s\n", (long long)ln, existing);
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
                vdev_printf(vdev, "Line %lld updated.\n", (long long)ln);
            }
        } else {
            vdev_printf(vdev, "Unknown editor command. Use L to list, 0 to exit, or enter a line number.\n");
        }
    }

    vdev_printf(vdev, "Exiting editor. Ready.\n\n");
    return err;
}

#endif // BASIC_LITE_BUILD

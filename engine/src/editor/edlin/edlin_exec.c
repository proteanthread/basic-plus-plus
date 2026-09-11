// FILENAME: edlin_exec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edlin_internal.h)
// Implements bytecode virtual machine execution and state for edlin_exec.
//
// ---- Includes ----

#include "editor/edlin_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/num_parse.h"

//
// ---- Help & Program Runner ----

void display_edlin_help(void) {
    int page_size = edlin_get_page_size();
    edlin_print("\nedlin - Built-in Line Editor\n");
    edlin_print("Available Commands:\n");
    edlin_print("  [line] - Edit a specific line (enter line number)\n");
    edlin_print("  a      - Append lines from disk into memory\n");
    edlin_print("  c      - Copy lines\n");
    edlin_print("  d      - Delete lines\n");
    edlin_print("  e      - End editing and save file\n");
    edlin_print("  h      - Display this help message\n");
    edlin_print("  i      - Insert lines at the end of the buffer\n");
    edlin_print("  l      - List all lines currently in the buffer\n");
    edlin_print("  m      - Move lines\n");
    edlin_print("  p      - Page display (%d lines at a time)\n", page_size);
    edlin_print("  q      - Quit without saving\n");
    edlin_print("  r      - Replace text\n");
    edlin_print("  run    - Run program\n");
    edlin_print("  s      - Search text\n");
    edlin_print("  t      - Transfer (merge) another file\n");
    edlin_print("  w      - Write lines to disk\n");
    edlin_print("  x      - Execute program\n\n");
}

#ifndef STANDALONE_EDITOR
static void edlin_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    if (runtime_strcmp(event_type, "step") == 0) {
        if (g_edlin_trace_mode) {
            edlin_print("[TRACE] Executing Line %d\n", line_num);
        } else if (g_edlin_debug_step) {
            edlin_print("[DEBUG] Line %d. Press Space to step, Enter to continue...\n", line_num);
            int key;
            do {
                key = platform_console_getchar();
            } while (key != ' ' && key != '\n' && key != '\r');
            if (key == '\n' || key == '\r') {
                g_edlin_debug_step = false;
            }
        }
    }
}
#endif

void execute_program_edlin(int exec_mode) {
    if (!g_edlin_current_vm) return;
    HalContext *hal = hal_get();
    if (!hal) return;

    const char *target = g_edlin_filename[0] ? g_edlin_filename : "untitled.bas";
    IoHandle h = hal->io.file_open(target, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_edlin_line_count; i++) {
            if (g_edlin_buffer[i].text) {
                hal->io.file_write(h, g_edlin_buffer[i].text, 1, (size_t)g_edlin_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
    
    edlin_print("\x1b[2J\x1b[H");
    
    g_edlin_debug_step = (exec_mode == 1);
    g_edlin_trace_mode = (exec_mode == 2);
    if (exec_mode > 0) {
        vm_set_debug_hook(g_edlin_current_vm, edlin_debug_hook, NULL);
    } else {
        vm_set_debug_hook(g_edlin_current_vm, NULL, NULL);
    }
    
    vm_load_program_file(g_edlin_current_vm, target);
    vm_run_program(g_edlin_current_vm);
    edlin_print("\n[Press Enter to return to editor...]\n");
    platform_console_getchar();
}

//
// ---- Editor Main Loop ----

int mod_edlin_main(VMContext *vm, const char *filename) {
    g_edlin_current_vm = vm;

    platform_init();

    edlin_print("%s", g_edlin_bright_colors[g_edlin_color_index]);

    char command[4096];
    g_edlin_line_count = 0;
    g_edlin_page_pos   = 0;
    runtime_memset(g_edlin_filename, 0, sizeof(g_edlin_filename));

    if (filename && filename[0] != '\0') {
        load_edlin_file(filename);
    }

    edlin_print("Type '?' or 'h' for a list of available commands.\n");

    for (;;) {
        edlin_print("*");
        if (edlin_read_line(command, sizeof(command)) == NULL) break;
        size_t cmd_len = runtime_strlen(command);
        while (cmd_len > 0 && (command[cmd_len - 1] == '\n' || command[cmd_len - 1] == '\r')) {
            command[cmd_len - 1] = '\0';
            cmd_len--;
        }
        if (command[0] == '\0') continue;

        if (runtime_isdigit((unsigned char)command[0])) {
            int64_t line_val = runtime_atoll(command);
            edit_edlin_line((int)line_val - 1);
            continue;
        }
        if (runtime_strcmp(command, "run") == 0 || runtime_strcmp(command, "r!") == 0) {
            execute_program_edlin(0);
            continue;
        }
        if (runtime_strcmp(command, "/debug") == 0) {
            execute_program_edlin(1);
            continue;
        }
        if (runtime_strcmp(command, "/trace") == 0) {
            execute_program_edlin(2);
            continue;
        }
        if (runtime_strcmp(command, "bye") == 0) {
            goto edlin_exit;
        }

        switch (runtime_tolower((unsigned char)command[0])) {
        case 'a': append_edlin_lines(); break;
        case 'c': copy_edlin_lines(); break;
        case 'd': delete_edlin_line(); break;
        case 'e': save_edlin_file(); goto edlin_exit;
        case 'h':
        case '?': display_edlin_help(); break;
        case 'i': insert_edlin_line(); break;
        case 'l': list_edlin_lines(); break;
        case 'm': move_edlin_lines(); break;
        case 'o':
            g_edlin_color_index = (g_edlin_color_index + 1) % (int)g_edlin_num_bright_colors;
            edlin_print("%s\x1b[2J\x1b[H[Color Changed]\r\n", g_edlin_bright_colors[g_edlin_color_index]);
            break;
        case 'p': page_edlin_display(); break;
        case 'q': goto edlin_exit;
        case 'r': replace_edlin_text(); break;
        case 's': search_edlin_text(); break;
        case 't': transfer_edlin_file(); break;
        case 'w': write_edlin_lines(); break;
        case 'x': execute_program_edlin(0); break;
        default:
            edlin_print("Entry error (type '?' for help)\n");
            break;
        }
    }

edlin_exit:
    edlin_print("\x1b[0m\x1b[2J\x1b[H");
    return 0;
}

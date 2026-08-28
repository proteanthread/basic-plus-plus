// FILENAME: edit_menu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit_menu.
//
// ---- Includes ----

#include "editor/edit_internal.h"

#define EDIT_NUM_BRIGHT_COLORS 6

void edit_execute_menu(VMContext *vm) {
    char buf[4096];
    g_edit_menu_mode = 0;
    HalContext *hal = hal_get();
    
    if (g_edit_menu_col == 0) { // File Menu
        if (g_edit_menu_row == 0) { // New
            if (g_edit_text_buffer) {
                for (int i = 0; i < g_edit_num_lines; i++) edit_free_line(i);
            }
            g_edit_num_lines = 0;
            edit_insert_empty_line(0);
            g_edit_cy = 0;
            g_edit_cx = 0;
            g_edit_current_filename[0] = '\0';
            g_edit_row_off = 0;
            g_edit_col_off = 0;
        } else if (g_edit_menu_row == 1) { // Open
            buf[0] = '\0';
            if (edit_prompt_input(" Open ", buf) && buf[0] != '\0') {
                edit_load_file(buf, false);
            }
        } else if (g_edit_menu_row == 2) { // Save
            if (g_edit_current_filename[0] == '\0') {
                buf[0] = '\0';
                if (edit_prompt_input(" Save As ", buf) && buf[0] != '\0') {
                    runtime_strncpy(g_edit_current_filename, buf, sizeof(g_edit_current_filename) - 1);
                    g_edit_current_filename[sizeof(g_edit_current_filename) - 1] = '\0';
                    edit_save_file();
                }
            } else {
                edit_save_file();
            }
        } else if (g_edit_menu_row == 3) { // Save As
            runtime_strncpy(buf, g_edit_current_filename, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if (edit_prompt_input(" Save As ", buf) && buf[0] != '\0') {
                runtime_strncpy(g_edit_current_filename, buf, sizeof(g_edit_current_filename) - 1);
                g_edit_current_filename[sizeof(g_edit_current_filename) - 1] = '\0';
                edit_save_file();
            }
        } else if (g_edit_menu_row == 4) { // Exit
            g_edit_exit_editor = true;
        }
    } else if (g_edit_menu_col == 1) { // Edit Menu
        if (g_edit_menu_row == 0) { // Cut
            char *sel = edit_get_selected_text();
            if (sel) {
                runtime_strncpy(g_edit_clipboard_line, sel, sizeof(g_edit_clipboard_line) - 1);
                g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
                if (hal && hal->mem.free) hal->mem.free(sel);
                edit_delete_selection();
            } else {
                runtime_strncpy(g_edit_clipboard_line, g_edit_text_buffer[g_edit_cy].text, sizeof(g_edit_clipboard_line) - 1);
                g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
                edit_delete_current_line();
            }
        } else if (g_edit_menu_row == 1) { // Copy
            char *sel = edit_get_selected_text();
            if (sel) {
                runtime_strncpy(g_edit_clipboard_line, sel, sizeof(g_edit_clipboard_line) - 1);
                g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
                if (hal && hal->mem.free) hal->mem.free(sel);
            } else {
                runtime_strncpy(g_edit_clipboard_line, g_edit_text_buffer[g_edit_cy].text, sizeof(g_edit_clipboard_line) - 1);
                g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
            }
        } else if (g_edit_menu_row == 2) { // Paste
            edit_insert_text_at_cursor(g_edit_clipboard_line);
        } else if (g_edit_menu_row == 3) { // Clear
            if (g_edit_sel_active) edit_delete_selection();
            else edit_delete_current_line();
        }
    } else if (g_edit_menu_col == 2) { // Search Menu
        if (g_edit_menu_row == 0) { // Find
            buf[0] = '\0';
            if (edit_prompt_input(" Find ", buf) && buf[0] != '\0') {
                runtime_strncpy(g_edit_search_term, buf, sizeof(g_edit_search_term) - 1);
                g_edit_search_term[sizeof(g_edit_search_term) - 1] = '\0';
                edit_do_find();
            }
        } else if (g_edit_menu_row == 1) { // Find Next
            if (g_edit_search_term[0] != '\0') edit_do_find();
        }
    } else if (g_edit_menu_col == 3) { // Display Menu
        if (g_edit_menu_row == 0) { // Color mode
            g_edit_color_index = (g_edit_color_index + 1) % EDIT_NUM_BRIGHT_COLORS;
        } else if (g_edit_menu_row == 1) { // Line gutter
            g_edit_display_gutter = !g_edit_display_gutter;
        }
    } else if (g_edit_menu_col == 4) { // Run Menu
        if (g_edit_menu_row == 0) edit_execute_program(vm, 0);
        else if (g_edit_menu_row == 1) edit_execute_program(vm, 1);
        else if (g_edit_menu_row == 2) g_edit_text_buffer[g_edit_cy].breakpoint = !g_edit_text_buffer[g_edit_cy].breakpoint;
    } else if (g_edit_menu_col == 5) { // Help Menu
        if (g_edit_menu_row == 0) {
            char about_msg[128];
            runtime_snprintf(about_msg, sizeof(about_msg), "BASIC++ Standard %s", BASIC_VERSION_STRING);
            edit_show_message(" About ", about_msg);
        }
    }
}

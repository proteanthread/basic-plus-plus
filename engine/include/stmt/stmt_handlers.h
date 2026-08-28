// FILENAME: stmt_handlers.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the HANDLERS statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_HANDLERS_H
#define STMT_HANDLERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer/lexer.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Core Control Flow & Execution Handlers ----

BppError stmt_goto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_return_handler(VMContext *vm, LexerContext *lex);
BppError stmt_if_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rem_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_on_handler(VMContext *vm, LexerContext *lex);
BppError stmt_error_handler(VMContext *vm, LexerContext *lex);
BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resume_handler(VMContext *vm, LexerContext *lex);
BppError stmt_select_handler(VMContext *vm, LexerContext *lex);
BppError stmt_case_handler(VMContext *vm, LexerContext *lex);
BppError stmt_when_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unless_handler(VMContext *vm, LexerContext *lex);
BppError stmt_whenever_handler(VMContext *vm, LexerContext *lex);
BppError stmt_enter_handler(VMContext *vm, LexerContext *lex);
BppError stmt_perform_handler(VMContext *vm, LexerContext *lex);
BppError stmt_invoke_handler(VMContext *vm, LexerContext *lex);
BppError stmt_prefix_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_retry_handler(VMContext *vm, LexerContext *lex);
BppError stmt_continue_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ask_handler(VMContext *vm, LexerContext *lex);
BppError stmt_use_handler(VMContext *vm, LexerContext *lex);
BppError stmt_handler_handler(VMContext *vm, LexerContext *lex);
BppError stmt_try_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_throw_handler(VMContext *vm, LexerContext *lex);
BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_exit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex);
BppError stmt_goodbye_handler(VMContext *vm, LexerContext *lex);
BppError stmt_system_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shell_handler(VMContext *vm, LexerContext *lex);
BppError stmt_environ_handler(VMContext *vm, LexerContext *lex);

//
// ---- Loops & Iteration Handlers ----

BppError stmt_for_handler(VMContext *vm, LexerContext *lex);
BppError stmt_next_handler(VMContext *vm, LexerContext *lex);
BppError stmt_while_handler(VMContext *vm, LexerContext *lex);
BppError stmt_wend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_do_handler(VMContext *vm, LexerContext *lex);
BppError stmt_loop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_exit_loop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_repeat_handler(VMContext *vm, LexerContext *lex);
BppError stmt_until_handler(VMContext *vm, LexerContext *lex);
BppError stmt_endloop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_exitif_handler(VMContext *vm, LexerContext *lex);

//
// ---- Variables, Assignment & Memory Handlers ----

BppError stmt_let_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dim_handler(VMContext *vm, LexerContext *lex);
BppError stmt_redim_handler(VMContext *vm, LexerContext *lex);
BppError stmt_erase_handler(VMContext *vm, LexerContext *lex);
BppError stmt_swap_handler(VMContext *vm, LexerContext *lex);
BppError stmt_exchange_handler(VMContext *vm, LexerContext *lex);
BppError stmt_incr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_decr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_clr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_void_handler(VMContext *vm, LexerContext *lex);
BppError stmt_def_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_deflng_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defcpx_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fnend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_option_handler(VMContext *vm, LexerContext *lex);
BppError stmt_common_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shared_handler(VMContext *vm, LexerContext *lex);
BppError stmt_share_handler(VMContext *vm, LexerContext *lex);
BppError stmt_static_handler(VMContext *vm, LexerContext *lex);
BppError stmt_local_handler(VMContext *vm, LexerContext *lex);
BppError stmt_global_handler(VMContext *vm, LexerContext *lex);
BppError stmt_public_handler(VMContext *vm, LexerContext *lex);
BppError stmt_private_handler(VMContext *vm, LexerContext *lex);
BppError stmt_external_handler(VMContext *vm, LexerContext *lex);
BppError stmt_const_handler(VMContext *vm, LexerContext *lex);
BppError stmt_declare_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mid_stmt_handler(VMContext *vm, LexerContext *lex);
BppError stmt_data_handler(VMContext *vm, LexerContext *lex);
BppError stmt_read_handler(VMContext *vm, LexerContext *lex);
BppError stmt_restore_handler(VMContext *vm, LexerContext *lex);
BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex);
BppError stmt_change_handler(VMContext *vm, LexerContext *lex);
BppError stmt_complex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_arrayfill_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_demux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unpack_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_param_handler(VMContext *vm, LexerContext *lex);
BppError stmt_create_handler(VMContext *vm, LexerContext *lex);
BppError stmt_destroy_handler(VMContext *vm, LexerContext *lex);

//
// ---- Input / Output Handlers ----

BppError stmt_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_display_handler(VMContext *vm, LexerContext *lex);
BppError stmt_input_handler(VMContext *vm, LexerContext *lex);
BppError stmt_linput_handler(VMContext *vm, LexerContext *lex);
BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lprint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_llist_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cls_handler(VMContext *vm, LexerContext *lex);
BppError stmt_home_handler(VMContext *vm, LexerContext *lex);
BppError stmt_locate_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_color_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_width_handler(VMContext *vm, LexerContext *lex);
BppError stmt_margin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nomargin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_zone_handler(VMContext *vm, LexerContext *lex);
BppError stmt_echo_handler(VMContext *vm, LexerContext *lex);
BppError stmt_noecho_handler(VMContext *vm, LexerContext *lex);
BppError stmt_page_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nopage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_text_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tab_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pos_handler(VMContext *vm, LexerContext *lex);
BppError stmt_spc_handler(VMContext *vm, LexerContext *lex);
BppError stmt_poke_handler(VMContext *vm, LexerContext *lex);
BppError stmt_out_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);
BppError stmt_key_handler(VMContext *vm, LexerContext *lex);
BppError stmt_timer_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex);
BppError stmt_set_handler(VMContext *vm, LexerContext *lex);

//
// ---- Filesystem & Device Stream Handlers ----

BppError stmt_open_handler(VMContext *vm, LexerContext *lex);
BppError stmt_close_handler(VMContext *vm, LexerContext *lex);
BppError stmt_reset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_put_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bput_handler(VMContext *vm, LexerContext *lex);
BppError stmt_field_handler(VMContext *vm, LexerContext *lex);
BppError stmt_files_handler(VMContext *vm, LexerContext *lex);
BppError stmt_kill_handler(VMContext *vm, LexerContext *lex);
BppError stmt_scratch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mkdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_name_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seek_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_write_file_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vdim_handler(VMContext *vm, LexerContext *lex);
BppError stmt_devices_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pwd_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex);
BppError stmt_username_handler(VMContext *vm, LexerContext *lex);
BppError stmt_path_handler(VMContext *vm, LexerContext *lex);
BppError stmt_append_handler(VMContext *vm, LexerContext *lex);
BppError stmt_modify_handler(VMContext *vm, LexerContext *lex);
BppError stmt_image_handler(VMContext *vm, LexerContext *lex);
BppError stmt_form_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rewind_handler(VMContext *vm, LexerContext *lex);
BppError stmt_backspace_handler(VMContext *vm, LexerContext *lex);
BppError stmt_assign_handler(VMContext *vm, LexerContext *lex);
BppError stmt_advance_handler(VMContext *vm, LexerContext *lex);
BppError stmt_find_handler(VMContext *vm, LexerContext *lex);
BppError stmt_map_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mapend_handler(VMContext *vm, LexerContext *lex);

//
// ---- Graphics & Multimedia Handlers ----

BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_line_handler(VMContext *vm, LexerContext *lex);
BppError stmt_circle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_preset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_draw_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_gfx_handler(VMContext *vm, LexerContext *lex);
BppError stmt_put_gfx_handler(VMContext *vm, LexerContext *lex);
BppError stmt_view_handler(VMContext *vm, LexerContext *lex);
BppError stmt_window_handler(VMContext *vm, LexerContext *lex);
BppError stmt_picture_handler(VMContext *vm, LexerContext *lex);
BppError stmt_palette_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pcopy_handler(VMContext *vm, LexerContext *lex);
BppError stmt_plot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_viewport_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sound_handler(VMContext *vm, LexerContext *lex);
BppError stmt_play_handler(VMContext *vm, LexerContext *lex);
BppError stmt_beep_handler(VMContext *vm, LexerContext *lex);
BppError stmt_voice_handler(VMContext *vm, LexerContext *lex);
BppError stmt_noise_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hmouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vmouse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_trig_handler(VMContext *vm, LexerContext *lex);
BppError stmt_title_handler(VMContext *vm, LexerContext *lex);
BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resize_handler(VMContext *vm, LexerContext *lex);
BppError stmt_icon_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex);
BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mode_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_border_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ink_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paper_handler(VMContext *vm, LexerContext *lex);
BppError stmt_initgraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_closegraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putpixel_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ellipse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rectangle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_outtextxy_handler(VMContext *vm, LexerContext *lex);

//
// ---- Matrix Handlers ----

BppError stmt_mat_handler(VMContext *vm, LexerContext *lex);
BppError stmt_arrayext_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mat_read_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mat_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mat_input_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mat_ops_handler(VMContext *vm, LexerContext *lex);

//
// ---- OOP & Modular Structure Handlers ----

BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex);
BppError stmt_function_handler(VMContext *vm, LexerContext *lex);
BppError stmt_call_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_subend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_subexit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_type_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_type_handler(VMContext *vm, LexerContext *lex);
BppError stmt_class_handler(VMContext *vm, LexerContext *lex);
BppError stmt_record_handler(VMContext *vm, LexerContext *lex);
BppError stmt_module_handler(VMContext *vm, LexerContext *lex);
BppError stmt_import_handler(VMContext *vm, LexerContext *lex);
BppError stmt_scope_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);
BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex);
BppError stmt_override_handler(VMContext *vm, LexerContext *lex);
BppError stmt_with_handler(VMContext *vm, LexerContext *lex);
BppError stmt_enum_handler(VMContext *vm, LexerContext *lex);
BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex);

//
// ---- System, Network, ISAM & UI Handlers ----

BppError stmt_doevents_handler(VMContext *vm, LexerContext *lex);
BppError stmt_msgbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_button_handler(VMContext *vm, LexerContext *lex);
BppError stmt_menu_handler(VMContext *vm, LexerContext *lex);
BppError stmt_textbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_listbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_checkbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_optionbutton_handler(VMContext *vm, LexerContext *lex);
BppError stmt_label_handler(VMContext *vm, LexerContext *lex);
BppError stmt_frame_handler(VMContext *vm, LexerContext *lex);
BppError stmt_combobox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hscrollbar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vscrollbar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drivelistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dirlistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_filelistbox_handler(VMContext *vm, LexerContext *lex);
BppError stmt_timercontrol_handler(VMContext *vm, LexerContext *lex);
BppError stmt_interrupt_handler(VMContext *vm, LexerContext *lex);
BppError stmt_interruptx_handler(VMContext *vm, LexerContext *lex);
BppError stmt_createindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_deleteindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_setindex_handler(VMContext *vm, LexerContext *lex);
BppError stmt_insert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_update_handler(VMContext *vm, LexerContext *lex);
BppError stmt_retrieve_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekeq_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekge_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seekgt_handler(VMContext *vm, LexerContext *lex);
BppError stmt_login_handler(VMContext *vm, LexerContext *lex);
BppError stmt_who_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tty_handler(VMContext *vm, LexerContext *lex);
BppError stmt_priority_handler(VMContext *vm, LexerContext *lex);
BppError stmt_extend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_noextend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_scale_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sys_handler(VMContext *vm, LexerContext *lex);
BppError stmt_suspend_handler(VMContext *vm, LexerContext *lex);

//
// ---- Program Management & Interactive Command Handlers ----

BppError stmt_run_handler(VMContext *vm, LexerContext *lex);
BppError stmt_brun_handler(VMContext *vm, LexerContext *lex);
BppError stmt_list_handler(VMContext *vm, LexerContext *lex);
BppError stmt_load_handler(VMContext *vm, LexerContext *lex);
BppError stmt_save_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_merge_handler(VMContext *vm, LexerContext *lex);
BppError stmt_chain_handler(VMContext *vm, LexerContext *lex);
BppError stmt_new_handler(VMContext *vm, LexerContext *lex);
BppError stmt_clear_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cont_handler(VMContext *vm, LexerContext *lex);
BppError stmt_auto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_renum_handler(VMContext *vm, LexerContext *lex);
BppError stmt_reformat_handler(VMContext *vm, LexerContext *lex);
BppError stmt_delete_handler(VMContext *vm, LexerContext *lex);
BppError stmt_edit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex);
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sleep_handler(VMContext *vm, LexerContext *lex);
BppError stmt_wait_handler(VMContext *vm, LexerContext *lex);
BppError stmt_assert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_break_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex);
BppError stmt_check_handler(VMContext *vm, LexerContext *lex);
BppError stmt_verify_handler(VMContext *vm, LexerContext *lex);
BppError stmt_test_handler(VMContext *vm, LexerContext *lex);
BppError stmt_endtest_handler(VMContext *vm, LexerContext *lex);
BppError stmt_trace_handler(VMContext *vm, LexerContext *lex);
BppError stmt_debug_handler(VMContext *vm, LexerContext *lex);
BppError stmt_backtrace_handler(VMContext *vm, LexerContext *lex);
BppError stmt_info_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dump_handler(VMContext *vm, LexerContext *lex);
BppError stmt_help_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex);
BppError stmt_category_handler(VMContext *vm, LexerContext *lex);
BppError stmt_remove_handler(VMContext *vm, LexerContext *lex);
BppError stmt_remove_str_handler(VMContext *vm, LexerContext *lex);
BppError stmt_version_handler(VMContext *vm, LexerContext *lex);
BppError stmt_selftest_handler(VMContext *vm, LexerContext *lex);

//
// ---- IoT & Microcontroller Statement Handlers ----

BppError stmt_pinmode_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dwrite_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dac_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pwm_handler(VMContext *vm, LexerContext *lex);
BppError stmt_servo_handler(VMContext *vm, LexerContext *lex);
BppError stmt_i2c_handler(VMContext *vm, LexerContext *lex);
BppError stmt_spi_handler(VMContext *vm, LexerContext *lex);
BppError stmt_neopixel_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dht_handler(VMContext *vm, LexerContext *lex);
BppError stmt_delay_handler(VMContext *vm, LexerContext *lex);
BppError stmt_every_handler(VMContext *vm, LexerContext *lex);
BppError stmt_deepsleep_handler(VMContext *vm, LexerContext *lex);
BppError stmt_reboot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_freq_handler(VMContext *vm, LexerContext *lex);
BppError stmt_wifi_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mqtt_handler(VMContext *vm, LexerContext *lex);
BppError stmt_webrepl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_peer_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bt_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ble_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nfc_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gopher_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tnfs_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gemini_browse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_remote_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nil_unpack_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_port_trigger_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sniff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_net_config_handler(VMContext *vm, LexerContext *lex);

#endif // STMT_HANDLERS_H

// FILENAME: exec_dispatch.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements bytecode virtual machine execution and state for exec_dispatch.
//
// ---- Includes ----

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vm_internal.h"
#include "device/vprinter.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/metadata.h"
#include "runtime/spec.h"
#include "runtime/strings.h"
#include "security/security.h"
#include "statements/oop/sub.h"
#include "stmt/stmt.h"
#include "stmt/stmt_handlers.h"
#include "vm/exec_internal.h"
#include "vm/vm.h"

//
// ---- Statement Registration Table ----

// registers all standard and extended statement keywords in the VM statement registry
void register_core_statements(VMContext *vm) {
    stmt_register(vm->stmt_reg, KW_PRINT,  stmt_print_handler,  "PRINT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DISPLAY,stmt_display_handler,"DISPLAY",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DISP,   stmt_print_handler,  "DISP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DOEVENTS, stmt_doevents_handler, "DOEVENTS", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MSGBOX, stmt_msgbox_handler, "MSGBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BUTTON, stmt_button_handler, "BUTTON", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MENU, stmt_menu_handler, "MENU", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TEXTBOX, stmt_textbox_handler, "TEXTBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LISTBOX, stmt_listbox_handler, "LISTBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHECKBOX, stmt_checkbox_handler, "CHECKBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OPTIONBUTTON, stmt_optionbutton_handler, "OPTIONBUTTON", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LABEL, stmt_label_handler, "LABEL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FRAME, stmt_frame_handler, "FRAME", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COMBOBOX, stmt_combobox_handler, "COMBOBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HSCROLLBAR, stmt_hscrollbar_handler, "HSCROLLBAR", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VSCROLLBAR, stmt_vscrollbar_handler, "VSCROLLBAR", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DRIVELISTBOX, stmt_drivelistbox_handler, "DRIVELISTBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DIRLISTBOX, stmt_dirlistbox_handler, "DIRLISTBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FILELISTBOX, stmt_filelistbox_handler, "FILELISTBOX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TIMERCONTROL, stmt_timercontrol_handler, "TIMERCONTROL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INTERRUPT, stmt_interrupt_handler, "INTERRUPT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INTERRUPTX, stmt_interruptx_handler, "INTERRUPTX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CREATEINDEX, stmt_createindex_handler, "CREATEINDEX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DELETEINDEX, stmt_deleteindex_handler, "DELETEINDEX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SETINDEX, stmt_setindex_handler, "SETINDEX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INSERT, stmt_insert_handler, "INSERT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UPDATE, stmt_update_handler, "UPDATE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RETRIEVE, stmt_retrieve_handler, "RETRIEVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SEEKEQ, stmt_seekeq_handler, "SEEKEQ", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SEEKGE, stmt_seekge_handler, "SEEKGE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SEEKGT, stmt_seekgt_handler, "SEEKGT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LPRINT, stmt_lprint_handler, "LPRINT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LLIST,  stmt_llist_handler,  "LLIST",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LET,    stmt_let_handler,    "LET",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LSET,   stmt_lset_handler,   "LSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RSET,   stmt_rset_handler,   "RSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SWAP,   stmt_swap_handler,   "SWAP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXCHANGE, stmt_exchange_handler, "EXCHANGE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PERFORM,  stmt_perform_handler,  "PERFORM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INVOKE,   stmt_invoke_handler,   "INVOKE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PREFIX,   stmt_prefix_handler,   "PREFIX",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REPEAT,   stmt_repeat_handler,   "REPEAT",   STMT_FLAG_PROGRAM);

    stmt_register(vm->stmt_reg, KW_UNTIL,    stmt_until_handler,    "UNTIL",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENDLOOP,  stmt_endloop_handler,  "ENDLOOP",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_EXITIF,   stmt_exitif_handler,   "EXITIF",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DESTROY,  stmt_destroy_handler,  "DESTROY",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CREATE,   stmt_create_handler,   "CREATE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PARAM,    stmt_param_handler,    "PARAM",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_INPUT,  stmt_input_handler,  "INPUT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RANDOMIZE, stmt_randomize_handler, "RANDOMIZE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEMAND, stmt_input_handler,  "DEMAND", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GOTO,   stmt_goto_handler,   "GOTO",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_GOSUB,  stmt_gosub_handler,  "GOSUB",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RETURN, stmt_return_handler, "RETURN", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_END,    stmt_end_handler,    "END",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SYSTEM, stmt_system_handler, "SYSTEM", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHELL,  stmt_shell_handler,  "SHELL",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENVIRON, stmt_environ_handler, "ENVIRON", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BYE,    stmt_bye_handler,    "BYE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GOODBYE, stmt_goodbye_handler, "GOODBYE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STOP,   stmt_stop_handler,   "STOP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXIT,   stmt_exit_handler,    "EXIT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REM,    stmt_rem_handler,    "REM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IF,     stmt_if_handler,     "IF",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LIST,   stmt_list_handler,   "LIST",   STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_RUN,    stmt_run_handler,    "RUN",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_NEW,    stmt_new_handler,    "NEW",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_FOR,    stmt_for_handler,    "FOR",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_NEXT,   stmt_next_handler,   "NEXT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WHILE,  stmt_while_handler,  "WHILE",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WEND,   stmt_wend_handler,   "WEND",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DO,     stmt_do_handler,     "DO",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_LOOP,   stmt_loop_handler,   "LOOP",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DIM,    stmt_dim_handler,    "DIM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCAL,  stmt_local_handler,  "LOCAL",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATIC, stmt_static_handler, "STATIC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_REDIM,  stmt_redim_handler,  "REDIM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ERASE,  stmt_erase_handler,  "ERASE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OPTION, stmt_option_handler, "OPTION", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DATA,   stmt_data_handler,   "DATA",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_READ,   stmt_read_handler,   "READ",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RESTORE,stmt_restore_handler,"RESTORE",STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DEF,    stmt_def_handler,    "DEF",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FNEND,  stmt_fnend_handler,  "FNEND",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DEFINT, stmt_defint_handler, "DEFINT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSNG, stmt_defsng_handler, "DEFSNG", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFDBL, stmt_defdbl_handler, "DEFDBL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSTR, stmt_defstr_handler, "DEFSTR", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFLNG, stmt_deflng_handler, "DEFLNG", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFCPX, stmt_defcpx_handler, "DEFCPX", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COMPLEX,stmt_complex_handler,"COMPLEX",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_USR,    stmt_defusr_handler, "USR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ON,     stmt_on_handler,     "ON",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_KEY,    stmt_key_handler,    "KEY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TIMER,  stmt_timer_handler,  "TIMER",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ALARM,  stmt_alarm_handler,  "ALARM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ALARM_STR, stmt_alarm_str_handler, "ALARM$", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SET,    stmt_set_handler,    "SET",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ERROR,  stmt_error_statement_handler, "ERROR", STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_TRY,      stmt_try_handler,      "TRY",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CATCH,    stmt_catch_handler,    "CATCH",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_THROW,    stmt_throw_handler,    "THROW",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_ALIAS,    stmt_alias_handler,    "ALIAS",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCOPE,    stmt_scope_handler,    "SCOPE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KEYWORD,  stmt_keyword_handler,  "KEYWORD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OVERRIDE, stmt_override_handler, "OVERRIDE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MODULE,   stmt_module_handler,   "MODULE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IMPORT,   stmt_import_handler,   "IMPORT",   STMT_FLAG_BOTH);

    stmt_register(vm->stmt_reg, KW_METADATA, stmt_metadata_handler, "METADATA", STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_RESUME, stmt_resume_handler, "RESUME", STMT_FLAG_PROGRAM);
#endif
#if SUPPORT_OOP
    stmt_register(vm->stmt_reg, KW_TYPE,   stmt_type_handler,   "TYPE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENDTYPE,stmt_end_type_handler,"ENDTYPE",STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CLASS,  stmt_class_handler,  "CLASS",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENUM,   stmt_enum_handler,   "ENUM",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WITH,   stmt_with_handler,   "WITH",   STMT_FLAG_PROGRAM);
#endif
    stmt_register(vm->stmt_reg, KW_LOAD,   stmt_load_handler,   "LOAD",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SAVE,   stmt_save_handler,   "SAVE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BLOAD,  stmt_bload_handler,  "BLOAD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BSAVE,  stmt_bsave_handler,  "BSAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BRUN,   stmt_brun_handler,   "BRUN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MERGE,  stmt_merge_handler,  "MERGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COMMON, stmt_common_handler, "COMMON", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CHAIN,  stmt_chain_handler,  "CHAIN",  STMT_FLAG_BOTH);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_SELFTEST,stmt_selftest_handler,"SELFTEST",STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_UNSAVE, stmt_unsave_handler, "UNSAVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ASSERT, stmt_assert_handler, "ASSERT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRON,   stmt_tron_handler,   "TRON",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TROFF,  stmt_troff_handler,  "TROFF",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BREAK,  stmt_break_handler,  "BREAK",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VARS,      stmt_vars_handler,      "VARS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHECK,     stmt_check_handler,     "CHECK",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VERIFY,    stmt_verify_handler,    "VERIFY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TEST,      stmt_test_handler,      "TEST",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENDTEST,   stmt_endtest_handler,   "ENDTEST",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRACE,     stmt_trace_handler,     "TRACE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEBUG,     stmt_debug_handler,     "DEBUG",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CONT,      stmt_cont_handler,      "CONT",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BACKTRACE, stmt_backtrace_handler, "BACKTRACE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INFO,      stmt_info_handler,      "INFO",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DUMP,      stmt_dump_handler,      "DUMP",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VERSION,   stmt_version_handler,   "VERSION",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VER,       stmt_version_handler,   "VER",       STMT_FLAG_BOTH);

#if SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_OPEN,   stmt_open_handler,   "OPEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FIELD,  stmt_field_handler,  "FIELD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSE,  stmt_close_handler,  "CLOSE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RESET,  stmt_reset_handler,  "RESET",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GET,    stmt_get_handler,    "GET",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BGET,   stmt_get_handler,    "BGET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PUT,    stmt_put_handler,    "PUT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BPUT,   stmt_put_handler,    "BPUT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SEEK,   stmt_seek_handler,   "SEEK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FILES,  stmt_files_handler,  "FILES",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KILL,   stmt_kill_handler,   "KILL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCRATCH,stmt_scratch_handler, "SCRATCH",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHDIR,  stmt_chdir_handler,  "CHDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MKDIR,  stmt_mkdir_handler,  "MKDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RMDIR,  stmt_rmdir_handler,  "RMDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NAME,   stmt_name_handler,   "NAME",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEVICES,stmt_devices_handler,"DEVICES",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DIR,    stmt_dir_handler,    "DIR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SETATTR,stmt_setattr_handler,"SETATTR",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PWD,      stmt_pwd_handler,      "PWD",      STMT_FLAG_BOTH);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_HOSTNAME, stmt_hostname_handler, "HOSTNAME", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_USERNAME, stmt_username_handler, "USERNAME", STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_PATH,     stmt_path_handler,     "PATH",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCK,   stmt_lock_handler,   "LOCK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UNLOCK, stmt_unlock_handler, "UNLOCK", STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_SELECT, stmt_select_handler, "SELECT", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CASE,   stmt_case_handler,   "CASE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SUB,      stmt_sub_handler,      "SUB",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PROCEDURE, stmt_procedure_handler, "PROCEDURE", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_FUNCTION, stmt_function_handler, "FUNCTION", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CALL,     stmt_call_handler,     "CALL",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CONST,    stmt_const_handler,    "CONST",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DECLARE,  stmt_declare_handler,  "DECLARE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENDFUNC,  stmt_end_function_handler, "ENDFUNC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENDPROC,  stmt_end_sub_handler,      "ENDPROC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SUBEND,   stmt_subend_handler,       "SUBEND",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SUBEXIT,  stmt_subexit_handler,      "SUBEXIT", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_AUTO,    stmt_auto_handler,     "AUTO",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BCOLOR,  stmt_bcolor_handler,   "BCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FCOLOR,  stmt_fcolor_handler,   "FCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLEAR,   stmt_clear_handler,    "CLEAR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CURSOR,  stmt_cursor_handler,   "CURSOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCATE,  stmt_locate_handler,   "LOCATE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHARED,   stmt_shared_handler,   "SHARED",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHARE,    stmt_share_handler,    "SHARE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUBLIC,   stmt_public_handler,   "PUBLIC",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PRIVATE,  stmt_private_handler,  "PRIVATE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MODULE,   stmt_module_handler,   "MODULE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IMPORT,   stmt_import_handler,   "IMPORT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GLOBAL,   stmt_global_handler,   "GLOBAL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXPORT,   stmt_public_handler,   "EXPORT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXTERNAL, stmt_external_handler, "EXTERNAL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WHEN,     stmt_when_handler,     "WHEN",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_USE,      stmt_use_handler,      "USE",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_HANDLER,  stmt_handler_handler,  "HANDLER",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CAUSE,    stmt_cause_handler,    "CAUSE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RETRY,    stmt_retry_handler,    "RETRY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CONTINUE, stmt_continue_handler, "CONTINUE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ASK,      stmt_ask_handler,      "ASK",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VIEWPORT, stmt_viewport_handler, "VIEWPORT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WINDOW,   stmt_window_handler,   "WINDOW",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PICTURE,  stmt_picture_handler,  "PICTURE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DRAW,     stmt_draw_handler,     "DRAW",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCREEN,  stmt_screen_handler,   "SCREEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COLOR,   stmt_color_handler,    "COLOR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLS,     stmt_cls_handler,      "CLS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HOME,    stmt_home_handler,     "HOME",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BEEP,    stmt_beep_handler,     "BEEP",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LINE,    stmt_line_handler,     "LINE",     STMT_FLAG_BOTH);
#if SUPPORT_GRAPHICS
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_CIRCLE,  stmt_circle_handler,   "CIRCLE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PSET,    stmt_pset_handler,     "PSET",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PRESET,  stmt_preset_handler,   "PRESET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PAINT,   stmt_paint_handler,    "PAINT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SOUND,   stmt_sound_handler,    "SOUND",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PLAY,    stmt_play_handler,     "PLAY",     STMT_FLAG_PROGRAM);
#endif
#endif
#if SUPPORT_EDITOR && !defined(BASIC_LITE_BUILD)
    stmt_register(vm->stmt_reg, KW_EDIT,    stmt_edit_handler,     "EDIT",     STMT_FLAG_BOTH);
#endif
#if SUPPORT_MAT
    stmt_register(vm->stmt_reg, KW_MAT,      stmt_mat_handler,      "MAT",      STMT_FLAG_BOTH);
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_ARRAY,    stmt_arrayext_handler, "ARRAY",    STMT_FLAG_BOTH);
#endif
#endif
    stmt_register(vm->stmt_reg, KW_MUX,      stmt_mux_handler,      "MUX",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEMUX,    stmt_demux_handler,    "DEMUX",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UNPACK,   stmt_unpack_handler,   "UNPACK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BITMUX,   stmt_bitmux_handler,   "BITMUX",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHANGE,   stmt_change_handler,   "CHANGE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MARGIN,   stmt_margin_handler,   "MARGIN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ZONE,     stmt_zone_handler,     "ZONE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_APPEND,   stmt_append_handler,   "APPEND",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MODIFY,   stmt_modify_handler,   "MODIFY",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TEXT,     stmt_text_handler,     "TEXT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LINPUT,   stmt_linput_handler,   "LINPUT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IMAGE,    stmt_image_handler,    "IMAGE",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_FORM,     stmt_form_handler,     "FORM",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RENUM,    stmt_renum_handler,    "RENUM",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_REFORMAT, stmt_reformat_handler, "REFORMAT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DELETE,   stmt_delete_handler,   "DELETE",   STMT_FLAG_BOTH);
#if SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_HELP,       stmt_help_handler,       "HELP",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CATALOG,    stmt_catalog_handler,    "CATALOG",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CATEGORY,   stmt_category_handler,   "CATEGORY",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CATEGORIES, stmt_category_handler,   "CATEGORIES", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REMOVE,     stmt_remove_handler,     "REMOVE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REMOVE_STR, stmt_remove_str_handler, "REMOVE$",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_POKE,     stmt_poke_handler,     "POKE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OUT,      stmt_out_handler,      "OUT",      STMT_FLAG_BOTH);

    stmt_register(vm->stmt_reg, KW_NOISE,      stmt_noise_handler,      "NOISE",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SNDPLAY,    stmt_sndplay_handler,    "_SNDPLAY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDLOOP,    stmt_sndloop_handler,    "_SNDLOOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDSTOP,    stmt_sndstop_handler,    "_SNDSTOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDPAUSE,   stmt_sndpause_handler,   "_SNDPAUSE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDVOL,     stmt_sndvol_handler,     "_SNDVOL",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEINPUT, stmt_mouseinput_handler, "_MOUSEINPUT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEHIDE,  stmt_mousehide_handler,  "_MOUSEHIDE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSESHOW,  stmt_mouseshow_handler,  "_MOUSESHOW",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSE,      stmt_mouse_handler,      "MOUSE",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HMOUSE,     stmt_hmouse_handler,     "HMOUSE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VMOUSE,     stmt_vmouse_handler,     "VMOUSE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRIG,       stmt_trig_handler,       "TRIG",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TITLE,      stmt_title_handler,      "_TITLE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCREENMOVE, stmt_screenmove_handler, "_SCREENMOVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FULLSCREEN, stmt_fullscreen_handler, "_FULLSCREEN", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RESIZE,     stmt_resize_handler,     "_RESIZE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ICON,       stmt_icon_handler,       "_ICON",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NWRITE,     stmt_nwrite_handler,     "NWRITE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FREEIMAGE,  stmt_freeimage_handler,  "_FREEIMAGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTIMAGE,   stmt_putimage_handler,   "_PUTIMAGE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATESAVE,  stmt_statesave_handler,  "_STATESAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STATELOAD,  stmt_stateload_handler,  "_STATELOAD",  STMT_FLAG_BOTH);

#if SUPPORT_GRAPHICS
#ifndef BASIC_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_GR,       stmt_gr_handler,       "GR",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR,      stmt_hgr_handler,      "HGR",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR2,     stmt_hgr2_handler,     "HGR2",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HCOLOR,   stmt_hcolor_handler,   "HCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PLOT,     stmt_plot_handler,     "PLOT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HLIN,     stmt_hlin_handler,     "HLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VLIN,     stmt_vlin_handler,     "VLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HPLOT,    stmt_hplot_handler,    "HPLOT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GRAPHICS, stmt_graphics_handler, "GRAPHICS", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MODE,     stmt_mode_handler,     "MODE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DRAWTO,   stmt_drawto_handler,   "DRAWTO",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BORDER,   stmt_border_handler,   "BORDER",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INK,      stmt_ink_handler,      "INK",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PAPER,    stmt_paper_handler,    "PAPER",    STMT_FLAG_BOTH);

    stmt_register(vm->stmt_reg, KW_INITGRAPH,  stmt_initgraph_handler,  "INITGRAPH",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSEGRAPH, stmt_closegraph_handler, "CLOSEGRAPH", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTPIXEL,   stmt_putpixel_handler,   "PUTPIXEL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BAR,        stmt_bar_handler,        "BAR",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ELLIPSE,    stmt_ellipse_handler,    "ELLIPSE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RECTANGLE,  stmt_rectangle_handler,  "RECTANGLE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OUTTEXTXY,  stmt_outtextxy_handler,  "OUTTEXTXY",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PALETTE,    stmt_palette_handler,    "PALETTE",    STMT_FLAG_BOTH);
#endif
#endif

    stmt_register(vm->stmt_reg, KW_PAUSE,    stmt_pause_handler,    "PAUSE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ASSIGN,   stmt_assign_handler,   "ASSIGN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ADVANCE,  stmt_advance_handler,  "ADVANCE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RECORD,   stmt_record_handler,   "RECORD",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FIND,     stmt_find_handler,     "FIND",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MAP,      stmt_map_handler,      "MAP",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MAPEND,   stmt_mapend_handler,   "MAPEND",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SLEEP,    stmt_sleep_handler,    "SLEEP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SUSPEND,  stmt_suspend_handler,  "SUSPEND",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WAIT,     stmt_wait_handler,     "WAIT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ECHO,     stmt_echo_handler,     "ECHO",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NOECHO,   stmt_noecho_handler,   "NOECHO",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOGIN,    stmt_login_handler,    "LOGIN",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HELLO,    stmt_login_handler,    "HELLO",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WHO,      stmt_who_handler,      "WHO",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TTY,      stmt_tty_handler,      "TTY",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PRIORITY, stmt_priority_handler, "PRIORITY", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXTEND,   stmt_extend_handler,   "EXTEND",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NOEXTEND, stmt_noextend_handler, "NOEXTEND", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCALE,    stmt_scale_handler,    "SCALE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENTER,    stmt_enter_handler,    "ENTER",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OLD,      stmt_load_handler,     "OLD",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COM,      stmt_common_handler,   "COM",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_INCR,     stmt_incr_handler,     "INCR",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DECR,     stmt_decr_handler,     "DECR",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLR,      stmt_clr_handler,      "CLR",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VOID,     stmt_void_handler,     "VOID",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ARRAYFILL,stmt_arrayfill_handler,"ARRAYFILL",STMT_FLAG_BOTH);
#if SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_GET,      stmt_get_handler,      "GET",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BGET,     stmt_get_handler,      "BGET",     STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_SYS,      stmt_sys_handler,      "SYS",      STMT_FLAG_BOTH);
#if SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_ONERR,    stmt_onerr_handler,    "ONERR",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_WHENEVER, stmt_whenever_handler, "WHENEVER", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PAGE,      stmt_page_handler,     "PAGE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NOPAGE,    stmt_nopage_handler,   "NOPAGE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NOMARGIN,  stmt_nomargin_handler, "NOMARGIN",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REWIND,    stmt_rewind_handler,   "REWIND",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UNLESS,    stmt_unless_handler,   "UNLESS",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VDIM,      stmt_vdim_handler,     "VDIM",      STMT_FLAG_BOTH);

    // IoT & Microcontroller statements
    stmt_register(vm->stmt_reg, KW_PINMODE,   stmt_pinmode_handler,   "PINMODE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DWRITE,    stmt_dwrite_handler,    "DWRITE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DAC,       stmt_dac_handler,       "DAC",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PWM,       stmt_pwm_handler,       "PWM",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SERVO,     stmt_servo_handler,     "SERVO",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_I2C,       stmt_i2c_handler,       "I2C",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SPI,       stmt_spi_handler,       "SPI",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NEOPIXEL,  stmt_neopixel_handler,  "NEOPIXEL",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DHT,       stmt_dht_handler,       "DHT",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DELAY,     stmt_delay_handler,     "DELAY",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EVERY,     stmt_every_handler,     "EVERY",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEEPSLEEP, stmt_deepsleep_handler, "DEEPSLEEP", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LIGHTSLEEP,stmt_deepsleep_handler, "LIGHTSLEEP",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REBOOT,    stmt_reboot_handler,    "REBOOT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FREQ,      stmt_freq_handler,      "FREQ",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WIFI,      stmt_wifi_handler,      "WIFI",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MQTT,      stmt_mqtt_handler,      "MQTT",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_WEBREPL,   stmt_webrepl_handler,   "WEBREPL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PEER,      stmt_peer_handler,      "PEER",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BT,        stmt_bt_handler,        "BT",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BLE,       stmt_ble_handler,       "BLE",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NFC,       stmt_nfc_handler,       "NFC",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GEMINI,    stmt_gemini_handler,    "GEMINI",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GOPHER,    stmt_gopher_handler,    "GOPHER",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TNFS,      stmt_tnfs_handler,      "TNFS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REMOTE,    stmt_remote_handler,    "REMOTE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NIL,       stmt_nil_unpack_handler,"NIL",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NET,       stmt_net_config_handler,"NET",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SOCK,      stmt_sock_handler,      "SOCK",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PORT,      stmt_port_trigger_handler,"PORT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNIFF,     stmt_sniff_handler,     "SNIFF",     STMT_FLAG_BOTH);
}

// executes compiler-level pragmas and configuration directives
BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char dir_name[64];
    int len = (int)(dir_tok.length < sizeof(dir_name) - 1 ? dir_tok.length : sizeof(dir_name) - 1);
    memcpy(dir_name, dir_tok.as.string, len);
    dir_name[len] = '\0';

    if (strcasecmp(dir_name, "OPTION") == 0) {
        BppToken opt_tok = lex_next(lex);
        if (opt_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected option name in ::OPTION";
            return err;
        }
        char opt_name[64];
        int opt_len = (int)(opt_tok.length < sizeof(opt_name) - 1 ? opt_tok.length : sizeof(opt_name) - 1);
        memcpy(opt_name, opt_tok.start, opt_len);
        opt_name[opt_len] = '\0';

        BppMetadataRegistry *reg = vm_get_metadata(vm);
        if (strcasecmp(opt_name, "STRICT") == 0 || strcasecmp(opt_name, "EXPLICIT") == 0) {
            if (reg) reg->option_strict = true;
        } else {
            err.code = 2; err.message = "Unsupported option in ::OPTION";
            return err;
        }
    } else if (strcasecmp(dir_name, "INCLUDE") == 0) {
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_STRING) {
            err.code = 2; err.message = "Expected string argument for ::INCLUDE";
            return err;
        }
        char inc_path[512] = {0};
        size_t in_len = (val_tok.length < sizeof(inc_path) - 1) ? val_tok.length : sizeof(inc_path) - 1;
        if (val_tok.start && in_len > 0) {
            memcpy(inc_path, val_tok.start, in_len);
            inc_path[in_len] = '\0';
        }
        size_t plen = strlen(inc_path);
        if (plen >= 2 && inc_path[0] == '"' && inc_path[plen - 1] == '"') {
            memmove(inc_path, inc_path + 1, plen - 2);
            inc_path[plen - 2] = '\0';
            plen -= 2;
        }
        if (plen >= 2 && ((inc_path[0] == '\'' && inc_path[plen - 1] == '\'') ||
                          (inc_path[0] == '<' && inc_path[plen - 1] == '>'))) {
            memmove(inc_path, inc_path + 1, plen - 2);
            inc_path[plen - 2] = '\0';
        }

        FILE *f = fopen(inc_path, "r");
        if (!f) {
            char alt_path[512];
            snprintf(alt_path, sizeof(alt_path), "engine/include/%s", inc_path);
            f = fopen(alt_path, "r");
        }
        if (!f) {
            char alt_path[512];
            snprintf(alt_path, sizeof(alt_path), "include/%s", inc_path);
            f = fopen(alt_path, "r");
        }
        if (!f) {
            err.code = 53;
            err.message = "File not found in $INCLUDE directive";
            return err;
        }

        char line_buf[1024];
        while (fgets(line_buf, sizeof(line_buf), f)) {
            size_t llen = strlen(line_buf);
            while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
                line_buf[--llen] = '\0';
            }
            if (llen == 0) continue;
            BppError line_err = vm_execute_line(vm, line_buf);
            if (line_err.code != 0) {
                fclose(f);
                return line_err;
            }
        }
        fclose(f);
    } else {
        err.code = 2; err.message = "Unsupported compiler directive";
    }

    return err;
}

// checks if a given source line is a closing block marker for metadata
static bool is_block_end_marker(const char *text, const char *block_type, const char *block_target, MemoryContext *mem) {
    LexerContext *check_lex = lex_init(mem, text);
    if (!check_lex) return false;

    BppToken tok = lex_next(check_lex);
    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 3 && platform_strncasecmp(tok.start, "END", 3) == 0) {
        BppToken next_tok = lex_next(check_lex);
        if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
            char next_name[64];
            size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
            memcpy(next_name, next_tok.start, nlen);
            next_name[nlen] = '\0';
            if (platform_strcasecmp(next_name, block_type) == 0) {
                lex_shutdown(check_lex);
                return true;
            }
        }
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
        char target_buf[64];
        size_t tlen = (tok.length < 63) ? tok.length : 63;
        memcpy(target_buf, tok.start, tlen);
        target_buf[tlen] = '\0';

        if (tok.start + tok.length < text + strlen(text) && *(tok.start + tok.length) == ':') {
            if (tlen + 1 < 63) {
                target_buf[tlen] = ':';
                target_buf[tlen + 1] = '\0';
            }
        }

        if (strcasecmp(target_buf, block_target) == 0) {
            BppToken next_tok = lex_next(check_lex);
            if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                char next_name[64];
                size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                memcpy(next_name, next_tok.start, nlen);
                next_name[nlen] = '\0';

                if (strcasecmp(next_name, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
                    lex_shutdown(check_lex);
                    return true;
                }
            }
        }
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char name_buf[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(name_buf, tok.start, len);
        name_buf[len] = '\0';

        if (strcasecmp(name_buf, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
            lex_shutdown(check_lex);
            return true;
        }
    }

    lex_shutdown(check_lex);
    return false;
}

// skips over multi-line declarative metadata blocks at runtime
BppError skip_metadata_block(VMContext *vm, LexerContext *lex, const char *block_type) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char block_target[64] = "";
    BppToken target_tok = lex_peek(lex);
    if (target_tok.type == TOK_IDENT || target_tok.type == TOK_KEYWORD || target_tok.type == TOK_STRING) {
        size_t tlen = (target_tok.length < 63) ? target_tok.length : 63;
        memcpy(block_target, target_tok.start, tlen);
        block_target[tlen] = '\0';

        if (target_tok.start + target_tok.length < target_tok.start + 100 && *(target_tok.start + target_tok.length) == ':') {
            if (tlen + 1 < 63) {
                block_target[tlen] = ':';
                block_target[tlen + 1] = '\0';
            }
        }
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    BppLineNumber curr_line = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found_start = false;
    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number == curr_line) {
            start_idx = i;
            found_start = true;
            break;
        }
    }
    if (!found_start) return err;

    bool has_closing = false;
    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            has_closing = true;
            break;
        }
    }

    if (!has_closing) {
        BppToken skip_tok = lex_next(lex);
        while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
            skip_tok = lex_next(lex);
        }
        return err;
    }

    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            vm_jump(vm, lines[i].line_number + 0.0001, NULL);
            return err;
        }
    }

    err.code = 2; err.message = "Block defined without closing block marker";
    return err;
}

// executes dynamically declared keywords and dialect extensions
BppError execute_custom_keyword_statement(VMContext *vm, LexerContext *lex, BppKeywordId kw) {
    BppError err;
    memset(&err, 0, sizeof(err));

    SpecObject *spec = spec_find_by_kw_id(kw);
    if (!spec) {
        err.code = 2;
        err.message = "Unknown dynamic keyword";
        return err;
    }

    int req_lvl_val = security_find_level_by_name(spec->required_level);
    BppSecLevel req_lvl = (req_lvl_val >= 0) ? (BppSecLevel)req_lvl_val : SEC_STANDARD;
    if (security_check_pinned_level(req_lvl) == 0) {
        err.code = 70;
        err.message = "Permission denied executing dynamic keyword statement (restricted via security pin)";
        return err;
    }

    char sub_name[256] = "";
    BppToken next_tok = lex_peek(lex);
    bool has_subcommand = false;
    if (next_tok.type == TOK_IDENT) {
        char subcmd[128];
        size_t slen = (next_tok.length < sizeof(subcmd) - 1) ? next_tok.length : sizeof(subcmd) - 1;
        memcpy(subcmd, next_tok.start, slen);
        subcmd[slen] = '\0';

        char target1[256];
        snprintf(target1, sizeof(target1), "%s.%s", spec->name, subcmd);

        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, target1, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, target1, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex);
        } else if (find_procedure(vm, subcmd, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, subcmd, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex);
        }
    }

    if (!has_subcommand) {
        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, spec->name, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, spec->name, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
        } else {
            err.code = 35;
            err.message = "Subprogram or subcommand not defined for dynamic feature";
            return err;
        }
    }

    BValue args[MAX_PARAMS];
    int arg_count = 0;

    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        if (arg_count >= MAX_PARAMS) {
            err.code = 2; err.message = "Too many arguments in custom statement call";
            for (int i = 0; i < arg_count; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        args[arg_count++] = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            for (int i = 0; i < arg_count - 1; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        }
    }

    BValue res = invoke_user_function(vm, sub_name, args, arg_count, &err);
    if (res.type == VAL_STRING && res.as.string) str_release(vm_get_str(vm), res.as.string);
    else if (res.type == VAL_MAP && res.as.map) map_release(vm_get_str(vm), res.as.map);
    return err;
}

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stmtreg_builtins.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers all standard built-in optional statements into the Statement Registry.
 * ===================================================================== */

#include "stmtreg.h"
#include "parser_internal.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for UEF
void pi_parse_import(Lexer *lex, RuntimeState *rt, int line_num);
void pi_parse_include(Lexer *lex, RuntimeState *rt, int line_num);

// Forward declarations for inline/special handlers
static void pi_parse_bye_inline(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)rt; (void)line_num;
    printf("Goodbye.\n");
    exit(0);
}

static void pi_parse_let_explicit(Lexer *lex, RuntimeState *rt, int line_num) {
    pi_parse_let(lex, rt, line_num, 1);
}

static void pi_parse_line_dispatch(Lexer *lex, RuntimeState *rt, int line_num) {
    if (lex->current.type == TOK_KEYWORD &&
        lex->current.value.keyword == KW_INPUT) {
        lexer_next(lex);
        pi_parse_line_input(lex, rt, line_num);
        return;
    }
#ifndef BPP_LITE_BUILD
    pi_parse_line(lex, rt, line_num);
#else
    error_raise(ERR_WHAT, line_num);
#endif
}

void stmtreg_register_builtins(void) {
    // Core optional variables
    stmtreg_register("LET", KW_LET, pi_parse_let_explicit);
    stmtreg_register("DIM", KW_DIM, pi_parse_dim);
    stmtreg_register("SWAP", KW_SWAP, pi_parse_swap);
    stmtreg_register("CLEAR", KW_CLEAR, pi_parse_clear);
    stmtreg_register("CLR", KW_CLR, pi_parse_clr);

    // Flow / Config modifications
    stmtreg_register("RANDOMIZE", KW_RANDOMIZE, pi_parse_randomize);
    stmtreg_register("AUTO", KW_AUTO, pi_parse_auto);

    // Help & Diagnostics
    stmtreg_register("HELP", KW_HELP, pi_parse_help);
    stmtreg_register("INFO", KW_INFO, pi_parse_info);
    stmtreg_register("CATALOG", KW_CATALOG, pi_parse_catalog);
    stmtreg_register("VER", KW_VER, pi_parse_ver);
    stmtreg_register("BYE", KW_BYE, pi_parse_bye_inline);

#ifndef BPP_LITE_BUILD
    // File I/O
    stmtreg_register("OPEN", KW_OPEN, pi_parse_open);
    stmtreg_register("CLOSE", KW_CLOSE, pi_parse_close);
    stmtreg_register("SET", KW_SET, pi_parse_set_file);
    stmtreg_register("ASK", KW_ASK, pi_parse_ask_file);
    stmtreg_register("REWRITE", KW_REWRITE, pi_parse_rewrite);
    stmtreg_register("MOUNT", KW_MOUNT, pi_parse_mount);
    stmtreg_register("UMOUNT", KW_UMOUNT, pi_parse_umount);
    stmtreg_register("MOUNTS", KW_MOUNTS, pi_parse_mounts);
    stmtreg_register("VPATH", KW_VPATH, pi_parse_vpath);

    // File Management
    stmtreg_register("FILES", KW_FILES, pi_parse_files);
    stmtreg_register("DIR", KW_DIR, pi_parse_dir);
    stmtreg_register("COPY", KW_COPY, pi_parse_copy);
    stmtreg_register("MOVE", KW_MOVE, pi_parse_move);
    stmtreg_register("KILL", KW_KILL, pi_parse_kill);
    stmtreg_register("SCRATCH", KW_SCRATCH, pi_parse_scratch);
    stmtreg_register("UNSAVE", KW_UNSAVE, pi_parse_unsave);
    stmtreg_register("PWD", KW_PWD, pi_parse_pwd);
    stmtreg_register("CHDIR", KW_CHDIR, pi_parse_chdir);
    stmtreg_register("MKDIR", KW_MKDIR, pi_parse_mkdir);
    stmtreg_register("RMDIR", KW_RMDIR, pi_parse_rmdir);
    stmtreg_register("NAME", KW_NAME, pi_parse_name);
    stmtreg_register("RENAME", KW_RENAME, pi_parse_rename);

    // Graphics
    stmtreg_register("SCREEN", KW_SCREEN, pi_parse_screen);
    stmtreg_register("GRAPHICS", KW_GRAPHICS, pi_parse_graphics);
    stmtreg_register("COLOR", KW_COLOR, pi_parse_color);
    stmtreg_register("LINE", KW_LINE, pi_parse_line_dispatch);
    stmtreg_register("CIRCLE", KW_CIRCLE, pi_parse_circle);
    stmtreg_register("PAINT", KW_PAINT, pi_parse_paint);
    stmtreg_register("PSET", KW_PSET, pi_parse_pset);
    stmtreg_register("PRESET", KW_PRESET, pi_parse_preset);
    stmtreg_register("PALETTE", KW_PALETTE, pi_parse_palette);
    stmtreg_register("PCOPY", KW_PCOPY, pi_parse_pcopy);
    stmtreg_register("VIEW", KW_VIEW, pi_parse_view);
    stmtreg_register("WINDOW", KW_WINDOW, pi_parse_window);
    stmtreg_register("DRAW", KW_DRAW, pi_parse_draw);

    // Display / Console
    stmtreg_register("CLS", KW_CLS, pi_parse_cls);
    stmtreg_register("HOME", KW_HOME, pi_parse_home);
    stmtreg_register("LOCATE", KW_LOCATE, pi_parse_locate);
    stmtreg_register("WIDTH", KW_WIDTH, pi_parse_width);
    stmtreg_register("INK", KW_INK, pi_parse_ink);
    stmtreg_register("PAPER", KW_PAPER, pi_parse_paper);
    stmtreg_register("BORDER", KW_BORDER, pi_parse_border);
    stmtreg_register("BRIGHT", KW_BRIGHT, pi_parse_bright);
    stmtreg_register("FLASH", KW_FLASH, pi_parse_flash);
    stmtreg_register("INVERSE", KW_INVERSE, pi_parse_inverse);
    stmtreg_register("OVER", KW_OVER, pi_parse_over);

    // Sound
    stmtreg_register("BEEP", KW_BEEP, pi_parse_beep);
    stmtreg_register("SOUND", KW_SOUND, pi_parse_sound);
    stmtreg_register("PLAY", KW_PLAY, pi_parse_play);

    // Debugging / SelfTest
    stmtreg_register("TRON", KW_TRON, pi_parse_tron);
    stmtreg_register("TROFF", KW_TROFF, pi_parse_troff);
    stmtreg_register("ASSERT", KW_ASSERT, pi_parse_assert);
    stmtreg_register("TEST", KW_TEST, pi_parse_test);
    stmtreg_register("ENDTEST", KW_ENDTEST, pi_parse_endtest);
    stmtreg_register("SELFTEST", KW_SELFTEST, pi_parse_selftest);
    stmtreg_register("CHECK", KW_CHECK, pi_parse_check);
    stmtreg_register("VERIFY", KW_VERIFY, pi_parse_verify);
    stmtreg_register("BREAK", KW_BREAK, pi_parse_break);
    stmtreg_register("CONT", KW_CONT, pi_parse_cont);

    // OOP / Classes
    stmtreg_register("CLASS", KW_CLASS, pi_parse_class);
    stmtreg_register("ENDCLASS", KW_ENDCLASS, pi_parse_endclass);

    // UEF Extension Framework
    stmtreg_register("IMPORT", KW_COUNT, pi_parse_import);
    stmtreg_register("INCLUDE", KW_COUNT, pi_parse_include);

    // Configuration / Scope / Alias
    stmtreg_register("ALIAS", KW_ALIAS, pi_parse_alias);
    stmtreg_register("SCOPE", KW_SCOPE, pi_parse_scope);
    stmtreg_register("KEYWORD", KW_KEYWORD, pi_parse_keyword);
    stmtreg_register("OVERRIDE", KW_OVERRIDE, pi_parse_override);
    stmtreg_register("SECURITY", KW_SECURITY, pi_parse_security);
    stmtreg_register("MODULE", KW_MODULE, pi_parse_module);

    // Virtual Introspection & VM
    stmtreg_register("VDEV", KW_VDEV, pi_parse_vdev);
    stmtreg_register("VMEM", KW_VMEM, pi_parse_vmem);
    stmtreg_register("VNET", KW_VNET, pi_parse_vnet);
    stmtreg_register("VCON", KW_VCON, pi_parse_vcon);
    stmtreg_register("VTERM", KW_VTERM, pi_parse_vterm);
    stmtreg_register("VMACH", KW_VMACH, pi_parse_vmach);
    stmtreg_register("DEVMAP", KW_DEVMAP, pi_parse_devmap);

    // Transactions / Database
    stmtreg_register("ATOMIC", KW_ATOMIC, pi_parse_atomic);
    stmtreg_register("TXN", KW_TXN, pi_parse_txn);
    stmtreg_register("COMMIT", KW_COMMIT, pi_parse_commit);
    stmtreg_register("ROLLBACK", KW_ROLLBACK, pi_parse_rollback);

    // Compilation / Exec archives
    stmtreg_register("COMPILE", KW_COMPILE, pi_parse_compile);
    stmtreg_register("BSAVE", KW_BSAVE, pi_parse_bsave);
    stmtreg_register("BLOAD", KW_BLOAD, pi_parse_bload);
    stmtreg_register("BRUN", KW_BRUN, pi_parse_brun);

    // Matrix
    stmtreg_register("MAT", KW_MAT, pi_parse_mat_cmd);

    // Miscellaneous
    stmtreg_register("PAUSE", KW_PAUSE, pi_parse_pause);
    stmtreg_register("DELAY", KW_DELAY, pi_parse_delay);
    stmtreg_register("REPEAT", KW_REPEAT, pi_parse_repeat);
    stmtreg_register("ENDREPEAT", KW_ENDREPEAT, pi_parse_endrepeat);
    stmtreg_register("ENDFOR", KW_ENDFOR, pi_parse_endfor);
    stmtreg_register("PROCEDURE", KW_PROCEDURE, pi_parse_procedure);
    stmtreg_register("LOCAL", KW_LOCAL, pi_parse_local);
    stmtreg_register("RENUM", KW_RENUM, pi_parse_renum);
    stmtreg_register("DELETE", KW_DELETE, pi_parse_delete);
    stmtreg_register("INP", KW_INP, NULL);
    stmtreg_register("OUT", KW_OUT, pi_parse_out);
    stmtreg_register("WAIT", KW_WAIT, pi_parse_wait);
    stmtreg_register("MOTOR", KW_MOTOR, pi_parse_motor);
    stmtreg_register("CLOAD", KW_CLOAD, pi_parse_cload);
    stmtreg_register("CSAVE", KW_CSAVE, pi_parse_csave);
    stmtreg_register("CRUN", KW_CRUN, pi_parse_crun);
    stmtreg_register("IMAGE", KW_IMAGE, pi_parse_image);
    stmtreg_register("COM", KW_COM, pi_parse_com);
#endif
}

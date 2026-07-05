/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_internal_additions.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Statement dispatch and recursive-descent expression parsing.
 *
 * 2. WHAT TO EXPECT:
 *    Highly recursive execution structure. Expression evaluation strictly obeys operator precedence levels.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Syntax parsing details for expressions, parser diagnostic logging, and parsing warnings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Operator precedence hierarchy, recursive call stack structure, keyword parsing dispatch methods.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Increase stack allocation limit if stack overflow occurs. Ensure expression parentheses evaluate in correct recursion.
 * ===================================================================== */

//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
// Auto-generated declarations for new parser modules

// parser_blockio.c
void pi_parse_field(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_get(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_put(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_lset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_seek(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_lock(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_unlock(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ioctl(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_config.c
void pi_parse_option(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_alias(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_scope(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_keyword(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_override(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_security(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_module(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_debug.c
void pi_parse_tron(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_troff(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_break(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_cont(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_assert(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_test(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endtest(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_selftest(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_info(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_display.c
void pi_parse_cls(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_home(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_locate(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_width(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ink(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_paper(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_border(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bright(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_flash(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_inverse(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_over(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_errhand.c
void pi_parse_error(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_cause(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_resume(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_when(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_use(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_retry(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_continue(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_filemgmt.c
void pi_parse_files(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_dir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_kill(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_scratch(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_unsave(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_copy(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_move(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pwd(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_chdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mkdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rmdir(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_name(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_rename(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_erase(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_flow.c
void pi_parse_on(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_else(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_elseif(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endif(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_graphics.c
void pi_parse_graphics(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_screen(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_console(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_color(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_draw(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_line(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_circle(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_paint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_preset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_palette(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pcopy(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_view(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_window(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_help.c
void pi_parse_help(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_catalog(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_ver(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_misc.c
void pi_parse_randomize(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pause(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_delay(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sleep(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_wait(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_clear(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_clr(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_reset(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_poke(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pokeb(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_task(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_memmap(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_mid(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_key(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_repeat(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endrepeat(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_endfor(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_out(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_com(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_pen(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_strig(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_access(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_image(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_progmgmt.c
void pi_parse_renum(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_delete(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_edit(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_auto(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_compile(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bsave(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_bload(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_shell.c
void pi_parse_shell(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_exec(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sys(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_system(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_environ(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_sound.c
void pi_parse_beep(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sound(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_play(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_streamio.c
void pi_parse_lprint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_llist(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_write(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_struct.c
void pi_parse_select(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_case(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_exit(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_sub(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_function(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_declare(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_call(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_procedure(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_define(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_enddefine(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_local(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_vars.c
void pi_parse_const_stmt(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_swap(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_redim(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_shared(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_static(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_common(Lexer *lex, RuntimeState *rt, int line_num);
void pi_parse_shared_cmd(Lexer *lex, RuntimeState *rt, int line_num);
void pi_parse_public_cmd(Lexer *lex, RuntimeState *rt, int line_num);
void pi_parse_type(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defint(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defdbl(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defsng(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_defstr(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vars(Lexer *lex, RuntimeState *rt,
    int line_num);

// parser_virtual.c
void pi_parse_bank(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vdev(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vmem(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vnet(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vcon(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vterm(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_vmach(Lexer *lex, RuntimeState *rt,
    int line_num);
void pi_parse_devmap(Lexer *lex, RuntimeState *rt,
    int line_num);


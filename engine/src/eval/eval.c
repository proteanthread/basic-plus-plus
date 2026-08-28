// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel, libstandard
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----

// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (stmt_reboot.c)
// NEEDED BY: libcore (bpp_api.c, clear.c, error.c, get.c, list.c, microplex.h)
// NEEDED BY: libcore (print_using.c, select.c, varptr.h)
// NEEDED BY: libengine (angle.h, append.c, array_ext.c, arrayfill.c, ask.c)
// NEEDED BY: libengine (assign.c, ast.c, ast_internal.h, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bios.c, bsave.c, builtins_internal.h, call.c)
// NEEDED BY: libengine (cause.c, chain.c, change.c, chdir.c, circle.c, class.c)
// NEEDED BY: libengine (close.c, cls.c, color.c, compat.c, complex.c, const.c)
// NEEDED BY: libengine (cont.c, context.c, control.c, conversion_fn.c)
// NEEDED BY: libengine (create.c, data.c, debug.c, def.c, def_seg.c, defseg.c)
// NEEDED BY: libengine (degrees.h, dim.c, dispatch_internal.h, do.c)
// NEEDED BY: libengine (doevents.c, draw.c, endloop.c, enter.c, enum.c, eps.h)
// NEEDED BY: libengine (eval_builtins.c, eval_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exit_loop.c, field.c, files.c, find.c)
// NEEDED BY: libengine (for.c, function.c, goodbye.c, gosub.c, goto.c, if.c)
// NEEDED BY: libengine (incr.c, inf.h, input.c, input_file.c, interrupt.c)
// NEEDED BY: libengine (invoke.c, isam.c, joystick.c, key.c, kill.c, let.c)
// NEEDED BY: libengine (line.c, line_input.c, linput.c, llist.c, load.c)
// NEEDED BY: libengine (lock.c, loop.c, lprint.c, lset.c, map.c, margin.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, mat_read.c)
// NEEDED BY: libengine (math_fn.c, maxnum.h, merge.c, mid_stmt.c, mkdir.c)
// NEEDED BY: libengine (modify.c, mouse.c, msgbox.c, mux.h, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, on_com.c, on_error.c, on_key.c)
// NEEDED BY: libengine (on_timer.c, open.c, option.c, out.c, pack.h, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, pick.h, play.c)
// NEEDED BY: libengine (poke.c, print.c, print_file.c, pset.c, put.c)
// NEEDED BY: libengine (radians.h, randomize.c, read.c, record.c, remainder.h)
// NEEDED BY: libengine (remove.c, restore.c, resume.c, rewind.c, rmdir.c)
// NEEDED BY: libengine (rset.c, run.c, save.c, scale.c, screen.c, seek.c)
// NEEDED BY: libengine (session_stmts.c, shared.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (stmt_dac.c, stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (string_fn.c, sub_internal.h, suspend.c, swap.c, sys.c)
// NEEDED BY: libengine (sys_fn.c, system.c, task.c, text.c, truncate.h, txn.c)
// NEEDED BY: libengine (type.c, unless.c, until.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_widgets.c, view.c, viewport.c, vm_internal.h)
// NEEDED BY: libengine (voice.c, void.c, wait.c, wend.c, whenever.c, while.c)
// NEEDED BY: libengine (window.c, with.c, write_file.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (gfx.c, gfx_audio.c, gfx_internal.h)
// NEEDED BY: libkernel (gfx_primitives_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----

// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (stmt_reboot.c)
// NEEDED BY: libcore (bpp_api.c, clear.c, error.c, get.c, list.c, microplex.h)
// NEEDED BY: libcore (print_using.c, select.c, varptr.h)
// NEEDED BY: libengine (angle.h, append.c, array_ext.c, arrayfill.c, ask.c)
// NEEDED BY: libengine (assign.c, ast.c, ast_internal.h, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bios.c, bsave.c, builtins_internal.h, call.c)
// NEEDED BY: libengine (cause.c, chain.c, change.c, chdir.c, circle.c, class.c)
// NEEDED BY: libengine (close.c, cls.c, color.c, compat.c, complex.c, const.c)
// NEEDED BY: libengine (cont.c, context.c, control.c, conversion_fn.c)
// NEEDED BY: libengine (create.c, data.c, debug.c, def.c, def_seg.c, defseg.c)
// NEEDED BY: libengine (degrees.h, dim.c, dispatch_internal.h, do.c)
// NEEDED BY: libengine (doevents.c, draw.c, endloop.c, enter.c, enum.c, eps.h)
// NEEDED BY: libengine (eval_builtins.c, eval_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exit_loop.c, field.c, files.c, find.c)
// NEEDED BY: libengine (for.c, function.c, goodbye.c, gosub.c, goto.c, if.c)
// NEEDED BY: libengine (incr.c, inf.h, input.c, input_file.c, interrupt.c)
// NEEDED BY: libengine (invoke.c, isam.c, joystick.c, key.c, kill.c, let.c)
// NEEDED BY: libengine (line.c, line_input.c, linput.c, llist.c, load.c)
// NEEDED BY: libengine (lock.c, loop.c, lprint.c, lset.c, map.c, margin.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, mat_read.c)
// NEEDED BY: libengine (math_fn.c, maxnum.h, merge.c, mid_stmt.c, mkdir.c)
// NEEDED BY: libengine (modify.c, mouse.c, msgbox.c, mux.h, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, on_com.c, on_error.c, on_key.c)
// NEEDED BY: libengine (on_timer.c, open.c, option.c, out.c, pack.h, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, pick.h, play.c)
// NEEDED BY: libengine (poke.c, print.c, print_file.c, pset.c, put.c)
// NEEDED BY: libengine (radians.h, randomize.c, read.c, record.c, remainder.h)
// NEEDED BY: libengine (remove.c, restore.c, resume.c, rewind.c, rmdir.c)
// NEEDED BY: libengine (rset.c, run.c, save.c, scale.c, screen.c, seek.c)
// NEEDED BY: libengine (session_stmts.c, shared.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (stmt_dac.c, stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (string_fn.c, sub_internal.h, suspend.c, swap.c, sys.c)
// NEEDED BY: libengine (sys_fn.c, system.c, task.c, text.c, truncate.h, txn.c)
// NEEDED BY: libengine (type.c, unless.c, until.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_widgets.c, view.c, viewport.c, vm_internal.h)
// NEEDED BY: libengine (voice.c, void.c, wait.c, wend.c, whenever.c, while.c)
// NEEDED BY: libengine (window.c, with.c, write_file.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (gfx.c, gfx_audio.c, gfx_internal.h)
// NEEDED BY: libkernel (gfx_primitives_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----

// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.c, clear.c, error.c, get.c, list.c, microplex.h)
// NEEDED BY: libcore (print_using.c, select.c, varptr.h)
// NEEDED BY: libengine (angle.h, append.c, array_ext.c, arrayfill.c, ask.c)
// NEEDED BY: libengine (assign.c, ast.c, ast_internal.h, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bios.c, bsave.c, builtins_internal.h, call.c)
// NEEDED BY: libengine (cause.c, chain.c, change.c, chdir.c, circle.c, class.c)
// NEEDED BY: libengine (close.c, cls.c, color.c, compat.c, complex.c, const.c)
// NEEDED BY: libengine (cont.c, context.c, control.c, conversion_fn.c)
// NEEDED BY: libengine (create.c, data.c, debug.c, def.c, def_seg.c, defseg.c)
// NEEDED BY: libengine (degrees.h, dim.c, dispatch_internal.h, do.c)
// NEEDED BY: libengine (doevents.c, draw.c, endloop.c, enter.c, enum.c, eps.h)
// NEEDED BY: libengine (eval_builtins.c, eval_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exit_loop.c, field.c, files.c, find.c)
// NEEDED BY: libengine (for.c, function.c, goodbye.c, gosub.c, goto.c, if.c)
// NEEDED BY: libengine (incr.c, inf.h, input.c, input_file.c, interrupt.c)
// NEEDED BY: libengine (invoke.c, isam.c, joystick.c, key.c, kill.c, let.c)
// NEEDED BY: libengine (line.c, line_input.c, linput.c, llist.c, load.c)
// NEEDED BY: libengine (lock.c, loop.c, lprint.c, lset.c, map.c, margin.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, mat_read.c)
// NEEDED BY: libengine (math_fn.c, maxnum.h, merge.c, mid_stmt.c, mkdir.c)
// NEEDED BY: libengine (modify.c, mouse.c, msgbox.c, mux.h, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, on_com.c, on_error.c, on_key.c)
// NEEDED BY: libengine (on_timer.c, open.c, option.c, out.c, pack.h, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, pick.h, play.c)
// NEEDED BY: libengine (poke.c, print.c, print_file.c, pset.c, put.c)
// NEEDED BY: libengine (radians.h, randomize.c, read.c, record.c, remainder.h)
// NEEDED BY: libengine (remove.c, restore.c, resume.c, rewind.c, rmdir.c)
// NEEDED BY: libengine (rset.c, run.c, save.c, scale.c, screen.c, seek.c)
// NEEDED BY: libengine (session_stmts.c, shared.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (stmt_home.c, stmt_plot.c, string_fn.c, sub_internal.h)
// NEEDED BY: libengine (suspend.c, swap.c, sys.c, sys_fn.c, system.c, task.c)
// NEEDED BY: libengine (text.c, truncate.h, txn.c, type.c, unless.c, until.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_widgets.c, view.c, viewport.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, void.c, wait.c, wend.c)
// NEEDED BY: libengine (whenever.c, while.c, window.c, with.c, write_file.c)
// NEEDED BY: libengine (zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (gfx.c, gfx_audio.c, gfx_internal.h)
// NEEDED BY: libkernel (gfx_primitives_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----

// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (microplex.h, varptr.h)
// NEEDED BY: libcore (bpp_api.c, clear.c, error.c, get.c, list.c)
// NEEDED BY: libcore (print_using.c, select.c)
// NEEDED BY: libengine (angle.h, ast_internal.h, builtins_internal.h)
// NEEDED BY: libengine (degrees.h, dispatch_internal.h, eps.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_internal.h, inf.h, mat_internal.h, maxnum.h)
// NEEDED BY: libengine (mux.h, pack.h, pick.h, radians.h, remainder.h)
// NEEDED BY: libengine (sub_internal.h, truncate.h, vm_internal.h)
// NEEDED BY: libengine (append.c, array_ext.c, arrayfill.c, ask.c, assign.c)
// NEEDED BY: libengine (ast.c, backspace.c, beep.c, bgi.c, bios.c, bsave.c)
// NEEDED BY: libengine (call.c, cause.c, chain.c, change.c, chdir.c, circle.c)
// NEEDED BY: libengine (class.c, close.c, cls.c, color.c, compat.c, complex.c)
// NEEDED BY: libengine (const.c, cont.c, context.c, control.c, conversion_fn.c)
// NEEDED BY: libengine (create.c, data.c, debug.c, def.c, def_seg.c, defseg.c)
// NEEDED BY: libengine (dim.c, do.c, doevents.c, draw.c, endloop.c, enter.c)
// NEEDED BY: libengine (enum.c, eval_builtins.c, exec_dispatch.c, exit_loop.c)
// NEEDED BY: libengine (field.c, files.c, find.c, for.c, function.c, goodbye.c)
// NEEDED BY: libengine (gosub.c, goto.c, if.c, incr.c, input.c, input_file.c)
// NEEDED BY: libengine (interrupt.c, invoke.c, isam.c, joystick.c, key.c)
// NEEDED BY: libengine (kill.c, let.c, line.c, line_input.c, linput.c, llist.c)
// NEEDED BY: libengine (load.c, lock.c, loop.c, lprint.c, lset.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_print.c, mat_read.c)
// NEEDED BY: libengine (math_fn.c, merge.c, mid_stmt.c, mkdir.c, modify.c)
// NEEDED BY: libengine (mouse.c, msgbox.c, mux.c, name.c, new.c, next.c)
// NEEDED BY: libengine (on_com.c, on_error.c, on_key.c, on_timer.c, open.c)
// NEEDED BY: libengine (option.c, out.c, page.c, paint.c, palette.c, pause.c)
// NEEDED BY: libengine (pen.c, play.c, poke.c, print.c, print_file.c, pset.c)
// NEEDED BY: libengine (put.c, randomize.c, read.c, record.c, remove.c)
// NEEDED BY: libengine (restore.c, resume.c, rewind.c, rmdir.c, rset.c, run.c)
// NEEDED BY: libengine (save.c, scale.c, screen.c, seek.c, session_stmts.c)
// NEEDED BY: libengine (shared.c, shell.c, sleep.c, sound.c, stmt_home.c)
// NEEDED BY: libengine (stmt_plot.c, string_fn.c, suspend.c, swap.c, sys.c)
// NEEDED BY: libengine (sys_fn.c, system.c, task.c, text.c, txn.c, type.c)
// NEEDED BY: libengine (unless.c, until.c, vbdos_controls.c, vbdos_widgets.c)
// NEEDED BY: libengine (view.c, viewport.c, voice.c, void.c, wait.c, wend.c)
// NEEDED BY: libengine (whenever.c, while.c, window.c, with.c, write_file.c)
// NEEDED BY: libengine (zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (gfx_internal.h, gfx_primitives_internal.h)
// NEEDED BY: libkernel (gfx.c, gfx_audio.c)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval within BASIC++.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"

//
// ---- Expression Evaluator Orchestrator ----

BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));


    #define EVAL_EARLY_RETURN do { vm_dec_eval_depth(vm); return null_val; } while(0)
    #define VAL_PUSH(v) do { if (val_ptr >= MAX_EVAL_DEPTH) { out_err->code = 14; out_err->message = "Evaluation stack overflow"; EVAL_EARLY_RETURN; } val_stack[val_ptr++] = (v); } while(0)
    #define OP_PUSH(o) do { if (op_ptr >= MAX_EVAL_DEPTH) { out_err->code = 14; out_err->message = "Operator stack overflow"; EVAL_EARLY_RETURN; } op_stack[op_ptr++] = (o); } while(0)

    // Guard against C-stack overflow from deeply nested expression evaluation
    vm_inc_eval_depth(vm);
    if (vm_get_eval_depth(vm) > 64) {
        vm_dec_eval_depth(vm);
        out_err->code = 14;
        out_err->message = "Expression nesting too deep (limit 64)";
        return null_val;
    }

    BValue val_stack[MAX_EVAL_DEPTH];
    BppTokenType op_stack[MAX_EVAL_DEPTH];
    size_t val_ptr = 0;
    size_t op_ptr = 0;
    int open_parens = 0;
    bool expect_operand = true;

    BppToken tok = lex_peek(lex);

    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA &&
           tok.type != TOK_SEMICOLON && (tok.type != TOK_RPAREN || open_parens > 0) &&
           tok.type != TOK_RBRACKET &&
           (open_parens > 0 || !eval_is_clause_delimiter(tok))) {

        // Stop parsing if AT identifier
        if (tok.type == TOK_IDENT && tok.length == 2 &&
            (tok.start[0] == 'A' || tok.start[0] == 'a') &&
            (tok.start[1] == 'T' || tok.start[1] == 't')) {
            break;
        }

        // Implied delimiter check
        if (!expect_operand) {
            if (tok.type != TOK_RPAREN && tok.type != TOK_RBRACKET &&
                !eval_is_operator(tok.type) &&
                !(tok.type == TOK_KEYWORD && (tok.as.keyword == KW_AND || tok.as.keyword == KW_OR ||
                                              tok.as.keyword == KW_XOR || tok.as.keyword == KW_MOD ||
                                              tok.as.keyword == KW_EQV || tok.as.keyword == KW_IMP ||
                                              tok.as.keyword == KW_SHL || tok.as.keyword == KW_SHR ||
                                              tok.as.keyword == KW_READBIT || tok.as.keyword == KW_SETBIT ||
                                              tok.as.keyword == KW_RESETBIT || tok.as.keyword == KW_TOGGLEBIT ||
                                              tok.as.keyword == KW_MIN || tok.as.keyword == KW_MAX ||
                                              tok.as.keyword == KW_HYPOT || tok.as.keyword == KW_REMAINDER ||
                                              tok.as.keyword == KW_ATAN2))) {
                break;
            }
        }

        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got number";
                EVAL_EARLY_RETURN;
            }
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_IMAGINARY) {
            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got imaginary number";
                EVAL_EARLY_RETURN;
            }
            BValue val;
            val.type = VAL_COMPLEX;
            val.as.complex_val.real = 0.0;
            val.as.complex_val.imag = tok.as.number;
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_STRING) {
            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got string";
                EVAL_EARLY_RETURN;
            }
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_RPN_LITERAL) {
            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got RPN literal";
                EVAL_EARLY_RETURN;
            }
            char *rpn_str = (char *)mem_scratch_alloc(vm_get_mem(vm), tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14; out_err->message = "Scratch memory exhausted";
                EVAL_EARLY_RETURN;
            }
            runtime_memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(vm_get_mem(vm), rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) EVAL_EARLY_RETURN;

            VAL_PUSH(res);
            expect_operand = false;
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_NEW) ||
                   (tok.type == TOK_IDENT && tok.length == 3 && runtime_strncasecmp(tok.start, "NEW", 3) == 0)) {

            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got NEW";
                EVAL_EARLY_RETURN;
            }
            BValue res_val;
            if (!eval_parse_new_expression(vm, lex, &res_val, out_err)) {
                EVAL_EARLY_RETURN;
            }
            VAL_PUSH(res_val);
            expect_operand = false;
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_PERIOD || tok.type == TOK_AT) {
            if (!expect_operand) {
                out_err->code = 2; out_err->message = "Expected operator, got variable, keyword or '.'";
                EVAL_EARLY_RETURN;
            }
            BValue val;
            bool is_func = false;
            if (!eval_parse_identifier_expression(vm, lex, tok, &val, &is_func, out_err)) {
                EVAL_EARLY_RETURN;
            }
            VAL_PUSH(val);
            expect_operand = false;
        } else if (tok.type == TOK_LPAREN) {
            if (!expect_operand) break;
            OP_PUSH(TOK_LPAREN);
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            if (expect_operand) {
                out_err->code = 2; out_err->message = "Expected operand before ')'";
                EVAL_EARLY_RETURN;
            }
            bool found_paren = false;
            while (op_ptr > 0) {
                BppTokenType top = op_stack[--op_ptr];
                if (top == TOK_LPAREN) {
                    found_paren = true;
                    break;
                }
                if (!eval_execute_op(vm, top, val_stack, &val_ptr, out_err)) {
                    EVAL_EARLY_RETURN;
                }
            }
            if (!found_paren) {
                out_err->code = 2; out_err->message = "Mismatched parentheses";
                EVAL_EARLY_RETURN;
            }
            open_parens--;
        } else if (eval_is_operator(tok.type)) {
            bool should_break = false;
            if (!eval_process_operator(vm, lex, tok, expect_operand, open_parens,
                                       val_stack, &val_ptr, op_stack, &op_ptr,
                                       &expect_operand, &should_break, out_err)) {
                EVAL_EARLY_RETURN;
            }
            if (should_break) break;
        } else {
            break;
        }

        tok = lex_peek(lex);
    }

    // Pop all remaining operators
    while (op_ptr > 0) {
        BppTokenType top = op_stack[--op_ptr];
        if (top == TOK_LPAREN) {
            out_err->code = 2; out_err->message = "Mismatched parentheses";
            EVAL_EARLY_RETURN;
        }
        if (!eval_execute_op(vm, top, val_stack, &val_ptr, out_err)) {
            EVAL_EARLY_RETURN;
        }
    }

    if (val_ptr != 1) {
        out_err->code = 2; out_err->message = "Invalid expression structure";
        vm_dec_eval_depth(vm);
        return null_val;
    }

    vm_dec_eval_depth(vm);
    return val_stack[0];

    #undef EVAL_EARLY_RETURN
    #undef VAL_PUSH
    #undef OP_PUSH
}

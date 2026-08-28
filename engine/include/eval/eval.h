// FILENAME: eval.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel, libstandard
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Implements component functionality for eval.h.
//
// ---- Includes ----

// FILENAME: eval.h
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
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Implements component functionality for eval.h.
//
// ---- Includes ----

// FILENAME: eval.h
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
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Implements component functionality for eval.h.
//
// ---- Includes ----

// FILENAME: eval.h
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
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Implements component functionality for eval.h.
//
// ---- Includes ----

// FILENAME: eval.h
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
// NEEDS: libcore (strings.h)
// NEEDS: libcore (strings.c)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libengine (lexer.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for eval within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include <stddef.h>

#include "lexer/lexer.h"
#include "runtime/strings.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Public Evaluation API ----

BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *err);
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);

//
// ---- C17 Ergonomic Helpers ----

// checks whether a value is any numeric type (double or int32)
static inline bool val_is_numeric(BValue val) {
    return val.type == VAL_NUMBER || val.type == VAL_INTEGER;
}

// checks whether a value is a reference-counted string
static inline bool val_is_string(BValue val) {
    return val.type == VAL_STRING;
}

// extracts numeric double from a numeric value
static inline double val_get_number(BValue val) {
    return val.as.number;
}

// safely releases reference-counted string value and clears pointer
static inline void val_release_string(VMContext *vm, BValue *val) {
    if (val && val->type == VAL_STRING && val->as.string) {
        str_release(vm_get_str(vm), val->as.string);
        val->as.string = NULL;
    }
}

#endif // EVAL_H

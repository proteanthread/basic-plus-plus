// FILENAME: types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libext, libkernel, libplatform, libserver
// NEEDS: platform, memory
// Implements component functionality for types.h.
//
// ---- Includes ----

// FILENAME: types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (analyzer.h, arrays.h, collections.h, complex_num.h)
// NEEDED BY: libcore (dialect.h, file.h, file_internal.h, funcreg.h, gemini.c)
// NEEDED BY: libcore (keyword_props.h, keyword_props.c, memory.h)
// NEEDED BY: libcore (micro_lib_metadata.h, module.h, segmented_mem.h)
// NEEDED BY: libcore (session.h, state.h, string.h, strings.h, struct.h)
// NEEDED BY: libcore (variables.h, variables_internal.h, varptr.h, vfs.h)
// NEEDED BY: libengine (abs.h, acos.h, alias.h, alias.c, append.h, ascii_fn.h)
// NEEDED BY: libengine (asin.h, ask.h, ast.h, ast_internal.h, atan2.h, ath.h)
// NEEDED BY: libengine (atn.h, auto.h, beep.h, bgi.h, bin.h, bios.h)
// NEEDED BY: libengine (bitcount.h, bload.h, brun.h, bsave.h)
// NEEDED BY: libengine (builtins_internal.h, call.h, category.h, cause.h)
// NEEDED BY: libengine (ceil.h, chain.h, change.h, chdir.h, check.h, chr.h)
// NEEDED BY: libengine (circle.h, clamp.h, class.h, clear.h, clock_num.h)
// NEEDED BY: libengine (clock_str.h, close.h, color.h, command_fn.h, common.h)
// NEEDED BY: libengine (comp.h, complex.h, complex_fn.h, cont.h, continue.h)
// NEEDED BY: libengine (cos.h, cosh.h, cot.h, create.h, cross.h, csc.h)
// NEEDED BY: libengine (csrlin.h, cvt.h, data.h, date.h, debug.h, declare.h)
// NEEDED BY: libengine (def.h, def_seg.h, def_usr.h, defdbl.h, defint.h)
// NEEDED BY: libengine (deflng.h, defseg.h, defsng.h, defstr.h, delete.h)
// NEEDED BY: libengine (destroy.h, det.h, dim.h, dispatch_internal.h, do.h)
// NEEDED BY: libengine (doevents.h, dot.h, draw.h, end.h, enter.h, enum.h)
// NEEDED BY: libengine (environ.h, eof_fn.h, erase.h, erl.h, err_fn.h, ert.h)
// NEEDED BY: libengine (eval.h, eval_builtins.c, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, exec_internal.h)
// NEEDED BY: libengine (exit_loop.h, exp.h, external.h, fid.h, field.h)
// NEEDED BY: libengine (files.h, financial.h, fix.h, floor.h, for.h, form.h)
// NEEDED BY: libengine (fpt.h, fre.h, freefile.h, function.h, get.h, global.h)
// NEEDED BY: libengine (gosub.h, goto.h, handler.h, help.h, hex.h, image.h)
// NEEDED BY: libengine (index_fn.h, inkey.h, inp.h, input.h, input_file.h)
// NEEDED BY: libengine (instr.h, int.h, interrupt.h, introspection.h, isam.h)
// NEEDED BY: libengine (joystick.h, keyword.h, keyword.c, kill.h, lbound.h)
// NEEDED BY: libengine (lcase.h, left.h, len.h, lerp.h, let.h, lexer.h, line.h)
// NEEDED BY: libengine (line_input.h, linput.h, list.h, llist.h, load.h)
// NEEDED BY: libengine (loc_fn.h, lock.h, lof.h, log.h, log10.h, log2.h)
// NEEDED BY: libengine (loop.h, lpos.h, lprint.h, lset.h, ltrim.h, mag.h)
// NEEDED BY: libengine (magtape.h, map.h, margin.h, mat_input.h)
// NEEDED BY: libengine (mat_internal.h, mat_ops.h, mat_print.h, mat_read.h)
// NEEDED BY: libengine (mbf.h, merge.h, mid.h, mid_stmt.h, mkdir.h, moddir.h)
// NEEDED BY: libengine (modify.h, mouse.h, msgbox.h, mux.h, name.h, new.h)
// NEEDED BY: libengine (next.h, num.h, oct.h, on_com.h, on_error.h, on_key.h)
// NEEDED BY: libengine (on_timer.h, open.h, option.h, override.h, override.c)
// NEEDED BY: libengine (paint.h, palette.h, param.h, pause.h, pdif.h)
// NEEDED BY: libengine (pds_datetime.h, pds_sys.h, peek.h, pen.h, pi.h)
// NEEDED BY: libengine (picture.h, play.h, point_fn.h, poke.h, pos.h, print.h)
// NEEDED BY: libengine (print_file.h, pset.h, public.h, put.h, rad.h)
// NEEDED BY: libengine (randomize.h, read.h, readbit.h, redim.h, reformat.h)
// NEEDED BY: libengine (rem.h, remove.h, renum.h, resetbit.h, restore.h)
// NEEDED BY: libengine (resume.h, retry.h, return.h, right.h, rmdir.h, rnd.h)
// NEEDED BY: libengine (round.h, rset.h, rtrim.h, run.h, save.h)
// NEEDED BY: libengine (scope.h, scope.c, screen.h, sec.h, seek.h, seg.h)
// NEEDED BY: libengine (selftest.h, session_stmts.h, setbit.h, sgn.h, share.h)
// NEEDED BY: libengine (shared.h, shell.h, shl.h, shr.h, shuffle.h, sin.h)
// NEEDED BY: libengine (sinh.h, sound.h, space.h, spc.h, spec_fn.h, sqr.h)
// NEEDED BY: libengine (stack.h, stmt.h, stmt_handlers.h, stop.h, str.h)
// NEEDED BY: libengine (str_math.h, sub.h, sub_internal.h, swap.h, sys.h)
// NEEDED BY: libengine (sys_fn.h, tab.h, tan.h, tanh.h, task.h, tek.h, test.h)
// NEEDED BY: libengine (text.h, ticks.h, time.h, time_fn.h, timer.h)
// NEEDED BY: libengine (togglebit.h, trim.h, txn.h, typ.h, type.h, ubound.h)
// NEEDED BY: libengine (ucase.h, unpack.h, ups.h, val.h, vbdos_controls.h)
// NEEDED BY: libengine (vbdos_filebox.h, vbdos_fn.h, vbdos_widgets.h, verify.h)
// NEEDED BY: libengine (verify_fn.h, view.h, viewport.h, vm.h, voice.h, wend.h)
// NEEDED BY: libengine (when.h, while.h, window.h, with.h, write_file.h)
// NEEDED BY: libengine (xlate.h, zone.h)
// NEEDED BY: libext (arrayext.h)
// NEEDED BY: libkernel (gfx.c, gfx_internal.h, security.h, usb.h, vcon.h)
// NEEDED BY: libkernel (vdev.h, version.h)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform.h)
// NEEDED BY: libserver (vnet.h)
// NEEDS: platform, memory
// Implements component functionality for types.h.
//
// ---- Includes ----

// FILENAME: types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (analyzer.h, arrays.h, collections.h, complex_num.h)
// NEEDED BY: libcore (dialect.h, file.h, file_internal.h, funcreg.h, gemini.c)
// NEEDED BY: libcore (keyword_props.h, keyword_props.c, memory.h)
// NEEDED BY: libcore (micro_lib_metadata.h, module.h, segmented_mem.h)
// NEEDED BY: libcore (session.h, state.h, string.h, strings.h, struct.h)
// NEEDED BY: libcore (variables.h, variables_internal.h, varptr.h, vfs.h)
// NEEDED BY: libengine (abs.h, acos.h, alias.h, alias.c, append.h, ascii_fn.h)
// NEEDED BY: libengine (asin.h, ask.h, ast.h, ast_internal.h, atan2.h, ath.h)
// NEEDED BY: libengine (atn.h, auto.h, beep.h, bgi.h, bin.h, bios.h)
// NEEDED BY: libengine (bitcount.h, bload.h, brun.h, bsave.h)
// NEEDED BY: libengine (builtins_internal.h, call.h, category.h, cause.h)
// NEEDED BY: libengine (ceil.h, chain.h, change.h, chdir.h, check.h, chr.h)
// NEEDED BY: libengine (circle.h, clamp.h, class.h, clear.h, clock_num.h)
// NEEDED BY: libengine (clock_str.h, close.h, color.h, command_fn.h, common.h)
// NEEDED BY: libengine (comp.h, complex.h, complex_fn.h, cont.h, continue.h)
// NEEDED BY: libengine (cos.h, cosh.h, cot.h, create.h, cross.h, csc.h)
// NEEDED BY: libengine (csrlin.h, cvt.h, data.h, date.h, debug.h, declare.h)
// NEEDED BY: libengine (def.h, def_seg.h, def_usr.h, defdbl.h, defint.h)
// NEEDED BY: libengine (deflng.h, defseg.h, defsng.h, defstr.h, delete.h)
// NEEDED BY: libengine (destroy.h, det.h, dim.h, dispatch_internal.h, do.h)
// NEEDED BY: libengine (doevents.h, dot.h, draw.h, end.h, enter.h, enum.h)
// NEEDED BY: libengine (environ.h, eof_fn.h, erase.h, erl.h, err_fn.h, ert.h)
// NEEDED BY: libengine (eval.h, eval_builtins.c, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, exec_internal.h)
// NEEDED BY: libengine (exit_loop.h, exp.h, external.h, fid.h, field.h)
// NEEDED BY: libengine (files.h, financial.h, fix.h, floor.h, for.h, form.h)
// NEEDED BY: libengine (fpt.h, fre.h, freefile.h, function.h, get.h, global.h)
// NEEDED BY: libengine (gosub.h, goto.h, handler.h, help.h, hex.h, image.h)
// NEEDED BY: libengine (index_fn.h, inkey.h, inp.h, input.h, input_file.h)
// NEEDED BY: libengine (instr.h, int.h, interrupt.h, introspection.h, isam.h)
// NEEDED BY: libengine (joystick.h, keyword.h, keyword.c, kill.h, lbound.h)
// NEEDED BY: libengine (lcase.h, left.h, len.h, lerp.h, let.h, lexer.h, line.h)
// NEEDED BY: libengine (line_input.h, linput.h, list.h, llist.h, load.h)
// NEEDED BY: libengine (loc_fn.h, lock.h, lof.h, log.h, log10.h, log2.h)
// NEEDED BY: libengine (loop.h, lpos.h, lprint.h, lset.h, ltrim.h, mag.h)
// NEEDED BY: libengine (magtape.h, map.h, margin.h, mat_input.h)
// NEEDED BY: libengine (mat_internal.h, mat_ops.h, mat_print.h, mat_read.h)
// NEEDED BY: libengine (mbf.h, merge.h, mid.h, mid_stmt.h, mkdir.h, moddir.h)
// NEEDED BY: libengine (modify.h, mouse.h, msgbox.h, mux.h, name.h, new.h)
// NEEDED BY: libengine (next.h, num.h, oct.h, on_com.h, on_error.h, on_key.h)
// NEEDED BY: libengine (on_timer.h, open.h, option.h, override.h, override.c)
// NEEDED BY: libengine (paint.h, palette.h, param.h, pause.h, pdif.h)
// NEEDED BY: libengine (pds_datetime.h, pds_sys.h, peek.h, pen.h, pi.h)
// NEEDED BY: libengine (picture.h, play.h, point_fn.h, poke.h, pos.h, print.h)
// NEEDED BY: libengine (print_file.h, pset.h, public.h, put.h, rad.h)
// NEEDED BY: libengine (randomize.h, read.h, readbit.h, redim.h, reformat.h)
// NEEDED BY: libengine (rem.h, remove.h, renum.h, resetbit.h, restore.h)
// NEEDED BY: libengine (resume.h, retry.h, return.h, right.h, rmdir.h, rnd.h)
// NEEDED BY: libengine (round.h, rset.h, rtrim.h, run.h, save.h)
// NEEDED BY: libengine (scope.h, scope.c, screen.h, sec.h, seek.h, seg.h)
// NEEDED BY: libengine (selftest.h, session_stmts.h, setbit.h, sgn.h, share.h)
// NEEDED BY: libengine (shared.h, shell.h, shl.h, shr.h, shuffle.h, sin.h)
// NEEDED BY: libengine (sinh.h, sound.h, space.h, spc.h, spec_fn.h, sqr.h)
// NEEDED BY: libengine (stack.h, stmt.h, stmt_handlers.h, stop.h, str.h)
// NEEDED BY: libengine (str_math.h, sub.h, sub_internal.h, swap.h, sys.h)
// NEEDED BY: libengine (sys_fn.h, tab.h, tan.h, tanh.h, task.h, tek.h, test.h)
// NEEDED BY: libengine (text.h, ticks.h, time.h, time_fn.h, timer.h)
// NEEDED BY: libengine (togglebit.h, trim.h, txn.h, typ.h, type.h, ubound.h)
// NEEDED BY: libengine (ucase.h, unpack.h, ups.h, val.h, vbdos_controls.h)
// NEEDED BY: libengine (vbdos_filebox.h, vbdos_fn.h, vbdos_widgets.h, verify.h)
// NEEDED BY: libengine (verify_fn.h, view.h, viewport.h, vm.h, voice.h, wend.h)
// NEEDED BY: libengine (when.h, while.h, window.h, with.h, write_file.h)
// NEEDED BY: libengine (xlate.h, zone.h)
// NEEDED BY: libext (arrayext.h)
// NEEDED BY: libkernel (gfx.c, gfx_internal.h, security.h, usb.h, vcon.h)
// NEEDED BY: libkernel (vdev.h, version.h)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform.h)
// NEEDED BY: libserver (vnet.h)
// NEEDS: platform, memory
// Implements component functionality for types.h.
//
// ---- Includes ----

// FILENAME: types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (analyzer.h, arrays.h, collections.h, complex_num.h)
// NEEDED BY: libcore (dialect.h, file.h, file_internal.h, funcreg.h, gemini.c)
// NEEDED BY: libcore (keyword_props.h, keyword_props.c, memory.h)
// NEEDED BY: libcore (micro_lib_metadata.h, module.h, segmented_mem.h)
// NEEDED BY: libcore (session.h, state.h, string.h, strings.h, struct.h)
// NEEDED BY: libcore (variables.h, variables_internal.h, varptr.h, vfs.h)
// NEEDED BY: libengine (abs.h, acos.h, alias.h, alias.c, append.h, ascii_fn.h)
// NEEDED BY: libengine (asin.h, ask.h, ast.h, ast_internal.h, atan2.h, ath.h)
// NEEDED BY: libengine (atn.h, auto.h, beep.h, bgi.h, bin.h, bios.h)
// NEEDED BY: libengine (bitcount.h, bload.h, brun.h, bsave.h)
// NEEDED BY: libengine (builtins_internal.h, call.h, category.h, cause.h)
// NEEDED BY: libengine (ceil.h, chain.h, change.h, chdir.h, check.h, chr.h)
// NEEDED BY: libengine (circle.h, clamp.h, class.h, clear.h, clock_num.h)
// NEEDED BY: libengine (clock_str.h, close.h, color.h, command_fn.h, common.h)
// NEEDED BY: libengine (comp.h, complex.h, complex_fn.h, cont.h, continue.h)
// NEEDED BY: libengine (cos.h, cosh.h, cot.h, create.h, cross.h, csc.h)
// NEEDED BY: libengine (csrlin.h, cvt.h, data.h, date.h, debug.h, declare.h)
// NEEDED BY: libengine (def.h, def_seg.h, def_usr.h, defdbl.h, defint.h)
// NEEDED BY: libengine (deflng.h, defseg.h, defsng.h, defstr.h, delete.h)
// NEEDED BY: libengine (destroy.h, det.h, dim.h, dispatch_internal.h, do.h)
// NEEDED BY: libengine (doevents.h, dot.h, draw.h, end.h, enter.h, enum.h)
// NEEDED BY: libengine (environ.h, eof_fn.h, erase.h, erl.h, err_fn.h, ert.h)
// NEEDED BY: libengine (eval.h, eval_builtins.c, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, exec_internal.h)
// NEEDED BY: libengine (exit_loop.h, exp.h, external.h, fid.h, field.h)
// NEEDED BY: libengine (files.h, financial.h, fix.h, floor.h, for.h, form.h)
// NEEDED BY: libengine (fpt.h, fre.h, freefile.h, function.h, get.h, global.h)
// NEEDED BY: libengine (gosub.h, goto.h, handler.h, help.h, hex.h, image.h)
// NEEDED BY: libengine (index_fn.h, inkey.h, inp.h, input.h, input_file.h)
// NEEDED BY: libengine (instr.h, int.h, interrupt.h, introspection.h, isam.h)
// NEEDED BY: libengine (joystick.h, keyword.h, keyword.c, kill.h, lbound.h)
// NEEDED BY: libengine (lcase.h, left.h, len.h, lerp.h, let.h, lexer.h, line.h)
// NEEDED BY: libengine (line_input.h, linput.h, list.h, llist.h, load.h)
// NEEDED BY: libengine (loc_fn.h, lock.h, lof.h, log.h, log10.h, log2.h)
// NEEDED BY: libengine (loop.h, lpos.h, lprint.h, lset.h, ltrim.h, mag.h)
// NEEDED BY: libengine (magtape.h, map.h, margin.h, mat_input.h)
// NEEDED BY: libengine (mat_internal.h, mat_ops.h, mat_print.h, mat_read.h)
// NEEDED BY: libengine (mbf.h, merge.h, mid.h, mid_stmt.h, mkdir.h, moddir.h)
// NEEDED BY: libengine (modify.h, mouse.h, msgbox.h, mux.h, name.h, new.h)
// NEEDED BY: libengine (next.h, num.h, oct.h, on_com.h, on_error.h, on_key.h)
// NEEDED BY: libengine (on_timer.h, open.h, option.h, override.h, override.c)
// NEEDED BY: libengine (paint.h, palette.h, param.h, pause.h, pdif.h)
// NEEDED BY: libengine (pds_datetime.h, pds_sys.h, peek.h, pen.h, pi.h)
// NEEDED BY: libengine (picture.h, play.h, point_fn.h, poke.h, pos.h, print.h)
// NEEDED BY: libengine (print_file.h, pset.h, public.h, put.h, rad.h)
// NEEDED BY: libengine (randomize.h, read.h, readbit.h, redim.h, reformat.h)
// NEEDED BY: libengine (rem.h, remove.h, renum.h, resetbit.h, restore.h)
// NEEDED BY: libengine (resume.h, retry.h, return.h, right.h, rmdir.h, rnd.h)
// NEEDED BY: libengine (round.h, rset.h, rtrim.h, run.h, save.h)
// NEEDED BY: libengine (scope.h, scope.c, screen.h, sec.h, seek.h, seg.h)
// NEEDED BY: libengine (selftest.h, session_stmts.h, setbit.h, sgn.h, share.h)
// NEEDED BY: libengine (shared.h, shell.h, shl.h, shr.h, shuffle.h, sin.h)
// NEEDED BY: libengine (sinh.h, sound.h, space.h, spc.h, spec_fn.h, sqr.h)
// NEEDED BY: libengine (stack.h, stmt.h, stmt_handlers.h, stop.h, str.h)
// NEEDED BY: libengine (str_math.h, sub.h, sub_internal.h, swap.h, sys.h)
// NEEDED BY: libengine (sys_fn.h, tab.h, tan.h, tanh.h, task.h, tek.h, test.h)
// NEEDED BY: libengine (text.h, ticks.h, time.h, time_fn.h, timer.h)
// NEEDED BY: libengine (togglebit.h, trim.h, txn.h, typ.h, type.h, ubound.h)
// NEEDED BY: libengine (ucase.h, unpack.h, ups.h, val.h, vbdos_controls.h)
// NEEDED BY: libengine (vbdos_filebox.h, vbdos_fn.h, vbdos_widgets.h, verify.h)
// NEEDED BY: libengine (verify_fn.h, view.h, viewport.h, vm.h, voice.h, wend.h)
// NEEDED BY: libengine (when.h, while.h, window.h, with.h, write_file.h)
// NEEDED BY: libengine (xlate.h, zone.h)
// NEEDED BY: libext (arrayext.h)
// NEEDED BY: libkernel (gfx.c, gfx_internal.h, security.h, usb.h, vcon.h)
// NEEDED BY: libkernel (vdev.h, version.h)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform.h)
// NEEDED BY: libserver (vnet.h)
// NEEDS: platform, memory
// Implements component functionality for types.h.
//
// ---- Includes ----

// FILENAME: types.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (analyzer.h, arrays.h, collections.h, complex_num.h)
// NEEDED BY: libcore (dialect.h, file.h, file_internal.h, funcreg.h)
// NEEDED BY: libcore (keyword_props.h, memory.h, micro_lib_metadata.h)
// NEEDED BY: libcore (module.h, segmented_mem.h, session.h, state.h, string.h)
// NEEDED BY: libcore (strings.h, struct.h, variables.h, variables_internal.h)
// NEEDED BY: libcore (varptr.h, vfs.h)
// NEEDED BY: libcore (gemini.c, keyword_props.c)
// NEEDED BY: libengine (abs.h, acos.h, alias.h, append.h, ascii_fn.h, asin.h)
// NEEDED BY: libengine (ask.h, ast.h, ast_internal.h, atan2.h, ath.h, atn.h)
// NEEDED BY: libengine (auto.h, beep.h, bgi.h, bin.h, bios.h, bitcount.h)
// NEEDED BY: libengine (bload.h, brun.h, bsave.h, builtins_internal.h, call.h)
// NEEDED BY: libengine (category.h, cause.h, ceil.h, chain.h, change.h)
// NEEDED BY: libengine (chdir.h, check.h, chr.h, circle.h, clamp.h, class.h)
// NEEDED BY: libengine (clear.h, clock_num.h, clock_str.h, close.h, color.h)
// NEEDED BY: libengine (command_fn.h, common.h, comp.h, complex.h)
// NEEDED BY: libengine (complex_fn.h, cont.h, continue.h, cos.h, cosh.h, cot.h)
// NEEDED BY: libengine (create.h, cross.h, csc.h, csrlin.h, cvt.h, data.h)
// NEEDED BY: libengine (date.h, debug.h, declare.h, def.h, def_seg.h)
// NEEDED BY: libengine (def_usr.h, defdbl.h, defint.h, deflng.h, defseg.h)
// NEEDED BY: libengine (defsng.h, defstr.h, delete.h, destroy.h, det.h, dim.h)
// NEEDED BY: libengine (dispatch_internal.h, do.h, doevents.h, dot.h, draw.h)
// NEEDED BY: libengine (end.h, enter.h, enum.h, environ.h, eof_fn.h, erase.h)
// NEEDED BY: libengine (erl.h, err_fn.h, ert.h, eval.h, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, exec_internal.h)
// NEEDED BY: libengine (exit_loop.h, exp.h, external.h, fid.h, field.h)
// NEEDED BY: libengine (files.h, financial.h, fix.h, floor.h, for.h, form.h)
// NEEDED BY: libengine (fpt.h, fre.h, freefile.h, function.h, get.h, global.h)
// NEEDED BY: libengine (gosub.h, goto.h, handler.h, help.h, hex.h, image.h)
// NEEDED BY: libengine (index_fn.h, inkey.h, inp.h, input.h, input_file.h)
// NEEDED BY: libengine (instr.h, int.h, interrupt.h, introspection.h, isam.h)
// NEEDED BY: libengine (joystick.h, keyword.h, kill.h, lbound.h, lcase.h)
// NEEDED BY: libengine (left.h, len.h, lerp.h, let.h, lexer.h, line.h)
// NEEDED BY: libengine (line_input.h, linput.h, list.h, llist.h, load.h)
// NEEDED BY: libengine (loc_fn.h, lock.h, lof.h, log.h, log10.h, log2.h)
// NEEDED BY: libengine (loop.h, lpos.h, lprint.h, lset.h, ltrim.h, mag.h)
// NEEDED BY: libengine (magtape.h, map.h, margin.h, mat_input.h)
// NEEDED BY: libengine (mat_internal.h, mat_ops.h, mat_print.h, mat_read.h)
// NEEDED BY: libengine (mbf.h, merge.h, mid.h, mid_stmt.h, mkdir.h, moddir.h)
// NEEDED BY: libengine (modify.h, mouse.h, msgbox.h, mux.h, name.h, new.h)
// NEEDED BY: libengine (next.h, num.h, oct.h, on_com.h, on_error.h, on_key.h)
// NEEDED BY: libengine (on_timer.h, open.h, option.h, override.h, paint.h)
// NEEDED BY: libengine (palette.h, param.h, pause.h, pdif.h, pds_datetime.h)
// NEEDED BY: libengine (pds_sys.h, peek.h, pen.h, pi.h, picture.h, play.h)
// NEEDED BY: libengine (point_fn.h, poke.h, pos.h, print.h, print_file.h)
// NEEDED BY: libengine (pset.h, public.h, put.h, rad.h, randomize.h, read.h)
// NEEDED BY: libengine (readbit.h, redim.h, reformat.h, rem.h, remove.h)
// NEEDED BY: libengine (renum.h, resetbit.h, restore.h, resume.h, retry.h)
// NEEDED BY: libengine (return.h, right.h, rmdir.h, rnd.h, round.h, rset.h)
// NEEDED BY: libengine (rtrim.h, run.h, save.h, scope.h, screen.h, sec.h)
// NEEDED BY: libengine (seek.h, seg.h, selftest.h, session_stmts.h, setbit.h)
// NEEDED BY: libengine (sgn.h, share.h, shared.h, shell.h, shl.h, shr.h)
// NEEDED BY: libengine (shuffle.h, sin.h, sinh.h, sound.h, space.h, spc.h)
// NEEDED BY: libengine (spec_fn.h, sqr.h, stack.h, stmt.h, stmt_handlers.h)
// NEEDED BY: libengine (stop.h, str.h, str_math.h, sub.h, sub_internal.h)
// NEEDED BY: libengine (swap.h, sys.h, sys_fn.h, tab.h, tan.h, tanh.h, task.h)
// NEEDED BY: libengine (tek.h, test.h, text.h, ticks.h, time.h, time_fn.h)
// NEEDED BY: libengine (timer.h, togglebit.h, trim.h, txn.h, typ.h, type.h)
// NEEDED BY: libengine (ubound.h, ucase.h, unpack.h, ups.h, val.h)
// NEEDED BY: libengine (vbdos_controls.h, vbdos_filebox.h, vbdos_fn.h)
// NEEDED BY: libengine (vbdos_widgets.h, verify.h, verify_fn.h, view.h)
// NEEDED BY: libengine (viewport.h, vm.h, voice.h, wend.h, when.h, while.h)
// NEEDED BY: libengine (window.h, with.h, write_file.h, xlate.h, zone.h)
// NEEDED BY: libengine (alias.c, eval_builtins.c, keyword.c, override.c)
// NEEDED BY: libengine (scope.c)
// NEEDED BY: libext (arrayext.h)
// NEEDED BY: libkernel (gfx_internal.h, security.h, usb.h, vcon.h, vdev.h)
// NEEDED BY: libkernel (version.h)
// NEEDED BY: libkernel (gfx.c)
// NEEDED BY: libplatform (platform.h)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c)
// NEEDED BY: libserver (vnet.h)
// NEEDS: platform, memory
// Provides core logic and interface definitions for types within BASIC++.
//
// ---- Includes ----

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

//
// ---- Forward Declarations ----

typedef struct BppString BppString;
typedef BppString* BppStringRef;
typedef struct BppMap BppMap;
typedef struct BppTypeRegistry BppTypeRegistry;

//
// ---- Value Types ----

// universal type codes for BValue tagged union
typedef enum {
    VAL_NONE = 0,       // uninitialized state
    VAL_NUMBER,         // universal floating-point number (double)
    VAL_INTEGER,        // optimized integer (int32_t represented in double or flag)
    VAL_STRING,         // reference-counted string handle
    VAL_MAP,            // reference-counted dictionary/map
    VAL_ARRAY_REF,      // symbolic link to ArrayContext array
    VAL_FIELD_STRING,   // GW-BASIC Random Access File bound string
    VAL_COMPLEX         // first-class complex number (real + imag I)
} ValueType;

// primary tagged-union value type representing any BASIC++ expression result
typedef struct {
    ValueType type;
    union {
        double       number; // stores float/double/integer values
        struct {
            double real;
            double imag;
        } complex_val;       // stores complex numbers
        BppStringRef string; // reference to isolated string heap
        BppMap      *map;    // reference-counted map
        const char  *array_name; // name of the target array for VAL_ARRAY_REF
        struct {
            int channel;
            int offset;
            int length;
        } field_str;         // bound to a VDev file buffer
    } as;
} BValue;

// line number representation (double supports fractional lines like 10.5)
typedef double BppLineNumber;
typedef double LineNumber;

//
// ---- Opcodes ----

// opcodes for cached opcode execution mode
typedef enum {
    OP_HALT = 0,
    OP_NOP,
    OP_PUSH_CONST,
    OP_PUSH_VAR,
    OP_POP_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_CMP_EQ,
    OP_CMP_NE,
    OP_CMP_LT,
    OP_CMP_GT,
    OP_CMP_LE,
    OP_CMP_GE,
    OP_JMP,
    OP_JMP_IF_FALSE,
    OP_CALL,
    OP_RET,
    OP_PRINT,
    OP_INPUT,
    OP_COUNT
} BppOpcode;

typedef BppOpcode Opcode;

//
// ---- Structured Error Definitions ----

// category of errors in the structured error handling subsystem
typedef enum {
    ERR_CAT_NONE = 0,
    ERR_CAT_SYNTAX,     // compile and parse errors
    ERR_CAT_RUNTIME,    // VM runtime execution errors
    ERR_CAT_SYSTEM,     // device, filesystem, and OS errors
    ERR_CAT_INTERNAL    // compiler limits and internal faults
} BppErrorCategory;

typedef BppErrorCategory ErrorCategory;

// structured error block containing error code and traceback metadata
typedef struct {
    int              code;       // GW-BASIC/QBASIC compatible error code (1-255)
    BppErrorCategory category;   // category filter
    const char      *message;    // readable error message
    BppLineNumber    line;       // BASIC program line where error occurred
    int              col;        // column position
    const char      *file;       // C source file where error was raised
} BppError;

typedef BppError VMError;
typedef BppError Error;

#endif // TYPES_H

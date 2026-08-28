// FILENAME: micro_lib_metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: libkernel (types.h)
// Implements component functionality for micro_lib_metadata.h.
//
// ---- Includes ----

// FILENAME: micro_lib_metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h, stmt_reboot.c)
// NEEDED BY: libcore (clear.c, docgen.c, get.c, list.c, micro_lib_metadata.c)
// NEEDED BY: libcore (select.c, varptr.c)
// NEEDED BY: libengine (abs.c, acos.c, alias.c, and.c, angle.c, append.c)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, asin.c, ask.c)
// NEEDED BY: libengine (assign.c, atan2.c, ath.c, atn.c, auto.c, backspace.c)
// NEEDED BY: libengine (beep.c, bgi.c, bin.c, bios.c, bitcount.c, bload.c)
// NEEDED BY: libengine (brun.c, bsave.c, call.c, category.c, cause.c, ceil.c)
// NEEDED BY: libengine (chain.c, change.c, chdir.c, check.c, chr.c, circle.c)
// NEEDED BY: libengine (clamp.c, class.c, clock_num.c, clock_str.c, close.c)
// NEEDED BY: libengine (clr.c, cls.c, color.c, command_fn.c, common.c, comp.c)
// NEEDED BY: libengine (compat.c, complex.c, complex_fn.c, const.c, cont.c)
// NEEDED BY: libengine (continue.c, cos.c, cosh.c, cot.c, create.c, cross.c)
// NEEDED BY: libengine (csc.c, csrlin.c, cvt.c, data.c, date.c, debug.c)
// NEEDED BY: libengine (declare.c, def.c, def_seg.c, def_usr.c, defdbl.c)
// NEEDED BY: libengine (defint.c, deflng.c, defseg.c, defsng.c, defstr.c)
// NEEDED BY: libengine (degrees.c, delete.c, destroy.c, det.c, dim.c, do.c)
// NEEDED BY: libengine (dot.c, draw.c, echo.c, end.c, endloop.c, enter.c)
// NEEDED BY: libengine (enum.c, environ.c, eof_fn.c, eps.c, eqv.c, erase.c)
// NEEDED BY: libengine (erl.c, err_fn.c, ert.c, exchange.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, freefile.c, func_aread.c, func_dread.c)
// NEEDED BY: libengine (func_hall.c, func_http.c, func_mem.c, func_python.c)
// NEEDED BY: libengine (func_ticks.c, func_touch.c, function.c, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, handler.c, help.c, hex.c)
// NEEDED BY: libengine (hypot.c, if.c, image.c, imp.c, incr.c, index_fn.c)
// NEEDED BY: libengine (inf.c, inkey.c, inp.c, input.c, input_file.c, instr.c)
// NEEDED BY: libengine (int.c, interrupt.c, introspection.c, invoke.c, isam.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, lbound.c)
// NEEDED BY: libengine (lcase.c, left.c, len.c, lerp.c, let.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, loc_fn.c, lock.c)
// NEEDED BY: libengine (lof.c, log.c, log10.c, log2.c, loop.c, lpos.c)
// NEEDED BY: libengine (lprint.c, lset.c, ltrim.c, mag.c, magtape.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_internal.h, mat_print.c)
// NEEDED BY: libengine (mat_read.c, mat_write.c, max.c, maxnum.c, mbf.c)
// NEEDED BY: libengine (merge.c, mid.c, mid_stmt.c, min.c, mkdir.c, mod.c)
// NEEDED BY: libengine (moddir.c, modify.c, module.c, mouse.c, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, not.c, num.c, oct.c, on_com.c)
// NEEDED BY: libengine (on_error.c, on_key.c, on_timer.c, open.c, option.c)
// NEEDED BY: libengine (or.c, out.c, override.c, pack.c, page.c, paint.c)
// NEEDED BY: libengine (palette.c, param.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, point_fn.c, poke.c, pos.c, prefix.c)
// NEEDED BY: libengine (preset.c, print.c, print_file.c, pset.c, public.c)
// NEEDED BY: libengine (put.c, rad.c, radians.c, randomize.c, read.c)
// NEEDED BY: libengine (readbit.c, record.c, redim.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, rem.c, remainder.c, remove.c)
// NEEDED BY: libengine (renum.c, repeat.c, resetbit.c, restore.c, resume.c)
// NEEDED BY: libengine (retry.c, return.c, rewind.c, right.c, rmdir.c, rnd.c)
// NEEDED BY: libengine (round.c, rset.c, rtrim.c, run.c, save.c, scale.c)
// NEEDED BY: libengine (scope.c, screen.c, sec.c, seek.c, seg.c, selftest.c)
// NEEDED BY: libengine (session_stmts.c, setbit.c, sgn.c, share.c, shared.c)
// NEEDED BY: libengine (shell.c, shl.c, shr.c, shuffle.c, sin.c, sinh.c)
// NEEDED BY: libengine (sleep.c, sound.c, space.c, spc.c, spec_fn.c, sqr.c)
// NEEDED BY: libengine (stmt_dac.c, stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (stop.c, str.c, str_math.c, string.c, sub_internal.h)
// NEEDED BY: libengine (suspend.c, swap.c, sys.c, sys_fn.c, system.c, tab.c)
// NEEDED BY: libengine (tan.c, tanh.c, task.c, tek.c, test.c, text.c, ticks.c)
// NEEDED BY: libengine (time.c, time_fn.c, timer.c, togglebit.c, trim.c)
// NEEDED BY: libengine (truncate.c, try.c, txn.c, typ.c, type.c, ubound.c)
// NEEDED BY: libengine (ucase.c, unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, vdim.c, verify.c, verify_fn.c)
// NEEDED BY: libengine (version.c, view.c, viewport.c, voice.c, void.c, wait.c)
// NEEDED BY: libengine (wend.c, when.c, whenever.c, while.c, window.c, with.c)
// NEEDED BY: libengine (write_file.c, xlate.c, xor.c, zone.c)
// NEEDED BY: libkernel (security.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for micro_lib_metadata.h.
//
// ---- Includes ----

// FILENAME: micro_lib_metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h, stmt_reboot.c)
// NEEDED BY: libcore (clear.c, docgen.c, get.c, list.c, micro_lib_metadata.c)
// NEEDED BY: libcore (select.c, varptr.c)
// NEEDED BY: libengine (abs.c, acos.c, alias.c, and.c, angle.c, append.c)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, asin.c, ask.c)
// NEEDED BY: libengine (assign.c, atan2.c, ath.c, atn.c, auto.c, backspace.c)
// NEEDED BY: libengine (beep.c, bgi.c, bin.c, bios.c, bitcount.c, bload.c)
// NEEDED BY: libengine (brun.c, bsave.c, call.c, category.c, cause.c, ceil.c)
// NEEDED BY: libengine (chain.c, change.c, chdir.c, check.c, chr.c, circle.c)
// NEEDED BY: libengine (clamp.c, class.c, clock_num.c, clock_str.c, close.c)
// NEEDED BY: libengine (clr.c, cls.c, color.c, command_fn.c, common.c, comp.c)
// NEEDED BY: libengine (compat.c, complex.c, complex_fn.c, const.c, cont.c)
// NEEDED BY: libengine (continue.c, cos.c, cosh.c, cot.c, create.c, cross.c)
// NEEDED BY: libengine (csc.c, csrlin.c, cvt.c, data.c, date.c, debug.c)
// NEEDED BY: libengine (declare.c, def.c, def_seg.c, def_usr.c, defdbl.c)
// NEEDED BY: libengine (defint.c, deflng.c, defseg.c, defsng.c, defstr.c)
// NEEDED BY: libengine (degrees.c, delete.c, destroy.c, det.c, dim.c, do.c)
// NEEDED BY: libengine (dot.c, draw.c, echo.c, end.c, endloop.c, enter.c)
// NEEDED BY: libengine (enum.c, environ.c, eof_fn.c, eps.c, eqv.c, erase.c)
// NEEDED BY: libengine (erl.c, err_fn.c, ert.c, exchange.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, freefile.c, func_aread.c, func_dread.c)
// NEEDED BY: libengine (func_hall.c, func_http.c, func_mem.c, func_python.c)
// NEEDED BY: libengine (func_ticks.c, func_touch.c, function.c, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, handler.c, help.c, hex.c)
// NEEDED BY: libengine (hypot.c, if.c, image.c, imp.c, incr.c, index_fn.c)
// NEEDED BY: libengine (inf.c, inkey.c, inp.c, input.c, input_file.c, instr.c)
// NEEDED BY: libengine (int.c, interrupt.c, introspection.c, invoke.c, isam.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, lbound.c)
// NEEDED BY: libengine (lcase.c, left.c, len.c, lerp.c, let.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, loc_fn.c, lock.c)
// NEEDED BY: libengine (lof.c, log.c, log10.c, log2.c, loop.c, lpos.c)
// NEEDED BY: libengine (lprint.c, lset.c, ltrim.c, mag.c, magtape.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_internal.h, mat_print.c)
// NEEDED BY: libengine (mat_read.c, mat_write.c, max.c, maxnum.c, mbf.c)
// NEEDED BY: libengine (merge.c, mid.c, mid_stmt.c, min.c, mkdir.c, mod.c)
// NEEDED BY: libengine (moddir.c, modify.c, module.c, mouse.c, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, not.c, num.c, oct.c, on_com.c)
// NEEDED BY: libengine (on_error.c, on_key.c, on_timer.c, open.c, option.c)
// NEEDED BY: libengine (or.c, out.c, override.c, pack.c, page.c, paint.c)
// NEEDED BY: libengine (palette.c, param.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, point_fn.c, poke.c, pos.c, prefix.c)
// NEEDED BY: libengine (preset.c, print.c, print_file.c, pset.c, public.c)
// NEEDED BY: libengine (put.c, rad.c, radians.c, randomize.c, read.c)
// NEEDED BY: libengine (readbit.c, record.c, redim.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, rem.c, remainder.c, remove.c)
// NEEDED BY: libengine (renum.c, repeat.c, resetbit.c, restore.c, resume.c)
// NEEDED BY: libengine (retry.c, return.c, rewind.c, right.c, rmdir.c, rnd.c)
// NEEDED BY: libengine (round.c, rset.c, rtrim.c, run.c, save.c, scale.c)
// NEEDED BY: libengine (scope.c, screen.c, sec.c, seek.c, seg.c, selftest.c)
// NEEDED BY: libengine (session_stmts.c, setbit.c, sgn.c, share.c, shared.c)
// NEEDED BY: libengine (shell.c, shl.c, shr.c, shuffle.c, sin.c, sinh.c)
// NEEDED BY: libengine (sleep.c, sound.c, space.c, spc.c, spec_fn.c, sqr.c)
// NEEDED BY: libengine (stmt_dac.c, stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (stop.c, str.c, str_math.c, string.c, sub_internal.h)
// NEEDED BY: libengine (suspend.c, swap.c, sys.c, sys_fn.c, system.c, tab.c)
// NEEDED BY: libengine (tan.c, tanh.c, task.c, tek.c, test.c, text.c, ticks.c)
// NEEDED BY: libengine (time.c, time_fn.c, timer.c, togglebit.c, trim.c)
// NEEDED BY: libengine (truncate.c, try.c, txn.c, typ.c, type.c, ubound.c)
// NEEDED BY: libengine (ucase.c, unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, vdim.c, verify.c, verify_fn.c)
// NEEDED BY: libengine (version.c, view.c, viewport.c, voice.c, void.c, wait.c)
// NEEDED BY: libengine (wend.c, when.c, whenever.c, while.c, window.c, with.c)
// NEEDED BY: libengine (write_file.c, xlate.c, xor.c, zone.c)
// NEEDED BY: libkernel (security.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for micro_lib_metadata.h.
//
// ---- Includes ----

// FILENAME: micro_lib_metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (clear.c, docgen.c, get.c, list.c, micro_lib_metadata.c)
// NEEDED BY: libcore (select.c, varptr.c)
// NEEDED BY: libengine (abs.c, acos.c, alias.c, and.c, angle.c, append.c)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, asin.c, ask.c)
// NEEDED BY: libengine (assign.c, atan2.c, ath.c, atn.c, auto.c, backspace.c)
// NEEDED BY: libengine (beep.c, bgi.c, bin.c, bios.c, bitcount.c, bload.c)
// NEEDED BY: libengine (brun.c, bsave.c, call.c, category.c, cause.c, ceil.c)
// NEEDED BY: libengine (chain.c, change.c, chdir.c, check.c, chr.c, circle.c)
// NEEDED BY: libengine (clamp.c, class.c, clock_num.c, clock_str.c, close.c)
// NEEDED BY: libengine (clr.c, cls.c, color.c, command_fn.c, common.c, comp.c)
// NEEDED BY: libengine (compat.c, complex.c, complex_fn.c, const.c, cont.c)
// NEEDED BY: libengine (continue.c, cos.c, cosh.c, cot.c, create.c, cross.c)
// NEEDED BY: libengine (csc.c, csrlin.c, cvt.c, data.c, date.c, debug.c)
// NEEDED BY: libengine (declare.c, def.c, def_seg.c, def_usr.c, defdbl.c)
// NEEDED BY: libengine (defint.c, deflng.c, defseg.c, defsng.c, defstr.c)
// NEEDED BY: libengine (degrees.c, delete.c, destroy.c, det.c, dim.c, do.c)
// NEEDED BY: libengine (dot.c, draw.c, echo.c, end.c, endloop.c, enter.c)
// NEEDED BY: libengine (enum.c, environ.c, eof_fn.c, eps.c, eqv.c, erase.c)
// NEEDED BY: libengine (erl.c, err_fn.c, ert.c, exchange.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, freefile.c, function.c, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, handler.c, help.c, hex.c)
// NEEDED BY: libengine (hypot.c, if.c, image.c, imp.c, incr.c, index_fn.c)
// NEEDED BY: libengine (inf.c, inkey.c, inp.c, input.c, input_file.c, instr.c)
// NEEDED BY: libengine (int.c, interrupt.c, introspection.c, invoke.c, isam.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, lbound.c)
// NEEDED BY: libengine (lcase.c, left.c, len.c, lerp.c, let.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, loc_fn.c, lock.c)
// NEEDED BY: libengine (lof.c, log.c, log10.c, log2.c, loop.c, lpos.c)
// NEEDED BY: libengine (lprint.c, lset.c, ltrim.c, mag.c, magtape.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_internal.h, mat_print.c)
// NEEDED BY: libengine (mat_read.c, mat_write.c, max.c, maxnum.c, mbf.c)
// NEEDED BY: libengine (merge.c, mid.c, mid_stmt.c, min.c, mkdir.c, mod.c)
// NEEDED BY: libengine (moddir.c, modify.c, module.c, mouse.c, mux.c, name.c)
// NEEDED BY: libengine (new.c, next.c, not.c, num.c, oct.c, on_com.c)
// NEEDED BY: libengine (on_error.c, on_key.c, on_timer.c, open.c, option.c)
// NEEDED BY: libengine (or.c, out.c, override.c, pack.c, page.c, paint.c)
// NEEDED BY: libengine (palette.c, param.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, point_fn.c, poke.c, pos.c, prefix.c)
// NEEDED BY: libengine (preset.c, print.c, print_file.c, pset.c, public.c)
// NEEDED BY: libengine (put.c, rad.c, radians.c, randomize.c, read.c)
// NEEDED BY: libengine (readbit.c, record.c, redim.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, rem.c, remainder.c, remove.c)
// NEEDED BY: libengine (renum.c, repeat.c, resetbit.c, restore.c, resume.c)
// NEEDED BY: libengine (retry.c, return.c, rewind.c, right.c, rmdir.c, rnd.c)
// NEEDED BY: libengine (round.c, rset.c, rtrim.c, run.c, save.c, scale.c)
// NEEDED BY: libengine (scope.c, screen.c, sec.c, seek.c, seg.c, selftest.c)
// NEEDED BY: libengine (session_stmts.c, setbit.c, sgn.c, share.c, shared.c)
// NEEDED BY: libengine (shell.c, shl.c, shr.c, shuffle.c, sin.c, sinh.c)
// NEEDED BY: libengine (sleep.c, sound.c, space.c, spc.c, spec_fn.c, sqr.c)
// NEEDED BY: libengine (stmt_home.c, stmt_plot.c, stop.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, sub_internal.h, suspend.c, swap.c, sys.c)
// NEEDED BY: libengine (sys_fn.c, system.c, tab.c, tan.c, tanh.c, task.c)
// NEEDED BY: libengine (tek.c, test.c, text.c, ticks.c, time.c, time_fn.c)
// NEEDED BY: libengine (timer.c, togglebit.c, trim.c, truncate.c, try.c, txn.c)
// NEEDED BY: libengine (typ.c, type.c, ubound.c, ucase.c, unless.c, unpack.c)
// NEEDED BY: libengine (until.c, ups.c, val.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_filebox.c, vbdos_fn.c, vbdos_widgets.c, vdim.c)
// NEEDED BY: libengine (verify.c, verify_fn.c, version.c, view.c, viewport.c)
// NEEDED BY: libengine (voice.c, void.c, wait.c, wend.c, when.c, whenever.c)
// NEEDED BY: libengine (while.c, window.c, with.c, write_file.c, xlate.c)
// NEEDED BY: libengine (xor.c, zone.c)
// NEEDED BY: libkernel (security.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for micro_lib_metadata.h.
//
// ---- Includes ----

// FILENAME: micro_lib_metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (clear.c, docgen.c, get.c, list.c, micro_lib_metadata.c)
// NEEDED BY: libcore (select.c, varptr.c)
// NEEDED BY: libengine (mat_internal.h, reformat_internal.h, sub_internal.h)
// NEEDED BY: libengine (abs.c, acos.c, alias.c, and.c, angle.c, append.c)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, asin.c, ask.c)
// NEEDED BY: libengine (assign.c, atan2.c, ath.c, atn.c, auto.c, backspace.c)
// NEEDED BY: libengine (beep.c, bgi.c, bin.c, bios.c, bitcount.c, bload.c)
// NEEDED BY: libengine (brun.c, bsave.c, call.c, category.c, cause.c, ceil.c)
// NEEDED BY: libengine (chain.c, change.c, chdir.c, check.c, chr.c, circle.c)
// NEEDED BY: libengine (clamp.c, class.c, clock_num.c, clock_str.c, close.c)
// NEEDED BY: libengine (clr.c, cls.c, color.c, command_fn.c, common.c, comp.c)
// NEEDED BY: libengine (compat.c, complex.c, complex_fn.c, const.c, cont.c)
// NEEDED BY: libengine (continue.c, cos.c, cosh.c, cot.c, create.c, cross.c)
// NEEDED BY: libengine (csc.c, csrlin.c, cvt.c, data.c, date.c, debug.c)
// NEEDED BY: libengine (declare.c, def.c, def_seg.c, def_usr.c, defdbl.c)
// NEEDED BY: libengine (defint.c, deflng.c, defseg.c, defsng.c, defstr.c)
// NEEDED BY: libengine (degrees.c, delete.c, destroy.c, det.c, dim.c, do.c)
// NEEDED BY: libengine (dot.c, draw.c, echo.c, end.c, endloop.c, enter.c)
// NEEDED BY: libengine (enum.c, environ.c, eof_fn.c, eps.c, eqv.c, erase.c)
// NEEDED BY: libengine (erl.c, err_fn.c, ert.c, exchange.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, freefile.c, function.c, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, handler.c, help.c, hex.c)
// NEEDED BY: libengine (hypot.c, if.c, image.c, imp.c, incr.c, index_fn.c)
// NEEDED BY: libengine (inf.c, inkey.c, inp.c, input.c, input_file.c, instr.c)
// NEEDED BY: libengine (int.c, interrupt.c, introspection.c, invoke.c, isam.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, lbound.c)
// NEEDED BY: libengine (lcase.c, left.c, len.c, lerp.c, let.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, loc_fn.c, lock.c)
// NEEDED BY: libengine (lof.c, log.c, log10.c, log2.c, loop.c, lpos.c)
// NEEDED BY: libengine (lprint.c, lset.c, ltrim.c, mag.c, magtape.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_print.c, mat_read.c)
// NEEDED BY: libengine (mat_write.c, max.c, maxnum.c, mbf.c, merge.c, mid.c)
// NEEDED BY: libengine (mid_stmt.c, min.c, mkdir.c, mod.c, moddir.c, modify.c)
// NEEDED BY: libengine (module.c, mouse.c, mux.c, name.c, new.c, next.c, not.c)
// NEEDED BY: libengine (num.c, oct.c, on_com.c, on_error.c, on_key.c)
// NEEDED BY: libengine (on_timer.c, open.c, option.c, or.c, out.c, override.c)
// NEEDED BY: libengine (pack.c, page.c, paint.c, palette.c, param.c, pause.c)
// NEEDED BY: libengine (pdif.c, pds_datetime.c, pds_sys.c, peek.c, pen.c)
// NEEDED BY: libengine (perform.c, pi.c, pick.c, picture.c, play.c, point_fn.c)
// NEEDED BY: libengine (poke.c, pos.c, prefix.c, preset.c, print.c)
// NEEDED BY: libengine (print_file.c, pset.c, public.c, put.c, rad.c)
// NEEDED BY: libengine (radians.c, randomize.c, read.c, readbit.c, record.c)
// NEEDED BY: libengine (redim.c, reformat.c, rem.c, remainder.c, remove.c)
// NEEDED BY: libengine (renum.c, repeat.c, resetbit.c, restore.c, resume.c)
// NEEDED BY: libengine (retry.c, return.c, rewind.c, right.c, rmdir.c, rnd.c)
// NEEDED BY: libengine (round.c, rset.c, rtrim.c, run.c, save.c, scale.c)
// NEEDED BY: libengine (scope.c, screen.c, sec.c, seek.c, seg.c, selftest.c)
// NEEDED BY: libengine (session_stmts.c, setbit.c, sgn.c, share.c, shared.c)
// NEEDED BY: libengine (shell.c, shl.c, shr.c, shuffle.c, sin.c, sinh.c)
// NEEDED BY: libengine (sleep.c, sound.c, space.c, spc.c, spec_fn.c, sqr.c)
// NEEDED BY: libengine (stmt_home.c, stmt_plot.c, stop.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, suspend.c, swap.c, sys.c, sys_fn.c, system.c)
// NEEDED BY: libengine (tab.c, tan.c, tanh.c, task.c, tek.c, test.c, text.c)
// NEEDED BY: libengine (ticks.c, time.c, time_fn.c, timer.c, togglebit.c)
// NEEDED BY: libengine (trim.c, truncate.c, try.c, txn.c, typ.c, type.c)
// NEEDED BY: libengine (ubound.c, ucase.c, unless.c, unpack.c, until.c, ups.c)
// NEEDED BY: libengine (val.c, vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, vdim.c, verify.c, verify_fn.c)
// NEEDED BY: libengine (version.c, view.c, viewport.c, voice.c, void.c, wait.c)
// NEEDED BY: libengine (wend.c, when.c, whenever.c, while.c, window.c, with.c)
// NEEDED BY: libengine (write_file.c, xlate.c, xor.c, zone.c)
// NEEDED BY: libkernel (security.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for micro_lib_metadata within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_MICRO_LIB_METADATA_H
#define RUNTIME_MICRO_LIB_METADATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types/types.h"

//
// ---- Constants & Bitmasks ----

#define MICROLIB_INITIAL_CAPACITY 1024

#define DIALECT_NONE        0x00000000U
#define DIALECT_GWBASIC     0x00000001U
#define DIALECT_QBASIC      0x00000002U
#define DIALECT_SUPERBASIC  0x00000004U
#define DIALECT_ECMA116     0x00000008U
#define DIALECT_BASICPP     0x00000010U
#define DIALECT_ALL         0xFFFFFFFFU

//
// ---- Types & Enumerations ----

typedef enum {
    FEATURE_STATEMENT = 0,
    FEATURE_FUNCTION,
    FEATURE_COMMAND,
    FEATURE_MODULE,
    FEATURE_OPERATOR,
    FEATURE_DEVICE
} FeatureType;

typedef enum {
    SAFETY_PURE = 0,       // no side-effects, deterministic (e.g. SIN, COS)
    SAFETY_SAFE,           // safe managed state (e.g. LEFT$, MID$)
    SAFETY_IO,             // file or console I/O (e.g. PRINT, INPUT)
    SAFETY_SYSTEM,         // system resources (e.g. PEEK, POKE, SHELL)
    SAFETY_UNSAFE          // privileged hardware access
} SafetyLevel;

typedef enum {
    SUBSYSTEM_BOOT = 0,
    SUBSYSTEM_PLATFORM,
    SUBSYSTEM_KERNEL,
    SUBSYSTEM_ENGINE,
    SUBSYSTEM_HARDWARE,
    SUBSYSTEM_SERVER,
    SUBSYSTEM_SCRIPT,
    SUBSYSTEM_CORE,
    SUBSYSTEM_FLEX,
    SUBSYSTEM_STANDARD,
    SUBSYSTEM_ADVANCED,
    SUBSYSTEM_EXT
} FunctionalSubsystem;

// authoritative feature metadata and runtime dispatch descriptor
typedef struct MicroLibraryMetadata {
    const char *name;              // canonical feature name (e.g. "VARPTR", "PRINT")
    const char *category;          // functional category (e.g. "Variables & Memory")
    const char *syntax;            // formal syntax signature
    const char *help_text;         // concise human-readable description
    const char *error_codes;       // potential generated error codes
    uint32_t    dialect_mask;      // bitmask of dialect origins
    uint32_t    subsystem;         // 12-library functional spectrum enum
    SafetyLevel safety;            // sandbox and execution safety classification
    FeatureType type;              // feature classification (statement, function, etc.)
    const char *examples;          // ready-to-run code example string
    void       *dispatch_fn;       // statement or function evaluation handler pointer
} MicroLibraryMetadata;

typedef MicroLibraryMetadata MicroLibMetadata;

//
// ---- Registration & Query API ----

void microlib_init(void);
void microlib_shutdown(void);
int  microlib_register(const MicroLibraryMetadata *meta);
int  microlib_override(const char *name, const MicroLibraryMetadata *meta);
const MicroLibraryMetadata *microlib_find(const char *name);
const MicroLibraryMetadata *microlib_get(int index);
int  microlib_count(void);
int  microlib_query_category(const char *category, const MicroLibraryMetadata **results, int max_results);
char *microlib_export_json(void);

//
// ---- Declarative Compile-Time Macro Helpers ----

#define REGISTER_STATEMENT(meta_ptr, handler_fn) do { \
    (meta_ptr)->type = FEATURE_STATEMENT; \
    (meta_ptr)->dispatch_fn = (void *)(handler_fn); \
    microlib_register(meta_ptr); \
} while(0)

#define REGISTER_FUNCTION(meta_ptr, handler_fn) do { \
    (meta_ptr)->type = FEATURE_FUNCTION; \
    (meta_ptr)->dispatch_fn = (void *)(handler_fn); \
    microlib_register(meta_ptr); \
} while(0)

#endif // RUNTIME_MICRO_LIB_METADATA_H

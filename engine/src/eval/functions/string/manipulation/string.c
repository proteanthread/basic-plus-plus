// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libext, libkernel, libplatform, libserver, libstandard
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// Implements component functionality for string.c.
//
// ---- Includes ----

// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (headless.c, stmt_reboot.c)
// NEEDED BY: libcore (array_sort.c, bppc_internal.h, bppc_transpile.c, clear.c)
// NEEDED BY: libcore (detok.c, esp32_hal.c, esp32_regs.c, funcreg.c, gears.c)
// NEEDED BY: libcore (get.c, interop_com.c, interop_error.c, interop_handle.c)
// NEEDED BY: libcore (interop_ipc.c, interop_jsonrpc.c, interop_marshal.c)
// NEEDED BY: libcore (iot_main.c, iot_sensors.c, list.c, map_serialize.c)
// NEEDED BY: libcore (mem_arena.c, micro_lib_metadata.c, nanox.c, num_format.c)
// NEEDED BY: libcore (print_using.c, select.c, session.c, spec.c, state.c)
// NEEDED BY: libcore (string_ext.c, strings.c, texobj.c, trans_internal.h)
// NEEDED BY: libcore (using_engine.c, variables_internal.h, varptr.c)
// NEEDED BY: libcore (vdev_core.c, x11.c, zbuffer.c, zgl.h, zmath.c)
// NEEDED BY: libengine (abs.c, acos.c, angle.c, append.c, array_ext.c)
// NEEDED BY: libengine (arrayfill.c, ascii_fn.c, asin.c, ask.c, assign.c)
// NEEDED BY: libengine (atan2.c, ath.c, atn.c, auto.c, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bin.c, bios.c, bload.c, brun.c, bsave.c, call.c)
// NEEDED BY: libengine (category.c, cause.c, ceil.c, chain.c, change.c)
// NEEDED BY: libengine (chdir.c, check.c, chr.c, circle.c, clamp.c, class.c)
// NEEDED BY: libengine (clock_num.c, clock_str.c, close.c, clr.c, cls.c)
// NEEDED BY: libengine (color.c, command_fn.c, common.c, comp.c, compat.c)
// NEEDED BY: libengine (complex.c, complex_fn.c, const.c, cont.c, continue.c)
// NEEDED BY: libengine (cos.c, cosh.c, cot.c, create.c, cross.c, csc.c)
// NEEDED BY: libengine (csrlin.c, cvt.c, data.c, date.c, debug.c, declare.c)
// NEEDED BY: libengine (def.c, def_seg.c, def_usr.c, defdbl.c, defint.c)
// NEEDED BY: libengine (deflng.c, defseg.c, defsng.c, defstr.c, degrees.c)
// NEEDED BY: libengine (delete.c, det.c, dim.c, do.c, doevents.c, dot.c)
// NEEDED BY: libengine (draw.c, echo.c, end.c, endloop.c, enter.c, enum.c)
// NEEDED BY: libengine (environ.c, eps.c, erase.c, erl.c, err_fn.c, ert.c)
// NEEDED BY: libengine (exchange.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exec_interrupt.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, func_http.c, func_python.c, func_ticks.c)
// NEEDED BY: libengine (function.c, global.c, goodbye.c, gosub.c, goto.c)
// NEEDED BY: libengine (handler.c, hex.c, hypot.c, if.c, image.c, incr.c)
// NEEDED BY: libengine (index_fn.c, inf.c, inkey.c, inp.c, input.c)
// NEEDED BY: libengine (input_file.c, instr.c, int.c, interrupt.c, invoke.c)
// NEEDED BY: libengine (isam.c, joystick.c, key.c, kill.c, lbound.c, lcase.c)
// NEEDED BY: libengine (left.c, len.c, lerp.c, let.c, line.c, line_input.c)
// NEEDED BY: libengine (linput.c, llist.c, load.c, lock.c, log.c, log10.c)
// NEEDED BY: libengine (log2.c, loop.c, lpos.c, lprint.c, lset.c, ltrim.c)
// NEEDED BY: libengine (mag.c, magtape.c, map.c, margin.c, mat_input.c)
// NEEDED BY: libengine (mat_internal.h, mat_print.c, mat_read.c, max.c)
// NEEDED BY: libengine (maxnum.c, mbf.c, merge.c, mid.c, mid_stmt.c, min.c)
// NEEDED BY: libengine (mkdir.c, mod.c, moddir.c, modify.c, module.c, mouse.c)
// NEEDED BY: libengine (msgbox.c, mux.c, name.c, new.c, next.c, num.c, oct.c)
// NEEDED BY: libengine (on_com.c, on_error.c, on_key.c, on_timer.c, open.c)
// NEEDED BY: libengine (option.c, out.c, pack.c, page.c, paint.c, palette.c)
// NEEDED BY: libengine (param.c, parser.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, poke.c, pos.c, prefix.c, preset.c)
// NEEDED BY: libengine (print.c, print_file.c, pset.c, public.c, put.c, rad.c)
// NEEDED BY: libengine (radians.c, randomize.c, read.c, record.c, redim.c)
// NEEDED BY: libengine (reformat.c, reformat_internal.h, rem.c, remainder.c)
// NEEDED BY: libengine (renum.c, repeat.c, restore.c, resume.c, retry.c)
// NEEDED BY: libengine (return.c, rewind.c, right.c, rmdir.c, rnd.c, round.c)
// NEEDED BY: libengine (rset.c, rtrim.c, run.c, save.c, scale.c, screen.c)
// NEEDED BY: libengine (sec.c, seek.c, seg.c, session_stmts.c, sgn.c, share.c)
// NEEDED BY: libengine (shared.c, shell.c, shuffle.c, sin.c, sinh.c, sleep.c)
// NEEDED BY: libengine (sound.c, space.c, spc.c, spec_fn.c, sqr.c, stmt_dac.c)
// NEEDED BY: libengine (stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (stop.c, str.c, str_math.c, string_fn.c, sub_internal.h)
// NEEDED BY: libengine (suspend.c, swap.c, sys.c, sys_fn.c, system.c, tab.c)
// NEEDED BY: libengine (tan.c, tanh.c, task.c, tek.c, test.c, text.c, ticks.c)
// NEEDED BY: libengine (time.c, time_fn.c, timer.c, trim.c, truncate.c, try.c)
// NEEDED BY: libengine (txn.c, type.c, ubound.c, ucase.c, unless.c, unpack.c)
// NEEDED BY: libengine (until.c, ups.c, val.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_filebox.c, vbdos_fn.c, vbdos_widgets.c, verify.c)
// NEEDED BY: libengine (verify_fn.c, version.c, view.c, viewport.c, voice.c)
// NEEDED BY: libengine (wait.c, wend.c, when.c, whenever.c, while.c, window.c)
// NEEDED BY: libengine (with.c, write_file.c, xlate.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (aalib.c, gfx_tui.c, security.c, vdev_esp32.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (iot_net.c)
// NEEDED BY: libstandard (standalone_runner.h)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// Implements component functionality for string.c.
//
// ---- Includes ----

// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (headless.c, stmt_reboot.c)
// NEEDED BY: libcore (array_sort.c, bppc_internal.h, bppc_transpile.c, clear.c)
// NEEDED BY: libcore (detok.c, esp32_hal.c, esp32_regs.c, funcreg.c, gears.c)
// NEEDED BY: libcore (get.c, interop_com.c, interop_error.c, interop_handle.c)
// NEEDED BY: libcore (interop_ipc.c, interop_jsonrpc.c, interop_marshal.c)
// NEEDED BY: libcore (iot_main.c, iot_sensors.c, list.c, map_serialize.c)
// NEEDED BY: libcore (mem_arena.c, micro_lib_metadata.c, nanox.c, num_format.c)
// NEEDED BY: libcore (print_using.c, select.c, session.c, spec.c, state.c)
// NEEDED BY: libcore (string_ext.c, strings.c, texobj.c, trans_internal.h)
// NEEDED BY: libcore (using_engine.c, variables_internal.h, varptr.c)
// NEEDED BY: libcore (vdev_core.c, x11.c, zbuffer.c, zgl.h, zmath.c)
// NEEDED BY: libengine (abs.c, acos.c, angle.c, append.c, array_ext.c)
// NEEDED BY: libengine (arrayfill.c, ascii_fn.c, asin.c, ask.c, assign.c)
// NEEDED BY: libengine (atan2.c, ath.c, atn.c, auto.c, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bin.c, bios.c, bload.c, brun.c, bsave.c, call.c)
// NEEDED BY: libengine (category.c, cause.c, ceil.c, chain.c, change.c)
// NEEDED BY: libengine (chdir.c, check.c, chr.c, circle.c, clamp.c, class.c)
// NEEDED BY: libengine (clock_num.c, clock_str.c, close.c, clr.c, cls.c)
// NEEDED BY: libengine (color.c, command_fn.c, common.c, comp.c, compat.c)
// NEEDED BY: libengine (complex.c, complex_fn.c, const.c, cont.c, continue.c)
// NEEDED BY: libengine (cos.c, cosh.c, cot.c, create.c, cross.c, csc.c)
// NEEDED BY: libengine (csrlin.c, cvt.c, data.c, date.c, debug.c, declare.c)
// NEEDED BY: libengine (def.c, def_seg.c, def_usr.c, defdbl.c, defint.c)
// NEEDED BY: libengine (deflng.c, defseg.c, defsng.c, defstr.c, degrees.c)
// NEEDED BY: libengine (delete.c, det.c, dim.c, do.c, doevents.c, dot.c)
// NEEDED BY: libengine (draw.c, echo.c, end.c, endloop.c, enter.c, enum.c)
// NEEDED BY: libengine (environ.c, eps.c, erase.c, erl.c, err_fn.c, ert.c)
// NEEDED BY: libengine (exchange.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exec_interrupt.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, func_http.c, func_python.c, func_ticks.c)
// NEEDED BY: libengine (function.c, global.c, goodbye.c, gosub.c, goto.c)
// NEEDED BY: libengine (handler.c, hex.c, hypot.c, if.c, image.c, incr.c)
// NEEDED BY: libengine (index_fn.c, inf.c, inkey.c, inp.c, input.c)
// NEEDED BY: libengine (input_file.c, instr.c, int.c, interrupt.c, invoke.c)
// NEEDED BY: libengine (isam.c, joystick.c, key.c, kill.c, lbound.c, lcase.c)
// NEEDED BY: libengine (left.c, len.c, lerp.c, let.c, line.c, line_input.c)
// NEEDED BY: libengine (linput.c, llist.c, load.c, lock.c, log.c, log10.c)
// NEEDED BY: libengine (log2.c, loop.c, lpos.c, lprint.c, lset.c, ltrim.c)
// NEEDED BY: libengine (mag.c, magtape.c, map.c, margin.c, mat_input.c)
// NEEDED BY: libengine (mat_internal.h, mat_print.c, mat_read.c, max.c)
// NEEDED BY: libengine (maxnum.c, mbf.c, merge.c, mid.c, mid_stmt.c, min.c)
// NEEDED BY: libengine (mkdir.c, mod.c, moddir.c, modify.c, module.c, mouse.c)
// NEEDED BY: libengine (msgbox.c, mux.c, name.c, new.c, next.c, num.c, oct.c)
// NEEDED BY: libengine (on_com.c, on_error.c, on_key.c, on_timer.c, open.c)
// NEEDED BY: libengine (option.c, out.c, pack.c, page.c, paint.c, palette.c)
// NEEDED BY: libengine (param.c, parser.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, poke.c, pos.c, prefix.c, preset.c)
// NEEDED BY: libengine (print.c, print_file.c, pset.c, public.c, put.c, rad.c)
// NEEDED BY: libengine (radians.c, randomize.c, read.c, record.c, redim.c)
// NEEDED BY: libengine (reformat.c, reformat_internal.h, rem.c, remainder.c)
// NEEDED BY: libengine (renum.c, repeat.c, restore.c, resume.c, retry.c)
// NEEDED BY: libengine (return.c, rewind.c, right.c, rmdir.c, rnd.c, round.c)
// NEEDED BY: libengine (rset.c, rtrim.c, run.c, save.c, scale.c, screen.c)
// NEEDED BY: libengine (sec.c, seek.c, seg.c, session_stmts.c, sgn.c, share.c)
// NEEDED BY: libengine (shared.c, shell.c, shuffle.c, sin.c, sinh.c, sleep.c)
// NEEDED BY: libengine (sound.c, space.c, spc.c, spec_fn.c, sqr.c, stmt_dac.c)
// NEEDED BY: libengine (stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c, stmt_home.c)
// NEEDED BY: libengine (stmt_i2c.c, stmt_mqtt.c, stmt_neopixel.c)
// NEEDED BY: libengine (stmt_pinmode.c, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (stop.c, str.c, str_math.c, string_fn.c, sub_internal.h)
// NEEDED BY: libengine (suspend.c, swap.c, sys.c, sys_fn.c, system.c, tab.c)
// NEEDED BY: libengine (tan.c, tanh.c, task.c, tek.c, test.c, text.c, ticks.c)
// NEEDED BY: libengine (time.c, time_fn.c, timer.c, trim.c, truncate.c, try.c)
// NEEDED BY: libengine (txn.c, type.c, ubound.c, ucase.c, unless.c, unpack.c)
// NEEDED BY: libengine (until.c, ups.c, val.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_filebox.c, vbdos_fn.c, vbdos_widgets.c, verify.c)
// NEEDED BY: libengine (verify_fn.c, version.c, view.c, viewport.c, voice.c)
// NEEDED BY: libengine (wait.c, wend.c, when.c, whenever.c, while.c, window.c)
// NEEDED BY: libengine (with.c, write_file.c, xlate.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (aalib.c, gfx_tui.c, security.c, vdev_esp32.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (iot_net.c)
// NEEDED BY: libstandard (standalone_runner.h)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// Implements component functionality for string.c.
//
// ---- Includes ----

// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (headless.c)
// NEEDED BY: libcore (array_sort.c, bppc_internal.h, bppc_transpile.c, clear.c)
// NEEDED BY: libcore (detok.c, funcreg.c, gears.c, get.c, interop_com.c)
// NEEDED BY: libcore (interop_error.c, interop_handle.c, interop_ipc.c)
// NEEDED BY: libcore (interop_jsonrpc.c, interop_marshal.c, list.c)
// NEEDED BY: libcore (map_serialize.c, mem_arena.c, micro_lib_metadata.c)
// NEEDED BY: libcore (nanox.c, num_format.c, print_using.c, select.c)
// NEEDED BY: libcore (session.c, spec.c, state.c, string_ext.c, strings.c)
// NEEDED BY: libcore (texobj.c, trans_internal.h, using_engine.c)
// NEEDED BY: libcore (variables_internal.h, varptr.c, vdev_core.c, x11.c)
// NEEDED BY: libcore (zbuffer.c, zgl.h, zmath.c)
// NEEDED BY: libengine (abs.c, acos.c, angle.c, append.c, array_ext.c)
// NEEDED BY: libengine (arrayfill.c, ascii_fn.c, asin.c, ask.c, assign.c)
// NEEDED BY: libengine (atan2.c, ath.c, atn.c, auto.c, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bin.c, bios.c, bload.c, brun.c, bsave.c, call.c)
// NEEDED BY: libengine (category.c, cause.c, ceil.c, chain.c, change.c)
// NEEDED BY: libengine (chdir.c, check.c, chr.c, circle.c, clamp.c, class.c)
// NEEDED BY: libengine (clock_num.c, clock_str.c, close.c, clr.c, cls.c)
// NEEDED BY: libengine (color.c, command_fn.c, common.c, comp.c, compat.c)
// NEEDED BY: libengine (complex.c, complex_fn.c, const.c, cont.c, continue.c)
// NEEDED BY: libengine (cos.c, cosh.c, cot.c, create.c, cross.c, csc.c)
// NEEDED BY: libengine (csrlin.c, cvt.c, data.c, date.c, debug.c, declare.c)
// NEEDED BY: libengine (def.c, def_seg.c, def_usr.c, defdbl.c, defint.c)
// NEEDED BY: libengine (deflng.c, defseg.c, defsng.c, defstr.c, degrees.c)
// NEEDED BY: libengine (delete.c, det.c, dim.c, do.c, doevents.c, dot.c)
// NEEDED BY: libengine (draw.c, echo.c, end.c, endloop.c, enter.c, enum.c)
// NEEDED BY: libengine (environ.c, eps.c, erase.c, erl.c, err_fn.c, ert.c)
// NEEDED BY: libengine (exchange.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exec_interrupt.c, exit_loop.c, exp.c)
// NEEDED BY: libengine (extend.c, external.c, fid.c, field.c, files.c)
// NEEDED BY: libengine (financial.c, find.c, fix.c, floor.c, for.c, form.c)
// NEEDED BY: libengine (fpt.c, fre.c, function.c, global.c, goodbye.c, gosub.c)
// NEEDED BY: libengine (goto.c, handler.c, hex.c, hypot.c, if.c, image.c)
// NEEDED BY: libengine (incr.c, index_fn.c, inf.c, inkey.c, inp.c, input.c)
// NEEDED BY: libengine (input_file.c, instr.c, int.c, interrupt.c, invoke.c)
// NEEDED BY: libengine (isam.c, joystick.c, key.c, kill.c, lbound.c, lcase.c)
// NEEDED BY: libengine (left.c, len.c, lerp.c, let.c, line.c, line_input.c)
// NEEDED BY: libengine (linput.c, llist.c, load.c, lock.c, log.c, log10.c)
// NEEDED BY: libengine (log2.c, loop.c, lpos.c, lprint.c, lset.c, ltrim.c)
// NEEDED BY: libengine (mag.c, magtape.c, map.c, margin.c, mat_input.c)
// NEEDED BY: libengine (mat_internal.h, mat_print.c, mat_read.c, max.c)
// NEEDED BY: libengine (maxnum.c, mbf.c, merge.c, mid.c, mid_stmt.c, min.c)
// NEEDED BY: libengine (mkdir.c, mod.c, moddir.c, modify.c, module.c, mouse.c)
// NEEDED BY: libengine (msgbox.c, mux.c, name.c, new.c, next.c, num.c, oct.c)
// NEEDED BY: libengine (on_com.c, on_error.c, on_key.c, on_timer.c, open.c)
// NEEDED BY: libengine (option.c, out.c, pack.c, page.c, paint.c, palette.c)
// NEEDED BY: libengine (param.c, parser.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, poke.c, pos.c, prefix.c, preset.c)
// NEEDED BY: libengine (print.c, print_file.c, pset.c, public.c, put.c, rad.c)
// NEEDED BY: libengine (radians.c, randomize.c, read.c, record.c, redim.c)
// NEEDED BY: libengine (reformat.c, reformat_internal.h, rem.c, remainder.c)
// NEEDED BY: libengine (renum.c, repeat.c, restore.c, resume.c, retry.c)
// NEEDED BY: libengine (return.c, rewind.c, right.c, rmdir.c, rnd.c, round.c)
// NEEDED BY: libengine (rset.c, rtrim.c, run.c, save.c, scale.c, screen.c)
// NEEDED BY: libengine (sec.c, seek.c, seg.c, session_stmts.c, sgn.c, share.c)
// NEEDED BY: libengine (shared.c, shell.c, shuffle.c, sin.c, sinh.c, sleep.c)
// NEEDED BY: libengine (sound.c, space.c, spc.c, spec_fn.c, sqr.c, stmt_home.c)
// NEEDED BY: libengine (stmt_plot.c, stop.c, str.c, str_math.c, string_fn.c)
// NEEDED BY: libengine (sub_internal.h, suspend.c, swap.c, sys.c, sys_fn.c)
// NEEDED BY: libengine (system.c, tab.c, tan.c, tanh.c, task.c, tek.c, test.c)
// NEEDED BY: libengine (text.c, ticks.c, time.c, time_fn.c, timer.c, trim.c)
// NEEDED BY: libengine (truncate.c, try.c, txn.c, type.c, ubound.c, ucase.c)
// NEEDED BY: libengine (unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify.c, verify_fn.c, version.c)
// NEEDED BY: libengine (view.c, viewport.c, voice.c, wait.c, wend.c, when.c)
// NEEDED BY: libengine (whenever.c, while.c, window.c, with.c, write_file.c)
// NEEDED BY: libengine (xlate.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (aalib.c, gfx_tui.c, security.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libstandard (standalone_runner.h)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// Implements component functionality for string.c.
//
// ---- Includes ----

// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (headless.c)
// NEEDED BY: libcore (bppc_internal.h, trans_internal.h, variables_internal.h)
// NEEDED BY: libcore (zgl.h)
// NEEDED BY: libcore (array_sort.c, bppc_transpile.c, clear.c, detok.c)
// NEEDED BY: libcore (funcreg.c, gears.c, get.c, interop_com.c)
// NEEDED BY: libcore (interop_error.c, interop_handle.c, interop_ipc.c)
// NEEDED BY: libcore (interop_jsonrpc.c, interop_marshal.c, list.c)
// NEEDED BY: libcore (map_serialize.c, mem_arena.c, micro_lib_metadata.c)
// NEEDED BY: libcore (nanox.c, num_format.c, print_using.c, select.c)
// NEEDED BY: libcore (session.c, spec.c, state.c, string_ext.c, strings.c)
// NEEDED BY: libcore (texobj.c, using_engine.c, varptr.c, vdev_core.c, x11.c)
// NEEDED BY: libcore (zbuffer.c, zmath.c)
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h)
// NEEDED BY: libengine (mat_internal.h, reformat_internal.h, sub_internal.h)
// NEEDED BY: libengine (abs.c, acos.c, angle.c, append.c, array_ext.c)
// NEEDED BY: libengine (arrayfill.c, ascii_fn.c, asin.c, ask.c, assign.c)
// NEEDED BY: libengine (atan2.c, ath.c, atn.c, auto.c, backspace.c, beep.c)
// NEEDED BY: libengine (bgi.c, bin.c, bios.c, bload.c, brun.c, bsave.c, call.c)
// NEEDED BY: libengine (category.c, cause.c, ceil.c, chain.c, change.c)
// NEEDED BY: libengine (chdir.c, check.c, chr.c, circle.c, clamp.c, class.c)
// NEEDED BY: libengine (clock_num.c, clock_str.c, close.c, clr.c, cls.c)
// NEEDED BY: libengine (color.c, command_fn.c, common.c, comp.c, compat.c)
// NEEDED BY: libengine (complex.c, complex_fn.c, const.c, cont.c, continue.c)
// NEEDED BY: libengine (cos.c, cosh.c, cot.c, create.c, cross.c, csc.c)
// NEEDED BY: libengine (csrlin.c, cvt.c, data.c, date.c, debug.c, declare.c)
// NEEDED BY: libengine (def.c, def_seg.c, def_usr.c, defdbl.c, defint.c)
// NEEDED BY: libengine (deflng.c, defseg.c, defsng.c, defstr.c, degrees.c)
// NEEDED BY: libengine (delete.c, det.c, dim.c, do.c, doevents.c, dot.c)
// NEEDED BY: libengine (draw.c, echo.c, end.c, endloop.c, enter.c, enum.c)
// NEEDED BY: libengine (environ.c, eps.c, erase.c, erl.c, err_fn.c, ert.c)
// NEEDED BY: libengine (exchange.c, exec_dispatch.c, exec_interrupt.c)
// NEEDED BY: libengine (exit_loop.c, exp.c, extend.c, external.c, fid.c)
// NEEDED BY: libengine (field.c, files.c, financial.c, find.c, fix.c, floor.c)
// NEEDED BY: libengine (for.c, form.c, fpt.c, fre.c, function.c, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, handler.c, hex.c, hypot.c)
// NEEDED BY: libengine (if.c, image.c, incr.c, index_fn.c, inf.c, inkey.c)
// NEEDED BY: libengine (inp.c, input.c, input_file.c, instr.c, int.c)
// NEEDED BY: libengine (interrupt.c, invoke.c, isam.c, joystick.c, key.c)
// NEEDED BY: libengine (kill.c, lbound.c, lcase.c, left.c, len.c, lerp.c)
// NEEDED BY: libengine (let.c, line.c, line_input.c, linput.c, llist.c, load.c)
// NEEDED BY: libengine (lock.c, log.c, log10.c, log2.c, loop.c, lpos.c)
// NEEDED BY: libengine (lprint.c, lset.c, ltrim.c, mag.c, magtape.c, map.c)
// NEEDED BY: libengine (margin.c, mat_input.c, mat_print.c, mat_read.c, max.c)
// NEEDED BY: libengine (maxnum.c, mbf.c, merge.c, mid.c, mid_stmt.c, min.c)
// NEEDED BY: libengine (mkdir.c, mod.c, moddir.c, modify.c, module.c, mouse.c)
// NEEDED BY: libengine (msgbox.c, mux.c, name.c, new.c, next.c, num.c, oct.c)
// NEEDED BY: libengine (on_com.c, on_error.c, on_key.c, on_timer.c, open.c)
// NEEDED BY: libengine (option.c, out.c, pack.c, page.c, paint.c, palette.c)
// NEEDED BY: libengine (param.c, parser.c, pause.c, pdif.c, pds_datetime.c)
// NEEDED BY: libengine (pds_sys.c, peek.c, pen.c, perform.c, pi.c, pick.c)
// NEEDED BY: libengine (picture.c, play.c, poke.c, pos.c, prefix.c, preset.c)
// NEEDED BY: libengine (print.c, print_file.c, pset.c, public.c, put.c, rad.c)
// NEEDED BY: libengine (radians.c, randomize.c, read.c, record.c, redim.c)
// NEEDED BY: libengine (reformat.c, rem.c, remainder.c, renum.c, repeat.c)
// NEEDED BY: libengine (restore.c, resume.c, retry.c, return.c, rewind.c)
// NEEDED BY: libengine (right.c, rmdir.c, rnd.c, round.c, rset.c, rtrim.c)
// NEEDED BY: libengine (run.c, save.c, scale.c, screen.c, sec.c, seek.c, seg.c)
// NEEDED BY: libengine (session_stmts.c, sgn.c, share.c, shared.c, shell.c)
// NEEDED BY: libengine (shuffle.c, sin.c, sinh.c, sleep.c, sound.c, space.c)
// NEEDED BY: libengine (spc.c, spec_fn.c, sqr.c, stmt_home.c, stmt_plot.c)
// NEEDED BY: libengine (stop.c, str.c, str_math.c, string_fn.c, suspend.c)
// NEEDED BY: libengine (swap.c, sys.c, sys_fn.c, system.c, tab.c, tan.c)
// NEEDED BY: libengine (tanh.c, task.c, tek.c, test.c, text.c, ticks.c, time.c)
// NEEDED BY: libengine (time_fn.c, timer.c, trim.c, truncate.c, try.c, txn.c)
// NEEDED BY: libengine (type.c, ubound.c, ucase.c, unless.c, unpack.c, until.c)
// NEEDED BY: libengine (ups.c, val.c, vbdos_controls.c, vbdos_filebox.c)
// NEEDED BY: libengine (vbdos_fn.c, vbdos_widgets.c, verify.c, verify_fn.c)
// NEEDED BY: libengine (version.c, view.c, viewport.c, voice.c, wait.c, wend.c)
// NEEDED BY: libengine (when.c, whenever.c, while.c, window.c, with.c)
// NEEDED BY: libengine (write_file.c, xlate.c, zone.c)
// NEEDED BY: libext (arrayext.c)
// NEEDED BY: libkernel (aalib.c, gfx_tui.c, security.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libstandard (standalone_runner.h)
// NEEDS: libcore (hal.h, memory.h, micro_lib_metadata.h, string.h, strings.h)
// NEEDS: libcore (memory.c, micro_lib_metadata.c, strings.c)
// Provides runtime implementation for the STRING built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/string.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_string_register(void) {
    MicroLibMetadata meta = {
        .name = "STRING$",
        .category = "String Functions",
        .syntax = "STRING$(n, char_spec)",
        .help_text = "Returns a string of n repeating characters specified by ASCII code or 1st char of string.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (STRING$ argument type error)"
    };
    microlib_register(&meta);
}

BValue func_string_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 2 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "STRING$ expects a number and a character/string";
        return res;
    }

    int n = (int)args[0].as.number;
    if (n < 0) {
        err->code = 5;
        err->message = "Negative count in STRING$";
        return res;
    }

    char c = ' ';
    if (args[1].type == VAL_NUMBER) {
        c = (char)args[1].as.number;
    } else {
        BppStringRef sr = args[1].as.string;
        const char *s = str_data(sr);
        if (str_len(sr) > 0) c = s[0];
        str_release(vm_get_str(vm), sr);
    }

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (n + 1)) : NULL);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    runtime_memset(buf, c, n);
    buf[n] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, n);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}

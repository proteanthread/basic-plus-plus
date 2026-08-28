// FILENAME: strings.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libplatform
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for strings.h.
//
// ---- Includes ----

// FILENAME: strings.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, strings.c, struct.c, variables.h)
// NEEDED BY: libcore (variables_internal.h)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, assign.c)
// NEEDED BY: libengine (ast_create.c, ast_eval_expr.c, ast_eval_stmt.c)
// NEEDED BY: libengine (ast_internal.h, ast_parse_block.c, ast_parse_expr.c)
// NEEDED BY: libengine (ast_parse_stmt.c, ath.c, beep.c, bgi.c, bin.c, bsave.c)
// NEEDED BY: libengine (category.c, chain.c, change.c, chdir.c, chr.c)
// NEEDED BY: libengine (clock_str.c, clr.c, command_fn.c, const.c, create.c)
// NEEDED BY: libengine (cvt.c, date.c, def.c, def_seg.c, draw.c, endloop.c)
// NEEDED BY: libengine (enter.c, environ.c, ert.c, eval.h, eval_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, fid.c, field.c, fre.c, gosub.c)
// NEEDED BY: libengine (goto.c, hex.c, host.c, incr.c, index_fn.c, inkey.c)
// NEEDED BY: libengine (input.c, input_file.c, instr.c, invoke.c, isam.c)
// NEEDED BY: libengine (key.c, kill.c, lcase.c, left.c, len.c, let.c)
// NEEDED BY: libengine (line_input.c, load.c, lock.c, lprint.c, lset.c)
// NEEDED BY: libengine (ltrim.c, map.c, mat_internal.h, mbf.c, merge.c, mid.c)
// NEEDED BY: libengine (mkdir.c, moddir.c, msgbox.c, name.c, num.c, oct.c)
// NEEDED BY: libengine (on_timer.c, pack.c, pause.c, pds_datetime.c, pds_sys.c)
// NEEDED BY: libengine (pick.c, play.c, print_file.c, rad.c, read.c, remove.c)
// NEEDED BY: libengine (restore.c, right.c, rmdir.c, rset.c, rtrim.c, run.c)
// NEEDED BY: libengine (save.c, seg.c, selftest.c, session_stmts.c, shell.c)
// NEEDED BY: libengine (shuffle.c, sound.c, space.c, spc.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, sub_internal.h, sys.c, sys_fn.c, tab.c)
// NEEDED BY: libengine (tek.c, ticks.c, time.c, trim.c, try.c, ucase.c)
// NEEDED BY: libengine (unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify_fn.c, vm.h, void.c, whenever.c)
// NEEDED BY: libengine (while.c, write_file.c, xlate.c)
// NEEDED BY: libkernel (config.h)
// NEEDED BY: libplatform (platform.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for strings.h.
//
// ---- Includes ----

// FILENAME: strings.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, strings.c, struct.c, variables.h)
// NEEDED BY: libcore (variables_internal.h)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, assign.c)
// NEEDED BY: libengine (ast_create.c, ast_eval_expr.c, ast_eval_stmt.c)
// NEEDED BY: libengine (ast_internal.h, ast_parse_block.c, ast_parse_expr.c)
// NEEDED BY: libengine (ast_parse_stmt.c, ath.c, beep.c, bgi.c, bin.c, bsave.c)
// NEEDED BY: libengine (category.c, chain.c, change.c, chdir.c, chr.c)
// NEEDED BY: libengine (clock_str.c, clr.c, command_fn.c, const.c, create.c)
// NEEDED BY: libengine (cvt.c, date.c, def.c, def_seg.c, draw.c, endloop.c)
// NEEDED BY: libengine (enter.c, environ.c, ert.c, eval.h, eval_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, fid.c, field.c, fre.c, gosub.c)
// NEEDED BY: libengine (goto.c, hex.c, host.c, incr.c, index_fn.c, inkey.c)
// NEEDED BY: libengine (input.c, input_file.c, instr.c, invoke.c, isam.c)
// NEEDED BY: libengine (key.c, kill.c, lcase.c, left.c, len.c, let.c)
// NEEDED BY: libengine (line_input.c, load.c, lock.c, lprint.c, lset.c)
// NEEDED BY: libengine (ltrim.c, map.c, mat_internal.h, mbf.c, merge.c, mid.c)
// NEEDED BY: libengine (mkdir.c, moddir.c, msgbox.c, name.c, num.c, oct.c)
// NEEDED BY: libengine (on_timer.c, pack.c, pause.c, pds_datetime.c, pds_sys.c)
// NEEDED BY: libengine (pick.c, play.c, print_file.c, rad.c, read.c, remove.c)
// NEEDED BY: libengine (restore.c, right.c, rmdir.c, rset.c, rtrim.c, run.c)
// NEEDED BY: libengine (save.c, seg.c, selftest.c, session_stmts.c, shell.c)
// NEEDED BY: libengine (shuffle.c, sound.c, space.c, spc.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, sub_internal.h, sys.c, sys_fn.c, tab.c)
// NEEDED BY: libengine (tek.c, ticks.c, time.c, trim.c, try.c, ucase.c)
// NEEDED BY: libengine (unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify_fn.c, vm.h, void.c, whenever.c)
// NEEDED BY: libengine (while.c, write_file.c, xlate.c)
// NEEDED BY: libkernel (config.h)
// NEEDED BY: libplatform (platform.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for strings.h.
//
// ---- Includes ----

// FILENAME: strings.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, strings.c, struct.c, variables.h)
// NEEDED BY: libcore (variables_internal.h)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, assign.c)
// NEEDED BY: libengine (ast_create.c, ast_eval_expr.c, ast_eval_stmt.c)
// NEEDED BY: libengine (ast_internal.h, ast_parse_block.c, ast_parse_expr.c)
// NEEDED BY: libengine (ast_parse_stmt.c, ath.c, beep.c, bgi.c, bin.c, bsave.c)
// NEEDED BY: libengine (category.c, chain.c, change.c, chdir.c, chr.c)
// NEEDED BY: libengine (clock_str.c, clr.c, command_fn.c, const.c, create.c)
// NEEDED BY: libengine (cvt.c, date.c, def.c, def_seg.c, draw.c, endloop.c)
// NEEDED BY: libengine (enter.c, environ.c, ert.c, eval.h, eval_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, fid.c, field.c, fre.c, gosub.c)
// NEEDED BY: libengine (goto.c, hex.c, host.c, incr.c, index_fn.c, inkey.c)
// NEEDED BY: libengine (input.c, input_file.c, instr.c, invoke.c, isam.c)
// NEEDED BY: libengine (key.c, kill.c, lcase.c, left.c, len.c, let.c)
// NEEDED BY: libengine (line_input.c, load.c, lock.c, lprint.c, lset.c)
// NEEDED BY: libengine (ltrim.c, map.c, mat_internal.h, mbf.c, merge.c, mid.c)
// NEEDED BY: libengine (mkdir.c, moddir.c, msgbox.c, name.c, num.c, oct.c)
// NEEDED BY: libengine (on_timer.c, pack.c, pause.c, pds_datetime.c, pds_sys.c)
// NEEDED BY: libengine (pick.c, play.c, print_file.c, rad.c, read.c, remove.c)
// NEEDED BY: libengine (restore.c, right.c, rmdir.c, rset.c, rtrim.c, run.c)
// NEEDED BY: libengine (save.c, seg.c, selftest.c, session_stmts.c, shell.c)
// NEEDED BY: libengine (shuffle.c, sound.c, space.c, spc.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, sub_internal.h, sys.c, sys_fn.c, tab.c)
// NEEDED BY: libengine (tek.c, ticks.c, time.c, trim.c, try.c, ucase.c)
// NEEDED BY: libengine (unless.c, unpack.c, until.c, ups.c, val.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify_fn.c, vm.h, void.c, whenever.c)
// NEEDED BY: libengine (while.c, write_file.c, xlate.c)
// NEEDED BY: libkernel (config.h)
// NEEDED BY: libplatform (platform.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for strings.h.
//
// ---- Includes ----

// FILENAME: strings.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (arrays.h, variables.h, variables_internal.h)
// NEEDED BY: libcore (array_sort.c, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, strings.c, struct.c)
// NEEDED BY: libengine (ast_internal.h, eval.h, eval_internal.h)
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h)
// NEEDED BY: libengine (mat_internal.h, sub_internal.h, vm.h)
// NEEDED BY: libengine (array_ext.c, arrayfill.c, ascii_fn.c, assign.c)
// NEEDED BY: libengine (ast_create.c, ast_eval_expr.c, ast_eval_stmt.c)
// NEEDED BY: libengine (ast_parse_block.c, ast_parse_expr.c, ast_parse_stmt.c)
// NEEDED BY: libengine (ath.c, beep.c, bgi.c, bin.c, bsave.c, category.c)
// NEEDED BY: libengine (chain.c, change.c, chdir.c, chr.c, clock_str.c, clr.c)
// NEEDED BY: libengine (command_fn.c, const.c, create.c, cvt.c, date.c, def.c)
// NEEDED BY: libengine (def_seg.c, draw.c, endloop.c, enter.c, environ.c)
// NEEDED BY: libengine (ert.c, exec_dispatch.c, fid.c, field.c, fre.c, gosub.c)
// NEEDED BY: libengine (goto.c, hex.c, host.c, incr.c, index_fn.c, inkey.c)
// NEEDED BY: libengine (input.c, input_file.c, instr.c, invoke.c, isam.c)
// NEEDED BY: libengine (key.c, kill.c, lcase.c, left.c, len.c, let.c)
// NEEDED BY: libengine (line_input.c, load.c, lock.c, lprint.c, lset.c)
// NEEDED BY: libengine (ltrim.c, map.c, mbf.c, merge.c, mid.c, mkdir.c)
// NEEDED BY: libengine (moddir.c, msgbox.c, name.c, num.c, oct.c, on_timer.c)
// NEEDED BY: libengine (pack.c, pause.c, pds_datetime.c, pds_sys.c, pick.c)
// NEEDED BY: libengine (play.c, print_file.c, rad.c, read.c, remove.c)
// NEEDED BY: libengine (restore.c, right.c, rmdir.c, rset.c, rtrim.c, run.c)
// NEEDED BY: libengine (save.c, seg.c, selftest.c, session_stmts.c, shell.c)
// NEEDED BY: libengine (shuffle.c, sound.c, space.c, spc.c, str.c, str_math.c)
// NEEDED BY: libengine (string.c, sys.c, sys_fn.c, tab.c, tek.c, ticks.c)
// NEEDED BY: libengine (time.c, trim.c, try.c, ucase.c, unless.c, unpack.c)
// NEEDED BY: libengine (until.c, ups.c, val.c, vbdos_controls.c)
// NEEDED BY: libengine (vbdos_filebox.c, vbdos_fn.c, vbdos_widgets.c)
// NEEDED BY: libengine (verify_fn.c, void.c, whenever.c, while.c, write_file.c)
// NEEDED BY: libengine (xlate.c)
// NEEDED BY: libkernel (config.h)
// NEEDED BY: libplatform (platform.h)
// NEEDS: libcore (memory.h)
// NEEDS: libcore (memory.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for strings within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_STRINGS_H
#define RUNTIME_STRINGS_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "memory/memory.h"

// Opaque String Manager Context
typedef struct StringContext StringContext;

// @brief Initialize the string manager context.
StringContext *str_init(MemoryContext *mem);

// @brief Shutdown the string manager and free all registered strings.
void str_shutdown(StringContext *ctx);

// @brief Create a new string in the isolated heap.
BppStringRef str_create(StringContext *ctx, const char *data, size_t length);

// @brief Concatenate two strings and return a new string.
BppStringRef str_concat(StringContext *ctx, BppStringRef a, BppStringRef b);

// @brief Append data to an existing string in-place if ref_count == 1, or reallocate.
BppStringRef str_concat_multi(StringContext *ctx, const char **parts, const size_t *lens, size_t count);
BppStringRef str_concat_multi_inplace(StringContext *ctx, BppStringRef target, const char **parts, const size_t *lens, size_t count);
BppStringRef str_append_inplace(StringContext *ctx, BppStringRef target, const char *data, size_t length);

// @brief Assign new contents to an existing string buffer in-place if ref_count == 1, or allocate.
BppStringRef str_assign_inplace(StringContext *ctx, BppStringRef target, const char *data, size_t length);

// @brief Concatenate multiple string slices in a single allocation pass.
BppStringRef str_concat_multi(StringContext *ctx, const char **parts, const size_t *lens, size_t count);

// @brief Extract a substring (implements MID$).
BppStringRef str_mid(StringContext *ctx, BppStringRef ref, size_t start, size_t len);

// @brief Retrieve raw text pointer from string reference.
const char *str_data(BppStringRef ref);

// @brief Retrieve mutable text pointer from string reference (internal use).
char *str_data_mut(BppStringRef ref);

// @brief Retrieve length of the string.
size_t str_len(BppStringRef ref);

// @brief Increment reference count of a string.
void str_add_ref(BppStringRef ref);

// @brief Decrement reference count of a string and free if count reaches 0.
void str_release(StringContext *ctx, BppStringRef ref);

// @brief Check if string has unique ownership (ref_count == 1).
bool str_is_unique(BppStringRef ref);

// @brief Create a static immutable string singleton for AST literals.
BppStringRef str_create_static(const char *data, size_t length);

// @brief Free a static immutable string singleton created by str_create_static.
void str_free_static(BppStringRef ref);

#endif // RUNTIME_STRINGS_H

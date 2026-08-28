// FILENAME: memops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, libstandard
// NEEDS: platform, memory
// Implements component functionality for memops.h.
//
// ---- Includes ----

// FILENAME: memops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memops.c, memory.h, metadata.c)
// NEEDED BY: libcore (qsort.c, regex.c, segmented_mem.c, snprintf.c, sscanf.c)
// NEEDED BY: libcore (string.h, strops.c, struct.c, vfs.c)
// NEEDED BY: libengine (alias.c, ast_internal.h, bios.c, bios_hal_vm.c)
// NEEDED BY: libengine (category.c, context.c, control.c, data.c)
// NEEDED BY: libengine (dispatch_internal.h, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, help.c, host.c)
// NEEDED BY: libengine (introspection.c, keyword.c, lexer.c, map.c, override.c)
// NEEDED BY: libengine (remove.c, scan_keyword.c, scan_string.c, scope.c)
// NEEDED BY: libengine (selftest.c, stack.c, task.c, vm_internal.h)
// NEEDED BY: libhardware (bios_at.c, bios_cpu8086.c, bios_int10.c)
// NEEDED BY: libhardware (bios_int13.c, bios_int16.c, bios_jr.c, bios_pc.c)
// NEEDED BY: libhardware (bios_xt.c)
// NEEDED BY: libkernel (bgi_autodetect.c, bgi_bridge.c, bgi_core.c, bgi_font.c)
// NEEDED BY: libkernel (bgi_gfx.c, bgi_modes.c, bgi_palette.c, bgi_raster.c)
// NEEDED BY: libkernel (bgi_text.c, bus.c, console.c, fujinet.c, gfx.c)
// NEEDED BY: libkernel (gfx_audio.c, gfx_draw_core.c, gfx_palette.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h, gfx_stmt_retro.c)
// NEEDED BY: libkernel (gfx_stmt_shapes.c, pdf_writer.c, security.c, vcon.c)
// NEEDED BY: libkernel (vdev.c, vdev_esp32.c, vprinter.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, editor.c, edlin_internal.h)
// NEEDED BY: libstandard (vi_internal.h, ws_internal.h)
// NEEDS: platform, memory
// Implements component functionality for memops.h.
//
// ---- Includes ----

// FILENAME: memops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memops.c, memory.h, metadata.c)
// NEEDED BY: libcore (qsort.c, regex.c, segmented_mem.c, snprintf.c, sscanf.c)
// NEEDED BY: libcore (string.h, strops.c, struct.c, vfs.c)
// NEEDED BY: libengine (alias.c, ast_internal.h, bios.c, bios_hal_vm.c)
// NEEDED BY: libengine (category.c, context.c, control.c, data.c)
// NEEDED BY: libengine (dispatch_internal.h, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, help.c, host.c)
// NEEDED BY: libengine (introspection.c, keyword.c, lexer.c, map.c, override.c)
// NEEDED BY: libengine (remove.c, scan_keyword.c, scan_string.c, scope.c)
// NEEDED BY: libengine (selftest.c, stack.c, task.c, vm_internal.h)
// NEEDED BY: libhardware (bios_at.c, bios_cpu8086.c, bios_int10.c)
// NEEDED BY: libhardware (bios_int13.c, bios_int16.c, bios_jr.c, bios_pc.c)
// NEEDED BY: libhardware (bios_xt.c)
// NEEDED BY: libkernel (bgi_autodetect.c, bgi_bridge.c, bgi_core.c, bgi_font.c)
// NEEDED BY: libkernel (bgi_gfx.c, bgi_modes.c, bgi_palette.c, bgi_raster.c)
// NEEDED BY: libkernel (bgi_text.c, bus.c, console.c, fujinet.c, gfx.c)
// NEEDED BY: libkernel (gfx_audio.c, gfx_draw_core.c, gfx_palette.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h, gfx_stmt_retro.c)
// NEEDED BY: libkernel (gfx_stmt_shapes.c, pdf_writer.c, security.c, vcon.c)
// NEEDED BY: libkernel (vdev.c, vdev_esp32.c, vprinter.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, editor.c, edlin_internal.h)
// NEEDED BY: libstandard (vi_internal.h, ws_internal.h)
// NEEDS: platform, memory
// Implements component functionality for memops.h.
//
// ---- Includes ----

// FILENAME: memops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memops.c, memory.h, metadata.c)
// NEEDED BY: libcore (qsort.c, regex.c, segmented_mem.c, snprintf.c, sscanf.c)
// NEEDED BY: libcore (string.h, strops.c, struct.c, vfs.c)
// NEEDED BY: libengine (alias.c, ast_internal.h, bios.c, bios_hal_vm.c)
// NEEDED BY: libengine (category.c, context.c, control.c, data.c)
// NEEDED BY: libengine (dispatch_internal.h, eval_expr_internal.h)
// NEEDED BY: libengine (eval_internal.h, events_internal.h, help.c, host.c)
// NEEDED BY: libengine (introspection.c, keyword.c, lexer.c, map.c, override.c)
// NEEDED BY: libengine (remove.c, scan_keyword.c, scan_string.c, scope.c)
// NEEDED BY: libengine (selftest.c, stack.c, task.c, vm_internal.h)
// NEEDED BY: libhardware (bios_at.c, bios_cpu8086.c, bios_int10.c)
// NEEDED BY: libhardware (bios_int13.c, bios_int16.c, bios_jr.c, bios_pc.c)
// NEEDED BY: libhardware (bios_xt.c)
// NEEDED BY: libkernel (bgi_autodetect.c, bgi_bridge.c, bgi_core.c, bgi_font.c)
// NEEDED BY: libkernel (bgi_gfx.c, bgi_modes.c, bgi_palette.c, bgi_raster.c)
// NEEDED BY: libkernel (bgi_text.c, bus.c, console.c, fujinet.c, gfx.c)
// NEEDED BY: libkernel (gfx_audio.c, gfx_draw_core.c, gfx_palette.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h, gfx_stmt_retro.c)
// NEEDED BY: libkernel (gfx_stmt_shapes.c, pdf_writer.c, security.c, vcon.c)
// NEEDED BY: libkernel (vdev.c, vprinter.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, editor.c, edlin_internal.h)
// NEEDED BY: libstandard (vi_internal.h, ws_internal.h)
// NEEDS: platform, memory
// Implements component functionality for memops.h.
//
// ---- Includes ----

// FILENAME: memops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (arrays_internal.h, file_internal.h, memory.h, string.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, bpp_api.c, calendar.c)
// NEEDED BY: libcore (compiler_ir.c, crypto.c, dialect.c, docgen.c)
// NEEDED BY: libcore (editor_buffer.c, editor_render.c, editor_selection.c)
// NEEDED BY: libcore (error.c, feature_reg.c, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memops.c, metadata.c, qsort.c)
// NEEDED BY: libcore (regex.c, segmented_mem.c, snprintf.c, sscanf.c, strops.c)
// NEEDED BY: libcore (struct.c, vfs.c)
// NEEDED BY: libengine (ast_internal.h, dispatch_internal.h)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, vm_internal.h)
// NEEDED BY: libengine (alias.c, bios.c, bios_hal_vm.c, category.c, context.c)
// NEEDED BY: libengine (control.c, data.c, help.c, host.c, introspection.c)
// NEEDED BY: libengine (keyword.c, lexer.c, map.c, override.c, remove.c)
// NEEDED BY: libengine (scan_keyword.c, scan_string.c, scope.c, selftest.c)
// NEEDED BY: libengine (stack.c, task.c)
// NEEDED BY: libhardware (bios_at.c, bios_cpu8086.c, bios_int10.c)
// NEEDED BY: libhardware (bios_int13.c, bios_int16.c, bios_jr.c, bios_pc.c)
// NEEDED BY: libhardware (bios_xt.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h)
// NEEDED BY: libkernel (bgi_autodetect.c, bgi_bridge.c, bgi_core.c, bgi_font.c)
// NEEDED BY: libkernel (bgi_gfx.c, bgi_modes.c, bgi_palette.c, bgi_raster.c)
// NEEDED BY: libkernel (bgi_text.c, bus.c, console.c, fujinet.c, gfx.c)
// NEEDED BY: libkernel (gfx_audio.c, gfx_draw_core.c, gfx_palette.c)
// NEEDED BY: libkernel (gfx_stmt_retro.c, gfx_stmt_shapes.c, pdf_writer.c)
// NEEDED BY: libkernel (security.c, vcon.c, vdev.c, vprinter.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, edlin_internal.h, vi_internal.h)
// NEEDED BY: libstandard (ws_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: platform, memory
// Freestanding memory copy, move, set, and compare primitives.
//
// ---- Includes ----

#ifndef RUNTIME_STRING_MEMOPS_H
#define RUNTIME_STRING_MEMOPS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Copies n bytes from src to dest. Memory areas must not overlap.
void *runtime_memcpy(void *dest, const void *src, size_t n);

// @brief Fills the first n bytes of the memory area pointed to by s with the constant byte c.
void *runtime_memset(void *s, int c, size_t n);

// @brief Copies n bytes from src to dest. Memory areas may overlap.
void *runtime_memmove(void *dest, const void *src, size_t n);

// @brief Compares the first n bytes of the memory areas s1 and s2.
int runtime_memcmp(const void *s1, const void *s2, size_t n);

// @brief Scans the initial n bytes of s for the first instance of c.
void *runtime_memchr(const void *s, int c, size_t n);

// @brief Reverses n bytes in-place.
void *runtime_memrev(void *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_STRING_MEMOPS_H

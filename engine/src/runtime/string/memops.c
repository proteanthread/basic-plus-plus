// FILENAME: memops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, libstandard
// NEEDS: libcore (memops.h)
// Implements component functionality for memops.c.
//
// ---- Includes ----

// FILENAME: memops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memory.h, metadata.c, qsort.c)
// NEEDED BY: libcore (regex.c, segmented_mem.c, snprintf.c, sscanf.c, string.h)
// NEEDED BY: libcore (strops.c, struct.c, vfs.c)
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
// NEEDS: libcore (memops.h)
// Implements component functionality for memops.c.
//
// ---- Includes ----

// FILENAME: memops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memory.h, metadata.c, qsort.c)
// NEEDED BY: libcore (regex.c, segmented_mem.c, snprintf.c, sscanf.c, string.h)
// NEEDED BY: libcore (strops.c, struct.c, vfs.c)
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
// NEEDS: libcore (memops.h)
// Implements component functionality for memops.c.
//
// ---- Includes ----

// FILENAME: memops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, arrays_internal.h)
// NEEDED BY: libcore (bpp_api.c, calendar.c, compiler_ir.c, crypto.c)
// NEEDED BY: libcore (dialect.c, docgen.c, editor_buffer.c, editor_render.c)
// NEEDED BY: libcore (editor_selection.c, error.c, feature_reg.c)
// NEEDED BY: libcore (file_internal.h, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, memory.h, metadata.c, qsort.c)
// NEEDED BY: libcore (regex.c, segmented_mem.c, snprintf.c, sscanf.c, string.h)
// NEEDED BY: libcore (strops.c, struct.c, vfs.c)
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
// NEEDS: libcore (memops.h)
// Implements component functionality for memops.c.
//
// ---- Includes ----

// FILENAME: memops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (arrays_internal.h, file_internal.h, memory.h, string.h)
// NEEDED BY: libcore (alloc.c, analyzer.c, arena.c, bpp_api.c, calendar.c)
// NEEDED BY: libcore (compiler_ir.c, crypto.c, dialect.c, docgen.c)
// NEEDED BY: libcore (editor_buffer.c, editor_render.c, editor_selection.c)
// NEEDED BY: libcore (error.c, feature_reg.c, float_parse.c, gemini.c)
// NEEDED BY: libcore (hal_freestanding.c, hal_sdl2.c, keyword_props.c)
// NEEDED BY: libcore (logger.c, mem_system.c, metadata.c, qsort.c, regex.c)
// NEEDED BY: libcore (segmented_mem.c, snprintf.c, sscanf.c, strops.c)
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
// NEEDS: libcore (memops.h)
// Freestanding memory copy, move, set, and compare implementation.
//
// ---- Includes ----

#include "runtime/string/memops.h"

void *runtime_memcpy(void *dest, const void *src, size_t n) {
    if (!dest || !src || n == 0) {
        return dest;
    }

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    // Word-aligned copy optimization for larger chunks
    if (n >= sizeof(uintptr_t) && (((uintptr_t)d & (sizeof(uintptr_t) - 1)) == ((uintptr_t)s & (sizeof(uintptr_t) - 1)))) {
        while (((uintptr_t)d & (sizeof(uintptr_t) - 1)) != 0 && n > 0) {
            *d++ = *s++;
            n--;
        }
        uintptr_t *wd = (uintptr_t *)d;
        const uintptr_t *ws = (const uintptr_t *)s;
        while (n >= sizeof(uintptr_t)) {
            *wd++ = *ws++;
            n -= sizeof(uintptr_t);
        }
        d = (uint8_t *)wd;
        s = (const uint8_t *)ws;
    }

    while (n > 0) {
        *d++ = *s++;
        n--;
    }

    return dest;
}

void *runtime_memset(void *s, int c, size_t n) {
    if (!s || n == 0) {
        return s;
    }

    uint8_t *p = (uint8_t *)s;
    uint8_t byte = (uint8_t)c;

    if (n >= sizeof(uintptr_t)) {
        uintptr_t word = byte;
        for (size_t i = 1; i < sizeof(uintptr_t); ++i) {
            word = (word << 8) | byte;
        }

        while (((uintptr_t)p & (sizeof(uintptr_t) - 1)) != 0 && n > 0) {
            *p++ = byte;
            n--;
        }

        uintptr_t *wp = (uintptr_t *)p;
        while (n >= sizeof(uintptr_t)) {
            *wp++ = word;
            n -= sizeof(uintptr_t);
        }
        p = (uint8_t *)wp;
    }

    while (n > 0) {
        *p++ = byte;
        n--;
    }

    return s;
}

void *runtime_memmove(void *dest, const void *src, size_t n) {
    if (!dest || !src || n == 0 || dest == src) {
        return dest;
    }

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d < s || d >= (s + n)) {
        // Non-overlapping or copying forwards
        return runtime_memcpy(dest, src, n);
    } else {
        // Overlapping with dest > src: copy backwards
        d += n;
        s += n;
        while (n > 0) {
            *(--d) = *(--s);
            n--;
        }
    }

    return dest;
}

int runtime_memcmp(const void *s1, const void *s2, size_t n) {
    if (!s1 || !s2 || n == 0) {
        return 0;
    }

    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }

    return 0;
}

void *runtime_memchr(const void *s, int c, size_t n) {
    if (!s) {
        return NULL;
    }

    const uint8_t *p = (const uint8_t *)s;
    uint8_t byte = (uint8_t)c;

    for (size_t i = 0; i < n; ++i) {
        if (p[i] == byte) {
            return (void *)(p + i);
        }
    }

    return NULL;
}

void *runtime_memrev(void *s, size_t n) {
    if (!s || n <= 1) {
        return s;
    }

    uint8_t *start = (uint8_t *)s;
    uint8_t *end = start + n - 1;

    while (start < end) {
        uint8_t tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }

    return s;
}

// FILENAME: strings.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libplatform
// NEEDS: libcore (string.h, strings.h)
// NEEDS: libengine (string.c)
// Implements component functionality for strings.c.
//
// ---- Includes ----

// FILENAME: strings.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, struct.c, variables.h)
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
// NEEDS: libcore (string.h, strings.h)
// NEEDS: libengine (string.c)
// Implements component functionality for strings.c.
//
// ---- Includes ----

// FILENAME: strings.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, struct.c, variables.h)
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
// NEEDS: libcore (string.h, strings.h)
// NEEDS: libengine (string.c)
// Implements component functionality for strings.c.
//
// ---- Includes ----

// FILENAME: strings.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (array_sort.c, arrays.h, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, struct.c, variables.h)
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
// NEEDS: libcore (string.h, strings.h)
// NEEDS: libengine (string.c)
// Implements component functionality for strings.c.
//
// ---- Includes ----

// FILENAME: strings.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libcore (arrays.h, variables.h, variables_internal.h)
// NEEDED BY: libcore (array_sort.c, bpp_api.c, crypto.c, list.c)
// NEEDED BY: libcore (map_serialize.c, microplex.c, regex.c, segmented_mem.c)
// NEEDED BY: libcore (string_ext.c, struct.c)
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
// NEEDS: libcore (string.h, strings.h), libengine (string.c)
// Provides core logic and interface definitions for strings within BASIC++.
//
// ---- Includes ----

#include "runtime/strings.h"
#include <string.h>
#include <stdlib.h>

struct BppString {
    uint32_t      ref_count;
    size_t        length;
    size_t        capacity;
    BppString    *next;
    BppString    *prev;
    char          data[]; // Flexible array member
};

typedef struct {
    uint32_t      ref_count;
    size_t        length;
    size_t        capacity;
    BppString    *next;
    BppString    *prev;
    char          data[2];
} BppCharString;

#define SMALL_STR_SLOT_SIZE 128
#define SMALL_STR_POOL_SLOTS 32768
#define SMALL_STR_POOL_SIZE (SMALL_STR_POOL_SLOTS * SMALL_STR_SLOT_SIZE)

typedef struct SmallSlot {
    struct SmallSlot *next_free;
} SmallSlot;

struct StringContext {
    MemoryContext *mem;
    BppString     *head;
    BppString     *tail;
    size_t         allocated_count;
    BppCharString  empty_str;
    BppCharString  char_table[256];
    char          *pool_storage;
    SmallSlot     *free_list;
};

StringContext *str_init(MemoryContext *mem) {
    if (!mem) return NULL;
    StringContext *ctx = (StringContext *)calloc(1, sizeof(StringContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->head = NULL;
    ctx->tail = NULL;
    ctx->allocated_count = 0;

    // Initialize singleton empty string
    ctx->empty_str.ref_count = 0xFFFFFFFF;
    ctx->empty_str.length = 0;
    ctx->empty_str.capacity = 1;
    ctx->empty_str.data[0] = '\0';
    ctx->empty_str.data[1] = '\0';

    // Initialize 256 single-character string singletons
    for (int i = 0; i < 256; i++) {
        ctx->char_table[i].ref_count = 0xFFFFFFFF;
        ctx->char_table[i].length = 1;
        ctx->char_table[i].capacity = 1;
        ctx->char_table[i].data[0] = (char)i;
        ctx->char_table[i].data[1] = '\0';
    }

    // Initialize high-speed Small String Slab Pool
    ctx->pool_storage = (char *)malloc(SMALL_STR_POOL_SIZE);
    if (ctx->pool_storage) {
        ctx->free_list = NULL;
        for (size_t i = 0; i < SMALL_STR_POOL_SLOTS; i++) {
            SmallSlot *slot = (SmallSlot *)(ctx->pool_storage + i * SMALL_STR_SLOT_SIZE);
            slot->next_free = ctx->free_list;
            ctx->free_list = slot;
        }
    }

    return ctx;
}

void str_shutdown(StringContext *ctx) {
    if (!ctx) return;

    // Free all registered dynamic strings, regardless of ref counts
    BppString *curr = ctx->head;
    while (curr) {
        BppString *next = curr->next;
        mem_string_free(ctx->mem, curr);
        curr = next;
    }
    if (ctx->pool_storage) {
        free(ctx->pool_storage);
        ctx->pool_storage = NULL;
    }
    free(ctx);
}

BppStringRef str_create(StringContext *ctx, const char *data, size_t length) {
    if (!ctx) return NULL;

    if (length == 0) {
        return (BppStringRef)&ctx->empty_str;
    }
    if (length == 1 && data) {
        return (BppStringRef)&ctx->char_table[(unsigned char)data[0]];
    }

    // Allocate BppString struct + characters + null terminator
    size_t size = sizeof(BppString) + length + 1;
    BppString *str = NULL;
    bool from_pool = false;

    if (size <= SMALL_STR_SLOT_SIZE && ctx->free_list != NULL) {
        SmallSlot *slot = ctx->free_list;
        ctx->free_list = slot->next_free;
        str = (BppString *)slot;
        str->capacity = SMALL_STR_SLOT_SIZE - sizeof(BppString) - 1;
        from_pool = true;
    } else {
        str = (BppString *)mem_string_alloc(ctx->mem, size);
        if (str) str->capacity = length;
    }
    if (!str) return NULL;

    str->ref_count = 1;
    str->length = length;

    if (data) {
        memcpy(str->data, data, length);
    }
    str->data[length] = '\0';

    if (!from_pool) {
        // Insert large strings into tracker list
        str->next = NULL;
        str->prev = ctx->tail;
        if (ctx->tail) {
            ctx->tail->next = str;
        } else {
            ctx->head = str;
        }
        ctx->tail = str;
    } else {
        str->next = NULL;
        str->prev = NULL;
    }
    ctx->allocated_count++;

    return str;
}

BppStringRef str_append_inplace(StringContext *ctx, BppStringRef target, const char *data, size_t length) {
    if (!ctx) return NULL;
    if (!data || length == 0) return target;
    if (!target || target->ref_count >= 0xF0000000) {
        size_t t_len = target ? target->length : 0;
        size_t new_len = t_len + length;
        BppStringRef res = str_create(ctx, NULL, new_len);
        if (!res) return NULL;
        if (target && t_len > 0) memcpy(res->data, target->data, t_len);
        memcpy(res->data + t_len, data, length);
        res->data[new_len] = '\0';
        return res;
    }

    // Fast Path: In-place append if ref_count == 1 and capacity permits
    if (target->ref_count == 1 && target->length + length <= target->capacity) {
        memcpy(target->data + target->length, data, length);
        target->length += length;
        target->data[target->length] = '\0';
        return target;
    }

    // Geometric capacity expansion (2x)
    size_t new_len = target->length + length;
    size_t new_cap = (new_len < 64) ? 64 : (new_len * 2);
    size_t size = sizeof(BppString) + new_cap + 1;
    BppString *new_str = (BppString *)mem_string_alloc(ctx->mem, size);
    if (!new_str) return NULL;

    new_str->ref_count = 1;
    new_str->length = new_len;
    new_str->capacity = new_cap;
    memcpy(new_str->data, target->data, target->length);
    memcpy(new_str->data + target->length, data, length);
    new_str->data[new_len] = '\0';

    new_str->next = NULL;
    new_str->prev = ctx->tail;
    if (ctx->tail) {
        ctx->tail->next = new_str;
    } else {
        ctx->head = new_str;
    }
    ctx->tail = new_str;
    ctx->allocated_count++;

    str_release(ctx, target);
    return new_str;
}

BppStringRef str_assign_inplace(StringContext *ctx, BppStringRef target, const char *data, size_t length) {
    if (!ctx) return NULL;
    if (!target || target->ref_count > 1 || (void *)target == (void *)&ctx->empty_str ||
        ((void *)target >= (void *)ctx->char_table && (void *)target < (void *)(ctx->char_table + 256))) {
        if (target) str_release(ctx, target);
        return str_create(ctx, data, length);
    }
    if (length <= target->capacity) {
        if (data && length > 0) memcpy(target->data, data, length);
        target->length = length;
        target->data[length] = '\0';
        return target;
    }
    size_t new_cap = (length < 64) ? 64 : (length * 2);
    size_t size = sizeof(BppString) + new_cap + 1;
    BppString *new_str = (BppString *)mem_string_alloc(ctx->mem, size);
    if (!new_str) {
        str_release(ctx, target);
        return str_create(ctx, data, length);
    }
    new_str->ref_count = 1;
    new_str->length = length;
    new_str->capacity = new_cap;
    if (data && length > 0) memcpy(new_str->data, data, length);
    new_str->data[length] = '\0';
    new_str->next = NULL;
    new_str->prev = ctx->tail;
    if (ctx->tail) {
        ctx->tail->next = new_str;
    } else {
        ctx->head = new_str;
    }
    ctx->tail = new_str;
    ctx->allocated_count++;
    str_release(ctx, target);
    return new_str;
}

BppStringRef str_concat_multi(StringContext *ctx, const char **parts, const size_t *lens, size_t count) {
    if (!ctx || count == 0) return str_create(ctx, "", 0);
    size_t total_len = 0;
    for (size_t i = 0; i < count; i++) {
        if (lens && lens[i] > 0) total_len += lens[i];
        else if (parts && parts[i]) total_len += strlen(parts[i]);
    }
    BppStringRef res = str_create(ctx, NULL, total_len);
    if (!res) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        size_t l = (lens ? lens[i] : (parts && parts[i] ? strlen(parts[i]) : 0));
        if (parts && parts[i] && l > 0) {
            memcpy(res->data + pos, parts[i], l);
            pos += l;
        }
    }
    res->data[pos] = '\0';
    return res;
}

BppStringRef str_concat_multi_inplace(StringContext *ctx, BppStringRef target, const char **parts, const size_t *lens, size_t count) {
    if (!ctx) return NULL;
    size_t total_len = 0;
    for (size_t i = 0; i < count; i++) {
        if (lens && lens[i] > 0) total_len += lens[i];
        else if (parts && parts[i]) total_len += strlen(parts[i]);
    }
    if (target && target->ref_count == 1 && target->capacity >= total_len &&
        (void *)target != (void *)&ctx->empty_str &&
        ((void *)target < (void *)ctx->char_table || (void *)target >= (void *)(ctx->char_table + 256))) {
        size_t pos = 0;
        for (size_t i = 0; i < count; i++) {
            size_t l = (lens ? lens[i] : (parts && parts[i] ? strlen(parts[i]) : 0));
            if (parts && parts[i] && l > 0) {
                memcpy(target->data + pos, parts[i], l);
                pos += l;
            }
        }
        target->length = pos;
        target->data[pos] = '\0';
        return target;
    }
    BppStringRef res = str_concat_multi(ctx, parts, lens, count);
    if (target) str_release(ctx, target);
    return res;
}

BppStringRef str_concat(StringContext *ctx, BppStringRef a, BppStringRef b) {
    if (!ctx) return NULL;
    size_t len_a = a ? a->length : 0;
    size_t len_b = b ? b->length : 0;
    size_t new_len = len_a + len_b;

    BppStringRef res = str_create(ctx, NULL, new_len);
    if (!res) return NULL;

    if (a && len_a > 0) {
        memcpy(res->data, a->data, len_a);
    }
    if (b && len_b > 0) {
        memcpy(res->data + len_a, b->data, len_b);
    }
    res->data[new_len] = '\0';

    return res;
}

BppStringRef str_mid(StringContext *ctx, BppStringRef ref, size_t start, size_t len) {
    if (!ctx || !ref || start == 0) return str_create(ctx, "", 0);

    // BASIC indices are 1-based
    size_t idx = start - 1;
    if (idx >= ref->length) {
        return str_create(ctx, "", 0);
    }

    size_t avail = ref->length - idx;
    if (len > avail) {
        len = avail;
    }

    return str_create(ctx, ref->data + idx, len);
}

const char *str_data(BppStringRef ref) {
    return ref ? ref->data : "";
}

char *str_data_mut(BppStringRef ref) {
    return ref ? ref->data : NULL;
}

size_t str_len(BppStringRef ref) {
    return ref ? ref->length : 0;
}

void str_add_ref(BppStringRef ref) {
    if (ref && ref->ref_count < 0xF0000000) {
        ref->ref_count++;
    }
}

void str_release(StringContext *ctx, BppStringRef ref) {
    if (!ctx || !ref) return;

    // Singletons have ref_count >= 0xF0000000 and are never freed
    if (ref->ref_count >= 0xF0000000) {
        return;
    }

    if (ref->ref_count > 0) {
        ref->ref_count--;
    }

    if (ref->ref_count == 0) {
        char *ptr = (char *)ref;
        if (ctx->pool_storage && ptr >= ctx->pool_storage &&
            ptr < ctx->pool_storage + SMALL_STR_POOL_SIZE) {
            SmallSlot *slot = (SmallSlot *)ptr;
            slot->next_free = ctx->free_list;
            ctx->free_list = slot;
            ctx->allocated_count--;
            return;
        }

        // Unlink from large registry list
        if (ref->prev) {
            ref->prev->next = ref->next;
        } else {
            ctx->head = ref->next;
        }
        if (ref->next) {
            ref->next->prev = ref->prev;
        } else {
            ctx->tail = ref->prev;
        }
        ctx->allocated_count--;

        // Free memory
        mem_string_free(ctx->mem, ref);
    }
}

void str_gc(StringContext *ctx) {
    (void)ctx;
}

bool str_is_unique(BppStringRef ref) {
    return ref && ref->ref_count == 1;
}

BppStringRef str_create_static(const char *data, size_t length) {
    if (!data && length == 0) data = "";
    size_t size = sizeof(BppString) + length + 1;
    BppString *str = (BppString *)malloc(size);
    if (!str) return NULL;
    str->ref_count = 0xF0000001; // Permanent static singleton
    str->length = length;
    str->capacity = length;
    str->next = NULL;
    str->prev = NULL;
    if (data && length > 0) {
        memcpy(str->data, data, length);
    }
    str->data[length] = '\0';
    return str;
}

void str_free_static(BppStringRef ref) {
    if (ref && ref->ref_count == 0xF0000001) {
        free((void *)ref);
    }
}

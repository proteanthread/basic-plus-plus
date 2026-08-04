/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "memory/memory.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "bios/mock_bios.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/dialect.h"
#include "core/struct.h"
#include "module/module.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


MemoryContext   *vm_get_mem(VMContext *vm)   { return vm ? vm->mem : NULL; }
StringContext   *vm_get_str(VMContext *vm)   { return vm ? vm->str : NULL; }
VariableContext *vm_get_var(VMContext *vm)   { return vm ? vm->var : NULL; }
BppMetadataRegistry *vm_get_metadata(VMContext *vm) { return vm ? &vm->metadata_reg : NULL; }
void vm_set_current_filename(VMContext *vm, const char *filename) {
    if (vm) {
        if (filename) {
            strncpy(vm->current_filename, filename, sizeof(vm->current_filename) - 1);
            vm->current_filename[sizeof(vm->current_filename) - 1] = '\0';
        } else {
            vm->current_filename[0] = '\0';
        }
    }
}
const char *vm_get_current_filename(VMContext *vm) {
    return vm ? vm->current_filename : "";
}
void vm_set_start_line(VMContext *vm, BppLineNumber line) {
    if (vm) vm->start_line = line;
}
BppLineNumber vm_get_start_line(VMContext *vm) {
    return vm ? vm->start_line : 0.0;
}
ArrayContext    *vm_get_arr(VMContext *vm)   { return vm ? vm->arr : NULL; }
VDevContext     *vm_get_vdev(VMContext *vm)  { return vm ? vm->vdev : NULL; }
FileContext     *vm_get_file(VMContext *vm)  { return vm ? vm->file : NULL; }

BppDialect *vm_get_active_dialect(VMContext *vm) { return vm ? vm->active_dialect : NULL; }
BppDialect *vm_get_defining_dialect(VMContext *vm) { return vm ? vm->defining_dialect : NULL; }
void vm_set_defining_dialect(VMContext *vm, BppDialect *d) { if (vm) vm->defining_dialect = d; }


double vm_get_ti_offset(VMContext *vm) { return vm ? vm->ti_offset : 0.0; }
void vm_set_ti_offset(VMContext *vm, double val) { if (vm) vm->ti_offset = val; }
void vm_set_active_dialect(VMContext *vm, BppDialect *dialect) {
    if (vm) {
        if (vm->active_dialect) {
            dialect_free(vm->active_dialect);
        }
        vm->active_dialect = dialect;
        
        /* Configure var context case sensitivity */
        var_set_case_sensitive(vm->var, dialect && dialect->case_sensitive);
        
        /* Configure array base if dialect has one */
        if (dialect && dialect->default_array_base >= 0) {
            arr_set_option_base(vm->arr, dialect->default_array_base);
        } else {
            arr_set_option_base(vm->arr, 0); /* Default to 0 */
        }
    }
}
VfsContext      *vm_get_vfs(VMContext *vm)   { return vm ? vm->vfs : NULL; }
VNetContext     *vm_get_vnet(VMContext *vm)  { return vm ? vm->vnet : NULL; }
UsbContext      *vm_get_usb(VMContext *vm)   { return vm ? vm->usb : NULL; }
VConContext     *vm_get_vcon(VMContext *vm)  { return vm ? vm->vcon : NULL; }
MockBiosContext *vm_get_bios(VMContext *vm)  { return vm ? vm->bios : NULL; }
VMemContext     *vm_get_vmem(VMContext *vm)  { return vm ? vm->vmem : NULL; }
uint8_t         *vm_get_bios_ram(VMContext *vm)  { return vm ? vm->bios_ram : NULL; }


GosubStack  *vm_get_gosub_stack(VMContext *vm)  { return vm ? vm->gosub_stack : NULL; }
ForStack    *vm_get_for_stack(VMContext *vm)    { return vm ? vm->for_stack : NULL; }
WhileStack  *vm_get_while_stack(VMContext *vm)  { return vm ? vm->while_stack : NULL; }
DoStack     *vm_get_do_stack(VMContext *vm)     { return vm ? vm->do_stack : NULL; }
SelectStack *vm_get_select_stack(VMContext *vm) { return vm ? vm->select_stack : NULL; }
SubStack    *vm_get_sub_stack(VMContext *vm)    { return vm ? vm->sub_stack : NULL; }
TryStack    *vm_get_try_stack(VMContext *vm)    { return vm ? vm->try_stack : NULL; }

void vm_get_bios_registers(VMContext *vm, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags) {
    if (vm) {
        if (ax) *ax = vm->regs.ax;
        if (bx) *bx = vm->regs.bx;
        if (cx) *cx = vm->regs.cx;
        if (dx) *dx = vm->regs.dx;
        if (flags) *flags = vm->regs.flags;
    }
}

void vm_set_bios_registers(VMContext *vm, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags) {
    if (vm) {
        vm->regs.ax = ax;
        vm->regs.bx = bx;
        vm->regs.cx = cx;
        vm->regs.dx = dx;
        vm->regs.flags = flags;
    }
}

void             vm_set_opt_eh(VMContext *vm, bool enable) { if (vm) vm->opt_eh = enable; }
bool             vm_get_opt_eh(VMContext *vm) { return vm ? vm->opt_eh : false; }
void             vm_set_arithmetic_decimal(VMContext *vm, bool enable) { if (vm) vm->opt_arithmetic_decimal = enable; }
bool             vm_get_arithmetic_decimal(VMContext *vm) { return vm ? vm->opt_arithmetic_decimal : false; }
BppLineNumber    vm_get_current_line(VMContext *vm) { return vm ? vm->current_line : 0.0; }
void             vm_set_current_line(VMContext *vm, BppLineNumber line) { if (vm) vm->current_line = line; }
const char      *vm_get_current_stmt_pos(VMContext *vm) { return vm ? vm->current_stmt_pos : NULL; }
StmtRegistry    *vm_get_stmt_registry(VMContext *vm) { return vm ? vm->stmt_reg : NULL; }
bool             vm_is_running(VMContext *vm) { return vm ? vm->running : false; }
void             vm_set_running(VMContext *vm, bool running) { if (vm) vm->running = running; }
int              vm_get_eval_depth(VMContext *vm) { return vm ? vm->eval_depth : 0; }
void             vm_inc_eval_depth(VMContext *vm) { if (vm) vm->eval_depth++; }
void             vm_dec_eval_depth(VMContext *vm) { if (vm) vm->eval_depth--; }
double           vm_get_jiffies_multiplier(VMContext *vm) { return vm ? vm->jiffies_multiplier : 60.0; }
void             vm_set_jiffies_multiplier(VMContext *vm, double val) { if (vm) vm->jiffies_multiplier = val; }

void vm_reset_for_run(VMContext *vm) {
    if (!vm) return;

    /* Clear ALL control flow stacks - prior program's stack frames
     * contain dangling pointers into freed program line text */
    gosub_stack_clear(vm->gosub_stack);
    for_stack_clear(vm->for_stack);
    while_stack_clear(vm->while_stack);
    do_stack_clear(vm->do_stack);
    select_stack_clear(vm->select_stack);
    sub_stack_clear(vm->sub_stack);
    try_stack_clear(vm->try_stack);

    /* Clear aliases - each program should define its own */
    vm_clear_aliases(vm);

    vm->with_stack_depth = 0;

    /* Reset VM option flags to defaults */
    vm->opt_eh = false;
    vm->opt_arithmetic_decimal = false;

    /* Reset active dialect to default (BASIC++) */
    vm_set_active_dialect(vm, NULL);

    /* NOTE: metadata_init() is NOT called here because
     * metadata_pre_scan_program() already resets the registry at the
     * start of each pre-scan pass. Calling it here would wipe out the
     * freshly-scanned labels, docstrings, and metadata blocks. */

    /* Clear user-defined type registry */
    struct_registry_init(&vm->type_reg);

    /* Clear arrays - prior program's arrays may hold string references
     * that become dangling after variable clear */
    if (!vm->is_chaining) {
        arr_clear_all(vm->arr);
    }

    /* Clear custom keyword registry to prevent stale LOAD FEATURE
     * keywords from conflicting with ALIAS names in new programs */
    keyword_clear_custom();

    /* Clear spec registry (LOAD FEATURE state) */
    spec_registry_init();

    /* Clear library program lines loaded by LOAD FEATURE */
    mem_lib_program_clear(vm->mem);

    /* Clear/free loaded sound and image buffers */
    vdev_sound_free_all();
    vdev_image_free_all();

    /* Clear active error handling states */
    vm->error_trap_line = 0.0;
    vm->in_error_handler = false;
    vm->err_code = 0;
    vm->err_line = 0.0;
    vm->error_occurred_line = 0.0;
    vm->error_occurred_pos = NULL;
    vm->error_next_pos = NULL;

    /* Reset security sandbox and restrictions */
    security_init(SEC_OPEN);
}

void vm_save_state(VMContext *vm, BppVMState *state) {
    if (vm && state) {
        state->current_line = vm->current_line;
        state->current_pos = vm->current_pos;
        state->next_line = vm->next_line;
        state->next_pos = vm->next_pos;
        state->jump_active = vm->jump_active;
    }
}

void vm_restore_state(VMContext *vm, const BppVMState *state) {
    if (vm && state) {
        vm->current_line = state->current_line;
        vm->current_pos = state->current_pos;
        vm->next_line = state->next_line;
        vm->next_pos = state->next_pos;
        vm->jump_active = state->jump_active;
    }
}

VMContext *vm_init(MemoryContext *mem, StringContext *str, VariableContext *var, VDevContext *vdev) {
    if (!mem || !str || !var || !vdev) return NULL;
    VMContext *vm = (VMContext *)calloc(1, sizeof(VMContext));
    if (!vm) return NULL;

    vm->mem = mem;
    vm->str = str;
    vm->var = var;
#ifndef BASIC_LITE_BUILD
    vm->vmem = vmem_init(var);
#endif
    vm->vdev = vdev;
    struct_registry_init(&vm->type_reg);

    vm->stmt_reg = stmt_registry_init(mem);
    if (!vm->stmt_reg) {
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->gosub_stack = gosub_stack_init();
    if (!vm->gosub_stack) {
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->for_stack = for_stack_init();
    if (!vm->for_stack) {
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->while_stack = while_stack_init();
    if (!vm->while_stack) {
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->do_stack = do_stack_init();
    if (!vm->do_stack) {
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->select_stack = select_stack_init();
    if (!vm->select_stack) {
        do_stack_shutdown(vm->do_stack);
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->sub_stack = sub_stack_init();
    if (!vm->sub_stack) {
        select_stack_shutdown(vm->select_stack);
        do_stack_shutdown(vm->do_stack);
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->try_stack = try_stack_init();
    if (!vm->try_stack) {
        sub_stack_shutdown(vm->sub_stack);
        select_stack_shutdown(vm->select_stack);
        do_stack_shutdown(vm->do_stack);
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->active_proc[0] = '\0';
    vm->opt_eh = false;
    vm->active_dialect = NULL;
    vm->jiffies_multiplier = 60.0;

    vm->arr = arr_init(mem, str);
    if (!vm->arr) {
        try_stack_shutdown(vm->try_stack);
        sub_stack_shutdown(vm->sub_stack);
        select_stack_shutdown(vm->select_stack);
        do_stack_shutdown(vm->do_stack);
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->file = file_init(mem);
    if (!vm->file) {
        arr_shutdown(vm->arr);
        try_stack_shutdown(vm->try_stack);
        sub_stack_shutdown(vm->sub_stack);
        select_stack_shutdown(vm->select_stack);
        do_stack_shutdown(vm->do_stack);
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        free(vm);
        return NULL;
    }

    vm->data_items = NULL;
    vm->data_count = 0;
    vm->data_ptr = 0;

    metadata_init(&vm->metadata_reg);
    vm->current_filename[0] = '\0';
    vm->start_line = 0.0;

    vm->error_trap_line = 0.0;
    vm->in_error_handler = false;
    vm->err_code = 0;
    vm->err_line = 0.0;
    vm->error_occurred_line = 0.0;
    vm->error_occurred_pos = NULL;
    vm->error_next_pos = NULL;

    register_core_statements(vm);
    vm->vfs = vfs_init(mem);
#if SUPPORT_NET
    vm->vnet = vnet_init(mem);
#else
    vm->vnet = NULL;
#endif
    vm->usb = usb_init(mem);
    vm->vcon = vcon_init();
#if SUPPORT_BIOS
    vm->bios_ram = (uint8_t *)calloc(1, 1024 * 1024);
    vm->bios = (MockBiosContext *)calloc(1, sizeof(MockBiosContext));
    if (!vm->bios_ram || !vm->bios) {
        if (vm->bios_ram) free(vm->bios_ram);
        if (vm->bios) free(vm->bios);
        vm->bios_ram = NULL;
        vm->bios = NULL;
        vm_shutdown(vm);
        return NULL;
    }
    if (vm->bios) {
        vm->bios->read_mem = bios_read_mem_cb;
        vm->bios->write_mem = bios_write_mem_cb;
        vm->bios->vdev_sleep = bios_vdev_sleep_cb;
        vm->bios->vdev_ioctl = bios_vdev_ioctl_cb;
        vm->bios->get_registers = bios_get_registers_cb;
        vm->bios->set_registers = bios_set_registers_cb;
        vm->bios->user_data = vm;
        if (vm->bios_ram) {
            vdev_bus_set_ram(vm->bios_ram, 1024 * 1024);
            vdev_bus_set_model(BIOS_MODEL_AT);
            mock_bios_init_mem(vm->bios, vm->bios_ram, 1024 * 1024, BIOS_MODEL_AT);
        }
    }
#else
    vm->bios_ram = NULL;
    vm->bios = NULL;
#endif
    vdev_bus_reset();
    vm->running = false;
    vm->jump_active = false;
    vm->current_line = 0.0;
    vm->current_pos = NULL;

    /* Initialize default key scan codes for F1 to F12 */
    for (int i = 1; i <= 10; ++i) {
        vm->key_code[i] = 58 + i;
    }
    vm->key_code[11] = 133;
    vm->key_code[12] = 134;

    return vm;
}

void vm_shutdown(VMContext *vm) {
    if (!vm) return;
    if (vm->alarms) {
        free(vm->alarms);
        vm->alarms = NULL;
    }
    if (vm->alarms_str) {
        free(vm->alarms_str);
        vm->alarms_str = NULL;
    }
    stmt_registry_shutdown(vm->stmt_reg);
    gosub_stack_shutdown(vm->gosub_stack);
    for_stack_shutdown(vm->for_stack);
    while_stack_shutdown(vm->while_stack);
    do_stack_shutdown(vm->do_stack);
    select_stack_shutdown(vm->select_stack);
    sub_stack_shutdown(vm->sub_stack);
    try_stack_shutdown(vm->try_stack);
    arr_shutdown(vm->arr);
    file_shutdown(vm->file);
#ifndef BASIC_LITE_BUILD
    vmem_shutdown(vm->vmem);
#endif
    vfs_shutdown(vm->vfs);
#if SUPPORT_NET
    vnet_shutdown(vm->vnet);
#endif
    usb_shutdown(vm->usb);
    vcon_shutdown(vm->vcon);
#if SUPPORT_BIOS
    if (vm->bios) {
        for (int i = 0; i < 20; i++) {
            if (vm->bios->dos_handles[i]) {
                fclose((FILE*)vm->bios->dos_handles[i]);
                vm->bios->dos_handles[i] = NULL;
            }
        }
        free(vm->bios);
    }
    if (vm->bios_ram) {
        free(vm->bios_ram);
    }
#endif
    if (vm->data_items) {
        free(vm->data_items);
    }
    if (vm->active_dialect) {
        dialect_free(vm->active_dialect);
    }
    free(vm);
}

void vm_stop(VMContext *vm) {
    if (vm) {
        vm->running = false;
    }
}

void vm_clear_aliases(VMContext *vm) {
    if (vm) {
        vm->alias_count = 0;
        vm->alias_expansion_depth = 0;
    }
}

void vm_register_alias(VMContext *vm, const char *name, const char *expansion) {
    if (!vm || !name || !expansion) return;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (strcasecmp(vm->aliases[i].name, name) == 0) {
            strncpy(vm->aliases[i].expansion, expansion, sizeof(vm->aliases[i].expansion) - 1);
            vm->aliases[i].expansion[sizeof(vm->aliases[i].expansion) - 1] = '\0';
            return;
        }
    }
    if (vm->alias_count >= 64) return;
    strncpy(vm->aliases[vm->alias_count].name, name, sizeof(vm->aliases[vm->alias_count].name) - 1);
    vm->aliases[vm->alias_count].name[sizeof(vm->aliases[vm->alias_count].name) - 1] = '\0';
    strncpy(vm->aliases[vm->alias_count].expansion, expansion, sizeof(vm->aliases[vm->alias_count].expansion) - 1);
    vm->aliases[vm->alias_count].expansion[sizeof(vm->aliases[vm->alias_count].expansion) - 1] = '\0';
    vm->alias_count++;
}

const char *vm_lookup_alias(VMContext *vm, const char *name) {
    if (!vm || !name) return NULL;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (strcasecmp(vm->aliases[i].name, name) == 0) {
            return vm->aliases[i].expansion;
        }
    }
    return NULL;
}

BppTypeRegistry *vm_get_types(VMContext *vm) {
    return vm ? &vm->type_reg : NULL;
}

int vm_get_last_mouse_click_btn(VMContext *vm) {
    return vm ? vm->last_mouse_click_btn : 0;
}

int vm_get_last_mouse_click_type(VMContext *vm) {
    return vm ? vm->last_mouse_click_type : 0;
}

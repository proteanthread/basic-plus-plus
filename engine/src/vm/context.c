// FILENAME: context.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for context.
//
// ---- Includes ----

#include "core/dialect.h"
#include "vm/vm.h"
#include "vm_internal.h"
#include "types/errors.h"
#include "memory/memory.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/struct.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/time/calendar.h"
#include "hal/hal.h"

#include "bios/bios.h"
#include "device/bgi_text.h"
#include "device/bgi_palette.h"
#include "device/bgi_gfx.h"
#include "device/bgi_autodetect.h"

MemoryContext   *vm_get_mem(VMContext *vm)   { return vm ? vm->mem : NULL; }
StringContext   *vm_get_str(VMContext *vm)   { return vm ? vm->str : NULL; }
VariableContext *vm_get_var(VMContext *vm)   { return vm ? vm->var : NULL; }
BiosContext     *vm_get_bios(VMContext *vm)  { return vm ? vm->bios : NULL; }
BppMetadataRegistry *vm_get_metadata(VMContext *vm) { return vm ? &vm->metadata_reg : NULL; }
#include "device/vprinter.h"

void vm_set_current_filename(VMContext *vm, const char *filename) {
    if (vm) {
        if (filename) {
            runtime_strncpy(vm->current_filename, filename, sizeof(vm->current_filename) - 1);
            vm->current_filename[sizeof(vm->current_filename) - 1] = '\0';
        } else {
            vm->current_filename[0] = '\0';
        }
    }
    vprinter_set_program_path(filename);
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


double vm_get_ti_offset(VMContext *vm) { return vm ? vm->ti_offset : 0.0; }
void vm_set_ti_offset(VMContext *vm, double val) { if (vm) vm->ti_offset = val; }
VfsContext      *vm_get_vfs(VMContext *vm)   { return vm ? vm->vfs : NULL; }
VNetContext     *vm_get_vnet(VMContext *vm)  { return vm ? vm->vnet : NULL; }
VConContext     *vm_get_vcon(VMContext *vm)  { return vm ? vm->vcon : NULL; }
VMemContext     *vm_get_vmem(VMContext *vm)  { return vm ? vm->vmem : NULL; }

GosubStack  *vm_get_gosub_stack(VMContext *vm)  { return vm ? vm->gosub_stack : NULL; }
ForStack    *vm_get_for_stack(VMContext *vm)    { return vm ? vm->for_stack : NULL; }
WhileStack  *vm_get_while_stack(VMContext *vm)  { return vm ? vm->while_stack : NULL; }
DoStack     *vm_get_do_stack(VMContext *vm)     { return vm ? vm->do_stack : NULL; }
SelectStack *vm_get_select_stack(VMContext *vm) { return vm ? vm->select_stack : NULL; }
SubStack    *vm_get_sub_stack(VMContext *vm)    { return vm ? vm->sub_stack : NULL; }
TryStack    *vm_get_try_stack(VMContext *vm)    { return vm ? vm->try_stack : NULL; }

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
int              vm_get_margin(VMContext *vm) { return (vm && vm->margin > 0) ? vm->margin : 80; }
void             vm_set_margin(VMContext *vm, int margin) { if (vm) vm->margin = margin; }
int              vm_get_zone_width(VMContext *vm) { return (vm && vm->zone_width > 0) ? vm->zone_width : 14; }
void             vm_set_zone_width(VMContext *vm, int zone_width) { if (vm) vm->zone_width = zone_width; }

void vm_reset_for_run(VMContext *vm) {
    if (!vm) return;

    // Clear ALL control flow stacks - prior program's stack frames
// contain dangling pointers into freed program line text
    gosub_stack_clear(vm->gosub_stack);
    for_stack_clear(vm->for_stack);
    while_stack_clear(vm->while_stack);
    do_stack_clear(vm->do_stack);
    select_stack_clear(vm->select_stack);
    sub_stack_clear(vm->sub_stack);
    try_stack_clear(vm->try_stack);

    // Clear aliases - each program should define its own
    vm_clear_aliases(vm);

    vm->with_stack_depth = 0;

    // Reset VM option flags to defaults
    vm->opt_eh = false;
    vm->opt_arithmetic_decimal = false;

    // NOTE: metadata_init() is NOT called here because
// metadata_pre_scan_program() already resets the registry at the
// start of each pre-scan pass. Calling it here would wipe out the
// freshly-scanned labels, docstrings, and metadata blocks.

    // Clear user-defined type registry
    struct_registry_init(&vm->type_reg);

    // Clear arrays - prior program's arrays may hold string references
// that become dangling after variable clear
    if (!vm->is_chaining) {
        arr_clear_all(vm->arr);
    }

    // Clear custom keyword registry to prevent stale LOAD FEATURE
// keywords from conflicting with ALIAS names in new programs
    keyword_clear_custom();

    // Clear spec registry (LOAD FEATURE state)
    spec_registry_init();

    // Clear library program lines loaded by LOAD FEATURE
    mem_lib_program_clear(vm->mem);

    // Clear/free loaded sound and image buffers
    vdev_sound_free_all();
    vdev_image_free_all();

    // Clear active error handling states
    vm->error_trap_line = 0.0;
    vm->in_error_handler = false;
    vm->err_code = 0;
    vm->err_line = 0.0;
    vm->error_occurred_line = 0.0;
    vm->error_occurred_pos = NULL;
    vm->error_next_pos = NULL;

    // Reset security sandbox and restrictions
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
    HalContext *hal = hal_get();
    VMContext *vm = NULL;
    if (hal && hal->mem.alloc) {
        vm = (VMContext *)hal->mem.alloc(sizeof(VMContext));
    }
    if (!vm) return NULL;
    runtime_memset(vm, 0, sizeof(VMContext));

    vm->mem = mem;
    vm->str = str;
    vm->var = var;
#ifndef BASIC_LITE_BUILD
    vm->vmem = vmem_init(var);
#endif
    vm->vdev = vdev;
    struct_registry_init(&vm->type_reg);
    session_init(&vm->session);

    vm->stmt_reg = stmt_registry_init(mem);
    if (!vm->stmt_reg) {
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        if (hal && hal->mem.free) hal->mem.free(vm);
        return NULL;
    }

    vm->gosub_stack = gosub_stack_init();
    if (!vm->gosub_stack) {
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        if (hal && hal->mem.free) hal->mem.free(vm);
        return NULL;
    }

    vm->for_stack = for_stack_init();
    if (!vm->for_stack) {
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
#ifndef BASIC_LITE_BUILD
        vmem_shutdown(vm->vmem);
#endif
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
        return NULL;
    }

    vm->active_proc[0] = '\0';
    vm->opt_eh = false;
    vm->jiffies_multiplier = 60.0;
    vm->margin = 80;
    vm->zone_width = 14;

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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
        if (hal && hal->mem.free) hal->mem.free(vm);
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
    vm->usb = NULL;
    vm->vcon = vcon_init();
    vm->bios = bios_create(BIOS_MODEL_IBM_PC);
    bgi_palette_init(bgi_palette_get_global());
    bgi_text_init(vm->bios, vm->vcon);
    bgi_gfx_init(vm->bios, BGI_get_global_context());
    vdev_bus_reset();
    vm->running = false;
    vm->jump_active = false;
    vm->current_line = 0.0;
    vm->current_pos = NULL;

    // Initialize default key scan codes for F1 to F12
    for (int i = 1; i <= 10; ++i) {
        vm->key_code[i] = 58 + i;
    }
    vm->key_code[11] = 133;
    vm->key_code[12] = 134;
    vm->break_enabled = false;
    vm->break_trap_line = 0.0;
    platform_setup_signals(vm);

    return vm;
}

const char *vm_map_source_pos(VMContext *vm, const char *pos) {
    if (!vm || !pos) return pos;
    if (vm->active_line_copy && vm->active_line_original &&
        pos >= vm->active_line_copy && pos <= vm->active_line_copy + runtime_strlen(vm->active_line_copy)) {
        ptrdiff_t offset = pos - vm->active_line_copy;
        return vm->active_line_original + offset;
    }
    return pos;
}

void vm_shutdown(VMContext *vm) {
    if (!vm) return;
    HalContext *hal = hal_get();
    if (vm->alarms) {
        if (hal && hal->mem.free) hal->mem.free(vm->alarms);
        vm->alarms = NULL;
    }
    if (vm->alarms_str) {
        if (hal && hal->mem.free) hal->mem.free(vm->alarms_str);
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
    vcon_shutdown(vm->vcon);
    if (vm->bios) {
        bgi_gfx_shutdown(vm->bios);
        bgi_text_shutdown(vm->bios);
        bios_destroy(vm->bios);
        vm->bios = NULL;
    }
    if (vm->data_items) {
        if (hal && hal->mem.free) hal->mem.free(vm->data_items);
        vm->data_items = NULL;
    }
    if (vm->line_scratch_buf) {
        if (hal && hal->mem.free) hal->mem.free(vm->line_scratch_buf);
        vm->line_scratch_buf = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(vm);
    }
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
        runtime_memset(vm->aliases, 0, sizeof(vm->aliases));
    }
}

void vm_register_alias(VMContext *vm, const char *name, const char *expansion) {
    if (!vm || !name || !expansion) return;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (runtime_strcasecmp(vm->aliases[i].name, name) == 0) {
            runtime_strncpy(vm->aliases[i].expansion, expansion, sizeof(vm->aliases[i].expansion) - 1);
            vm->aliases[i].expansion[sizeof(vm->aliases[i].expansion) - 1] = '\0';
            return;
        }
    }
    if (vm->alias_count >= 64) return;
    runtime_strncpy(vm->aliases[vm->alias_count].name, name, sizeof(vm->aliases[vm->alias_count].name) - 1);
    vm->aliases[vm->alias_count].name[sizeof(vm->aliases[vm->alias_count].name) - 1] = '\0';
    runtime_strncpy(vm->aliases[vm->alias_count].expansion, expansion, sizeof(vm->aliases[vm->alias_count].expansion) - 1);
    vm->aliases[vm->alias_count].expansion[sizeof(vm->aliases[vm->alias_count].expansion) - 1] = '\0';
    vm->alias_count++;
}

const char *vm_lookup_alias(VMContext *vm, const char *name) {
    if (!vm || !name) return NULL;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (runtime_strcasecmp(vm->aliases[i].name, name) == 0) {
            return vm->aliases[i].expansion;
        }
    }
    return NULL;
}

bool vm_remove_alias(VMContext *vm, const char *name) {
    if (!vm || !name) return false;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (runtime_strcasecmp(vm->aliases[i].name, name) == 0) {
            for (int j = i; j < vm->alias_count - 1; ++j) {
                vm->aliases[j] = vm->aliases[j + 1];
            }
            runtime_memset(&vm->aliases[vm->alias_count - 1], 0, sizeof(BppAlias));
            vm->alias_count--;
            return true;
        }
    }
    return false;
}

int vm_get_alias_count(VMContext *vm) {
    return vm ? vm->alias_count : 0;
}

void vm_clear_operator_aliases(VMContext *vm) {
    if (vm) {
        vm->oper_alias_count = 0;
        runtime_memset(vm->oper_aliases, 0, sizeof(vm->oper_aliases));
    }
}

void vm_register_operator_alias(VMContext *vm, const char *op_name, const char *expansion) {
    if (!vm || !op_name || !expansion) return;
    for (int i = 0; i < vm->oper_alias_count; ++i) {
        if (runtime_strcasecmp(vm->oper_aliases[i].name, op_name) == 0) {
            runtime_strncpy(vm->oper_aliases[i].expansion, expansion, sizeof(vm->oper_aliases[i].expansion) - 1);
            vm->oper_aliases[i].expansion[sizeof(vm->oper_aliases[i].expansion) - 1] = '\0';
            return;
        }
    }
    if (vm->oper_alias_count >= 64) return;
    runtime_strncpy(vm->oper_aliases[vm->oper_alias_count].name, op_name, sizeof(vm->oper_aliases[vm->oper_alias_count].name) - 1);
    vm->oper_aliases[vm->oper_alias_count].name[sizeof(vm->oper_aliases[vm->oper_alias_count].name) - 1] = '\0';
    runtime_strncpy(vm->oper_aliases[vm->oper_alias_count].expansion, expansion, sizeof(vm->oper_aliases[vm->oper_alias_count].expansion) - 1);
    vm->oper_aliases[vm->oper_alias_count].expansion[sizeof(vm->oper_aliases[vm->oper_alias_count].expansion) - 1] = '\0';
    vm->oper_alias_count++;
}

const char *vm_lookup_operator_alias(VMContext *vm, const char *op_name) {
    if (!vm || !op_name) return NULL;
    for (int i = 0; i < vm->oper_alias_count; ++i) {
        if (runtime_strcasecmp(vm->oper_aliases[i].name, op_name) == 0) {
            return vm->oper_aliases[i].expansion;
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


BppDialect *vm_get_active_dialect(VMContext *vm) {
    return vm ? vm->active_dialect : NULL;
}
void vm_set_active_dialect(VMContext *vm, BppDialect *d) {
    if (vm) vm->active_dialect = d;
}
BppDialect *vm_get_defining_dialect(VMContext *vm) {
    return vm ? vm->defining_dialect : NULL;
}
void vm_set_defining_dialect(VMContext *vm, BppDialect *d) {
    if (vm) vm->defining_dialect = d;
}

bool vm_is_in_test(VMContext *vm) {
    return vm ? vm->in_test : false;
}

void vm_set_in_test(VMContext *vm, bool in_test, const char *test_name) {
    if (!vm) return;
    vm->in_test = in_test;
    if (test_name) {
        runtime_strncpy(vm->test_name, test_name, sizeof(vm->test_name) - 1);
        vm->test_name[sizeof(vm->test_name) - 1] = '\0';
    } else {
        vm->test_name[0] = '\0';
    }
    if (in_test) {
        vm->test_pass = 0;
        vm->test_fail = 0;
        vm->test_total = 0;
    }
}


void vm_get_test_metrics(VMContext *vm, int *pass, int *fail, int *total) {
    if (pass) *pass = vm ? vm->test_pass : 0;
    if (fail) *fail = vm ? vm->test_fail : 0;
    if (total) *total = vm ? vm->test_total : 0;
}

void vm_inc_test_pass(VMContext *vm) {
    if (vm) vm->test_pass++;
}

void vm_inc_test_fail(VMContext *vm) {
    if (vm) vm->test_fail++;
}

void vm_inc_test_total(VMContext *vm) {
    if (vm) vm->test_total++;
}

bool vm_get_debug_active(VMContext *vm) {
    return vm ? vm->debug_active : false;
}

void vm_set_debug_active(VMContext *vm, bool active) {
    if (vm) vm->debug_active = active;
}

BppSessionContext *vm_get_session(VMContext *vm) {
    return vm ? &vm->session : NULL;
}

void vm_set_timeout(VMContext *vm, double timeout_ms) {
    if (vm) {
        vm->timeout_ms = timeout_ms;
        vm->start_time_ms = platform_get_uptime() * 1000.0;
        vm->watchdog_enabled = (timeout_ms > 0.0 || vm->max_cycles > 0);
    }
}

double vm_get_timeout(VMContext *vm) {
    return vm ? vm->timeout_ms : 0.0;
}

void vm_set_max_cycles(VMContext *vm, uint64_t max_cycles) {
    if (vm) {
        vm->max_cycles = max_cycles;
        vm->cycle_count = 0;
        vm->watchdog_enabled = (max_cycles > 0 || vm->timeout_ms > 0.0);
    }
}

uint64_t vm_get_max_cycles(VMContext *vm) {
    return vm ? vm->max_cycles : 0;
}

void vm_reset_watchdog(VMContext *vm) {
    if (vm) {
        vm->start_time_ms = platform_get_uptime() * 1000.0;
        vm->cycle_count = 0;
    }
}

bool vm_check_watchdog(VMContext *vm, BppError *out_err) {
    if (!vm) return true;
    if (vm->break_triggered) {
        vm->break_triggered = false;
        if (out_err) {
            out_err->code = ERR_DEVICE_IO_ERROR;
            out_err->message = "Break";
        }
        return false;
    }
    if (!vm->watchdog_enabled) return true;
    vm->cycle_count++;
    if (vm->max_cycles > 0 && vm->cycle_count > vm->max_cycles) {
        if (out_err) {
            out_err->code = ERR_DEVICE_TIMEOUT;
            out_err->message = "Execution limit exceeded: cycle watchdog timeout";
        }
        return false;
    }
    if (vm->timeout_ms > 0.0 && ((vm->cycle_count & 1023) == 0)) {
        double now = platform_get_uptime() * 1000.0;
        if (now - vm->start_time_ms > vm->timeout_ms) {
            if (out_err) {
                out_err->code = ERR_DEVICE_TIMEOUT;
                out_err->message = "Execution timeout exceeded: watchdog timer triggered";
            }
            return false;
        }
    }
    return true;
}


/**
 * @file context.c
 * @brief VM Context lifecycle, initialization, teardown, and accessor methods for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vm_create()`, `vm_destroy()`, `vm_reset()`, `vm_init()`, and accessor functions (`vm_get_mem`, `vm_get_str`, `vm_get_var`, etc.) for `VMContext`.
 *
 * 2. WHY IT EXISTS:
 * Serves as the central state container for the BASIC++ Virtual Machine, holding pointers to subsystem contexts (memory, variables, strings, I/O devices, graphics, sockets).
 *
 * 3. WHY IT WORKS THIS WAY:
 * Allocates `VMContext` via `calloc()` (zero-initialization), initializes memory manager (`mem_create`), variable table, string table, file table, AST evaluator, virtual device bus, and security sandbox policies.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "vm/vm.h", "vm_internal.h",
 * "core/dialect.h", "memory/memory.h", "stmt/stmt.h", "device/vdev.h", "runtime/metadata.h", "runtime/vfs.h",
 * "runtime/vnet.h", "runtime/file.h", "device/vcon.h", "device/bus.h", "runtime/spec.h", "security/security.h",
 * "eval/eval.h", "debug/logger.h", "runtime/variables.h", "platform/platform.h", "core/struct.h", "bios/bios.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs'). Uses `#ifndef BASIC_LITE_BUILD` to conditionally omit segmented memory `vmem`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new subsystem context pointers or configuration flags to `VMContext` struct in `engine/include/vm/vm.h` and initialize them here in `vm_create()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory zero-initialization invariant and cleanup ordering in `vm_destroy()` (must release variables/strings before memory pool destruction).
 *
 * 8. WHAT TO EXPECT:
 * `vm_create()` returns a fully-initialized `VMContext*` allocated from heap or NULL on memory allocation failure.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify allocation order in `vm_create()` and teardown sequence in `vm_destroy()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Memory allocation requested size is within system available RAM footprint limits (640MB for baspp, 384MB for bpp, 64MB for bs).
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. All sub-allocations zero-initialized. 64-bit pointer safe (`uintptr_t`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/memory.c
 * - engine/src/runtime/variables.c
 * - engine/src/runtime/strings.c
 * - engine/src/runtime/file.c
 * - engine/src/device/vdev.c
 * - engine/src/device/bus.c
 * - engine/src/security/security.c
 * - engine/src/bios/bios.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/include/vm/vm_internal.h
 * - engine/include/core/dialect.h
 * - engine/include/memory/memory.h
 * - engine/include/device/vdev.h
 * - engine/include/security/security.h
 */

#include "core/dialect.h"
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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


#include "bios/bios.h"

MemoryContext   *vm_get_mem(VMContext *vm)   { return vm ? vm->mem : NULL; }
StringContext   *vm_get_str(VMContext *vm)   { return vm ? vm->str : NULL; }
VariableContext *vm_get_var(VMContext *vm)   { return vm ? vm->var : NULL; }
BiosContext     *vm_get_bios(VMContext *vm)  { return vm ? vm->bios : NULL; }
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
    vm->usb = NULL;
    vm->vcon = vcon_init();
    vm->bios = bios_create(BIOS_MODEL_IBM_PC);
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
    vcon_shutdown(vm->vcon);
    if (vm->bios) {
        bios_destroy(vm->bios);
        vm->bios = NULL;
    }
    if (vm->data_items) {
        free(vm->data_items);
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
        memset(vm->aliases, 0, sizeof(vm->aliases));
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

bool vm_remove_alias(VMContext *vm, const char *name) {
    if (!vm || !name) return false;
    for (int i = 0; i < vm->alias_count; ++i) {
        if (strcasecmp(vm->aliases[i].name, name) == 0) {
            for (int j = i; j < vm->alias_count - 1; ++j) {
                vm->aliases[j] = vm->aliases[j + 1];
            }
            memset(&vm->aliases[vm->alias_count - 1], 0, sizeof(BppAlias));
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
        memset(vm->oper_aliases, 0, sizeof(vm->oper_aliases));
    }
}

void vm_register_operator_alias(VMContext *vm, const char *op_name, const char *expansion) {
    if (!vm || !op_name || !expansion) return;
    for (int i = 0; i < vm->oper_alias_count; ++i) {
        if (strcasecmp(vm->oper_aliases[i].name, op_name) == 0) {
            strncpy(vm->oper_aliases[i].expansion, expansion, sizeof(vm->oper_aliases[i].expansion) - 1);
            vm->oper_aliases[i].expansion[sizeof(vm->oper_aliases[i].expansion) - 1] = '\0';
            return;
        }
    }
    if (vm->oper_alias_count >= 64) return;
    strncpy(vm->oper_aliases[vm->oper_alias_count].name, op_name, sizeof(vm->oper_aliases[vm->oper_alias_count].name) - 1);
    vm->oper_aliases[vm->oper_alias_count].name[sizeof(vm->oper_aliases[vm->oper_alias_count].name) - 1] = '\0';
    strncpy(vm->oper_aliases[vm->oper_alias_count].expansion, expansion, sizeof(vm->oper_aliases[vm->oper_alias_count].expansion) - 1);
    vm->oper_aliases[vm->oper_alias_count].expansion[sizeof(vm->oper_aliases[vm->oper_alias_count].expansion) - 1] = '\0';
    vm->oper_alias_count++;
}

const char *vm_lookup_operator_alias(VMContext *vm, const char *op_name) {
    if (!vm || !op_name) return NULL;
    for (int i = 0; i < vm->oper_alias_count; ++i) {
        if (strcasecmp(vm->oper_aliases[i].name, op_name) == 0) {
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
        strncpy(vm->test_name, test_name, sizeof(vm->test_name) - 1);
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

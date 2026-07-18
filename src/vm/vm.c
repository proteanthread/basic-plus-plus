/**
 * @file vm.c
 * @brief Virtual Machine core execution engine implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the VM initialization, shutdown, program run loop, and accessor functions.
 * - Why it exists: Serves as the central execution manager. Runs program lines sequentially or
 *   dispatches single commands from the REPL.
 * - Why it works this way: By utilizing standard pointer redirects and a loop state check,
 *   we execute statements iteratively. Jumps overwrite line index pointers which are checked on each iteration,
 *   satisfying the "no stack recursion" requirement.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Pre-execution validation steps, statement dispatch flags.
 * - What cannot be changed: Dynamic line lookup loop ordering (must maintain sequential fallback).
 * - What to expect: Calling vm_run_program clears variables first and runs lines starting at the minimum number.
 * - What to do if something breaks: If jumps skip lines or loop infinitely, check the current_line index updates.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Stored lines do not disappear during active execution loops.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add line tracing (TRON/TROFF) or step execution debugging.
 * - How to write external extensions: External plugins register statements that hook directly into the registry.
 */

#include "bpp_vm.h"
#include "bpp_stmt.h"
#include "bpp_vdev.h"
#include "bpp_config.h"
#include "bpp_metadata.h"
#include "bpp_vfs.h"
#include "bpp_vnet.h"
#ifndef BPP_LITE_BUILD
#include "bpp_segmented_mem.h"
#endif
#include "bpp_usb.h"
#include "bpp_file.h"
#include "bpp_vcon.h"
#include "bpp_bus.h"
#include "bpp_mock_bios.h"
#include "bpp_spec.h"
#include "bpp_security.h"
#include "bpp_eval.h"
#include "bpp_logger.h"
#include "bpp_variables.h"
#include "bpp_platform.h"
#include "bpp_dialect.h"
#include "bpp_struct.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PARAMS 8

void        vm_register_alias(VMContext *vm, const char *name, const char *expansion);
const char *vm_lookup_alias(VMContext *vm, const char *name);
void        vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg);
BValue      eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);
BValue      invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);

/* Forward declare stack helpers from vm_stack.c */
typedef struct GosubStack GosubStack;
GosubStack *gosub_stack_init(void);
void gosub_stack_shutdown(GosubStack *stack);
void gosub_stack_clear(GosubStack *stack);

extern bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text);
bool gosub_stack_push(GosubStack *stack, BppLineNumber line, const char *pos);
bool gosub_stack_pop(GosubStack *stack, BppLineNumber *out_line, const char **out_pos);

typedef struct ForStack ForStack;
ForStack *for_stack_init(void);
void for_stack_shutdown(ForStack *stack);
void for_stack_clear(ForStack *stack);
bool for_stack_push(ForStack *stack, const char *var_name, double target, double step, BppLineNumber line, const char *pos);
bool for_stack_pop(ForStack *stack, const char *var_name, BppForFrame *out_frame);
bool for_stack_peek(ForStack *stack, const char *var_name, BppForFrame *out_frame);
bool for_stack_update(ForStack *stack, const char *var_name, double target, double step, const char *next_range_pos);

typedef struct WhileStack WhileStack;
WhileStack *while_stack_init(void);
void while_stack_shutdown(WhileStack *stack);
void while_stack_clear(WhileStack *stack);
bool while_stack_push(WhileStack *stack, BppLineNumber line, const char *pos);
bool while_stack_pop(WhileStack *stack, BppLineNumber *out_line, const char **out_pos);
bool while_stack_peek(WhileStack *stack, BppLineNumber *out_line, const char **out_pos);

typedef struct DoStack DoStack;
DoStack *do_stack_init(void);
void do_stack_shutdown(DoStack *stack);
void do_stack_clear(DoStack *stack);
bool do_stack_push(DoStack *stack, BppLineNumber line, const char *pos);
bool do_stack_pop(DoStack *stack, BppLineNumber *out_line, const char **out_pos);
bool do_stack_peek(DoStack *stack, BppLineNumber *out_line, const char **out_pos);

typedef struct {
    char name[64];
    char expansion[256];
} BppAlias;

struct VMContext {
    MemoryContext   *mem;
    StringContext   *str;
    VariableContext *var;
    VDevContext     *vdev;
    StmtRegistry    *stmt_reg;
    BppAlias         aliases[64];
    int              alias_count;
    int              alias_expansion_depth;
    GosubStack      *gosub_stack;
    ForStack        *for_stack;
    WhileStack      *while_stack;
    DoStack         *do_stack;
    SelectStack     *select_stack;
    SubStack        *sub_stack;
    TryStack        *try_stack;
    char             active_proc[256];
    ArrayContext    *arr;
    FileContext     *file;
    VMemContext     *vmem;
    VfsContext      *vfs;
    VNetContext     *vnet;
    UsbContext      *usb;
    VConContext     *vcon;
    MockBiosContext *bios;
    uint8_t         *bios_ram;
    MockBiosRegs     regs;
    bool             opt_eh;
    bool             opt_arithmetic_decimal;
    BppDataPosition *data_items;
    int              data_count;
    int              data_ptr;

    /* Debugger Hooks */
    void (*debug_hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data);
    void *debug_user_data;

    /* Error trapping state */
    BppLineNumber    error_trap_line;
    bool             in_error_handler;
    int              err_code;
    BppLineNumber    err_line;
    BppLineNumber    error_occurred_line;
    const char      *error_occurred_pos;
    const char      *error_next_pos;

    /* Execution state pointers */
    BppLineNumber    current_line;
    const char      *current_pos;
    BppLineNumber    next_line;
    const char      *next_pos;
    bool             jump_active;
    bool             running;
    bool             exit_requested;
    bool             debug_single_step;
    int              eval_depth;      /* Guard against recursive eval_expression overflow */
    const char      *current_stmt_pos;

    /* Error reporting context */
    BppError         last_error;

    /* Metadata Registry for namespaces, global labels, and docstrings */
    BppMetadataRegistry metadata_reg;

    /* Current executing filename for cross-file jumps */
    char current_filename[256];

    /* Initial starting line for tasks */
    BppLineNumber start_line;

    /* Pointer mapping for execution text copies */
    const char      *active_line_original;
    const char      *active_line_copy;
    BppDialect      *active_dialect;
    BppDialect      *defining_dialect;
    bool             in_preprocessor_hook;

    /* WITH block context stack */
    char             with_stack[8][256];
    int              with_stack_depth;

    /* Event trapping state */
    int              timer_state; /* 0=OFF, 1=ON, 2=STOP */
    double           timer_interval;
    double           timer_last_trigger;
    BppLineNumber    timer_gosub_line;
    bool             timer_pending;
    bool             in_timer_handler;

    int              key_state[11]; /* index 1 to 10 */
    int              key_code[11];
    BppLineNumber    key_gosub_line[11];
    bool             key_pending[11];
    bool             in_key_handler[11];

    int              play_state;
    int              play_note_threshold;
    BppLineNumber    play_gosub_line;
    bool             play_pending;
    bool             in_play_handler;
    BppTypeRegistry  type_reg;
    double           last_rnd;
};

/* Accessors */
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

double vm_get_last_rnd(VMContext *vm) { return vm ? vm->last_rnd : 0.0; }
void vm_set_last_rnd(VMContext *vm, double val) { if (vm) vm->last_rnd = val; }
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
int              vm_get_eval_depth(VMContext *vm) { return vm ? vm->eval_depth : 0; }
void             vm_inc_eval_depth(VMContext *vm) { if (vm) vm->eval_depth++; }
void             vm_dec_eval_depth(VMContext *vm) { if (vm) vm->eval_depth--; }

/**
 * @brief Reset all accumulated execution state for a new RUN "file" load.
 *
 * What it does: Clears all control flow stacks, aliases, custom keywords,
 * spec registry, library program lines, struct/type registry, metadata,
 * and active dialect. This ensures a clean slate for the new program.
 *
 * Why it exists: When chaining programs via RUN "file", accumulated state
 * from prior programs can cause crashes. Stale custom keywords from
 * LOAD FEATURE collide with ALIAS names, stale stack frames contain
 * dangling pointers to freed program text, and stale library SUB
 * procedures interfere with dispatch.
 *
 * Why it works this way: The function is a single point of cleanup called
 * by the RUN handler in stmt_program.c. All cleanup is grouped here to
 * prevent accidental omissions when new state is added to the VM.
 *
 * Assumptions: Called AFTER vm_load_program_file() has already cleared
 * and reloaded program memory, but BEFORE vm_build_data_table().
 *
 * Portability concerns: None. Platform-independent.
 *
 * Future expansions: When new persistent VM state is added, add its
 * cleanup call here to maintain correct RUN chaining behavior.
 */
/* File scope declarations for virtual device cleanup */
extern void vdev_sound_free_all(void);
extern void vdev_image_free_all(void);

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
    arr_clear_all(vm->arr);

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

/* Error handling */
void vm_set_error(VMContext *vm, int code, const char *msg) {
    if (vm) {
        vm->last_error.code = code;
        vm->last_error.category = ERR_CAT_RUNTIME;
        vm->last_error.message = msg;
        vm->last_error.line = vm->current_line;
        vm->last_error.file = "vm.c";
    }
}

void vm_clear_error(VMContext *vm) {
    if (vm) {
        memset(&vm->last_error, 0, sizeof(BppError));
    }
}

bool vm_has_error(VMContext *vm) {
    return vm ? (vm->last_error.code != 0) : false;
}

BppError vm_get_error(VMContext *vm) {
    if (vm) return vm->last_error;
    BppError null_err;
    memset(&null_err, 0, sizeof(null_err));
    return null_err;
}

/* Control flow modifiers */
void vm_jump(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) {
        vm->next_line = line;
        vm->next_pos = pos;
        vm->jump_active = true;
    }
}

static const char *vm_map_copy_to_original(VMContext *vm, const char *pos) {
    if (!vm || !pos || !vm->active_line_original || !vm->active_line_copy) {
        return pos;
    }
    size_t len = strlen(vm->active_line_copy);
    if (pos >= vm->active_line_copy && pos <= vm->active_line_copy + len) {
        ptrdiff_t offset = pos - vm->active_line_copy;
        return vm->active_line_original + offset;
    }
    return pos;
}

bool vm_gosub_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? gosub_stack_push(vm->gosub_stack, line, pos) : false;
}

bool vm_gosub_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? gosub_stack_pop(vm->gosub_stack, out_line, out_pos) : false;
}

bool vm_for_push(VMContext *vm, const char *var_name, double target, double step, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? for_stack_push(vm->for_stack, var_name, target, step, line, pos) : false;
}

bool vm_for_update(VMContext *vm, const char *var_name, double target, double step, const char *next_range_pos) {
    if (vm && next_range_pos) next_range_pos = vm_map_copy_to_original(vm, next_range_pos);
    return vm ? for_stack_update(vm->for_stack, var_name, target, step, next_range_pos) : false;
}

bool vm_for_pop(VMContext *vm, const char *var_name, BppForFrame *out_frame) {
    return vm ? for_stack_pop(vm->for_stack, var_name, out_frame) : false;
}

bool vm_for_peek(VMContext *vm, const char *var_name, BppForFrame *out_frame) {
    return vm ? for_stack_peek(vm->for_stack, var_name, out_frame) : false;
}

bool vm_while_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? while_stack_push(vm->while_stack, line, pos) : false;
}

bool vm_while_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? while_stack_pop(vm->while_stack, out_line, out_pos) : false;
}

bool vm_while_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? while_stack_peek(vm->while_stack, out_line, out_pos) : false;
}

bool vm_do_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? do_stack_push(vm->do_stack, line, pos) : false;
}

bool vm_do_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? do_stack_pop(vm->do_stack, out_line, out_pos) : false;
}

bool vm_do_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? do_stack_peek(vm->do_stack, out_line, out_pos) : false;
}

bool vm_select_push(VMContext *vm, BValue val, bool matched, BppLineNumber line, const char *pos) {
    if (!vm) return false;
    pos = vm_map_copy_to_original(vm, pos);
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
    return select_stack_push(vm->select_stack, val, matched, line, pos);
}

bool vm_select_pop(VMContext *vm, BppSelectFrame *out_frame) {
    if (!vm) return false;
    BppSelectFrame frame;
    bool ok = select_stack_pop(vm->select_stack, &frame);
    if (ok) {
        if (out_frame) *out_frame = frame;
        if (frame.val.type == VAL_STRING && frame.val.as.string) {
            str_release(vm->str, frame.val.as.string);
        }
    }
    return ok;
}

bool vm_select_peek(VMContext *vm, BppSelectFrame *out_frame) {
    return vm ? select_stack_peek(vm->select_stack, out_frame) : false;
}

bool vm_sub_push(VMContext *vm, const char *name, BppLineNumber line, const char *pos, bool is_func) {
    if (!vm) return false;
    pos = vm_map_copy_to_original(vm, pos);
    bool ok = sub_stack_push(vm->sub_stack, name, line, pos, is_func);
    if (ok) {
        strncpy(vm->active_proc, name, sizeof(vm->active_proc) - 1);
        vm->active_proc[sizeof(vm->active_proc) - 1] = '\0';
    }
    return ok;
}

bool vm_sub_pop(VMContext *vm, BppSubFrame *out_frame) {
    if (!vm) return false;
    BppSubFrame frame;
    bool ok = sub_stack_pop(vm->sub_stack, &frame);
    if (ok) {
        if (out_frame) *out_frame = frame;
        /* Clear local variables for popped sub scope */
        var_clear_scope(vm->var, frame.name);

        BppSubFrame prev;
        if (sub_stack_peek(vm->sub_stack, &prev)) {
            strncpy(vm->active_proc, prev.name, sizeof(vm->active_proc) - 1);
            vm->active_proc[sizeof(vm->active_proc) - 1] = '\0';
        } else {
            vm->active_proc[0] = '\0';
        }
    }
    return ok;
}

bool vm_sub_peek(VMContext *vm, BppSubFrame *out_frame) {
    return vm ? sub_stack_peek(vm->sub_stack, out_frame) : false;
}

const char *vm_get_active_proc(VMContext *vm) {
    return vm ? vm->active_proc : "";
}

void vm_halt(VMContext *vm) {
    if (vm) {
        vm->running = false;
    }
}

/**
 * vm_request_exit - Signal the REPL to terminate the interpreter.
 *
 * Called by the SYSTEM and EXIT statement handlers.
 * The REPL checks vm_exit_requested() after each vm_execute_line().
 */
void vm_request_exit(VMContext *vm) {
    if (vm) {
        vm->exit_requested = true;
    }
}

/**
 * vm_exit_requested - Check if the interpreter should exit.
 */
bool vm_exit_requested(VMContext *vm) {
    return vm ? vm->exit_requested : false;
}

/* Statement registration bootstrap declarations */
BppError stmt_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_let_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_input_handler(VMContext *vm, LexerContext *lex);
BppError stmt_goto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_return_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_handler(VMContext *vm, LexerContext *lex);
BppError stmt_system_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rem_handler(VMContext *vm, LexerContext *lex);
BppError stmt_if_handler(VMContext *vm, LexerContext *lex);
BppError stmt_list_handler(VMContext *vm, LexerContext *lex);
BppError stmt_run_handler(VMContext *vm, LexerContext *lex);
BppError stmt_new_handler(VMContext *vm, LexerContext *lex);
BppError stmt_for_handler(VMContext *vm, LexerContext *lex);
BppError stmt_next_handler(VMContext *vm, LexerContext *lex);
BppError stmt_while_handler(VMContext *vm, LexerContext *lex);
BppError stmt_wend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_do_handler(VMContext *vm, LexerContext *lex);
BppError stmt_loop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dim_handler(VMContext *vm, LexerContext *lex);
BppError stmt_erase_handler(VMContext *vm, LexerContext *lex);
BppError stmt_option_handler(VMContext *vm, LexerContext *lex);
BppError stmt_data_handler(VMContext *vm, LexerContext *lex);
BppError stmt_read_handler(VMContext *vm, LexerContext *lex);
BppError stmt_restore_handler(VMContext *vm, LexerContext *lex);
BppError stmt_on_handler(VMContext *vm, LexerContext *lex);
BppError stmt_timer_handler(VMContext *vm, LexerContext *lex);
BppError stmt_key_handler(VMContext *vm, LexerContext *lex);
BppError stmt_try_handler(VMContext *vm, LexerContext *lex);
BppError stmt_with_handler(VMContext *vm, LexerContext *lex);
BppError stmt_catch_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_try_handler(VMContext *vm, LexerContext *lex);
BppError stmt_throw_handler(VMContext *vm, LexerContext *lex);
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dialect_handler(VMContext *vm, LexerContext *lex);
BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resume_handler(VMContext *vm, LexerContext *lex);
BppError stmt_load_handler(VMContext *vm, LexerContext *lex);
BppError stmt_save_handler(VMContext *vm, LexerContext *lex);
BppError stmt_merge_handler(VMContext *vm, LexerContext *lex);
BppError stmt_selftest_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shell_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex);
#if BPP_SUPPORT_OOP
BppError stmt_type_handler(VMContext *vm, LexerContext *lex);
BppError stmt_class_handler(VMContext *vm, LexerContext *lex);
BppError stmt_enum_handler(VMContext *vm, LexerContext *lex);
#endif

/* Phase 3 & 4 statement handlers */
BppError stmt_open_handler(VMContext *vm, LexerContext *lex);
BppError stmt_field_handler(VMContext *vm, LexerContext *lex);
BppError stmt_close_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_put_handler(VMContext *vm, LexerContext *lex);
BppError stmt_seek_handler(VMContext *vm, LexerContext *lex);
BppError stmt_files_handler(VMContext *vm, LexerContext *lex);
BppError stmt_kill_handler(VMContext *vm, LexerContext *lex);
BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex);

/* Phase 11b and 11c statement handlers */
BppError stmt_noise_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex);
BppError stmt_title_handler(VMContext *vm, LexerContext *lex);
BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_resize_handler(VMContext *vm, LexerContext *lex);
BppError stmt_icon_handler(VMContext *vm, LexerContext *lex);
BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex);
BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex);
BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mkdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_name_handler(VMContext *vm, LexerContext *lex);
BppError stmt_dir_handler(VMContext *vm, LexerContext *lex);
BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_environ_handler(VMContext *vm, LexerContext *lex);
BppError stmt_lock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ioctl_handler(VMContext *vm, LexerContext *lex);
BppError stmt_devices_handler(VMContext *vm, LexerContext *lex);
BppError stmt_select_handler(VMContext *vm, LexerContext *lex);
BppError stmt_case_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex);
BppError stmt_function_handler(VMContext *vm, LexerContext *lex);
BppError stmt_call_handler(VMContext *vm, LexerContext *lex);
BppError stmt_declare_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex);
#ifndef BPP_LITE_BUILD
BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);
BppError stmt_color_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_auto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_clear_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_locate_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shared_handler(VMContext *vm, LexerContext *lex);
BppError stmt_beep_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_brun_handler(VMContext *vm, LexerContext *lex);
BppError stmt_line_handler(VMContext *vm, LexerContext *lex);
#ifndef BPP_LITE_BUILD
BppError stmt_circle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_pset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_preset_handler(VMContext *vm, LexerContext *lex);
BppError stmt_cls_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paint_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sound_handler(VMContext *vm, LexerContext *lex);
BppError stmt_play_handler(VMContext *vm, LexerContext *lex);
#endif
#if BPP_SUPPORT_EDITOR
BppError stmt_edit_handler(VMContext *vm, LexerContext *lex);
#endif
#ifndef BPP_LITE_BUILD
BppError stmt_security_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_module_handler(VMContext *vm, LexerContext *lex);
BppError stmt_task_handler(VMContext *vm, LexerContext *lex);
BppError stmt_help_handler(VMContext *vm, LexerContext *lex);
#if BPP_SUPPORT_MAT
BppError stmt_mat_handler(VMContext *vm, LexerContext *lex);
#ifndef BPP_LITE_BUILD
BppError stmt_arrayext_handler(VMContext *vm, LexerContext *lex);
#endif
#endif
#ifndef BPP_LITE_BUILD
BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);
#endif
#if BPP_SUPPORT_EDITOR
BppError stmt_renum_handler(VMContext *vm, LexerContext *lex);
BppError stmt_delete_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex);
#if BPP_SUPPORT_NET
BppError stmt_mount_handler(VMContext *vm, LexerContext *lex);
BppError stmt_umount_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_chvt_handler(VMContext *vm, LexerContext *lex);
#if BPP_SUPPORT_NET
BppError stmt_net_handler(VMContext *vm, LexerContext *lex);
#endif
BppError stmt_out_handler(VMContext *vm, LexerContext *lex);
BppError stmt_poke_handler(VMContext *vm, LexerContext *lex);
#if BPP_SUPPORT_BIOS
BppError stmt_bios_handler(VMContext *vm, LexerContext *lex);
#endif
#if BPP_SUPPORT_GEMINI
BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex);
#endif

#ifndef BPP_LITE_BUILD
BppError stmt_gr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex);
BppError stmt_plot_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex);
BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex);

/* Transaction statement handlers */
BppError stmt_txn_handler(VMContext *vm, LexerContext *lex);
BppError stmt_atomic_handler(VMContext *vm, LexerContext *lex);
BppError stmt_commit_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rollback_handler(VMContext *vm, LexerContext *lex);
BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex);
BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex);
BppError stmt_border_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ink_handler(VMContext *vm, LexerContext *lex);
BppError stmt_paper_handler(VMContext *vm, LexerContext *lex);

/* BGI (BASIC++ Graphics Interface) statement handlers */
BppError stmt_initgraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_closegraph_handler(VMContext *vm, LexerContext *lex);
BppError stmt_putpixel_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bar_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ellipse_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rectangle_handler(VMContext *vm, LexerContext *lex);
BppError stmt_outtextxy_handler(VMContext *vm, LexerContext *lex);
BppError stmt_palette_handler(VMContext *vm, LexerContext *lex);
#endif

BppError stmt_pause_handler(VMContext *vm, LexerContext *lex);
BppError stmt_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_sys_handler(VMContext *vm, LexerContext *lex);
BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_assert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex);
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_break_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex);

/* Register all core statements */
extern BppError stmt_swap_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_local_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_static_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_redim_handler(VMContext *vm, LexerContext *lex);

static void register_core_statements(VMContext *vm) {
    stmt_register(vm->stmt_reg, KW_PRINT,  stmt_print_handler,  "PRINT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LET,    stmt_let_handler,    "LET",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LSET,   stmt_lset_handler,   "LSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RSET,   stmt_rset_handler,   "RSET",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SWAP,   stmt_swap_handler,   "SWAP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INPUT,  stmt_input_handler,  "INPUT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEMAND, stmt_input_handler,  "DEMAND", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GOTO,   stmt_goto_handler,   "GOTO",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_GOSUB,  stmt_gosub_handler,  "GOSUB",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RETURN, stmt_return_handler, "RETURN", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_END,    stmt_end_handler,    "END",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SYSTEM, stmt_system_handler, "SYSTEM", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHELL,  stmt_shell_handler,  "SHELL",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENVIRON, stmt_environ_handler, "ENVIRON", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BYE,    stmt_bye_handler,    "BYE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STOP,   stmt_stop_handler,   "STOP",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_EXIT,   stmt_bye_handler,    "EXIT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_REM,    stmt_rem_handler,    "REM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IF,     stmt_if_handler,     "IF",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LIST,   stmt_list_handler,   "LIST",   STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_RUN,    stmt_run_handler,    "RUN",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_NEW,    stmt_new_handler,    "NEW",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_FOR,    stmt_for_handler,    "FOR",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_NEXT,   stmt_next_handler,   "NEXT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WHILE,  stmt_while_handler,  "WHILE",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WEND,   stmt_wend_handler,   "WEND",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DO,     stmt_do_handler,     "DO",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_LOOP,   stmt_loop_handler,   "LOOP",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DIM,    stmt_dim_handler,    "DIM",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCAL,  stmt_local_handler,  "LOCAL",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATIC, stmt_static_handler, "STATIC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_REDIM,  stmt_redim_handler,  "REDIM",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ERASE,  stmt_erase_handler,  "ERASE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OPTION, stmt_option_handler, "OPTION", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DATA,   stmt_data_handler,   "DATA",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_READ,   stmt_read_handler,   "READ",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_RESTORE,stmt_restore_handler,"RESTORE",STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DEFINT, stmt_defint_handler, "DEFINT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSNG, stmt_defsng_handler, "DEFSNG", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFDBL, stmt_defdbl_handler, "DEFDBL", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEFSTR, stmt_defstr_handler, "DEFSTR", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ON,     stmt_on_handler,     "ON",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_TIMER,  stmt_timer_handler,  "TIMER",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KEY,    stmt_key_handler,    "KEY",    STMT_FLAG_BOTH);
#if BPP_SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_TRY,      stmt_try_handler,      "TRY",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CATCH,    stmt_catch_handler,    "CATCH",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_THROW,    stmt_throw_handler,    "THROW",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_ALIAS,    stmt_alias_handler,    "ALIAS",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DIALECT,  stmt_dialect_handler,  "DIALECT",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_METADATA, stmt_metadata_handler, "METADATA", STMT_FLAG_BOTH);
#if BPP_SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_RESUME, stmt_resume_handler, "RESUME", STMT_FLAG_PROGRAM);
#endif
#if BPP_SUPPORT_OOP
    stmt_register(vm->stmt_reg, KW_TYPE,   stmt_type_handler,   "TYPE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CLASS,  stmt_class_handler,  "CLASS",  STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENUM,   stmt_enum_handler,   "ENUM",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_WITH,   stmt_with_handler,   "WITH",   STMT_FLAG_PROGRAM);
#endif
    stmt_register(vm->stmt_reg, KW_LOAD,   stmt_load_handler,   "LOAD",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SAVE,   stmt_save_handler,   "SAVE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BLOAD,  stmt_bload_handler,  "BLOAD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BSAVE,  stmt_bsave_handler,  "BSAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BRUN,   stmt_brun_handler,   "BRUN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MERGE,  stmt_merge_handler,  "MERGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SELFTEST,stmt_selftest_handler,"SELFTEST",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UNSAVE, stmt_unsave_handler, "UNSAVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ASSERT, stmt_assert_handler, "ASSERT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TRON,   stmt_tron_handler,   "TRON",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TROFF,  stmt_troff_handler,  "TROFF",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BREAK,  stmt_break_handler,  "BREAK",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VARS,   stmt_vars_handler,   "VARS",   STMT_FLAG_BOTH);

    /* Phase 3 & 4 Registrations */
#if BPP_SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_OPEN,   stmt_open_handler,   "OPEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FIELD,  stmt_field_handler,  "FIELD",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSE,  stmt_close_handler,  "CLOSE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GET,    stmt_get_handler,    "GET",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BGET,   stmt_get_handler,    "BGET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PUT,    stmt_put_handler,    "PUT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_BPUT,   stmt_put_handler,    "BPUT",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SEEK,   stmt_seek_handler,   "SEEK",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FILES,  stmt_files_handler,  "FILES",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_KILL,   stmt_kill_handler,   "KILL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CHDIR,  stmt_chdir_handler,  "CHDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MKDIR,  stmt_mkdir_handler,  "MKDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RMDIR,  stmt_rmdir_handler,  "RMDIR",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NAME,   stmt_name_handler,   "NAME",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_IOCTL,  stmt_ioctl_handler,  "IOCTL",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DEVICES,stmt_devices_handler,"DEVICES",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DIR,    stmt_dir_handler,    "DIR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SETATTR,stmt_setattr_handler,"SETATTR",STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCK,   stmt_lock_handler,   "LOCK",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_UNLOCK, stmt_unlock_handler, "UNLOCK", STMT_FLAG_PROGRAM);

    /* Transaction statement registrations */
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_TXN,      stmt_txn_handler,      "TXN",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ATOMIC,   stmt_atomic_handler,   "ATOMIC",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COMMIT,   stmt_commit_handler,   "COMMIT",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ROLLBACK, stmt_rollback_handler, "ROLLBACK", STMT_FLAG_BOTH);
#endif
#endif
    stmt_register(vm->stmt_reg, KW_SELECT, stmt_select_handler, "SELECT", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CASE,   stmt_case_handler,   "CASE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SUB,      stmt_sub_handler,      "SUB",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PROCEDURE, stmt_procedure_handler, "PROCEDURE", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_FUNCTION, stmt_function_handler, "FUNCTION", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CALL,     stmt_call_handler,     "CALL",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_DECLARE,  stmt_declare_handler,  "DECLARE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ENDFUNC,  stmt_end_function_handler, "ENDFUNC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_ENDPROC,  stmt_end_sub_handler,      "ENDPROC", STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_AUTO,    stmt_auto_handler,     "AUTO",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BCOLOR,  stmt_bcolor_handler,   "BCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FCOLOR,  stmt_fcolor_handler,   "FCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLEAR,   stmt_clear_handler,    "CLEAR",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CURSOR,  stmt_cursor_handler,   "CURSOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LOCATE,  stmt_locate_handler,   "LOCATE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SHARED,  stmt_shared_handler,   "SHARED",   STMT_FLAG_BOTH);
#if BPP_SUPPORT_GRAPHICS
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_SCREEN,  stmt_screen_handler,   "SCREEN",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_COLOR,   stmt_color_handler,    "COLOR",    STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_BEEP,    stmt_beep_handler,     "BEEP",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_LINE,    stmt_line_handler,     "LINE",     STMT_FLAG_PROGRAM);
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_CIRCLE,  stmt_circle_handler,   "CIRCLE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PSET,    stmt_pset_handler,     "PSET",     STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PRESET,  stmt_preset_handler,   "PRESET",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_CLS,     stmt_cls_handler,      "CLS",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PAINT,   stmt_paint_handler,    "PAINT",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SOUND,   stmt_sound_handler,    "SOUND",    STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_PLAY,    stmt_play_handler,     "PLAY",     STMT_FLAG_PROGRAM);
#endif
#endif
#if BPP_SUPPORT_EDITOR
    stmt_register(vm->stmt_reg, KW_EDIT,    stmt_edit_handler,     "EDIT",     STMT_FLAG_BOTH);
#endif
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_SECURITY, stmt_security_handler, "SECURITY", STMT_FLAG_BOTH);
#endif
#if BPP_SUPPORT_MODULE
    stmt_register(vm->stmt_reg, KW_MODULE,   stmt_module_handler,   "MODULE",   STMT_FLAG_BOTH);
#endif
#if BPP_SUPPORT_TASK
    stmt_register(vm->stmt_reg, KW_TASK,     stmt_task_handler,     "TASK",     STMT_FLAG_BOTH);
#endif
#if BPP_SUPPORT_MAT
    stmt_register(vm->stmt_reg, KW_MAT,      stmt_mat_handler,      "MAT",      STMT_FLAG_BOTH);
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_ARRAY,    stmt_arrayext_handler, "ARRAY",    STMT_FLAG_BOTH);
#endif
#endif
#if BPP_SUPPORT_EDITOR
    stmt_register(vm->stmt_reg, KW_RENUM,    stmt_renum_handler,    "RENUM",    STMT_FLAG_IMMEDIATE);
    stmt_register(vm->stmt_reg, KW_DELETE,   stmt_delete_handler,   "DELETE",   STMT_FLAG_IMMEDIATE);
#endif
#if BPP_SUPPORT_HELP
    stmt_register(vm->stmt_reg, KW_HELP,     stmt_help_handler,     "HELP",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CATALOG,  stmt_catalog_handler,  "CATALOG",  STMT_FLAG_BOTH);
#endif
#if BPP_SUPPORT_NET
    stmt_register(vm->stmt_reg, KW_MOUNT,    stmt_mount_handler,    "MOUNT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_UMOUNT,   stmt_umount_handler,   "UMOUNT",   STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_CHVT,     stmt_chvt_handler,     "CHVT",     STMT_FLAG_BOTH);
#if BPP_SUPPORT_NET
    stmt_register(vm->stmt_reg, KW_GET,      stmt_get_handler,      "GET",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BGET,     stmt_get_handler,      "BGET",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUT,      stmt_put_handler,      "PUT",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BPUT,     stmt_put_handler,      "BPUT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NET,      stmt_net_handler,      "NET",      STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_OUT,      stmt_out_handler,      "OUT",      STMT_FLAG_BOTH);
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_DEFSEG,   stmt_defseg_handler,   "DEF SEG",  STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_POKE,     stmt_poke_handler,     "POKE",     STMT_FLAG_BOTH);
#if BPP_SUPPORT_BIOS
    stmt_register(vm->stmt_reg, KW_BIOS,     stmt_bios_handler,     "BIOS",     STMT_FLAG_BOTH);
#endif
#if BPP_SUPPORT_GEMINI
    stmt_register(vm->stmt_reg, KW_GEMINI,   stmt_gemini_handler,   "GEMINI",   STMT_FLAG_BOTH);
#endif

    /* Phase 11b and 11c statements */
    stmt_register(vm->stmt_reg, KW_NOISE,      stmt_noise_handler,      "NOISE",      STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_SNDPLAY,    stmt_sndplay_handler,    "_SNDPLAY",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDLOOP,    stmt_sndloop_handler,    "_SNDLOOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDSTOP,    stmt_sndstop_handler,    "_SNDSTOP",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDPAUSE,   stmt_sndpause_handler,   "_SNDPAUSE",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SNDVOL,     stmt_sndvol_handler,     "_SNDVOL",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEINPUT, stmt_mouseinput_handler, "_MOUSEINPUT", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSEHIDE,  stmt_mousehide_handler,  "_MOUSEHIDE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_MOUSESHOW,  stmt_mouseshow_handler,  "_MOUSESHOW",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_TITLE,      stmt_title_handler,      "_TITLE",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_SCREENMOVE, stmt_screenmove_handler, "_SCREENMOVE", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FULLSCREEN, stmt_fullscreen_handler, "_FULLSCREEN", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RESIZE,     stmt_resize_handler,     "_RESIZE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ICON,       stmt_icon_handler,       "_ICON",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_NWRITE,     stmt_nwrite_handler,     "NWRITE",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_FREEIMAGE,  stmt_freeimage_handler,  "_FREEIMAGE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTIMAGE,   stmt_putimage_handler,   "_PUTIMAGE",   STMT_FLAG_PROGRAM);
    stmt_register(vm->stmt_reg, KW_STATESAVE,  stmt_statesave_handler,  "_STATESAVE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_STATELOAD,  stmt_stateload_handler,  "_STATELOAD",  STMT_FLAG_BOTH);

    /* Legacy dialect compatibility statements */
#if BPP_SUPPORT_GRAPHICS
#ifndef BPP_LITE_BUILD
    stmt_register(vm->stmt_reg, KW_GR,       stmt_gr_handler,       "GR",       STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR,      stmt_hgr_handler,      "HGR",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HGR2,     stmt_hgr2_handler,     "HGR2",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HCOLOR,   stmt_hcolor_handler,   "HCOLOR",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PLOT,     stmt_plot_handler,     "PLOT",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HLIN,     stmt_hlin_handler,     "HLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_VLIN,     stmt_vlin_handler,     "VLIN",     STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_HPLOT,    stmt_hplot_handler,    "HPLOT",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_GRAPHICS, stmt_graphics_handler, "GRAPHICS", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_DRAWTO,   stmt_drawto_handler,   "DRAWTO",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BORDER,   stmt_border_handler,   "BORDER",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_INK,      stmt_ink_handler,      "INK",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PAPER,    stmt_paper_handler,    "PAPER",    STMT_FLAG_BOTH);

    /* BGI (BASIC++ Graphics Interface) statements */
    stmt_register(vm->stmt_reg, KW_INITGRAPH,  stmt_initgraph_handler,  "INITGRAPH",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_CLOSEGRAPH, stmt_closegraph_handler, "CLOSEGRAPH", STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PUTPIXEL,   stmt_putpixel_handler,   "PUTPIXEL",   STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BAR,        stmt_bar_handler,        "BAR",        STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_ELLIPSE,    stmt_ellipse_handler,    "ELLIPSE",    STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_RECTANGLE,  stmt_rectangle_handler,  "RECTANGLE",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_OUTTEXTXY,  stmt_outtextxy_handler,  "OUTTEXTXY",  STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_PALETTE,    stmt_palette_handler,    "PALETTE",    STMT_FLAG_BOTH);
#endif
#endif

    stmt_register(vm->stmt_reg, KW_PAUSE,    stmt_pause_handler,    "PAUSE",    STMT_FLAG_BOTH);
#if BPP_SUPPORT_FILES
    stmt_register(vm->stmt_reg, KW_GET,      stmt_get_handler,      "GET",      STMT_FLAG_BOTH);
    stmt_register(vm->stmt_reg, KW_BGET,     stmt_get_handler,      "BGET",     STMT_FLAG_BOTH);
#endif
    stmt_register(vm->stmt_reg, KW_SYS,      stmt_sys_handler,      "SYS",      STMT_FLAG_BOTH);
#if BPP_SUPPORT_TRY
    stmt_register(vm->stmt_reg, KW_ONERR,    stmt_onerr_handler,    "ONERR",    STMT_FLAG_BOTH);
#endif
}

#if BPP_SUPPORT_BIOS
static uint8_t bios_read_mem_cb(void *user_data, uint32_t addr) {
    VMContext *vm = (VMContext *)user_data;
    bool intercepted = false;
    uint8_t val = vdev_bus_peek((unsigned long)addr, &intercepted);
    if (intercepted) {
        return val;
    }
    if (vm && vm->bios_ram && addr < 1024 * 1024) {
        return vm->bios_ram[addr];
    }
    return 0;
}

static void bios_write_mem_cb(void *user_data, uint32_t addr, uint8_t val) {
    VMContext *vm = (VMContext *)user_data;
    bool intercepted = false;
    vdev_bus_poke((unsigned long)addr, val, &intercepted);
    if (vm && vm->bios_ram && addr < 1024 * 1024) {
        vm->bios_ram[addr] = val;
    }
}

static void bios_vdev_sleep_cb(void *user_data, int ms) {
    (void)user_data;
    if (ms > 0) {
        platform_sleep_ms((uint32_t)ms);
    }
}

static int bios_vdev_ioctl_cb(void *user_data, int cmd, void *arg) {
    VMContext *vm = (VMContext *)user_data;
    if (!vm) return 0;
    if (cmd == VDEV_IOCTL_INT10) {
        MockBiosRegs *r = (MockBiosRegs *)arg;
        uint8_t al = r->ax & 0xFF;
        uint8_t ah = (r->ax >> 8) & 0xFF;
        if (ah == 0x0E || ah == 0x09) {
            vdev_putc(vm->vdev, al);
        }
        return 1;
    } else if (cmd == VDEV_IOCTL_INT16) {
        MockBiosRegs *r = (MockBiosRegs *)arg;
        uint8_t ah = (r->ax >> 8) & 0xFF;
        if (ah == 0x01 || ah == 0x11) {
            if (platform_kbhit()) {
                r->ax = platform_getch();
            } else {
                r->ax = 0;
            }
        } else {
            r->ax = platform_getch();
        }
        return 1;
    } else if (cmd == VDEV_IOCTL_VFS_RESOLVE) {
        typedef struct { const char *path; char *out_buffer; int out_max; int for_write; } BiosVfsArgs;
        BiosVfsArgs *r = (BiosVfsArgs *)arg;
        if (vfs_resolve(vm->vfs, r->path, r->out_buffer, (size_t)r->out_max)) return 1;
        return 0;
    } else if (cmd == VDEV_IOCTL_DIR_MKDIR) {
        const char *path = (const char *)arg;
        return platform_mkdir(path) == 0 ? 1 : 0;
    } else if (cmd == VDEV_IOCTL_DIR_RMDIR) {
        const char *path = (const char *)arg;
        return platform_rmdir(path) == 0 ? 1 : 0;
    } else if (cmd == VDEV_IOCTL_DIR_CHDIR) {
        const char *path = (const char *)arg;
        return platform_chdir(path) == 0 ? 1 : 0;
    } else if (cmd == VDEV_IOCTL_DIR_GETCWD) {
        char *path = (char *)arg;
        return platform_getcwd(path, 256) != NULL ? 1 : 0;
    }
    return 0;
}

static void bios_get_registers_cb(void *user_data, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags) {
    VMContext *vm = (VMContext *)user_data;
    if (vm) {
        *ax = vm->regs.ax;
        *bx = vm->regs.bx;
        *cx = vm->regs.cx;
        *dx = vm->regs.dx;
        *flags = vm->regs.flags;
    }
}

static void bios_set_registers_cb(void *user_data, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags) {
    VMContext *vm = (VMContext *)user_data;
    if (vm) {
        vm->regs.ax = ax;
        vm->regs.bx = bx;
        vm->regs.cx = cx;
        vm->regs.dx = dx;
        vm->regs.flags = flags;
    }
}
#endif

VMContext *vm_init(MemoryContext *mem, StringContext *str, VariableContext *var, VDevContext *vdev) {
    if (!mem || !str || !var || !vdev) return NULL;
    VMContext *vm = (VMContext *)calloc(1, sizeof(VMContext));
    if (!vm) return NULL;

    vm->mem = mem;
    vm->str = str;
    vm->var = var;
#ifndef BPP_LITE_BUILD
    vm->vmem = vmem_init(var);
#endif
    vm->vdev = vdev;
    struct_registry_init(&vm->type_reg);

    vm->stmt_reg = stmt_registry_init(mem);
    if (!vm->stmt_reg) {
        free(vm);
        return NULL;
    }

    vm->gosub_stack = gosub_stack_init();
    if (!vm->gosub_stack) {
        stmt_registry_shutdown(vm->stmt_reg);
        free(vm);
        return NULL;
    }

    vm->for_stack = for_stack_init();
    if (!vm->for_stack) {
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
        free(vm);
        return NULL;
    }

    vm->while_stack = while_stack_init();
    if (!vm->while_stack) {
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
        free(vm);
        return NULL;
    }

    vm->do_stack = do_stack_init();
    if (!vm->do_stack) {
        while_stack_shutdown(vm->while_stack);
        for_stack_shutdown(vm->for_stack);
        gosub_stack_shutdown(vm->gosub_stack);
        stmt_registry_shutdown(vm->stmt_reg);
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
        free(vm);
        return NULL;
    }

    vm->active_proc[0] = '\0';
    vm->opt_eh = false;
    vm->active_dialect = NULL;

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
#if BPP_SUPPORT_NET
    vm->vnet = vnet_init(mem);
#else
    vm->vnet = NULL;
#endif
    vm->usb = usb_init(mem);
    vm->vcon = vcon_init();
#if BPP_SUPPORT_BIOS
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

    return vm;
}

void vm_shutdown(VMContext *vm) {
    if (!vm) return;
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
#ifndef BPP_LITE_BUILD
    vmem_shutdown(vm->vmem);
#endif
    vfs_shutdown(vm->vfs);
#if BPP_SUPPORT_NET
    vnet_shutdown(vm->vnet);
#endif
    usb_shutdown(vm->usb);
    vcon_shutdown(vm->vcon);
#if BPP_SUPPORT_BIOS
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

#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif

#include "bpp_module.h"

static BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char dir_name[64];
    int len = (int)(dir_tok.length < sizeof(dir_name) - 1 ? dir_tok.length : sizeof(dir_name) - 1);
    memcpy(dir_name, dir_tok.as.string, len);
    dir_name[len] = '\0';

    if (strcasecmp(dir_name, "BIOS") == 0) {
#if BPP_SUPPORT_BIOS
        BppToken model_tok = lex_next(lex);
        char m_name[64];
        if (model_tok.type == TOK_STRING) {
            int m_len = (int)(model_tok.length < sizeof(m_name) - 1 ? model_tok.length : sizeof(m_name) - 1);
            memcpy(m_name, model_tok.as.string, m_len);
            m_name[m_len] = '\0';
        } else if (model_tok.type == TOK_IDENT) {
            int m_len = (int)(model_tok.length < sizeof(m_name) - 1 ? model_tok.length : sizeof(m_name) - 1);
            memcpy(m_name, model_tok.start, m_len);
            m_name[m_len] = '\0';
        } else {
            err.code = 2; err.message = "Expected string or identifier for ::BIOS";
            return err;
        }

        MockBiosModel model = mock_bios_model_from_string(m_name);
        if (model == BIOS_MODEL_NONE && strcasecmp(m_name, "NONE") != 0) {
            err.code = 2; err.message = "Unknown BIOS model name";
            return err;
        }

        if (vm->bios && vm->bios_ram) {
            vdev_bus_set_ram(vm->bios_ram, 1024 * 1024);
            vdev_bus_set_model(model);
            mock_bios_init_mem(vm->bios, vm->bios_ram, 1024 * 1024, model);
        }
#else
        err.code = 2; err.message = "BIOS emulation not supported in this build";
        return err;
#endif
    }
    else if (strcasecmp(dir_name, "OPTION") == 0) {
        BppToken opt_tok = lex_next(lex);
        if (opt_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected option name in ::OPTION";
            return err;
        }
        char opt_name[64];
        int opt_len = (int)(opt_tok.length < sizeof(opt_name) - 1 ? opt_tok.length : sizeof(opt_name) - 1);
        memcpy(opt_name, opt_tok.as.string, opt_len);
        opt_name[opt_len] = '\0';

        BppMetadataRegistry *reg = vm_get_metadata(vm);
        if (strcasecmp(opt_name, "STRICT") == 0 || strcasecmp(opt_name, "EXPLICIT") == 0) {
            if (reg) reg->option_strict = true;
        } else {
            err.code = 2; err.message = "Unsupported option in ::OPTION";
            return err;
        }
    }
    else if (strcasecmp(dir_name, "INCLUDE") == 0) {
        /* Already statically loaded at parse/pre-scan time; consume string argument as NOP */
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_STRING) {
            err.code = 2; err.message = "Expected string argument for ::INCLUDE";
            return err;
        }
    }
    else if (strcasecmp(dir_name, "IMPORT") == 0) {
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_STRING) {
            err.code = 2; err.message = "Expected string argument for ::IMPORT";
            return err;
        }
        char imp_path[256];
        int imp_len = (int)(val_tok.length < sizeof(imp_path) - 1 ? val_tok.length : sizeof(imp_path) - 1);
        memcpy(imp_path, val_tok.as.string, imp_len);
        imp_path[imp_len] = '\0';
        
        if (module_load_dynamic(vm, imp_path) != 0) {
            err.code = 70; err.message = "Failed to import module/plugin";
            return err;
        }
    }
    else {
        err.code = 2; err.message = "Unsupported compiler directive";
    }

    return err;
}

static bool is_block_end_marker(const char *text, const char *block_type, const char *block_target, MemoryContext *mem) {
    LexerContext *check_lex = lex_init(mem, text);
    if (!check_lex) return false;

    BppToken tok = lex_next(check_lex);
    
    /* 1. Check for 'END {block_type}' */
    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 3 && strncasecmp(tok.start, "END", 3) == 0) {
        BppToken next_tok = lex_next(check_lex);
        if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
            char next_name[64];
            size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
            memcpy(next_name, next_tok.start, nlen);
            next_name[nlen] = '\0';
            if (strcasecmp(next_name, block_type) == 0) {
                lex_shutdown(check_lex);
                return true;
            }
        }
    }

    /* 2. Check for target-qualified endings: '{target} {block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
        char target_buf[64];
        size_t tlen = (tok.length < 63) ? tok.length : 63;
        memcpy(target_buf, tok.start, tlen);
        target_buf[tlen] = '\0';

        if (tok.start + tok.length < text + strlen(text) && *(tok.start + tok.length) == ':') {
            if (tlen + 1 < 63) {
                target_buf[tlen] = ':';
                target_buf[tlen + 1] = '\0';
            }
        }

        if (strcasecmp(target_buf, block_target) == 0) {
            BppToken next_tok = lex_next(check_lex);
            if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                char next_name[64];
                size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                memcpy(next_name, next_tok.start, nlen);
                next_name[nlen] = '\0';

                if (strcasecmp(next_name, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
                    lex_shutdown(check_lex);
                    return true;
                }
            }
        }
    }

    /* 3. Check for standard block ending: '{block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char name_buf[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(name_buf, tok.start, len);
        name_buf[len] = '\0';

        if (strcasecmp(name_buf, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
            lex_shutdown(check_lex);
            return true;
        }
    }

    lex_shutdown(check_lex);
    return false;
}

static BppError skip_metadata_block(VMContext *vm, LexerContext *lex, const char *block_type) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Parse target name from the current line for qualified ending checks */
    char block_target[64] = "";
    BppToken target_tok = lex_peek(lex);
    if (target_tok.type == TOK_IDENT || target_tok.type == TOK_KEYWORD || target_tok.type == TOK_STRING) {
        size_t tlen = (target_tok.length < 63) ? target_tok.length : 63;
        memcpy(block_target, target_tok.start, tlen);
        block_target[tlen] = '\0';
        
        if (target_tok.start + target_tok.length < target_tok.start + 100 && *(target_tok.start + target_tok.length) == ':') {
            if (tlen + 1 < 63) {
                block_target[tlen] = ':';
                block_target[tlen + 1] = '\0';
            }
        }
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    BppLineNumber curr_line = vm_get_current_line(vm);

    size_t start_idx = 0;
    bool found_start = false;
    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number == curr_line) {
            start_idx = i;
            found_start = true;
            break;
        }
    }
    if (!found_start) return err;

    /* Scan forward to see if a closing marker exists */
    bool has_closing = false;
    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            has_closing = true;
            break;
        }
    }

    if (!has_closing) {
        /* Single-line block, consume the rest of the current line so it isn't executed as code */
        BppToken skip_tok = lex_next(lex);
        while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
            skip_tok = lex_next(lex);
        }
        return err;
    }

    /* Jump to the line containing the matching block end marker */
    for (size_t i = start_idx; i < count; i++) {
        if (is_block_end_marker(lines[i].text, block_type, block_target, vm_get_mem(vm))) {
            vm_jump(vm, lines[i].line_number + 0.0001, NULL); /* Jump past this line (fractional safe) */
            return err;
        }
    }

    err.code = 2; err.message = "Block defined without closing block marker";
    return err;
}

/* Execute custom dynamically-registered keywords */
static BppError execute_custom_keyword_statement(VMContext *vm, LexerContext *lex, BppKeywordId kw) {
    BppError err;
    memset(&err, 0, sizeof(err));

    SpecObject *spec = spec_find_by_kw_id(kw);
    if (!spec) {
        err.code = 2;
        err.message = "Unknown dynamic keyword";
        return err;
    }

    /* Check required security level */
    int req_lvl_val = security_find_level_by_name(spec->required_level);
    BppSecLevel req_lvl = (req_lvl_val >= 0) ? (BppSecLevel)req_lvl_val : SEC_STANDARD;
    if (security_check_pinned_level(req_lvl) == 0) {
        err.code = 70;
        err.message = "Permission denied executing dynamic keyword statement (restricted via security pin)";
        return err;
    }

    /* Check if next token is a subcommand name */
    char sub_name[256] = "";
    BppToken next_tok = lex_peek(lex);
    bool has_subcommand = false;
    if (next_tok.type == TOK_IDENT) {
        char subcmd[128];
        size_t slen = (next_tok.length < sizeof(subcmd) - 1) ? next_tok.length : sizeof(subcmd) - 1;
        memcpy(subcmd, next_tok.start, slen);
        subcmd[slen] = '\0';
        
        /* Look for SUB specName.subcmd or SUB subcmd in library */
        char target1[256];
        snprintf(target1, sizeof(target1), "%s.%s", spec->name, subcmd);
        
        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, target1, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, target1, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex); /* Consume subcommand identifier */
        } else if (find_procedure(vm, subcmd, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, subcmd, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
            has_subcommand = true;
            lex_next(lex); /* Consume subcommand identifier */
        }
    }

    if (!has_subcommand) {
        /* No subcommand matched. Look for a subroutine with the same name as the spec itself */
        BppLineNumber def_line = 0.0;
        const char *def_text = NULL;
        if (find_procedure(vm, spec->name, KW_SUB, &def_line, &def_text)) {
            strncpy(sub_name, spec->name, sizeof(sub_name) - 1);
            sub_name[sizeof(sub_name) - 1] = '\0';
        } else {
            err.code = 35;
            err.message = "Subprogram or subcommand not defined for dynamic feature";
            return err;
        }
    }

    /* Evaluate arguments (comma or space separated) */
    BValue args[MAX_PARAMS];
    int arg_count = 0;

    /* Read arguments until TOK_EOL, TOK_EOF, or colon (end of statement) */
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        if (arg_count >= MAX_PARAMS) {
            err.code = 2; err.message = "Too many arguments in custom statement call";
            for (int i = 0; i < arg_count; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        args[arg_count++] = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            for (int i = 0; i < arg_count - 1; i++) {
                if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
            }
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        }
    }

    const char *ret_pos = lex_get_pos(lex);
    err = vm_call_sub_procedure(vm, sub_name, args, arg_count, ret_pos);
    return err;
}

/* Execute a single statement in the current lexer stream */
BppError execute_single_statement(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    vm->current_stmt_pos = tok.start;
    BppKeywordId kw = KW_NONE;

    if (tok.type == TOK_DOCSTRING || tok.type == TOK_NAMESPACE_DECL || tok.type == TOK_GLOBAL_LABEL) {
        lex_next(lex); /* Consume the NOP token */
        return err;
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char word_buf[64];
        size_t w_len = (tok.length < 63) ? tok.length : 63;
        memcpy(word_buf, tok.start, w_len);
        word_buf[w_len] = '\0';

        const char *expansion = vm_lookup_alias(vm, word_buf);
        if (expansion && vm->alias_expansion_depth < 10) {
            lex_next(lex); /* Consume alias token */
            const char *rest = lex_get_pos(lex);

            char *expanded = (char *)mem_scratch_alloc(vm_get_mem(vm), strlen(expansion) + strlen(rest) + 2);
            if (!expanded) {
                err.code = 14;
                err.message = "Scratch memory exhausted during alias expansion";
                return err;
            }
            snprintf(expanded, strlen(expansion) + strlen(rest) + 2, "%s %s", expansion, rest);

            while (tok.type != TOK_EOF && tok.type != TOK_EOL) {
                lex_next(lex);
                tok = lex_peek(lex);
            }

            vm->alias_expansion_depth++;
            err = vm_execute_line(vm, expanded);
            vm->alias_expansion_depth--;
            return err;
        }
    }

    if (tok.type == TOK_DIRECTIVE) {
        lex_next(lex); /* Consume directive token */

        char dir_name[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(dir_name, tok.as.string, len);
        dir_name[len] = '\0';

        if (strcasecmp(dir_name, "KEYWORD") == 0 || strcasecmp(dir_name, "SCOPE") == 0 || strcasecmp(dir_name, "ALIAS") == 0 || strcasecmp(dir_name, "OPTION") == 0) {
            err = skip_metadata_block(vm, lex, dir_name);
            return err;
        }

        err = execute_directive(vm, lex, tok);
        return err;
    }

    if (tok.type == TOK_PERIOD && vm_with_stack_peek(vm) != NULL) {
        /* Peek past the dot and identifier(s) to see if it is a method call or assignment */
        bool is_method_call = false;
        LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (temp_lex) {
            lex_set_dialect(temp_lex, vm_get_active_dialect(vm));
            lex_next(temp_lex); /* Consume '.' */
            BppToken sub = lex_next(temp_lex); /* Consume identifier */
            (void)sub;
            while (lex_peek(temp_lex).type == TOK_PERIOD) {
                lex_next(temp_lex);
                lex_next(temp_lex);
            }
            if (lex_peek(temp_lex).type == TOK_LPAREN) {
                is_method_call = true;
            }
            lex_shutdown(temp_lex);
        }
        
        if (is_method_call) {
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    bpp_map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }
        
        kw = KW_LET;
    } else if (tok.type == TOK_KEYWORD) {
        kw = tok.as.keyword;
        if (kw >= 1000) {
            /* Peek next token to see if it is '=' */
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {
                lex_set_dialect(temp_lex, vm_get_active_dialect(vm));
            }
            BppToken next_tok = lex_next(temp_lex);
            lex_shutdown(temp_lex);
            if (next_tok.type == TOK_EQ) {
                kw = KW_LET; /* Treat as implicit LET assignment */
            } else {
                lex_next(lex); /* Consume keyword */
            }
        } else {
            lex_next(lex); /* Consume keyword */
        }
    } else if (tok.type == TOK_IDENT) {
        /* Check if this is a method call statement: e.g. obj.method(...) */
        bool is_method_call = false;
        if (memchr(tok.start, '.', tok.length) != NULL) {
            /* Clone lexer to scan ahead and see if the token is followed by '(' */
            LexerContext *temp_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
            if (temp_lex) {
                lex_set_dialect(temp_lex, vm_get_active_dialect(vm));
                lex_next(temp_lex); /* Consume current identifier */
                BppToken next_tok = lex_next(temp_lex);
                if (next_tok.type == TOK_LPAREN) {
                    is_method_call = true;
                }
                lex_shutdown(temp_lex);
            }
        }
        
        if (is_method_call) {
            /* Evaluate the method call expression and discard its return value */
            BValue res = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (res.type == VAL_STRING && res.as.string) {
                    str_release(vm_get_str(vm), res.as.string);
                } else if (res.type == VAL_MAP && res.as.map) {
                    bpp_map_release(vm_get_str(vm), res.as.map);
                }
            }
            return err;
        }
        
        /* Check if this is a call-less subroutine/procedure call */
        char sub_name[256];
        size_t slen = (tok.length < sizeof(sub_name) - 1) ? tok.length : sizeof(sub_name) - 1;
        memcpy(sub_name, tok.start, slen);
        sub_name[slen] = '\0';
        if (find_procedure(vm, sub_name, KW_SUB, NULL, NULL)) {
            kw = KW_CALL;
        } else {
            /* Implicit LET */
            kw = KW_LET;
        }
    } else if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        /* NOP */
        return err;
    } else {
        err.code = 2; /* Syntax error */
        err.message = "Expected statement keyword or assignment";
        return err;
    }

    if (security_is_keyword_restricted((int)kw)) {
        err.code = 70;
        err.message = "Keyword restricted via SECURITY RESTRICT";
        return err;
    }

    if (kw >= 1000) {
        err = execute_custom_keyword_statement(vm, lex, kw);
        return err;
    }

    /* Scan ahead in a cloned lexer to find all trailing unnested IF or UNLESS */
    const char *postfix_ptrs[16];
    BppKeywordId postfix_kws[16];
    int postfix_count = 0;
    
    if (kw != KW_IF && kw != KW_UNLESS && kw != KW_FOR && kw != KW_WHILE &&
        kw != KW_DO && kw != KW_SELECT && kw != KW_SUB && kw != KW_FUNCTION && kw != KW_DECLARE) {
        
        int open_parens = 0;
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (scan_lex) {
            lex_set_dialect(scan_lex, vm_get_active_dialect(vm));
            BppToken scan_tok = lex_next(scan_lex);
            while (scan_tok.type != TOK_EOF && scan_tok.type != TOK_EOL) {
                if (scan_tok.type == TOK_LPAREN) open_parens++;
                else if (scan_tok.type == TOK_RPAREN) open_parens--;
                else if (open_parens == 0 && scan_tok.type == TOK_KEYWORD && 
                         (scan_tok.as.keyword == KW_IF || scan_tok.as.keyword == KW_UNLESS)) {
                    if (postfix_count < 16) {
                        postfix_ptrs[postfix_count] = scan_tok.start;
                        postfix_kws[postfix_count] = scan_tok.as.keyword;
                        postfix_count++;
                    }
                }
                scan_tok = lex_next(scan_lex);
            }
            lex_shutdown(scan_lex);
        }
    }

    bool condition_met = true;
    const char *postfix_end_pos = NULL;
    
    for (int i = postfix_count - 1; i >= 0; i--) {
        const char *p_pos = postfix_ptrs[i];
        BppKeywordId p_kw = postfix_kws[i];
        
        char saved_char = '\0';
        char *mutable_next = NULL;
        if (i + 1 < postfix_count) {
            mutable_next = (char *)postfix_ptrs[i + 1];
            saved_char = *mutable_next;
            *mutable_next = '\0';
        }
        
        LexerContext *cond_lex = lex_init(vm_get_mem(vm), p_pos);
        if (cond_lex) {
            lex_set_dialect(cond_lex, vm_get_active_dialect(vm));
            lex_next(cond_lex); /* Consume IF/UNLESS keyword */
            BValue cond_val = eval_expression(vm, cond_lex, &err);
            if (err.code == 0) {
                if (cond_val.type == VAL_STRING) {
                    err.code = 13;
                    err.message = "String expression not allowed in postfix conditional";
                } else {
                    bool truth = (cond_val.as.number != 0.0);
                    condition_met = (p_kw == KW_IF) ? truth : !truth;
                    if (i == postfix_count - 1) {
                        postfix_end_pos = lex_get_pos(cond_lex);
                    }
                }
            }
            lex_shutdown(cond_lex);
        }
        
        if (mutable_next) {
            *mutable_next = saved_char;
        }
        
        if (err.code != 0) {
            if (err.code == 2 && vm->opt_eh) {
                err.message = "Eh?";
            }
            return err;
        }
        
        if (!condition_met) {
            break;
        }
    }

    if (!condition_met) {
        BppToken skip_tok = lex_peek(lex);
        while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
            lex_next(lex);
            skip_tok = lex_peek(lex);
        }
        return err;
    }

    BppStmtHandler handler = stmt_lookup(vm->stmt_reg, kw);
    if (!handler) {
        err.code = 2; /* Syntax error */
        err.message = "Unknown command or unsupported dialect keyword";
        if (vm->opt_eh) err.message = "Eh?";
        return err;
    }

    char *mutable_postfix = (postfix_count > 0) ? (char *)postfix_ptrs[0] : NULL;
    char saved_char = '\0';
    if (postfix_count > 0 && mutable_postfix) {
        saved_char = *mutable_postfix;
        *mutable_postfix = '\0';
    }

    /* Execute the registered handler callback */
    err = handler(vm, lex);

    /* Restore the original character */
    if (postfix_count > 0 && mutable_postfix) {
        *mutable_postfix = saved_char;
    }

    if (err.code == 0 && postfix_count > 0) {
        lex_set_pos(lex, postfix_end_pos);
    }
    if (err.code != 0 && vm->opt_eh) {
        err.message = "Eh?";
    }
    return err;
}

static void get_namespace_at_line(VMContext *vm, BppLineNumber target_line, char *out_ns, size_t max_len) {
    out_ns[0] = '\0';
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    char current_ns[64] = "";

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number > target_line) {
            break;
        }
        LexerContext *scan_lex = lex_init(vm_get_mem(vm), lines[i].text);
        if (scan_lex) {
            BppToken tok = lex_next(scan_lex);
            if (tok.type == TOK_NAMESPACE_DECL) {
                int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                memcpy(current_ns, tok.as.string, len);
                current_ns[len] = '\0';
                if (strcasecmp(current_ns, "DEFAULT") == 0) {
                    current_ns[0] = '\0';
                }
            }
            lex_shutdown(scan_lex);
        }
    }
    size_t copy_len = strlen(current_ns);
    if (copy_len >= max_len) copy_len = max_len - 1;
    memcpy(out_ns, current_ns, copy_len);
    out_ns[copy_len] = '\0';
}

/* Execute a whole line of code, handling colons for multi-statement execution */
BppError vm_execute_line(VMContext *vm, const char *source) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !source) return err;

    const char *processed_source = source;
    BppStringRef arg_str = NULL;
    BValue preproc_res;
    memset(&preproc_res, 0, sizeof(preproc_res));

    if (vm->active_dialect && vm->active_dialect->preprocessor_hook[0] != '\0' && !vm->in_preprocessor_hook) {
        BppError preproc_err;
        memset(&preproc_err, 0, sizeof(preproc_err));
        
        vm->in_preprocessor_hook = true;
        arg_str = str_create(vm_get_str(vm), source, strlen(source));
        BValue args[1];
        args[0].type = VAL_STRING;
        args[0].as.string = arg_str;
        
        preproc_res = invoke_user_function(vm, vm->active_dialect->preprocessor_hook, args, 1, &preproc_err);
        vm->in_preprocessor_hook = false;
        
        if (preproc_err.code != 0) {
            if (arg_str) {
                str_release(vm_get_str(vm), arg_str);
            }
            return preproc_err;
        }
        
        if (preproc_res.type == VAL_STRING && preproc_res.as.string) {
            processed_source = str_data(preproc_res.as.string);
        }
    }

    /* Make a copy of the source line to prevent use-after-free if RUN/LOAD clears the program memory */
    size_t slen = strlen(processed_source);
    char *source_copy = (char *)malloc(slen + 1);
    if (!source_copy) {
        if (arg_str) {
            str_release(vm_get_str(vm), arg_str);
        }
        if (preproc_res.type == VAL_STRING && preproc_res.as.string) {
            str_release(vm_get_str(vm), preproc_res.as.string);
        }
        err.code = 14; /* Out of memory */
        err.message = "Failed to copy statement source line";
        return err;
    }
    memcpy(source_copy, processed_source, slen + 1);

    vm->active_line_original = source;
    vm->active_line_copy = source_copy;

    char ns[64];
    get_namespace_at_line(vm, vm->current_line, ns, sizeof(ns));
    var_set_namespace(vm->var, ns);

    const char *start_pos = source_copy;
    if (vm->current_pos && vm->current_pos >= source && vm->current_pos <= source + strlen(source)) {
        ptrdiff_t offset = vm->current_pos - source;
        /* Clamp offset if processed source size changed */
        if (offset >= 0 && (size_t)offset <= slen) {
            start_pos = source_copy + offset;
        }
    }
    vm->current_pos = NULL;

    /* Reset the expression recursion depth counter at the start of each line.
     * This prevents stale depth values from error-path early returns from
     * carrying over between statements. */
    vm->eval_depth = 0;

    LexerContext *lex = lex_init(vm->mem, start_pos);
    if (!lex) {
        free(source_copy);
        if (arg_str) {
            str_release(vm_get_str(vm), arg_str);
        }
        if (preproc_res.type == VAL_STRING && preproc_res.as.string) {
            str_release(vm_get_str(vm), preproc_res.as.string);
        }
        err.code = 14; /* Out of memory */
        err.message = "Failed to initialize statement parser";
        return err;
    }

    /* Configure the lexer with the active dialect options */
    lex_set_dialect(lex, vm_get_active_dialect(vm));

    /* Save the running state at entry. If we're NOT in the program execution
     * loop (i.e. REPL immediate mode), we must not gate the loop on vm->running
     * or no statement will ever execute. We only check vm->running when the
     * caller was already running a program (to allow END/STOP to halt). */
    bool was_running = vm->running;

    BppToken tok = lex_peek(lex);
    while (tok.type != TOK_EOF && (was_running ? vm->running : true)) {
        /* Map current_pos back to the original source string */
        ptrdiff_t offset = tok.start - source_copy;
        vm->current_pos = source + offset;

        err = execute_single_statement(vm, lex);
        if (err.code != 0) {
            if (file_txn_status(vm->file) == 2) {
                file_txn_rollback(vm->file);
            }
            /* Scan forward to next statement separator (TOK_EOL) or EOF */
            BppToken skip_tok = lex_peek(lex);
            while (skip_tok.type != TOK_EOF && skip_tok.type != TOK_EOL) {
                lex_next(lex);
                skip_tok = lex_peek(lex);
            }
            if (skip_tok.type == TOK_EOL) {
                lex_next(lex); /* Consume separator or EOL */
            }
            ptrdiff_t next_offset = lex_get_pos(lex) - source_copy;
            vm->next_pos = source + next_offset;
            break;
        }

        if (vm->jump_active) {
            if (vm->next_line == vm->current_line) {
                ptrdiff_t jmp_offset = vm->next_pos - source;
                if (jmp_offset >= 0 && (size_t)jmp_offset <= slen) {
                    lex_shutdown(lex);
                    lex = lex_init(vm->mem, source_copy + jmp_offset);
                    if (lex) {
                        lex_set_dialect(lex, vm_get_active_dialect(vm));
                        vm->jump_active = false;
                        tok = lex_peek(lex);
                        continue;
                    }
                }
            }
            /* Control flow jump broke execution of the rest of the line */
            break;
        }

        tok = lex_peek(lex);
        ptrdiff_t next_offset = tok.start - source_copy;
        vm->next_pos = source + next_offset;

        if (tok.type == TOK_EOL) {
            lex_next(lex); /* Consume separator or EOL */
            tok = lex_peek(lex);
        }
    }

    lex_shutdown(lex);
    vm->active_line_original = NULL;
    vm->active_line_copy = NULL;
    free(source_copy);

    if (arg_str) {
        str_release(vm_get_str(vm), arg_str);
    }
    if (preproc_res.type == VAL_STRING && preproc_res.as.string) {
        str_release(vm_get_str(vm), preproc_res.as.string);
    }

    return err;
}

void vm_set_debug_hook(VMContext *vm, void (*hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data), void *user_data) {
    if (vm) {
        vm->debug_hook = hook;
        vm->debug_user_data = user_data;
    }
}

bool vm_get_single_step(VMContext *vm) {
    return vm ? vm->debug_single_step : false;
}

void vm_set_single_step(VMContext *vm, bool enable) {
    if (vm) {
        vm->debug_single_step = enable;
    }
}

void vm_trigger_breakpoint(VMContext *vm, const char *reason) {
    if (!vm) return;
    VDevContext *vdev = vm->vdev;

    if (vdev) {
        vdev_printf(vdev, "\n[BREAKPOINT] at line %g: %s\n", vm->current_line, reason ? reason : "unknown");
        vdev_printf(vdev, "Commands: [s] Step, [c] Continue, [v] View Variables (VARS), [q] Quit\n");
    }
    bpp_log_warn("Breakpoint triggered at line %g: %s", vm->current_line, reason ? reason : "unknown");

    while (true) {
        if (vdev) {
            vdev_printf(vdev, "debug> ");
        }
        char cmd_line[128] = {0};
        if (!fgets(cmd_line, sizeof(cmd_line), stdin)) {
            vm_halt(vm);
            break;
        }
        size_t len = strlen(cmd_line);
        while (len > 0 && (cmd_line[len - 1] == '\n' || cmd_line[len - 1] == '\r')) {
            cmd_line[len - 1] = '\0';
            len--;
        }

        if (strcmp(cmd_line, "s") == 0 || strcmp(cmd_line, "S") == 0 || len == 0) {
            vm->debug_single_step = true;
            break;
        } else if (strcmp(cmd_line, "c") == 0 || strcmp(cmd_line, "C") == 0) {
            vm->debug_single_step = false;
            break;
        } else if (strcmp(cmd_line, "v") == 0 || strcmp(cmd_line, "V") == 0) {
            var_print_all(vm->var, vdev);
        } else if (strcmp(cmd_line, "q") == 0 || strcmp(cmd_line, "Q") == 0) {
            vm_halt(vm);
            break;
        } else {
            if (vdev) {
                vdev_printf(vdev, "Unknown debug command. Use: s, c, v, q\n");
            }
        }
    }
}


/* Program sequential runner */
void vm_run_program(VMContext *vm) {
    if (!vm) return;

    /* Initialize ERR and ERL variables to 0 */
    BValue *p_err = var_lookup(vm->var, "ERR", true);
    if (p_err) {
        p_err->type = VAL_NUMBER;
        p_err->as.number = 0.0;
    }
    BValue *p_erl = var_lookup(vm->var, "ERL", true);
    if (p_erl) {
        p_erl->type = VAL_NUMBER;
        p_erl->as.number = 0.0;
    }

    vm_build_data_table(vm);
    vm->running = true;
    vm->jump_active = false;
    vm_clear_error(vm);
    gosub_stack_clear(vm->gosub_stack);
    for_stack_clear(vm->for_stack);
    while_stack_clear(vm->while_stack);
    do_stack_clear(vm->do_stack);

    /* Fetch minimum line number */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm->mem, &count);
    if (count == 0) {
        vm->running = false;
        return; /* No lines to run */
    }

    if (vm->start_line > 0.0) {
        vm->current_line = vm->start_line;
        vm->start_line = 0.0;
    } else {
        vm->current_line = lines[0].line_number;
    }
    vm->current_pos = NULL;

    while (vm->running && !vm_has_error(vm)) {
        if (vm->jump_active) {
            vm->current_line = vm->next_line;
            vm->current_pos = vm->next_pos;
            vm->jump_active = false;
        }

        lines = mem_program_get_all(vm->mem, &count);
        size_t idx = 0;
        bool found = false;
        bool is_lib = false;

        /* Fetch target line number from main program */
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == vm->current_line) {
                idx = i;
                found = true;
                break;
            }
        }

        /* If not found, fetch target line number from library program */
        size_t lib_count = 0;
        BppProgramLine *lib_lines = mem_lib_program_get_all(vm->mem, &lib_count);
        if (!found && lib_lines) {
            for (size_t i = 0; i < lib_count; ++i) {
                if (lib_lines[i].line_number == vm->current_line) {
                    idx = i;
                    found = true;
                    is_lib = true;
                    break;
                }
            }
        }

        if (!found) {
            /* Find first line number >= current_line in main program */
            for (size_t i = 0; i < count; ++i) {
                if (lines[i].line_number >= vm->current_line) {
                    idx = i;
                    found = true;
                    vm->current_line = lines[i].line_number;
                    break;
                }
            }
        }

        if (!found && lib_lines) {
            /* Find first line number >= current_line in library program */
            for (size_t i = 0; i < lib_count; ++i) {
                if (lib_lines[i].line_number >= vm->current_line) {
                    idx = i;
                    found = true;
                    is_lib = true;
                    vm->current_line = lib_lines[i].line_number;
                    break;
                }
            }
        }

        if (!found) {
            /* Out of program boundaries */
            vm->running = false;
            break;
        }

        /* Reset the scratch arena before each program line runs */
        mem_scratch_reset(vm->mem);

        BppProgramLine *active_lines = is_lib ? lib_lines : lines;
        size_t active_count = is_lib ? lib_count : count;

        if (bpp_logger_is_trace()) {
            VDevContext *vdev = vm->vdev;
            if (vdev) {
                vdev_printf(vdev, "[Line %g]\n", vm->current_line);
            }
            bpp_log_info("[Line %g]", vm->current_line);
        }

        if (vm->debug_single_step) {
            char reason_buf[128];
            snprintf(reason_buf, sizeof(reason_buf), "Line %g: %s", vm->current_line, active_lines[idx].text);
            vm_trigger_breakpoint(vm, reason_buf);
        }

        if (vm->debug_hook) {
            vm->debug_hook(vm, "line", (int)vm->current_line, NULL, vm->debug_user_data);
        }

        BppError err = vm_execute_line(vm, active_lines[idx].text);
#ifndef BPP_LITE_BUILD
        vdev_gfx_poll_events();
#endif
        if (err.code != 0) {
            if (try_stack_count(vm->try_stack) > 0) {
                vm_trigger_try_catch_handler(vm, err.code, err.message);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
            } else if (vm->error_trap_line > 0.0 && !vm->in_error_handler) {
                BppLineNumber err_ln = (err.line != 0.0) ? err.line : vm->current_line;
                vm_trigger_error_trap(vm, err.code, err_ln, vm->current_pos, vm->next_pos);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
            } else {
                vm->last_error = err;
                if (vm->last_error.line == 0.0) {
                    vm->last_error.line = vm->current_line;
                }
                break;
            }
        }

        if (err.code == 0) {
            vm_trigger_event_polling(vm);
        }

        if (!vm->jump_active) {
            if (idx + 1 < active_count) {
                vm->current_line = active_lines[idx + 1].line_number;
            } else {
                vm->running = false; /* Finished last line */
            }
        }

    }
}

void vm_build_data_table(VMContext *vm) {
    if (!vm) return;

    /* Free old table */
    if (vm->data_items) {
        free(vm->data_items);
        vm->data_items = NULL;
    }
    vm->data_count = 0;
    vm->data_ptr = 0;

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    /* Allocate capacity */
    int capacity = 32;
    vm->data_items = (BppDataPosition *)malloc(capacity * sizeof(BppDataPosition));
    if (!vm->data_items) return;

    for (size_t i = 0; i < count; ++i) {
        LexerContext *lex = lex_init(mem, lines[i].text);
        if (!lex) continue;

        BppToken tok = lex_next(lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DATA) {
                while (true) {
                    BppToken val_tok = lex_peek(lex);
                    if (val_tok.type == TOK_EOL || val_tok.type == TOK_EOF || 
                        (val_tok.type == TOK_KEYWORD && val_tok.as.keyword != KW_NONE)) {
                        break;
                    }

                    if (vm->data_count >= capacity) {
                        capacity *= 2;
                        BppDataPosition *temp = (BppDataPosition *)realloc(vm->data_items, capacity * sizeof(BppDataPosition));
                        if (!temp) {
                            lex_shutdown(lex);
                            return;
                        }
                        vm->data_items = temp;
                    }

                    vm->data_items[vm->data_count].line = lines[i].line_number;
                    vm->data_items[vm->data_count].pos = val_tok.start;
                    vm->data_count++;

                    lex_next(lex); /* Consume literal */
                    BppToken comma = lex_peek(lex);
                    if (comma.type == TOK_COMMA) {
                        lex_next(lex); /* Consume ',' */
                    } else {
                        break;
                    }
                }
            }
            tok = lex_next(lex);
        }
        lex_shutdown(lex);
    }
}

int vm_get_data_ptr(VMContext *vm) {
    return vm ? vm->data_ptr : 0;
}

void vm_set_data_ptr(VMContext *vm, int ptr) {
    if (vm) {
        vm->data_ptr = ptr;
    }
}

int vm_get_data_count(VMContext *vm) {
    return vm ? vm->data_count : 0;
}

BppDataPosition *vm_get_data_items(VMContext *vm) {
    return vm ? vm->data_items : NULL;
}

int vm_get_err_code(VMContext *vm) {
    return vm ? vm->err_code : 0;
}

BppLineNumber vm_get_err_line(VMContext *vm) {
    return vm ? vm->err_line : 0.0;
}

void vm_set_error_trap(VMContext *vm, BppLineNumber line) {
    if (vm) {
        vm->error_trap_line = line;
    }
}

void vm_reset_error_state(VMContext *vm) {
    if (vm) {
        vm->error_trap_line = 0.0;
        vm->in_error_handler = false;
        vm->err_code = 0;
        vm->err_line = 0.0;
        vm->error_occurred_line = 0.0;
        vm->error_occurred_pos = NULL;
        vm->error_next_pos = NULL;
    }
}

BppLineNumber vm_get_error_trap(VMContext *vm) {
    return vm ? vm->error_trap_line : 0.0;
}

bool vm_is_in_error_handler(VMContext *vm) {
    return vm ? vm->in_error_handler : false;
}

void vm_set_in_error_handler(VMContext *vm, bool in_handler) {
    if (vm) {
        vm->in_error_handler = in_handler;
    }
}

BppLineNumber vm_get_error_occurred_line(VMContext *vm) {
    return vm ? vm->error_occurred_line : 0.0;
}

const char *vm_get_error_occurred_pos(VMContext *vm) {
    return vm ? vm->error_occurred_pos : NULL;
}

const char *vm_get_error_next_pos(VMContext *vm) {
    return vm ? vm->error_next_pos : NULL;
}

void vm_trigger_error_trap(VMContext *vm, int code, BppLineNumber line, const char *pos, const char *next_pos) {
    if (!vm) return;
    vm->err_code = code;
    vm->err_line = line;
    vm->error_occurred_line = line;
    vm->error_occurred_pos = pos;
    vm->error_next_pos = next_pos;
    vm->in_error_handler = true;

    BValue *p_err = var_lookup(vm->var, "ERR", true);
    if (p_err) {
        p_err->type = VAL_NUMBER;
        p_err->as.number = code;
    }
    BValue *p_erl = var_lookup(vm->var, "ERL", true);
    if (p_erl) {
        p_erl->type = VAL_NUMBER;
        p_erl->as.number = line;
    }

    vm_jump(vm, vm->error_trap_line, NULL);
}


bool vm_is_jump_active(VMContext *vm) {
    return vm ? vm->jump_active : false;
}

bool vm_handle_jump_active(VMContext *vm, BppLineNumber *out_line) {
    if (vm && vm->jump_active) {
        vm->current_line = vm->next_line;
        vm->current_pos = vm->next_pos;
        vm->jump_active = false;
        if (out_line) *out_line = vm->current_line;
        return true;
    }
    return false;
}

void vm_clear_header_jump(VMContext *vm, BppLineNumber def_line) {
    if (vm) {
        vm->jump_active = false;
        vm->current_line = def_line;
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

void vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg) {
    BppTryFrame frame;
    if (vm && try_stack_pop(vm->try_stack, &frame)) {
        vm_restore_stack_depths(vm, frame);

        vm->err_code = code;
        vm->err_line = vm->current_line;

        BValue err_val;
        err_val.type = VAL_NUMBER;
        err_val.as.number = (double)code;
        var_assign(vm->var, "ERR", err_val);

        const char *m = msg ? msg : "";
        BppStringRef str_ref = str_create(vm->str, m, strlen(m));
        BValue errs_val;
        errs_val.type = VAL_STRING;
        errs_val.as.string = str_ref;
        var_assign(vm->var, "ERR$", errs_val);
        str_release(vm->str, str_ref);

        vm_jump(vm, frame.catch_line, frame.catch_pos);
    }
}

BppTypeRegistry *vm_get_types(VMContext *vm) {
    return vm ? &vm->type_reg : NULL;
}

double platform_get_timer(void);
int platform_inkey_char(void);
#ifndef BPP_LITE_BUILD
int vdev_music_note_count(void);
#endif

void vm_set_timer_trap(VMContext *vm, double seconds, BppLineNumber line) {
    if (!vm) return;
    vm->timer_interval = seconds;
    vm->timer_gosub_line = line;
    vm->timer_last_trigger = platform_get_timer();
}

void vm_set_timer_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->timer_state = state;
    if (state == 1) {
        if (vm->timer_pending) {
            vm->timer_pending = false;
        }
    }
}

void vm_set_key_trap(VMContext *vm, int key_idx, int key_code, BppLineNumber line) {
    if (!vm || key_idx < 1 || key_idx > 10) return;
    vm->key_code[key_idx] = key_code;
    vm->key_gosub_line[key_idx] = line;
}

void vm_set_key_state(VMContext *vm, int key_idx, int state) {
    if (!vm || key_idx < 1 || key_idx > 10) return;
    vm->key_state[key_idx] = state;
}

void vm_set_play_trap(VMContext *vm, int note_threshold, BppLineNumber line) {
    if (!vm) return;
    vm->play_note_threshold = note_threshold;
    vm->play_gosub_line = line;
}

void vm_set_play_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->play_state = state;
}

void vm_trigger_event_polling(VMContext *vm) {
    if (!vm || !vm->running) return;

    /* 1. Timer Check */
    if (vm->timer_state == 1 && vm->timer_interval > 0.0) {
        double now = platform_get_timer();
        if (now - vm->timer_last_trigger >= vm->timer_interval) {
            if (!vm->in_timer_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->in_timer_handler = true;
                vm->timer_last_trigger = now;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->timer_gosub_line, NULL);
                    return;
                }
            } else if (vm->in_timer_handler) {
                vm->timer_last_trigger = now;
            }
        }
    } else if (vm->timer_state == 2 && vm->timer_interval > 0.0) {
        double now = platform_get_timer();
        if (now - vm->timer_last_trigger >= vm->timer_interval) {
            vm->timer_pending = true;
            vm->timer_last_trigger = now;
        }
    }

    /* 2. Key Check */
    int key_pressed = platform_inkey_char();
    if (key_pressed > 0) {
        for (int idx = 1; idx <= 10; ++idx) {
            if (vm->key_state[idx] > 0 && vm->key_code[idx] == key_pressed) {
                if (vm->key_state[idx] == 1) {
                    if (!vm->in_key_handler[idx] && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                        vm->in_key_handler[idx] = true;
                        if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                            vm_jump(vm, vm->key_gosub_line[idx], NULL);
                            return;
                        }
                    }
                } else if (vm->key_state[idx] == 2) {
                    vm->key_pending[idx] = true;
                }
            }
        }
    }

    for (int idx = 1; idx <= 10; ++idx) {
        if (vm->key_state[idx] == 1 && vm->key_pending[idx]) {
            if (!vm->in_key_handler[idx] && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->key_pending[idx] = false;
                vm->in_key_handler[idx] = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->key_gosub_line[idx], NULL);
                    return;
                }
            }
        }
    }

    /* 3. Play Check */
#ifndef BPP_LITE_BUILD
    int current_notes = vdev_music_note_count();
    if (vm->play_state == 1 && vm->play_note_threshold > 0) {
        if (current_notes < vm->play_note_threshold) {
            if (!vm->in_play_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->in_play_handler = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->play_gosub_line, NULL);
                    return;
                }
            }
        }
    } else if (vm->play_state == 2 && vm->play_note_threshold > 0) {
        if (current_notes < vm->play_note_threshold) {
            vm->play_pending = true;
        }
    }

    if (vm->play_state == 1 && vm->play_pending) {
        if (current_notes < vm->play_note_threshold) {
            if (!vm->in_play_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->play_pending = false;
                vm->in_play_handler = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->play_gosub_line, NULL);
                    return;
                }
            }
        }
    }
    #endif
}

void vm_clear_event_handlers(VMContext *vm) {
    if (!vm) return;
    vm->in_timer_handler = false;
    for (int i = 1; i <= 10; ++i) {
        vm->in_key_handler[i] = false;
    }
    vm->in_play_handler = false;
}

void vm_with_stack_push(VMContext *vm, const char *path) {
    if (vm && vm->with_stack_depth < 8) {
        strncpy(vm->with_stack[vm->with_stack_depth++], path, 255);
        vm->with_stack[vm->with_stack_depth - 1][255] = '\0';
    }
}

void vm_with_stack_pop(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        vm->with_stack_depth--;
    }
}

void vm_with_stack_clear(VMContext *vm) {
    if (vm) {
        vm->with_stack_depth = 0;
    }
}

const char *vm_with_stack_peek(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        return vm->with_stack[vm->with_stack_depth - 1];
    }
    return NULL;
}

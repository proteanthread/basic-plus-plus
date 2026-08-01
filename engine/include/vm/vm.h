/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_vm.h
 * @brief Virtual Machine core execution context API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the opaque VMContext and functions to access core subsystem managers
 *   (memory, variables, strings, virtual devices) and run programs.
 * - Why it exists: Serves as the central state coordinator for the fetch-decode-execute loop.
 * - Why it works this way: It isolates individual subsystems behind accessor functions. The parser,
 *   lexer, and statement handlers use this interface instead of reading a monolithic RuntimeState struct.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional context accessor declarations, VM lifecycle status codes.
 * - What cannot be changed: Opaque context pointer structures.
 * - What to expect: Initializing a VMContext allocates all sub-managers cleanly.
 * - What to do if something breaks: If accessor returns NULL, check the boot initialization phases.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Thread safety is not required. All pointer access is serial.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add plugin registration and event hooks inside the execution loop.
 * - How to write external extensions: External plugins query VM state and manipulate resources through these accessors.
 */

#ifndef BPP_VM_H
#define BPP_VM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "types/types.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/metadata.h"

/* Opaque VM Context */
typedef struct VMContext VMContext;

/* Forward declaration for VDevContext */
typedef struct VDevContext VDevContext;

/* Forward declaration for FileContext */
typedef struct FileContext FileContext;

/* Forward declaration for StmtRegistry */
typedef struct StmtRegistry StmtRegistry;

typedef struct VfsContext VfsContext;
typedef struct VNetContext VNetContext;
typedef struct UsbContext UsbContext;
typedef struct VConContext VConContext;
typedef struct MockBiosContext MockBiosContext;
typedef struct VMemContext VMemContext;

/**
 * @brief Initialize and shut down VM contexts.
 */
VMContext *vm_init(MemoryContext *mem, StringContext *str, VariableContext *var, VDevContext *vdev);
void       vm_shutdown(VMContext *vm);

/**
 * @brief Run the stored program sequentially from the beginning.
 */
void       vm_run_program(VMContext *vm);
bool       vm_is_running(VMContext *vm);
void       vm_set_running(VMContext *vm, bool running);
int        vm_get_eval_depth(VMContext *vm);
void       vm_inc_eval_depth(VMContext *vm);
void       vm_dec_eval_depth(VMContext *vm);
void       vm_reset_error_state(VMContext *vm);
void       vm_clear_aliases(VMContext *vm);
void       vm_reset_for_run(VMContext *vm);
void        vm_register_alias(VMContext *vm, const char *name, const char *expansion);
const char *vm_lookup_alias(VMContext *vm, const char *name);
BppError   vm_load_program_file(VMContext *vm, const char *filename);
BppError   vm_bload_program_from_stream(VMContext *vm, FILE *fp);
void       vm_stop(VMContext *vm);

/**
 * @brief Execute a single immediate line of code (REPL or direct entry).
 */
BppError   vm_execute_line(VMContext *vm, const char *source);
BppError   execute_single_statement(VMContext *vm, LexerContext *lex);

/**
 * @brief Subsystem Context Accessors
 */
MemoryContext   *vm_get_mem(VMContext *vm);
StringContext   *vm_get_str(VMContext *vm);
VariableContext *vm_get_var(VMContext *vm);
ArrayContext    *vm_get_arr(VMContext *vm);
VDevContext     *vm_get_vdev(VMContext *vm);
FileContext     *vm_get_file(VMContext *vm);
VfsContext      *vm_get_vfs(VMContext *vm);
VNetContext     *vm_get_vnet(VMContext *vm);
UsbContext      *vm_get_usb(VMContext *vm);
VConContext     *vm_get_vcon(VMContext *vm);
MockBiosContext *vm_get_bios(VMContext *vm);
VMemContext     *vm_get_vmem(VMContext *vm);
uint8_t         *vm_get_bios_ram(VMContext *vm);
void             vm_get_bios_registers(VMContext *vm, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags);
void             vm_set_bios_registers(VMContext *vm, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags);
BppLineNumber    vm_get_current_line(VMContext *vm);
const char      *vm_get_current_stmt_pos(VMContext *vm);
StmtRegistry    *vm_get_stmt_registry(VMContext *vm);
BppTypeRegistry *vm_get_types(VMContext *vm);
int              vm_get_last_mouse_click_btn(VMContext *vm);
int              vm_get_last_mouse_click_type(VMContext *vm);

/**
 * @brief Static DATA block position entry.
 */
typedef struct {
    BppLineNumber line;
    const char   *pos;
} BppDataPosition;

void             vm_build_data_table(VMContext *vm);
int              vm_get_data_ptr(VMContext *vm);
void             vm_set_data_ptr(VMContext *vm, int ptr);
int              vm_get_data_count(VMContext *vm);
BppDataPosition *vm_get_data_items(VMContext *vm);

/* Error trapping APIs */
int              vm_get_err_code(VMContext *vm);
BppLineNumber    vm_get_err_line(VMContext *vm);
void             vm_set_error_trap(VMContext *vm, BppLineNumber line);
BppLineNumber    vm_get_error_trap(VMContext *vm);
bool             vm_is_in_error_handler(VMContext *vm);
void             vm_set_in_error_handler(VMContext *vm, bool in_handler);
BppLineNumber    vm_get_error_occurred_line(VMContext *vm);
const char      *vm_get_error_occurred_pos(VMContext *vm);
const char      *vm_get_error_next_pos(VMContext *vm);
void             vm_trigger_error_trap(VMContext *vm, int code, BppLineNumber line, const char *pos, const char *next_pos);

void             vm_trigger_breakpoint(VMContext *vm, const char *reason);
bool             vm_get_single_step(VMContext *vm);
void             vm_set_single_step(VMContext *vm, bool enable);

/* Event trapping APIs */
void vm_set_timer_trap(VMContext *vm, double seconds, BppLineNumber line);
void vm_set_timer_state(VMContext *vm, int state); /* 0=OFF, 1=ON, 2=STOP */
void vm_set_key_trap(VMContext *vm, int key_idx, int key_code, BppLineNumber line);
void vm_set_key_state(VMContext *vm, int key_idx, int state); /* 0=OFF, 1=ON, 2=STOP */
int  vm_get_key_state(const VMContext *vm, int key_idx);
int  vm_get_key_code(const VMContext *vm, int key_idx);
BppLineNumber vm_get_key_gosub_line(const VMContext *vm, int key_idx);
void vm_set_play_trap(VMContext *vm, int note_threshold, BppLineNumber line);
void vm_set_play_state(VMContext *vm, int state); /* 0=OFF, 1=ON, 2=STOP */
void vm_set_mouse_trap(VMContext *vm, int state, BppLineNumber line, int target_char);
void vm_set_hmouse_trap(VMContext *vm, int state, BppLineNumber line);
void vm_set_vmouse_trap(VMContext *vm, int state, BppLineNumber line);
void vm_set_trig_trap(VMContext *vm, int state, BppLineNumber line, int target_btn);
void vm_set_com_trap(VMContext *vm, int port_idx, BppLineNumber line);
void vm_set_com_state(VMContext *vm, int port_idx, int state);
void vm_set_pen_trap(VMContext *vm, BppLineNumber line);
void vm_set_pen_state(VMContext *vm, int state);
void vm_set_strig_trap(VMContext *vm, int strig_idx, BppLineNumber line);
void vm_set_strig_state(VMContext *vm, int strig_idx, int state);
void vm_set_usr_ptr(VMContext *vm, int idx, uintptr_t ptr);
uintptr_t vm_get_usr_ptr(const VMContext *vm, int idx);
void vm_trigger_event_polling(VMContext *vm);
void vm_clear_event_handlers(VMContext *vm);

/* Alarm event trapping APIs */
typedef struct {
    double initial_seconds;   /* original duration (the key) */
    double remaining_seconds; /* remaining time (decrements) */
    int state;                /* 0=OFF, 1=ON, 2=STOP (paused) */
    double last_update_time;  /* platform uptime or epoch when last updated */
    BppLineNumber gosub_line; /* specific GOSUB target line */
} BppAlarmCountdown;

typedef struct {
    char target_time[32];      /* target time string (the key, e.g. "14:30:00") */
    int state;                 /* 0=OFF, 1=ON, 2=STOP (paused) */
    char last_triggered_date[16]; /* YYYY-MM-DD to avoid double trigger in same minute/second */
    int seconds_since_midnight; /* target time parsed as seconds since midnight */
    BppLineNumber gosub_line;  /* specific GOSUB target line */
    int snooze_trigger_time;   /* seconds since midnight when to trigger snoozed alarm, -1 if inactive */
} BppAlarmDaily;

void vm_set_alarm_countdown_trap(VMContext *vm, double seconds, BppLineNumber line);
void vm_set_alarm_countdown_state(VMContext *vm, double seconds, int state);
void vm_set_alarm_countdown_remaining(VMContext *vm, double seconds, double remaining);
double vm_get_alarm_countdown(VMContext *vm, double seconds);
double vm_get_closest_alarm_countdown(VMContext *vm);
void vm_snooze_alarm_countdown(VMContext *vm, double seconds, double snooze_amount);
void vm_unset_alarm_countdown(VMContext *vm, double seconds);
void vm_snooze_all_countdowns(VMContext *vm, double snooze_amount);

void vm_set_alarm_daily_trap(VMContext *vm, const char *time_str, BppLineNumber line);
void vm_set_alarm_daily_state(VMContext *vm, const char *time_str, int state);
double vm_get_alarm_daily_remaining(VMContext *vm, const char *time_str);
double vm_get_closest_alarm_daily_remaining(VMContext *vm);
bool vm_validate_time_str(const char *time_str, int *out_secs);
void vm_snooze_alarm_daily(VMContext *vm, const char *time_str, int snooze_amount);
void vm_unset_alarm_daily(VMContext *vm, const char *time_str);
void vm_snooze_all_dailies(VMContext *vm, int snooze_amount);

void vm_set_global_alarm_state(VMContext *vm, int state);
void vm_set_global_alarm_str_state(VMContext *vm, int state);

/* Structured Exception Handling TryStack APIs */
typedef struct {
    BppLineNumber catch_line;
    const char   *catch_pos;
    BppLineNumber end_try_line;
    const char   *end_try_pos;
    size_t        gosub_stack_depth;
    size_t        for_stack_depth;
    size_t        while_stack_depth;
    size_t        do_stack_depth;
    size_t        select_stack_depth;
    size_t        sub_stack_depth;
} BppTryFrame;

typedef struct TryStack TryStack;

TryStack *try_stack_init(void);
void      try_stack_shutdown(TryStack *stack);
void      try_stack_clear(TryStack *stack);
bool      try_stack_push(TryStack *stack, BppTryFrame frame);
bool      try_stack_pop(TryStack *stack, BppTryFrame *out_frame);
bool      try_stack_peek(TryStack *stack, BppTryFrame *out_frame);
size_t    try_stack_count(TryStack *stack);
void      vm_restore_stack_depths(VMContext *vm, BppTryFrame frame);
TryStack *vm_get_try_stack(VMContext *vm);

/* Stack depth query APIs */
typedef struct GosubStack GosubStack;
typedef struct ForStack ForStack;
typedef struct WhileStack WhileStack;
typedef struct DoStack DoStack;
typedef struct SelectStack SelectStack;
typedef struct SubStack SubStack;

size_t gosub_stack_depth(GosubStack *stack);
size_t for_stack_depth(ForStack *stack);
size_t while_stack_depth(WhileStack *stack);
size_t do_stack_depth(DoStack *stack);
size_t select_stack_depth(SelectStack *stack);
size_t sub_stack_depth(SubStack *stack);

GosubStack  *vm_get_gosub_stack(VMContext *vm);
ForStack    *vm_get_for_stack(VMContext *vm);
WhileStack  *vm_get_while_stack(VMContext *vm);
DoStack     *vm_get_do_stack(VMContext *vm);
SelectStack *vm_get_select_stack(VMContext *vm);
SubStack    *vm_get_sub_stack(VMContext *vm);

/**
 * @brief Set the last error on the VM context (for propagation).
 */
void vm_set_error(VMContext *vm, int code, const char *msg);

/**
 * @brief Clear the last error state.
 */
void vm_clear_error(VMContext *vm);

/**
 * @brief Check if there is an active error pending.
 */
bool vm_has_error(VMContext *vm);

/**
 * @brief Retrieve the pending error struct.
 */
void       vm_set_opt_eh(VMContext *vm, bool enable);
bool       vm_get_opt_eh(VMContext *vm);
void       vm_set_current_line(VMContext *vm, BppLineNumber line);
void       vm_set_arithmetic_decimal(VMContext *vm, bool enable);
bool       vm_get_arithmetic_decimal(VMContext *vm);

typedef struct BppDialect BppDialect;
BppDialect *vm_get_active_dialect(VMContext *vm);
void        vm_set_active_dialect(VMContext *vm, BppDialect *d);
BppDialect *vm_get_defining_dialect(VMContext *vm);
void        vm_set_defining_dialect(VMContext *vm, BppDialect *d);

double      vm_get_last_rnd(VMContext *vm);
void        vm_set_last_rnd(VMContext *vm, double val);
double      vm_get_jiffies_multiplier(VMContext *vm);
void        vm_set_jiffies_multiplier(VMContext *vm, double val);

typedef struct {
    BppLineNumber current_line;
    const char   *current_pos;
    BppLineNumber next_line;
    const char   *next_pos;
    bool          jump_active;
} BppVMState;

void vm_save_state(VMContext *vm, BppVMState *state);
void vm_restore_state(VMContext *vm, const BppVMState *state);

BppError vm_get_error(VMContext *vm);
void vm_set_chaining(VMContext *vm, bool chaining);
bool vm_get_chaining(VMContext *vm);

/**
 * @brief Jump VM execution to a specific program line and position offset.
 */
void vm_jump(VMContext *vm, BppLineNumber line, const char *pos);

/**
 * @brief Push a return address onto the GOSUB execution stack.
 * @return true on success, false on stack overflow.
 */
bool vm_gosub_push(VMContext *vm, BppLineNumber line, const char *pos);

/**
 * @brief Pop a return address from the GOSUB execution stack.
 * @return true on success, false on stack underflow (e.g. RETURN without GOSUB).
 */
bool vm_gosub_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos);

/**
 * @brief FOR loop frame structure for heap loop stack.
 */
typedef struct {
    char          var_name[64];
    double        target;
    double        step;
    BppLineNumber line;
    const char   *pos;
    const char   *next_range_pos;
} BppForFrame;

/* Set the debug hook for execution events */
void vm_set_debug_hook(VMContext *vm, void (*hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data), void *user_data);

/**
 * @brief Push a loop frame onto the FOR loop stack.
 */
bool vm_for_push(VMContext *vm, const char *var_name, double target, double step, BppLineNumber line, const char *pos);

/**
 * @brief Update an active loop frame on the FOR loop stack.
 */
bool vm_for_update(VMContext *vm, const char *var_name, double target, double step, const char *next_range_pos);

/**
 * @brief Pop a loop frame matching a variable name (or top frame if name is NULL) from the FOR stack.
 */
bool vm_for_pop(VMContext *vm, const char *var_name, BppForFrame *out_frame);

/**
 * @brief Peek at a loop frame matching a variable name (or top frame if name is NULL) from the FOR stack.
 */
bool vm_for_peek(VMContext *vm, const char *var_name, BppForFrame *out_frame);

/**
 * @brief Push a loop frame onto the WHILE loop stack.
 */
bool vm_while_push(VMContext *vm, BppLineNumber line, const char *pos);

/**
 * @brief Pop a loop frame from the WHILE loop stack.
 */
bool vm_while_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos);

/**
 * @brief Peek at the top loop frame of the WHILE loop stack.
 */
bool vm_while_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos);

/**
 * @brief Push a loop frame onto the DO loop stack.
 */
bool vm_do_push(VMContext *vm, BppLineNumber line, const char *pos);

/**
 * @brief Pop a loop frame from the DO loop stack.
 */
bool vm_do_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos);

/**
 * @brief Peek at the top loop frame of the DO loop stack.
 */
bool vm_do_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos);

/* SELECT CASE Stack Frame */
typedef struct {
    BValue        val;
    bool          matched;
    BppLineNumber line;
    const char   *pos;
} BppSelectFrame;

typedef struct SelectStack SelectStack;

SelectStack *select_stack_init(void);
void         select_stack_shutdown(SelectStack *stack);
void         select_stack_clear(SelectStack *stack);
bool         select_stack_push(SelectStack *stack, BValue val, bool matched, BppLineNumber line, const char *pos);
bool         select_stack_pop(SelectStack *stack, BppSelectFrame *out_frame);
bool         select_stack_peek(SelectStack *stack, BppSelectFrame *out_frame);

bool         vm_select_push(VMContext *vm, BValue val, bool matched, BppLineNumber line, const char *pos);
bool         vm_select_pop(VMContext *vm, BppSelectFrame *out_frame);
bool         vm_select_peek(VMContext *vm, BppSelectFrame *out_frame);

/* SUB and FUNCTION Call Stack Frame */
typedef struct {
    char          name[256];
    BppLineNumber line;
    const char   *pos;
    bool          is_func;
} BppSubFrame;

typedef struct SubStack SubStack;

SubStack    *sub_stack_init(void);
void         sub_stack_shutdown(SubStack *stack);
void         sub_stack_clear(SubStack *stack);
bool         sub_stack_push(SubStack *stack, const char *name, BppLineNumber line, const char *pos, bool is_func);
bool         sub_stack_pop(SubStack *stack, BppSubFrame *out_frame);
bool         sub_stack_peek(SubStack *stack, BppSubFrame *out_frame);

bool         vm_sub_push(VMContext *vm, const char *name, BppLineNumber line, const char *pos, bool is_func);
bool         vm_sub_pop(VMContext *vm, BppSubFrame *out_frame);
bool         vm_sub_peek(VMContext *vm, BppSubFrame *out_frame);
const char  *vm_get_active_proc(VMContext *vm);
BppMetadataRegistry *vm_get_metadata(VMContext *vm);
void         vm_set_current_filename(VMContext *vm, const char *filename);
const char  *vm_get_current_filename(VMContext *vm);
void         vm_set_start_line(VMContext *vm, BppLineNumber line);
BppLineNumber vm_get_start_line(VMContext *vm);

bool         vm_is_jump_active(VMContext *vm);
bool         vm_handle_jump_active(VMContext *vm, BppLineNumber *out_line);
void         vm_clear_header_jump(VMContext *vm, BppLineNumber def_line);
void         vm_with_stack_push(VMContext *vm, const char *path);
void         vm_with_stack_pop(VMContext *vm);
void         vm_with_stack_clear(VMContext *vm);
const char  *vm_with_stack_peek(VMContext *vm);
void vm_halt(VMContext *vm);
void vm_request_exit(VMContext *vm);
bool vm_exit_requested(VMContext *vm);
void vm_trigger_break(VMContext *vm);
bool vm_break_triggered(VMContext *vm);
void vm_reset_break(VMContext *vm);

/**
 * @brief Pluggable custom detokenizer callback type.
 */
typedef int (*DetokenizerFn)(
    const unsigned char *data,  /* raw file bytes */
    int len,                    /* file length */
    char *out_text,             /* output buffer */
    int max_out                 /* buffer size */
);

/**
 * @brief Register a custom detokenizer for BLOAD files.
 */
void bytecode_set_detokenizer(DetokenizerFn fn);

/**
 * @brief Get the currently registered custom detokenizer.
 */
DetokenizerFn bytecode_get_detokenizer(void);

#endif /* BPP_VM_H */

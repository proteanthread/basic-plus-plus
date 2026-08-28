// FILENAME: vm_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_interrupt.c, math.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for vm_internal.
//
// ---- Includes ----

#ifndef VM_INTERNAL_H
#define VM_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "vm/vm.h"

#include "vm/vm.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
void register_core_statements(VMContext *vm);

uint8_t bios_read_mem_cb(void *ctx, uint32_t addr);
void bios_write_mem_cb(void *ctx, uint32_t addr, uint8_t val);
void bios_vdev_sleep_cb(void *ctx, int ms);
int bios_vdev_ioctl_cb(void *ctx, int request, void *argp);
void bios_get_registers_cb(void *ctx, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *flags);
void bios_set_registers_cb(void *ctx, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t flags);
void vdev_sound_free_all(void);
void vdev_image_free_all(void);

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
#include "runtime/session.h"

extern void console_hide_mouse_cursor(void);
extern void console_draw_mouse_cursor(void);
extern void platform_mouse_get_position(int *col, int *row);
extern int platform_mouse_get_button(int btn_idx);
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"


#define MAX_PARAMS 8

void        vm_register_alias(VMContext *vm, const char *name, const char *expansion);
const char *vm_lookup_alias(VMContext *vm, const char *name);
void        vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg);
BValue      eval_expression(VMContext *vm, LexerContext *lex, BppError *out_err);
BValue      invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);

// Forward declare stack helpers from vm_stack.c
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
bool for_stack_push_multi(ForStack *stack, const char **var_names, int var_count, double target, double step, BppLineNumber line, const char *pos);
bool for_stack_pop(ForStack *stack, const char *var_name, BppForFrame *out_frame);
bool for_stack_peek(ForStack *stack, const char *var_name, BppForFrame *out_frame);
bool for_stack_update(ForStack *stack, const char *var_name, double target, double step, const char *next_range_pos);
void for_stack_set_cached_ptr(ForStack *stack, BValue *ptr);

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

typedef struct BppDialect BppDialect;
struct VMContext {
    BppDialect      *active_dialect;
    BppDialect      *defining_dialect;
    MemoryContext   *mem;
    StringContext   *str;
    VariableContext *var;
    VDevContext     *vdev;
    StmtRegistry    *stmt_reg;
    BppAlias         aliases[64];
    int              alias_count;
    int              alias_expansion_depth;
    BppAlias         oper_aliases[64];
    int              oper_alias_count;
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
    BiosContext     *bios;
    bool             opt_eh;
    bool             opt_arithmetic_decimal;
    int              margin;
    int              zone_width;
    BppSessionContext session;
    BppDataPosition *data_items;
    int              data_count;
    int              data_ptr;

    // Debugger Hooks
    void (*debug_hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data);
    void *debug_user_data;

    // Unit Testing Suite State
    bool             in_test;
    char             test_name[128];
    int              test_pass;
    int              test_fail;
    int              test_total;

    // Interactive Debugger State
    bool             debug_active;
    bool             debug_trap_on_error;
    bool             single_step_mode;
    int              watchpoint_count;
    char             watchpoints[16][32];
    BppLineNumber    breakpoints[16];
    int              breakpoint_count;

    // Error trapping state
    BppLineNumber    error_trap_line;
    bool             in_error_handler;
    int              err_code;
    BppLineNumber    err_line;
    BppLineNumber    error_occurred_line;
    const char      *error_occurred_pos;
    const char      *error_next_pos;

    // Execution state pointers
    BppLineNumber    current_line;
    const char      *current_pos;
    BppLineNumber    next_line;
    const char      *next_pos;
    bool             jump_active;
    bool             running;
    bool             exit_requested;
    bool             break_triggered;
    bool             break_enabled;
    BppLineNumber    break_trap_line;
    bool             is_chaining;
    bool             debug_single_step;
    int              eval_depth;      // Guard against recursive eval_expression overflow
    const char      *current_stmt_pos;

    // Error reporting context
    BppError         last_error;

    // Metadata Registry for namespaces, global labels, and docstrings
    BppMetadataRegistry metadata_reg;

    // Current executing filename for cross-file jumps
    char current_filename[256];

    // Initial starting line for tasks
    BppLineNumber start_line;

    // Pointer mapping for execution text copies
    const char      *active_line_original;
    const char      *active_line_copy;
    double           jiffies_multiplier;

    // WITH block context stack
    char             with_stack[8][256];
    int              with_stack_depth;

    // Event trapping state
    int              timer_state; // 0=OFF, 1=ON, 2=STOP
    double           timer_interval;
    double           timer_last_trigger;
    BppLineNumber    timer_gosub_line;
    bool             timer_pending;
    bool             in_timer_handler;

    int              key_state[15100];
    int              key_code[15100];
    BppLineNumber    key_gosub_line[15100];
    bool             key_pending[15100];
    bool             in_key_handler[15100];

    int              play_state;
    int              play_note_threshold;
    BppLineNumber    play_gosub_line;
    bool             play_pending;
    bool             in_play_handler;

    // Alarm event trapping state
    BppAlarmCountdown *alarms;
    int              alarm_count;
    BppAlarmDaily    *alarms_str;
    int              alarm_str_count;
    int              alarm_state;     // 0=OFF, 1=ON, 2=STOP
    int              alarm_str_state; // 0=OFF, 1=ON, 2=STOP
    BppLineNumber    alarm_gosub_line;     // generic countdown trap line
    BppLineNumber    alarm_str_gosub_line; // generic daily trap line
    bool             in_alarm_handler;
    bool             in_alarm_str_handler;

    BppTypeRegistry  type_reg;
    double           last_rnd;
    double           ti_offset;

    // USR routine pointers (0 to 9)
    uintptr_t        usr_pointers[10];

    // COM, PEN, STRIG event traps
    int              com_state[5];
    BppLineNumber    com_gosub_line[5];
    bool             com_pending[5];
    bool             in_com_handler[5];

    int              pen_state;
    BppLineNumber    pen_gosub_line;
    bool             pen_pending;
    bool             in_pen_handler;

    int              strig_state[5];
    BppLineNumber    strig_gosub_line[5];
    bool             strig_pending[5];
    bool             in_strig_handler[5];

    // Mouse event trapping & state
    int              mouse_state;         // 0=OFF, 1=ON, 2=STOP
    BppLineNumber    mouse_gosub_line;
    int              mouse_target_char;   // target character for ON MOUSE(char) or -1
    bool             mouse_pending;
    bool             in_mouse_handler;

    int              hmouse_state;        // 0=OFF, 1=ON, 2=STOP
    BppLineNumber    hmouse_gosub_line;
    bool             hmouse_pending;
    bool             in_hmouse_handler;

    int              vmouse_state;        // 0=OFF, 1=ON, 2=STOP
    BppLineNumber    vmouse_gosub_line;
    bool             vmouse_pending;
    bool             in_vmouse_handler;

    int              trig_state;          // 0=OFF, 1=ON, 2=STOP
    BppLineNumber    trig_gosub_line;
    int              trig_target_btn;     // target button or -1
    bool             trig_pending;
    bool             in_trig_handler;

    // Custom Cursor configuration
    bool             mouse_cursor_visible;
    int              mouse_cursor_char;   // custom cursor character code (e.g. 219 or 249 or 0 to invert)
    int              mouse_cursor_attrib; // custom cursor attribute/color
    int              last_mouse_col;
    int              last_mouse_row;
    int              last_mouse_btn_mask;
    int              last_mouse_click_btn;
    int              last_mouse_click_type; // 1 = single, 2 = double, 3 = triple
    double           last_click_time;

    // AUTO statement line numbering state
    BppLineNumber    auto_line_start;
    BppLineNumber    auto_line_step;
    bool             auto_line_active;

    // Watchdog Timer & Cycle Limits
    double           timeout_ms;
    double           start_time_ms;
    uint64_t         max_cycles;
    uint64_t         cycle_count;
    bool             watchdog_enabled;

    // High-Speed Execution Cache (Pillar 1)
    struct {
        BppLineNumber line;
        size_t        idx;
        bool          is_lib;
        bool          valid;
    } jump_cache[16];
    uint8_t          jump_cache_head;

    // Reusable Line Scratch Buffer (Eliminates per-line calloc/free)
    char            *line_scratch_buf;
    size_t           line_scratch_cap;
};

void register_core_statements(VMContext *vm);

uint8_t bios_read_mem_cb(void *ctx, uint32_t addr);
void bios_write_mem_cb(void *ctx, uint32_t addr, uint8_t val);
void bios_vdev_sleep_cb(void *ctx, int ms);
int bios_vdev_ioctl_cb(void *ctx, int request, void *argp);
void bios_get_registers_cb(void *ctx, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx, uint32_t *flags);
void bios_set_registers_cb(void *ctx, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t flags);
void vdev_sound_free_all(void);
void vdev_image_free_all(void);

#endif


# VM Subsystem API Reference

Header File: [`include/vm.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/vm.h)

## Overview
Provides the execution environment, bytecode dispatch, statement evaluator, and stack handling.

## Exposed API Entities
### Structs & Types
- `VMContext VMContext`
- `VDevContext VDevContext`
- `FileContext FileContext`
- `StmtRegistry StmtRegistry`
- `VfsContext VfsContext`
- `VNetContext VNetContext`
- `UsbContext UsbContext`
- `VConContext VConContext`
- `MockBiosContext MockBiosContext`
- `VMemContext VMemContext`
- `TryStack TryStack`
- `GosubStack GosubStack`
- `ForStack ForStack`
- `WhileStack WhileStack`
- `DoStack DoStack`
- `SelectStack SelectStack`
- `SubStack SubStack`
- `BppDialect BppDialect`
- `SelectStack SelectStack`
- `SubStack SubStack`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vm_shutdown` | `void` | `VMContext *vm` |
| `vm_run_program` | `void` | `VMContext *vm` |
| `vm_is_running` | `bool` | `VMContext *vm` |
| `vm_get_eval_depth` | `int` | `VMContext *vm` |
| `vm_inc_eval_depth` | `void` | `VMContext *vm` |
| `vm_dec_eval_depth` | `void` | `VMContext *vm` |
| `vm_reset_error_state` | `void` | `VMContext *vm` |
| `vm_clear_aliases` | `void` | `VMContext *vm` |
| `vm_reset_for_run` | `void` | `VMContext *vm` |
| `vm_register_alias` | `void` | `VMContext *vm, const char *name, const char *expansion` |
| `vm_load_program_file` | `BppError` | `VMContext *vm, const char *filename` |
| `vm_bload_program_from_stream` | `BppError` | `VMContext *vm, FILE *fp` |
| `vm_stop` | `void` | `VMContext *vm` |
| `vm_execute_line` | `BppError` | `VMContext *vm, const char *source` |
| `execute_single_statement` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `vm_get_bios_registers` | `void` | `VMContext *vm, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags` |
| `vm_set_bios_registers` | `void` | `VMContext *vm, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags` |
| `vm_get_current_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_build_data_table` | `void` | `VMContext *vm` |
| `vm_get_data_ptr` | `int` | `VMContext *vm` |
| `vm_set_data_ptr` | `void` | `VMContext *vm, int ptr` |
| `vm_get_data_count` | `int` | `VMContext *vm` |
| `vm_get_err_code` | `int` | `VMContext *vm` |
| `vm_get_err_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_set_error_trap` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_get_error_trap` | `BppLineNumber` | `VMContext *vm` |
| `vm_is_in_error_handler` | `bool` | `VMContext *vm` |
| `vm_set_in_error_handler` | `void` | `VMContext *vm, bool in_handler` |
| `vm_get_error_occurred_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_trigger_error_trap` | `void` | `VMContext *vm, int code, BppLineNumber line, const char *pos, const char *next_pos` |
| `vm_trigger_breakpoint` | `void` | `VMContext *vm, const char *reason` |
| `vm_get_single_step` | `bool` | `VMContext *vm` |
| `vm_set_single_step` | `void` | `VMContext *vm, bool enable` |
| `vm_set_timer_trap` | `void` | `VMContext *vm, double seconds, BppLineNumber line` |
| `vm_set_timer_state` | `void` | `VMContext *vm, int state` |
| `vm_set_key_trap` | `void` | `VMContext *vm, int key_idx, int key_code, BppLineNumber line` |
| `vm_set_key_state` | `void` | `VMContext *vm, int key_idx, int state` |
| `vm_set_play_trap` | `void` | `VMContext *vm, int note_threshold, BppLineNumber line` |
| `vm_set_play_state` | `void` | `VMContext *vm, int state` |
| `vm_trigger_event_polling` | `void` | `VMContext *vm` |
| `vm_clear_event_handlers` | `void` | `VMContext *vm` |
| `try_stack_shutdown` | `void` | `TryStack *stack` |
| `try_stack_clear` | `void` | `TryStack *stack` |
| `try_stack_push` | `bool` | `TryStack *stack, BppTryFrame frame` |
| `try_stack_pop` | `bool` | `TryStack *stack, BppTryFrame *out_frame` |
| `try_stack_peek` | `bool` | `TryStack *stack, BppTryFrame *out_frame` |
| `try_stack_count` | `size_t` | `TryStack *stack` |
| `vm_restore_stack_depths` | `void` | `VMContext *vm, BppTryFrame frame` |
| `gosub_stack_depth` | `size_t` | `GosubStack *stack` |
| `for_stack_depth` | `size_t` | `ForStack *stack` |
| `while_stack_depth` | `size_t` | `WhileStack *stack` |
| `do_stack_depth` | `size_t` | `DoStack *stack` |
| `select_stack_depth` | `size_t` | `SelectStack *stack` |
| `sub_stack_depth` | `size_t` | `SubStack *stack` |
| `vm_set_error` | `void` | `VMContext *vm, int code, const char *msg` |
| `vm_clear_error` | `void` | `VMContext *vm` |
| `vm_has_error` | `bool` | `VMContext *vm` |
| `vm_set_opt_eh` | `void` | `VMContext *vm, bool enable` |
| `vm_get_opt_eh` | `bool` | `VMContext *vm` |
| `vm_set_current_line` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_set_arithmetic_decimal` | `void` | `VMContext *vm, bool enable` |
| `vm_get_arithmetic_decimal` | `bool` | `VMContext *vm` |
| `vm_set_active_dialect` | `void` | `VMContext *vm, BppDialect *d` |
| `vm_set_defining_dialect` | `void` | `VMContext *vm, BppDialect *d` |
| `vm_get_last_rnd` | `double` | `VMContext *vm` |
| `vm_set_last_rnd` | `void` | `VMContext *vm, double val` |
| `vm_save_state` | `void` | `VMContext *vm, BppVMState *state` |
| `vm_restore_state` | `void` | `VMContext *vm, const BppVMState *state` |
| `vm_get_error` | `BppError` | `VMContext *vm` |
| `vm_jump` | `void` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_gosub_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_gosub_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_for_push` | `bool` | `VMContext *vm, const char *var_name, double target, double step, BppLineNumber line, const char *pos` |
| `vm_for_update` | `bool` | `VMContext *vm, const char *var_name, double target, double step, const char *next_range_pos` |
| `vm_for_pop` | `bool` | `VMContext *vm, const char *var_name, BppForFrame *out_frame` |
| `vm_for_peek` | `bool` | `VMContext *vm, const char *var_name, BppForFrame *out_frame` |
| `vm_while_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_while_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_while_peek` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_do_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_do_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_do_peek` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `select_stack_shutdown` | `void` | `SelectStack *stack` |
| `select_stack_clear` | `void` | `SelectStack *stack` |
| `select_stack_push` | `bool` | `SelectStack *stack, BValue val, bool matched, BppLineNumber line, const char *pos` |
| `select_stack_pop` | `bool` | `SelectStack *stack, BppSelectFrame *out_frame` |
| `select_stack_peek` | `bool` | `SelectStack *stack, BppSelectFrame *out_frame` |
| `vm_select_push` | `bool` | `VMContext *vm, BValue val, bool matched, BppLineNumber line, const char *pos` |
| `vm_select_pop` | `bool` | `VMContext *vm, BppSelectFrame *out_frame` |
| `vm_select_peek` | `bool` | `VMContext *vm, BppSelectFrame *out_frame` |
| `sub_stack_shutdown` | `void` | `SubStack *stack` |
| `sub_stack_clear` | `void` | `SubStack *stack` |
| `sub_stack_push` | `bool` | `SubStack *stack, const char *name, BppLineNumber line, const char *pos, bool is_func` |
| `sub_stack_pop` | `bool` | `SubStack *stack, BppSubFrame *out_frame` |
| `sub_stack_peek` | `bool` | `SubStack *stack, BppSubFrame *out_frame` |
| `vm_sub_push` | `bool` | `VMContext *vm, const char *name, BppLineNumber line, const char *pos, bool is_func` |
| `vm_sub_pop` | `bool` | `VMContext *vm, BppSubFrame *out_frame` |
| `vm_sub_peek` | `bool` | `VMContext *vm, BppSubFrame *out_frame` |
| `vm_set_current_filename` | `void` | `VMContext *vm, const char *filename` |
| `vm_set_start_line` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_get_start_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_is_jump_active` | `bool` | `VMContext *vm` |
| `vm_handle_jump_active` | `bool` | `VMContext *vm, BppLineNumber *out_line` |
| `vm_clear_header_jump` | `void` | `VMContext *vm, BppLineNumber def_line` |
| `vm_with_stack_push` | `void` | `VMContext *vm, const char *path` |
| `vm_with_stack_pop` | `void` | `VMContext *vm` |
| `vm_with_stack_clear` | `void` | `VMContext *vm` |
| `vm_halt` | `void` | `VMContext *vm` |
| `vm_request_exit` | `void` | `VMContext *vm` |
| `vm_exit_requested` | `bool` | `VMContext *vm` |
| `bytecode_set_detokenizer` | `void` | `DetokenizerFn fn` |
| `bytecode_get_detokenizer` | `DetokenizerFn` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "vm.h"

void run_prog(VMContext *vm) {
    BppError err = vm_load_program_file(vm, "test.bas");
    if (err.code == 0) {
        vm_run_program(vm);
    }
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.

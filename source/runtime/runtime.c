/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: runtime.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Runtime state container (RuntimeState) and statement execution dispatch loops.
 *
 * 2. WHAT TO EXPECT:
 *    RuntimeState encapsulates all interpreter variables, stack frames, heap pools, and execution context.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic trace outputs, signal trapping overrides.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    RuntimeState struct layout, statement execution sequence logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - runtime.c
 // ---
 //
 // Runtime state management implementation.
 //
int g_screen_lock = 0; // 1=SCREEN LOCK active (no auto-flush)
 // DESIGN RATIONALE:
 // The runtime state is a single struct that captures ALL mutable
 // interpreter state. This design:
 //
 // 1. Keeps state off the C call stack - no recursion-dependent state.
 // 2. Makes state inspection trivial (dump the struct).
 // 3. Makes reset trivial (zero the struct, reinitialize).
 // 4. Supports future features like BREAK, CONT, debug stepping.
 //
 // The @() array is allocated from the variable pool after the
 // 26 integer variables. The remaining pool space determines the
 // array size. With a 64K pool and sizeof(long)=4:
 // (65536 - 26*4) / 4 = 16,358 elements
 //
 // The RNG uses a simple LCG (linear congruential generator) with
 // parameters that work well on both 16-bit and 32-bit targets.
 // No <time.h> dependency - the seed starts at 1 and advances
 // with each call.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "runtime.h"
#include "lexer.h"
#include "errors.h"
#include "boot.h"
#ifndef BPP_LITE_BUILD
#include "pcode.h"
#endif
#include "../console.h"
#include "modules/lib_space.h"
#include "modules/bpl_format.h"

// --- Runtime Initialization ---

 // runtime_init - Initialize runtime state.
 //
 // Sets up the @() array from the variable pool. The first
 // (MAX_VARIABLES * sizeof(long)) bytes of the variable pool
 // are reserved for A-Z variables (though we use the variables[]
 // array in the RuntimeState struct, not the pool, for direct
 // access). The remaining pool space is for the @() array.
 //
 // Actually, we keep A-Z in the struct for fast access and use
 // the ENTIRE variable pool for the @() array. This gives maximum
 // array space.
void runtime_init(RuntimeState *rt, ProgramStore *program,
 MemorySystem *memory)
{
 int i;

 rt->program = program;
 rt->memory = memory;
 rt->running = 0;
 rt->current_index = 0;
 rt->next_index = -1;
 rt->stack_top = 0;
 rt->print_width = DEFAULT_PRINT_WIDTH;
 rt->option_base = 0;
 rt->arithmetic_decimal = 0;
 rt->print_col = 1;
 rt->stopped = 0;
 rt->rnd_seed = 1;

 // Clear all variables to integer zero
 for (i = 0; i < MAX_VARIABLES; i++) {
 rt->variables[i] = bval_int(0);
 }

 // Set up @() array using the variable pool
 rt->array_base = (long *)memory->variable.base;
 rt->array_size = memory->variable.size / (long)sizeof(long);
 memset(rt->array_base, 0,
 (size_t)(rt->array_size * (long)sizeof(long)));

 // Clear the stack
 memset(rt->stack, 0, sizeof(rt->stack));

 // Clear named variables and DATA pool
 rt->named_count = 0;
 memset(rt->named_vars, 0, sizeof(rt->named_vars));
 rt->data_count = 0;
 rt->data_ptr = 0;

  // String variables
  for (i = 0; i < MAX_STRING_VARS; i++) {
  rt->string_vars[i] = bval_string(NULL, 0);
  rt->string_capacities[i] = 0;
  }

 // DIM arrays
 rt->dim_count = 0;
 rt->dim_elements_used = 0;
 memset(rt->dim_arrays, 0, sizeof(rt->dim_arrays));
 for (i = 0; i < MAX_ARRAY_ELEMENTS; i++) {
 rt->dim_elements[i] = bval_int(0);
 }

  // String pool
  if (rt->strpool.base == NULL) {
#ifdef BPP_LITE_BUILD
      strpool_init(&rt->strpool, (rt->memory->variable.size * 25L) / 15L);
#else
      strpool_init(&rt->strpool, rt->memory->variable.size);
#endif
      strpool_set_runtime(&rt->strpool, rt);
  }

 // Virtual devices
 rt->dev_con = vdev_get(VDEV_CON);
 rt->dev_err = vdev_get(VDEV_ERR);

  // Trace and error handler
  rt->trace_on = 0;
  rt->debug_on = 0;
  rt->in_test = 0;
  memset(rt->test_name, 0, sizeof(rt->test_name));
  rt->test_pass = 0;
  rt->test_fail = 0;
  rt->test_total = 0;
  memset(rt->breakpoints, 0, sizeof(rt->breakpoints));
  rt->breakpoint_count = 0;
  rt->single_step = 0;
  rt->resume_index = 0;
  if (!rt->log_fp) {
      rt->log_level = 3; // LOG_ERROR
      rt->log_to_stderr = 0;
      rt->log_fp = NULL;
  }
  rt->on_error_line = 0;

 // User-defined functions
 rt->user_func_count = 0;
 memset(rt->user_funcs, 0, sizeof(rt->user_funcs));

 // VM state and eval stack
 rt->vm_state = 0; // VM_STOPPED
 vm_eval_init(&rt->eval_stack);

  rt->line_asts = NULL;
  rt->line_asts_count = 0;
  rt->acc = bval_int(0);
  rt->chain_pending = 0;
  rt->chain_file[0] = '\0';

  // Debugger
 rt->breakpoint_count = 0;
 rt->single_step = 0;
 rt->resume_index = -1;
 memset(rt->breakpoints, 0, sizeof(rt->breakpoints));

 // Self-test framework
 rt->test_pass = 0;
 rt->test_fail = 0;
 rt->test_total = 0;
 rt->in_test = 0;
 rt->test_name[0] = '\0';
 rt->assert_pass_total = 0;
 rt->assert_fail_total = 0;
 rt->test_block_count = 0;

 // AUTO mode
 rt->auto_line = 0;
 rt->auto_step = 0;

 // Block IF depth
 rt->block_if_depth = 0;

 // OPTION ANGLE
 rt->angle_degrees = 0; // radians (default)
 rt->tab_mode = 0; // spaces (default)
 rt->zone_override = -1; // use dialect default

 // SCREEN / DRAW state
  rt->screen_mode = 0; // text mode
  rt->is_atari_graphics = 0;
  rt->atari_graphics_mode = 0;
  rt->draw_x = 40; // center of 80-col screen
 rt->draw_y = 25; // center of 50-row canvas
 rt->draw_color = '*'; // default pen character

 // Cursor tracking
 rt->cursor_row = 1; // 1-based
 rt->cursor_col = 1; // 1-based

 // WIDTH state
 rt->screen_width = 80; // 80 columns default
 rt->screen_lines = 25; // 25 lines default

 // CONST table
 rt->const_count = 0;

 // SUB/FUNCTION table
 rt->sub_count = 0;
 rt->fn_return_value = bval_int(0);
 rt->in_sub_index = -1;
 rt->suspended = 0;
 rt->resumed = 0;

 // User-Defined Types (Milestone 17)
 rt->type_count = 0;
 rt->typed_var_count = 0;
 memset(rt->user_types, 0, sizeof(rt->user_types));
 memset(rt->typed_vars, 0, sizeof(rt->typed_vars));

 // Dynamic scope stack (Milestone 9)
 scope_stack_init(&rt->scope_stack);

 // Label table
 rt->label_count = 0;

 // Function key macros
 memset(rt->fkey_macros, 0, sizeof(rt->fkey_macros));
 rt->fkey_display = 0;

 // ALARM$
 rt->alarm_str[0] = '\0';

 // SCOPE hook state
 rt->scope_before_done = -1;
 rt->scope_after_kw = -1;
 rt->scope_hook_depth = -1;

 // SCOPE event queue
 rt->scope_evq_head = 0;
 rt->scope_evq_tail = 0;
 rt->scope_evq_count = 0;


 // WINDOW logical coordinate system
 rt->win_active = 0;
 rt->win_screen_flag = 0;
 rt->win_x1 = 0.0; rt->win_y1 = 0.0;
 rt->win_x2 = 0.0; rt->win_y2 = 0.0;

 // VIEW PRINT text scroll region
 rt->view_print_top = 1;
 rt->view_print_bottom = 25;

 // Virtual console screen buffer
 memset(rt->vcon_chars, ' ', sizeof(rt->vcon_chars));
 memset(rt->vcon_colors, 7, sizeof(rt->vcon_colors));

 // Event trap handlers
 memset(rt->on_com_line, 0, sizeof(rt->on_com_line));
 memset(rt->on_key_line, 0, sizeof(rt->on_key_line));
 rt->on_pen_line = 0;
 rt->on_play_line = 0;
 memset(rt->on_strig_line, 0,
  sizeof(rt->on_strig_line));
 rt->timer_event_state = EVT_OFF;
 rt->timer_interval = 0.0;
 rt->timer_last_fire = 0.0;
 memset(rt->key_event_state, 0,
  sizeof(rt->key_event_state));
 memset(rt->com_event_state, 0,
  sizeof(rt->com_event_state));
 rt->pen_event_state = 0;
 rt->play_event_state = EVT_OFF;
 memset(rt->strig_event_state, 0,
  sizeof(rt->strig_event_state));

 // Tier 2: Device I/O interrupt handlers
 memset(rt->on_device_line, 0,
  sizeof(rt->on_device_line));
 memset(rt->device_event_state, 0,
  sizeof(rt->device_event_state));

 // Tier 3: OS / system interrupt handlers
 rt->on_break_line = 0;
 rt->break_event_state = EVT_OFF;
 rt->signal_pending = 0;

 // Tier 4: File I/O event handlers
 rt->on_fileio_line = 0;
 rt->fileio_event_state = EVT_OFF;
 rt->fileio_pending = 0;

 // Event system infrastructure
 rt->event_in_handler = 0;
 rt->evq_head = 0;
 rt->evq_tail = 0;
 rt->evq_count = 0;

 // DEF USR addresses
 memset(rt->usr_addresses, 0,
  sizeof(rt->usr_addresses));

 // Default variable type map (all NONE)
 memset(rt->deftype_map, DEFTYPE_NONE,
  sizeof(rt->deftype_map));

  rt->loaded_pcode = NULL;
  rt->has_loaded_pcode = 0;
  rt->bytecode_only = 0;

  rt->vm_call_stack_capacity = 256;
  rt->vm_call_stack = (int *)malloc((size_t)rt->vm_call_stack_capacity * sizeof(int));
  if (rt->vm_call_stack) {
      memset(rt->vm_call_stack, 0, (size_t)rt->vm_call_stack_capacity * sizeof(int));
  }

  rt->vm_for_stack_capacity = 64;
  rt->vm_for_stack = (VMForFrame *)malloc((size_t)rt->vm_for_stack_capacity * sizeof(VMForFrame));
  if (rt->vm_for_stack) {
      memset(rt->vm_for_stack, 0, (size_t)rt->vm_for_stack_capacity * sizeof(VMForFrame));
  }

    rt->direct_mode = 1;
}

void runtime_cleanup(RuntimeState *rt)
{
  if (rt->vm_call_stack) {
      free(rt->vm_call_stack);
      rt->vm_call_stack = NULL;
  }
  rt->vm_call_stack_capacity = 0;

  if (rt->vm_for_stack) {
      free(rt->vm_for_stack);
      rt->vm_for_stack = NULL;
  }
  rt->vm_for_stack_capacity = 0;

  pcode_cache_invalidate(rt);
}

 // runtime_reset - Reset state for a new RUN.
 //
 // Preserves the program and memory pointers but resets all
 // execution state: variables, stack, array, position.
void runtime_reset(RuntimeState *rt)
{
 int i;

#ifndef BPP_LITE_BUILD
   if (rt->has_loaded_pcode && rt->loaded_pcode != NULL) {
       PCodeProgram *pcode = (PCodeProgram *)rt->loaded_pcode;
       pcode_free(pcode);
       free(pcode);
       rt->loaded_pcode = NULL;
       rt->has_loaded_pcode = 0;
   }
#endif

 rt->running = 0;
 rt->current_index = 0;
 rt->next_index = -1;
 rt->stack_top = 0;
 rt->print_width = DEFAULT_PRINT_WIDTH;
 rt->option_base = 0;
 rt->arithmetic_decimal = 0;
 rt->print_col = 1;
 rt->stopped = 0;

 // Clear variables
 for (i = 0; i < MAX_VARIABLES; i++) {
 rt->variables[i] = bval_int(0);
 }

 // Clear @() array
 if (rt->array_base != NULL) {
 memset(rt->array_base, 0,
 (size_t)(rt->array_size * (long)sizeof(long)));
 }

 // Clear named variables and DATA pool
 rt->named_count = 0;
 memset(rt->named_vars, 0, sizeof(rt->named_vars));
 rt->data_count = 0;
 rt->data_ptr = 0;

 // Reset string variables
 for (i = 0; i < MAX_STRING_VARS; i++) {
 rt->string_vars[i] = bval_string(NULL, 0);
 rt->string_capacities[i] = 0;
 }

 // Reset DIM arrays
 rt->dim_count = 0;
 rt->dim_elements_used = 0;

 // Reset string pool
 strpool_reset(&rt->strpool);

  // Reset trace and error handler
  rt->trace_on = 0;
  rt->debug_on = 0;
  if (!rt->log_fp) {
      rt->log_level = 3; // LOG_ERROR
      rt->log_to_stderr = 0;
      rt->log_fp = NULL;
  }
  rt->on_error_line = 0;

  rt->line_asts = NULL;
  rt->line_asts_count = 0;

  // Reset VM state and eval stack
  rt->vm_state = 0; // VM_STOPPED
  vm_eval_init(&rt->eval_stack);
  rt->acc = bval_int(0);
  rt->chain_pending = 0;
  rt->chain_file[0] = '\0';

  // Reset CONST table
 rt->const_count = 0;

 // Reset SUB/FUNCTION table -- free static storage
 {
  int si;
  for (si = 0; si < rt->sub_count; si++) {
   SubDef *sd = &rt->subs[si];
   if (sd->has_static_data) {
    free(sd->static_vars);
    free(sd->static_strvars);
    free(sd->static_named);
    sd->static_vars = NULL;
    sd->static_strvars = NULL;
    sd->static_named = NULL;
    sd->has_static_data = 0;
   }
  }
 }
 rt->sub_count = 0;
 rt->fn_return_value = bval_int(0);
 rt->in_sub_index = -1;
 rt->suspended = 0;
 rt->resumed = 0;

 // Reset dynamic scope stack (Milestone 9)
 scope_stack_free(&rt->scope_stack);
 scope_stack_init(&rt->scope_stack);

 // Reset label table
 rt->label_count = 0;

 // Reset SCOPE hook state
 rt->scope_before_done = -1;
 rt->scope_after_kw = -1;
 rt->scope_hook_depth = -1;

 // SCOPE event queue
 rt->scope_evq_head = 0;
 rt->scope_evq_tail = 0;
 rt->scope_evq_count = 0;


 // Reset WINDOW state
 rt->win_active = 0;
 rt->win_screen_flag = 0;

 // Reset VIEW PRINT
 rt->view_print_top = 1;
 rt->view_print_bottom = 25;

 // Reset virtual console
 memset(rt->vcon_chars, ' ', sizeof(rt->vcon_chars));
 memset(rt->vcon_colors, 7, sizeof(rt->vcon_colors));

 // Reset event traps
 memset(rt->on_com_line, 0, sizeof(rt->on_com_line));
 memset(rt->on_key_line, 0, sizeof(rt->on_key_line));
 rt->on_pen_line = 0;
 rt->on_play_line = 0;
 memset(rt->on_strig_line, 0,
  sizeof(rt->on_strig_line));
 rt->timer_event_state = EVT_OFF;
 rt->timer_interval = 0.0;
 rt->timer_last_fire = 0.0;
 memset(rt->key_event_state, 0,
  sizeof(rt->key_event_state));
 memset(rt->com_event_state, 0,
  sizeof(rt->com_event_state));
 rt->pen_event_state = 0;
 rt->play_event_state = EVT_OFF;
 memset(rt->strig_event_state, 0,
  sizeof(rt->strig_event_state));
 memset(rt->on_device_line, 0,
  sizeof(rt->on_device_line));
 memset(rt->device_event_state, 0,
  sizeof(rt->device_event_state));
 rt->on_break_line = 0;
 rt->break_event_state = EVT_OFF;
 rt->signal_pending = 0;
 rt->on_fileio_line = 0;
 rt->fileio_event_state = EVT_OFF;
 rt->fileio_pending = 0;
 rt->event_in_handler = 0;
 rt->evq_head = 0;
 rt->evq_tail = 0;
 rt->evq_count = 0;

 // Reset DEF USR
 memset(rt->usr_addresses, 0,
  sizeof(rt->usr_addresses));

 // Reset deftype map
 memset(rt->deftype_map, DEFTYPE_NONE,
  sizeof(rt->deftype_map));
}

// --- Stack Operations ---

 // runtime_push - Push a frame onto the runtime stack.
 //
 // Used by GOSUB (and future FOR/WHILE/DO). Checks for stack
 // overflow and raises ERR_SORRY if full.
int runtime_push(RuntimeState *rt, const StackFrame *frame)
{
    if (rt->stack_top >= MAX_STACK_DEPTH) {
        printf("DEBUG: Stack overflow! Type=%d, Top=%d\n", frame->type, rt->stack_top);
        error_raise(ERR_SORRY, 0);
        return -1;
    }

 rt->stack[rt->stack_top] = *frame;
 rt->stack_top++;
 return 0;
}

 // runtime_pop - Pop a frame with type checking.
 //
 // Verifies that the top frame matches the expected type. This
 // prevents RETURN from popping a FOR frame, or NEXT from popping
 // a GOSUB frame - both of which would corrupt execution.
 //
 // Raises ERR_HOW on type mismatch or empty stack.
int runtime_pop(RuntimeState *rt, FrameType expected, StackFrame *out)
{
 StackFrame *top;

 if (rt->stack_top <= 0) {
 error_raise(ERR_HOW, 0);
 return -1;
 }

 top = &rt->stack[rt->stack_top - 1];

 if (top->type != expected) {
 error_raise(ERR_HOW, 0);
 return -1;
 }

 if (out != NULL) {
 *out = *top;
 }

  rt->stack_top--;

  // Check if we just popped FRAME_SUB and a suspension frame is next!
  if (expected == FRAME_SUB && rt->stack_top > 0 && rt->stack[rt->stack_top - 1].type == FRAME_SUSPEND) {
      StackFrame susp_frame = rt->stack[rt->stack_top - 1];
      rt->stack_top--; // pop the suspension frame too!

      rt->resumed = 1;
      memcpy(rt->restored_lexer, susp_frame.data.suspend.lex_state, sizeof(rt->restored_lexer));
      rt->restored_frame = susp_frame;
      rt->next_index = susp_frame.data.suspend.return_index;
      rt->suspended = 0;
  }

  return 0;
}

// --- Variable Access ---

 // runtime_get_var - Get variable A-Z value.
 //
 // Validates the name is A-Z and returns the stored value.
 // Invalid names return 0 (defensive, should not happen if
 // the lexer correctly identifies variables).
long runtime_get_var(RuntimeState *rt, char name)
{
 int index;
 if (name < 'A' || name > 'Z') return 0;
 index = name - 'A';
 return bval_to_int(&rt->variables[index]);
}

BValue runtime_get_var_bval(RuntimeState *rt, char name)
{
 int index;
 unsigned char dtype;
 BValue v;
 if (name < 'A' || name > 'Z') return bval_int(0);
 index = name - 'A';
 v = rt->variables[index];
 // Coerce on read per deftype_map
 dtype = rt->deftype_map[index];
 if (dtype == DEFTYPE_INT) {
  v = bval_int(bval_to_int(&v));
 } else if (dtype == DEFTYPE_SNG ||
  dtype == DEFTYPE_DBL) {
  v = bval_float(bval_to_float(&v));
 }
 return v;
}

void runtime_set_var(RuntimeState *rt, char name, long value)
{
	int index;
	if (name < 'A' || name > 'Z') return;
	index = name - 'A';
	rt->variables[index] = bval_int(value);
	boot_log(BOOT_VERBOSE, "Variable Assignment: %c = %ld", name, value);
}

void runtime_set_var_bval(RuntimeState *rt, char name, BValue value)
{
	int index;
	unsigned char dtype;
	if (name < 'A' || name > 'Z') return;
	index = name - 'A';
	// Enforce deftype_map coercion
	dtype = rt->deftype_map[index];
	if (dtype == DEFTYPE_INT) {
		value = bval_int(bval_to_int(&value));
	} else if (dtype == DEFTYPE_SNG ||
		dtype == DEFTYPE_DBL) {
		value = bval_float(bval_to_float(&value));
	} else if (dtype == DEFTYPE_STR) {
		if (!bval_is_string(&value)) {
			error_raise(ERR_WHAT, 0);
			return;
		}
	}
	rt->variables[index] = value;
	if (bval_is_string(&value)) {
		char sbuf[256];
		bval_to_string_buf(&value, sbuf, sizeof(sbuf));
		boot_log(BOOT_VERBOSE, "Variable Assignment: %c$ = \"%s\"", name, sbuf);
	} else if (bval_is_float(&value)) {
		boot_log(BOOT_VERBOSE, "Variable Assignment: %c = %f", name, bval_to_float(&value));
	} else {
		boot_log(BOOT_VERBOSE, "Variable Assignment: %c = %ld", name, bval_to_int(&value));
	}
}

// --- Array Access ---

 // runtime_get_array - Get @(index) value with bounds checking.
 //
 // Raises ERR_HOW for out-of-bounds access. In PATB, the valid
 // index range is 0 to SIZE/2 (approximately). We use 0 to
 // array_size-1.
long runtime_get_array(RuntimeState *rt, long index)
{
 if (index < 0 || index >= rt->array_size) {
 error_raise(ERR_HOW, 0);
 return 0;
 }

 return rt->array_base[index];
}

 // runtime_set_array - Set @(index) value with bounds checking.
void runtime_set_array(RuntimeState *rt, long index, long value)
{
 if (index < 0 || index >= rt->array_size) {
 error_raise(ERR_HOW, 0);
 return;
 }

 rt->array_base[index] = value;
}

// --- Random Number Generator ---
 // Uses a linear congruential generator (LCG) with parameters from
 // the Numerical Recipes family. These parameters are chosen for:
 //
 // 1. Reasonable distribution without requiring 64-bit types.
 // 2. Portability across 16-bit and 32-bit targets.
 // 3. No dependency on <time.h> or OS-specific APIs.
 //
 // The formula: seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
 //
 // This is the same LCG used by many C library rand() implementations
 // (POSIX / glibc). Using our own ensures identical behavior across
 // all target platforms.
 //
 // PATB's RND(X) returns a value between 1 and X inclusive.
 // If X <= 0, we return 1 (defensive).
long runtime_rnd(RuntimeState *rt, long max)
{
 long result;

 // Advance the LCG
 rt->rnd_seed = (rt->rnd_seed * 1103515245UL + 12345UL) & 0x7FFFFFFFUL;

 if (max <= 0) {
 return 1;
 }

 // Map to range [1, max]
 result = (long)(rt->rnd_seed % (unsigned long)max) + 1;
 return result;
}

 // runtime_size - Return available memory for @() array.
 //
 // Reports the total variable pool size (which determines @() capacity).
 // In PATB, SIZE returns the free memory in bytes. We report the full
 // variable pool size since we don't use it for anything else.
long runtime_size(RuntimeState *rt)
{
 return rt->memory->variable.size;
}

// --- Forward Scan - Find Matching Loop End ---
 // When a WHILE condition is false or a DO condition fails the
 // pre-check, we need to skip forward to the matching WEND or LOOP.
 // This requires scanning through program lines, tokenizing each
 // one, and tracking nesting depth.
 //
 // Algorithm:
 // 1. Start at start_index + 1.
 // 2. For each line, tokenize and check the first keyword.
 // 3. If it matches open_kw, increment depth (nested loop).
 // 4. If it matches close_kw:
 // a. If depth == 0, this is our match - return this index.
 // b. Otherwise, decrement depth.
 // 5. If we reach the end of the program, raise ERR_HOW.
 //
 // This is an O(n) scan through program lines, which is acceptable
 // because it only happens when a loop condition is false (not on
 // every iteration).
int runtime_find_matching(RuntimeState *rt, int start_index,
 int open_kw, int close_kw, double line_num)
{
 int depth = 0;
 int i;
 Lexer scan_lex;

    for (i = start_index; i < rt->program->count; i++) {
        lexer_init(&scan_lex, rt->program->lines[i].text);

        if (scan_lex.current.type == TOK_NUMBER || scan_lex.current.type == TOK_FLOAT_LIT) {
            lexer_next(&scan_lex);
        }

        int is_start_line = (i == start_index);
        int first_open_skipped = 0;

        while (scan_lex.current.type != TOK_EOF) {
            if (scan_lex.current.type == TOK_KEYWORD) {
                if ((int)scan_lex.current.value.keyword == open_kw) {
                    if (is_start_line && !first_open_skipped) {
                        first_open_skipped = 1;
                    } else {
                        depth++;
                    }
                } else if ((int)scan_lex.current.value.keyword == close_kw) {
                    if (depth == 0) {
                        return i;
                    }
                    depth--;
                }
            }
            lexer_next(&scan_lex);
        }
    }


 // No matching end found - mismatched loop structure
 error_raise(ERR_HOW, line_num);
 return -1;
}

// --- Named Variable Access ---
 // Named variables use a linear-search table. At MAX_NAMED_VARS=256,
 // linear search is efficient enough. Variables are matched by a
 // case-insensitive comparison of up to MAX_VAR_NAME_LEN characters.

 // str_eq_nocase - Compare two strings case-insensitively.
 //
 // Compares name (with given length, not necessarily null-terminated)
 // against stored (null-terminated). Returns 1 if equal, 0 if not.
static int str_eq_nocase(const char *name, int len, const char *stored)
{
 int i;
 for (i = 0; i < len; i++) {
 char a = name[i];
 char b = stored[i];
 if (b == '\0') return 0; // stored is shorter
 if (a >= 'a' && a <= 'z') a = (char)(a - 32);
 if (b >= 'a' && b <= 'z') b = (char)(b - 32);
 if (a != b) return 0;
 }
 return (stored[len] == '\0'); // stored must end here too
}

 // runtime_get_named_var - Look up a named variable.
 //
 // Returns its value, or 0 if not yet defined.
long runtime_get_named_var(RuntimeState *rt, const char *name, int len)
{
 int i;
 for (i = 0; i < rt->named_count; i++) {
 if (str_eq_nocase(name, len, rt->named_vars[i].name)) {
 return bval_to_int(&rt->named_vars[i].value);
 }
 }
 return 0;
}

BValue runtime_get_named_var_bval(RuntimeState *rt, const char *name,
 int len)
{
 int i;
 for (i = 0; i < rt->named_count; i++) {
 if (str_eq_nocase(name, len, rt->named_vars[i].name)) {
 return rt->named_vars[i].value;
 }
 }
 // Variable not found. Return default:
 // String vars (name ends with $) -> empty string
 // Numeric vars -> 0
 if (len > 0 && name[len - 1] == '$') {
 return bval_string(NULL, 0);
 }
 return bval_int(0);
}

int runtime_set_named_var(RuntimeState *rt, const char *name, int len,
 long value)
{
 return runtime_set_named_var_bval(rt, name, len, bval_int(value));
}

int runtime_set_named_var_bval(RuntimeState *rt, const char *name,
 int len, BValue value)
{
 int i;
 int copy_len;

 // Type suffix enforcement
 if (len > 0) {
  char last = name[len - 1];
  if (last == '%') {
  value = bval_int(bval_to_int(&value));
  } else if (last == '!') {
  value = bval_float(bval_to_float(&value));
  } else if (last == '#') {
  value = bval_float(bval_to_float(&value));
  } else if (last == '&') {
  value = bval_int(bval_to_int(&value));
  } else if (last == '$') {
  if (!bval_is_string(&value)) {
   error_raise(ERR_WHAT, 0);
   return -1;
  }
  }
 }

  // Look for existing variable
  for (i = 0; i < rt->named_count; i++) {
  if (str_eq_nocase(name, len, rt->named_vars[i].name)) {
  if (value.type == VAL_STRING) {
      int cap = rt->named_vars[i].capacity;
      if (cap > 0 && value.v.sval.length > cap) {
          value.v.sval.length = cap;
      }
  }
  rt->named_vars[i].value = value;
  return 0;
  }
  }

 // Create new variable
 if (rt->named_count >= MAX_NAMED_VARS) {
 error_raise(ERR_SORRY, 0);
 return -1;
 }

 copy_len = len;
 if (copy_len > MAX_VAR_NAME_LEN) {
 copy_len = MAX_VAR_NAME_LEN;
 }
 memcpy(rt->named_vars[rt->named_count].name, name, (size_t)copy_len);
 rt->named_vars[rt->named_count].name[copy_len] = '\0';

 // Uppercase the stored name for consistent lookups
 for (i = 0; i < copy_len; i++) {
 char c = rt->named_vars[rt->named_count].name[i];
 if (c >= 'a' && c <= 'z') {
 rt->named_vars[rt->named_count].name[i] = (char)(c - 32);
 }
 }

 rt->named_vars[rt->named_count].value = value;
 rt->named_count++;
 return 0;
}

// --- String Variable Access ---

BValue runtime_get_string_var(RuntimeState *rt, char name)
{
 int index;
 if (name < 'A' || name > 'Z') return bval_string(NULL, 0);
 index = name - 'A';
 return rt->string_vars[index];
}

void runtime_set_string_var(RuntimeState *rt, char name, BValue value)
{
    int index;
    if (name < 'A' || name > 'Z') return;
    index = name - 'A';
    if (value.type == VAL_STRING) {
        int cap = rt->string_capacities[index];
        if (cap > 0 && value.v.sval.length > cap) {
            value.v.sval.length = cap;
        }
    }
    rt->string_vars[index] = value;
}

void runtime_set_string_capacity(RuntimeState *rt, const char *name, int len, int capacity)
{
    if (len == 2 && name[1] == '$') {
        char c = name[0];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        if (c >= 'A' && c <= 'Z') {
            rt->string_capacities[c - 'A'] = capacity;
            rt->string_vars[c - 'A'] = bval_string("", 0);
            return;
        }
    }
    // Check named vars
    int i;
    for (i = 0; i < rt->named_count; i++) {
        if (str_eq_nocase(name, len, rt->named_vars[i].name)) {
            rt->named_vars[i].capacity = capacity;
            rt->named_vars[i].value = bval_string("", 0);
            return;
        }
    }
    // Create if not exists
    if (rt->named_count < MAX_NAMED_VARS) {
        int copy_len = len < MAX_VAR_NAME_LEN ? len : MAX_VAR_NAME_LEN;
        memcpy(rt->named_vars[rt->named_count].name, name, (size_t)copy_len);
        rt->named_vars[rt->named_count].name[copy_len] = '\0';
        for (i = 0; i < copy_len; i++) {
            char c = rt->named_vars[rt->named_count].name[i];
            if (c >= 'a' && c <= 'z') rt->named_vars[rt->named_count].name[i] = (char)(c - 32);
        }
        rt->named_vars[rt->named_count].value = bval_string("", 0);
        rt->named_vars[rt->named_count].capacity = capacity;
        rt->named_count++;
    }
}

// --- DIM Array Support ---

DimArray *runtime_find_dim(RuntimeState *rt, const char *name,
 int name_len)
{
 int i;
 for (i = 0; i < rt->dim_count; i++) {
 if (str_eq_nocase(name, name_len, rt->dim_arrays[i].name)) {
 return &rt->dim_arrays[i];
 }
 }
 return NULL;
}

int runtime_dim(RuntimeState *rt, const char *name, int name_len,
 int dim1, int dim2, int dim3, double line_num)
{
 DimArray *arr;
 int total;
 int copy_len;
 int i;

 // Check if already DIMmed
 if (runtime_find_dim(rt, name, name_len) != NULL) {
 error_raise(ERR_HOW, line_num); // re-DIM error
 return -1;
 }

 if (rt->dim_count >= MAX_DIM_ARRAYS) {
 error_raise(ERR_SORRY, line_num);
 return -1;
 }

 // BASIC arrays: DIM A(10) gives elements base..10
 total = dim1 + 1 - rt->option_base;
 if (dim2 > 0) {
 total *= (dim2 + 1 - rt->option_base);
 }
 if (dim3 > 0) {
 total *= (dim3 + 1 - rt->option_base);
 }

 if (rt->dim_elements_used + total > MAX_ARRAY_ELEMENTS) {
 error_raise(ERR_SORRY, line_num);
 return -1;
 }

 arr = &rt->dim_arrays[rt->dim_count];

 copy_len = name_len;
 if (copy_len > MAX_VAR_NAME_LEN) copy_len = MAX_VAR_NAME_LEN;
 memcpy(arr->name, name, (size_t)copy_len);
 arr->name[copy_len] = '\0';
 // Uppercase
 for (i = 0; i < copy_len; i++) {
 if (arr->name[i] >= 'a' && arr->name[i] <= 'z') {
 arr->name[i] = (char)(arr->name[i] - 32);
 }
 }

 if (dim3 > 0) {
 arr->dims = 3;
 } else if (dim2 > 0) {
 arr->dims = 2;
 } else {
 arr->dims = 1;
 }
 arr->size[0] = dim1 + 1 - rt->option_base;
 arr->size[1] = (dim2 > 0) ? dim2 + 1 - rt->option_base : 0;
 arr->size[2] = (dim3 > 0) ? dim3 + 1 - rt->option_base : 0;
 arr->elements = &rt->dim_elements[rt->dim_elements_used];
 arr->total = total;
 arr->type_index = -1; // normal (non-typed) array
 arr->v_channel = 0;
 arr->v_elem_size = 0;
 arr->capacity = 0;

 // Initialize elements.
 // String arrays (name ends with '$') default to empty
 // strings. Numeric arrays default to 0.
 {
 int is_string_arr = (copy_len > 0 &&
 arr->name[copy_len - 1] == '$');
 for (i = 0; i < total; i++) {
 if (is_string_arr) {
 arr->elements[i] = bval_string(NULL, 0);
 } else {
 arr->elements[i] = bval_int(0);
 }
 }
 }

 rt->dim_elements_used += total;
 rt->dim_count++;
 return 0;
}

BValue runtime_get_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, int idx3, double line_num)
{
 DimArray *arr = runtime_find_dim(rt, name, name_len);
 int offset;

 if (arr == NULL) {
 // GW-BASIC auto-DIM: create array with default size 10
 if (1) {
  runtime_dim(rt, name, name_len, 10, 0, 0, line_num);
  if (error_occurred()) return bval_int(0);
  arr = runtime_find_dim(rt, name, name_len);
 }
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return bval_int(0);
 }
 }

 if (arr->dims == 1) {
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0]) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 offset = idx1 - rt->option_base;
 } else if (arr->dims == 2) {
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0] ||
 idx2 - rt->option_base < 0 ||
 idx2 - rt->option_base >= arr->size[1]) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 offset = (idx1 - rt->option_base) * arr->size[1]
 + (idx2 - rt->option_base);
 } else {
 // 3D
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0] ||
 idx2 - rt->option_base < 0 ||
 idx2 - rt->option_base >= arr->size[1] ||
 idx3 - rt->option_base < 0 ||
 idx3 - rt->option_base >= arr->size[2]) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 offset = ((idx1 - rt->option_base) * arr->size[1]
 + (idx2 - rt->option_base)) * arr->size[2]
 + (idx3 - rt->option_base);
 }

 if (arr->v_channel > 0) {
     extern FILE *fileio_get_fp(int chan);
     FILE *fp = fileio_get_fp(arr->v_channel);
     if (fp == NULL) {
         error_raise(ERR_HOW, line_num);
         return bval_int(0);
     }
     long file_offset = (long)offset * arr->v_elem_size;
     fseek(fp, file_offset, SEEK_SET);
     if (arr->v_elem_size == 2) {
         short val = 0;
         if (fread(&val, 2, 1, fp) != 1) {
             val = 0;
         }
         return bval_int(val);
     } else if (arr->v_elem_size == 4) {
         int val = 0;
         if (fread(&val, 4, 1, fp) != 1) {
             val = 0;
         }
         return bval_int(val);
     } else {
         int val = 0;
         fread(&val, 1, (size_t)arr->v_elem_size, fp);
         return bval_int(val);
     }
 }

 return arr->elements[offset];
}

void runtime_set_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, int idx3, BValue val, double line_num)
{
 DimArray *arr = runtime_find_dim(rt, name, name_len);
 int offset;

 if (arr == NULL) {
 // GW-BASIC auto-DIM: create array with default size 10
 if (1) {
  runtime_dim(rt, name, name_len, 10, 0, 0, line_num);
  if (error_occurred()) return;
  arr = runtime_find_dim(rt, name, name_len);
 }
 if (arr == NULL) {
  error_raise(ERR_HOW, line_num);
  return;
 }
 }

 if (arr->dims == 1) {
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 offset = idx1 - rt->option_base;
 } else if (arr->dims == 2) {
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0] ||
 idx2 - rt->option_base < 0 ||
 idx2 - rt->option_base >= arr->size[1]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 offset = (idx1 - rt->option_base) * arr->size[1]
 + (idx2 - rt->option_base);
 } else {
 // 3D
 if (idx1 - rt->option_base < 0 ||
 idx1 - rt->option_base >= arr->size[0] ||
 idx2 - rt->option_base < 0 ||
 idx2 - rt->option_base >= arr->size[1] ||
 idx3 - rt->option_base < 0 ||
 idx3 - rt->option_base >= arr->size[2]) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 offset = ((idx1 - rt->option_base) * arr->size[1]
 + (idx2 - rt->option_base)) * arr->size[2]
 + (idx3 - rt->option_base);
 }

 if (arr->v_channel > 0) {
     extern FILE *fileio_get_fp(int chan);
     FILE *fp = fileio_get_fp(arr->v_channel);
     if (fp == NULL) {
         error_raise(ERR_HOW, line_num);
         return;
     }
     long file_offset = (long)offset * arr->v_elem_size;
     fseek(fp, file_offset, SEEK_SET);
     long int_val = bval_to_int(&val);
     if (arr->v_elem_size == 2) {
         short sval = (short)int_val;
         fwrite(&sval, 2, 1, fp);
     } else if (arr->v_elem_size == 4) {
         int ival = (int)int_val;
         fwrite(&ival, 4, 1, fp);
     } else {
         fwrite(&int_val, 1, (size_t)arr->v_elem_size, fp);
     }
     fflush(fp);
     return;
 }

 if (val.type == VAL_STRING && arr->capacity > 0) {
     if (val.v.sval.length > arr->capacity) {
         val.v.sval.length = arr->capacity;
     }
 }
 arr->elements[offset] = val;
}

// --- DATA Pool (, extended for BValue) ---

void runtime_collect_data(RuntimeState *rt)
{
 int i;
 rt->data_count = 0;
 rt->data_ptr = 0;

 for (i = 0; i < rt->program->count; i++) {
 Lexer scan_lex;
 lexer_init(&scan_lex, rt->program->lines[i].text);

 // Skip line number
 if (scan_lex.current.type == TOK_NUMBER || scan_lex.current.type == TOK_FLOAT_LIT || scan_lex.current.type == TOK_FLOAT_LIT) {
 lexer_next(&scan_lex);
 }

 // Check for DATA keyword
 if (scan_lex.current.type == TOK_KEYWORD &&
 scan_lex.current.value.keyword == KW_DATA) {
 lexer_next(&scan_lex); // consume DATA

 // Parse comma-separated values
 while (scan_lex.current.type != TOK_EOF &&
 scan_lex.current.type != TOK_CR) {
 int negate = 0;

 // Handle optional sign
 if (scan_lex.current.type == TOK_MINUS) {
 negate = 1;
 lexer_next(&scan_lex);
 } else if (scan_lex.current.type == TOK_PLUS) {
 lexer_next(&scan_lex);
 }

 if (scan_lex.current.type == TOK_NUMBER) {
 long val = scan_lex.current.value.num_value;
 if (negate) val = -val;
 lexer_next(&scan_lex);
 if (rt->data_count < MAX_DATA_ITEMS) {
 rt->data_pool[rt->data_count++] = bval_int(val);
 }
 } else if (scan_lex.current.type == TOK_FLOAT_LIT) {
 double fval = scan_lex.current.value.fval;
 if (negate) fval = -fval;
 lexer_next(&scan_lex);
 if (rt->data_count < MAX_DATA_ITEMS) {
 rt->data_pool[rt->data_count++] = bval_float(fval);
 }
 } else if (scan_lex.current.type == TOK_STRING) {
 // String DATA item
 int slen = scan_lex.current.str_length;
 char *ptr = strpool_store(&rt->strpool,
 scan_lex.current.str_start,
 slen);
 lexer_next(&scan_lex);
 if (rt->data_count < MAX_DATA_ITEMS && ptr != NULL) {
 rt->data_pool[rt->data_count++] =
 bval_string(ptr, slen);
 }
 } else {
 break;
 }

 // Skip comma separator
 if (scan_lex.current.type == TOK_COMMA) {
 lexer_next(&scan_lex);
 } else {
 break;
 }
 }
 }
 }
}

BValue runtime_read_data_bval(RuntimeState *rt, double line_num)
{
 if (rt->data_ptr >= rt->data_count) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 return rt->data_pool[rt->data_ptr++];
}

long runtime_read_data(RuntimeState *rt, double line_num)
{
 BValue v = runtime_read_data_bval(rt, line_num);
 return bval_to_int(&v);
}

void runtime_restore_data(RuntimeState *rt)
{
 rt->data_ptr = 0;
}

// --- User-Defined Functions (DEF FN) ---
 //
 // IMPLEMENTATION NOTES:
 //
 // DEF FN stores function definitions as text. When FN is invoked,
 // the body text is re-lexed and re-parsed as an expression. This
 // approach:
 // 1. Matches historical BASIC behavior (text-based DEF FN)
 // 2. Requires no bytecode or AST storage
 // 3. Is memory-efficient (just a string)
 // 4. Supports all expression features naturally
 //
 // LOCAL SCOPING:
 // Parameter variables are saved before evaluation and restored
 // after. This provides transparent local scoping that matches
 // Dartmouth BASIC, GW-BASIC, and AppleSoft semantics.
 //
 // REDEFINITION:
 // Redefining a function (DEF FNA twice) replaces the previous
 // definition. This matches GW-BASIC and most dialects.

 // runtime_def_fn - Define or redefine a user function.
 //
 // If a function with the same name already exists, it is replaced.
 // Otherwise a new entry is created.
 //
 // Returns 0 on success, -1 if the function table is full.
int runtime_def_fn(RuntimeState *rt, const char *name, int name_len,
 const char *params, int param_count,
 const char *body, int body_len)
{
 int i;
 UserFunction *fn;

 // Check for existing definition (allow redefinition)
 fn = runtime_find_fn(rt, name, name_len);
 if (fn == NULL) {
 // New definition
 if (rt->user_func_count >= MAX_USER_FUNCS) {
 return -1; // table full
 }
 fn = &rt->user_funcs[rt->user_func_count++];
 }

 // Store function name
 if (name_len > MAX_VAR_NAME_LEN) {
 name_len = MAX_VAR_NAME_LEN;
 }
 for (i = 0; i < name_len; i++) {
 fn->name[i] = name[i];
 }
 fn->name[name_len] = '\0';
 fn->name_len = name_len;

 // Store parameter names
 fn->param_count = param_count;
 for (i = 0; i < param_count && i < MAX_FN_PARAMS; i++) {
 fn->params[i] = params[i];
 }

 // Store body expression
 if (body_len > MAX_FN_BODY) {
 body_len = MAX_FN_BODY;
 }
 memcpy(fn->body, body, (size_t)body_len);
 fn->body[body_len] = '\0';
 fn->body_len = body_len;

 return 0;
}

 // runtime_find_fn - Look up a user function by name.
 //
 // Case-insensitive comparison. Returns pointer to the UserFunction
 // entry, or NULL if not found.
UserFunction *runtime_find_fn(RuntimeState *rt, const char *name,
 int name_len)
{
 int i;

 for (i = 0; i < rt->user_func_count; i++) {
 UserFunction *fn = &rt->user_funcs[i];
 int j, match;

 if (fn->name_len != name_len) continue;

 match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = fn->name[j];
 if (a >= 'a' && a <= 'z') a = (char)(a - 32);
 if (b >= 'a' && b <= 'z') b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 }
 if (match) return fn;
 }

 return NULL;
}

// --- Breakpoint Management ---

 // runtime_breakpoint_add - Add a breakpoint at line_num.
 // Returns 0 on success, -1 if table full or already set.
int runtime_breakpoint_add(RuntimeState *rt, double line_num)
{
 int i;

 // Check for duplicate
 for (i = 0; i < rt->breakpoint_count; i++) {
 if (rt->breakpoints[i] == line_num) {
 return 0; // already set
 }
 }

 if (rt->breakpoint_count >= MAX_BREAKPOINTS) {
 printf("Breakpoint table full (%d max).\n",
 MAX_BREAKPOINTS);
 return -1;
 }

 rt->breakpoints[rt->breakpoint_count++] = line_num;
 return 0;
}

 // runtime_breakpoint_remove - Remove breakpoint at line_num.
 // Returns 0 on success, -1 if not found.
int runtime_breakpoint_remove(RuntimeState *rt, double line_num)
{
 int i;

 for (i = 0; i < rt->breakpoint_count; i++) {
 if (rt->breakpoints[i] == line_num) {
 // Shift remaining down
 int j;
 for (j = i; j < rt->breakpoint_count - 1; j++) {
 rt->breakpoints[j] = rt->breakpoints[j + 1];
 }
 rt->breakpoint_count--;
 return 0;
 }
 }

 return -1;
}

 // runtime_breakpoint_clear - Remove all breakpoints.
void runtime_breakpoint_clear(RuntimeState *rt)
{
 rt->breakpoint_count = 0;
}

 // runtime_is_breakpoint - Check if line_num has a breakpoint.
 // Returns 1 if breakpoint set, 0 otherwise.
int runtime_is_breakpoint(RuntimeState *rt, double line_num)
{
 int i;

 for (i = 0; i < rt->breakpoint_count; i++) {
 if (rt->breakpoints[i] == line_num) {
 return 1;
 }
 }

 return 0;
}

 // runtime_breakpoint_list - Print all breakpoints.
void runtime_breakpoint_list(RuntimeState *rt)
{
 int i;

 if (rt->breakpoint_count == 0) {
 printf("No breakpoints set.\n");
 return;
 }

 printf("Breakpoints:");
 for (i = 0; i < rt->breakpoint_count; i++) {
 double bp = rt->breakpoints[i];
 if (floor(bp) == bp) {
     printf(" %.0f", bp);
 } else {
     printf(" %.2f", bp);
 }
 }
 printf("\n");
}

// --- Label Collection and Lookup ---
 // Scan all program lines for labels. A label is an identifier
 // followed by a colon at the start of a line (after optional
 // line number). Labels are stored in a table for GOTO/GOSUB
 // resolution.

 // runtime_collect_labels - Build the label table from program.
void runtime_collect_labels(RuntimeState *rt)
{
 int i;
 rt->label_count = 0;

 for (i = 0; i < rt->program->count; i++) {
 const char *text = rt->program->lines[i].text;
 int pos = 0;
 int tlen = (int)strlen(text);
 int name_start, name_len, j, copy_len;

 // Skip leading whitespace
 while (pos < tlen && (text[pos] == ' ' ||
 text[pos] == '\t'))
 pos++;

 // Skip optional line number
 if (pos < tlen && text[pos] >= '0' &&
 text[pos] <= '9') {
 while (pos < tlen && text[pos] >= '0' &&
 text[pos] <= '9')
 pos++;
 // Skip space after line number
 while (pos < tlen && (text[pos] == ' ' ||
 text[pos] == '\t'))
 pos++;
 }

 // Check for alphabetic identifier
 if (pos >= tlen) continue;
 if (!((text[pos] >= 'A' && text[pos] <= 'Z') ||
 (text[pos] >= 'a' && text[pos] <= 'z')))
 continue;

 name_start = pos;
 while (pos < tlen &&
 ((text[pos] >= 'A' && text[pos] <= 'Z') ||
 (text[pos] >= 'a' && text[pos] <= 'z') ||
 (text[pos] >= '0' && text[pos] <= '9') ||
 text[pos] == '_'))
 pos++;
 name_len = pos - name_start;

 // Skip optional whitespace before colon
 while (pos < tlen && text[pos] == ' ')
 pos++;

 // Must be followed by colon
 if (pos >= tlen || text[pos] != ':')
 continue;

 // Don't treat keywords like REM: as labels
 // Simple heuristic: skip common keywords
 if (name_len <= 6) {
 char upper[8];
 for (j = 0; j < name_len && j < 7; j++) {
 upper[j] = text[name_start + j];
 if (upper[j] >= 'a' && upper[j] <= 'z')
 upper[j] = (char)(upper[j] - 32);
 }
 upper[j] = '\0';
 if (strcmp(upper, "REM") == 0 ||
 strcmp(upper, "IF") == 0 ||
 strcmp(upper, "THEN") == 0 ||
 strcmp(upper, "ELSE") == 0)
 continue;
 }

 // Store label
 if (rt->label_count >= MAX_LABELS) continue;

 copy_len = name_len;
 if (copy_len > MAX_VAR_NAME_LEN)
 copy_len = MAX_VAR_NAME_LEN;
 memcpy(rt->labels[rt->label_count].name,
 text + name_start, (size_t)copy_len);
 rt->labels[rt->label_count].name[copy_len] = '\0';
 // Uppercase for consistent lookup
 for (j = 0; j < copy_len; j++) {
 char c = rt->labels[rt->label_count].name[j];
 if (c >= 'a' && c <= 'z')
 rt->labels[rt->label_count].name[j] =
 (char)(c - 32);
 }
 rt->labels[rt->label_count].program_index = i;
 rt->label_count++;
 }
}

 // runtime_find_label - Look up a label by name.
 //
 // Returns program index, or -1 if not found.
int runtime_find_label(RuntimeState *rt, const char *name,
 int len)
{
 int i, j;
 char upper[MAX_VAR_NAME_LEN + 1];
 int clen = len;

 if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
 for (j = 0; j < clen; j++) {
 upper[j] = name[j];
 if (upper[j] >= 'a' && upper[j] <= 'z')
 upper[j] = (char)(upper[j] - 32);
 }
 upper[clen] = '\0';

 for (i = 0; i < rt->label_count; i++) {
 if ((int)strlen(rt->labels[i].name) == clen &&
 memcmp(rt->labels[i].name, upper,
 (size_t)clen) == 0) {
 return rt->labels[i].program_index;
 }
 }
 return -1;
}

// --- SUB/FUNCTION Lookup ---

 // runtime_find_sub - Look up a SUB or FUNCTION by name.
SubDef *runtime_find_sub(RuntimeState *rt, const char *name,
 int len)
{
 int i, j;
 char upper[MAX_VAR_NAME_LEN + 1];
 int clen = len;

 if (clen > MAX_VAR_NAME_LEN) clen = MAX_VAR_NAME_LEN;
 for (j = 0; j < clen; j++) {
 upper[j] = name[j];
 if (upper[j] >= 'a' && upper[j] <= 'z')
 upper[j] = (char)(upper[j] - 32);
 }
 upper[clen] = '\0';

 for (i = 0; i < rt->sub_count; i++) {
 if (rt->subs[i].name_len == clen &&
 memcmp(rt->subs[i].name, upper,
 (size_t)clen) == 0) {
 return &rt->subs[i];
 }
 }
 return NULL;
}

extern int bpp_load(ProgramStore *prog, const char *filename, void *rt_ptr);
extern int bpe_load(const char *filename, ProgramStore *prog, void *rt_ptr);
extern int fileio_load(ProgramStore *store, const char *filename);
extern void parser_execute_line(Lexer *lex, RuntimeState *rt, double line_num);

int runtime_load_external_sub(RuntimeState *rt, SubDef *sd)
{
    ProgramStore temp_prog;
    unsigned char magic[4] = {0};
    FILE *mf;
    int load_ok = 0;
    double max_no = 0.0;
    int l;
    double start_line_no;
    int idx;
    int first_new_idx;
    int scan_idx;
    ProgramStore *pgm;
    int saved_curr;
    int saved_next;

    if (sd->external_file[0] == '\0') {
        return -1;
    }

    temp_prog.count = 0;
    temp_prog.capacity = MAX_PROGRAM_LINES;
    temp_prog.bulk_buffer = NULL;
    temp_prog.bulk_size = 0;
    temp_prog.lines = (ProgramLine *)calloc(MAX_PROGRAM_LINES, sizeof(ProgramLine));
    if (temp_prog.lines == NULL) {
        return -1;
    }

    mf = fopen(sd->external_file, "rb");
    if (mf) {
        if (fread(magic, 1, 4, mf) != 4) {
            memset(magic, 0, 4);
        }
        fclose(mf);
    } else {
        free(temp_prog.lines);
        return -1;
    }

    if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'E' && magic[3] == '\x1A') {
        load_ok = (bpe_load(sd->external_file, &temp_prog, rt) == 0);
    } else if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'P' && (magic[3] == '\x1B' || magic[3] == '\x1A')) {
        load_ok = (bpp_load(&temp_prog, sd->external_file, rt) == 0);
    } else if (magic[0] == 'B' && magic[1] == 'P' && magic[2] == 'L' && magic[3] == '\x1A') {
        LoadedLibrary bpl_lib;
        memset(&bpl_lib, 0, sizeof(LoadedLibrary));
        if (bpl_load(sd->external_file, &bpl_lib) == 0) {
            int i;
            for (i = 0; i < bpl_lib.src_line_count && i < MAX_PROGRAM_LINES; i++) {
                temp_prog.lines[i].line_number = bpl_lib.src_lines[i].vline;
                char *new_txt = (char *)malloc(strlen(bpl_lib.src_lines[i].text) + 1);
                if (new_txt) {
                    strcpy(new_txt, bpl_lib.src_lines[i].text);
                    temp_prog.lines[i].text = new_txt;
                    temp_prog.count++;
                }
            }
            if (bpl_lib.src_lines) {
                free(bpl_lib.src_lines);
            }
            load_ok = 1;
        }
    } else {
        load_ok = (fileio_load(&temp_prog, sd->external_file) == 0);
    }

    if (!load_ok || temp_prog.count <= 0) {
        free(temp_prog.lines);
        return -1;
    }

    for (l = 0; l < rt->program->count; l++) {
        if (rt->program->lines[l].line_number > max_no) {
            max_no = rt->program->lines[l].line_number;
        }
    }
    start_line_no = max_no + 1000;
    if (start_line_no < 900000) start_line_no = 900000;

    for (idx = 0; idx < temp_prog.count; idx++) {
        double target_line_no = start_line_no + idx * 10;
        const char *text = temp_prog.lines[idx].text;
        char new_text[MAX_LINE_LENGTH + 32];
        
        while (*text == ' ' || *text == '\t') text++;
        while (*text >= '0' && *text <= '9') text++;
        while (*text == ' ' || *text == '\t') text++;
        
        snprintf(new_text, sizeof(new_text), "%.0f %s", target_line_no, text);
        if (program_insert(rt->program, target_line_no, new_text) != 0) {
            program_clear(&temp_prog);
            free(temp_prog.lines);
            return -1;
        }
    }
    program_clear(&temp_prog);
    free(temp_prog.lines);

    first_new_idx = program_find(rt->program, start_line_no);
    if (first_new_idx < 0) {
        return -1;
    }

    pgm = rt->program;
    saved_curr = rt->current_index;
    saved_next = rt->next_index;
    for (scan_idx = first_new_idx; scan_idx < pgm->count; scan_idx++) {
        Lexer cl;
        const char *text = pgm->lines[scan_idx].text;
        double ln = pgm->lines[scan_idx].line_number;
        lexer_init(&cl, text);
        if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT || cl.current.type == TOK_FLOAT_LIT)
            lexer_next(&cl);
        if (cl.current.type == TOK_KEYWORD &&
            (cl.current.value.keyword == KW_SUB ||
             cl.current.value.keyword == KW_FUNCTION)) {
            rt->current_index = scan_idx;
            rt->next_index = -1;
            parser_execute_line(&cl, rt, ln);
            if (error_occurred()) {
                rt->current_index = saved_curr;
                rt->next_index = saved_next;
                return -1;
            }
            if (rt->next_index > scan_idx)
                scan_idx = rt->next_index - 1;
        }
    }
    rt->current_index = saved_curr;
    rt->next_index = saved_next;

    return 0;
}

void runtime_pre_scan_external_declarations(RuntimeState *rt)
{
    int idx;
    ProgramStore *pgm = rt->program;
    if (!pgm) return;
    for (idx = 0; idx < pgm->count; idx++) {
        Lexer cl;
        const char *text = pgm->lines[idx].text;
        lexer_init(&cl, text);
        if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT || cl.current.type == TOK_FLOAT_LIT)
            lexer_next(&cl);
        if (cl.current.type == TOK_KEYWORD && cl.current.value.keyword == KW_DECLARE) {
            double ln = pgm->lines[idx].line_number;
            parser_execute_line(&cl, rt, ln);
        }
    }
    
    // Trigger loading for all registered external subs
    for (idx = 0; idx < rt->sub_count; idx++) {
        SubDef *sd = &rt->subs[idx];
        if (sd->is_external && sd->body_index == -1) {
            runtime_load_external_sub(rt, sd);
        }
    }
}

// --- User-Defined Type Runtime Functions (Milestone 17) ---

 // runtime_find_type - Look up a UserTypeDef by name.
UserTypeDef *runtime_find_type(RuntimeState *rt,
                               const char *name, int len)
{
 int i;
 for (i = 0; i < rt->type_count; i++) {
  if (str_eq_nocase(name, len, rt->user_types[i].name))
   return &rt->user_types[i];
 }
 return NULL;
}

 // runtime_find_typed_var - Look up a TypedVar by name.
TypedVar *runtime_find_typed_var(RuntimeState *rt,
                                 const char *name, int len)
{
 int i;
 for (i = 0; i < rt->typed_var_count; i++) {
  if (str_eq_nocase(name, len, rt->typed_vars[i].name))
   return &rt->typed_vars[i];
 }
 return NULL;
}

 // runtime_find_field - Look up field index in a type.
int runtime_find_field(UserTypeDef *td,
                       const char *name, int len)
{
 int i;
 if (td == NULL) return -1;
 for (i = 0; i < td->field_count; i++) {
  if (str_eq_nocase(name, len, td->fields[i].name))
   return i;
 }
 return -1;
}

 // runtime_create_typed_var - Allocate a new TypedVar.
 //
 // Creates a typed variable with all fields initialized to
 // zero (numeric) or empty string (string fields).
 // For nested type fields (nested_type_index >= 0),
 // recursively allocates child TypedVars and stores
 // the child index as bval_int in the parent field slot.
 // Nesting depth is bounded by pool exhaustion
 // (MAX_TYPED_VARS).
int runtime_create_typed_var(RuntimeState *rt,
                             const char *name, int len,
                             int type_index)
{
 int idx, i, copy_len;
 TypedVar *tv;
 UserTypeDef *td;

 if (rt->typed_var_count >= MAX_TYPED_VARS)
  return -1;
 if (type_index < 0 || type_index >= rt->type_count)
  return -1;

 idx = rt->typed_var_count;
 tv = &rt->typed_vars[idx];
 td = &rt->user_types[type_index];

 // Store name (uppercase)
 copy_len = len;
 if (copy_len > MAX_VAR_NAME_LEN)
  copy_len = MAX_VAR_NAME_LEN;
 memcpy(tv->name, name, (size_t)copy_len);
 tv->name[copy_len] = '\0';
 for (i = 0; i < copy_len; i++) {
  if (tv->name[i] >= 'a' && tv->name[i] <= 'z')
   tv->name[i] = (char)(tv->name[i] - 32);
 }

 tv->type_index = type_index;
 rt->typed_var_count++;

 // Initialize fields based on type definition
 for (i = 0; i < MAX_TYPE_FIELDS; i++) {
  if (i < td->field_count &&
      td->fields[i].nested_type_index >= 0) {
   // Nested type: allocate child TypedVar.
    // Child name = "parent.field" (internal).
    // Store child index in parent's field. 
   int child_idx;
   char cname[MAX_VAR_NAME_LEN + 1];
   int clen;
   // Build child name: truncated "PARENT.FIELD"
   clen = copy_len;
   if (clen + 1 + (int)strlen(
       td->fields[i].name) <= MAX_VAR_NAME_LEN) {
    memcpy(cname, tv->name, (size_t)copy_len);
    cname[copy_len] = '.';
    strcpy(cname + copy_len + 1,
     td->fields[i].name);
    clen = (int)strlen(cname);
   } else {
    // Name too long: use field name only
    strcpy(cname, td->fields[i].name);
    clen = (int)strlen(cname);
   }
   child_idx = runtime_create_typed_var(
    rt, cname, clen,
    td->fields[i].nested_type_index);
   if (child_idx < 0) {
    // Pool exhausted: store -1
    tv->fields[i] = bval_int(-1);
   } else {
    tv->fields[i] = bval_int(child_idx);
   }
  } else if (i < td->field_count &&
      td->fields[i].is_string) {
   tv->fields[i] = bval_string(NULL, 0);
  } else {
   tv->fields[i] = bval_int(0);
  }
 }

 return idx;
}

 // runtime_get_typed_array_field - Get field BValue from
 // a typed array element.
 //
 // Typed arrays store field_count BValues per element in
 // field-stride layout: element[i * field_count + field_index].
BValue *runtime_get_typed_array_field(RuntimeState *rt,
    DimArray *arr, int elem_index, int field_index)
{
 UserTypeDef *td;
 int offset;

 if (arr == NULL || arr->type_index < 0 ||
     arr->type_index >= rt->type_count)
  return NULL;

 td = &rt->user_types[arr->type_index];
 if (field_index < 0 || field_index >= td->field_count)
  return NULL;

 offset = elem_index * td->field_count + field_index;
 if (offset < 0 || offset >= arr->total)
  return NULL;

 return &arr->elements[offset];
}

 // runtime_copy_typed_var - Copy all fields from src to dst.
 //
 // Both must have the same type_index (strict).
 // String fields are re-stored in the string pool.
 // Nested type fields are recursively copied.
 // Returns 0 on success, -1 on error.
int runtime_copy_typed_var(RuntimeState *rt,
    TypedVar *dst, TypedVar *src)
{
 UserTypeDef *td;
 int i;

 if (dst == NULL || src == NULL)
  return -1;
 if (dst->type_index != src->type_index)
  return -1;
 if (dst->type_index < 0 ||
     dst->type_index >= rt->type_count)
  return -1;

 td = &rt->user_types[dst->type_index];

 for (i = 0; i < td->field_count; i++) {
  if (td->fields[i].nested_type_index >= 0) {
   // Nested: recursively copy child TypedVars
   int src_ci = (int)bval_to_int(&src->fields[i]);
   int dst_ci = (int)bval_to_int(&dst->fields[i]);
   if (src_ci >= 0 &&
       src_ci < rt->typed_var_count &&
       dst_ci >= 0 &&
       dst_ci < rt->typed_var_count) {
    runtime_copy_typed_var(rt,
     &rt->typed_vars[dst_ci],
     &rt->typed_vars[src_ci]);
   }
  } else if (td->fields[i].is_string) {
   // Re-pool string data
   BValue sv = src->fields[i];
   if (bval_is_string(&sv) &&
       sv.v.sval.data != NULL) {
    char *p = strpool_store(&rt->strpool,
     sv.v.sval.data, sv.v.sval.length);
    dst->fields[i] = bval_string(p,
     sv.v.sval.length);
   } else {
    dst->fields[i] = bval_string(NULL, 0);
   }
  } else {
   dst->fields[i] = src->fields[i];
  }
 }
 return 0;
}

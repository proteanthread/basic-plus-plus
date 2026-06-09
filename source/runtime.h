/*
 * =====================================================================
 * BASIC++ Interpreter - runtime.h
 * =====================================================================
 *
 * Runtime execution state and stack management interface.
 *
 * PURPOSE:
 * Manages the interpreter's execution state during program runs.
 * This includes:
 * - Current execution position (which line, which statement)
 * - Variable storage (A-Z and @() array)
 * - GOSUB return stack (and future FOR/loop frames)
 * - Run/stop state
 * - PRINT format state
 *
 * WHY THIS EXISTS:
 * The runtime state is separated from the parser and executor
 * so that:
 * 1. The interpreter state lives on the heap, not the C stack.
 * 2. Execution can be paused, inspected, or reset cleanly.
 * 3. The stack is bounded and checked (no C stack overflow).
 * 4. Future phases can add loop frames, debug state, etc.
 * without modifying the parser or executor.
 *
 * STACK FRAME DESIGN:
 * All flow-control constructs (GOSUB, FOR, WHILE, DO) share
 * a unified StackFrame type. The FrameType field distinguishes
 * them. This allows RETURN to check that it's popping a GOSUB
 * frame (not a FOR frame), and NEXT to check for its matching
 * FOR frame.
 *
 * HOW TO EXTEND:
 * To add a new loop type:
 * 1. Add a FrameType entry (e.g., FRAME_FOR, FRAME_WHILE).
 * 2. Add the loop-specific fields to the StackFrame union.
 * 3. Use runtime_push() and runtime_pop() for stack management.
 * 4. The stack bounds checking and type validation are automatic.
 *
 * =====================================================================
 */

#ifndef BASICPP_RUNTIME_H
#define BASICPP_RUNTIME_H

#include "config.h"
#include "memory.h"
#include "value.h"
#include "stringpool.h"
#include "vdev.h"

/* =====================================================================
 * Stack Frame Types
 * =====================================================================
 * Each type of flow-control construct that uses the runtime stack
 * gets its own FrameType. The unified frame system ensures that
 * RETURN checks for GOSUB frames, NEXT checks for FOR frames,
 * and WEND/LOOP check for their matching frames.
 */
typedef enum FrameType {
 FRAME_GOSUB = 0, /* GOSUB return address */
 FRAME_FOR, /* FOR/NEXT loop */
 FRAME_WHILE, /* WHILE/WEND loop */
 FRAME_DO, /* DO/LOOP loop */
 FRAME_SUB, /* SUB/FUNCTION call */
 FRAME_EXCEPTION /* WHEN EXCEPTION IN block */
} FrameType;

/* =====================================================================
 * Stack Frame
 * =====================================================================
 * A single entry on the runtime stack. The 'type' field identifies
 * what kind of frame this is. The 'data' union holds type-specific
 * data.
 *
 * FRAME_GOSUB:
 * return_index - ProgramStore index to return to.
 *
 * FRAME_FOR:
 * var_name - loop variable ('A'-'Z')
 * limit - TO value
 * step - STEP value (default 1)
 * body_index - ProgramStore index of loop body (line after FOR)
 *
 * FRAME_WHILE:
 * loop_index - ProgramStore index of the WHILE line itself
 * (WEND jumps back here to re-evaluate condition)
 *
 * FRAME_DO:
 * body_index - ProgramStore index of the line after DO
 * (LOOP jumps back here to re-enter loop body)
 * is_until - 1 if UNTIL condition, 0 if WHILE condition
 * is_pre - 1 if condition is on DO line (pre-check),
 * 0 if condition is on LOOP line (post-check)
 *
 * FRAME_EXCEPTION:
 * when_index - ProgramStore index of the WHEN EXCEPTION IN line
 * use_index - ProgramStore index of the USE line
 * end_when_index - ProgramStore index of the END WHEN line
 * err_index - ProgramStore index where error occurred
 * (set at error time, used by CONTINUE)
 */
typedef struct StackFrame {
 FrameType type;
 union {
 struct {
 int return_index;
 } gosub;
 struct {
 char var_name; /* loop variable 'A'-'Z' */
 long limit; /* TO value */
 long step; /* STEP value (default 1) */
 int body_index; /* index of first line of loop body */
 } for_loop;
 struct {
 int loop_index; /* index of the WHILE line */
 } while_loop;
 struct {
 int body_index; /* index of line after DO */
 int is_until; /* 1=UNTIL, 0=WHILE */
 int is_pre; /* 1=pre-check (DO WHILE), 0=post-check */
 } do_loop;
 struct {
 int return_index; /* program index to return to */
 int sub_index; /* index into subs[] table */
 BValue saved_vars[MAX_VARIABLES]; /* saved A-Z */
 BValue saved_strvars[MAX_STRING_VARS]; /* saved A$-Z$ */
 } sub_call;
 struct {
 int when_index; /* WHEN EXCEPTION IN line */
 int use_index; /* USE line */
 int end_when_index; /* END WHEN line */
 int err_index; /* line where error occurred */
 } exception;
 } data;
} StackFrame;

/* =====================================================================
 * Named Variable Entry
 * =====================================================================
 * Stores a single named variable (multi-character identifier).
 * Used when the active dialect supports extended variable names.
 */
typedef struct NamedVariable {
 char name[MAX_VAR_NAME_LEN + 1]; /* null-terminated name */
 BValue value;
} NamedVariable;

/* =====================================================================
 * DIM Array Entry
 * =====================================================================
 * Stores a single DIMmed array. Supports 1D and 2D arrays.
 * Elements are stored in row-major order in a flat BValue array.
 */
typedef struct DimArray {
 char name[MAX_VAR_NAME_LEN + 1]; /* array name */
 int dims; /* number of dimensions (1 or 2) */
 int size[MAX_ARRAY_DIMS]; /* size of each dimension */
 BValue *elements; /* pointer into element pool */
 int total; /* total number of elements */
} DimArray;

/* =====================================================================
 * User-Defined Type (TYPE...END TYPE)
 * =====================================================================
 * Each field has a name and a type flag (numeric or string).
 * Typed variables store field values as BValue arrays.
 */
typedef struct UserTypeField {
 char name[MAX_VAR_NAME_LEN + 1];
 int is_string; /* 0=numeric, 1=string */
} UserTypeField;

typedef struct UserTypeDef {
 char name[MAX_VAR_NAME_LEN + 1];
 UserTypeField fields[MAX_TYPE_FIELDS];
 int field_count;
} UserTypeDef;

typedef struct TypedVar {
 char name[MAX_VAR_NAME_LEN + 1];
 int type_index; /* index into user_types */
 BValue fields[MAX_TYPE_FIELDS]; /* field values */
} TypedVar;

/* =====================================================================
 * SUB/FUNCTION Definition (QBasic compatibility)
 * =====================================================================
 * Stores a subprogram or function definition.
 * name - subprogram/function name
 * body_index - program store index of first line of body
 * params - parameter names (multi-char identifiers)
 * param_is_string - 1 if the param ends with $ (string param)
 * param_count - number of parameters
 * is_function - 0=SUB, 1=FUNCTION
 *
 * SUB/FUNCTION bodies live inline in the program source.
 * The SUB/FUNCTION statement at definition time just skips
 * past END SUB/END FUNCTION. CALL (or expression evaluation
 * for FUNCTION) jumps to body_index.
 */
#define MAX_SUBS 64
#define MAX_SUB_PARAMS 8
typedef struct SubDef {
 char name[MAX_VAR_NAME_LEN + 1];
 int name_len;
 int body_index; /* index of first line after SUB/FUNCTION */
 char params[MAX_SUB_PARAMS][MAX_VAR_NAME_LEN + 1];
 int param_is_string[MAX_SUB_PARAMS];
 int param_count;
 int is_function; /* 0=SUB, 1=FUNCTION */
} SubDef;

/* =====================================================================
 * Runtime State
 * =====================================================================
 * Complete interpreter execution state. All mutable state during
 * program execution is contained here.
 *
 * Fields:
 * program - pointer to the program store (not owned)
 * memory - pointer to the memory system (not owned)
 * running - 1 if a program is executing, 0 if stopped
 * current_index - index of the currently executing line in
 * the ProgramStore array
 * next_index - next line index to execute (-1 = auto-advance)
 * Set by GOTO/GOSUB to override sequential flow.
 * variables - integer values for A-Z (index 0=A, 25=Z)
 * array_base - pointer to @() array in variable pool
 * array_size - number of available @() elements
/* =====================================================================
 * User-Defined Function Entry
 * =====================================================================
 * Stores a DEF FN definition. Each entry holds:
 * name - function name (single letter A-Z, or extended)
 * name_len - length of function name
 * params - parameter variable names (single letters)
 * param_count - number of parameters
 * body - expression body text (re-parsed at call time)
 * body_len - length of body expression text
 *
 * HOW DEF FN WORKS:
 * 1. DEF FNA(X) = X*X+1 -> stores name="A", params={'X'},
 * body="X*X+1"
 * 2. PRINT FNA(5) -> saves X, sets X=5, evaluates "X*X+1",
 * restores X, returns result (26)
 *
 * LOCAL SCOPING:
 * When FN is invoked, parameter variables are saved, set to
 * argument values, the body is evaluated, then the original
 * variable values are restored. This provides local scoping
 * without a separate scope stack - matching classic BASIC
 * behavior (Dartmouth, GW-BASIC, Applesoft).
 */
typedef struct UserFunction {
 char name[MAX_VAR_NAME_LEN + 1]; /* function name */
 int name_len;
 char params[MAX_FN_PARAMS]; /* param var names (A-Z) */
 int param_count;
 char body[MAX_FN_BODY + 1]; /* expression body text */
 int body_len;
} UserFunction;

/* =====================================================================
 * Runtime State (main interpreter state)
 * =====================================================================
 * Contains all mutable interpreter state:
 * program - pointer to the program store
 * memory - pointer to the memory system
 * running - 1 if program is executing (RUN), 0 otherwise
 * current_index - index of the currently executing line
 * next_index - index of the next line to execute
 * variables - 26 single-letter variables (A-Z)
 * array_base - pointer to @() legacy array base
 * array_size - number of elements in @() array
 * stack - GOSUB/loop stack frames
 * stack_top - current stack pointer (0 = empty)
 * print_width - current PRINT # format width
 * stopped - 1 if STOP or END was executed
 * named_vars - named variable table
 * named_count - number of named variables defined
 * data_pool - collected DATA values from program
 * data_count - total number of DATA items
 * data_ptr - current READ position in data_pool
 * user_funcs - DEF FN function table
 * user_func_count - number of defined user functions
 */
typedef struct RuntimeState {
 ProgramStore *program;
 MemorySystem *memory;
 int running;
 int current_index;
 int next_index;
 BValue variables[MAX_VARIABLES]; /* A-Z */
 long *array_base; /* @() legacy array */
 long array_size;
 StackFrame stack[MAX_STACK_DEPTH];
 int stack_top;
 int print_width;
 int option_base; /* OPTION BASE 0 or 1 */
 int print_col; /* current column in PRINT */
 int stopped;
 unsigned long rnd_seed;
 /* named variables */
 NamedVariable named_vars[MAX_NAMED_VARS];
 int named_count;
 /* DATA pool (now BValue for mixed types) */
 BValue data_pool[MAX_DATA_ITEMS];
 int data_count;
 int data_ptr;
 /* string variables A$-Z$ */
 BValue string_vars[MAX_STRING_VARS];
 /* DIM arrays */
 DimArray dim_arrays[MAX_DIM_ARRAYS];
 int dim_count;
 BValue dim_elements[MAX_ARRAY_ELEMENTS]; /* flat pool */
 int dim_elements_used;
 /* string pool */
 StringPool strpool;
 /* virtual devices */
 VDev *dev_con; /* console device (CON:) */
 VDev *dev_err; /* error device (ERR:) */
 /* trace system (TRON/TROFF) */
 int trace_on; /* 1 = trace active */
 /* error handler (ON ERROR GOTO) */
 int on_error_line; /* target line, 0 = disabled */
 int on_timer_line; /* ON TIMER target, 0 = off */
 int last_err_code; /* ERR - last error code */
 int last_err_line; /* ERL - line where error occurred */
 /* user-defined functions (DEF FN) */
 UserFunction user_funcs[MAX_USER_FUNCS];
 int user_func_count;
 /* VM state machine */
 int vm_state; /* VMState enum value */
 /* expression evaluation stack (vm.h) */
 BValue eval_items[VM_EVAL_STACK_SIZE];
 int eval_top; /* -1 = empty */
 /* Interactive debugger */
 int breakpoints[MAX_BREAKPOINTS];
 int breakpoint_count;
 int single_step; /* 1 = pause after each line */
 int resume_index; /* program index to resume from */
 /* Self-test framework */
 int test_pass;
 int test_fail;
 int test_total;
 char test_name[64]; /* current TEST block name */
 int in_test; /* 1 = inside TEST/ENDTEST block */
 /* AUTO mode */
 int auto_line; /* next AUTO line number, 0=off */
 int auto_step; /* AUTO increment (default 10) */
 /* Block IF depth (ECMA-116 / QBasic) */
 int block_if_depth; /* nesting depth of active true blocks */
 /* OPTION ANGLE (ECMA-116) */
 int angle_degrees; /* 0=radians (default), 1=degrees */

 /* SCREEN / DRAW state */
 int screen_mode; /* 0=text (default) */
 int draw_x; /* DRAW cursor X (0-79) */
 int draw_y; /* DRAW cursor Y (0-49) */
 int draw_color; /* DRAW pen character */

 /* Cursor tracking for CSRLIN / POS(0) */
 int cursor_row; /* 1-based row (CSRLIN) */
 int cursor_col; /* 1-based column (POS) */

 /* WIDTH state */
 int screen_width; /* columns per line (40 or 80) */
 int screen_lines; /* lines per screen (25 default) */

 /* CONST table */
#define MAX_CONSTANTS 64
 struct {
 char name[32];
 int name_len;
 BValue value;
 } constants[MAX_CONSTANTS];
 int const_count;

 /* SUB/FUNCTION table */
 SubDef subs[MAX_SUBS];
 int sub_count;
 BValue fn_return_value; /* FUNCTION return value */
 int in_sub_index; /* index of currently executing SUB, -1 = none */

 /* Line label table */
#define MAX_LABELS 128
 struct {
 char name[MAX_VAR_NAME_LEN + 1];
 int program_index;
 } labels[MAX_LABELS];
 int label_count;

 /* Virtual memory (DEF SEG / PEEK / POKE) */
 unsigned char mem_segment[MAX_MEM_SEGMENT];
 int mem_seg_base; /* current DEF SEG offset */
 int memmap_type; /* active memory map (MemMapType) */
 int last_shell_exitcode; /* ERRORLEVEL */

 /* User-defined types (TYPE...END TYPE) */
 UserTypeDef user_types[MAX_USER_TYPES];
 int type_count;
 /* Typed variable instances */
 TypedVar typed_vars[MAX_TYPED_VARS];
 int typed_var_count;

 /* Function key macros (KEY statement)
 * Slots 1-12: F1-F12
 * Slots 13-24: SHIFT+F1-F12
 * Slots 25-36: CTRL+F1-F12
 * Slots 37-48: ALT+F1-F12 (also SUPER+F1-F12)
 */
#define FKEY_MAX_SLOTS 48
#define FKEY_MAX_LEN 15
 char fkey_macros[FKEY_MAX_SLOTS + 1]
 [FKEY_MAX_LEN + 1];
 int fkey_display; /* KEY ON=1, OFF=0 */
} RuntimeState;

/* =====================================================================
 * Runtime Functions
 * =====================================================================
 */

/*
 * runtime_init - Initialize runtime state.
 *
 * Sets up variables (all zero), clears the stack, configures the
 * @() array from the variable pool, and sets defaults.
 *
 * Parameters:
 * rt - runtime state to initialize
 * program - pointer to the program store
 * memory - pointer to the memory system
 */
void runtime_init(RuntimeState *rt, ProgramStore *program,
 MemorySystem *memory);

/*
 * runtime_reset - Reset execution state for a new RUN.
 *
 * Clears all variables to zero, resets the stack, resets the
 * @() array, and sets current_index to 0 (first line).
 * Does NOT clear the program store.
 */
void runtime_reset(RuntimeState *rt);

/*
 * runtime_push - Push a stack frame.
 *
 * Returns 0 on success, -1 if the stack is full (ERR_SORRY).
 */
int runtime_push(RuntimeState *rt, const StackFrame *frame);

/*
 * runtime_pop - Pop a stack frame with type checking.
 *
 * Pops the top frame and verifies its type matches 'expected'.
 * Returns 0 on success, -1 on error (stack empty or type mismatch;
 * raises ERR_HOW).
 *
 * The popped frame is stored in 'out' if non-NULL.
 */
int runtime_pop(RuntimeState *rt, FrameType expected, StackFrame *out);

/*
 * runtime_get_var - Get the value of variable A-Z.
 */
long runtime_get_var(RuntimeState *rt, char name);

/*
 * runtime_get_var_bval - Get variable as BValue.
 */
BValue runtime_get_var_bval(RuntimeState *rt, char name);

/*
 * runtime_set_var - Set the value of variable A-Z (integer).
 */
void runtime_set_var(RuntimeState *rt, char name, long value);

/*
 * runtime_set_var_bval - Set variable from BValue.
 */
void runtime_set_var_bval(RuntimeState *rt, char name, BValue value);

/*
 * runtime_get_array - Get an element of the @() array.
 */
long runtime_get_array(RuntimeState *rt, long index);

/*
 * runtime_set_array - Set an element of the @() array.
 */
void runtime_set_array(RuntimeState *rt, long index, long value);

/*
 * runtime_rnd - Generate a pseudo-random number.
 *
 * Returns a random integer between 1 and max (inclusive).
 * Uses a simple linear congruential generator (LCG) that is
 * portable and does not depend on <time.h> or platform APIs.
 *
 * The LCG parameters are chosen for reasonable distribution
 * on both 16-bit and 32-bit targets.
 */
long runtime_rnd(RuntimeState *rt, long max);

/*
 * runtime_size - Return available memory (SIZE function).
 *
 * Reports the available space in the variable pool, which
 * determines how large the @() array can be.
 */
long runtime_size(RuntimeState *rt);

/*
 * Interactive debugger functions.
 */
int runtime_breakpoint_add(RuntimeState *rt, int line_num);
int runtime_breakpoint_remove(RuntimeState *rt, int line_num);
void runtime_breakpoint_clear(RuntimeState *rt);
int runtime_is_breakpoint(RuntimeState *rt, int line_num);
void runtime_breakpoint_list(RuntimeState *rt);

/*
 * runtime_find_matching - Scan forward to find a matching keyword.
 *
 * Starting from 'start_index', scans program lines forward looking
 * for the keyword 'close_kw' that matches the given 'open_kw' at
 * nesting depth 0. Handles nested structures correctly.
 *
 * Used by:
 * - WHILE (when false) to skip to matching WEND
 * - DO (when false) to skip to matching LOOP
 *
 * Returns the ProgramStore index of the matching line, or -1 if
 * not found (raises ERR_HOW).
 *
 * Parameters:
 * rt - runtime state (provides access to program store)
 * start_index - index to start scanning from (exclusive)
 * open_kw - the opening keyword (KW_WHILE or KW_DO)
 * close_kw - the closing keyword (KW_WEND or KW_LOOP)
 * line_num - current line number for error context
 */
int runtime_find_matching(RuntimeState *rt, int start_index,
 int open_kw, int close_kw, int line_num);

/*
 * runtime_get_named_var - Get a named variable's value (integer).
 */
long runtime_get_named_var(RuntimeState *rt, const char *name, int len);

/*
 * runtime_get_named_var_bval - Get named variable as BValue.
 */
BValue runtime_get_named_var_bval(RuntimeState *rt, const char *name,
 int len);

/*
 * runtime_set_named_var - Set a named variable (integer).
 */
int runtime_set_named_var(RuntimeState *rt, const char *name, int len,
 long value);

/*
 * runtime_set_named_var_bval - Set named variable from BValue.
 */
int runtime_set_named_var_bval(RuntimeState *rt, const char *name,
 int len, BValue value);

/* =====================================================================
 * String Variables
 * =====================================================================
 */

BValue runtime_get_string_var(RuntimeState *rt, char name);
void runtime_set_string_var(RuntimeState *rt, char name, BValue value);

/* =====================================================================
 * DIM Arrays
 * =====================================================================
 */

/*
 * runtime_dim - Create a DIM array.
 * For 2D: pass both dim1 and dim2.
 * For 1D: pass dim2 = 0.
 */
int runtime_dim(RuntimeState *rt, const char *name, int name_len,
 int dim1, int dim2, int line_num);

/*
 * runtime_get_dim - Get element from DIMmed array.
 */
BValue runtime_get_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, int line_num);

/*
 * runtime_set_dim - Set element in DIMmed array.
 */
void runtime_set_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, BValue val, int line_num);

/*
 * runtime_find_dim - Find a DIM array by name.
 * Returns pointer or NULL.
 */
DimArray *runtime_find_dim(RuntimeState *rt, const char *name,
 int name_len);

/* =====================================================================
 * DATA Pool (, extended in for BValue)
 * =====================================================================
 */

void runtime_collect_data(RuntimeState *rt);
BValue runtime_read_data_bval(RuntimeState *rt, int line_num);
long runtime_read_data(RuntimeState *rt, int line_num);
void runtime_restore_data(RuntimeState *rt);

/* =====================================================================
 * User-Defined Functions (DEF FN)
 * =====================================================================
 *
 * DEF FN stores a named function with parameters and a body
 * expression. When invoked via FN, the parameters are bound
 * to argument values (with save/restore of globals), and the
 * body expression is re-parsed and evaluated.
 *
 * This matches classic BASIC behavior:
 * 10 DEF FNA(X) = X*X+1
 * 20 PRINT FNA(5) -> prints 26
 *
 * Multi-parameter (GW-BASIC style):
 * 10 DEF FNA(X,Y) = X*Y+1
 * 20 PRINT FNA(3,4) -> prints 13
 */

/*
 * runtime_def_fn - Define a user function.
 *
 * Stores or replaces a DEF FN definition. Parameters:
 * name - function name (single letter or extended)
 * name_len - length of function name
 * params - array of parameter variable names (A-Z)
 * param_count - number of parameters
 * body - expression body text
 * body_len - length of body text
 *
 * Returns 0 on success, -1 if the function table is full.
 */
int runtime_def_fn(RuntimeState *rt, const char *name, int name_len,
 const char *params, int param_count,
 const char *body, int body_len);

/*
 * runtime_find_fn - Look up a user function by name.
 *
 * Returns pointer to UserFunction, or NULL if not found.
 * Name comparison is case-insensitive.
 */
UserFunction *runtime_find_fn(RuntimeState *rt, const char *name,
 int name_len);

/* =====================================================================
 * Label Collection and Lookup
 * =====================================================================
 */

/*
 * runtime_collect_labels - Scan program for line labels.
 *
 * A line label is an identifier followed by a colon at the start
 * of a line (after the optional line number). E.g.:
 * 100 Main:
 * 200 Done: PRINT "Bye"
 *
 * Called at RUN time alongside runtime_collect_data().
 */
void runtime_collect_labels(RuntimeState *rt);

/*
 * runtime_find_label - Look up a label by name.
 *
 * Returns the program store index for the label, or -1 if
 * not found. Case-insensitive comparison.
 */
int runtime_find_label(RuntimeState *rt, const char *name, int len);

/* =====================================================================
 * SUB/FUNCTION Lookup
 * =====================================================================
 */

/*
 * runtime_find_sub - Look up a SUB or FUNCTION by name.
 *
 * Returns pointer to SubDef, or NULL if not found.
 * Case-insensitive comparison.
 */
SubDef *runtime_find_sub(RuntimeState *rt, const char *name, int len);

#endif /* BASICPP_RUNTIME_H */

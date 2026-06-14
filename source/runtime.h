/*
 * runtime.h -- interpreter execution state
 *
 * All mutable state during a program run lives in RuntimeState:
 * variables, stack frames, DATA pool, user functions, arrays,
 * string pool, virtual devices, debugger state, etc.
 *
 * The runtime is separated from the parser so execution can be
 * paused, inspected, or reset without touching parse state.
 * The stack is bounded (MAX_STACK_DEPTH) and type-checked --
 * RETURN verifies it's popping a GOSUB, not a FOR.
 *
 * Memory pools:
 *   Pool sizes come from config.h (MAX_VARIABLES, MAX_DIM_ARRAYS,
 *   MAX_ARRAY_ELEMENTS, etc). The string pool (strpool) grows
 *   dynamically. DIM array elements are allocated from a flat
 *   pool (dim_elements[]). If a dialect needs more room, bump
 *   the config.h limits and rebuild -- everything adjusts.
 *
 *   For dialects that emulate hardware memory maps (C64, Atari),
 *   mem_segment[] provides a 64KB virtual address space that
 *   PEEK/POKE can read and write. See memmap.h for how different
 *   machines overlay their I/O registers onto this space.
 *
 * Self-modifying code:
 *   Since programs are stored as text lines in ProgramStore,
 *   a running program can rewrite itself by manipulating the
 *   program store directly. Build a line as a string, feed it
 *   to the line editor (the same path that processes typed-in
 *   numbered lines), and it replaces the existing line. The
 *   next time execution reaches that line number, the new code
 *   runs. Watch out for infinite self-modification loops.
 *
 * Extending the stack:
 *   To add a new loop type, add a FrameType enum value and a
 *   new union member in StackFrame. Use runtime_push/pop as
 *   usual. The type tag ensures mismatched push/pop is caught.
 */

#ifndef BASICPP_RUNTIME_H
#define BASICPP_RUNTIME_H

#include "config.h"
#include "memory.h"
#include "value.h"
#include "stringpool.h"
#include "vdev.h"

/* --- Stack Frame Types ---
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
 FRAME_EXCEPTION, /* WHEN EXCEPTION IN block */
 FRAME_REPEAT /* REPeat/END REPeat loop */
} FrameType;

/* --- Stack Frame ---
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
 struct {
 char name[MAX_VAR_NAME_LEN + 1]; /* loop identifier */
 int body_index; /* index of line after REPeat */
 } repeat_loop;
 } data;
} StackFrame;

/* --- Named Variable Entry ---
 * Stores a single named variable (multi-character identifier).
 * Used when the active dialect supports extended variable names.
 */
typedef struct NamedVariable {
 char name[MAX_VAR_NAME_LEN + 1]; /* null-terminated name */
 BValue value;
} NamedVariable;

/* --- DIM Array Entry ---
 * Stores a single DIMmed array. Supports 1D, 2D, and 3D arrays.
 * Elements are stored in row-major order in a flat BValue array.
 */
typedef struct DimArray {
 char name[MAX_VAR_NAME_LEN + 1]; /* array name */
 int dims; /* number of dimensions (1, 2, or 3) */
 int size[MAX_ARRAY_DIMS]; /* size of each dimension */
 BValue *elements; /* pointer into element pool */
 int total; /* total number of elements */
} DimArray;

/* --- User-Defined Type (TYPE...END TYPE) ---
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

/* --- SUB/FUNCTION Definition (QBasic compatibility) ---
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

/* --- Runtime State ---
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
 */
/* --- User-Defined Function Entry ---
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

/* --- Runtime State (main interpreter state) ---
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
 int debug_on; /* 1 = verbose trace (DEBUG) */
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
 int assert_pass_total; /* cumulative pass across blocks */
 int assert_fail_total; /* cumulative fail across blocks */
 int test_block_count; /* number of TEST blocks run */
 /* AUTO mode */
 int auto_line; /* next AUTO line number, 0=off */
 int auto_step; /* AUTO increment (default 10) */
 /* Block IF depth (ECMA-116 / QBasic) */
 int block_if_depth; /* nesting depth of active true blocks */
 /* OPTION ANGLE (ECMA-116) */
 int angle_degrees; /* 0=radians (default), 1=degrees */
 /* OPTION TAB: 0=spaces (default), 1=real HT chars */
 int tab_mode;
 /* OPTION ZONE: -1=use dialect default, >0=override */
 int zone_override;

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

 /* ALARM$ time string */
 char alarm_str[16]; /* "HH:MM:SS" or empty */

 /* SCOPE hook state */
 int scope_before_done; /* index of line whose BEFORE hook already fired, -1=none */
 int scope_after_kw;    /* KeywordId of pending AFTER hook, -1=none */
 int scope_hook_depth;  /* re-entrancy guard: >0 means inside a hook */

 /* SCOPE event queue (Q10: hooks queue between statements) */
#define SCOPE_EVQ_SIZE 16
 struct {
  int hook_type;  /* 0=before, 1=after, 2=override */
  int hook_line;  /* GOSUB target line */
  int return_idx; /* program index to return to */
  int keyword_id; /* KeywordId that triggered */
 } scope_evq[SCOPE_EVQ_SIZE];
 int scope_evq_head;  /* next slot to read */
 int scope_evq_tail;  /* next slot to write */
 int scope_evq_count; /* items in queue */

 /* WINDOW logical coordinate system */
 int win_active;         /* 0=physical, 1=logical */
 int win_screen_flag;    /* WINDOW SCREEN inverts Y */
 double win_x1, win_y1;  /* logical min */
 double win_x2, win_y2;  /* logical max */

 /* VIEW PRINT text scroll region */
 int view_print_top;    /* 1-based top row (default 1) */
 int view_print_bottom; /* 1-based bottom row (default 25) */

 /* Virtual console screen buffer (80x25 character cells)
  * Written by PRINT, read by SCREEN(row, col) function.
  * Provides real screen buffer for retro BASIC programs. */
#define VCON_COLS 80
#define VCON_ROWS 25
 unsigned char vcon_chars[VCON_ROWS][VCON_COLS];
 unsigned char vcon_colors[VCON_ROWS][VCON_COLS];

  /* Event trap handlers (ON xxx GOSUB targets) */
#define MAX_COM_PORTS 4
#define MAX_STRIG_BUTTONS 4
#define MAX_KEY_TRAPS 20
 int on_com_line[MAX_COM_PORTS];
 int on_key_line[MAX_KEY_TRAPS];
 int on_pen_line;
 int on_play_line;
 int on_strig_line[MAX_STRIG_BUTTONS];

 /* Event enable state: 0=OFF, 1=ON, 2=STOP */
#define EVT_OFF  0
#define EVT_ON   1
#define EVT_STOP 2
 int timer_event_state;    /* ON TIMER: ON/OFF/STOP */
 double timer_interval;    /* seconds between fires */
 double timer_last_fire;   /* timestamp of last fire */
 int key_event_state[MAX_KEY_TRAPS]; /* per-key ON/OFF/STOP */
 int com_event_state[MAX_COM_PORTS];
 int pen_event_state;
 int play_event_state;     /* ON PLAY: ON/OFF/STOP */
 int strig_event_state[MAX_STRIG_BUTTONS];

 /* Tier 2: Device I/O interrupt handlers */
#define MAX_DEVICE_TRAPS 8
 int on_device_line[MAX_DEVICE_TRAPS];  /* GOSUB targets */
 int device_event_state[MAX_DEVICE_TRAPS]; /* ON/OFF/STOP */

 /* Tier 3: OS / system interrupt handlers */
 int on_break_line;        /* ON BREAK GOSUB target */
 int break_event_state;    /* ON/OFF/STOP */
 int signal_pending;       /* set by OS signal handler */

 /* Tier 4: File I/O event handlers */
 int on_fileio_line;       /* ON FILEIO GOSUB target */
 int fileio_event_state;   /* ON/OFF/STOP */
 int fileio_pending;       /* set when disk event occurs */

 /* Event system infrastructure */
 int event_in_handler;     /* re-entrancy guard */

 /* Event queue for STOP-mode deferred events */
#define EVENT_QUEUE_SIZE 16
#define EVTYPE_TIMER   1
#define EVTYPE_KEY     2
#define EVTYPE_COM     3
#define EVTYPE_PEN     4
#define EVTYPE_PLAY    5
#define EVTYPE_STRIG   6
#define EVTYPE_DEVICE  7
#define EVTYPE_BREAK   8
#define EVTYPE_FILEIO  9
 struct {
  int event_type;       /* EVTYPE_xxx */
  int event_id;         /* device index or key number */
  int handler_line;     /* target GOSUB line */
 } event_queue[EVENT_QUEUE_SIZE];
 int evq_head;
 int evq_tail;
 int evq_count;

 /* DEF USR addresses (USR0 through USR9) */
#define MAX_USR_FUNCS 10
 long usr_addresses[MAX_USR_FUNCS];

 /* Default variable type per letter range (DEFINT/DEFDBL/etc) */
#define DEFTYPE_NONE 0
#define DEFTYPE_INT  1
#define DEFTYPE_SNG  2
#define DEFTYPE_DBL  3
#define DEFTYPE_STR  4
 unsigned char deftype_map[26]; /* A-Z */
} RuntimeState;

/* --- Runtime Functions ---
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

/* --- String Variables ---
 */

BValue runtime_get_string_var(RuntimeState *rt, char name);
void runtime_set_string_var(RuntimeState *rt, char name, BValue value);

/* --- DIM Arrays ---
 */

/*
 * runtime_dim - Create a DIM array.
 * For 3D: pass dim1, dim2, and dim3.
 * For 2D: pass dim1 and dim2, dim3 = 0.
 * For 1D: pass dim2 = 0, dim3 = 0.
 */
int runtime_dim(RuntimeState *rt, const char *name, int name_len,
 int dim1, int dim2, int dim3, int line_num);

/*
 * runtime_get_dim - Get element from DIMmed array.
 */
BValue runtime_get_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, int idx3, int line_num);

/*
 * runtime_set_dim - Set element in DIMmed array.
 */
void runtime_set_dim(RuntimeState *rt, const char *name, int name_len,
 int idx1, int idx2, int idx3, BValue val, int line_num);

/*
 * runtime_find_dim - Find a DIM array by name.
 * Returns pointer or NULL.
 */
DimArray *runtime_find_dim(RuntimeState *rt, const char *name,
 int name_len);

/* --- DATA Pool (, extended in for BValue) ---
 */

void runtime_collect_data(RuntimeState *rt);
BValue runtime_read_data_bval(RuntimeState *rt, int line_num);
long runtime_read_data(RuntimeState *rt, int line_num);
void runtime_restore_data(RuntimeState *rt);

/* --- User-Defined Functions (DEF FN) ---
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

/* --- Label Collection and Lookup ---
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

/* --- SUB/FUNCTION Lookup ---
 */

/*
 * runtime_find_sub - Look up a SUB or FUNCTION by name.
 *
 * Returns pointer to SubDef, or NULL if not found.
 * Case-insensitive comparison.
 */
SubDef *runtime_find_sub(RuntimeState *rt, const char *name, int len);

#endif /* BASICPP_RUNTIME_H */

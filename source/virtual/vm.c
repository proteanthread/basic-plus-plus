/*
 * ---
 * BASIC++ Interpreter - vm.c
 * ---
 *
 * Virtual Machine formalization layer.
 *
 * IMPLEMENTATION:
 * - Static dispatch table maps KeywordId -> VMOpcode
 * - Opcode name table for trace/debug output
 * - State machine get/set with VMState enum
 * - Expression evaluation stack (bounded, checked)
 * - Control flow primitives (jump, call, return, halt, stop)
 *
 * The dispatch table is built once at boot by vm_init() and
 * remains constant during execution. Handler registration
 * is through the static table below.
 *
 * ---
 */

#include <string.h>
#include "vm.h"
#include "runtime.h"
#include "errors.h"
#include "dialect.h"

/* --- Opcode Name Table ---
 * Human-readable names for trace output and debug logging.
 * Index matches VMOpcode enum values.
 */
static const char *opcode_names[OP_COUNT] = {
 "NOP",
 "PRINT", "LET", "INPUT",
 "GOTO", "GOSUB", "RETURN", "IF",
 "FOR", "NEXT", "WHILE", "WEND",
 "DO", "LOOP", "END", "STOP",
 "REM", "DATA", "READ", "RESTORE",
 "DIM", "DEF", "ON",
 "TRON", "TROFF", "CLS",
 "LIST", "RUN", "NEW",
 "SAVE", "LOAD", "COMPILE",
 "MAT", "OPEN", "CLOSE",
 "CHAIN", "MERGE", "DIALECT",
 "BSAVE", "BLOAD",
 "BRUN",
 "MODULE",
 "SECURITY",
 "SYSTEM",
 "BREAK", "CONT",
 "VARS",
 "ASSERT", "TEST",
 "ENDTEST", "SELFTEST",
 "HELP", "INFO",
 "CATALOG",
 "RENUM", "DELETE",
 "VER",
 "BYE",
 "VDEV", "VMEM", "VNET",
 "VCON", "VTERM", "VMACH",
 "DEVMAP",
 "ASSIGN",
 "UNKNOWN"
};

/* --- Keyword -> Opcode Mapping Table ---
 * Maps each KeywordId to the corresponding VMOpcode. Entries with
 * OP_UNKNOWN indicate keywords that are not statement-level
 * (they are sub-keywords like AS, ERROR, THEN, STEP, TO).
 */
typedef struct KeywordOpcodeMap {
 KeywordId keyword;
 VMOpcode opcode;
} KeywordOpcodeMap;

static const KeywordOpcodeMap kw_opcode_map[] = {
 { KW_PRINT, OP_PRINT },
 { KW_LET, OP_LET },
 { KW_INPUT, OP_INPUT },
 { KW_GOTO, OP_GOTO },
 { KW_GOSUB, OP_GOSUB },
 { KW_RETURN, OP_RETURN },
 { KW_IF, OP_IF },
 { KW_FOR, OP_FOR },
 { KW_NEXT, OP_NEXT },
 { KW_WHILE, OP_WHILE },
 { KW_WEND, OP_WEND },
 { KW_DO, OP_DO },
 { KW_LOOP, OP_LOOP },
 { KW_END, OP_END },
 { KW_STOP, OP_STOP },
 { KW_REM, OP_REM },
 { KW_DATA, OP_DATA },
 { KW_READ, OP_READ },
 { KW_RESTORE, OP_RESTORE },
 { KW_DIM, OP_DIM },
 { KW_DEF, OP_DEF },
 { KW_ON, OP_ON },
 { KW_TRON, OP_TRON },
 { KW_TROFF, OP_TROFF },
 { KW_CLS, OP_CLS },
 { KW_LIST, OP_LIST },
 { KW_RUN, OP_RUN },
 { KW_NEW, OP_NEW },
 { KW_SAVE, OP_SAVE },
 { KW_LOAD, OP_LOAD },
 { KW_COMPILE, OP_COMPILE },
 { KW_MAT, OP_MAT },
 { KW_OPEN, OP_OPEN },
 { KW_CLOSE, OP_CLOSE },
 { KW_CHAIN, OP_CHAIN },
 { KW_MERGE, OP_MERGE },
 { KW_DIALECT, OP_DIALECT },
 
 { KW_BSAVE, OP_BSAVE },
 { KW_BLOAD, OP_BLOAD },
 
 { KW_BRUN, OP_BRUN },
 
 { KW_MODULE, OP_MODULE },
 
 { KW_SECURITY, OP_SECURITY },
 
 { KW_SYSTEM, OP_SYSTEM },
 
 { KW_BREAK, OP_BREAK },
 { KW_CONT, OP_CONT },
 { KW_VARS, OP_VARS },
 
 { KW_ASSERT, OP_ASSERT },
 { KW_TEST, OP_TEST },
 { KW_ENDTEST, OP_ENDTEST },
 { KW_SELFTEST, OP_SELFTEST },
 
 { KW_HELP, OP_HELP },
 { KW_INFO, OP_INFO },
 { KW_CATALOG, OP_CATALOG },
 
 { KW_RENUM, OP_RENUM },
 { KW_DELETE, OP_DELETE },
 { KW_VER, OP_VER },
 { KW_BYE, OP_BYE },
 /* Virtual subsystem introspection */
 { KW_VDEV, OP_VDEV },
 { KW_VMEM, OP_VMEM },
 { KW_VNET, OP_VNET },
 { KW_VCON, OP_VCON },
 { KW_VTERM, OP_VTERM },
 { KW_VMACH, OP_VMACH },
 { KW_DEVMAP, OP_DEVMAP }
};

#define KW_OPCODE_MAP_SIZE \
 (int)(sizeof(kw_opcode_map) / sizeof(kw_opcode_map[0]))

/* --- Fast Lookup Table: KeywordId -> VMOpcode ---
 * Built by vm_init() for O(1) keyword->opcode resolution.
 */
static VMOpcode kw_to_opcode[KW_COUNT];
static int vm_initialized = 0;

/* --- vm_init - Build the dispatch lookup table. ---
 */
void vm_init(void)
{
 int i;

 /* Initialize all keywords to OP_UNKNOWN */
 for (i = 0; i < KW_COUNT; i++) {
 kw_to_opcode[i] = OP_UNKNOWN;
 }

 /* Populate from the mapping table */
 for (i = 0; i < KW_OPCODE_MAP_SIZE; i++) {
 kw_to_opcode[kw_opcode_map[i].keyword] =
 kw_opcode_map[i].opcode;
 }

 vm_initialized = 1;
}

/* --- vm_resolve_opcode - Map KeywordId -> VMOpcode. ---
 */
VMOpcode vm_resolve_opcode(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) {
 return OP_UNKNOWN;
 }
 return kw_to_opcode[kw];
}

/* --- vm_get_handler - Get handler for an opcode. ---
 * infrastructure: returns NULL. Handlers are currently
 * dispatched through parser.c's switch. Future phases will register
 * handlers here for full table-driven dispatch.
 */
VMHandler vm_get_handler(VMOpcode op)
{
 (void)op;
 return NULL; /* handlers live in parser.c switch for now */
}

/* --- vm_opcode_name - Human-readable opcode name. ---
 */
const char *vm_opcode_name(VMOpcode op)
{
 if (op < 0 || op >= OP_COUNT) {
 return "UNKNOWN";
 }
 return opcode_names[op];
}

/* --- vm_dispatch - Resolve keyword and return opcode. ---
 * resolves the opcode for logging/tracing. Actual handler
 * dispatch is still done by parser.c's switch statement.
 */
VMOpcode vm_dispatch(KeywordId kw, Lexer *lex, void *rt, int line_num)
{
 VMOpcode op = vm_resolve_opcode(kw);
 VMHandler handler = vm_get_handler(op);

 if (handler != NULL) {
 handler(lex, rt, line_num);
 }
 /* If no handler registered, caller falls through to parser */
 return op;
}

/* --- State Machine ---
 */

void vm_set_state(void *rt_ptr, VMState state)
{
 RuntimeState *rt = (RuntimeState *)rt_ptr;
 rt->vm_state = state;

 /* Keep legacy flags in sync for backward compatibility */
 switch (state) {
 case VM_RUNNING:
 rt->running = 1;
 rt->stopped = 0;
 break;
 case VM_PAUSED:
 rt->running = 0;
 rt->stopped = 1;
 dialect_clear_mixed();
 break;
 case VM_HALTED:
 rt->running = 0;
 rt->stopped = 1;
 dialect_clear_mixed();
 break;
 case VM_ERROR:
 rt->running = 0;
 rt->stopped = 0;
 dialect_clear_mixed();
 break;
 case VM_STOPPED:
 default:
 rt->running = 0;
 rt->stopped = 0;
 dialect_clear_mixed();
 break;
 }
}

VMState vm_get_state(void *rt_ptr)
{
 RuntimeState *rt = (RuntimeState *)rt_ptr;
 return rt->vm_state;
}

/* --- Expression Evaluation Stack ---
 */

void vm_eval_init(VMEvalStack *stk)
{
 stk->top = -1;
}

int vm_eval_push(VMEvalStack *stk, BValue val)
{
 if (stk->top >= VM_EVAL_STACK_SIZE - 1) {
 error_raise(ERR_SORRY, 0);
 return -1;
 }
 stk->items[++stk->top] = val;
 return 0;
}

BValue vm_eval_pop(VMEvalStack *stk)
{
 if (stk->top < 0) {
 error_raise(ERR_HOW, 0);
 return bval_int(0);
 }
 return stk->items[stk->top--];
}

BValue vm_eval_peek(VMEvalStack *stk)
{
 if (stk->top < 0) {
 error_raise(ERR_HOW, 0);
 return bval_int(0);
 }
 return stk->items[stk->top];
}

int vm_eval_depth(VMEvalStack *stk)
{
 return stk->top + 1;
}

/* --- Control Flow Primitives ---
 * These centralize the line-number->index resolution and stack
 * management that was previously scattered across parser.c.
 */

/*
 * vm_jump - Unconditional jump (GOTO).
 *
 * Resolves the BASIC line number to a program store index and
 * sets rt->next_index. Raises ERR_HOW if line not found.
 */
void vm_jump(void *rt_ptr, int target_line, int line_num)
{
 RuntimeState *rt = (RuntimeState *)rt_ptr;
 int i;

 for (i = 0; i < rt->program->count; i++) {
 if (rt->program->lines[i].line_number == target_line) {
 rt->next_index = i;
 return;
 }
 }

 /* Line not found */
 error_raise(ERR_HOW, line_num);
}

/*
 * vm_call - Subroutine call (GOSUB).
 *
 * Pushes a FRAME_GOSUB onto the stack with the return address
 * (current_index + 1), then jumps to the target line.
 */
void vm_call(void *rt_ptr, int target_line, int line_num)
{
 RuntimeState *rt = (RuntimeState *)rt_ptr;
 StackFrame frame;

 frame.type = FRAME_GOSUB;
 frame.data.gosub.return_index = rt->current_index + 1;

 if (runtime_push(rt, &frame) != 0) {
 return; /* stack full - runtime_push raised ERR_SORRY */
 }

 vm_jump(rt_ptr, target_line, line_num);
}

/*
 * vm_return_sub - Return from subroutine (RETURN).
 *
 * Pops the top GOSUB frame and sets next_index to the
 * saved return address.
 */
void vm_return_sub(void *rt_ptr, int line_num)
{
 RuntimeState *rt = (RuntimeState *)rt_ptr;
 StackFrame frame;

 if (runtime_pop(rt, FRAME_GOSUB, &frame) != 0) {
 /* runtime_pop already raised ERR_HOW */
 (void)line_num;
 return;
 }

 rt->next_index = frame.data.gosub.return_index;

 /* If we were inside an event handler, clear the guard
 * so future events can fire */
 if (rt->event_in_handler)
 rt->event_in_handler = 0;
}

/*
 * vm_halt - Halt execution (END).
 */
void vm_halt(void *rt_ptr)
{
 vm_set_state(rt_ptr, VM_HALTED);
}

/*
 * vm_stop - Pause execution (STOP).
 */
void vm_stop(void *rt_ptr)
{
 vm_set_state(rt_ptr, VM_PAUSED);
}

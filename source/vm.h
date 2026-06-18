 // ---
 // BASIC++ Interpreter - vm.h
 // ---
 //
 // Virtual Machine formalization layer.
 //
 // PURPOSE:
 // Formalizes the interpreter's execution engine into a proper
 // virtual machine with:
 // 1. Execution state machine (VMState)
 // 2. Instruction opcode enumeration (VMOpcode)
 // 3. Dispatch table (keyword -> handler mapping)
 // 4. Expression evaluation stack (VMEvalStack)
 // 5. Control flow primitives (jump, call, return, halt, stop)
 //
 // WHY THIS EXISTS:
 // Before , execution was ad-hoc:
 // - exec.c had a raw while-loop checking rt->running
 // - Statement dispatch was a giant switch in parser.c
 // - Flow control was scattered across parser.c and exec.c
 // - No formal execution states - just boolean flags
 //
 // This formalization provides the foundation for:
 // - Bytecode format (.BPP)
 // - Module system (sandboxed dispatch)
 // - Security (capability-gated handlers)
 // - Cross-platform backends
 //
 // DISPATCH MODEL:
 // At boot, vm_init() builds a static dispatch table mapping each
 // KeywordId to a VMOpcode and handler function pointer. During
 // execution, the VM resolves the first keyword on each line to
 // an opcode, then calls the registered handler.
 //
 // Handler signature: void handler(Lexer*, RuntimeState*, int)
 // This matches the existing parse_xxx() functions exactly,
 // so migration is zero-cost.
 //
 // ---

#ifndef BASICPP_VM_H
#define BASICPP_VM_H

#include "config.h"
#include "lexer.h"
#include "value.h"

// Forward declaration - RuntimeState defined in runtime.h
struct RuntimeState_tag;

// --- Execution State Machine ---
 // Formal VM execution states. Replaces the ad-hoc combination of
 // rt->running (int) and rt->stopped (int) with a single enum.
 //
 // State transitions:
 // VM_STOPPED -> VM_RUNNING (RUN command)
 // VM_RUNNING -> VM_PAUSED (STOP statement)
 // VM_RUNNING -> VM_HALTED (END statement)
 // VM_RUNNING -> VM_ERROR (runtime error)
 // VM_PAUSED -> VM_RUNNING (CONT - future)
 // VM_ERROR -> VM_STOPPED (error handled, back to REPL)
 // VM_HALTED -> VM_STOPPED (program finished)
typedef enum VMState {
 VM_STOPPED = 0, // not running - idle at REPL
 VM_RUNNING, // normal execution in progress
 VM_PAUSED, // STOP encountered (resumable)
 VM_ERROR, // error state
 VM_HALTED // END encountered (terminal)
} VMState;

// --- Instruction Opcodes ---
 // Every statement keyword maps to exactly one opcode. The opcode
 // is used as an index into the dispatch table for O(1) handler
 // lookup.
 //
 // OP_ASSIGN handles bare variable assignment (no LET keyword).
 // OP_UNKNOWN is the fallback for unrecognized tokens.
typedef enum VMOpcode {
 OP_NOP = 0,
 OP_PRINT, OP_LET, OP_INPUT,
 OP_GOTO, OP_GOSUB, OP_RETURN, OP_IF,
 OP_FOR, OP_NEXT, OP_WHILE, OP_WEND,
 OP_DO, OP_LOOP, OP_END, OP_STOP,
 OP_REM, OP_DATA, OP_READ, OP_RESTORE,
 OP_DIM, OP_DEF, OP_ON,
 OP_TRON, OP_TROFF, OP_CLS,
 OP_LIST, OP_RUN, OP_NEW,
 OP_SAVE, OP_LOAD, OP_COMPILE,
 OP_MAT, OP_OPEN, OP_CLOSE,
 OP_CHAIN, OP_MERGE, OP_DIALECT,
 OP_BSAVE, OP_BLOAD, // bytecode
 OP_BRUN, // bytecode run
 OP_MODULE, // module system
 OP_SECURITY, // security
 OP_SYSTEM, // cross-platform
 OP_BREAK, OP_CONT, // debugger
 OP_VARS,
 OP_ASSERT, OP_TEST, // test framework
 OP_ENDTEST, OP_SELFTEST,
 OP_HELP, OP_INFO, // introspection
 OP_CATALOG,
 OP_RENUM, OP_DELETE, // final polish
 OP_VER,
 OP_BYE, // exit interpreter
 // Virtual subsystem introspection
 OP_VDEV, OP_VMEM, OP_VNET,
 OP_VCON, OP_VTERM, OP_VMACH,
 OP_DEVMAP,
 OP_ASSIGN, // bare variable assignment
 OP_UNKNOWN, // unrecognized - fallback
 OP_COUNT // sentinel - must be last
} VMOpcode;

// --- Statement Handler Signature ---
 // Every statement handler follows this uniform signature.
 // The handler receives:
 // lex - lexer positioned after the statement keyword
 // rt - runtime state (as opaque pointer for C89 compat)
 // line_num - current BASIC line number (for errors)
 //
 // Note: We use void* for rt to avoid circular includes between
 // vm.h and runtime.h. Handlers cast to (RuntimeState*) internally.
typedef void (*VMHandler)(Lexer *lex, void *rt, int line_num);

// --- Dispatch Table Entry ---
 // Maps an opcode to its handler and metadata.
typedef struct VMDispatchEntry {
 VMOpcode opcode; // the opcode this entry handles
 KeywordId keyword; // the keyword that maps to this opcode
 VMHandler handler; // function pointer to execute
 const char *name; // human-readable name (for trace/debug)
} VMDispatchEntry;

// --- Expression Evaluation Stack ---
 // A bounded stack for expression evaluation. This provides the
 // infrastructure for stack-based expression evaluation (used by
 // future bytecode VM in ). The current recursive evaluator
 // continues to work alongside this stack.
 //
 // Operations are bounds-checked. Overflow raises ERR_SORRY,
 // underflow raises ERR_HOW.
typedef struct VMEvalStack {
 BValue items[VM_EVAL_STACK_SIZE];
 int top; // index of top element, -1 = empty
} VMEvalStack;

// --- VM Functions ---

 // vm_init - Initialize the VM dispatch table.
 //
 // Must be called once at boot, after lexer initialization and
 // before any program execution. Populates the internal dispatch
 // table mapping keywords to opcodes and handlers.
void vm_init(void);

 // vm_resolve_opcode - Map a KeywordId to its VMOpcode.
 //
 // Returns OP_UNKNOWN if the keyword has no registered handler.
 // O(1) via an index table.
VMOpcode vm_resolve_opcode(KeywordId kw);

 // vm_get_handler - Get the handler function for an opcode.
 //
 // Returns NULL if the opcode has no registered handler.
VMHandler vm_get_handler(VMOpcode op);

 // vm_opcode_name - Get the human-readable name of an opcode.
 //
 // Used for trace output (TRON) and debug logging.
const char *vm_opcode_name(VMOpcode op);

 // vm_dispatch - Resolve a keyword and call its handler.
 //
 // Convenience function that combines vm_resolve_opcode +
 // vm_get_handler + call. Returns the resolved opcode.
VMOpcode vm_dispatch(KeywordId kw, Lexer *lex, void *rt, int line_num);

// --- State Machine Functions ---

void vm_set_state(void *rt, VMState state);
VMState vm_get_state(void *rt);

// --- Expression Evaluation Stack Functions ---

void vm_eval_init(VMEvalStack *stk);
int vm_eval_push(VMEvalStack *stk, BValue val);
BValue vm_eval_pop(VMEvalStack *stk);
BValue vm_eval_peek(VMEvalStack *stk);
int vm_eval_depth(VMEvalStack *stk);

// --- Control Flow Primitives ---
 // These encapsulate the line-number->index resolution and stack
 // operations that were previously scattered across parser.c and
 // exec.c. All control flow goes through these functions.

 // vm_jump - Unconditional jump to a target line number.
 //
 // Resolves the line number to an index in the program store
 // and sets rt->next_index. Raises ERR_HOW if the line is not found.
void vm_jump(void *rt, int target_line, int line_num);

 // vm_call - Subroutine call (GOSUB).
 //
 // Pushes a GOSUB frame with the return address, then jumps
 // to the target line. Raises ERR_SORRY if the stack is full.
void vm_call(void *rt, int target_line, int line_num);

 // vm_return_sub - Return from subroutine (RETURN).
 //
 // Pops the top GOSUB frame and jumps to the return address.
 // Raises ERR_HOW if the stack is empty or the frame is not GOSUB.
void vm_return_sub(void *rt, int line_num);

 // vm_halt - Halt execution (END statement).
 //
 // Sets VM state to VM_HALTED. The execution loop will stop.
void vm_halt(void *rt);

 // vm_stop - Pause execution (STOP statement).
 //
 // Sets VM state to VM_PAUSED. In future, CONT can resume.
void vm_stop(void *rt);

#endif // BASICPP_VM_H

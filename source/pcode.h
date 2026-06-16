/*
 * ---
 * BASIC++ Interpreter - pcode.h
 * ---
 *
 * PCode bytecode instruction set and program container.
 *
 * PURPOSE:
 * Defines the stack-based bytecode instruction set that the PCode
 * emitter produces from the AST and the VM executor consumes.
 * This is the intermediate representation between source text and
 * execution.
 *
 * ARCHITECTURE:
 *
 *   Source (.BAS) -> Lexer -> AST -> PCode Emitter -> PCodeProgram
 *                                                          |
 *                                          +-------+-------+
 *                                          |               |
 *                                       vm_exec       bpp_save_v2
 *                                     (interpret)    (serialize to .BPP)
 *
 * DESIGN DECISIONS:
 * 1. Stack-based VM (no register allocation needed, simpler emitter)
 * 2. Operand is a tagged union fitting in 8 bytes on 32-bit targets
 * 3. String constants live in a separate pool (not inline in instrs)
 * 4. Line map enables error reporting: instruction -> BASIC line#
 * 5. Extensible opcode space: core ops 0-63, extended 64-127,
 *    module ops 128-191, JIT hints 192-255
 * 6. C89 compliant, no VLAs, no recursion in the VM loop
 *
 * ---
 */

#ifndef BASICPP_PCODE_H
#define BASICPP_PCODE_H

#include "ast.h"
#include "vm.h"
#include "value.h"
#include "runtime.h"

/* ===================================================================
 * PCODE INSTRUCTION SET
 * ===================================================================
 *
 * Core opcodes (0-63): implemented in Phase 1-3
 * Extended opcodes (64-127): reserved for file I/O, events, etc.
 * Module opcodes (128-191): reserved for MODULE extensions
 * JIT hint opcodes (192-255): reserved for optional JIT module
 */
typedef enum PCodeOp {
    /* --- Stack / Control (0-7) --- */
    PCODE_NOP = 0,          /* no operation */
    PCODE_HALT,             /* stop execution (END) */
    PCODE_STOP,             /* pause execution (STOP) */
    PCODE_REM,              /* comment (skipped) */
    PCODE_DATA,             /* DATA (skipped at exec time) */
    PCODE_LINE,             /* line boundary marker (debug) */
    PCODE_POP,              /* discard top of stack */

    /* --- Push Constants (8-15) --- */
    PCODE_PUSH_INT = 8,     /* push integer: operand.ival */
    PCODE_PUSH_FLOAT,       /* push float: operand.fval */
    PCODE_PUSH_STRING,      /* push string: operand.str (pool ref) */
    PCODE_PUSH_ZERO,        /* push integer 0 (common case) */
    PCODE_PUSH_ONE,         /* push integer 1 (common case) */

    /* --- Load Variables (16-23) --- */
    PCODE_LOAD_VAR = 16,    /* push variable A-Z: operand.var_id */
    PCODE_LOAD_STRVAR,      /* push string var A$-Z$: operand.var_id */
    PCODE_LOAD_NAMED,       /* push named var: operand.name */
    PCODE_LOAD_AT,          /* push @(idx): idx on stack */
    PCODE_LOAD_DIM,         /* push array(i[,j]): operand.dim, indices on stack */

    /* --- Store Variables (24-31) --- */
    PCODE_STORE_VAR = 24,   /* pop into variable A-Z: operand.var_id */
    PCODE_STORE_STRVAR,     /* pop into string var A$-Z$ */
    PCODE_STORE_NAMED,      /* pop into named var: operand.name */
    PCODE_STORE_AT,         /* pop into @(idx): idx+value on stack */
    PCODE_STORE_DIM,        /* pop into array: operand.dim, indices+value on stack */

    /* --- Arithmetic (32-39) --- */
    PCODE_ADD = 32,         /* a + b  (numeric) */
    PCODE_SUB,              /* a - b */
    PCODE_MUL,              /* a * b */
    PCODE_DIV,              /* a / b */
    PCODE_MOD,              /* a MOD b */
    PCODE_POW,              /* a ^ b */
    PCODE_NEG,              /* -a  (unary negate) */
    PCODE_CONCAT,           /* a$ + b$  (string concatenation) */

    /* --- Comparison (40-47) --- */
    PCODE_CMP_EQ = 40,      /* a = b  -> 0 or -1 */
    PCODE_CMP_NE,           /* a <> b */
    PCODE_CMP_LT,           /* a < b */
    PCODE_CMP_GT,           /* a > b */
    PCODE_CMP_LE,           /* a <= b */
    PCODE_CMP_GE,           /* a >= b */

    /* --- Logic (48-51) --- */
    PCODE_AND = 48,         /* a AND b */
    PCODE_OR,               /* a OR b */
    PCODE_NOT,              /* NOT a */

    /* --- Control Flow (52-59) --- */
    PCODE_JUMP = 52,        /* unconditional: pc = operand.offset */
    PCODE_JUMP_FALSE,       /* if TOS == 0: pc = operand.offset */
    PCODE_JUMP_TRUE,        /* if TOS != 0: pc = operand.offset */
    PCODE_GOSUB,            /* push return addr, jump to operand.offset */
    PCODE_RETURN,           /* pop return addr, jump back */
    PCODE_ON_GOTO,          /* ON expr GOTO: operand.ival = table base */
    PCODE_ON_GOSUB,         /* ON expr GOSUB: operand.ival = table base */

    /* --- I/O (60-67) --- */
    PCODE_PRINT_EXPR = 60,  /* print TOS (auto-detect num/str) */
    PCODE_PRINT_NL,         /* print newline */
    PCODE_PRINT_TAB,        /* advance to next print zone */
    PCODE_PRINT_SPC,        /* print n spaces (TOS = count) */
    PCODE_INPUT_VAR,        /* input into var: operand.var_id */
    PCODE_INPUT_STRVAR,     /* input into string var */
    PCODE_INPUT_PROMPT,     /* print prompt string from pool */

    /* --- Built-in Functions (68-71) --- */
    PCODE_FUNC1 = 68,       /* 1-arg function: operand.func_id */
    PCODE_FUNC2,            /* 2-arg function */
    PCODE_FUNC3,            /* 3-arg function */

    /* --- Loops & Arrays (72-79) --- */
    PCODE_FOR_INIT = 72,    /* FOR: init,limit,step on stack; operand.var_id */
    PCODE_FOR_CHECK,        /* FOR: check limit, jmp operand.offset if done */
    PCODE_NEXT,             /* NEXT: increment, jmp operand.offset */
    PCODE_DIM_ALLOC,        /* DIM: operand.dim has name+dims, sizes on stack */
    PCODE_READ_NUM,         /* READ into numeric var: operand.var_id */
    PCODE_READ_STR,         /* READ into string var: operand.var_id */
    PCODE_RESTORE,          /* RESTORE data pointer */

    /* === Extended Range (80-127): future file I/O, events === */
    PCODE_EXTENDED_BASE = 80,

    /* === Module Range (128-191): future module opcodes === */
    PCODE_MODULE_BASE = 128,

    /* === JIT Hint Range (192-255): future JIT module === */
    PCODE_JIT_BASE = 192,

    PCODE_COUNT = 256       /* opcode space size */
} PCodeOp;

/* ===================================================================
 * PCODE OPERAND
 * ===================================================================
 *
 * Each instruction has exactly one operand. The type of the operand
 * is determined by the opcode (no separate tag byte needed).
 */
typedef struct PCodeOperand {
    union {
        long   ival;        /* integer constant, var slot, func id */
        double fval;        /* float constant */
        int    offset;      /* jump target (instruction index) */
        struct {
            int idx;        /* string pool index */
            int len;        /* string length */
        } str;
        struct {
            char name[MAX_VAR_NAME_LEN + 1];
            int  ndims;     /* 1 or 2 */
        } dim;
    } u;
} PCodeOperand;

/* ===================================================================
 * PCODE INSTRUCTION
 * ===================================================================
 */
typedef struct PCodeInstr {
    unsigned char op;       /* PCodeOp (fits in 1 byte) */
    PCodeOperand  operand;  /* instruction operand */
} PCodeInstr;

/* ===================================================================
 * PCODE PROGRAM
 * ===================================================================
 *
 * The compiled bytecode representation of a BASIC program.
 * Contains:
 * - Instruction array (the bytecode itself)
 * - String constant pool (packed, NUL-separated)
 * - Line map (instruction index -> BASIC line number)
 * - ON GOTO/GOSUB jump tables
 */
typedef struct PCodeProgram {
    /* Instruction array */
    PCodeInstr *instrs;
    int         count;
    int         capacity;

    /* String constant pool */
    char       *str_pool;
    int         str_used;
    int         str_capacity;

    /* Line number map: line_map[instr_idx] = BASIC line number.
     * Used for error reporting and debug output. */
    int        *line_map;

    /* ON GOTO/GOSUB jump tables.
     * on_tables[table_base + selector] = target instruction index.
     * Multiple tables packed sequentially. */
    int        *on_tables;
    int         on_table_count;
    int         on_table_capacity;
} PCodeProgram;

/* ===================================================================
 * PCODE COMPILER API
 * ===================================================================
 */

/*
 * pcode_compile - Compile a program store into bytecode.
 *
 * Walks each line in the program store, builds AST via ast_build_line(),
 * and emits PCode instructions. Jump targets are resolved after all
 * lines are processed.
 *
 * Returns 0 on success, -1 on error.
 */
int pcode_compile(ProgramStore *program, PCodeProgram *out_pcode);

/*
 * pcode_free - Release all memory owned by a PCodeProgram.
 */
void pcode_free(PCodeProgram *pcode);

/* ===================================================================
 * PCODE VM EXECUTOR API
 * ===================================================================
 */

/*
 * vm_exec_pcode - Execute a compiled PCodeProgram.
 *
 * Stack-based interpreter loop. Uses the RuntimeState for variable
 * storage, GOSUB stack, and error state. Returns 0 on normal halt,
 * -1 on error.
 */
int vm_exec_pcode(RuntimeState *rt, PCodeProgram *pcode);

/* ===================================================================
 * PCODE EMITTER API (used by pcode_compiler.c)
 * ===================================================================
 */

/*
 * pcode_emit_init - Initialize a PCodeProgram for emission.
 */
void pcode_emit_init(PCodeProgram *prog);

/*
 * pcode_emit_instr - Append an instruction to the program.
 * Returns the index of the emitted instruction (for backpatching).
 */
int pcode_emit_instr(PCodeProgram *prog, PCodeOp op, PCodeOperand operand);

/*
 * pcode_emit_simple - Emit a no-operand instruction.
 */
int pcode_emit_simple(PCodeProgram *prog, PCodeOp op);

/*
 * pcode_emit_int - Emit a PUSH_INT or similar with integer operand.
 */
int pcode_emit_int(PCodeProgram *prog, PCodeOp op, long ival);

/*
 * pcode_emit_float - Emit a PUSH_FLOAT with float operand.
 */
int pcode_emit_float(PCodeProgram *prog, PCodeOp op, double fval);

/*
 * pcode_emit_offset - Emit a JUMP or similar with offset operand.
 */
int pcode_emit_offset(PCodeProgram *prog, PCodeOp op, int offset);

/*
 * pcode_add_string - Add a string to the constant pool.
 * Returns the pool index. Deduplicates identical strings.
 */
int pcode_add_string(PCodeProgram *prog, const char *str, int len);

/*
 * pcode_get_string - Retrieve a string from the constant pool.
 * Returns pointer and sets *out_len to the length.
 */
const char *pcode_get_string(PCodeProgram *prog, int idx, int *out_len);

/*
 * pcode_set_line - Set the current BASIC line number for
 * subsequent emitted instructions.
 */
void pcode_set_line(PCodeProgram *prog, int line_num);

/*
 * pcode_patch_offset - Backpatch a jump target.
 * Sets prog->instrs[instr_idx].operand.u.offset = target.
 */
void pcode_patch_offset(PCodeProgram *prog, int instr_idx, int target);

/*
 * pcode_emit_expr - Emit bytecode for an AST expression tree.
 */
void pcode_emit_expr(PCodeProgram *prog, AstExpr *expr);

/*
 * pcode_emit_stmt - Emit bytecode for an AST statement.
 */
void pcode_emit_stmt(PCodeProgram *prog, AstStmt *stmt,
                     RuntimeState *rt);

#endif /* BASICPP_PCODE_H */

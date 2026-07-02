/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_jit.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - mod_jit.c
 // ---
 //
 // Optional JIT (Just-In-Time) compilation module.
 //
 // This module provides native x86-64 code generation from PCode
 // bytecode on Windows and Linux. On unsupported platforms, all
 // functions are safe no-ops.
 //
 // ARCHITECTURE:
 //   1. Allocate executable memory (VirtualAlloc / mmap)
 //   2. Walk PCode instructions, emit x86-64 machine code
 //   3. Call the generated code as a function pointer
 //   4. Free executable memory on completion
 //
 // SCOPE (initial):
 //   - PUSH_INT, PUSH_FLOAT, PUSH_ZERO, PUSH_ONE
 //   - LOAD_VAR, STORE_VAR (A-Z single-letter only)
 //   - ADD, SUB, MUL, NEG (integer arithmetic)
 //   - CMP_EQ, CMP_LT, CMP_GT
 //   - JUMP, JUMP_FALSE
 //   - PRINT_EXPR, PRINT_NL
 //   - HALT
 //
 // All other opcodes: fall back to vm_exec_pcode() interpreter.
 //
 // C17 COMPLIANCE:
 //   - ISO/IEC 9899:2018 standards compliant
 //   - Platform-specific code gated by #ifdef
 //
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mod_jit.h"
#include "module.h"
#include "vm.h"
#include "runtime.h"
#include "value.h"
#include "pcode.h"

// ===================================================================
 // JIT STATE
 // ===================================================================
static int jit_active = 0;

int jit_is_active(void) { return jit_active; }

// ===================================================================
 // PLATFORM: EXECUTABLE MEMORY ALLOCATION
 // ===================================================================
#if BPP_HAS_JIT

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType

static void *jit_alloc_exec(size_t size)
{
    return VirtualAlloc(NULL, size,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

static void jit_free_exec(void *ptr, size_t size)
{
    (void)size;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

#else // __linux__
#include <sys/mman.h>
#include "../console.h"

static void *jit_alloc_exec(size_t size)
{
    void *p = mmap(NULL, size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    return p;
}

static void jit_free_exec(void *ptr, size_t size)
{
    munmap(ptr, size);
}
#endif // _WIN32 / __linux__

// ===================================================================
 // x86-64 CODE EMITTER
 // ===================================================================
 //
 // Simple linear buffer emitter. Writes raw bytes into a flat
 // code buffer. No register allocation -- uses the x86 stack
 // for the BValue evaluation stack (mirroring the VM).
 //
 // Calling convention (System V AMD64 / Win64):
 //   We generate a void(*)(RuntimeState *rt) function.
 //   RT pointer is passed in RDI (Linux) or RCX (Windows).
 //
 // For this initial implementation, we JIT only simple integer
 // arithmetic programs. The generated code calls back into C
 // helper functions (jit_helper_print_int, etc.) for I/O.

#define JIT_CODE_MAX 65536

typedef struct JitEmitter {
    unsigned char *code;
    int            pos;
    int            capacity;
} JitEmitter;

static void emit_byte(JitEmitter *e, unsigned char b)
{
    if (e->pos < e->capacity) {
        e->code[e->pos++] = b;
    }
}

static void emit_bytes(JitEmitter *e, const unsigned char *data,
                        int count)
{
    int i;
    for (i = 0; i < count; i++)
        emit_byte(e, data[i]);
}

static void emit_u32(JitEmitter *e, unsigned int v)
{
    emit_byte(e, (unsigned char)(v & 0xFF));
    emit_byte(e, (unsigned char)((v >> 8) & 0xFF));
    emit_byte(e, (unsigned char)((v >> 16) & 0xFF));
    emit_byte(e, (unsigned char)((v >> 24) & 0xFF));
}

static void emit_u64(JitEmitter *e, unsigned long long v)
{
    emit_u32(e, (unsigned int)(v & 0xFFFFFFFFULL));
    emit_u32(e, (unsigned int)((v >> 32) & 0xFFFFFFFFULL));
}

// --- x86-64 instruction helpers ---

// push rax
static void emit_push_rax(JitEmitter *e) { emit_byte(e, 0x50); }
// pop rax
static void emit_pop_rax(JitEmitter *e) { emit_byte(e, 0x58); }
// pop rcx
static void emit_pop_rcx(JitEmitter *e) { emit_byte(e, 0x59); }
// ret
static void emit_ret(JitEmitter *e) { emit_byte(e, 0xC3); }

// mov rax, imm64
static void emit_mov_rax_imm64(JitEmitter *e, long long val)
{
    // REX.W + mov rax, imm64
    emit_byte(e, 0x48);
    emit_byte(e, 0xB8);
    emit_u64(e, (unsigned long long)val);
}

// add rax, rcx
static void emit_add_rax_rcx(JitEmitter *e)
{
    emit_byte(e, 0x48); // REX.W
    emit_byte(e, 0x01); // ADD r/m64, r64
    emit_byte(e, 0xC8); // mod=11 reg=rcx rm=rax
}

// sub rax, rcx (rax = rax - rcx)
static void emit_sub_rax_rcx(JitEmitter *e)
{
    emit_byte(e, 0x48);
    emit_byte(e, 0x29); // SUB r/m64, r64
    emit_byte(e, 0xC8); // mod=11 reg=rcx rm=rax
}

// imul rax, rcx
static void emit_imul_rax_rcx(JitEmitter *e)
{
    emit_byte(e, 0x48); // REX.W
    emit_byte(e, 0x0F);
    emit_byte(e, 0xAF);
    emit_byte(e, 0xC1); // mod=11 reg=rax rm=rcx
}

// neg rax
static void emit_neg_rax(JitEmitter *e)
{
    emit_byte(e, 0x48);
    emit_byte(e, 0xF7);
    emit_byte(e, 0xD8); // mod=11 /3 rm=rax
}

// --- C callback helpers (called from generated code) ---

// Print a long integer value followed by optional space
static void jit_helper_print_int(long val)
{
    printf("%ld", val);
}

static void jit_helper_print_nl(void)
{
    printf("\n");
}

// mov rax, addr; call rax  (call absolute function pointer)
static void emit_call_abs(JitEmitter *e, void *func_ptr)
{
    // mov rax, imm64
    emit_mov_rax_imm64(e, (long long)(size_t)func_ptr);
    // call rax: FF D0
    emit_byte(e, 0xFF);
    emit_byte(e, 0xD0);
}

// ===================================================================
 // JIT COMPILER: PCode -> x86-64
 // ===================================================================
 //
 // Walks PCode instructions and emits native code.
 // Tracks which opcodes we can JIT; if any unsupported opcode
 // is encountered, we set a flag and fall back to the VM.
 //
 // The generated function signature is:
 //   void jit_program(void)
 //
 // It uses the x86 stack for the evaluation stack (push/pop).

static int jit_emit_program(JitEmitter *e, PCodeProgram *pcode,
                             RuntimeState *rt)
{
    int pc;
    int can_jit = 1;

    (void)rt; // used in future for variable access helpers

    // Function prologue: push rbp; mov rbp, rsp
    emit_byte(e, 0x55); // push rbp
    emit_byte(e, 0x48); emit_byte(e, 0x89);
    emit_byte(e, 0xE5); // mov rbp, rsp

    // Align stack to 16 bytes
    emit_byte(e, 0x48); emit_byte(e, 0x83);
    emit_byte(e, 0xE4); emit_byte(e, 0xF0); // and rsp, -16

    for (pc = 0; pc < pcode->count && can_jit; pc++) {
        PCodeInstr *inst = &pcode->instrs[pc];
        PCodeOp op = (PCodeOp)inst->op;

        switch (op) {
        case PCODE_NOP:
        case PCODE_REM:
        case PCODE_DATA:
        case PCODE_LINE:
            break;

        case PCODE_HALT:
        case PCODE_STOP:
            // epilogue + ret
            emit_byte(e, 0x48); emit_byte(e, 0x89);
            emit_byte(e, 0xEC); // mov rsp, rbp
            emit_byte(e, 0x5D); // pop rbp
            emit_ret(e);
            break;

        case PCODE_PUSH_INT:
            emit_mov_rax_imm64(e, (long long)inst->operand.u.ival);
            emit_push_rax(e);
            break;

        case PCODE_PUSH_ZERO:
            emit_mov_rax_imm64(e, 0);
            emit_push_rax(e);
            break;

        case PCODE_PUSH_ONE:
            emit_mov_rax_imm64(e, 1);
            emit_push_rax(e);
            break;

        case PCODE_ADD:
            emit_pop_rcx(e); // b
            emit_pop_rax(e); // a
            emit_add_rax_rcx(e);
            emit_push_rax(e);
            break;

        case PCODE_SUB:
            emit_pop_rcx(e);
            emit_pop_rax(e);
            emit_sub_rax_rcx(e);
            emit_push_rax(e);
            break;

        case PCODE_MUL:
            emit_pop_rcx(e);
            emit_pop_rax(e);
            emit_imul_rax_rcx(e);
            emit_push_rax(e);
            break;

        case PCODE_NEG:
            emit_pop_rax(e);
            emit_neg_rax(e);
            emit_push_rax(e);
            break;

        case PCODE_PRINT_EXPR:
            // Pop value into first arg register, call helper
            emit_pop_rax(e);
#ifdef _WIN32
            // Win64: first arg in RCX
            emit_byte(e, 0x48); emit_byte(e, 0x89);
            emit_byte(e, 0xC1); // mov rcx, rax
#else
            // SysV: first arg in RDI
            emit_byte(e, 0x48); emit_byte(e, 0x89);
            emit_byte(e, 0xC7); // mov rdi, rax
#endif
            // Align stack before call (push dummy if needed)
            emit_byte(e, 0x48); emit_byte(e, 0x83);
            emit_byte(e, 0xEC); emit_byte(e, 0x20);
            // sub rsp, 32 (shadow space for Win64)
            emit_call_abs(e, (void *)jit_helper_print_int);
            emit_byte(e, 0x48); emit_byte(e, 0x83);
            emit_byte(e, 0xC4); emit_byte(e, 0x20);
            // add rsp, 32
            break;

        case PCODE_PRINT_NL:
            emit_byte(e, 0x48); emit_byte(e, 0x83);
            emit_byte(e, 0xEC); emit_byte(e, 0x20);
            emit_call_abs(e, (void *)jit_helper_print_nl);
            emit_byte(e, 0x48); emit_byte(e, 0x83);
            emit_byte(e, 0xC4); emit_byte(e, 0x20);
            break;

        case PCODE_PRINT_TAB:
            break; // no-op for now

        default:
            // Unsupported opcode -- fall back to VM
            can_jit = 0;
            break;
        }
    }

    if (can_jit) {
        // Ensure we have a return at the end
        emit_byte(e, 0x48); emit_byte(e, 0x89);
        emit_byte(e, 0xEC); // mov rsp, rbp
        emit_byte(e, 0x5D); // pop rbp
        emit_ret(e);
    }

    return can_jit ? 0 : -1;
}

// ===================================================================
 // PUBLIC API
 // ===================================================================

int jit_compile_and_run(void *rt_opaque, PCodeProgram *pcode)
{
    RuntimeState *rt = (RuntimeState *)rt_opaque;
    JitEmitter emitter;
    void *exec_mem;
    int result;
    typedef void (*JitFunc)(void);
    JitFunc fn;

    if (!pcode || pcode->count == 0) return -1;

    // Allocate executable memory
    exec_mem = jit_alloc_exec(JIT_CODE_MAX);
    if (!exec_mem) {
        printf("JIT: Cannot allocate executable memory.\n");
        return -1;
    }

    // Initialize emitter
    emitter.code = (unsigned char *)exec_mem;
    emitter.pos = 0;
    emitter.capacity = JIT_CODE_MAX;

    // Compile PCode to x86-64
    result = jit_emit_program(&emitter, pcode, rt);

    if (result != 0) {
        // Unsupported opcodes -- fall back to VM
        jit_free_exec(exec_mem, JIT_CODE_MAX);
        printf("JIT: Unsupported opcode, falling back to VM.\n");
        return vm_exec_pcode(rt, pcode);
    }

    // Execute the generated code
    printf("JIT: Compiled %d bytes of native code.\n", emitter.pos);
    fn = (JitFunc)exec_mem;
    fn();

    // Free executable memory
    jit_free_exec(exec_mem, JIT_CODE_MAX);

    return 0;
}

// --- Module callbacks ---

static int mod_jit_init(void *rt)
{
    (void)rt;
    jit_active = 1;
    printf("JIT engine activated (x86-64 native code generation).\n");
    return 0;
}

static void mod_jit_cleanup(void)
{
    jit_active = 0;
}

void mod_jit_register(void)
{
    static ModuleInfo info;
    info.name = "JIT";
    info.version = "1.0";
    info.description = "x86-64 JIT compiler for PCode bytecode";
    info.mod_class = MOD_EXTENSION;
    info.capabilities = CAP_SYSTEM;
    info.init = mod_jit_init;
    info.cleanup = mod_jit_cleanup;
    module_register(&info);
}

#else // !BPP_HAS_JIT

// ===================================================================
 // STUB: Platforms without JIT support
 // ===================================================================

int jit_compile_and_run(void *rt, PCodeProgram *pcode)
{
    (void)rt;
    (void)pcode;
    printf("JIT not available on this platform.\n");
    return -1;
}

void mod_jit_register(void)
{
    // No-op on unsupported platforms
}

#endif // BPP_HAS_JIT

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vm_exec.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - vm_exec.c
 // ---
 //
 // PCode VM executor: stack-based bytecode interpreter.
 //
 // DESIGN:
 // Classic fetch-decode-execute loop over PCodeInstr array.
 // Uses VMEvalStack (from vm.c) for expression evaluation.
 // Uses RuntimeState for variable storage, GOSUB stack, and
 // DATA pointer.
 //
 // BValue API:
 // - bval_to_float(const BValue *v) -- takes pointer
 // - bval_string(char *data, int length) -- constructor
 // - bval_is_string(const BValue *v) -- type check
 // - bval_add/sub/mul/div(const BValue *, const BValue *, int line_num)
 //
  // C17 COMPLIANCE:
  // - ISO/IEC 9899:2018 portability and type-safety guidelines
  // - All variables declared at top block scope
  // - No recursion in the main loop
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

#ifndef BPP_LITE_BUILD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pcode.h"
#include "vm.h"
#include "parser.h"
#include "runtime.h"
#include "funcreg.h"
#include "value.h"
#include "errors.h"
#include "stringpool.h"
#include "../console.h"
#include "memmap.h"
#include "io/vfs.h"
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

// ===================================================================
 // GOSUB CALL STACK
 // ===================================================================
#define VM_CALL_STACK_SIZE 256

// ===================================================================
 // FOR LOOP FRAME
 // ===================================================================
#define VM_FOR_STACK_SIZE 64

// ===================================================================
 // HELPER: Print a BValue
 // ===================================================================
static void print_bvalue(const BValue *val)
{
    if (bval_is_string(val)) {
        if (val->v.sval.data && val->v.sval.length > 0)
            printf("%.*s", val->v.sval.length, val->v.sval.data);
    } else {
        double d = bval_to_float(val);
        if (d == (double)(long)d) {
            printf("%ld", (long)d);
        } else {
            printf("%g", d);
        }
    }
}

static BValue vm_push_string(RuntimeState *rt, const char *s)
{
    if (s == NULL) s = "";
    int len = (int)strlen(s);
    char *pooled = strpool_alloc(&rt->strpool, len);
    if (pooled) {
        memcpy(pooled, s, (size_t)len);
        pooled[len] = '\0';
    }
    return bval_string(pooled, len);
}

// ===================================================================
 // BUILT-IN FUNCTION DISPATCH
 // ===================================================================
BValue exec_func1(int func_id, BValue *arg, int line_num,
                          RuntimeState *rt)
{
    (void)rt;
    switch (func_id) {
    case 0:  return bval_abs(arg, line_num);
    case 1: // RND
    {
        long mx = bval_to_int(arg);
        if (mx <= 0) return bval_int(0);
        return bval_int(runtime_rnd(rt, mx));
    }
    case 2:  return bval_int(runtime_size(rt));
    case 3:  return bval_sin(arg, line_num);
    case 4:  return bval_cos(arg, line_num);
    case 5:  return bval_tan(arg, line_num);
    case 6:  return bval_atn(arg, line_num);
    case 7:  return bval_sqr(arg, line_num);
    case 8:  return bval_log(arg, line_num);
    case 9:  return bval_exp(arg, line_num);
    case 10: return bval_sgn(arg, line_num);
    case 11: return bval_int_func(arg, line_num);
    case 12: return bval_len(arg, line_num);
    case 13: return bval_asc(arg, line_num);
    case 14: return bval_val(arg, line_num);
    case 15: return bval_chr(arg, line_num, &rt->strpool);
    case 16: return bval_str(arg, line_num, &rt->strpool);
    case FUNC_MEMMAP:
        return vm_push_string(rt, memmap_get_name((MemMapType)rt->memmap_type));
    case FUNC_VPATH:
        return vm_push_string(rt, vfs_get_vpath());
    case FUNC_CWD:
    case FUNC_PWD:
    {
        char cwdbuf[512];
#ifdef _WIN32
        if (_getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#else
        if (getcwd(cwdbuf, sizeof(cwdbuf)) == NULL) cwdbuf[0] = '\0';
#endif
        return vm_push_string(rt, cwdbuf);
    }
    default:
        return bval_int(0);
    }
}

BValue exec_func2(int func_id, BValue *a1, BValue *a2,
                          int line_num, RuntimeState *rt)
{
    switch (func_id) {
    case 17: // LEFT$
        return bval_left(a1, a2, line_num, &rt->strpool);
    case 18: // RIGHT$
        return bval_right(a1, a2, line_num, &rt->strpool);
    default:
        return bval_int(0);
    }
}

BValue exec_func3(int func_id, BValue *a1, BValue *a2,
                          BValue *a3, int line_num, RuntimeState *rt)
{
    switch (func_id) {
    case 19: // MID$
        return bval_mid(a1, a2, a3, line_num, &rt->strpool);
    default:
        return bval_int(0);
    }
}

static int g_bios_key_buffer = 0;

static long get_register_val(RuntimeState *rt, const char *reg_name) {
    BValue val = runtime_get_named_var_bval(rt, reg_name, (int)strlen(reg_name));
    return bval_to_int(&val);
}

static void set_register_val(RuntimeState *rt, const char *reg_name, long val) {
    runtime_set_named_var_bval(rt, reg_name, (int)strlen(reg_name), bval_int(val));
}

static void load_registers_from_vars(RuntimeState *rt, unsigned int *ax, unsigned int *bx, unsigned int *cx, unsigned int *dx) {
    long ax_val = get_register_val(rt, "AX");
    long bx_val = get_register_val(rt, "BX");
    long cx_val = get_register_val(rt, "CX");
    long dx_val = get_register_val(rt, "DX");

    long ah_val = get_register_val(rt, "AH");
    long al_val = get_register_val(rt, "AL");
    long bh_val = get_register_val(rt, "BH");
    long bl_val = get_register_val(rt, "BL");
    long ch_val = get_register_val(rt, "CH");
    long cl_val = get_register_val(rt, "CL");
    long dh_val = get_register_val(rt, "DH");
    long dl_val = get_register_val(rt, "DL");

    if (ah_val != 0 || al_val != 0) {
        *ax = (unsigned int)(((ah_val & 0xFF) << 8) | (al_val & 0xFF));
    } else {
        *ax = (unsigned int)(ax_val & 0xFFFF);
    }

    if (bh_val != 0 || bl_val != 0) {
        *bx = (unsigned int)(((bh_val & 0xFF) << 8) | (bl_val & 0xFF));
    } else {
        *bx = (unsigned int)(bx_val & 0xFFFF);
    }

    if (ch_val != 0 || cl_val != 0) {
        *cx = (unsigned int)(((ch_val & 0xFF) << 8) | (cl_val & 0xFF));
    } else {
        *cx = (unsigned int)(cx_val & 0xFFFF);
    }

    if (dh_val != 0 || dl_val != 0) {
        *dx = (unsigned int)(((dh_val & 0xFF) << 8) | (dl_val & 0xFF));
    } else {
        *dx = (unsigned int)(dx_val & 0xFFFF);
    }
}

static void write_registers_to_vars(RuntimeState *rt, unsigned int ax, unsigned int bx, unsigned int cx, unsigned int dx) {
    set_register_val(rt, "AX", ax);
    set_register_val(rt, "BX", bx);
    set_register_val(rt, "CX", cx);
    set_register_val(rt, "DX", dx);

    set_register_val(rt, "AH", (ax >> 8) & 0xFF);
    set_register_val(rt, "AL", ax & 0xFF);
    set_register_val(rt, "BH", (bx >> 8) & 0xFF);
    set_register_val(rt, "BL", bx & 0xFF);
    set_register_val(rt, "CH", (cx >> 8) & 0xFF);
    set_register_val(rt, "CL", cx & 0xFF);
    set_register_val(rt, "DH", (dx >> 8) & 0xFF);
    set_register_val(rt, "DL", dx & 0xFF);
}

static unsigned char bios_read_mem(RuntimeState *rt, int addr) {
#ifndef BPP_LITE_BUILD
    extern struct GW_Memory *g_gw_mem;
    extern uint8_t *gw_mem_get_buffer(struct GW_Memory *mem);
    if (g_gw_mem != NULL) {
        uint8_t *gw_buf = gw_mem_get_buffer(g_gw_mem);
        if (gw_buf != NULL && addr >= 0 && addr < 1048576) {
            return gw_buf[addr];
        }
    }
#endif
    if (addr >= 0 && addr < MAX_MEM_SEGMENT) {
        return rt->mem_segment[addr];
    }
    return 0;
}

static void bios_write_mem(RuntimeState *rt, int addr, unsigned char val) {
#ifndef BPP_LITE_BUILD
    extern struct GW_Memory *g_gw_mem;
    extern uint8_t *gw_mem_get_buffer(struct GW_Memory *mem);
    if (g_gw_mem != NULL) {
        uint8_t *gw_buf = gw_mem_get_buffer(g_gw_mem);
        if (gw_buf != NULL && addr >= 0 && addr < 1048576) {
            gw_buf[addr] = val;
        }
    }
#endif
    if (addr >= 0 && addr < MAX_MEM_SEGMENT) {
        rt->mem_segment[addr] = val;
    }
}

void emulate_interrupt(RuntimeState *rt, int int_num, int line_num)
{
    (void)line_num;
    unsigned int ax = 0, bx = 0, cx = 0, dx = 0;
    load_registers_from_vars(rt, &ax, &bx, &cx, &dx);

    if (int_num == 0x10) {
        unsigned char ah = (ax >> 8) & 0xFF;
        unsigned char al = ax & 0xFF;
        if (ah == 0x00) {
            bios_write_mem(rt, 0x0449, al);
            bios_write_mem(rt, 0x044A, (al == 1 || al == 0) ? 40 : 80);
            bios_write_mem(rt, 0x044B, 0);
            if (al == 3) {
                rt->cursor_row = 1;
                rt->cursor_col = 1;
                bios_write_mem(rt, 0x0450, 0);
                bios_write_mem(rt, 0x0451, 0);
            }
        } else if (ah == 0x02) {
            unsigned char dh = (dx >> 8) & 0xFF;
            unsigned char dl = dx & 0xFF;
            int row = dh + 1;
            int col = dl + 1;
            rt->cursor_row = row;
            rt->cursor_col = col;
            bios_write_mem(rt, 0x0450, dl);
            bios_write_mem(rt, 0x0451, dh);
        } else if (ah == 0x03) {
            unsigned char dl = bios_read_mem(rt, 0x0450);
            unsigned char dh = bios_read_mem(rt, 0x0451);
            cx = 0x0607;
            dx = (dh << 8) | dl;
        } else if (ah == 0x0E) {
            putchar(al);
            fflush(stdout);
            unsigned char dl = bios_read_mem(rt, 0x0450);
            unsigned char dh = bios_read_mem(rt, 0x0451);
            dl++;
            if (dl >= bios_read_mem(rt, 0x044A)) {
                dl = 0;
                dh++;
                putchar('\n');
                fflush(stdout);
            }
            bios_write_mem(rt, 0x0450, dl);
            bios_write_mem(rt, 0x0451, dh);
        }
    }
    else if (int_num == 0x16) {
        unsigned char ah = (ax >> 8) & 0xFF;
        if (ah == 0x00 || ah == 0x10) {
            int key = 0;
            while (1) {
                extern int vdev_inkey(void);
                extern void vdev_sleep(int);
                if (g_bios_key_buffer != 0) {
                    key = g_bios_key_buffer;
                    g_bios_key_buffer = 0;
                    break;
                }
                key = vdev_inkey();
                if (key != 0) break;
                vdev_sleep(5);
            }
            ax = (unsigned int)(key & 0xFF);
        } else if (ah == 0x01 || ah == 0x11) {
            extern int vdev_inkey(void);
            if (g_bios_key_buffer == 0) {
                g_bios_key_buffer = vdev_inkey();
            }
            long flags = get_register_val(rt, "FLAGS");
            if (g_bios_key_buffer != 0) {
                flags &= ~0x40; // clear ZF (key waiting)
                ax = (unsigned int)(g_bios_key_buffer & 0xFF);
            } else {
                flags |= 0x40; // set ZF (no key)
            }
            set_register_val(rt, "FLAGS", flags);
        }
    }
    else if (int_num == 0x1A) {
        unsigned char ah = (ax >> 8) & 0xFF;
        if (ah == 0x00) {
            time_t t = time(NULL);
            struct tm *lt = localtime(&t);
            long seconds = 0;
            if (lt) {
                seconds = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
            } else {
                seconds = (long)(t % 86400);
            }
            uint32_t ticks = (uint32_t)(seconds * 18.20648);
            cx = (ticks >> 16) & 0xFFFF;
            dx = ticks & 0xFFFF;
            ax = 0;
            bios_write_mem(rt, 0x046C, ticks & 0xFF);
            bios_write_mem(rt, 0x046D, (ticks >> 8) & 0xFF);
            bios_write_mem(rt, 0x046E, (ticks >> 16) & 0xFF);
            bios_write_mem(rt, 0x046F, (ticks >> 24) & 0xFF);
        }
    }
    else if (int_num == 0x11) {
        unsigned char low = bios_read_mem(rt, 0x0410);
        unsigned char high = bios_read_mem(rt, 0x0411);
        ax = (high << 8) | low;
    }
    else if (int_num == 0x12) {
        unsigned char low = bios_read_mem(rt, 0x0413);
        unsigned char high = bios_read_mem(rt, 0x0414);
        ax = (high << 8) | low;
    }

    write_registers_to_vars(rt, ax, bx, cx, dx);
}

// ===================================================================
// MAIN EXECUTION LOOP
// ===================================================================

int vm_exec_pcode(RuntimeState *rt, PCodeProgram *pcode)
{
    int pc = 0;
#define stk (rt->eval_stack)

#define VM_CALL_STACK_PUSH(val) do { \
    if (call_sp >= rt->vm_call_stack_capacity) { \
        int new_capacity = rt->vm_call_stack_capacity * 2; \
        int *new_stack = (int *)realloc(rt->vm_call_stack, (size_t)new_capacity * sizeof(int)); \
        if (new_stack == NULL) { \
            printf("Out of memory resizing call stack\n"); \
            vm_set_state(rt, VM_ERROR); \
            return -1; \
        } \
        rt->vm_call_stack = new_stack; \
        rt->vm_call_stack_capacity = new_capacity; \
        call_stack = rt->vm_call_stack; \
    } \
    call_stack[call_sp++] = (val); \
} while(0)

#define VM_CALL_STACK_POP(line) \
    ((call_sp <= 0) ? \
        ((floor(line) == (line) ? printf("RETURN without GOSUB at line %.0f\n", (double)(line)) : printf("RETURN without GOSUB at line %.2f\n", (double)(line))), \
         vm_set_state(rt, VM_ERROR), \
         -1) : \
        call_stack[--call_sp])

#define VM_FOR_STACK_PUSH(vid, limit_val, step_val, check_val) do { \
    if (for_sp >= rt->vm_for_stack_capacity) { \
        int new_capacity = rt->vm_for_stack_capacity * 2; \
        VMForFrame *new_stack = (VMForFrame *)realloc(rt->vm_for_stack, (size_t)new_capacity * sizeof(VMForFrame)); \
        if (new_stack == NULL) { \
            printf("Out of memory resizing FOR stack\n"); \
            vm_set_state(rt, VM_ERROR); \
            return -1; \
        } \
        rt->vm_for_stack = new_stack; \
        rt->vm_for_stack_capacity = new_capacity; \
        for_stack = rt->vm_for_stack; \
    } \
    for_stack[for_sp].var_id = (vid); \
    for_stack[for_sp].limit = bval_to_float(&(limit_val)); \
    for_stack[for_sp].step = bval_to_float(&(step_val)); \
    for_stack[for_sp].check_pc = (check_val); \
    for_sp++; \
} while(0)

#define VM_FOR_STACK_POP() \
    do { \
        if (for_sp > 0) for_sp--; \
    } while(0)

    // Call stack for GOSUB/RETURN (dynamic stacks from rt)
    int *call_stack = rt->vm_call_stack;
    int call_sp = 0;

    // FOR loop stack (dynamic stacks from rt)
    VMForFrame *for_stack = rt->vm_for_stack;
    int for_sp = 0;

    if (!call_stack) {
        rt->vm_call_stack_capacity = 256;
        rt->vm_call_stack = (int *)malloc((size_t)rt->vm_call_stack_capacity * sizeof(int));
        call_stack = rt->vm_call_stack;
    }
    if (!for_stack) {
        rt->vm_for_stack_capacity = 64;
        rt->vm_for_stack = (VMForFrame *)malloc((size_t)rt->vm_for_stack_capacity * sizeof(VMForFrame));
        for_stack = rt->vm_for_stack;
    }

    double line_num_d = 0.0;

    if (!pcode || !pcode->instrs || pcode->count == 0) return -1;

    vm_eval_init(&stk);

    int is_loaded_run = rt->has_loaded_pcode;

    while (pc < pcode->count) {
        if (rt->chain_pending) {
            return 0;
        }
        if (is_loaded_run && (!rt->has_loaded_pcode || rt->loaded_pcode != (void *)pcode)) {
            return 0;
        }
        PCodeInstr *inst = &pcode->instrs[pc];
        PCodeOp op = (PCodeOp)inst->op;


        // Track current line for error reporting
        line_num_d = (pcode->line_map) ? pcode->line_map[pc] : 0.0;
        g_current_executing_line = line_num_d;
        int line_num = (int)line_num_d;

        switch (op) {

        // --- Stack / Control ---
        case PCODE_NOP:
            break;

        case PCODE_HALT:
            vm_set_state(rt, VM_HALTED);
            return 0;

        case PCODE_STOP:
            vm_set_state(rt, VM_PAUSED);
            return 0;

        case PCODE_REM:
        case PCODE_DATA:
        case PCODE_LINE:
            break;

        case PCODE_POP:
            vm_eval_pop(&stk);
            break;

        // --- Push Constants ---
        case PCODE_PUSH_INT:
            vm_eval_push(&stk, bval_int(inst->operand.u.ival));
            break;

        case PCODE_PUSH_FLOAT:
            vm_eval_push(&stk, bval_float(inst->operand.u.fval));
            break;

        case PCODE_PUSH_STRING:
        {
            int slen = 0;
            const char *s;
            char *pooled;
            s = pcode_get_string(pcode,
                inst->operand.u.str.idx, &slen);
            // Allocate in string pool for proper ownership
            pooled = strpool_alloc(&rt->strpool, slen);
            if (pooled) {
                memcpy(pooled, s, (size_t)slen);
                vm_eval_push(&stk, bval_string(pooled, slen));
            } else {
                vm_eval_push(&stk, bval_string(NULL, 0));
            }
            break;
        }

        case PCODE_PUSH_ZERO:
            vm_eval_push(&stk, bval_int(0));
            break;

        case PCODE_PUSH_ONE:
            vm_eval_push(&stk, bval_int(1));
            break;

        // --- Load Variables ---
        case PCODE_LOAD_VAR:
        {
            int vid = (int)inst->operand.u.ival;
            if (vid >= 0 && vid < 26) {
                BValue val = runtime_get_var_bval(rt, (char)('A' + vid));
                vm_eval_push(&stk, val);
            } else {
                vm_eval_push(&stk, bval_int(0));
            }
            break;
        }

        case PCODE_LOAD_STRVAR:
        {
            int vid = (int)inst->operand.u.ival;
            if (vid >= 0 && vid < 26) {
                BValue val = runtime_get_string_var(rt,
                                (char)('A' + vid));
                vm_eval_push(&stk, val);
            } else {
                vm_eval_push(&stk, bval_string(NULL, 0));
            }
            break;
        }

        case PCODE_LOAD_NAMED:
        {
            const char *name = inst->operand.u.dim.name;
            int nlen = (int)strlen(name);
            BValue val = runtime_get_named_var_bval(rt, name, nlen);
            vm_eval_push(&stk, val);
            break;
        }

        case PCODE_LOAD_AT:
        {
            BValue idx_val = vm_eval_pop(&stk);
            long idx = bval_to_int(&idx_val);
            vm_eval_push(&stk,
                bval_int(runtime_get_array(rt, idx)));
            break;
        }

        case PCODE_LOAD_DIM:
        {
            const char *name = inst->operand.u.dim.name;
            int ndims = inst->operand.u.dim.ndims;
            int nlen = (int)strlen(name);
            long i1 = 0, i2 = 0;
            BValue val;
            if (ndims >= 2) {
                BValue v2 = vm_eval_pop(&stk);
                i2 = bval_to_int(&v2);
            }
            {
                BValue v1 = vm_eval_pop(&stk);
                i1 = bval_to_int(&v1);
            }
            val = runtime_get_dim(rt, name, nlen, (int)i1, (int)i2,
                                   0, line_num);
            vm_eval_push(&stk, val);
            break;
        }

        // --- Store Variables ---
        case PCODE_STORE_VAR:
        {
            int vid = (int)inst->operand.u.ival;
            BValue val = vm_eval_pop(&stk);
            if (vid >= 0 && vid < 26)
                runtime_set_var_bval(rt, (char)('A' + vid), val);
            break;
        }

        case PCODE_STORE_STRVAR:
        {
            int vid = (int)inst->operand.u.ival;
            BValue val = vm_eval_pop(&stk);
            if (vid >= 0 && vid < 26)
                runtime_set_string_var(rt, (char)('A' + vid), val);
            break;
        }

        case PCODE_STORE_NAMED:
        {
            const char *name = inst->operand.u.dim.name;
            int nlen = (int)strlen(name);
            BValue val = vm_eval_pop(&stk);
            runtime_set_named_var_bval(rt, name, nlen, val);
            break;
        }

        case PCODE_STORE_AT:
        {
            BValue val = vm_eval_pop(&stk);
            BValue idx_val = vm_eval_pop(&stk);
            long idx = bval_to_int(&idx_val);
            runtime_set_array(rt, idx, bval_to_int(&val));
            break;
        }

        case PCODE_STORE_DIM:
        {
            const char *name = inst->operand.u.dim.name;
            int ndims = inst->operand.u.dim.ndims;
            int nlen = (int)strlen(name);
            BValue val = vm_eval_pop(&stk);
            long i1 = 0, i2 = 0;
            if (ndims >= 2) {
                BValue v2 = vm_eval_pop(&stk);
                i2 = bval_to_int(&v2);
            }
            {
                BValue v1 = vm_eval_pop(&stk);
                i1 = bval_to_int(&v1);
            }
            runtime_set_dim(rt, name, nlen, (int)i1, (int)i2, 0,
                            val, line_num);
            break;
        }

        // --- Arithmetic ---
        case PCODE_ADD:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_add(&a, &b, line_num));
            break;
        }

        case PCODE_SUB:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_sub(&a, &b, line_num));
            break;
        }

        case PCODE_MUL:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_mul(&a, &b, line_num));
            break;
        }

        case PCODE_DIV:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            BValue result = bval_div(&a, &b, line_num);
            if (error_occurred()) {
                vm_set_state(rt, VM_ERROR);
                return -1;
            }
            vm_eval_push(&stk, result);
            break;
        }

        case PCODE_MOD:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_mod(&a, &b, line_num));
            break;
        }

        case PCODE_POW:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            double da = bval_to_float(&a);
            double db = bval_to_float(&b);
            vm_eval_push(&stk, bval_float(pow(da, db)));
            break;
        }

        case PCODE_NEG:
        {
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_neg(&a, line_num));
            break;
        }

        case PCODE_CONCAT:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk,
                bval_concat(&a, &b, line_num, &rt->strpool));
            break;
        }

        // --- Comparison ---
        case PCODE_CMP_EQ:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp == 0 ? -1 : 0));
            break;
        }

        case PCODE_CMP_NE:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp != 0 ? -1 : 0));
            break;
        }

        case PCODE_CMP_LT:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp < 0 ? -1 : 0));
            break;
        }

        case PCODE_CMP_GT:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp > 0 ? -1 : 0));
            break;
        }

        case PCODE_CMP_LE:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp <= 0 ? -1 : 0));
            break;
        }

        case PCODE_CMP_GE:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            int cmp = bval_compare(&a, &b, line_num);
            vm_eval_push(&stk, bval_int(cmp >= 0 ? -1 : 0));
            break;
        }

        // --- Logic ---
        case PCODE_AND:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_int(
                (bval_to_int(&a) != 0 && bval_to_int(&b) != 0)
                ? -1 : 0));
            break;
        }

        case PCODE_OR:
        {
            BValue b = vm_eval_pop(&stk);
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_int(
                (bval_to_int(&a) != 0 || bval_to_int(&b) != 0)
                ? -1 : 0));
            break;
        }

        case PCODE_NOT:
        {
            BValue a = vm_eval_pop(&stk);
            vm_eval_push(&stk, bval_int(
                bval_to_int(&a) == 0 ? -1 : 0));
            break;
        }

        // --- Control Flow ---
        case PCODE_JUMP:
            pc = inst->operand.u.offset;
            continue; // skip pc++

        case PCODE_JUMP_FALSE:
        {
            BValue cond = vm_eval_pop(&stk);
            if (bval_to_int(&cond) == 0) {
                pc = inst->operand.u.offset;
                continue;
            }
            break;
        }

        case PCODE_JUMP_TRUE:
        {
            BValue cond = vm_eval_pop(&stk);
            if (bval_to_int(&cond) != 0) {
                pc = inst->operand.u.offset;
                continue;
            }
            break;
        }
        case PCODE_GOSUB:
        {
            VM_CALL_STACK_PUSH(pc + 1);
            pc = inst->operand.u.offset;
            continue;
        }

        case PCODE_RETURN:
        {
            pc = VM_CALL_STACK_POP(line_num);
            if (pc < 0) return -1;
            continue;
        }

        case PCODE_ON_GOTO:
        {
            BValue sel_val = vm_eval_pop(&stk);
            int sel = (int)bval_to_int(&sel_val);
            int table_base = (int)inst->operand.u.ival;
            int target_idx;

            sel--; // ON is 1-based
            if (sel < 0) break;

            target_idx = table_base + sel;
            if (target_idx >= 0 &&
                target_idx < pcode->on_table_count &&
                pcode->on_tables[target_idx] > 0) {
                pc = (int)pcode->on_tables[target_idx];
                continue;
            }
            break;
        }

        case PCODE_ON_GOSUB:
        {
            BValue sel_val = vm_eval_pop(&stk);
            int sel = (int)bval_to_int(&sel_val);
            int table_base = (int)inst->operand.u.ival;
            int target_idx;

            sel--; // ON is 1-based
            if (sel < 0) break;

            target_idx = table_base + sel;
            if (target_idx >= 0 &&
                target_idx < pcode->on_table_count &&
                pcode->on_tables[target_idx] > 0) {
                VM_CALL_STACK_PUSH(pc + 1);
                pc = (int)pcode->on_tables[target_idx];
                continue;
            }
            break;
        }

        // --- I/O ---
        case PCODE_PRINT_EXPR:
        {
            BValue val = vm_eval_pop(&stk);
            print_bvalue(&val);
            break;
        }

        case PCODE_PRINT_NL:
            printf("\n");
            break;

        case PCODE_PRINT_TAB:
            printf("\t");
            break;

        case PCODE_PRINT_SPC:
        {
            BValue n = vm_eval_pop(&stk);
            int spaces = (int)bval_to_int(&n);
            int si;
            for (si = 0; si < spaces; si++) putchar(' ');
            break;
        }

        case PCODE_INPUT_PROMPT:
        {
            BValue prompt = vm_eval_pop(&stk);
            print_bvalue(&prompt);
            fflush(stdout);
            break;
        }

        case PCODE_INPUT_VAR:
        {
            int vid = (int)inst->operand.u.ival;
            char buf[256];
            if (fgets(buf, sizeof(buf), stdin)) {
                int blen = (int)strlen(buf);
                if (blen > 0 && buf[blen - 1] == '\n')
                    buf[blen - 1] = '\0';
                if (vid >= 0 && vid < 26)
                    runtime_set_var(rt, (char)('A' + vid), atol(buf));
            }
            break;
        }

        case PCODE_INPUT_STRVAR:
        {
            int vid = (int)inst->operand.u.ival;
            char buf[256];
            if (fgets(buf, sizeof(buf), stdin)) {
                char *pooled;
                int blen = (int)strlen(buf);
                if (blen > 0 && buf[blen - 1] == '\n')
                    buf[blen - 1] = '\0';
                blen = (int)strlen(buf);
                pooled = strpool_alloc(&rt->strpool, blen);
                if (pooled) {
                    memcpy(pooled, buf, (size_t)blen);
                    if (vid >= 0 && vid < 26)
                        runtime_set_string_var(rt, (char)('A' + vid),
                            bval_string(pooled, blen));
                }
            }
            break;
        }

        // --- Built-in Functions ---
        case PCODE_FUNC1:
        {
            int fid = (int)inst->operand.u.ival;
            if ((inst->operand.u.ival & 0xFF) == FUNC_FN_USER) {
                fid = FUNC_FN_USER;
            }
            int zero_args = 0;
            if (fid == FUNC_SIZE || (fid >= FUNC_MEMMAP && fid <= FUNC_PWD)) {
                zero_args = 1;
            } else if (fid >= FUNC_BUILTIN) {
                const FunctionEntry *fn = funcreg_find_by_keyword((KeywordId)fid);
                if (fn && fn->max_args == 0) {
                    zero_args = 1;
                }
            }
            BValue arg = bval_int(0);
            if (!zero_args) {
                arg = vm_eval_pop(&stk);
            }
            BValue result;
            if ((fid >= 0 && fid < FUNC_BUILTIN) ||
                (fid >= FUNC_MEMMAP && fid <= FUNC_PWD)) {
                result = exec_func1(fid, &arg, line_num, rt);
            } else {
                const FunctionEntry *fn = funcreg_find_by_keyword((KeywordId)fid);
                if (fn && fn->handler) {
                    BValue args[1];
                    args[0] = arg;
                    result = fn->handler(args, 1, rt);
                } else {
                    printf("Runtime error: Unhandled function ID %d\n", fid);
                    result = bval_int(0);
                }
            }
            vm_eval_push(&stk, result);
            break;
        }

        case PCODE_FUNC2:
        {
            int fid = (int)inst->operand.u.ival;
            BValue a2 = vm_eval_pop(&stk);
            BValue a1 = vm_eval_pop(&stk);
            BValue result;
            if (fid >= 0 && fid < FUNC_BUILTIN) {
                result = exec_func2(fid, &a1, &a2, line_num, rt);
            } else {
                const FunctionEntry *fn = funcreg_find_by_keyword((KeywordId)fid);
                if (fn && fn->handler) {
                    BValue args[2];
                    args[0] = a1;
                    args[1] = a2;
                    result = fn->handler(args, 2, rt);
                } else {
                    printf("Runtime error: Unhandled function ID %d\n", fid);
                    result = bval_int(0);
                }
            }
            vm_eval_push(&stk, result);
            break;
        }

        case PCODE_FUNC3:
        {
            int fid = (int)inst->operand.u.ival;
            BValue a3 = vm_eval_pop(&stk);
            BValue a2 = vm_eval_pop(&stk);
            BValue a1 = vm_eval_pop(&stk);
            BValue result;
            if (fid >= 0 && fid < FUNC_BUILTIN) {
                result = exec_func3(fid, &a1, &a2, &a3, line_num, rt);
            } else {
                const FunctionEntry *fn = funcreg_find_by_keyword((KeywordId)fid);
                if (fn && fn->handler) {
                    BValue args[3];
                    args[0] = a1;
                    args[1] = a2;
                    args[2] = a3;
                    result = fn->handler(args, 3, rt);
                } else {
                    printf("Runtime error: Unhandled function ID %d\n", fid);
                    result = bval_int(0);
                }
            }
            vm_eval_push(&stk, result);
            break;
        }

        // --- Loops ---
        case PCODE_FOR_INIT:
        {
            int vid = (int)inst->operand.u.ival;
            BValue step_val = vm_eval_pop(&stk);
            BValue limit_val = vm_eval_pop(&stk);
            BValue init_val = vm_eval_pop(&stk);

            // Initialize loop variable
            if (vid >= 0 && vid < 26)
                runtime_set_var(rt, (char)('A' + vid),
                                bval_to_int(&init_val));

            VM_FOR_STACK_PUSH(vid, limit_val, step_val, pc + 1);
            break;
        }

        case PCODE_FOR_CHECK:
        {
            int vid = (int)inst->operand.u.ival;
            int done = 0;

            if (for_sp > 0 && for_stack[for_sp - 1].var_id == vid) {
                double val = (vid >= 0 && vid < 26)
                    ? (double)runtime_get_var(rt, (char)('A' + vid))
                    : 0.0;
                double limit = for_stack[for_sp - 1].limit;
                double step = for_stack[for_sp - 1].step;

                if (step >= 0) {
                    done = (val > limit);
                } else {
                    done = (val < limit);
                }
            }

            if (done) {
                VM_FOR_STACK_POP();
                pc = inst->operand.u.offset;
                continue;
            }
            break;
        }

        case PCODE_NEXT:
        {
            if (for_sp > 0) {
                int fvid = for_stack[for_sp - 1].var_id;
                double step = for_stack[for_sp - 1].step;
                long cur;

                if (fvid >= 0 && fvid < 26) {
                    cur = runtime_get_var(rt, (char)('A' + fvid));
                    runtime_set_var(rt, (char)('A' + fvid),
                                    cur + (long)step);
                }

                pc = inst->operand.u.offset;
                continue;
            }
            printf("NEXT without FOR at line %d\n", line_num);
            vm_set_state(rt, VM_ERROR);
            return -1;
        }

        case PCODE_DIM_ALLOC:
        {
            const char *name = inst->operand.u.dim.name;
            int ndims = inst->operand.u.dim.ndims;
            int nlen = (int)strlen(name);
            int d1 = 0, d2 = 0;
            if (ndims >= 2) {
                BValue v2 = vm_eval_pop(&stk);
                d2 = (int)bval_to_int(&v2);
            }
            {
                BValue v1 = vm_eval_pop(&stk);
                d1 = (int)bval_to_int(&v1);
            }
            runtime_dim(rt, name, nlen, d1, d2, 0, line_num);
            break;
        }

        case PCODE_READ_NUM:
        {
            int vid = (int)inst->operand.u.ival;
            long val = runtime_read_data(rt, line_num);
            if (vid >= 0 && vid < 26)
                runtime_set_var(rt, (char)('A' + vid), val);
            break;
        }

        case PCODE_READ_STR:
        {
            int vid = (int)inst->operand.u.ival;
            BValue val = runtime_read_data_bval(rt, line_num);
            if (vid >= 0 && vid < 26)
                runtime_set_string_var(rt, (char)('A' + vid), val);
            break;
        }

        case PCODE_READ_NAMED:
        {
            const char *name = inst->operand.u.dim.name;
            int nlen = (int)strlen(name);
            BValue val = runtime_read_data_bval(rt, line_num);
            runtime_set_named_var_bval(rt, name, nlen, val);
            break;
        }

        case PCODE_RESTORE:
            runtime_restore_data(rt);
            break;

        case PCODE_INT:
            emulate_interrupt(rt, (int)inst->operand.u.ival, line_num);
            break;

        case PCODE_DIRECT_EXEC:
        {
            int slen = 0;
            const char *s = pcode_get_string(pcode, inst->operand.u.str.idx, &slen);
            Lexer lex;
            lexer_init(&lex, s);
            parser_execute_line(&lex, rt, line_num);

            if (!rt->has_loaded_pcode || rt->loaded_pcode == NULL) {
                return 0;
            }
            break;
        }

        case PCODE_WHEN_BEGIN:
        {
            if (rt->stack_top >= MAX_STACK_DEPTH) {
                printf("Exception stack overflow at line %d\n", line_num);
                vm_set_state(rt, VM_ERROR);
                return -1;
            }
            StackFrame *f = &rt->stack[rt->stack_top++];
            f->type = FRAME_EXCEPTION;
            f->data.exception.when_index = pc;
            f->data.exception.use_index = inst->operand.u.offset;
            f->data.exception.end_when_index = -1;
            f->data.exception.err_index = -1;
            break;
        }

        case PCODE_POP_EXCEPTION:
        {
            int i;
            for (i = rt->stack_top - 1; i >= 0; i--) {
                if (rt->stack[i].type == FRAME_EXCEPTION) {
                    rt->stack_top = i;
                    break;
                }
            }
            break;
        }

        case PCODE_CONTINUE:
        {
            int i;
            for (i = rt->stack_top - 1; i >= 0; i--) {
                if (rt->stack[i].type == FRAME_EXCEPTION) {
                    int err_idx = rt->stack[i].data.exception.err_index;
                    if (err_idx < 0) {
                        printf("CONTINUE without exception at line %d\n", line_num);
                        vm_set_state(rt, VM_ERROR);
                        return -1;
                    }
                    rt->stack_top = i;
                    pc = err_idx + 1;
                    continue;
                }
            }
            printf("CONTINUE outside USE block at line %d\n", line_num);
            vm_set_state(rt, VM_ERROR);
            return -1;
        }

        case PCODE_LOAD_ACC:
        {
            if (vm_eval_depth(&stk) > 0) {
                rt->acc = vm_eval_pop(&stk);
            } else {
                rt->acc = bval_int(0);
            }
            break;
        }

        case PCODE_STORE_ACC:
        {
            vm_eval_push(&stk, rt->acc);
            break;
        }

        case PCODE_SWAP:
        {
            if (vm_eval_depth(&stk) > 0) {
                BValue tmp = vm_eval_pop(&stk);
                vm_eval_push(&stk, rt->acc);
                rt->acc = tmp;
            }
            break;
        }

        case PCODE_DUP:
        {
            if (vm_eval_depth(&stk) > 0) {
                BValue top_val = vm_eval_peek(&stk);
                vm_eval_push(&stk, top_val);
            } else {
                vm_eval_push(&stk, bval_int(0));
            }
            break;
        }

        case PCODE_CLRSTACK:
        {
            rt->eval_stack.top = -1;
            break;
        }

        default:
            // Unimplemented opcode -- skip
            break;

        } // end switch

        // Check if an error occurred during execution of the instruction
        if (error_occurred()) {
            int found_handler = 0;
            int i;
            for (i = rt->stack_top - 1; i >= 0; i--) {
                if (rt->stack[i].type == FRAME_EXCEPTION) {
                    rt->stack[i].data.exception.err_index = pc;
                    rt->last_err_code = (int)error_get();
                    rt->last_err_line = line_num;
                    rt->stack_top = i + 1; // keep the exception frame on stack
                    pc = rt->stack[i].data.exception.use_index;
                    error_clear();
                    found_handler = 1;
                    break;
                }
            }
            if (found_handler) {
                continue; // execute next instruction from handler
            } else {
                if (rt->on_error_line > 0) {
                    rt->last_err_code = (int)error_get();
                    rt->last_err_line = line_num;
                    error_clear();
                    int target_idx = -1;
                    if (pcode->line_map) {
                        int k;
                        for (k = 0; k < pcode->count; k++) {
                            if (pcode->line_map[k] == rt->on_error_line) {
                                target_idx = k;
                                break;
                            }
                        }
                    }
                    if (target_idx >= 0) {
                        pc = target_idx;
                        continue;
                    }
                }
                vm_set_state(rt, VM_ERROR);
                return -1;
            }
        }

        pc++;
    } // end while

    vm_set_state(rt, VM_HALTED);
#undef stk
    return 0;
}
#endif

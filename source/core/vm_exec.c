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
 // C89 COMPLIANCE:
 // - No VLAs, no C99 features
 // - All locals declared at block top
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pcode.h"
#include "vm.h"
#include "runtime.h"
#include "value.h"
#include "errors.h"
#include "stringpool.h"

// ===================================================================
 // GOSUB CALL STACK
 // ===================================================================
#define VM_CALL_STACK_SIZE 256

// ===================================================================
 // FOR LOOP FRAME
 // ===================================================================
#define VM_FOR_STACK_SIZE 64

typedef struct VMForFrame {
    int    var_id; // 0-25 for A-Z
    double limit;
    double step;
    int    check_pc; // instruction index of FOR_CHECK
} VMForFrame;

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

// ===================================================================
 // BUILT-IN FUNCTION DISPATCH
 // ===================================================================
static BValue exec_func1(int func_id, BValue *arg, int line_num,
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
    default:
        return bval_int(0);
    }
}

static BValue exec_func2(int func_id, BValue *a1, BValue *a2,
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

static BValue exec_func3(int func_id, BValue *a1, BValue *a2,
                          BValue *a3, int line_num, RuntimeState *rt)
{
    switch (func_id) {
    case 19: // MID$
        return bval_mid(a1, a2, a3, line_num, &rt->strpool);
    default:
        return bval_int(0);
    }
}

// ===================================================================
 // MAIN EXECUTION LOOP
 // ===================================================================

int vm_exec_pcode(RuntimeState *rt, PCodeProgram *pcode)
{
    int pc = 0;
    VMEvalStack stk;

    // Call stack for GOSUB/RETURN
    int call_stack[VM_CALL_STACK_SIZE];
    int call_sp = 0;

    // FOR loop stack
    VMForFrame for_stack[VM_FOR_STACK_SIZE];
    int for_sp = 0;

    int line_num = 0;

    if (!pcode || !pcode->instrs || pcode->count == 0) return -1;

    vm_eval_init(&stk);

    while (pc < pcode->count) {
        PCodeInstr *inst = &pcode->instrs[pc];
        PCodeOp op = (PCodeOp)inst->op;

        // Track current line for error reporting
        line_num = (pcode->line_map) ? pcode->line_map[pc] : 0;

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
            if (call_sp >= VM_CALL_STACK_SIZE) {
                printf("GOSUB stack overflow at line %d\n", line_num);
                vm_set_state(rt, VM_ERROR);
                return -1;
            }
            call_stack[call_sp++] = pc + 1;
            pc = inst->operand.u.offset;
            continue;
        }

        case PCODE_RETURN:
        {
            if (call_sp <= 0) {
                printf("RETURN without GOSUB at line %d\n", line_num);
                vm_set_state(rt, VM_ERROR);
                return -1;
            }
            pc = call_stack[--call_sp];
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
                pc = pcode->on_tables[target_idx];
                continue;
            }
            break;
        }

        case PCODE_ON_GOSUB:
            break; // TODO

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
            int fid = (int)(inst->operand.u.ival & 0xFF);
            BValue arg = vm_eval_pop(&stk);
            BValue result = exec_func1(fid, &arg, line_num, rt);
            vm_eval_push(&stk, result);
            break;
        }

        case PCODE_FUNC2:
        {
            int fid = (int)(inst->operand.u.ival & 0xFF);
            BValue a2 = vm_eval_pop(&stk);
            BValue a1 = vm_eval_pop(&stk);
            BValue result = exec_func2(fid, &a1, &a2, line_num, rt);
            vm_eval_push(&stk, result);
            break;
        }

        case PCODE_FUNC3:
        {
            int fid = (int)(inst->operand.u.ival & 0xFF);
            BValue a3 = vm_eval_pop(&stk);
            BValue a2 = vm_eval_pop(&stk);
            BValue a1 = vm_eval_pop(&stk);
            BValue result = exec_func3(fid, &a1, &a2, &a3,
                                        line_num, rt);
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

            if (for_sp >= VM_FOR_STACK_SIZE) {
                printf("FOR nesting too deep at line %d\n", line_num);
                vm_set_state(rt, VM_ERROR);
                return -1;
            }

            // Initialize loop variable
            if (vid >= 0 && vid < 26)
                runtime_set_var(rt, (char)('A' + vid),
                                bval_to_int(&init_val));

            for_stack[for_sp].var_id = vid;
            for_stack[for_sp].limit = bval_to_float(&limit_val);
            for_stack[for_sp].step = bval_to_float(&step_val);
            for_stack[for_sp].check_pc = pc + 1;
            for_sp++;
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
                for_sp--;
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

        case PCODE_RESTORE:
            runtime_restore_data(rt);
            break;

        default:
            // Unimplemented opcode -- skip
            break;

        } // end switch

        pc++;
    } // end while

    vm_set_state(rt, VM_HALTED);
    return 0;
}

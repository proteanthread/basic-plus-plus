/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: pcode_emit.c
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
 // BASIC++ Interpreter - pcode_emit.c
 // ---
 //
 // PCode bytecode emitter.
 //
 // PURPOSE:
 // Walks the AST (built by ast.c) and emits stack-based bytecode
 // instructions into a PCodeProgram. This is the "compiler" phase
 // of the bytecode pipeline.
 //
 // DESIGN:
 // - Expression emission is recursive: walk the tree, emit leaves
 //   first (post-order), then operators. Result is left on stack.
 // - Statement emission is linear: each statement type has a
 //   dedicated emitter function.
 // - Jump targets use backpatching: emit a placeholder JUMP(0),
 //   record its index, then patch it once the target is known.
 // - String constants are interned into the string pool.
 //
 // C17 COMPLIANCE:
 // - ISO/IEC 9899:2018 guidelines
 // - All variables declared at top block scope.
 //
 // ---

#ifndef BPP_LITE_BUILD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcode.h"
#include "errors.h"
#include "config.h"
#include "../console.h"

// ===================================================================
 // PCODE PROGRAM INITIALIZATION & MEMORY
 // ===================================================================

// Current line number for line_map tracking
static int s_current_line = 0;

void pcode_emit_init(PCodeProgram *prog)
{
    memset(prog, 0, sizeof(*prog));
    prog->capacity = 256;
    prog->instrs = (PCodeInstr *)malloc(
        sizeof(PCodeInstr) * (size_t)prog->capacity);
    prog->line_map = (int *)malloc(
        sizeof(int) * (size_t)prog->capacity);
    prog->str_capacity = 1024;
    prog->str_pool = (char *)malloc((size_t)prog->str_capacity);
    prog->on_table_capacity = 64;
    prog->on_tables = (int *)malloc(
        sizeof(int) * (size_t)prog->on_table_capacity);

    if (!prog->instrs || !prog->line_map ||
        !prog->str_pool || !prog->on_tables) {
        // Fatal: cannot allocate bytecode buffers
        printf("PCODE: Out of memory.\n");
    }
}

static void pcode_ensure_capacity(PCodeProgram *prog, int needed)
{
    if (prog->count + needed <= prog->capacity) return;
    while (prog->capacity < prog->count + needed)
        prog->capacity *= 2;
    prog->instrs = (PCodeInstr *)realloc(prog->instrs,
        sizeof(PCodeInstr) * (size_t)prog->capacity);
    prog->line_map = (int *)realloc(prog->line_map,
        sizeof(int) * (size_t)prog->capacity);
}

// ===================================================================
 // INSTRUCTION EMISSION HELPERS
 // ===================================================================

int pcode_emit_instr(PCodeProgram *prog, PCodeOp op, PCodeOperand operand)
{
    int idx;
    pcode_ensure_capacity(prog, 1);
    idx = prog->count;
    prog->instrs[idx].op = (unsigned char)op;
    prog->instrs[idx].operand = operand;
    prog->line_map[idx] = s_current_line;
    prog->count++;
    return idx;
}

int pcode_emit_simple(PCodeProgram *prog, PCodeOp op)
{
    PCodeOperand zero;
    memset(&zero, 0, sizeof(zero));
    return pcode_emit_instr(prog, op, zero);
}

int pcode_emit_int(PCodeProgram *prog, PCodeOp op, long ival)
{
    PCodeOperand o;
    memset(&o, 0, sizeof(o));
    o.u.ival = ival;
    return pcode_emit_instr(prog, op, o);
}

int pcode_emit_float(PCodeProgram *prog, PCodeOp op, double fval)
{
    PCodeOperand o;
    memset(&o, 0, sizeof(o));
    o.u.fval = fval;
    return pcode_emit_instr(prog, op, o);
}

int pcode_emit_offset(PCodeProgram *prog, PCodeOp op, int offset)
{
    PCodeOperand o;
    memset(&o, 0, sizeof(o));
    o.u.offset = offset;
    return pcode_emit_instr(prog, op, o);
}

void pcode_set_line(PCodeProgram *prog, int line_num)
{
    (void)prog;
    s_current_line = line_num;
}

void pcode_patch_offset(PCodeProgram *prog, int instr_idx, int target)
{
    if (instr_idx >= 0 && instr_idx < prog->count) {
        prog->instrs[instr_idx].operand.u.offset = target;
    }
}

// ===================================================================
 // STRING CONSTANT POOL
 // ===================================================================

int pcode_add_string(PCodeProgram *prog, const char *str, int len)
{
    int idx;

    // Simple dedup: scan existing pool for match
    {
        int pos = 0;
        while (pos < prog->str_used) {
            int slen = (int)strlen(&prog->str_pool[pos]);
            if (slen == len &&
                memcmp(&prog->str_pool[pos], str,
                       (size_t)len) == 0) {
                return pos;
            }
            pos += slen + 1;
        }
    }

    // Need len+1 bytes (string + NUL)
    while (prog->str_used + len + 1 > prog->str_capacity) {
        prog->str_capacity *= 2;
        prog->str_pool = (char *)realloc(prog->str_pool,
            (size_t)prog->str_capacity);
    }

    idx = prog->str_used;
    memcpy(&prog->str_pool[idx], str, (size_t)len);
    prog->str_pool[idx + len] = '\0';
    prog->str_used += len + 1;
    return idx;
}

const char *pcode_get_string(PCodeProgram *prog, int idx, int *out_len)
{
    const char *s;
    if (idx < 0 || idx >= prog->str_used) {
        if (out_len) *out_len = 0;
        return "";
    }
    s = &prog->str_pool[idx];
    if (out_len) *out_len = (int)strlen(s);
    return s;
}

// ===================================================================
 // EXPRESSION EMITTER
 // ===================================================================
 //
 // Post-order traversal: emit children first, then operator.
 // The result is always left on top of the evaluation stack.

void pcode_emit_expr(PCodeProgram *prog, AstExpr *expr)
{
    PCodeOperand o;

    if (!expr) {
        // NULL expression -> push 0
        pcode_emit_simple(prog, PCODE_PUSH_ZERO);
        return;
    }

    memset(&o, 0, sizeof(o));

    switch (expr->type) {
    case EXPR_INT_LIT:
        if (expr->v.ival == 0) {
            pcode_emit_simple(prog, PCODE_PUSH_ZERO);
        } else if (expr->v.ival == 1) {
            pcode_emit_simple(prog, PCODE_PUSH_ONE);
        } else {
            pcode_emit_int(prog, PCODE_PUSH_INT, expr->v.ival);
        }
        break;

    case EXPR_FLOAT_LIT:
        pcode_emit_float(prog, PCODE_PUSH_FLOAT, expr->v.fval);
        break;

    case EXPR_STRING_LIT:
    {
        int pool_idx = pcode_add_string(prog,
            expr->v.sval.data, expr->v.sval.length);
        o.u.str.idx = pool_idx;
        o.u.str.len = expr->v.sval.length;
        pcode_emit_instr(prog, PCODE_PUSH_STRING, o);
        break;
    }

    case EXPR_VAR:
        pcode_emit_int(prog, PCODE_LOAD_VAR,
                       (long)(expr->v.var_name - 'A'));
        break;

    case EXPR_STRING_VAR:
        pcode_emit_int(prog, PCODE_LOAD_STRVAR,
                       (long)(expr->v.var_name - 'A'));
        break;

    case EXPR_NAMED_VAR:
    {
        memcpy(o.u.dim.name, expr->v.named.name,
               (size_t)(expr->v.named.name_len + 1));
        o.u.dim.ndims = 0; // not an array access
        pcode_emit_instr(prog, PCODE_LOAD_NAMED, o);
        break;
    }

    case EXPR_ARRAY_AT:
        // Push index, then LOAD_AT
        pcode_emit_expr(prog, expr->v.array_at.index);
        pcode_emit_simple(prog, PCODE_LOAD_AT);
        break;

    case EXPR_DIM_ACCESS:
    {
        // Push indices, then LOAD_DIM with name
        pcode_emit_expr(prog, expr->v.dim_access.idx1);
        if (expr->v.dim_access.idx2) {
            pcode_emit_expr(prog, expr->v.dim_access.idx2);
            o.u.dim.ndims = 2;
        } else {
            o.u.dim.ndims = 1;
        }
        memcpy(o.u.dim.name, expr->v.dim_access.name,
               (size_t)(expr->v.dim_access.name_len + 1));
        pcode_emit_instr(prog, PCODE_LOAD_DIM, o);
        break;
    }

    case EXPR_BINOP:
        // Emit left, right, then operator
        pcode_emit_expr(prog, expr->v.binop.left);
        pcode_emit_expr(prog, expr->v.binop.right);
        switch (expr->v.binop.op) {
        case BOP_ADD:    pcode_emit_simple(prog, PCODE_ADD); break;
        case BOP_SUB:    pcode_emit_simple(prog, PCODE_SUB); break;
        case BOP_MUL:    pcode_emit_simple(prog, PCODE_MUL); break;
        case BOP_DIV:    pcode_emit_simple(prog, PCODE_DIV); break;
        case BOP_MOD:    pcode_emit_simple(prog, PCODE_MOD); break;
        case BOP_POW:    pcode_emit_simple(prog, PCODE_POW); break;
        case BOP_EQ:     pcode_emit_simple(prog, PCODE_CMP_EQ); break;
        case BOP_NE:     pcode_emit_simple(prog, PCODE_CMP_NE); break;
        case BOP_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
        case BOP_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
        case BOP_LE:     pcode_emit_simple(prog, PCODE_CMP_LE); break;
        case BOP_GE:     pcode_emit_simple(prog, PCODE_CMP_GE); break;
        case BOP_AND:    pcode_emit_simple(prog, PCODE_AND); break;
        case BOP_OR:     pcode_emit_simple(prog, PCODE_OR); break;
        case BOP_CONCAT: pcode_emit_simple(prog, PCODE_CONCAT); break;
        }
        break;

    case EXPR_UNOP:
        pcode_emit_expr(prog, expr->v.unop.operand);
        switch (expr->v.unop.op) {
        case UOP_NEG: pcode_emit_simple(prog, PCODE_NEG); break;
        case UOP_NOT: pcode_emit_simple(prog, PCODE_NOT); break;
        }
        break;

    case EXPR_FUNC_CALL:
    {
        // Push arguments, then FUNC1/2/3 with func_id
        int argc = expr->v.func_call.arg_count;
        int fi;
        PCodeOp func_op;
        for (fi = 0; fi < argc && fi < 3; fi++) {
            if (expr->v.func_call.args[fi]) {
                pcode_emit_expr(prog, expr->v.func_call.args[fi]);
            }
        }
        switch (argc) {
        case 0:  func_op = PCODE_FUNC1; break; // SIZE() etc
        case 1:  func_op = PCODE_FUNC1; break;
        case 2:  func_op = PCODE_FUNC2; break;
        case 3:  func_op = PCODE_FUNC3; break;
        default: func_op = PCODE_FUNC1; break;
        }
        // Encode function ID + FN letter for user funcs
        if (expr->v.func_call.func == FUNC_FN_USER) {
            // Pack: high byte = fn_letter, low byte = FUNC_FN_USER
            o.u.ival = (long)expr->v.func_call.func |
                       ((long)expr->v.func_call.fn_letter << 8);
        } else if (expr->v.func_call.func == FUNC_BUILTIN) {
            o.u.ival = (long)expr->v.func_call.builtin_kw;
        } else {
            o.u.ival = (long)expr->v.func_call.func;
        }
        pcode_emit_instr(prog, func_op, o);
        break;
    }
    } // end switch
}

// ===================================================================
 // STATEMENT EMITTER
 // ===================================================================

void pcode_emit_stmt(PCodeProgram *prog, AstStmt *stmt,
                     RuntimeState *rt)
{
    PCodeOperand o;
    (void)rt; // reserved for future use (DATA collection etc.)

    if (!stmt) return;

    memset(&o, 0, sizeof(o));

    switch (stmt->type) {

    case STMT_REM:
        // Comments are not emitted
        break;

    case STMT_DATA:
        // DATA is collected separately, not executed
        break;

    case STMT_PRINT:
    {
        int pi;
        for (pi = 0; pi < stmt->v.print.item_count; pi++) {
            AstPrintItem *item = &stmt->v.print.items[pi];
            if (item->is_hash_width) {
                // #width: emit expression but don't print it --
                 // set print width. For now, pop and discard. 
                if (item->expr) {
                    pcode_emit_expr(prog, item->expr);
                    pcode_emit_simple(prog, PCODE_POP);
                }
            } else if (item->expr) {
                pcode_emit_expr(prog, item->expr);
                pcode_emit_simple(prog, PCODE_PRINT_EXPR);
            } else {
                // NULL expr = tab advance (comma separator)
                pcode_emit_simple(prog, PCODE_PRINT_TAB);
            }
        }
        if (!stmt->v.print.trailing_comma) {
            pcode_emit_simple(prog, PCODE_PRINT_NL);
        }
        break;
    }

    case STMT_LET:
        pcode_emit_expr(prog, stmt->v.let.value);
        pcode_emit_int(prog, PCODE_STORE_VAR,
                       (long)(stmt->v.let.var_name - 'A'));
        break;

    case STMT_LET_STRVAR:
        pcode_emit_expr(prog, stmt->v.let_strvar.value);
        pcode_emit_int(prog, PCODE_STORE_STRVAR,
                       (long)(stmt->v.let_strvar.var_name - 'A'));
        break;

    case STMT_LET_ARRAY_AT:
        pcode_emit_expr(prog, stmt->v.let_array_at.index);
        pcode_emit_expr(prog, stmt->v.let_array_at.value);
        pcode_emit_simple(prog, PCODE_STORE_AT);
        break;

    case STMT_LET_DIM:
    {
        // Push indices, then value, then STORE_DIM
        pcode_emit_expr(prog, stmt->v.let_dim.idx1);
        if (stmt->v.let_dim.idx2) {
            pcode_emit_expr(prog, stmt->v.let_dim.idx2);
            o.u.dim.ndims = 2;
        } else {
            o.u.dim.ndims = 1;
        }
        pcode_emit_expr(prog, stmt->v.let_dim.value);
        memcpy(o.u.dim.name, stmt->v.let_dim.name,
               (size_t)(stmt->v.let_dim.name_len + 1));
        pcode_emit_instr(prog, PCODE_STORE_DIM, o);
        break;
    }

    case STMT_IF:
    {
        int jmp_false_idx;
        // Emit condition
        pcode_emit_expr(prog, stmt->v.if_stmt.condition);
        // Emit conditional jump (patch later)
        jmp_false_idx = pcode_emit_offset(prog, PCODE_JUMP_FALSE, 0);
        // Emit THEN body
        if (stmt->v.if_stmt.then_stmt) {
            pcode_emit_stmt(prog, stmt->v.if_stmt.then_stmt, rt);
        }
        // Patch jump to skip over THEN body
        pcode_patch_offset(prog, jmp_false_idx, prog->count);
        break;
    }

    case STMT_GOTO:
    {
        // GOTO: emit a JUMP. The target is a BASIC line number
         // stored as an expression (usually a constant). We store
         // the line number in the operand and resolve later. 
        if (stmt->v.goto_stmt.target &&
            stmt->v.goto_stmt.target->type == EXPR_INT_LIT) {
            // Store negative line number as marker for
             // line-number-based jump (resolved after full compile) 
            pcode_emit_int(prog, PCODE_JUMP,
                           -(stmt->v.goto_stmt.target->v.ival));
        } else {
            // Computed GOTO -- emit expression, not supported yet
            pcode_emit_simple(prog, PCODE_HALT);
        }
        break;
    }

    case STMT_GOSUB:
    {
        if (stmt->v.gosub.target &&
            stmt->v.gosub.target->type == EXPR_INT_LIT) {
            pcode_emit_int(prog, PCODE_GOSUB,
                           -(stmt->v.gosub.target->v.ival));
        } else {
            pcode_emit_simple(prog, PCODE_HALT);
        }
        break;
    }

    case STMT_RETURN:
        pcode_emit_simple(prog, PCODE_RETURN);
        break;

    case STMT_FOR:
    {
        // FOR var = init TO limit [STEP step]
         // Emit: push init, push limit, push step, FOR_INIT(var)
         // The FOR_CHECK and NEXT are emitted by NEXT. 
        pcode_emit_expr(prog, stmt->v.for_stmt.init);
        pcode_emit_expr(prog, stmt->v.for_stmt.limit);
        if (stmt->v.for_stmt.step) {
            pcode_emit_expr(prog, stmt->v.for_stmt.step);
        } else {
            pcode_emit_simple(prog, PCODE_PUSH_ONE);
        }
        pcode_emit_int(prog, PCODE_FOR_INIT,
                       (long)(stmt->v.for_stmt.var_name - 'A'));
        // FOR_CHECK is emitted as a placeholder -- the NEXT handler
         // will patch back to here for the loop check. The FOR_CHECK
         // instruction index is recorded by the compiler for pairing. 
        pcode_emit_int(prog, PCODE_FOR_CHECK, 0);
        break;
    }

    case STMT_NEXT:
    {
        // NEXT var: increment and jump back to FOR_CHECK.
         // The target is resolved by the compiler's loop stack. 
        pcode_emit_int(prog, PCODE_NEXT,
                       (long)(stmt->v.next.var_name - 'A'));
        break;
    }

    case STMT_INPUT:
    {
        int vi;
        // Emit prompt if present
        if (stmt->v.input.prompt) {
            pcode_emit_expr(prog, stmt->v.input.prompt);
            pcode_emit_simple(prog, PCODE_INPUT_PROMPT);
        } else {
            // Default prompt: "? "
            int pidx = pcode_add_string(prog, "? ", 2);
            o.u.str.idx = pidx;
            o.u.str.len = 2;
            pcode_emit_instr(prog, PCODE_PUSH_STRING, o);
            pcode_emit_simple(prog, PCODE_INPUT_PROMPT);
        }
        // Input each variable
        for (vi = 0; vi < stmt->v.input.var_count; vi++) {
            if (stmt->v.input.var_types[vi] == 1) {
                // String variable
                pcode_emit_int(prog, PCODE_INPUT_STRVAR,
                    (long)(stmt->v.input.var_names[vi] - 'A'));
            } else {
                // Numeric variable
                pcode_emit_int(prog, PCODE_INPUT_VAR,
                    (long)(stmt->v.input.var_names[vi] - 'A'));
            }
        }
        break;
    }

    case STMT_END:
        pcode_emit_simple(prog, PCODE_HALT);
        break;

    case STMT_STOP:
        pcode_emit_simple(prog, PCODE_STOP);
        break;

    case STMT_DIM:
    {
        // DIM name(d1[,d2])
        pcode_emit_expr(prog, stmt->v.dim.dim1);
        if (stmt->v.dim.dim2) {
            pcode_emit_expr(prog, stmt->v.dim.dim2);
            o.u.dim.ndims = 2;
        } else {
            o.u.dim.ndims = 1;
        }
        memcpy(o.u.dim.name, stmt->v.dim.name,
               (size_t)(stmt->v.dim.name_len + 1));
        pcode_emit_instr(prog, PCODE_DIM_ALLOC, o);
        break;
    }

    case STMT_READ:
    {
        int ri;
        for (ri = 0; ri < stmt->v.read.var_count; ri++) {
            if (stmt->v.read.var_types[ri] == 1) {
                pcode_emit_int(prog, PCODE_READ_STR,
                    (long)(stmt->v.read.var_names[ri] - 'A'));
            } else {
                pcode_emit_int(prog, PCODE_READ_NUM,
                    (long)(stmt->v.read.var_names[ri] - 'A'));
            }
        }
        break;
    }

    case STMT_RESTORE:
        pcode_emit_simple(prog, PCODE_RESTORE);
        break;

    case STMT_WHILE:
    {
        // WHILE is handled specially: emit condition check.
         // WEND will backpatch the jump back to here. 
        // WHILE uses left relop right format in the AST
        pcode_emit_expr(prog, stmt->v.while_stmt.left);
        if (stmt->v.while_stmt.right) {
            pcode_emit_expr(prog, stmt->v.while_stmt.right);
            // Emit comparison based on relop
            switch (stmt->v.while_stmt.relop) {
            case TOK_EQUALS: pcode_emit_simple(prog, PCODE_CMP_EQ); break;
            case TOK_NOT_EQ: pcode_emit_simple(prog, PCODE_CMP_NE); break;
            case TOK_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
            case TOK_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
            case TOK_LT_EQ:  pcode_emit_simple(prog, PCODE_CMP_LE); break;
            case TOK_GT_EQ:  pcode_emit_simple(prog, PCODE_CMP_GE); break;
            default:         pcode_emit_simple(prog, PCODE_CMP_NE); break;
            }
        }
        // JUMP_FALSE placeholder (patched by WEND)
        pcode_emit_offset(prog, PCODE_JUMP_FALSE, 0);
        break;
    }

    case STMT_WEND:
        // WEND: jump back to WHILE condition.
         // Resolved by the compiler's loop stack. 
        pcode_emit_offset(prog, PCODE_JUMP, 0);
        break;

    case STMT_DO:
        // DO [WHILE/UNTIL condition]: loop top marker.
         // Condition check emitted here for pre-test loops. 
        if (stmt->v.do_stmt.has_condition == 1) {
            // DO WHILE: emit condition
            pcode_emit_expr(prog, stmt->v.do_stmt.left);
            if (stmt->v.do_stmt.right) {
                pcode_emit_expr(prog, stmt->v.do_stmt.right);
                // Comparison for relop
                switch (stmt->v.do_stmt.relop) {
                case TOK_EQUALS: pcode_emit_simple(prog, PCODE_CMP_EQ); break;
                case TOK_NOT_EQ: pcode_emit_simple(prog, PCODE_CMP_NE); break;
                case TOK_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
                case TOK_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
                case TOK_LT_EQ:  pcode_emit_simple(prog, PCODE_CMP_LE); break;
                case TOK_GT_EQ:  pcode_emit_simple(prog, PCODE_CMP_GE); break;
                default:         pcode_emit_simple(prog, PCODE_CMP_NE); break;
                }
            }
            pcode_emit_offset(prog, PCODE_JUMP_FALSE, 0);
        } else if (stmt->v.do_stmt.has_condition == 2) {
            // DO UNTIL: emit condition, jump if TRUE
            pcode_emit_expr(prog, stmt->v.do_stmt.left);
            if (stmt->v.do_stmt.right) {
                pcode_emit_expr(prog, stmt->v.do_stmt.right);
                switch (stmt->v.do_stmt.relop) {
                case TOK_EQUALS: pcode_emit_simple(prog, PCODE_CMP_EQ); break;
                case TOK_NOT_EQ: pcode_emit_simple(prog, PCODE_CMP_NE); break;
                case TOK_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
                case TOK_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
                case TOK_LT_EQ:  pcode_emit_simple(prog, PCODE_CMP_LE); break;
                case TOK_GT_EQ:  pcode_emit_simple(prog, PCODE_CMP_GE); break;
                default:         pcode_emit_simple(prog, PCODE_CMP_NE); break;
                }
            }
            pcode_emit_offset(prog, PCODE_JUMP_TRUE, 0);
        }
        // DO (infinite): no condition, no jump
        break;

    case STMT_LOOP:
        // LOOP: jump back to DO. Resolved by compiler loop stack.
         // LOOP WHILE/UNTIL conditions handled similarly. 
        if (stmt->v.loop_stmt.has_condition == 1) {
            // LOOP WHILE
            pcode_emit_expr(prog, stmt->v.loop_stmt.left);
            if (stmt->v.loop_stmt.right) {
                pcode_emit_expr(prog, stmt->v.loop_stmt.right);
                switch (stmt->v.loop_stmt.relop) {
                case TOK_EQUALS: pcode_emit_simple(prog, PCODE_CMP_EQ); break;
                case TOK_NOT_EQ: pcode_emit_simple(prog, PCODE_CMP_NE); break;
                case TOK_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
                case TOK_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
                case TOK_LT_EQ:  pcode_emit_simple(prog, PCODE_CMP_LE); break;
                case TOK_GT_EQ:  pcode_emit_simple(prog, PCODE_CMP_GE); break;
                default:         pcode_emit_simple(prog, PCODE_CMP_NE); break;
                }
            }
            // Jump back to loop top if TRUE
            pcode_emit_offset(prog, PCODE_JUMP_TRUE, 0);
        } else if (stmt->v.loop_stmt.has_condition == 2) {
            // LOOP UNTIL
            pcode_emit_expr(prog, stmt->v.loop_stmt.left);
            if (stmt->v.loop_stmt.right) {
                pcode_emit_expr(prog, stmt->v.loop_stmt.right);
                switch (stmt->v.loop_stmt.relop) {
                case TOK_EQUALS: pcode_emit_simple(prog, PCODE_CMP_EQ); break;
                case TOK_NOT_EQ: pcode_emit_simple(prog, PCODE_CMP_NE); break;
                case TOK_LT:     pcode_emit_simple(prog, PCODE_CMP_LT); break;
                case TOK_GT:     pcode_emit_simple(prog, PCODE_CMP_GT); break;
                case TOK_LT_EQ:  pcode_emit_simple(prog, PCODE_CMP_LE); break;
                case TOK_GT_EQ:  pcode_emit_simple(prog, PCODE_CMP_GE); break;
                default:         pcode_emit_simple(prog, PCODE_CMP_NE); break;
                }
            }
            // Jump back if FALSE (condition NOT met yet)
            pcode_emit_offset(prog, PCODE_JUMP_FALSE, 0);
        } else {
            // LOOP (infinite) - unconditional jump back
            pcode_emit_offset(prog, PCODE_JUMP, 0);
        }
        break;

    case STMT_ON_GOTO:
    {
        int ti;
        // ON expr GOTO/GOSUB line1, line2, ...
        pcode_emit_expr(prog, stmt->v.on_goto.selector);
        // Store jump table base in on_tables
        o.u.ival = (long)prog->on_table_count;
        pcode_emit_instr(prog, stmt->v.on_goto.is_gosub ? PCODE_ON_GOSUB : PCODE_ON_GOTO, o);
        // Append target line numbers to on_tables (resolved later)
        for (ti = 0; ti < stmt->v.on_goto.target_count; ti++) {
            if (prog->on_table_count >= prog->on_table_capacity) {
                prog->on_table_capacity *= 2;
                prog->on_tables = (int *)realloc(prog->on_tables,
                    sizeof(int) * (size_t)prog->on_table_capacity);
            }
            // Store as negative line number for later resolution
            prog->on_tables[prog->on_table_count++] =
                -(stmt->v.on_goto.targets[ti]);
        }
        // Sentinel: 0 marks end of table
        if (prog->on_table_count >= prog->on_table_capacity) {
            prog->on_table_capacity *= 2;
            prog->on_tables = (int *)realloc(prog->on_tables,
                sizeof(int) * (size_t)prog->on_table_capacity);
        }
        prog->on_tables[prog->on_table_count++] = 0;
        break;
    }

    case STMT_WHEN:
        pcode_emit_offset(prog, PCODE_WHEN_BEGIN, 0);
        break;

    case STMT_USE:
        pcode_emit_simple(prog, PCODE_POP_EXCEPTION);
        pcode_emit_offset(prog, PCODE_JUMP, 0);
        break;

    case STMT_END_WHEN:
        pcode_emit_simple(prog, PCODE_POP_EXCEPTION);
        break;

    case STMT_RETRY:
        pcode_emit_offset(prog, PCODE_JUMP, 0);
        break;

    case STMT_CONTINUE:
        pcode_emit_simple(prog, PCODE_CONTINUE);
        break;

    case STMT_INT:
    {
        long int_val = 0;
        if (stmt->v.int_stmt.interrupt_number) {
            AstExpr *ex = stmt->v.int_stmt.interrupt_number;
            if (ex->type == EXPR_INT_LIT) {
                int_val = ex->v.ival;
            }
        }
        pcode_emit_int(prog, PCODE_INT, int_val);
        break;
    }

    case STMT_DIRECT_EXEC:
    {
        int pool_idx = pcode_add_string(prog,
            stmt->v.direct_exec.text, (int)strlen(stmt->v.direct_exec.text));
        o.u.str.idx = pool_idx;
        o.u.str.len = (int)strlen(stmt->v.direct_exec.text);
        pcode_emit_instr(prog, PCODE_DIRECT_EXEC, o);
        break;
    }

    case STMT_DEF_FN:
        // DEF FN is handled at parse time, not in bytecode
        break;

    } // end switch

    // Emit chain (multi-statement lines: 10 PRINT "A": PRINT "B")
    if (stmt->next) {
        pcode_emit_stmt(prog, stmt->next, rt);
    }
}
#endif

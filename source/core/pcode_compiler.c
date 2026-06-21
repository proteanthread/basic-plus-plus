/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: pcode_compiler.c
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
 // BASIC++ Interpreter - pcode_compiler.c
 // ---
 //
 // PCode compiler: orchestrates AST building and bytecode emission.
 //
 // IMPLEMENTATION:
 // For each program line:
 // 1. Initialize a Lexer on the line text
 // 2. Skip the line number token
 // 3. Build an AST via ast_build_line()
 // 4. Emit bytecode via pcode_emit_stmt()
 // 5. Free the AST
 //
 // After all lines are processed:
 // - Resolve GOTO/GOSUB line-number targets to instruction offsets
 // - Patch FOR/NEXT loop pairs
 // - Emit a trailing HALT instruction
 //
 // LOOP PAIRING:
 // FOR/NEXT and WHILE/WEND are paired using a compile-time stack.
 // When we see FOR_INIT+FOR_CHECK, we push the FOR_CHECK index.
 // When we see NEXT, we pop and patch both sides.
 //
 // LINE NUMBER RESOLUTION:
 // GOTO/GOSUB emit negative line numbers as placeholders.
 // After all lines compile, we build a line_num -> instruction_idx
 // map and patch every negative-value JUMP/GOSUB operand.
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
#include "pcode.h"
#include "ast.h"
#include "lexer.h"
#include "errors.h"
#include "runtime.h"

// ===================================================================
 // LOOP STACK (compile-time FOR/NEXT, WHILE/WEND pairing)
 // ===================================================================
#define MAX_LOOP_DEPTH 64

typedef enum {
    LOOP_FOR,
    LOOP_WHILE,
    LOOP_DO
} LoopType;

typedef struct {
    LoopType type;
    int      check_idx; // instruction index of FOR_CHECK or WHILE cond
    int      body_start; // first instruction of loop body
    int      var_id; // FOR variable (0-25)
} LoopEntry;

static LoopEntry s_loop_stack[MAX_LOOP_DEPTH];
static int       s_loop_top = -1;

static void loop_push(LoopType type, int check_idx, int body_start,
                      int var_id)
{
    if (s_loop_top >= MAX_LOOP_DEPTH - 1) {
        printf("PCODE: Loop nesting too deep.\n");
        return;
    }
    s_loop_top++;
    s_loop_stack[s_loop_top].type = type;
    s_loop_stack[s_loop_top].check_idx = check_idx;
    s_loop_stack[s_loop_top].body_start = body_start;
    s_loop_stack[s_loop_top].var_id = var_id;
}

static int loop_pop(LoopType expected, LoopEntry *out)
{
    if (s_loop_top < 0) {
        printf("PCODE: Unmatched loop terminator.\n");
        return -1;
    }
    if (s_loop_stack[s_loop_top].type != expected) {
        printf("PCODE: Mismatched loop type.\n");
        return -1;
    }
    *out = s_loop_stack[s_loop_top];
    s_loop_top--;
    return 0;
}

// ===================================================================
 // LINE NUMBER -> INSTRUCTION INDEX MAP
 // ===================================================================
 //
 // After all lines are compiled, we build a map from BASIC line numbers
 // to instruction indices. Then we patch every JUMP/GOSUB that has
 // a negative operand (which is -(target_line_number)).

typedef struct {
    int line_num;
    int instr_idx;
} LineMapEntry;

static LineMapEntry *s_line_map = NULL;
static int           s_line_map_count = 0;
static int           s_line_map_capacity = 0;

static void linemap_reset(void)
{
    if (s_line_map) {
        free(s_line_map);
        s_line_map = NULL;
    }
    s_line_map_count = 0;
    s_line_map_capacity = 0;
}

static void linemap_add(int line_num, int instr_idx)
{
    if (s_line_map_count >= s_line_map_capacity) {
        s_line_map_capacity = (s_line_map_capacity == 0)
                              ? 128 : s_line_map_capacity * 2;
        s_line_map = (LineMapEntry *)realloc(s_line_map,
            sizeof(LineMapEntry) * (size_t)s_line_map_capacity);
    }
    s_line_map[s_line_map_count].line_num = line_num;
    s_line_map[s_line_map_count].instr_idx = instr_idx;
    s_line_map_count++;
}

static int linemap_resolve(int line_num)
{
    int i;
    for (i = 0; i < s_line_map_count; i++) {
        if (s_line_map[i].line_num == line_num)
            return s_line_map[i].instr_idx;
    }
    return -1; // not found
}

// ===================================================================
 // RESOLVE PASS
 // ===================================================================
 //
 // Walk all instructions and patch negative JUMP/GOSUB operands.
static int resolve_line_jumps(PCodeProgram *pcode)
{
    int i;
    int errors = 0;

    for (i = 0; i < pcode->count; i++) {
        PCodeInstr *inst = &pcode->instrs[i];

        // JUMP and GOSUB with negative operand = -(line number)
        if ((inst->op == (unsigned char)PCODE_JUMP ||
             inst->op == (unsigned char)PCODE_GOSUB) &&
            inst->operand.u.ival < 0) {
            int target_line = (int)(-(inst->operand.u.ival));
            int target_idx = linemap_resolve(target_line);
            if (target_idx < 0) {
                printf("PCODE: Undefined line %d at instruction %d\n",
                       target_line, i);
                errors++;
            } else {
                inst->operand.u.offset = target_idx;
            }
        }
    }

    // Also resolve ON GOTO tables
    for (i = 0; i < pcode->on_table_count; i++) {
        if (pcode->on_tables[i] < 0) {
            int target_line = -(pcode->on_tables[i]);
            int target_idx = linemap_resolve(target_line);
            if (target_idx < 0) {
                printf("PCODE: Undefined line %d in ON GOTO table\n",
                       target_line);
                errors++;
            } else {
                pcode->on_tables[i] = target_idx;
            }
        }
    }

    return errors;
}

// ===================================================================
 // MAIN COMPILER ENTRY POINT
 // ===================================================================

int pcode_compile(ProgramStore *program, PCodeProgram *out_pcode)
{
    int i;
    int compile_errors = 0;

    if (!program || !out_pcode) return -1;

    // Initialize
    pcode_emit_init(out_pcode);
    s_loop_top = -1;
    linemap_reset();

    // Pass 1: Compile each program line
    for (i = 0; i < program->count; i++) {
        ProgramLine *pl = &program->lines[i];
        Lexer lex;
        AstStmt *stmts;
        int line_num = pl->line_number;
        int instr_before;
        int instr_after;

        // Record line -> instruction mapping
        instr_before = out_pcode->count;
        linemap_add(line_num, instr_before);
        pcode_set_line(out_pcode, line_num);

        // Initialize lexer and skip line number
        lexer_init(&lex, pl->text);
        if (lex.current.type == TOK_NUMBER) {
            lexer_next(&lex);
        }

        // Skip empty lines
        if (lex.current.type == TOK_EOF ||
            lex.current.type == TOK_CR) {
            continue;
        }

        // Clear error state before parsing
        error_clear();

        // Build AST
        stmts = ast_build_line(&lex, line_num);

        if (error_occurred() || !stmts) {
            // Parse error -- skip this line
            if (stmts) ast_free_line(stmts);
            error_clear();
            compile_errors++;
            continue;
        }

        // Emit bytecode from AST
        pcode_emit_stmt(out_pcode, stmts, NULL);

        // Track loop constructs
        instr_after = out_pcode->count;
        {
            // Scan emitted instructions for loop markers
            int j;
            for (j = instr_before; j < instr_after; j++) {
                unsigned char op = out_pcode->instrs[j].op;

                if (op == (unsigned char)PCODE_FOR_CHECK) {
                    // FOR_CHECK: push loop entry
                    int var_id = (int)out_pcode->instrs[j]
                                 .operand.u.ival;
                    // The FOR_CHECK is the loop-check instruction.
                     // Its offset will be patched to jump past NEXT
                     // when the limit is exceeded. body_start is
                     // the instruction after FOR_CHECK. 
                    loop_push(LOOP_FOR, j, j + 1, var_id);
                }
                else if (op == (unsigned char)PCODE_NEXT) {
                    // NEXT: pop matching FOR
                    LoopEntry entry;
                    if (loop_pop(LOOP_FOR, &entry) == 0) {
                        // NEXT jumps back to FOR_CHECK
                        out_pcode->instrs[j].operand.u.offset =
                            entry.check_idx;
                        // FOR_CHECK jumps past NEXT when done
                        pcode_patch_offset(out_pcode,
                            entry.check_idx, j + 1);
                    }
                }
                else if (op == (unsigned char)PCODE_JUMP_FALSE) {
                    // Check if this is a WHILE condition.
                     // We identify WHILE by looking at the STMT_WHILE
                     // context -- but since we've already flattened to
                     // instructions, we use a heuristic: if the previous
                     // instruction sequence looks like a WHILE pattern,
                     // push onto loop stack. Actually, let's use the
                     // line map: if this line had a WHILE statement,
                     // the AST would have emitted the condition +
                     // JUMP_FALSE. We track this in the statement
                     // emitter directly. For now, WHILE/WEND pairing
                     // uses the line-number-based GOTO. 
                    // TODO: Enhanced WHILE/WEND loop pairing
                }
            }
        }

        // Free AST
        ast_free_line(stmts);
    }

    // Trailing HALT (safety net)
    pcode_emit_simple(out_pcode, PCODE_HALT);

    // Pass 2: Resolve line-number-based jumps
    compile_errors += resolve_line_jumps(out_pcode);

    // Check for unmatched loops
    if (s_loop_top >= 0) {
        printf("PCODE: %d unmatched FOR without NEXT\n",
               s_loop_top + 1);
        compile_errors++;
    }

    // Clean up
    linemap_reset();

    if (compile_errors > 0) {
        printf("PCODE: %d compile error(s).\n", compile_errors);
        return -1;
    }

    return 0;
}

void pcode_free(PCodeProgram *pcode)
{
    if (!pcode) return;
    if (pcode->instrs) {
        free(pcode->instrs);
        pcode->instrs = NULL;
    }
    if (pcode->line_map) {
        free(pcode->line_map);
        pcode->line_map = NULL;
    }
    if (pcode->str_pool) {
        free(pcode->str_pool);
        pcode->str_pool = NULL;
    }
    if (pcode->on_tables) {
        free(pcode->on_tables);
        pcode->on_tables = NULL;
    }
    pcode->count = 0;
    pcode->capacity = 0;
    pcode->str_used = 0;
    pcode->str_capacity = 0;
    pcode->on_table_count = 0;
    pcode->on_table_capacity = 0;
}

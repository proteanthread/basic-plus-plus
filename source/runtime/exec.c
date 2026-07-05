/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: exec.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Runtime state container (RuntimeState) and statement execution dispatch loops.
 *
 * 2. WHAT TO EXPECT:
 *    RuntimeState encapsulates all interpreter variables, stack frames, heap pools, and execution context.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic trace outputs, signal trapping overrides.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    RuntimeState struct layout, statement execution sequence logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - exec.c
 // ---
 //
 // Program execution loop implementation.
 //
 // DESIGN RATIONALE:
 // The execution loop iterates through program lines in stored
 // order, tokenizes each line on-the-fly, and delegates to the
 // parser for execution. Flow control (GOTO, GOSUB, RETURN)
 // works by modifying rt->next_index, which the loop checks
 // after each line.
 //
 // VM Formalization
 // The execution loop now uses the formal VM state machine
 // (VMState) instead of the ad-hoc rt->running flag. State
 // transitions go through vm_set_state()/vm_get_state().
 // The opcode is resolved via vm_resolve_opcode() for trace
 // output, and the control flow primitives (vm_jump, vm_call,
 // vm_return_sub) are available to the parser.
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
#include <signal.h>
#include <time.h>
#include "exec.h"
#include "parser.h"
#include "lexer.h"
#include "runtime.h"
#include "errors.h"
#include "vdev.h"
#include "vm.h"
#include "pcode.h"
#include "scope.h"
#include "override.h"
#include "task.h"
#include "../console.h"
#include "fileio.h"
#include "ast.h"
#include "ast_interpreter.h"

// --- OS Signal Handler (Tier 3) ---
 // Async-safe: only sets a flag. The event_poll() loop
 // checks this flag cooperatively.
static volatile int g_signal_pending = 0;
static RuntimeState *g_signal_rt = NULL;

static void signal_handler(int sig)
{
 (void)sig;
 g_signal_pending = 1;
 if (g_signal_rt != NULL)
  g_signal_rt->signal_pending = 1;
 // Re-install handler (required on some platforms)
 signal(SIGINT, signal_handler);
}

 // event_poll - Cooperative event dispatch (called between statements).
 //
 // Checks all event tiers in priority order and fires the first
 // pending event handler via GOSUB. Only one event fires per poll
 // cycle to keep latency bounded.
 //
 // Priority:
 // Tier 3 - OS signals (Ctrl+C / SIGINT)
 // Tier 1 - TIMER
 // Tier 2 - Device I/O (VDev poll)
 // Tier 4 - File I/O
 //
 // STOP-mode: events in state==EVT_STOP are queued instead of fired.
static void event_poll(RuntimeState *rt, double line_num)
{
 // Guard: don't fire events inside event handlers
 if (rt->event_in_handler) return;

 // Guard: don't fire if error or not running
 if (error_occurred()) return;
 if (vm_get_state(rt) != VM_RUNNING) return;

 // --- Tier 3: OS signal (SIGINT / Ctrl+C) ---
 if (rt->signal_pending) {
  rt->signal_pending = 0;
  g_signal_pending = 0;
  if (rt->on_break_line > 0 &&
      rt->break_event_state == EVT_ON) {
   StackFrame bf;
   rt->event_in_handler = 1;
   bf.type = FRAME_GOSUB;
   bf.data.gosub.return_index =
    rt->current_index + 1;
   if (runtime_push(rt, &bf) == 0) {
    vm_jump(rt, rt->on_break_line, line_num);
   }
   return;
  }
  // No handler: default behavior (stop program)
  if (rt->on_break_line == 0) {
   if (floor(line_num) == line_num) {
       printf("\n[BREAK - Ctrl+C at line %.0f]\n", line_num);
   } else {
       printf("\n[BREAK - Ctrl+C at line %.2f]\n", line_num);
   }
   vm_set_state(rt, VM_PAUSED);
   rt->resume_index = rt->current_index;
   return;
  }
 }

 // --- Tier 1: TIMER event ---
 if (rt->timer_event_state == EVT_ON &&
     rt->on_timer_line > 0 &&
     rt->timer_interval > 0.0) {
  double now = (double)time(NULL);
  if (now - rt->timer_last_fire >= rt->timer_interval) {
   StackFrame tf;
   rt->timer_last_fire = now;
   rt->event_in_handler = 1;
   tf.type = FRAME_GOSUB;
   tf.data.gosub.return_index =
    rt->current_index + 1;
   if (runtime_push(rt, &tf) == 0) {
    vm_jump(rt, rt->on_timer_line, line_num);
   }
   return;
  }
 }

 // --- Tier 2: Device I/O (VDev VDCAP_EVENT poll) ---
  // Walk registered devices. For any with VDCAP_EVENT
  // and ON state, call dev_poll(). Fire handler on
  // data ready.
 {
  int di;
  for (di = 0; di < MAX_DEVICE_TRAPS; di++) {
   if (rt->device_event_state[di] == EVT_ON &&
       rt->on_device_line[di] > 0) {
    VDev *dv = vdev_get(di);
    if (dv != NULL &&
        (dv->dev_caps & VDCAP_EVENT) &&
        dv->dev_poll != NULL &&
        dv->dev_poll(dv) > 0) {
     StackFrame df;
     rt->event_in_handler = 1;
     df.type = FRAME_GOSUB;
     df.data.gosub.return_index =
      rt->current_index + 1;
     if (runtime_push(rt, &df) == 0) {
      vm_jump(rt, rt->on_device_line[di],
       line_num);
     }
     return;
    }
   }
  }
 }

 // --- Tier 4: File I/O events ---
 if (rt->fileio_event_state == EVT_ON &&
     rt->on_fileio_line > 0 &&
     rt->fileio_pending) {
  StackFrame ff;
  rt->fileio_pending = 0;
  rt->event_in_handler = 1;
  ff.type = FRAME_GOSUB;
  ff.data.gosub.return_index =
   rt->current_index + 1;
  if (runtime_push(rt, &ff) == 0) {
   vm_jump(rt, rt->on_fileio_line, line_num);
  }
  return;
 }
}

 // exec_run - Main program execution loop.
 //
 // Execution flow:
 // 1. Reset runtime state (variables, stack, position).
 // 2. Collect DATA values from all program lines.
 // 3. Set VM state to VM_RUNNING.
 // 4. Loop:
 // a. Get line at current_index.
 // b. Tokenize line.
 // c. Skip line number token.
 // d. Resolve opcode via VM dispatch table (for trace).
 // e. Execute via parser_execute_line().
 // f. Check for errors -> transition to VM_ERROR.
 // g. Check next_index -> jump or advance.
 // h. Check bounds -> halt if past last line.
 // 5. On exit: set VM state to VM_STOPPED.
 // exec_run_from - Internal execution loop starting from a given index.
 //
 // Shared by exec_run (fresh start) and exec_cont (resume).
static void exec_run_from(RuntimeState *rt, int start_index)
{
#ifndef BPP_LITE_BUILD
    if (!rt->direct_mode) {
        vm_set_state(rt, VM_RUNNING);
        while (vm_get_state(rt) == VM_RUNNING && !error_occurred()) {
            if (rt->lite_mode && (!rt->has_loaded_pcode || !rt->loaded_pcode)) {
                if (rt->line_asts == NULL) {
                    rt->line_asts = (AstStmt **)calloc((size_t)rt->program->count, sizeof(AstStmt *));
                    rt->line_asts_count = rt->program->count;
                    int i;
                    for (i = 0; i < rt->program->count; i++) {
                        Lexer lex;
                        lexer_init(&lex, rt->program->lines[i].text);
                        while (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
                            lexer_next(&lex);
                        }
                        rt->line_asts[i] = ast_build_line(&lex, (int)rt->program->lines[i].line_number);
                    }
                }

                vm_set_state(rt, VM_RUNNING);
                rt->current_index = start_index;
                g_arithmetic_decimal = rt->arithmetic_decimal;

                while (vm_get_state(rt) == VM_RUNNING &&
                       rt->current_index < rt->program->count &&
                       !error_occurred()) {
                    int idx = rt->current_index;
                    AstStmt *stmt = rt->line_asts[idx];
                    double line_num = rt->program->lines[idx].line_number;

                    int trigger_compile = 0;
                    AstStmt *s = stmt;
                    while (s) {
                        if (s->type == STMT_FOR || s->type == STMT_WHILE) {
                            trigger_compile = 1;
                            break;
                        }
                        s = s->next;
                    }

                    if (trigger_compile) {
                        PCodeProgram *pcode = (PCodeProgram *)malloc(sizeof(PCodeProgram));
                        if (pcode) {
                            if (pcode_compile(rt->program, pcode) == 0) {
                                rt->loaded_pcode = pcode;
                                rt->has_loaded_pcode = 1;

                                int resume_pc = 0;
                                int i;
                                for (i = 0; i < pcode->count; i++) {
                                    if (pcode->line_map && pcode->line_map[i] == line_num) {
                                        resume_pc = i;
                                        break;
                                    }
                                }
                                rt->resume_index = resume_pc;
                                start_index = 0;
                                break;
                            } else {
                                free(pcode);
                            }
                        }
                    }

                    rt->next_index = rt->current_index + 1;
                    AstStmt *curr = stmt;
                    while (curr && vm_get_state(rt) == VM_RUNNING && !error_occurred()) {
                        ast_interpret_stmt(rt, curr, (int)line_num);
                        curr = curr->next;
                    }
                    rt->current_index = rt->next_index;
                }

                if (vm_get_state(rt) == VM_RUNNING && rt->has_loaded_pcode) {
                    continue;
                }

                if (vm_get_state(rt) != VM_PAUSED && vm_get_state(rt) != VM_RUNNING) {
                    vm_set_state(rt, VM_STOPPED);
                }
                break;
            }

            PCodeProgram *p;
            if (!rt->has_loaded_pcode || !rt->loaded_pcode) {
                PCodeProgram *pcode = (PCodeProgram *)malloc(sizeof(PCodeProgram));
                if (!pcode) {
                    printf("Out of memory for bytecode compiler\n");
                    vm_set_state(rt, VM_ERROR);
                    return;
                }
                if (pcode_compile(rt->program, pcode) != 0) {
                    printf("Compilation failed.\n");
                    free(pcode);
                    vm_set_state(rt, VM_ERROR);
                    return;
                }
                rt->loaded_pcode = pcode;
                rt->has_loaded_pcode = 1;

                double resume_line = 0.0;
                if (rt->current_index >= 0 && rt->current_index < rt->program->count) {
                    resume_line = rt->program->lines[rt->current_index].line_number;
                }
                if (resume_line > 0.0) {
                    int i;
                    int found_pc = -1;
                    for (i = 0; i < pcode->count; i++) {
                        if (pcode->line_map && pcode->line_map[i] == resume_line) {
                            found_pc = i;
                            break;
                        }
                    }
                    if (found_pc >= 0) {
                        rt->resume_index = found_pc;
                    }
                }
            }

            p = (PCodeProgram *)rt->loaded_pcode;

            if (rt->resume_index < 0 && start_index > 0 && start_index < rt->program->count) {
                double target_line = rt->program->lines[start_index].line_number;
                int i;
                for (i = 0; i < p->count; i++) {
                    if (p->line_map && p->line_map[i] == target_line) {
                        rt->resume_index = i;
                        break;
                    }
                }
                start_index = 0;
            }

            g_arithmetic_decimal = rt->arithmetic_decimal;
            int result = vm_exec_pcode(rt, p);

            if (rt->chain_pending) {
                if (fileio_chain(&rt->memory->program, rt->chain_file) == 0) {
                    rt->chain_pending = 0;
                    rt->chain_file[0] = '\0';
                    pcode_cache_invalidate(rt);
                    
                    runtime_pre_scan_external_declarations(rt);
                    runtime_collect_data(rt);
                    runtime_collect_labels(rt);
                    {
                        int idx;
                        ProgramStore *pgm = rt->program;
                        for (idx = 0; idx < pgm->count; idx++) {
                            Lexer cl;
                            const char *text = pgm->lines[idx].text;
                            double ln = pgm->lines[idx].line_number;
                            lexer_init(&cl, text);
                            if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT || cl.current.type == TOK_FLOAT_LIT)
                                lexer_next(&cl);
                            if (cl.current.type == TOK_KEYWORD &&
                                (cl.current.value.keyword == KW_SUB ||
                                 cl.current.value.keyword == KW_FUNCTION ||
                                 cl.current.value.keyword == KW_DEFINE)) {
                                rt->current_index = idx;
                                rt->next_index = -1;
                                parser_execute_line(&cl, rt, ln);
                                if (error_occurred()) {
                                    vm_set_state(rt, VM_ERROR);
                                    return;
                                }
                                if (rt->next_index > idx)
                                    idx = rt->next_index - 1;
                            }
                        }
                    }
                    rt->current_index = 0;
                    continue; // re-evaluate start of new program
                } else {
                    rt->chain_pending = 0;
                    error_raise(ERR_HOW, 0);
                    vm_set_state(rt, VM_ERROR);
                    break;
                }
            }

            if (!rt->has_loaded_pcode) {
                continue;
            }

            if (result != 0 && vm_get_state(rt) != VM_PAUSED) {
                vm_set_state(rt, VM_ERROR);
            } else if (vm_get_state(rt) != VM_PAUSED) {
                vm_set_state(rt, VM_STOPPED);
            }
            break;
        }
        return;
    }
#endif
    {
 Lexer lex;
 int skip_first_break = 1; // Skip breakpoint on first line (CONT resume)

 vm_set_state(rt, VM_RUNNING);
 rt->current_index = start_index;
 g_arithmetic_decimal = rt->arithmetic_decimal;

 while (vm_get_state(rt) == VM_RUNNING &&
 rt->current_index < rt->program->count &&
 !error_occurred()) {
 ProgramLine *line;
 double line_num;

 if (rt->chain_pending) {
     if (fileio_chain(&rt->memory->program, rt->chain_file) == 0) {
         rt->chain_pending = 0;
         rt->chain_file[0] = '\0';
         pcode_cache_invalidate(rt);
         
         runtime_pre_scan_external_declarations(rt);
         runtime_collect_data(rt);
         runtime_collect_labels(rt);
         {
             int idx;
             ProgramStore *pgm = rt->program;
             for (idx = 0; idx < pgm->count; idx++) {
                 Lexer cl;
                 const char *text = pgm->lines[idx].text;
                 double ln = pgm->lines[idx].line_number;
                 lexer_init(&cl, text);
                 if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT || cl.current.type == TOK_FLOAT_LIT)
                     lexer_next(&cl);
                 if (cl.current.type == TOK_KEYWORD &&
                     (cl.current.value.keyword == KW_SUB ||
                      cl.current.value.keyword == KW_FUNCTION ||
                      cl.current.value.keyword == KW_DEFINE)) {
                     rt->current_index = idx;
                     rt->next_index = -1;
                     parser_execute_line(&cl, rt, ln);
                     if (error_occurred()) {
                         vm_set_state(rt, VM_ERROR);
                         return;
                     }
                     if (rt->next_index > idx)
                         idx = rt->next_index - 1;
                 }
             }
         }
         rt->current_index = 0;
         rt->next_index = -1;
         skip_first_break = 1;
         continue;
     } else {
         rt->chain_pending = 0;
         error_raise(ERR_HOW, 0);
         vm_set_state(rt, VM_ERROR);
         break;
     }
 }

  // Get the current line
  line = &rt->program->lines[rt->current_index];
  line_num = line->line_number;
  g_current_executing_line = line_num;

  // Breakpoint / single-step check.
  // If we hit a breakpoint or single_step is on,
  // pause and return to REPL so user can inspect.
  // Skip the check on the first line after CONT.
  if (!skip_first_break) {
  if (rt->single_step ||
  (rt->breakpoint_count > 0 &&
  runtime_is_breakpoint(rt, line_num))) {
  if (floor(line_num) == line_num) {
      printf("[BREAK at line %.0f]\n", line_num);
  } else {
      printf("[BREAK at line %.2f]\n", line_num);
  }
  vm_set_state(rt, VM_PAUSED);
  rt->resume_index = rt->current_index;
  return;
  }
  }
  skip_first_break = 0;

   // Trace output (TRON/TROFF).
   if (rt->trace_on) {
       if (floor(line_num) == line_num) {
           vdev_printf(rt->dev_con, "[%.0f]", line_num);
       } else {
           vdev_printf(rt->dev_con, "[%.2f]", line_num);
       }
   }

   // Verbose trace output (DEBUG ON/OFF).
   // Shows [line] followed by the full source text.
   if (rt->debug_on) {
       if (floor(line_num) == line_num) {
           vdev_printf(rt->dev_con, "[%.0f] %s\n", line_num, line->text);
       } else {
           vdev_printf(rt->dev_con, "[%.2f] %s\n", line_num, line->text);
       }
   }

  // Initialize lexer on the line text
  if (rt->resumed) {
  rt->resumed = 0;
  lex = *(Lexer *)rt->restored_lexer;
  } else {
  lexer_init(&lex, line->text);

  // Skip the line number.
  if (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
  lexer_next(&lex);
  }
  }

 // Reset next_index to -1 (no jump pending)
 rt->next_index = -1;

 // Suppress error messages when ON ERROR GOTO
 // or WHEN EXCEPTION is active.
 {
 int suppress = 0;
 if (rt->on_error_line > 0)
 suppress = 1;
 if (!suppress) {
 // Check stack for FRAME_EXCEPTION
 int ei;
 for (ei = rt->stack_top - 1;
 ei >= 0; ei--) {
 if (rt->stack[ei].type ==
 FRAME_EXCEPTION) {
 suppress = 1;
 break;
 }
 }
 }
 error_set_suppress(suppress);
 }

   // SCOPE hook dispatch via event queue (Q10).
   //
   // Before executing the line, peek at the first keyword
   // and check for SCOPE rules (disabled, BEFORE,
   // OVERRIDE, AFTER hooks). Hooks are queued into the
   // event queue and drained between statements.
  {
  KeywordId first_kw = KW_COUNT;
  int in_hook = 0;

  // Peek at first keyword without consuming
  if (lex.current.type == TOK_KEYWORD)
   first_kw = lex.current.value.keyword;

  // Disabled check (always applies)
  if (first_kw < KW_COUNT &&
   scope_is_disabled(first_kw)) {
   printf("Keyword disabled by SCOPE"
    " at line %d\n", line_num);
   error_raise(ERR_WHAT, line_num);
   goto scope_done;
  }

   // Re-entrancy guard: if we're inside
   // a hook subroutine (stack is above
   // the saved hook entry point), don't
   // fire hooks.
  if (rt->scope_hook_depth >= 0 &&
   rt->stack_top >
   rt->scope_hook_depth) {
   in_hook = 1;
  }

  // Queue hooks (only outside hook subs)
  if (!in_hook && first_kw < KW_COUNT) {

   // BEFORE hook -> queue
   if (scope_get_before(first_kw) >= 0 &&
    rt->scope_before_done !=
    rt->current_index &&
    rt->scope_evq_count < SCOPE_EVQ_SIZE) {
    int qi = rt->scope_evq_tail;
    rt->scope_evq[qi].hook_type = 0;
    rt->scope_evq[qi].hook_line =
     scope_get_before(first_kw);
    rt->scope_evq[qi].return_idx =
     rt->current_index;
    rt->scope_evq[qi].keyword_id =
     (int)first_kw;
    rt->scope_evq_tail =
     (qi + 1) % SCOPE_EVQ_SIZE;
    rt->scope_evq_count++;
    rt->scope_before_done =
     rt->current_index;
   }

   // OVERRIDE hook -> queue (skips keyword)
   if (scope_get_override(first_kw) >= 0 &&
    rt->scope_evq_count < SCOPE_EVQ_SIZE) {
    int qi = rt->scope_evq_tail;
    rt->scope_evq[qi].hook_type = 2;
    rt->scope_evq[qi].hook_line =
     scope_get_override(first_kw);
    rt->scope_evq[qi].return_idx =
     rt->current_index + 1;
    rt->scope_evq[qi].keyword_id =
     (int)first_kw;
    rt->scope_evq_tail =
     (qi + 1) % SCOPE_EVQ_SIZE;
    rt->scope_evq_count++;
   }
  }

   // Drain event queue: process queued hooks
   // before executing the current statement.
  if (rt->scope_evq_count > 0) {
   int qi = rt->scope_evq_head;
   int htype = rt->scope_evq[qi].hook_type;
   int hline = rt->scope_evq[qi].hook_line;
   int hret = rt->scope_evq[qi].return_idx;
   int hkw = rt->scope_evq[qi].keyword_id;
   StackFrame hf;

   rt->scope_evq_head =
    (qi + 1) % SCOPE_EVQ_SIZE;
   rt->scope_evq_count--;

   rt->scope_hook_depth = rt->stack_top;
   scope_set_last_kw((KeywordId)hkw);

   hf.type = FRAME_GOSUB;
   hf.data.gosub.return_index = hret;
   if (runtime_push(rt, &hf) != 0)
    goto scope_done;
   vm_jump(rt, hline, line_num);
   // BEFORE: return to same line
    // OVERRIDE: return to next line 
   (void)htype;
   goto scope_done;
  }

  // Record AFTER hook keyword for post-exec
  if (!in_hook &&
   first_kw < KW_COUNT &&
   scope_get_after(first_kw) >= 0) {
   rt->scope_after_kw = (int)first_kw;
  } else {
   rt->scope_after_kw = -1;
  }

   // OVERRIDE keyword interpretation.
    // If the first keyword has an active
    // override and we're not inside a hook,
    // build a modified parse string from
    // the override text + original args. 
   if (!in_hook && first_kw < KW_COUNT &&
    override_is_active(first_kw)) {
    const char *otxt =
     override_get(first_kw);
    if (otxt != NULL) {
    char splice[1024];
    const char *src = lex.source;
    int slen = 0;
    int otlen = (int)strlen(otxt);
    const char *rest;
    int rlen;

    rest = src + lex.pos;
    while (*rest == ' ' || *rest == '\t')
     rest++;
    rlen = (int)strlen(rest);

    if (otlen + 1 + rlen < 1023) {
     memcpy(splice, otxt,
      (size_t)otlen);
     slen = otlen;
     if (rlen > 0) {
     splice[slen++] = ' ';
     memcpy(splice + slen, rest,
      (size_t)rlen);
     slen += rlen;
     }
     splice[slen] = '\0';

     {
     Lexer olex;
     int saved_depth =
      rt->scope_hook_depth;
     rt->scope_hook_depth =
      rt->stack_top;
     lexer_init(&olex, splice);
     parser_execute_line(
      &olex, rt, line_num);
     rt->scope_hook_depth =
      saved_depth;
     }
     goto override_done;
    }
    }
   }

   // Parse and execute the line
   parser_execute_line(&lex, rt, line_num);
override_done:
   ; // empty statement after label
   } // end scope/override block

  // Clear BEFORE-done guard now that
   // the line has fully executed. 
  if (rt->scope_before_done ==
   rt->current_index) {
   rt->scope_before_done = -1;
  }

  // SCOPE AFTER hook: queue into event queue
  if (rt->scope_after_kw >= 0 &&
   !error_occurred() &&
   rt->next_index < 0 &&
   rt->scope_evq_count < SCOPE_EVQ_SIZE) {
   int after_kw = rt->scope_after_kw;
   int hook_line =
    scope_get_after((KeywordId)after_kw);
   rt->scope_after_kw = -1;

   if (hook_line >= 0) {
    int qi = rt->scope_evq_tail;
    rt->scope_evq[qi].hook_type = 1;
    rt->scope_evq[qi].hook_line = hook_line;
    rt->scope_evq[qi].return_idx =
     rt->current_index + 1;
    rt->scope_evq[qi].keyword_id = after_kw;
    rt->scope_evq_tail =
     (qi + 1) % SCOPE_EVQ_SIZE;
    rt->scope_evq_count++;
   }
  } else {
   rt->scope_after_kw = -1;
  }

  // Drain AFTER events immediately
  if (rt->scope_evq_count > 0 &&
   !error_occurred() &&
   rt->next_index < 0) {
   int qi = rt->scope_evq_head;
   int hline = rt->scope_evq[qi].hook_line;
   int hret = rt->scope_evq[qi].return_idx;
   int hkw = rt->scope_evq[qi].keyword_id;
   StackFrame af;

   rt->scope_evq_head =
    (qi + 1) % SCOPE_EVQ_SIZE;
   rt->scope_evq_count--;

   scope_set_last_kw((KeywordId)hkw);
   rt->scope_hook_depth = rt->stack_top;
   af.type = FRAME_GOSUB;
   af.data.gosub.return_index = hret;
   if (runtime_push(rt, &af) == 0) {
    vm_jump(rt, hline, line_num);
   }
  }

   // --- Event polling (6-tier cooperative dispatch) ---
   if (!error_occurred() &&
       vm_get_state(rt) == VM_RUNNING &&
       rt->next_index < 0) {
    event_poll(rt, line_num);
   }
   if (task_has_background_active()) {
       task_scheduler_tick();
   }
scope_done:

 // Restore error output
 error_set_suppress(0);

 // WHEN EXCEPTION handler (ECMA-116).
 //
 // Block-scoped: checked FIRST, before ON ERROR GOTO.
 // If an error occurred and there's a FRAME_EXCEPTION
 // on the stack, redirect to the USE handler.
 if (error_occurred()) {
 int ei;
 for (ei = rt->stack_top - 1; ei >= 0; ei--) {
 if (rt->stack[ei].type == FRAME_EXCEPTION) {
 int use_idx = rt->stack[ei]
 .data.exception.use_index;

 // Save error info for ERR/ERL
 if (rt->last_err_line != line_num) {
 rt->last_err_code =
 (int)error_get();
 rt->last_err_line = line_num;
 }

 // Record err_index for CONTINUE
 rt->stack[ei].data.exception
 .err_index = rt->current_index;

 error_clear();
 // Jump to first line of USE handler
 rt->next_index = use_idx + 1;
 break;
 }
 }
 }

 // ON ERROR GOTO handler.
 // (Only reached if no WHEN EXCEPTION frame handled it.)
 if (error_occurred() && rt->on_error_line > 0) {
 double target_line = rt->on_error_line;

 // Save error info for ERL/ERR.
 // If CAUSE EXCEPTION already set last_err_code
 // and last_err_line for this line, preserve
 // the user's exception code.
 if (rt->last_err_line != line_num) {
 rt->last_err_code = (int)error_get();
 rt->last_err_line = line_num;
 }

 rt->on_error_line = 0;
 error_clear();

 vm_jump(rt, target_line, line_num);

 if (rt->next_index < 0) {
 vm_set_state(rt, VM_ERROR);
 break;
 }
 }

 // Check for unhandled error
 if (error_occurred()) {
 vm_set_state(rt, VM_ERROR);
 break;
 }

 // Check if STOP paused execution.
 // If VM is PAUSED (set by STOP handler), save
 // resume position and return to REPL.
 if (vm_get_state(rt) == VM_PAUSED) {
 // Determine resume point
 if (rt->next_index >= 0) {
 rt->resume_index = rt->next_index;
 } else {
 rt->resume_index = rt->current_index + 1;
 }
 return; // Don't set STOPPED
 }

 // Determine next line
 if (rt->next_index >= 0) {
 rt->current_index = rt->next_index;
 } else if (vm_get_state(rt) == VM_RUNNING) {
 rt->current_index++;
 }
 }

 vm_set_state(rt, VM_STOPPED);
    }
}

 // exec_run - Start program execution from the beginning.
void exec_run(RuntimeState *rt)
{
    

    // Reset execution state for fresh run
    runtime_reset(rt);

    // Pre-scan and load external declarations first
    runtime_pre_scan_external_declarations(rt);

    // Collect DATA values before execution begins
    runtime_collect_data(rt);

    // Collect line labels for GOTO/GOSUB label resolution
    runtime_collect_labels(rt);

 // Install OS signal handler (Tier 3)
 g_signal_rt = rt;
 g_signal_pending = 0;
 signal(SIGINT, signal_handler);

 // Pre-scan for SUB/FUNCTION definitions.
 //
 // QBasic-style: resolve all SUB/FUNCTION bodies
 // before the first line executes. This makes
 // DECLARE EXTERNAL and forward references work
 // correctly. Execute each SUB/FUNCTION line which
 // registers the body and skips to END SUB/FUNCTION.
 {
 int idx;
 ProgramStore *pgm = rt->program;
 for (idx = 0; idx < pgm->count; idx++) {
 Lexer cl;
 const char *text = pgm->lines[idx].text;
 double ln = pgm->lines[idx].line_number;
 lexer_init(&cl, text);
 if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT)
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 (cl.current.value.keyword == KW_SUB ||
 cl.current.value.keyword == KW_FUNCTION ||
 cl.current.value.keyword == KW_DEFINE)) {
 printf("DEBUG: exec_run_from prescanned DEFINE on line %.0f\n", ln);
 // Execute this line to register the
 // SUB/FUNCTION definition 
 rt->current_index = idx;
 rt->next_index = -1;
 parser_execute_line(&cl, rt, ln);
 if (error_occurred()) {
 vm_set_state(rt, VM_ERROR);
 return;
 }
 // SUB handler skips to END SUB; advance
 // past it 
 if (rt->next_index > idx)
 idx = rt->next_index - 1;
 }
 }
 // Reset execution state for actual run
 rt->current_index = 0;
 rt->next_index = -1;
 }

 exec_run_from(rt, 0);
}

 // exec_chain_run - Execute after CHAIN (preserves variables).
 //
 // Like exec_run but does NOT call runtime_reset, so variables
 // (A-Z, A$-Z$, named vars, @() array) survive. The caller
 // (pi_parse_chain_cmd) has already reset the scope stack,
 // SUB table, call stack, and execution pointers.
 //
 // We still need to:
 // - Collect DATA values from the new program
 // - Collect labels
 // - Pre-scan SUB/FUNCTION definitions
 // - Install signal handler
 // - Run from index 0
void exec_chain_run(RuntimeState *rt)
{
#ifndef BPP_LITE_BUILD
   if (rt->has_loaded_pcode && rt->loaded_pcode != NULL) {
       PCodeProgram *pcode = (PCodeProgram *)rt->loaded_pcode;
       pcode_free(pcode);
       free(pcode);
       rt->loaded_pcode = NULL;
       rt->has_loaded_pcode = 0;
   }
#endif
    

    // Pre-scan and load external declarations first
    runtime_pre_scan_external_declarations(rt);

    // Collect DATA values from the new program
    runtime_collect_data(rt);

 // Collect line labels
 runtime_collect_labels(rt);

 // Install OS signal handler
 g_signal_rt = rt;
 g_signal_pending = 0;
 signal(SIGINT, signal_handler);

 // Pre-scan for SUB/FUNCTION definitions
 {
 int idx;
 ProgramStore *pgm = rt->program;
 for (idx = 0; idx < pgm->count; idx++) {
 Lexer cl;
 const char *text = pgm->lines[idx].text;
 double ln = pgm->lines[idx].line_number;
 lexer_init(&cl, text);
 if (cl.current.type == TOK_NUMBER || cl.current.type == TOK_FLOAT_LIT)
 lexer_next(&cl);
 if (cl.current.type == TOK_KEYWORD &&
 (cl.current.value.keyword == KW_SUB ||
 cl.current.value.keyword == KW_FUNCTION ||
 cl.current.value.keyword == KW_DEFINE)) {
 rt->current_index = idx;
 rt->next_index = -1;
 parser_execute_line(&cl, rt, ln);
 if (error_occurred()) {
 vm_set_state(rt, VM_ERROR);
 return;
 }
 if (rt->next_index > idx)
 idx = rt->next_index - 1;
 }
 }
 rt->current_index = 0;
 rt->next_index = -1;
 }

 exec_run_from(rt, 0);
}

 // exec_cont - Continue execution from paused state.
 //
 // Resumes from the saved resume_index without resetting state.
 // Returns 0 on success, -1 if not paused.
int exec_cont(RuntimeState *rt)
{
 if (vm_get_state(rt) != VM_PAUSED) {
 printf("Not paused.\n");
 return -1;
 }

 if (rt->resume_index < 0 ||
 rt->resume_index >= rt->program->count) {
 printf("Cannot continue - no resume point.\n");
 vm_set_state(rt, VM_STOPPED);
 return -1;
 }

 exec_run_from(rt, rt->resume_index);
 return 0;
}

 // exec_brun - Compile to bytecode and execute via VM.
 //
 // Milestone 13 bytecode execution path:
 // 1. Reset runtime state
 // 2. Collect DATA values
 // 3. Compile program to PCode bytecode
 // 4. Execute via vm_exec_pcode()
 // 5. Free bytecode
 //
 // The interpreter path (exec_run) is untouched.
void exec_brun(RuntimeState *rt)
{
#ifdef BPP_LITE_BUILD
    (void)rt;
    printf("BRUN: Bytecode execution not supported in Lite build.\n");
#else
 PCodeProgram pcode;
 int result;

 if (!rt->program || rt->program->count == 0) {
 printf("No program to BRUN.\n");
 return;
 }

 // Reset runtime state for fresh run
 runtime_reset(rt);

 // Pre-scan and load external declarations first
 runtime_pre_scan_external_declarations(rt);

 // Collect DATA values before execution
 runtime_collect_data(rt);

 // Compile to bytecode
 printf("Compiling %d lines to bytecode...\n",
 rt->program->count);

 if (pcode_compile(rt->program, &pcode) != 0) {
 printf("BRUN: Compilation failed.\n");
 return;
 }

 printf("Compiled: %d instructions, %d string bytes\n",
 pcode.count,
 pcode.str_used);

 // Execute bytecode
 vm_set_state(rt, VM_RUNNING);
 g_arithmetic_decimal = rt->arithmetic_decimal;
 result = vm_exec_pcode(rt, &pcode);

 if (result != 0 && vm_get_state(rt) != VM_PAUSED) {
 vm_set_state(rt, VM_ERROR);
 }

 // Free bytecode
 pcode_free(&pcode);

  if (vm_get_state(rt) != VM_PAUSED) {
  vm_set_state(rt, VM_STOPPED);
  }
#endif
}

void exec_run_step_cooperative(RuntimeState *rt)
{
    if (rt->direct_mode) {
        Lexer lex;
        g_arithmetic_decimal = rt->arithmetic_decimal;
        if (vm_get_state(rt) != VM_RUNNING || rt->current_index >= rt->program->count || error_occurred()) {
            return;
        }

        ProgramLine *line = &rt->program->lines[rt->current_index];
        double line_num = line->line_number;

        // Initialize lexer
        lexer_init(&lex, line->text);

        // If we yielded, resume from the exact position
        if (rt->yielded) {
            rt->yielded = 0;
            lex.pos = rt->yield_pos;
            lexer_next(&lex); 
        } else {
            // Skip line number
            if (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
                lexer_next(&lex);
            }
            rt->next_index = -1;
        }

        // Run one statement/line
        parser_execute_line(&lex, rt, line_num);

        // Advance PC if no jump and no yield occurred
        if (!rt->yielded) {
            if (rt->next_index == -1) {
                rt->current_index++;
            } else if (rt->next_index >= 0) {
                rt->current_index = rt->next_index;
                rt->next_index = -1;
            }
        }
        return;
    }

#ifndef BPP_LITE_BUILD
    if (vm_get_state(rt) != VM_RUNNING || rt->current_index >= rt->program->count || error_occurred()) {
        return;
    }

    ProgramLine *line = &rt->program->lines[rt->current_index];
    ProgramStore temp_prog;
    ProgramLine temp_line;
    PCodeProgram pcode;
    int result;

    temp_line.line_number = line->line_number;
    temp_line.text = line->text;

    temp_prog.lines = &temp_line;
    temp_prog.count = 1;
    temp_prog.capacity = 1;
    temp_prog.bulk_buffer = NULL;
    temp_prog.bulk_size = 0;

    rt->next_index = -1;

    if (pcode_compile(&temp_prog, &pcode) == 0) {
        g_arithmetic_decimal = rt->arithmetic_decimal;
        result = vm_exec_pcode(rt, &pcode);
        pcode_free(&pcode);
        if (result != 0 && vm_get_state(rt) != VM_PAUSED) {
            vm_set_state(rt, VM_ERROR);
        }
    } else {
        Lexer lex;
        lexer_init(&lex, line->text);
        if (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
            lexer_next(&lex);
        }
        parser_execute_line(&lex, rt, line->line_number);
    }

    if (!rt->yielded) {
        if (rt->next_index == -1) {
            rt->current_index++;
        } else if (rt->next_index >= 0) {
            rt->current_index = rt->next_index;
            rt->next_index = -1;
        }
    }
#endif
}

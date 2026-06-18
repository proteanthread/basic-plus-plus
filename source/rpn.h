 // ---
 // BASIC++ Interpreter - rpn.h
 // ---
 //
 // RPN (Reverse Polish Notation) Calculator Mode.
 //
 // Provides a Forth-like stack-based calculator that can be
 // activated via OPTION RPN. When active, the REPL evaluates
 // input as RPN expressions instead of BASIC statements.
 //
 // Stack operations:
 //   DUP    - Duplicate top of stack
 //   DROP   - Remove top of stack
 //   SWAP   - Swap top two elements
 //   OVER   - Copy second element to top
 //   ROT    - Rotate third element to top
 //   DEPTH  - Push stack depth
 //   .      - Pop and print top
 //   .S     - Print entire stack (non-destructive)
 //   CLEAR  - Clear entire stack
 //
 // Arithmetic: + - * / MOD
 // Comparison: = < > <> AND OR NOT
 //
 // Entering a number pushes it onto the stack.
 // OPTION RPN OFF returns to normal BASIC mode.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_RPN_H
#define BASICPP_RPN_H

#define RPN_STACK_SIZE 256

typedef struct RpnState {
 double stack[RPN_STACK_SIZE];
 int top; // index of next free slot (0 = empty)
 int active; // 1 = RPN mode is on
} RpnState;

 // rpn_init - Initialize RPN state.
 // Sets top to 0, active to 0.
void rpn_init(RpnState *s);

 // rpn_eval_line - Evaluate a line of RPN input.
 //
 // Tokenizes the line by whitespace and processes each
 // token as a number (push) or operator (execute).
 // Returns 1 if the line was processed, 0 on error.
int rpn_eval_line(RpnState *s, const char *line);

 // rpn_is_active - Returns 1 if RPN mode is on.
int rpn_is_active(RpnState *s);

 // rpn_set_active - Enable or disable RPN mode.
void rpn_set_active(RpnState *s, int on);

#endif // BASICPP_RPN_H

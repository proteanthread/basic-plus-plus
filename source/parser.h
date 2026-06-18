 // ---
 // BASIC++ Interpreter - parser.h
 // ---
 //
 // Parser interface.
 //
 // PURPOSE:
 // Parses BASIC statements and expressions from the token stream
 // and directly dispatches execution. This is a "parse-and-execute"
 // architecture matching the original Palo Alto Tiny BASIC design.
 //
 // WHY DIRECT DISPATCH (NO AST):
 // 1. Faithful to the original PATB interpreter architecture.
 // 2. Simpler implementation for 
 // 3. Lower memory usage (no AST node allocation).
 // 4. An AST layer can be added later for the compiler subsystem
 // without changing the parser's external interface.
 //
 // PARSING ARCHITECTURE:
 // The parser is structured in four layers:
 //
 // 1. Statement parser - identifies the statement keyword and
 // dispatches to the appropriate handler.
 //
 // 2. Expression parser - evaluates additive expressions
 // (term +/- term).
 //
 // 3. Term parser - evaluates multiplicative expressions
 // (factor * / factor).
 //
 // 4. Factor parser - evaluates atoms (numbers, variables,
 // parenthesized expressions, function calls).
 //
 // Recursion depth is bounded to 3 levels (expression -> term ->
 // factor -> expression via parentheses). This is shallow enough
 // to be safe on all target platforms including FreeDOS.
 //
 // HOW TO EXTEND:
 // To add a new statement: add a case to parse_statement() in
 // parser.c, implement a parse_xxx() function, and add the
 // corresponding exec_xxx() function in exec.c.
 //
 // To add a new operator or function: extend parse_factor() or
 // parse_expression() as appropriate.
 //
 // ---

#ifndef BASICPP_PARSER_H
#define BASICPP_PARSER_H

#include "lexer.h"
#include "runtime.h"

// --- Parser Functions ---

 // parser_execute_line - Parse and execute a complete BASIC line.
 //
 // Handles one or more statements separated by the dialect's
 // statement separator (';' for PATB). Stops on end-of-line,
 // error, or a flow-control statement that transfers execution
 // (GOTO, GOSUB, RETURN, END, STOP).
 //
 // Parameters:
 // lex - lexer positioned at the first token of the line
 // (after the line number, if any)
 // rt - runtime state
 // line_num - BASIC line number (for error messages), or 0 for
 // immediate mode
 //
 // The lexer should already be initialized on the line text.
 // For stored program lines, the caller should advance past the
 // line number token before calling this function.
void parser_execute_line(Lexer *lex, RuntimeState *rt, int line_num);

 // parse_expression - Parse and evaluate an integer expression.
 //
 // Evaluates an expression following standard BASIC precedence:
 // expression = [+|-] term ((+|-) term)*
 //
 // Returns the computed integer value. On error, raises ERR_WHAT
 // and returns 0.
 //
 // Parameters:
 // lex - lexer positioned at the start of the expression
 // rt - runtime state (for variable lookups)
 // line_num - current line number (for error context)
long parse_expression(Lexer *lex, RuntimeState *rt, int line_num);

 // parse_expression_bval - Parse and evaluate an expression as BValue.
 //
 // Returns a BValue (integer, float, or string) rather than
 // a raw long. This is the primary expression evaluator for dialects
 // that support floats and strings.
BValue parse_expression_bval(Lexer *lex, RuntimeState *rt, int line_num);

#endif // BASICPP_PARSER_H

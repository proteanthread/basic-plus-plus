/*
 * ---
 * BASIC++ Compiler - compiler.c
 * ---
 *
 * Compiler driver implementation.
 *
 * DESIGN RATIONALE:
 * The compiler orchestrates a three-step pipeline:
 * 1. Tokenize each stored program line (reusing the Lexer).
 * 2. Build an AST for each line (via ast_build_line).
 * 3. Pass the AST array to codegen_emit to produce C output.
 *
 * The AST is transient - allocated during compilation and freed
 * after code generation. The interpreter's runtime state is not
 * used; compilation is a pure source-to-source translation.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "ast.h"
#include "codegen.h"
#include "lexer.h"
#include "errors.h"

/*
 * compiler_compile - Main compilation entry point.
 *
 * Reads all stored program lines, builds ASTs, generates C code.
 */
int compiler_compile(ProgramStore *program, const char *filename, const char *target)
{
 FILE *out;
 AstLine *ast_lines;
 int i;
 int success = 1;

 /* Validate */
 if (!program || program->count == 0) {
 printf("No program to compile.\n");
 return -1;
 }

 if (!filename || filename[0] == '\0') {
 printf("No output filename specified.\n");
 return -1;
 }

 /* Open output file */
 out = fopen(filename, "w");
 if (!out) {
 printf("Cannot open '%s' for writing.\n", filename);
 return -1;
 }

 /* Allocate AST line array */
 ast_lines = (AstLine *)malloc(
 sizeof(AstLine) * (size_t)program->count);
 if (!ast_lines) {
 printf("Out of memory for compilation.\n");
 fclose(out);
 return -1;
 }
 memset(ast_lines, 0, sizeof(AstLine) * (size_t)program->count);

 printf("Compiling %d lines to '%s'...\n", program->count, filename);

 /* Pass 1: Build AST for each line */
 for (i = 0; i < program->count; i++) {
 Lexer lex;
 ProgramLine *pl = &program->lines[i];

 ast_lines[i].line_number = pl->line_number;

 /* Initialize lexer on the line text */
 lexer_init(&lex, pl->text);

 /* Skip the line number token */
 if (lex.current.type == TOK_NUMBER) {
 lexer_next(&lex);
 }

 /* Clear error state for each line */
 error_clear();

 /* Build AST */
 ast_lines[i].stmts = ast_build_line(&lex, pl->line_number);

 if (error_occurred()) {
 printf("Compile error at line %d\n", pl->line_number);
 success = 0;
 error_clear();
 /* Continue to free everything cleanly */
 }
 }

 /* Pass 2: Generate code */
 if (success) {
 error_clear();
 if (codegen_emit(out, ast_lines, program->count, program) != 0) {
 printf("Code generation failed.\n");
 success = 0;
 }
 }

 /* Cleanup */
 for (i = 0; i < program->count; i++) {
 ast_free_line(ast_lines[i].stmts);
 }
 free(ast_lines);
 fclose(out);

 if (success) {
 printf("Compilation successful: %s\n", filename);
 return 0;
 } else {
 /* Remove partial output on failure */
 remove(filename);
 return -1;
 }
}

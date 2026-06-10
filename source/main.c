/*
 * ---
 * BASIC++ Interpreter - main.c
 * ---
 *
 * Entry point, REPL (Read-Eval-Print Loop), and bootstrap.
 *
 * PURPOSE:
 * This module provides:
 * 1. The main() entry point.
 * 2. Memory subsystem initialization and shutdown.
 * 3. The interactive REPL loop (classic BASIC prompt style).
 * 4. Input classification (line storage vs. immediate execution).
 *
 * REPL BEHAVIOR (Palo Alto Tiny BASIC compatible):
 * - On startup, prints a banner with version and dialect info.
 * - Prints "READY" and a prompt ("> ").
 * - Reads a line of input.
 * - If the line starts with a digit -> it's a program line:
 * - If only a line number -> delete that line.
 * - Otherwise -> store the line (insert or replace).
 * - If the line starts with a letter or keyword -> immediate mode:
 * - Tokenize and execute immediately.
 * - Results are printed; errors return to prompt.
 * - After any error, prints "READY" and returns to prompt.
 * - On EOF (Ctrl-Z on Windows, Ctrl-D on Unix) -> exit.
 *
 * DESIGN RATIONALE:
 * The REPL is kept minimal. It classifies input and delegates
 * to the appropriate subsystem. No parsing logic lives here
 * (that's in parser.c). No execution logic lives here (that's
 * in exec.c). Main.c is glue code.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "memory.h"
#include "lexer.h"
#include "parser.h"
#include "exec.h"
#include "runtime.h"
#include "dialect.h"
#include "errors.h"
#include "vdev.h"
#include "funcreg.h"
#include "builtins.h"
#include "fileio.h"
#include "vm.h"
#include "module.h"
#include "mod_stdlib.h"
#include "mod_usb.h"
#include "mod_fujinet.h"
#include "security.h"
#include "platform.h"
#include "gfxbuf.h"
#include "scope.h"
#include "keyword_props.h"
#include "override.h"

/* --- Input Classification ---
 */

/*
 * parse_line_number - Extract a line number from the start of input.
 *
 * If the input starts with digits, parses the line number and
 * returns it. Sets *end_pos to the position after the line number
 * and any trailing whitespace.
 *
 * Returns the line number (>= 1), or 0 if the input does not
 * start with a digit.
 */
static int parse_line_number(const char *input, int *end_pos)
{
 int pos = 0;
 int num = 0;

 /* Skip leading whitespace */
 while (input[pos] == ' ' || input[pos] == '\t') {
 pos++;
 }

 /* Check for digit */
 if (!isdigit((unsigned char)input[pos])) {
 *end_pos = pos;
 return 0;
 }

 /* Parse the number */
 while (isdigit((unsigned char)input[pos])) {
 num = num * 10 + (input[pos] - '0');
 pos++;
 }

 *end_pos = pos;
 return num;
}

/*
 * is_blank_after - Check if the rest of the line is blank.
 *
 * Returns 1 if input[pos..] contains only whitespace, 0 otherwise.
 * Used to detect "line number only" input (delete line).
 */
static int is_blank_after(const char *input, int pos)
{
 while (input[pos] != '\0') {
 if (input[pos] != ' ' && input[pos] != '\t') {
 return 0;
 }
 pos++;
 }
 return 1;
}

/*
 * strip_newline - Remove trailing \n and \r from a string.
 *
 * Modifies the string in place.
 */
static void strip_newline(char *str)
{
 int len = (int)strlen(str);
 while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
 str[--len] = '\0';
 }
}

/* --- Main Entry Point ---
 */

int main(void)
{
 MemorySystem memory;
 RuntimeState runtime;
 char input_buf[INPUT_BUFFER_SIZE];

 /* ----- Print startup banner ----- */
 printf("\n");
 printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
 printf("%s\n", BASICPP_COPYRIGHT);
 printf("%s\n", __DATE__);
 printf("\n");

 /* ----- Initialize memory subsystem ----- */
 if (mem_init(&memory) != 0) {
 printf("SORRY. Cannot allocate memory.\n");
 return 1;
 }

 /* Initialize platform detection */
 platform_init();

 /* Register all compiled-in dialects */
 dialect_register_all();

 /* ----- Initialize dialect system ----- */
 dialect_init(DIALECT_TINY_BASIC);

 /* Initialize virtual device system */
 vdev_init();

 /* Initialize function registry */
 funcreg_init();

 /* Initialize security system */
 security_init(SEC_OPEN);

 /* Initialize module system */
 module_system_init();
 mod_stdlib_register();
 mod_usb_register(); /* USB devices (user activates) */
 mod_fujinet_register(); /* FujiNet N:/FUJI:/CLOCK: devices */
 module_activate("STDLIB", NULL);

 /* Apply dialect-specific overrides */
 dialect_apply();

 /* Initialize file channels */
 fileio_channels_init();

 /* Initialize VM dispatch table */
 vm_init();

 /* Initialize SCOPE system */
 scope_init();

 /* Initialize keyword property system */
 keyword_props_init();

 /* Initialize OVERRIDE system */
 override_init();

 /* ----- Initialize graphics framebuffer ----- */
 gfxbuf_init();

 /* ----- Initialize runtime state ----- */
 runtime_init(&runtime, &memory.program, &memory);

 /* ----- REPL loop ----- */
 printf("%s\n", BASICPP_READY);

 for (;;) {
 int line_num;
 int end_pos;

 /* --- AUTO mode prompt --- */
 if (runtime.auto_line > 0) {
 /*
 * AUTO mode is active. Print the line number
 * as the prompt. If the user enters text, it
 * becomes the content of that line. If the
 * user enters an empty line or '.', AUTO
 * mode is cancelled.
 */
 printf("%d ", runtime.auto_line);
 fflush(stdout);

 if (fgets(input_buf, INPUT_BUFFER_SIZE,
 stdin) == NULL) {
 /* EOF - exit AUTO and interpreter */
 printf("\n");
 break;
 }
 strip_newline(input_buf);

 /* Cancel on empty line or '.' */
 if (input_buf[0] == '\0' ||
 (input_buf[0] == '.' &&
 input_buf[1] == '\0')) {
 runtime.auto_line = 0;
 runtime.auto_step = 0;
 printf("%s\n", BASICPP_READY);
 continue;
 }

 /* Build full line: "linenum content" */
 {
 char full_line[INPUT_BUFFER_SIZE + 16];
 sprintf(full_line, "%d %s",
 runtime.auto_line, input_buf);
 program_insert(&memory.program,
 runtime.auto_line, full_line);
 }

 /* Advance to next line */
 runtime.auto_line += runtime.auto_step;
 if (runtime.auto_line > LINE_NUMBER_MAX) {
 runtime.auto_line = 0;
 runtime.auto_step = 0;
 printf("Line number overflow.\n");
 printf("%s\n", BASICPP_READY);
 }
 continue;
 }

 /* Print prompt */
 printf("%s", BASICPP_PROMPT);
 fflush(stdout);

 /* Read input line */
 if (fgets(input_buf, INPUT_BUFFER_SIZE, stdin) == NULL) {
 /* EOF - exit gracefully */
 printf("\n");
 break;
 }

 /* Strip trailing newline */
 strip_newline(input_buf);

 /* Skip empty lines */
 if (input_buf[0] == '\0') {
 continue;
 }

 /* Clear any previous error state */
 error_clear();

 /* Reset scratch pool for this input cycle */
 mem_pool_reset(&memory.scratch);

 /* ----- Classify input ----- */
 line_num = parse_line_number(input_buf, &end_pos);

 if (line_num > 0) {
 /*
 * Input starts with a line number.
 *
 * If the rest of the line is blank, this is a
 * delete command (entering just "10" deletes line 10).
 *
 * Otherwise, store the entire line (including the
 * line number prefix) in the program store.
 */
 if (line_num > LINE_NUMBER_MAX) {
 error_raise(ERR_HOW, 0);
 } else if (is_blank_after(input_buf, end_pos)) {
 /* Delete line */
 program_delete(&memory.program, line_num);
 } else {
 /* Store line */
 program_insert(&memory.program, line_num, input_buf);
 }
 } else {
 /*
 * Immediate mode - tokenize and execute.
 *
 * The input is a command or statement to execute
 * right now (e.g., "PRINT 2+3" or "RUN" or "LIST").
 */
 Lexer lex;

 lexer_init(&lex, input_buf);
 parser_execute_line(&lex, &runtime, 0);
 }

 /* After any error, print READY again */
 if (error_occurred()) {
 printf("%s\n", BASICPP_READY);
 }
 }

 /* ----- Shutdown ----- */
 mem_shutdown(&memory);

 return 0;
}

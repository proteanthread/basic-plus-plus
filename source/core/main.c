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
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
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
#include "io/vdev_net.h"
#include "vm.h"
#include "module.h"
#include "error_registry.h"
#include "mod_stdlib.h"
#ifndef BPP_FREEDOS
#include "mod_usb.h"
#include "mod_fujinet.h"
#include "mod_upnp.h"
#endif
#include "security.h"
#include "platform.h"
#include "gfxbuf.h"
#include "scope.h"
#include "keyword_props.h"
#include "override.h"
#include "config_file.h"

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

/* --- Usage / Help ---
 */
static void print_usage(const char *prog)
{
    printf("Usage: %s [options] [program.bas]\n\n", prog);
    printf("Execution Modes:\n");
    printf("  <filename>       Load, run, and remain in basic++\n");
    printf("  --run <file>     Run specified program then exit\n");
    printf("  --load <file>    Load program and remain in basic++\n");
    printf("  --list <file>    Load, list program, then exit\n");
    printf("  --dry-run <file> Load in debug/step-by-step mode\n");
    printf("  --edit           Start in screen-editor mode\n");
    printf("  -c \"CMD\"         Execute a command and exit\n");
    printf("\nInfo:\n");
    printf("  -h, --help       Print this help and exit\n");
    printf("  -v, --version    Print version and exit\n");
    printf("  --about          Describe version and what's new\n");
    printf("  --license        Display full MIT license text\n");
    printf("\nEnvironment & Memory:\n");
    printf("  -d DIALECT       Set default dialect (GWBS, QBAS, PATB, etc.)\n");
    printf("  -s LEVEL         Set security level (OPEN, STANDARD, RESTRICTED)\n");
    printf("  -S               Enable strict dialect mode\n");
    printf("  -f FILE          Use a specific configuration file\n");
    printf("  -q               Quiet mode (suppress startup banner)\n");
    printf("  --debug          Enable verbose error mode and diagnostics\n");
    printf("  --com <n>        RS-232 receive buffer size (bytes)\n");
    printf("  --files <n>      Max open files (Default 3)\n");
    printf("  --mem <m,n>      Set memory limits (high mem, workspace)\n");
    printf("  --records <n>    Max record length for RANDOM files (Default 128)\n");
    printf("  --block          Preallocate blocks/buffers\n");
    printf("  --dbl            Enable double-precision math\n");
    printf("  --dynamic        Allow dynamic arrays and >64KB structures\n");
    printf("\nVideo & Display:\n");
    printf("  --no-color       Monochrome display mode\n");
    printf("  --cga            Enable faster CGA screen updates\n");
    printf("  --hi-res         Use maximum display lines\n");
    printf("  --no-high        Disable high-intensity colors\n");
    printf("\nCompatibility & Interop:\n");
    printf("  --mbf            Use Microsoft Binary Format (MBF)\n");
    printf("  --kbd            Full keyboard re-mapping\n");
    printf("  --break <on|off> Enable/disable Ctrl+Break handling\n");
    printf("  --cmd <string>   Pass string to COMMAND$ (must be last)\n");
    printf("  --lib <file>     Load external library (.dll / .so)\n");
    printf("  --func <name>    Load external function\n");
    printf("  --mod <name>     Load external module\n");
    printf("  --ext            Enable extended features\n");
    printf("  <file, >file     Manual file redirection\n");
    printf("\nConfig file: %s (searched in current dir, then home)\n",
           config_file_get_name(prog));
    printf("Priority: config file < CLI switches < runtime commands\n");
}


/* --- Main Entry Point ---
 */

int main(int argc, char *argv[])
{
 MemorySystem memory;
 static RuntimeState runtime;
 char input_buf[INPUT_BUFFER_SIZE];

    /* --- CLI argument storage --- */
    const char *cli_dialect = NULL;
    const char *cli_security = NULL;
    int cli_strict = -1;    /* -1 = unset */
    int cli_quiet = 0;
    const char *cli_run_file = NULL;
    const char *cli_load_file = NULL;
    const char *cli_list_file = NULL;
    const char *cli_dry_run_file = NULL;
    int cli_edit = 0;
    int cli_debug = 0;
    const char *cli_command = NULL;
    const char *cli_config_file = NULL;
    const char *cli_program = NULL;
    const char *cli_cmd_str = NULL;
    const char *cli_lib = NULL;
    const char *cli_mod = NULL;
    const char *cli_func = NULL;
    
    /* Redirection storage */
    const char *cli_redirect_in = NULL;
    const char *cli_redirect_out = NULL;
    const char *cli_redirect_append = NULL;

    /* --- Config file --- */
    ConfigFile cfg;

    /* --- Effective settings --- */
    DialectId eff_dialect = BASICPP_DEFAULT_DIALECT;
    SecLevel eff_security = SEC_OPEN;
    int eff_strict = 0;
    int eff_quiet = 0;

    int i;

    /* ----- Parse command-line arguments ----- */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
                return 0;
            }
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                return 0;
            }
            if (strcmp(argv[i], "--about") == 0) {
                printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
                printf("A hybrid AST/P-Code interpreter for classic BASIC dialects.\n");
                printf("Milestone 5 includes Config and CLI Supercharging.\n");
                return 0;
            }
            if (strcmp(argv[i], "--license") == 0) {
                printf("MIT License\n(See LICENSE file for full text)\n");
                return 0;
            }
            if (strcmp(argv[i], "-q") == 0) {
                cli_quiet = 1;
                continue;
            }
            if (strcmp(argv[i], "--debug") == 0) {
                cli_debug = 1;
                continue;
            }
            if (strcmp(argv[i], "--edit") == 0) {
                cli_edit = 1;
                continue;
            }
            if (strcmp(argv[i], "-S") == 0) {
                cli_strict = 1;
                continue;
            }
            if (strcmp(argv[i], "--run") == 0 || strcmp(argv[i], "-r") == 0) {
                if (i + 1 < argc) cli_run_file = argv[++i];
                else { printf("Error: %s requires a filename\n", argv[i-1]); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--load") == 0) {
                if (i + 1 < argc) cli_load_file = argv[++i];
                else { printf("Error: --load requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--list") == 0) {
                if (i + 1 < argc) cli_list_file = argv[++i];
                else { printf("Error: --list requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--dry-run") == 0) {
                if (i + 1 < argc) cli_dry_run_file = argv[++i];
                else { printf("Error: --dry-run requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-d") == 0) {
                if (i + 1 < argc) cli_dialect = argv[++i];
                else { printf("Error: -d requires a dialect name\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-s") == 0) {
                if (i + 1 < argc) cli_security = argv[++i];
                else { printf("Error: -s requires a security level\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-c") == 0) {
                if (i + 1 < argc) cli_command = argv[++i];
                else { printf("Error: -c requires a command string\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-f") == 0) {
                if (i + 1 < argc) cli_config_file = argv[++i];
                else { printf("Error: -f requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--cmd") == 0) {
                if (i + 1 < argc) {
                    /* Join remaining arguments for COMMAND$ */
                    cli_cmd_str = argv[++i];
                    break;
                }
            }
            /* Silently accept/ignore environment & hardware switches for now */
            if (strcmp(argv[i], "--com") == 0 || strcmp(argv[i], "--files") == 0 ||
                strcmp(argv[i], "--mem") == 0 || strcmp(argv[i], "--records") == 0 ||
                strcmp(argv[i], "--lib") == 0 || strcmp(argv[i], "--func") == 0 ||
                strcmp(argv[i], "--mod") == 0 || strcmp(argv[i], "--break") == 0) {
                if (i + 1 < argc) i++;
                continue;
            }
            if (strcmp(argv[i], "--block") == 0 || strcmp(argv[i], "--dbl") == 0 ||
                strcmp(argv[i], "--no-color") == 0 || strcmp(argv[i], "--cga") == 0 ||
                strcmp(argv[i], "--hi-res") == 0 || strcmp(argv[i], "--no-high") == 0 ||
                strcmp(argv[i], "--mbf") == 0 || strcmp(argv[i], "--dynamic") == 0 ||
                strcmp(argv[i], "--ext") == 0 || strcmp(argv[i], "--kbd") == 0) {
                continue;
            }
            
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else if (argv[i][0] == '<') {
            cli_redirect_in = argv[i] + 1;
            if (*cli_redirect_in == '\0' && i + 1 < argc) cli_redirect_in = argv[++i];
        } else if (argv[i][0] == '>') {
            if (argv[i][1] == '>') {
                cli_redirect_append = argv[i] + 2;
                if (*cli_redirect_append == '\0' && i + 1 < argc) cli_redirect_append = argv[++i];
            } else {
                cli_redirect_out = argv[i] + 1;
                if (*cli_redirect_out == '\0' && i + 1 < argc) cli_redirect_out = argv[++i];
            }
        } else {
            /* Positional argument: treat as program file */
            cli_program = argv[i];
        }
    }

    /* Process manual file redirection */
    if (cli_redirect_in != NULL) freopen(cli_redirect_in, "r", stdin);
    if (cli_redirect_out != NULL) freopen(cli_redirect_out, "w", stdout);
    if (cli_redirect_append != NULL) freopen(cli_redirect_append, "a", stdout);

 /* ----- Early init (needed before config lookups) ----- */
 platform_init();
 dialect_register_all();
 keyword_registry_init();

 /* ----- Load config file (lowest priority) ----- */
 if (cli_config_file != NULL) {
  if (config_file_load_path(&cfg, cli_config_file) != 0) {
   printf("Error: cannot open config file '%s'\n",
    cli_config_file);
   return 1;
  }
 } else {
  config_file_load(&cfg, argv[0]);
 }

 /* ----- Apply settings: config file first, then CLI overrides ----- */

 /* Dialect */
 if (cfg.found && cfg.dialect[0] != '\0') {
  int did = dialect_find_by_name(cfg.dialect);
  if (did >= 0) eff_dialect = (DialectId)did;
 }
 if (cli_dialect != NULL) {
  int did = dialect_find_by_name(cli_dialect);
  if (did >= 0) {
   eff_dialect = (DialectId)did;
  } else {
   printf("Warning: unknown dialect '%s', using default\n",
    cli_dialect);
  }
 }

 /* Default security depends on context */
 eff_security = SEC_OPEN;
 if (cli_run_file != NULL) {
  eff_security = SEC_STANDARD;
 }
 
 /* Piped execution defaults to RESTRICTED */
#ifdef _WIN32
 if (!_isatty(_fileno(stdin))) {
  eff_security = SEC_RESTRICTED;
 }
#else
 if (!isatty(fileno(stdin))) {
  eff_security = SEC_RESTRICTED;
 }
#endif

 /* Dialect defaults clamp */
 {
  SecLevel dialect_sec = (SecLevel)dialect_default_security(eff_dialect);
  if (dialect_sec > eff_security) {
   eff_security = dialect_sec;
  }
 }
 
 /* Security */
 if (cfg.found && cfg.security[0] != '\0') {
  if (strcmp(cfg.security, "OPEN") == 0)
   eff_security = SEC_OPEN;
  else if (strcmp(cfg.security, "STANDARD") == 0)
   eff_security = SEC_STANDARD;
  else if (strcmp(cfg.security, "RESTRICTED") == 0)
   eff_security = SEC_RESTRICTED;
 }
 if (cli_security != NULL) {
  char sec_upper[16];
  int j;
  int len = (int)strlen(cli_security);
  if (len > 15) len = 15;
  for (j = 0; j < len; j++) {
   char c = cli_security[j];
   if (c >= 'a' && c <= 'z') c = (char)(c - 32);
   sec_upper[j] = c;
  }
  sec_upper[len] = '\0';
  if (strcmp(sec_upper, "OPEN") == 0)
   eff_security = SEC_OPEN;
  else if (strcmp(sec_upper, "STANDARD") == 0)
   eff_security = SEC_STANDARD;
  else if (strcmp(sec_upper, "RESTRICTED") == 0)
   eff_security = SEC_RESTRICTED;
  else
   printf("Warning: unknown security level '%s'\n",
    cli_security);
 }

 /* Strict */
 if (cfg.found && cfg.strict >= 0)
  eff_strict = cfg.strict;
 if (cli_strict >= 0)
  eff_strict = cli_strict;

 /* Quiet */
 if (cfg.found && cfg.quiet >= 0)
  eff_quiet = cfg.quiet;
 if (cli_quiet)
  eff_quiet = 1;

 /* Run file from positional arg */
 if (cli_program != NULL && cli_run_file == NULL)
  cli_run_file = cli_program;

 /* ----- Print startup banner (unless quiet) ----- */
 if (!eff_quiet) {
  printf("\n");
  printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
  printf("%s\n", BASICPP_COPYRIGHT);
  printf("%s\n", __DATE__);
  if (cfg.found) {
   printf("Config: %s\n", cfg.filepath);
  }
  printf("\n");
 }

 /* ----- Initialize memory subsystem ----- */
 if (mem_init(&memory) != 0) {
  printf("SORRY. Cannot allocate memory.\n");
  return 1;
 }

 /* ----- Initialize dialect system (from effective settings) ----- */
 dialect_init(eff_dialect);

 /* Initialize virtual device system */
 vdev_net_init();
 vdev_init();

 /* Initialize function registry */
 funcreg_init();
 error_registry_init();

 /* Initialize security system (from effective settings) */
 security_init(eff_security);

 /* Initialize module system */
 module_system_init();
 mod_stdlib_register();
#ifndef BPP_FREEDOS
 mod_usb_register(); /* USB devices (user activates) */
 mod_fujinet_register(); /* FujiNet N:/FUJI:/CLOCK: devices */
 mod_upnp_register();    /* UPnP/SSDP network discovery */
#endif
 module_activate("STDLIB", NULL);

 /* Load external modules from CLI */
 if (cli_lib != NULL) module_load_dynamic(cli_lib);
 if (cli_mod != NULL) module_load_dynamic(cli_mod);
 if (cli_func != NULL) module_load_dynamic(cli_func);

 /* Apply dialect-specific overrides */
 dialect_apply();

 /* Apply strict mode if configured */
 if (eff_strict) {
  dialect_set_strict(1);
 }

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

 /* ----- Handle -c (batch command) mode ----- */
 if (cli_command != NULL) {
  Lexer lex;
  error_clear();
  mem_pool_reset(&memory.scratch);
  lexer_init(&lex, cli_command);
  parser_execute_line(&lex, &runtime, 0);
  fflush(stdout);
  mem_shutdown(&memory);
  return error_occurred() ? 1 : 0;
 }

 /* ----- Handle -r (run file) mode ----- */
 if (cli_run_file != NULL) {
  char load_cmd[INPUT_BUFFER_SIZE];
  Lexer lex;

  /* Build LOAD command */
  sprintf(load_cmd, "LOAD \"%s\"", cli_run_file);
  error_clear();
  mem_pool_reset(&memory.scratch);
  lexer_init(&lex, load_cmd);
  parser_execute_line(&lex, &runtime, 0);

  if (!error_occurred()) {
   /* RUN the loaded program */
   error_clear();
   mem_pool_reset(&memory.scratch);
   lexer_init(&lex, "RUN");
   parser_execute_line(&lex, &runtime, 0);
  }
 }

 /* ----- Handle --edit (screen editor mode) ----- */
 if (cli_edit) {
  printf("Starting BASIC++ Editor (Memo Pad Mode)\n");
  printf("Press Ctrl+Z (Windows) or Ctrl+D (Unix) to exit.\n");
  for (;;) {
   if (fgets(input_buf, INPUT_BUFFER_SIZE, stdin) == NULL) {
    break; /* EOF */
   }
   /* In a true full-screen mode, this would hook into vdev_display.
      For now, it acts as a simple text buffer that doesn't scroll
      beyond screen limits (simulated). */
  }
  printf("\nExiting editor.\n");
  mem_shutdown(&memory);
  return 0;
 }

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
 lexer_normalize_line(full_line);
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
 lexer_normalize_line(input_buf);
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

 if (security_check(SECOP_EVAL, 0) == 0) {
  parser_execute_line(&lex, &runtime, 0);
 }
 }

 /* After any error, print READY again */
 if (error_occurred()) {
 printf("%s\n", BASICPP_READY);
 }
 }

 /* ----- Shutdown ----- */
 vdev_net_cleanup();
 mem_shutdown(&memory);

#ifdef _WIN32
 /* Prevent console window from closing instantly when launched from Windows Explorer */
 if (cli_run_file == NULL && cli_command == NULL && !cli_edit) {
  printf("Interpreter exited.\nPress Enter to exit...");
  fflush(stdout);
  getchar();
 }
#endif

 return 0;
}

/*
 * =============================================================================
 * FILENAME:    ib.c
 * VERSION:     5.0
 * AUTHOR:      BASIC++ Community (Jeff)
 * DESCRIPTION: BASIC++ (IB) Interpreter
 *
 * This is a minimal, portable BASIC interpreter written in a single C file.
 * It is designed as a framework, prioritizing memory optimization and clarity,
 * with speed as a secondary goal. It is intended to be easily expandable.
 * =============================================================================
 *
 *
 * =============================================================================
 * LICENSE
 * =============================================================================
 *
 * Modified MIT License
 *
 * Copyright (c) 2025 Jeff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, but not sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * THIS SOFTWARE IS NOT TO BE SOLD.
 *
 * =============================================================================
 * DEVELOPMENT PHILOSOPHY (UNIX PHILOSOPHY)
 * =============================================================================
 *
 * 1. Do one thing and do it well:
 * This interpreter executes a minimal, 8-bit Integer BASIC. It does not
 * manage a GUI, edit files (beyond SAVE/LOAD), or perform complex OS tasks.
 *
 * 2. Write programs that work together:
 * This interpreter reads/writes plain text files (.bas, lprint.out),
 * allowing it to be scripted or have its output piped to other tools.
 *
 * 3. Handle plain text streams:
 * The core I/O (PRINT, INPUT, SAVE, LOAD) is text-based, adhering
 * to this principle.
 *
 * 4. Modularity:
 * While this is a single file for portability, the design (stubs for
 * $IMPORT, etc.) is intended to be refactored into a modular architecture.
 *
 * 5. Clarity over cleverness:
 * The code prioritizes readability. For example, it uses a simple
 * left-to-right parser and a straightforward GOSUB stack.
 *
 * =============================================================================
 * HOW TO COMPILE (POSIX / GCC)
 * =============================================================================
 *
 * This code is POSIX-compliant C and should compile on any standard
 * Linux distribution (Debian, Fedora, etc.) or FreeDOS (using DJGPP)
 * with GCC or an equivalent C compiler.
 *
 * 1.  For Portability and Size (Recommended):
 * This flag (-Os) optimizes for the smallest possible executable size,
 * which is ideal for portability and resource-constrained systems.
 *
 * gcc -Wall -Os -o ib ib.c
 *
 * 2.  For Speed (Performance):
 * This flag (-O2) enables a suite of optimizations for execution speed,
 * potentially at the cost of a larger binary.
 *
 * gcc -Wall -O2 -o ib ib.c
 *
 * 3.  For Debugging (Development):
 * This flag (-g) includes debugging symbols in the executable,
 * allowing you to use a debugger like GDB.
 *
 * gcc -Wall -g -o ib ib.c
 *
 * =============================================================================
 *
 * MEMORY LAYOUT:
 *
 * The interpreter's "user memory" is defined by static, fixed-size
 * arrays. The total allocatable memory (with v5.0 defaults) is:
 *
 * | Memory Area       | Constant      | Size         | Calculation (assuming 4-byte int)     | Total Size   |
 * | :---------------- | :------------ | :----------- | :------------------------------------ | :----------- |
 * | Program Storage   | MAX_LINES     | 500 lines    | 500 * (127 chars + 4 bytes for line#) | 64.0 KB      |
 * | Variable Storage  | NUM_VARIABLES | 26 vars      | 26 * 1 byte (signed char)             | 26 bytes     |
 * | GOSUB Stack       | STACK_SIZE    | 64 levels    | 64 * 4 bytes (int)                    | 256 bytes    |
 * | **Total**         |               |              |                                       | **~64.2 KB** |
 *
 * The "xx kbytes Free" message at startup only reports the main
 * "Program Storage" (65,500 bytes / 1024 = 63 KB, via integer division).
 *
 * --- HOW TO ADJUST MEMORY ---
 *
 * To change the memory limits, you must edit the #define constants
 * in the "--- Constants ---" section below:
 *
 * - To increase/decrease program memory, change MAX_LINES or MAX_LINE_LEN.
 * - To increase/decrease GOSUB depth, change STACK_SIZE.
 * - NUM_VARIABLES is fixed at 26 (A-Z) and should not be changed
 * without modifying the variable storage logic.
 *
 * After changing these values, you MUST re-compile the interpreter.
 *
 * =============================================================================
 *
 * HOW TO STOP AN ENDLESS LOOP:
 *
 * If your BASIC program enters an endless loop (e.g., "10 GOTO 10"),
 * you can force-quit the interpreter by pressing:
 *
 * Ctrl+C
 *
 * This will send an interrupt signal (SIGINT), stop the program, and return you
 * to your command-line shell (POSIX or FreeDOS).
 *
 * =============================================================================
 */


/*
 * =============================================================================
 * --- Includes ---
 * =============================================================================
 */

#include <stdio.h>    /* For printf, fgets, FILE, fopen, etc. (Standard I/O) */
#include <stdlib.h>   /* For strtol, exit, atoi (Standard Library) */
#include <string.h>   /* For strcmp, strncpy, strlen, strtok, memset (String ops) */
#include <ctype.h>    /* For isdigit, isalpha, isspace (Character types) */
#include <stddef.h>   /* For size_t (used by string.h etc.) */

/*
 * Note: We provide our own portable, case-insensitive string compare
 * function `ib_stricmp` at the end of this file to avoid
 * non-standard functions like `stricmp` or `strcasecmp`.
 */


/*
 * =============================================================================
 * --- Constants ---
 * =============================================================================
 */

/**
 * @brief MAX_LINES
 * The maximum number of lines the BASIC program can have.
 * This directly impacts the "Program Storage" memory.
 */
#define MAX_LINES 500

/**
 * @brief MAX_LINE_LEN
 * The maximum number of characters allowed in a single line of BASIC code,
 * *not* including the line number itself.
 * This directly impacts the "Program Storage" memory.
 */
#define MAX_LINE_LEN 127

/**
 * @brief STACK_SIZE
 * The maximum number of nested GOSUB calls.
 * This directly impacts the "GOSUB Stack" memory.
 */
#define STACK_SIZE 64

/**
 * @brief NUM_VARIABLES
 * The number of variables (A-Z). This is fixed at 26.
 */
#define NUM_VARIABLES 26

/**
 * @brief COMMAND_MAX_LEN
 * Internal buffer size for parsing the command (e.g., "PRINT", "GOTO").
 * Should be a few chars larger than the longest command.
 */
#define COMMAND_MAX_LEN 32


/*
 * =============================================================================
 * --- Data Structures ---
 * =============================================================================
 */

/**
 * @brief Line
 * A structure to hold a single line of BASIC code in program storage.
 * It stores the line number and the text of the line.
 */
typedef struct
{
    int line_number;
    char text[MAX_LINE_LEN];
} Line;


/*
 * =============================================================================
 * --- Global Variables ---
 * =============================================================================
 */

/*
 * Program Storage:
 * An array to hold all lines of the user's BASIC program.
 * Its size is (MAX_LINES * sizeof(Line)).
 */
static Line program_storage[MAX_LINES];

/**
 * @brief line_count
 * The number of lines *currently* stored in `program_storage`.
 */
static int line_count = 0;

/**
 * @brief variables
 * A simple array for variables A-Z. 'A' maps to index 0, 'B' to 1, etc.
 * We use `signed char` as it's typically the smallest signed type (1 byte),
 * which gives us our 8-bit signed range (-128 to +127).
 */
static signed char variables[NUM_VARIABLES];

/**
 * @brief gosub_stack
 * A fixed-size stack to store return addresses (line indices) for GOSUB.
 */
static int gosub_stack[STACK_SIZE];

/**
 * @brief stack_pointer
 * Points to the next *free* slot in the `gosub_stack`.
 */
static int stack_pointer = 0;

/**
 * @brief program_counter
 * The *index* in `program_storage` of the line being executed.
 * This is only used when `RUN`ning a program.
 */
static int program_counter = 0;

/**
 * @brief is_running
 * A flag to control the `RUN` loop. Set to 0 by END, STOP, or an error.
 */
static int is_running = 0;

/**
 * @brief is_debug_mode
 * A flag set at startup by the --debug argument.
 * This will enable verbose logging of interpreter state.
 */
static int is_debug_mode = 0;

/**
 * @brief parser_ptr
 * A global string pointer used by the parser.
 * This points to the *current character* being parsed within a line.
 * This is a common and simple way to manage state in a recursive parser.
 */
static char* parser_ptr;

/**
 * @brief current_dialect_name
 * A global variable to hold the name of the dialect for the startup banner.
 * This allows modules to "re-brand" the interpreter.
 */
static const char* current_dialect_name = "core";

/**
 * @brief current_version
 * A global variable to hold the version string.
 * This is the "single source of truth" for the version number.
 */
static const char* current_version = "5.0";


/*
 * =============================================================================
 * --- Forward Declarations ---
 * =============================================================================
 */

/*
 * This is a "table of contents" for all our functions.
 * It tells the C compiler about the "shape" of each function *before* it's
 * used. This allows us to define functions in a clean, logical order
 * without worrying about "implicit declaration" compile errors.
 */

/* --- Core Interpreter Functions --- */
static void execute_statement(void);
static void run_program(void);
static void list_program(void);
static void new_program(void);
static void save_program(const char* filename);
static void load_program(const char* filename);

/* --- Program Storage Functions --- */
static int  find_line_index(int line_number);
static void store_line(const char* line_str);

/* --- Command Handlers (cmd_*.c) --- */
static void cmd_print(void);
static void cmd_lprint(void);
static void cmd_input(void);
static void cmd_let(void);
static void cmd_goto(void);
static void cmd_gosub(void);
static void cmd_return(void);
static void cmd_if(void);
static void cmd_rem(void);
static void cmd_end(void);
static void cmd_beep(void);
static void cmd_system(void);
static void cmd_exit(void);
static void cmd_quit(void);
static void cmd_stub(const char* command);

/* --- Parser Functions (parser.c) --- */
static signed char parse_expression(void);
static signed char parse_term(void);
static signed char parse_number(void);

/* --- Utility Functions (utils.c) --- */
static void report_error(const char* message);
static void skip_whitespace(void);
static int  ib_stricmp(const char* s1, const char* s2);


/*
 * =============================================================================
 * --- Core Interpreter ---
 * =============================================================================
 */

/**
 * @brief main
 * The main entry point for the IB interpreter.
 * It initializes the interpreter and runs the main REPL
 * (Read-Evaluate-Print Loop).
 *
 * @param argc The count of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return 0 on successful exit.
 *
 * We add `argc` and `argv` to check for command-line arguments
 * like `--debug`, as requested in the project specifications.
 */
int main(int argc, char *argv[])
{
    /*
     * We use MAX_LINE_LEN for the input buffer, but add extra space
     * (+20) to safely accommodate a line number and spaces, preventing
     * a buffer overflow on user input.
     */
    char input_buffer[MAX_LINE_LEN + 20];
    long total_program_bytes = sizeof(program_storage);
    long total_program_kb = total_program_bytes / 1024;

    /*
     * A temporary buffer for the "immediate mode" line.
     * We copy the user's input into this buffer because
     * `execute_statement` will modify it (via `parser_ptr` and `strtok`).
     */
    char immediate_line[MAX_LINE_LEN + 20];

    /* --- Check for --debug flag --- */
    /*
     * This fulfills the requirement for a --debug flag.
     * We loop through all arguments, not just the first one.
     */
    int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            is_debug_mode = 1;
            printf("[DEBUG] Debug mode enabled.\n");
            break; /* Stop checking once found */
        }
    }


    /* Clear memory and prepare the interpreter for startup. */
    new_program();

    /* --- Startup Banner --- */
    /* Note: We use %ld for 'long' to be portable. */
    printf("BASIC++ (%s) v%s\n", current_dialect_name, current_version);
    printf("%ld kbytes Free\n", total_program_kb);
    printf("READY\n");

    /*
     * --- Main REPL Loop ---
     * This is the main Read-Evaluate-Print Loop.
     * 1. READ:   It waits for user input (using fgets).
     * 2. EVAL:   It figures out if it's a direct command or a stored line.
     * 3. PRINT:  It executes the command (printing output) or stores the line.
     * 4. LOOP:   It repeats.
     */
    while (1)
    {
        printf("> ");
        fflush(stdout); /* Ensure prompt is displayed before input */

        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL)
        {
            /*
             * User pressed Ctrl+D (end-of-file) or an error occurred.
             * This is a clean way to exit the REPL.
             */
            printf("\n"); /* Print a newline to make it clean */
            break; /* Exit loop and terminate program */
        }

        /* Remove newline character (both \n and \r) from input */
        input_buffer[strcspn(input_buffer, "\r\n")] = 0;

        /* --- Direct Mode or Stored Line? --- */
        parser_ptr = input_buffer;  /* MUST set parser_ptr before skip_whitespace() */
        skip_whitespace();

        if (isdigit((unsigned char)*parser_ptr))
        {
            /*
             * 1. STORED LINE
             * The line starts with a digit, so it's a program line
             * that needs to be stored.
             */
            store_line(input_buffer);
        }
        else if (*parser_ptr != '\0')
        {
            /*
             * 2. DIRECT MODE
             * The line does NOT start with a digit, and it's not empty.
             * This is a direct command to be executed immediately.
             */

            /*
             * We MUST copy the input line before executing.
             * `execute_statement` modifies the string it's parsing,
             * so we give it a temporary copy to work with.
             */
			strncpy(immediate_line, input_buffer, sizeof(immediate_line));
			immediate_line[sizeof(immediate_line) - 1] = '\0';  /* Safety */

            /* Set the global parser pointer to the start of our copy */
            parser_ptr = immediate_line;

            /*
             * Set the `is_running` flag. This tells our functions
             * (like `report_error`) that we are in "execution mode"
             * (even though it's just one line).
             */
            is_running = 1;

            /* Call the main dispatch function to run this single line. */
            execute_statement();

            /*
             * The line is done, so we clear the flag.
             * If an error occurred, `is_running` will already be 0,
             * but it's safe to set it again.
             */
            is_running = 0;
            printf("OK\n"); /* "OK" is the standard response in direct mode */
            printf("READY\n");
        }
        else
        {
            /*
             * 3. BLANK LINE
             * The user just hit Enter. Do nothing and show the prompt.
             */
            printf("READY\n");
        }
    }

    return 0; /* User exited the REPL */
}


/**
 * @brief execute_statement
 * The main "dispatch" function for the interpreter.
 *
 * This function reads the *first word* from the global `parser_ptr`
 * (which is assumed to be a command) and then "dispatches"
 * execution to the correct `cmd_...` handler function.
 *
 * This is the primary place to **ADD NEW COMMANDS**.
 * As per the architecture proposals, this large if/else block is the
 * component that will be replaced by a dynamic dispatch table.
 */
static void execute_statement(void)
{
    /*
     * A small, local buffer to hold the parsed command.
     * We use a fixed size for simplicity (COMMAND_MAX_LEN).
     */
    char command[COMMAND_MAX_LEN];
    int i = 0;

    /* Ensure the `is_running` flag is checked before we do anything. */
    if (!is_running) return;

    /* 1. Get the command (the first "token") */
    skip_whitespace();

    /*
     * Parse the command token manually.
     * We use `toupper` to make the command case-insensitive
     * *as we read it*.
     *
     * We don't use `strtok` here because it's destructive and can
     * cause issues. A simple loop is safer and cleaner.
     */
    while (*parser_ptr && !isspace((unsigned char)*parser_ptr) && i < (COMMAND_MAX_LEN - 1))
    {
        command[i] = toupper((unsigned char)*parser_ptr);
        parser_ptr++;
        i++;
    }
    command[i] = '\0'; /* Null-terminate the command string */

    /* 2. Skip whitespace *after* the command to point to its arguments */
    skip_whitespace();

    if (is_debug_mode)
    {
        printf("[DEBUG] Executing command: '%s', Args: '%s'\n", command, parser_ptr);
    }

    /*
     * --- COMMAND DISPATCH TABLE ---
     *
     * This is the "switchboard" of the interpreter. It compares the
     * parsed `command` string to our list of known commands and
     * calls the appropriate function.
     *
     * TO ADD A NEW COMMAND:
     * 1. Write a `cmd_mycommand(void)` function.
     * 2. Add its prototype to the "Forward Declarations" section.
     * 3. Add an `else if (strcmp(command, "MYCOMMAND") == 0)`
     * block here to call your new function.
     */

    if (command[0] == '\0')
    {
        /* Empty line (or just a line number), do nothing. */
        return;
    }
    else if (strcmp(command, "PRINT") == 0)
    {
        cmd_print();
    }
    else if (strcmp(command, "LPRINT") == 0)
    {
        cmd_lprint();
    }
    else if (strcmp(command, "LET") == 0)
    {
        cmd_let();
    }
    else if (strcmp(command, "INPUT") == 0)
    {
        cmd_input();
    }
    else if (strcmp(command, "GOTO") == 0)
    {
        cmd_goto();
    }
    else if (strcmp(command, "GOSUB") == 0)
    {
        cmd_gosub();
    }
    else if (strcmp(command, "RETURN") == 0)
    {
        cmd_return();
    }
    else if (strcmp(command, "IF") == 0)
    {
        cmd_if();
    }
    else if (strcmp(command, "REM") == 0)
    {
        cmd_rem(); /* Does nothing, just a comment */
    }
    else if (strcmp(command, "END") == 0)
    {
        cmd_end();
    }
    else if (strcmp(command, "STOP") == 0)
    {
        /* STOP is an alias for END in our simple dialect */
        cmd_end();
    }
    else if (strcmp(command, "BEEP") == 0)
    {
        cmd_beep();
    }
    else if (strcmp(command, "RUN") == 0)
    {
        /*
         * Special case: RUN is a direct-mode command.
         * If we are *already* running, it's an error.
         * (This check prevents "10 RUN" from causing chaos).
         */
        if (is_running && program_counter > 0)
        {
            report_error("CAN'T USE RUN IN A PROGRAM");
        }
        else
        {
            run_program();
        }
    }
    else if (strcmp(command, "LIST") == 0)
    {
        /* Same as RUN, this is a direct-mode command. */
        if (is_running && program_counter > 0)
        {
            report_error("CAN'T USE LIST IN A PROGRAM");
        }
        else
        {
            list_program();
        }
    }
    else if (strcmp(command, "NEW") == 0)
    {
        /* Same as RUN, this is a direct-mode command. */
        if (is_running && program_counter > 0)
        {
            report_error("CAN'T USE NEW IN A PROGRAM");
        }
        else
        {
            new_program();
        }
    }
    else if (strcmp(command, "SAVE") == 0)
    {
        if (is_running && program_counter > 0)
        {
            report_error("CAN'T USE SAVE IN A PROGRAM");
        }
        else
        {
            /*
             * We pass the `parser_ptr` (which now points
             * to the filename) to the save function.
             */
            save_program(parser_ptr);
        }
    }
    else if (strcmp(command, "LOAD") == 0)
    {
        if (is_running && program_counter > 0)
        {
            report_error("CAN'T USE LOAD IN A PROGRAM");
        }
        else
        {
            load_program(parser_ptr);
        }
    }
    else if (strcmp(command, "SYSTEM") == 0)
    {
        cmd_system();
    }
    else if (strcmp(command, "QUIT") == 0)
    {
        /*
         * QUIT is a direct-mode command, but we also allow
         * "10 QUIT" as a way to exit a program.
         */
        cmd_quit();
    }
    else if (strcmp(command, "EXIT") == 0)
    {
        /* EXIT is an alias for QUIT */
        cmd_exit();
    }
    /*
     * --- Module Extension Stubs ---
     * These commands are reserved for future modules.
     * They are handled by a simple "stub" function.
     */
    else if (strcmp(command, "$IMPORT") == 0)
    {
        cmd_stub("$IMPORT");
    }
    else if (strcmp(command, "$INCLUDE") == 0)
    {
        cmd_stub("$INCLUDE");
    }
    else if (strcmp(command, "$MERGE") == 0)
    {
        cmd_stub("$MERGE");
    }
    else
    {
        /*
         * If the command is not in our list, it's an error.
         * We pass the buffer we parsed for a clean error message.
         */
        report_error("UNKNOWN COMMAND");
    }
}


/*
 * =============================================================================
 * --- Core Functions ---
 * =============================================================================
 */

/**
 * @brief run_program
 * Executes the stored BASIC program from the beginning.
 */
static void run_program(void)
{
    /*
     * A temporary buffer for the line being executed.
     * We *must* copy the line from `program_storage` before
     * parsing, because the parser (`execute_statement`)
     * modifies the string it is parsing.
     */
    char line_buffer[MAX_LINE_LEN];

    if (is_debug_mode)
    {
        printf("[DEBUG] --- RUNNING PROGRAM ---\n");
    }

    /* 1. Initialize the "CPU" */
    is_running = 1;       /* Set the run flag to ON */
    program_counter = 0;  /* Start at the first line (index 0) */
    stack_pointer = 0;    /* Clear the GOSUB stack */
    memset(variables, 0, sizeof(variables)); /* Clear all variables */

    /*
     * 2. Main Execution Loop
     * This loop is the "CPU" of our interpreter.
     * It runs as long as `is_running` is true and we haven't
     * run off the end of the program.
     */
    while (is_running && program_counter < line_count)
    {
        /*
         * We store the *current* line index.
         * This lets us detect if a GOTO or GOSUB
         * has changed the `program_counter`.
         */
        int current_line_index = program_counter;

        if (is_debug_mode)
        {
            printf("[DEBUG] Running line %d: %s\n",
                   program_storage[program_counter].line_number,
                   program_storage[program_counter].text);
        }

        /*
         * Copy the line text to our temporary buffer.
         * This is CRITICAL. We must not let the parser
         * modify our original program code in `program_storage`.
         */
        strncpy(line_buffer, program_storage[program_counter].text, MAX_LINE_LEN);
        line_buffer[MAX_LINE_LEN - 1] = '\0'; /* Ensure null-termination */

        /* Set the global parser pointer to the start of this line */
        parser_ptr = line_buffer;

        /* Execute the statement(s) on this line */
        execute_statement();

        /*
         * 3. Advance the Program Counter
         * If the `program_counter` *was not* changed by the statement
         * (e.g., it was not a GOTO, GOSUB, or RETURN),
         * then we manually advance it to the next line.
         */
        if (is_running && program_counter == current_line_index)
        {
            program_counter++;
        }
    }

    /* 4. Program finished */
    if (is_debug_mode)
    {
        printf("[DEBUG] --- PROGRAM ENDED ---\n");
    }
    is_running = 0; /* Set the run flag to OFF */
}

/**
 * @brief list_program
 * Prints all lines currently in program storage.
 */
static void list_program(void)
{
    int i;
    for (i = 0; i < line_count; i++)
    {
        printf("%d %s\n", program_storage[i].line_number, program_storage[i].text);
    }
}

/**
 * @brief new_program
 * Clears the program, variables, and GOSUB stack.
 * Resets the interpreter to a clean state.
 */
static void new_program(void)
{
    if (is_debug_mode)
    {
        printf("[DEBUG] Clearing all memory (NEW).\n");
    }
    /*
     * We just set line_count to 0. This "orphans" all the
     * data in `program_storage`, effectively clearing it
     * without spending time zeroing the memory.
     */
    line_count = 0;
    program_counter = 0;
    stack_pointer = 0;

    /* We *must*, however, zero the variable and stack memory. */
    memset(variables, 0, sizeof(variables));
    memset(gosub_stack, 0, sizeof(gosub_stack));
}

/**
 * @brief save_program
 * Saves the current program to a text file.
 * @param filename The name of the file to save.
 */
static void save_program(const char* filename)
{
    FILE *file;
    int i;

    /*
     * The `filename` pointer comes from `parser_ptr`,
     * which is already pointing at the filename.
     * We just need to check if it's empty.
     */
    if (filename == NULL || *filename == '\0')
    {
        report_error("FILENAME REQUIRED");
        return;
    }
    /*
     * NOTE: We are not trimming whitespace from the filename
     * for simplicity. "SAVE my file.bas" will fail.
     * The user must type "SAVE myfile.bas".
     */

    if (is_debug_mode)
    {
        printf("[DEBUG] Saving program to '%s'\n", filename);
    }

    file = fopen(filename, "w"); /* "w" = Write mode (create/overwrite) */
    if (file == NULL)
    {
        report_error("CANNOT OPEN FILE");
        return;
    }

    /* Iterate through program storage and print each line to the file */
    for (i = 0; i < line_count; i++)
    {
        fprintf(file, "%d %s\n", program_storage[i].line_number, program_storage[i].text);
    }

    fclose(file);
}

/**
 * @brief load_program
 * Loads a program from a text file, replacing the current one.
 * @param filename The name of the file to load.
 */
static void load_program(const char* filename)
{
    FILE *file;
    /*
     * Buffer for reading lines *from the file*.
     * It must be large enough to hold the line number, space,
     * and the line text.
     */
    char file_line_buffer[MAX_LINE_LEN + 20];

    if (filename == NULL || *filename == '\0')
    {
        report_error("FILENAME REQUIRED");
        return;
    }

    if (is_debug_mode)
    {
        printf("[DEBUG] Loading program from '%s'\n", filename);
    }

    file = fopen(filename, "r"); /* "r" = Read mode */
    if (file == NULL)
    {
        report_error("FILE NOT FOUND");
        return;
    }

    /* 1. Clear the old program */
    new_program();

    /*
     * 2. Read every line from the file
     * `fgets` reads one line at a time into `file_line_buffer`.
     */
    while (fgets(file_line_buffer, sizeof(file_line_buffer), file) != NULL)
    {
        /* Remove newline character */
        file_line_buffer[strcspn(file_line_buffer, "\r\n")] = 0;

        /*
         * 3. Store the line
         * We pass the freshly-read line to `store_line`,
         * which will parse the line number and add it to
         * our `program_storage`.
         */
        store_line(file_line_buffer);
    }

    fclose(file);
}


/*
 * =============================================================================
 * --- Program Storage Functions ---
 * =============================================================================
 */

/**
 * @brief find_line_index
 * Finds the array index for a given line number.
 * Uses a simple linear search.
 *
 * @param line_number The BASIC line number to find (e.g., 100).
 * @return The array index (0 to line_count-1), or -1 if not found.
 */
static int find_line_index(int line_number)
{
    int i;
    for (i = 0; i < line_count; i++)
    {
        if (program_storage[i].line_number == line_number)
        {
            return i; /* Found it */
        }
        if (program_storage[i].line_number > line_number)
        {
            /*
             * The list is sorted, so if we've passed it,
             * it doesn't exist. Stop early.
             */
            return -1;
        }
    }
    return -1; /* Not found */
}

/**
 * @brief store_line
 * Inserts or replaces a line in the program storage.
 * This function is the core of the line editor. It handles
 * adding, replacing, and deleting lines.
 *
 * It also keeps the `program_storage` array sorted by line number.
 *
 * @param line_str The full text of the line, e.g., "10 PRINT A".
 */
static void store_line(const char* line_str)
{
    int line_number;
    const char *text_part;
    int index;
    int i;
    char first_char;

    /* 1. Parse the line number */
    line_number = atoi(line_str);
    if (line_number <= 0 || line_number > 65535)
    {
        report_error("INVALID LINE NUMBER");
        return;
    }

    /* 2. Find where the actual text begins */
    text_part = line_str;
    while (isdigit((unsigned char)*text_part)) text_part++; /* Skip line number */
    while (isspace((unsigned char)*text_part)) text_part++; /* Skip spaces */

    /* `text_part` now points to the "PRINT A" part */

    /* 3. Find the existing index for this line, if any */
    index = find_line_index(line_number);

    /*
     * 4. Handle "Delete Line"
     * If the user typed just "10" (with no text after it),
     * `text_part` will point to an empty string.
     */
    first_char = *text_part;
    if (first_char == '\0')
    {
        if (index != -1)
        {
            /*
             * Line "10" exists, and the user wants to delete it.
             * We do this by "shuffling up" all subsequent lines
             * to overwrite the one at `index`.
             */
            if (is_debug_mode)
            {
                printf("[DEBUG] Deleting line %d at index %d.\n", line_number, index);
            }
            for (i = index; i < line_count - 1; i++)
            {
                program_storage[i] = program_storage[i + 1];
            }
            line_count--; /* The program is now one line shorter */
        }
        /* If index == -1, the user tried to delete a line
         * that doesn't exist, so we do nothing.
         */
        return;
    }

    /*
     * 5. Handle "Replace Line"
     * The line number *was* found (index is not -1),
     * and there is text. This is a simple replacement.
     */
    if (index != -1)
    {
        if (is_debug_mode)
        {
            printf("[DEBUG] Replacing line %d at index %d.\n", line_number, index);
        }
        /*
         * We MUST clear the buffer *before* copying,
         * to ensure it's always null-terminated.
         */
        memset(program_storage[index].text, 0, MAX_LINE_LEN);
        strncpy(program_storage[index].text, text_part, MAX_LINE_LEN - 1);
        return;
    }

    /*
     * 6. Handle "Insert New Line"
     * The line number was *not* found (index == -1).
     * We need to find the correct sorted position and insert it.
     */
    if (line_count >= MAX_LINES)
    {
        report_error("PROGRAM MEMORY FULL");
        return;
    }

    /* Find the insertion point (the first line > our new line) */
    for (i = 0; i < line_count; i++)
    {
        if (program_storage[i].line_number > line_number)
        {
            break;
        }
    }
    /* `i` is now the index where we need to insert */

    if (is_debug_mode)
    {
        printf("[DEBUG] Inserting line %d at index %d.\n", line_number, i);
    }

    /*
     * "Shuffle down" all lines from `i` to the end
     * to make room for the new line.
     */
    int j;
    for (j = line_count; j > i; j--)
    {
        program_storage[j] = program_storage[j - 1];
    }

    /* 7. Insert the new line into the empty slot */
    program_storage[i].line_number = line_number;
    /*
     * We MUST clear the buffer *before* copying.
     */
    memset(program_storage[i].text, 0, MAX_LINE_LEN);
    strncpy(program_storage[i].text, text_part, MAX_LINE_LEN - 1);
    line_count++; /* The program is now one line longer */
}


/*
 * =============================================================================
 * --- Command Handlers ---
 * =============================================================================
 */

/**
 * @brief cmd_print
 * Handler for: PRINT [expression] OR PRINT "[string]"
 * Parses and prints the value of an expression or a string literal.
 */
static void cmd_print(void)
{
    signed char value;
    char* str_end;

    skip_whitespace();

    /* Check if the first argument is a string literal */
    if (*parser_ptr == '"')
    {
        parser_ptr++; /* Consume the opening quote */

        /* Find the closing quote */
        str_end = strchr(parser_ptr, '"');

        if (str_end == NULL)
        {
            /* No closing quote found. This is a syntax error. */
            report_error("UNTERMINATED STRING");
        }
        else
        {
            /*
             * A closing quote was found.
             * Temporarily replace it with a null terminator
             * to "cut" the string.
             */
            *str_end = '\0';
            printf("%s\n", parser_ptr); /* Print the cut string */
            *str_end = '"'; /* Restore the quote (good practice) */

            /* Move the parser to just after the closing quote */
            parser_ptr = str_end + 1;
        }
    }
    else if (*parser_ptr == '\0')
    {
        /*
         * Check if the line is just "PRINT".
         * If so, print a blank line (a value of 0).
         */
        printf("0\n");
    }
    else
    {
        /*
         * It's not a string, so we assume it's an expression.
         */
        value = parse_expression();
        if (is_running) /* parse_expression might have set is_running=0 on error */
        {
            printf("%d\n", value);
        }
    }
}

/**
 * @brief cmd_lprint
 * Handler for: LPRINT [expression]
 *
 * Parses an expression and "prints" it to a file `lprint.out`.
 * This simulates a line printer (LPT1).
 *
 * Per the project specification, this is the fallback for systems
 * without a physical printer. It appends to the file, so multiple
 * LPRINT commands will build up the file.
 */
static void cmd_lprint(void)
{
    signed char value;
    FILE *lpr_file;

    if (*parser_ptr == '\0')
    {
        value = 0; /* LPRINT with no expression prints 0 */
    }
    else
    {
        value = parse_expression();
    }

    if (!is_running) return; /* Error during parsing */

    /* Open the printer file in "append" mode */
    lpr_file = fopen("lprint.out", "a");
    if (lpr_file == NULL)
    {
        /* We report an error, but this is not a fatal
         * error for the BASIC program itself.
         */
        report_error("COULD NOT OPEN LPRINT.OUT FILE");
        return;
    }

    fprintf(lpr_file, "%d\n", value);
    fclose(lpr_file);
}

/**
 * @brief cmd_input
 * Handler for: INPUT [variable]
 * Prompts the user for a value and stores it in a variable.
 */
static void cmd_input(void)
{
    char var_name;
    char input_buffer[20]; /* Small buffer for user's number */
    signed char value;

    skip_whitespace();
    if (!isalpha((unsigned char)*parser_ptr))
    {
        report_error("EXPECTED VARIABLE FOR INPUT");
        return;
    }
    var_name = toupper((unsigned char)*parser_ptr);

    /*
     * We must check the variable *before* asking for input,
     * otherwise the user gets a "?" prompt for a bad variable.
     */
    if (var_name < 'A' || var_name > 'Z')
    {
        report_error("INVALID VARIABLE");
        return;
    }

    printf("? "); /* The classic BASIC input prompt */
    fflush(stdout); /* Make sure the "?" appears before we wait */

    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL)
    {
        /* Handle end-of-file (Ctrl+D) - stop the program */
        if (is_running) is_running = 0;
        return;
    }

    /*
     * Parse the user's input as a DECIMAL number.
     * We use `strtol` (base 10) and then cast to `signed char`
     * to get our 8-bit wraparound.
     */
    value = (signed char)strtol(input_buffer, NULL, 10);
    variables[var_name - 'A'] = value;
}

/**
 * @brief cmd_let
 * Handler for: LET [variable] = [expression]
 * Assigns the result of an expression to a variable.
 */
static void cmd_let(void)
{
    char var_name;

    skip_whitespace();
    if (!isalpha((unsigned char)*parser_ptr))
    {
        report_error("EXPECTED VARIABLE FOR LET");
        return;
    }
    var_name = toupper((unsigned char)*parser_ptr);
    parser_ptr++; /* Consume variable name */

    if (var_name < 'A' || var_name > 'Z')
    {
        report_error("INVALID VARIABLE");
        return;
    }

    skip_whitespace();
    if (*parser_ptr != '=')
    {
        report_error("EXPECTED '=' IN LET");
        return;
    }
    parser_ptr++; /* Consume '=' */

    /*
     * Parse the expression on the right-hand side
     * and assign it to the variable.
     */
    variables[var_name - 'A'] = parse_expression();
}

/**
 * @brief cmd_goto
 * Handler for: GOTO [line_number]
 * Unconditionally jumps to a different line number.
 */
static void cmd_goto(void)
{
    int line_num;
    int index;

    /* GOTO's argument is a single number, not a full expression */
    line_num = (int)parse_number();

    /* Stop if parsing the line number failed */
    if (!is_running) return;

    if (is_debug_mode)
    {
        printf("[DEBUG] GOTO: Jumping to line %d\n", line_num);
    }

    index = find_line_index(line_num);

    if (index == -1)
    {
        report_error("LINE NOT FOUND");
    }
    else
    {
        /*
         * This is the "jump". We set the program counter
         * to the *index* of the target line.
         */
        program_counter = index;
    }
}

/**
 * @brief cmd_gosub
 * Handler for: GOSUB [line_number]
 * Pushes the next line onto the stack and jumps (GOTO) to a line.
 */
static void cmd_gosub(void)
{
    /* 1. Check for Stack Overflow */
    if (stack_pointer >= STACK_SIZE)
    {
        report_error("GOSUB STACK OVERFLOW");
        return;
    }

    if (is_debug_mode)
    {
        printf("[DEBUG] GOSUB: Pushing return index %d to stack slot %d\n",
               program_counter + 1, stack_pointer);
    }

    /*
     * 2. Push the *next* line index onto the stack
     * We push `program_counter + 1` so that when
     * `RETURN` is called, we resume on the line *after*
     * the GOSUB.
     */
    gosub_stack[stack_pointer] = program_counter + 1;
    stack_pointer++;

    /*
     * 3. Now, just perform a GOTO
     * We re-use the GOTO logic to handle the jump.
     */
    cmd_goto();
}

/**
 * @brief cmd_return
 * Handler for: RETURN
 * Pops a line index from the stack and jumps to it.
 */
static void cmd_return(void)
{
    /* 1. Check for Stack Underflow */
    if (stack_pointer <= 0)
    {
        report_error("RETURN WITHOUT GOSUB");
        return;
    }

    /*
     * 2. Pop the return address
     * We decrement the pointer *first*, then read the value.
     */
    stack_pointer--;
    program_counter = gosub_stack[stack_pointer];

    if (is_debug_mode)
    {
        printf("[DEBUG] RETURN: Popping index %d from stack. New PC: %d\n",
               program_counter, program_counter);
    }
}

/**
 * @brief cmd_if
 * Handler for: IF [expr] [op] [expr] THEN [statement]
 *
 * Evaluates a condition and executes a statement if true.
 * e.g., IF A > B THEN GOTO 100
 * e.g., IF A = 7 THEN PRINT 1
 *
 * NOTE: This is a minimal IF. It does *not* support ELSE.
 */
static void cmd_if(void)
{
    signed char val1, val2;
    char op[3]; /* For operators like "<>" */
    int condition = 0;

    /* 1. Parse the first expression */
    val1 = parse_expression();
    if (!is_running) return; /* Stop if parse failed */

    /* 2. Parse the operator */
    skip_whitespace();
    if (*parser_ptr == '=')
    {
        op[0] = '='; op[1] = '\0';
        parser_ptr++;
    }
    else if (*parser_ptr == '<')
    {
        parser_ptr++;
        if (*parser_ptr == '>')
        {
            op[0] = '<'; op[1] = '>'; op[2] = '\0'; /* "<>" operator */
            parser_ptr++;
        }
        else
        {
            op[0] = '<'; op[1] = '\0'; /* "<" operator */
        }
    }
    else if (*parser_ptr == '>')
    {
        op[0] = '>'; op[1] = '\0';
        parser_ptr++;
    }
    else
    {
        report_error("EXPECTED OPERATOR IN IF");
        return;
    }

    /* 3. Parse the second expression */
    val2 = parse_expression();
    if (!is_running) return; /* Stop if parse failed */

    /* 4. Evaluate the condition */
    if (strcmp(op, "=") == 0)
    {
        condition = (val1 == val2);
    }
    else if (strcmp(op, "<>") == 0)
    {
        condition = (val1 != val2);
    }
    else if (strcmp(op, "<") == 0)
    {
        condition = (val1 < val2);
    }
    else if (strcmp(op, ">") == 0)
    {
        condition = (val1 > val2);
    }

    if (is_debug_mode)
    {
        printf("[DEBUG] IF: val1=%d, op='%s', val2=%d. Condition is %s\n",
               val1, op, val2, condition ? "TRUE" : "FALSE");
    }

    /* 5. Find the "THEN" keyword */
    skip_whitespace();

    /*
     * We must use `ib_stricmp`
     * to check for "THEN" case-insensitively.
     */
    if (ib_stricmp(parser_ptr, "THEN") != 0)
    {
        /*
         * Our portable ib_stricmp checks that the keyword
         * is followed by a space or end-of-string.
         * If it fails, it's an error.
         */
        report_error("EXPECTED 'THEN' IN IF");
        return;
    }

    parser_ptr += 4; /* Move parser past "THEN" */
    skip_whitespace();

    /*
     * 6. Execute if true
     * If the condition was true, we just call `execute_statement`
     * again on the *rest of the line*.
     */
    if (condition)
    {
        if (is_debug_mode)
        {
            printf("[DEBUG] IF (TRUE): Executing remainder of line: '%s'\n", parser_ptr);
        }
        /*
         * Special implicit GOTO: "IF A=1 THEN 100"
         * If the token after THEN is a number, we do a GOTO.
         */
        if (isdigit((unsigned char)*parser_ptr))
        {
            cmd_goto();
        }
        else
        {
            /*
             * It's a regular command (e.g., "THEN PRINT A")
             * We recursively call execute_statement() on the
             * remainder of the line.
             */
            execute_statement();
        }
    }
    /*
     * If condition is false, we do nothing.
     * `cmd_if` returns, and the `run_program` loop
     * will advance to the next line.
     */
}

/**
 * @brief cmd_rem
 * Handler for: REM [any text]
 * Does nothing. This is a "REMark" (a comment).
 */
static void cmd_rem(void)
{
    /*
     * We just "do nothing". The parser has already
     * consumed the "REM" token. The rest of the
     * line (`parser_ptr`) is ignored.
     */
}

/**
 * @brief cmd_end
 * Handler for: END (or STOP)
 * Stops the program execution.
 */
static void cmd_end(void)
{
    /* This is the "off switch" for the `run_program` loop. */
    is_running = 0;
}

/**
 * @brief cmd_beep
 * Handler for: BEEP
 * Sounds the terminal's alert bell.
 */
static void cmd_beep(void)
{
    printf("\a"); /* \a is the standard C "alert" character */
    fflush(stdout);
}

/**
 * @brief cmd_system
 * Handler for: SYSTEM
 * Reserved command. Does nothing.
 */
static void cmd_system(void)
{
    /*
     * This is a stub. Per the spec, it is reserved.
     * We may add functionality later via a module.
     */
    cmd_stub("SYSTEM");
}

/**
 * @brief cmd_exit
 * Handler for: EXIT
 * Reserved command. Does nothing. (Alias for QUIT)
 */
static void cmd_exit(void)
{
    /*
     * Per ECMA-55, QUIT is the command to exit to the OS.
     * We provide this as an alias.
     */
    cmd_quit();
}

/**
 * @brief cmd_quit
 * Handler for: QUIT
 * Exits the interpreter and returns to the OS.
 */
static void cmd_quit(void)
{
    /* This stops the program if it's running */
    if (is_running)
    {
        is_running = 0;
    }
    /*
     * This exits the entire `ib` process.
     * `exit(0)` = "normal, successful exit".
     */
    exit(0);
}

/**
 * @brief cmd_stub
 * Handler for reserved module commands (e.g., $IMPORT)
 * Prints a "not implemented" message.
 * @param command The name of the command (e.g., "$IMPORT").
 */
static void cmd_stub(const char* command)
{
    /*
     * We don't use `report_error` here because this isn't
     * a *syntax* error, just an unimplemented feature.
     */
    printf("FRAMEWORK: Command %s is not implemented.\n", command);
    /* We don't stop the program, just ignore it. */
}


/*
 * =============================================================================
 * --- Parser Functions ---
 * =============================================================================
 */

/**
 * @brief parse_expression
 * Parses a simple mathematical expression (e.g., A + 10 - B).
 *
 * **LIMITATION:** This parser has NO operator precedence.
 * It evaluates strictly left-to-right.
 * `A + B * C` is evaluated as `(A + B) * C`.
 *
 * @return The final 8-bit signed result of the expression.
 */
static signed char parse_expression(void)
{
    signed char result;
    signed char next_term;
    char op;

    /* Guard clause for cascading errors */
    if (!is_running) return 0;

    /* 1. Get the first term (e.g., "A" or "10" or "(...") */
    result = parse_term();

    /* 2. Loop for more terms (e.g., "+ 10", "- B") */
    while (is_running)
    {
        skip_whitespace();
        op = *parser_ptr; /* Peek at the next char */

        /* Check for a valid operator */
        if (op != '+' && op != '-' && op != '*' && op != '/')
        {
            /*
             * No more operators. The expression is done.
             * Return the result we've calculated so far.
             */
            return result;
        }
        parser_ptr++; /* Consume the operator */

        /* 3. Get the next term */
        next_term = parse_term();

        /* 4. Perform the operation */
        if (op == '+')
        {
            /*
             * We cast to (signed char) *after* the operation
             * to perform the 8-bit wraparound.
             */
            result = (signed char)(result + next_term);
        }
        else if (op == '-')
        {
            result = (signed char)(result - next_term);
        }
        else if (op == '*')
        {
            result = (signed char)(result * next_term);
        }
        else if (op == '/')
        {
            if (next_term == 0)
            {
                report_error("DIVISION BY ZERO");
                return 0; /* Stop immediately */
            }
            /*
             * C's integer division automatically truncates,
             * which is exactly what we want.
             */
            result = (signed char)(result / next_term);
        }
    }

    return result;
}

/**
 * @brief parse_term
 * Parses a single "term" in an expression.
 * A term is either a number, a variable, or a (sub-expression).
 *
 * @return The 8-bit signed value of the term.
 */
static signed char parse_term(void)
{
    signed char value;

    if (!is_running) return 0; /* Guard clause */
    skip_whitespace();

    if (isalpha((unsigned char)*parser_ptr))
    {
        /*
         * 1. Term is a Variable (A-Z)
         * We get its value from storage.
         */
        char var_name = toupper((unsigned char)*parser_ptr);
        parser_ptr++; /* Consume the variable name */
        if (var_name < 'A' || var_name > 'Z')
        {
            report_error("INVALID VARIABLE");
            return 0;
        }
        return variables[var_name - 'A'];
    }
    else if (*parser_ptr == '(')
    {
        /*
         * 2. Term is a Sub-Expression, e.g., (A + 5)
         * This adds support for operator precedence.
         */
        parser_ptr++; /* Consume the '(' */
        value = parse_expression(); /* Recursively parse the expression inside */
        skip_whitespace();
        if (*parser_ptr != ')')
        {
            report_error("EXPECTED ')'");
            return 0;
        }
        parser_ptr++; /* Consume the ')' */
        return value;
    }
    else
    {
        /*
         * 3. Term is a Number
         * It's a positive or negative number (e.g., 127, -8).
         */
        return parse_number();
    }
}

/**
 * @brief parse_number
 * Reads a decimal (base 10) number from the `parser_ptr` string,
 * advances the pointer, and returns the 8-bit signed value.
 *
 * @return The 8-bit signed value.
 */
static signed char parse_number(void)
{
    long value = 0;
    char *end_ptr;

    if (!is_running) return 0; /* Guard clause */
    skip_whitespace();

    /*
     * We use `strtol` (string-to-long) to parse the number.
     * - `parser_ptr`: Where to start parsing.
     * - `&end_ptr`: `strtol` will set this to point *after* the parsed number.
     * - `10`: The base (decimal).
     */
    value = strtol(parser_ptr, &end_ptr, 10);

    /* Check 1: Did `strtol` parse *anything*? */
    if (parser_ptr == end_ptr)
    {
        /*
         * No, it didn't. This means `parser_ptr` was pointing
         * at something that isn't a number (e.g., in "PRINT "Hello"").
         */
        report_error("EXPECTED NUMBER");
        return 0;
    }

    /*
     * Check 2: Did `strtol` parse a *valid* number?
     * `end_ptr` should now be pointing at a space or the end
     * of the string. If it's pointing at a letter, it means
     * the user typed "100ABC", which is an error.
     */
    if (*end_ptr != '\0' && !isspace((unsigned char)*end_ptr) && *end_ptr != ')')
    {
        report_error("INVALID NUMBER");
        return 0;
    }


    /* Success. Move our global parser pointer to where `strtol` finished */
    parser_ptr = end_ptr;

    /*
     * Finally, cast the `long` value to a `signed char`.
     * This is where the 8-bit wraparound happens!
     * `(signed char)128` automatically becomes -128.
     * `(signed char)300` (which is 0x12C) becomes 0x2C, which is 44.
     */
    return (signed char)value;
}


/*
 * =============================================================================
 * --- Utility Functions ---
 * =============================================================================
 */

/**
 * @brief report_error
 * A centralized function for reporting errors.
 * It sounds the bell, prints the message, and stops the program
 * *if* it's running.
 * @param message The error message (e.g., "SYNTAX ERROR").
 */
static void report_error(const char* message)
{
    printf("\a"); /* Sound the alert bell */
    printf("ERROR: %s\n", message);
    fflush(stdout);

    if (is_running)
    {
        if (is_debug_mode)
        {
            printf("[DEBUG] Halting program due to error.\n");
        }
        is_running = 0; /* Stop the program */
    }
}

/**
 * @brief skip_whitespace
 * Advances the global `parser_ptr` past any spaces or tabs.
 */
void skip_whitespace(void)
{
    if (parser_ptr == NULL) return;  /* Prevent accidental crash */

    while (*parser_ptr == ' ' || *parser_ptr == '\t')
        parser_ptr++;
}

/**
 * @brief ib_stricmp
 * A portable, case-insensitive string comparison.
 *
 * NOTE: This is a *simple* implementation for our `IF...THEN`
 * check. It is not a full, robust `strcasecmp` (which is not in ANSI C).
 * It only checks if the *start* of `s1` matches `s2` and is
 * followed by a space or null-terminator.
 *
 * @param s1 The string to check (e.g., "THEN GOTO 10").
 * @param s2 The keyword to look for (e.g., "THEN").
 * @return 0 if they match, non-zero if they don't.
 */
static int ib_stricmp(const char* s1, const char* s2)
{
    while (*s2)
    {
        if (toupper((unsigned char)*s1) != toupper((unsigned char)*s2))
        {
            return -1; /* Mismatch */
        }
        s1++;
        s2++;
    }
    /*
     * If we're here, `s2` (e.g., "THEN") matched the start
     * of `s1`. We must now check that `s1` is followed by
     * a space or the end of the string.
     * This prevents "THEN" from matching "THENOR".
     */
    if (*s1 == '\0' || isspace((unsigned char)*s1))
    {
        return 0; /* Match! */
    }
    return -1; /* Mismatch (e.g., "THENOR") */
}

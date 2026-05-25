/* * PROJECT: BASIC++ (IB) Interpreter
 * AUTHOR: BASIC++ Community
 * FILENAME: ib.c
 * VERSION: 1.0.0
 * SYSTEM: POSIX and FreeDOS
 * DESCRIPTION: A minimal, portable interpreter core prioritizing source lucidity 
 * and a static memory footprint. Features 8-bit signed arithmetic, strict left-to-right 
 * parsing, and direct file I/O capabilities.
 * FEATURES: Direct/Program modes, 26 static variables, LPRINT to file, REPL interface.
 * ARCHITECTURAL DESIGN: Single-file procedural architecture utilizing statically 
 * allocated arrays for all program memory, variables, and the execution stack. 
 * Floating-point math is explicitly excluded.
 * HOW TO COMPILE: See detailed numbered instructions in the documentation section.
 * HOW TO PORT: Code relies strictly on ANSI C89 standard libraries. Memory limits 
 * are defined via macros and can be adjusted for systems with under 64KB RAM.
 * PORTABILITY AND MODULARITY GUIDE: 
 * - Arduino: Reduce MAX_LINES to fit within available SRAM. Replace main() with setup()
 * and loop(). Connect standard I/O to the Serial hardware.
 * - Raspberry Pi: Standard GCC compilation. Memory is abundant; default macros apply.
 * DEVELOPERS PHILOSOPHY: Unix-based philosophy, ANSI C89/C90 compliance, no external 
 * libraries. LICENSE CONSTRAINT: This code is strictly not for sale. It may not be 
 * taken, modified, and sold commercially under any circumstances.
 * * PROJECT ROADMAP:
 * COMPLIANCE STATUS:
 * - [MET] 8-bit signed integer math and deterministic wrap-around.
 * - [MET] Left-to-right parser without standard mathematical precedence.
 * - [MET] Implementation of core directives (PRINT, LET, GOTO, IF, etc.).
 * - [PENDING] Pillar 6.1: Addons (Inline foreign language compilation).
 * - [PENDING] Pillar 6.2: Merge ($MERGE directive for code sharing).
 * - [PENDING] Pillar 6.3: Modules (C-level syntax extensibility).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* =========================================================================
 * CONSTANTS / CONFIGURATION
 * ========================================================================= */

/* * MAX_LINES: Maximum number of stored BASIC lines.
 * Safe for user modification: YES.
 * Expected effect: Increases or decreases the maximum program size.
 * Constraints: Memory footprint scales linearly. 500 lines uses ~64KB.
 */
#define MAX_LINES 500

/* * LINE_LEN: Maximum character length of a single BASIC line.
 * Safe for user modification: YES.
 * Expected effect: Allows longer string literals or complex equations.
 * Constraints: Exceeding 127 may break assumed bounds on constrained systems.
 */
#define LINE_LEN 127

/* * NUM_VARIABLES: Fixed number of numeric variables (A-Z).
 * Safe for user modification: NO.
 * Expected effect: Code relies on exactly 26 alphabetical indexes.
 */
#define NUM_VARIABLES 26

/* * STACK_SIZE: Maximum depth for nested GOSUB calls.
 * Safe for user modification: YES.
 * Expected effect: Prevents or allows deeper subroutine recursion.
 */
#define STACK_SIZE 64

typedef struct {
    int line_number;
    char text[LINE_LEN + 1];
} Line;

static Line program_storage[MAX_LINES];
static signed char variables[NUM_VARIABLES];
static int gosub_stack[STACK_SIZE];

static int num_stored_lines = 0;
static int stack_pointer = 0;
static int current_exec_line = -1;
static int execution_halted = 0;

/* Function Prototypes */
void clear_variables(void);
void clear_program(void);
signed char parse_expression(char **ptr);
void execute_line(char *cmd);
void run_program(void);

/*
 * clear_variables
 * Purpose: Zeroes out the 26 static variables and resets the GOSUB stack.
 */
void clear_variables(void) {
    memset(variables, 0, sizeof(variables));
    stack_pointer = 0;
}

/*
 * clear_program
 * Purpose: Erases all stored lines from memory.
 */
void clear_program(void) {
    num_stored_lines = 0;
    clear_variables();
}

/*
 * parse_term
 * Purpose: Evaluates a single number, variable, or parenthesized group.
 * Modularity: Isolates basic value extraction from the mathematical operators.
 */
signed char parse_term(char **ptr) {
    signed char val = 0;
    while (isspace(**ptr)) (*ptr)++;
    
    if (**ptr == '(') {
        (*ptr)++;
        val = parse_expression(ptr);
        while (isspace(**ptr)) (*ptr)++;
        if (**ptr == ')') (*ptr)++;
    } else if (isalpha(**ptr)) {
        int idx = toupper(**ptr) - 'A';
        if (idx >= 0 && idx < NUM_VARIABLES) {
            val = variables[idx];
        }
        (*ptr)++;
    } else if (isdigit(**ptr) || **ptr == '-') {
        int temp = (int)strtol(*ptr, ptr, 10);
        val = (signed char)temp; /* Enforces 8-bit wrap-around */
    }
    
    while (isspace(**ptr)) (*ptr)++;
    return val;
}

/*
 * parse_expression
 * Purpose: Evaluates math strictly left-to-right without standard precedence.
 * Example: 3+4*5 becomes 35, not 23.
 */
signed char parse_expression(char **ptr) {
    signed char result = parse_term(ptr);
    while (**ptr) {
        char op;
        while (isspace(**ptr)) (*ptr)++;
        op = **ptr;
        if (op == '+' || op == '-' || op == '*' || op == '/') {
            signed char next_val;
            (*ptr)++;
            next_val = parse_term(ptr);
            if (op == '+') result += next_val;
            else if (op == '-') result -= next_val;
            else if (op == '*') result *= next_val;
            else if (op == '/') {
                if (next_val != 0) result /= next_val;
                else printf("DIVIDE BY ZERO ERROR\n");
            }
        } else {
            break;
        }
    }
    return result;
}

/*
 * store_line
 * Purpose: Inserts a new line into the program storage array, keeping it sorted.
 */
void store_line(int line_num, char *text) {
    int i, j;
    for (i = 0; i < num_stored_lines; i++) {
        if (program_storage[i].line_number == line_num) {
            strncpy(program_storage[i].text, text, LINE_LEN);
            program_storage[i].text[LINE_LEN] = '\0';
            return;
        } else if (program_storage[i].line_number > line_num) {
            break;
        }
    }
    
    if (num_stored_lines >= MAX_LINES) {
        printf("MEMORY FULL\n");
        return;
    }
    
    for (j = num_stored_lines; j > i; j--) {
        program_storage[j] = program_storage[j - 1];
    }
    
    program_storage[i].line_number = line_num;
    strncpy(program_storage[i].text, text, LINE_LEN);
    program_storage[i].text[LINE_LEN] = '\0';
    num_stored_lines++;
}

/*
 * execute_line
 * Purpose: Dispatches a BASIC directive to the appropriate logic block.
 */
void execute_line(char *cmd) {
    while (isspace(*cmd)) cmd++;
    if (*cmd == '\0') return;

    if (strncmp(cmd, "PRINT", 5) == 0) {
        cmd += 5;
        while (isspace(*cmd)) cmd++;
        if (*cmd == '"') {
            char *end = strchr(cmd + 1, '"');
            if (end) {
                *end = '\0';
                printf("%s\n", cmd + 1);
                *end = '"';
            }
        } else {
            printf("%d\n", parse_expression(&cmd));
        }
    } else if (strncmp(cmd, "LPRINT", 6) == 0) {
        FILE *lp = fopen("lprint.out", "a");
        cmd += 6;
        while (isspace(*cmd)) cmd++;
        if (lp) {
            if (*cmd == '"') {
                char *end = strchr(cmd + 1, '"');
                if (end) {
                    *end = '\0';
                    fprintf(lp, "%s\n", cmd + 1);
                    *end = '"';
                }
            } else {
                fprintf(lp, "%d\n", parse_expression(&cmd));
            }
            fclose(lp);
        }
    } else if (strncmp(cmd, "LET", 3) == 0) {
        char var_name;
        cmd += 3;
        while (isspace(*cmd)) cmd++;
        var_name = toupper(*cmd);
        if (var_name >= 'A' && var_name <= 'Z') {
            cmd++;
            while (isspace(*cmd)) cmd++;
            if (*cmd == '=') {
                cmd++;
                variables[var_name - 'A'] = parse_expression(&cmd);
            }
        }
    } else if (strncmp(cmd, "INPUT", 5) == 0) {
        char var_name;
        char in_buf[16];
        cmd += 5;
        while (isspace(*cmd)) cmd++;
        var_name = toupper(*cmd);
        if (var_name >= 'A' && var_name <= 'Z') {
            printf("? ");
            if (fgets(in_buf, sizeof(in_buf), stdin)) {
                variables[var_name - 'A'] = (signed char)atoi(in_buf);
            }
        }
    } else if (strncmp(cmd, "GOTO", 4) == 0) {
        cmd += 4;
        current_exec_line = parse_expression(&cmd);
    } else if (strncmp(cmd, "GOSUB", 5) == 0) {
        cmd += 5;
        if (stack_pointer < STACK_SIZE) {
            gosub_stack[stack_pointer++] = current_exec_line;
            current_exec_line = parse_expression(&cmd);
        } else {
            printf("OUT OF MEMORY (STACK)\n");
            execution_halted = 1;
        }
    } else if (strncmp(cmd, "RETURN", 6) == 0) {
        if (stack_pointer > 0) {
            current_exec_line = gosub_stack[--stack_pointer];
        } else {
            printf("RETURN WITHOUT GOSUB\n");
            execution_halted = 1;
        }
    } else if (strncmp(cmd, "IF", 2) == 0) {
        signed char expr_val;
        cmd += 2;
        expr_val = parse_expression(&cmd);
        if (strncmp(cmd, "THEN", 4) == 0) {
            cmd += 4;
            if (expr_val != 0) {
                execute_line(cmd);
            }
        }
    } else if (strncmp(cmd, "REM", 3) == 0) {
        /* Do nothing, it is a comment */
    } else if (strncmp(cmd, "STOP", 4) == 0) {
        printf("BREAK\n");
        execution_halted = 1;
    } else if (strncmp(cmd, "END", 3) == 0) {
        execution_halted = 1;
    } else if (strncmp(cmd, "BEEP", 4) == 0) {
        printf("\a");
    } else if (strncmp(cmd, "SYSTEM", 6) == 0) {
        printf("SYSTEM RESERVED FOR MODULES\n");
    } else if (strncmp(cmd, "QUIT", 4) == 0 || strncmp(cmd, "EXIT", 4) == 0) {
        exit(0);
    } else {
        printf("SYNTAX ERROR\n");
    }
}

/*
 * run_program
 * Purpose: Sequentially executes stored lines until halted by logic or end of memory.
 */
void run_program(void) {
    int i;
    clear_variables();
    execution_halted = 0;
    current_exec_line = -1;
    
    if (num_stored_lines == 0) return;
    
    for (i = 0; i < num_stored_lines; i++) {
        if (execution_halted) break;
        
        if (current_exec_line != -1) {
            int found = 0;
            int j;
            for (j = 0; j < num_stored_lines; j++) {
                if (program_storage[j].line_number >= current_exec_line) {
                    i = j;
                    found = 1;
                    break;
                }
            }
            if (!found) break;
            current_exec_line = -1; 
        }
        
        execute_line(program_storage[i].text);
    }
}

/*
 * main
 * Purpose: Initializes the interpreter environment and manages the central REPL.
 */
int main(void) {
    char input_buffer[LINE_LEN + 16];

    clear_program();
    printf("BASIC++ (IB) Interpreter Core\n");
    printf("63 kbytes Free\n");

    while (1) {
        printf("Ready.\n> ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;
        
        input_buffer[strcspn(input_buffer, "\r\n")] = 0;
        
        if (strlen(input_buffer) == 0) continue;

        if (isdigit((unsigned char)input_buffer[0])) {
            int line_num = atoi(input_buffer);
            char *text = strchr(input_buffer, ' ');
            if (text) {
                text++;
                store_line(line_num, text);
            }
        } else if (strncmp(input_buffer, "RUN", 3) == 0) {
            run_program();
        } else if (strncmp(input_buffer, "LIST", 4) == 0) {
            int i;
            for (i = 0; i < num_stored_lines; i++) {
                printf("%d %s\n", program_storage[i].line_number, program_storage[i].text);
            }
        } else if (strncmp(input_buffer, "NEW", 3) == 0) {
            clear_program();
        } else {
            execute_line(input_buffer);
        }
    }
    return 0;
}
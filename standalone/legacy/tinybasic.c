// FILENAME: tinybasic.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// DESCRIPTION: Provides core logic and implementation for tinybasic.c within BASIC++.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* =========================================================================
 * CONSTANTS
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_LINES: increase for larger programs (costs ~260 bytes each)
 *   - LINE_LEN:  increase for longer source lines
 *   - MAX_VARS:  increase for a bigger @() array
 *   - STACK_SIZE: increase for deeper GOSUB nesting
 *
 * WHAT CANNOT BE CHANGED:
 *   - NUM_VARIABLES must remain 26 (A-Z is a dialect requirement)
 *   - Line numbers must stay in 1..32767 range
 *
 * WHAT TO EXPECT:
 *   Total static memory is roughly MAX_LINES * LINE_LEN bytes.
 *   On systems with limited RAM (e.g. Arduino), reduce MAX_LINES.
 *
 * IF SOMETHING BREAKS:
 *   If the program crashes at startup, the static arrays may exceed
 *   available memory.  Reduce MAX_LINES or MAX_VARS.
 * ========================================================================= */

#define MAX_LINES      500
#define LINE_LEN       255
#define MAX_VARS     32500
#define STACK_SIZE      64
#define NUM_VARIABLES   26

/* =========================================================================
 * DATA STRUCTURES
 *
 * WHAT CAN BE CHANGED:
 *   - The text[] buffer size tracks LINE_LEN.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The line_number field must remain int for line number range.
 *
 * WHAT TO EXPECT:
 *   Each ProgramLine occupies LINE_LEN+1+sizeof(int) bytes.
 *
 * IF SOMETHING BREAKS:
 *   If stored lines are truncated, verify LINE_LEN matches strncpy.
 * ========================================================================= */

typedef struct {
    int line_number;
    char text[LINE_LEN + 1];
} ProgramLine;

/* =========================================================================
 * STATIC MEMORY
 *
 * All state is stored in static (BSS) memory to avoid stack overflow
 * on small systems.
 *
 * WHAT CAN BE CHANGED:
 *   - Array sizes (see CONSTANTS section above).
 *
 * WHAT CANNOT BE CHANGED:
 *   - These must remain static/global for C89 portability.
 *
 * WHAT TO EXPECT:
 *   All memory is zero-initialized at program start.
 *   clear_all() resets everything to initial state.
 *
 * IF SOMETHING BREAKS:
 *   If variables contain garbage, call clear_variables() explicitly.
 * ========================================================================= */

static ProgramLine program[MAX_LINES];
static int num_lines = 0;

static short variables[NUM_VARIABLES];
static short array_mem[MAX_VARS];

static int gosub_stack[STACK_SIZE];
static int stack_ptr = 0;

static int program_counter = 0;
static int running = 0;

/* =========================================================================
 * FUNCTION PROTOTYPES
 * ========================================================================= */

static void  clear_all(void);
static void  clear_variables(void);
static void  trim_newline(char *str);
static void  skip_spaces(char **p);
static int   match_keyword(char **p, const char *kw);
static int   find_line(int line_num);
static void  store_line(const char *raw);
static void  delete_line(int line_num);
static short parse_expression(char **p);
static short parse_factor(char **p);
static int   eval_condition(char **p);
static void  execute_line(char *line);
static void  run_program(void);
static void  list_program(void);
static void  cmd_help(void);
static void  resolve_filename(const char *raw, char *out, int out_size);
static void  save_program(const char *filename);
static void  load_program(const char *filename);

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllimport) int __stdcall Beep(unsigned long dwFreq, unsigned long dwDuration);
#ifdef __cplusplus
}
#endif
#endif

static void trigger_beep(void) {
#ifdef _WIN32
    Beep(800, 250);
#else
    putchar('\a');
    fflush(stdout);
#endif
}

/* =========================================================================
 * UTILITY FUNCTIONS
 *
 * Small helpers used throughout the parser and executor.
 *
 * WHAT CAN BE CHANGED:
 *   - trim_newline() could be extended to strip other whitespace.
 *   - match_keyword() could be made case-insensitive at the call
 *     site (currently the caller uppercases first).
 *
 * WHAT CANNOT BE CHANGED:
 *   - skip_spaces() must only skip space and tab (not newlines).
 *   - match_keyword() must check for non-alpha after the keyword
 *     to avoid matching partial words (e.g. PRINTING != PRINT).
 *
 * WHAT TO EXPECT:
 *   These functions modify the pointer passed to them.
 *
 * IF SOMETHING BREAKS:
 *   - If keywords are not recognized, verify the caller uppercases
 *     the input before calling match_keyword().
 *   - If lines have trailing garbage, check trim_newline().
 * ========================================================================= */

static void clear_all(void) {
    num_lines = 0;
    clear_variables();
}

static void clear_variables(void) {
    memset(variables, 0, sizeof(variables));
    memset(array_mem, 0, sizeof(array_mem));
    stack_ptr = 0;
}

static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

/*
 * uppercase_line
 * Converts an entire input line to uppercase, preserving string
 * literals inside double quotes.  These early machines only had
 * uppercase characters.
 */
static void uppercase_line(char *str) {
    int in_quotes = 0;
    while (*str) {
        if (*str == '"') in_quotes = !in_quotes;
        else if (!in_quotes) *str = (char)toupper((unsigned char)*str);
        str++;
    }
}

static void skip_spaces(char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
}

static int match_keyword(char **p, const char *kw) {
    size_t len = strlen(kw);
    if (strncmp(*p, kw, len) == 0 && !isalpha((unsigned char)(*p)[len])) {
        *p += len;
        return 1;
    }
    return 0;
}

/* =========================================================================
 * PROGRAM STORAGE
 *
 * Lines are kept sorted by line number in the program[] array.
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_LINES controls maximum program size.
 *   - The search could be changed from linear to binary for speed.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Lines must always be sorted by line number (GOTO/GOSUB depend on it).
 *   - Entering a line number with no text must delete that line.
 *
 * WHAT TO EXPECT:
 *   - store_line() inserts in sorted order or replaces existing.
 *   - delete_line() removes and compacts the array.
 *   - find_line() returns -1 if the line does not exist.
 *
 * IF SOMETHING BREAKS:
 *   - If LIST shows lines out of order, check the insertion sort in
 *     store_line().
 *   - If GOTO fails, verify find_line() returns the correct index.
 * ========================================================================= */

static int find_line(int line_num) {
    int i;
    for (i = 0; i < num_lines; i++) {
        if (program[i].line_number == line_num) return i;
        if (program[i].line_number > line_num) return -1;
    }
    return -1;
}

static void store_line(const char *raw) {
    int line_num, idx, i, j;
    const char *text;

    line_num = atoi(raw);
    if (line_num <= 0 || line_num > 32767) {
        printf("INVALID LINE NUMBER\n");
        return;
    }

    text = raw;
    while (isdigit((unsigned char)*text)) text++;
    while (*text == ' ' || *text == '\t') text++;

    if (*text == '\0') { delete_line(line_num); return; }

    idx = find_line(line_num);
    if (idx >= 0) {
        strncpy(program[idx].text, text, LINE_LEN);
        program[idx].text[LINE_LEN] = '\0';
        return;
    }

    if (num_lines >= MAX_LINES) { printf("MEMORY FULL\n"); return; }

    for (i = 0; i < num_lines; i++) {
        if (program[i].line_number > line_num) break;
    }
    for (j = num_lines; j > i; j--) program[j] = program[j - 1];

    program[i].line_number = line_num;
    strncpy(program[i].text, text, LINE_LEN);
    program[i].text[LINE_LEN] = '\0';
    num_lines++;
}

static void delete_line(int line_num) {
    int idx, i;
    idx = find_line(line_num);
    if (idx < 0) return;
    for (i = idx; i < num_lines - 1; i++) program[i] = program[i + 1];
    num_lines--;
}

/* =========================================================================
 * EXPRESSION PARSER
 *
 * Implements left-to-right evaluation without operator precedence,
 * matching the original Palo Alto Tiny BASIC behavior.
 *
 * Grammar:
 *   expression = factor { ("+" | "-" | "*" | "/") factor }
 *   factor     = NUMBER | VARIABLE | "@(" expression ")"
 *              | "ABS(" expression ")" | "RND(" expression ")"
 *              | "SIZE" | "(" expression ")" | "-" factor
 *
 * WHAT CAN BE CHANGED:
 *   - Add new functions by adding cases in parse_factor().
 *   - Change numeric range by changing the 'short' type.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Left-to-right evaluation order (dialect requirement).
 *   - The recursive descent structure.
 *   - Division by zero must halt execution.
 *
 * WHAT TO EXPECT:
 *   - 3+4*5 evaluates as (3+4)*5 = 35, not 23.
 *   - Division is integer truncation toward zero.
 *   - RND(X) returns a random integer from 1 to X inclusive.
 *   - SIZE returns total free memory bytes.
 *
 * IF SOMETHING BREAKS:
 *   - Check that all ctype casts use (unsigned char).
 *   - Verify parse_factor returns correct short values.
 *   - If RND always returns the same value, check srand() in main().
 * ========================================================================= */

static short parse_factor(char **p) {
    short val = 0;
    skip_spaces(p);

    /* Parenthesized sub-expression */
    if (**p == '(') {
        (*p)++;
        val = parse_expression(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        return val;
    }

    /* @(index) array access */
    if (**p == '@') {
        short idx;
        (*p)++;
        skip_spaces(p);
        if (**p == '(') (*p)++;
        idx = parse_expression(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        if (idx < 0 || idx >= MAX_VARS) {
            printf("ARRAY BOUNDS ERROR\n");
            running = 0;
            return 0;
        }
        return array_mem[idx];
    }

    /* Unary negation */
    if (**p == '-') {
        (*p)++;
        return (short)-parse_factor(p);
    }

    /* ABS(X) function */
    if (strncmp(*p, "ABS(", 4) == 0 || strncmp(*p, "ABS (", 5) == 0) {
        (*p) += 3;
        skip_spaces(p);
        if (**p == '(') (*p)++;
        val = parse_expression(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        return (short)(val < 0 ? -val : val);
    }

    /* RND(X) function - random integer from 1 to X inclusive */
    if (strncmp(*p, "RND(", 4) == 0 || strncmp(*p, "RND (", 5) == 0) {
        int range;
        (*p) += 3;
        skip_spaces(p);
        if (**p == '(') (*p)++;
        val = parse_expression(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        range = (int)val;
        if (range <= 0) return 0;
        return (short)(rand() % range + 1);
    }

    /* SIZE keyword - returns free memory */
    if (strncmp(*p, "SIZE", 4) == 0 && !isalpha((unsigned char)(*p)[4])) {
        long used = (long)num_lines * (long)(LINE_LEN + 1);
        long total = (long)MAX_LINES * (long)(LINE_LEN + 1);
        (*p) += 4;
        return (short)(total - used > 32767 ? 32767 : total - used);
    }

    /* Variable A-Z */
    if (isalpha((unsigned char)**p)) {
        int idx = toupper((unsigned char)**p) - 'A';
        (*p)++;
        if (idx >= 0 && idx < NUM_VARIABLES) return variables[idx];
        return 0;
    }

    /* Numeric literal */
    if (isdigit((unsigned char)**p)) {
        val = (short)strtol(*p, p, 10);
        return val;
    }

    return 0;
}

static short parse_expression(char **p) {
    short result;
    skip_spaces(p);
    result = parse_factor(p);

    while (**p) {
        char op;
        short rhs;
        skip_spaces(p);
        op = **p;
        if (op != '+' && op != '-' && op != '*' && op != '/') break;
        (*p)++;
        rhs = parse_factor(p);

        if (op == '+')      result = (short)(result + rhs);
        else if (op == '-') result = (short)(result - rhs);
        else if (op == '*') result = (short)(result * rhs);
        else if (op == '/') {
            if (rhs == 0) {
                printf("DIVIDE BY ZERO ERROR\n");
                running = 0;
                return 0;
            }
            result = (short)(result / rhs);
        }
    }
    return result;
}

/* =========================================================================
 * CONDITION EVALUATOR
 *
 * Parses: expression relop expression
 * Supports: =  <>  #  <  >  <=  >=
 *
 * WHAT CAN BE CHANGED:
 *   - Additional relational operators could be added here.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The two-character operator parsing logic (<>, <=, >=).
 *   - The # operator must be an alias for <> (dialect requirement).
 *
 * WHAT TO EXPECT:
 *   - Returns 1 for true, 0 for false.
 *   - An invalid operator halts execution with an error.
 *
 * IF SOMETHING BREAKS:
 *   - If IF/THEN never branches, check that eval_condition parses
 *     the operator correctly.
 *   - Verify the operator characters are not consumed by
 *     parse_expression().
 * ========================================================================= */

static int eval_condition(char **p) {
    short lhs, rhs;
    char op1, op2;

    lhs = parse_expression(p);
    skip_spaces(p);

    op1 = **p;
    op2 = '\0';

    /* # is an alias for not-equal */
    if (op1 == '#') {
        (*p)++;
        rhs = parse_expression(p);
        return lhs != rhs;
    }

    if (op1 == '=' || op1 == '<' || op1 == '>') {
        (*p)++;
        if (**p == '>' || **p == '=') { op2 = **p; (*p)++; }
    } else {
        printf("EXPECTED RELATIONAL OPERATOR\n");
        running = 0;
        return 0;
    }

    rhs = parse_expression(p);

    if (op1 == '=' && op2 == '\0') return lhs == rhs;
    if (op1 == '<' && op2 == '>')  return lhs != rhs;
    if (op1 == '<' && op2 == '=')  return lhs <= rhs;
    if (op1 == '<' && op2 == '\0') return lhs < rhs;
    if (op1 == '>' && op2 == '=')  return lhs >= rhs;
    if (op1 == '>' && op2 == '\0') return lhs > rhs;

    return 0;
}

/* =========================================================================
 * STATEMENT EXECUTION
 *
 * Parses and dispatches a single BASIC statement.
 *
 * WHAT CAN BE CHANGED:
 *   - Add new statements by adding match_keyword() cases.
 *   - PRINT formatting (tab stops, spacing) can be adjusted.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The uppercase conversion of the command keyword at entry.
 *   - LET must remain optional (implicit LET is a dialect feature).
 *   - GOSUB/RETURN must use the stack (not recursion).
 *   - RUN with a filename must produce SYNTAX ERROR.
 *
 * WHAT TO EXPECT:
 *   - Unrecognized statements produce SYNTAX ERROR.
 *   - PRINT with trailing ; suppresses the newline.
 *   - PRINT with , advances to the next 8-column tab stop.
 *   - Numeric values are printed with a leading space for positive
 *     numbers and a trailing space after all numbers.
 *
 * IF SOMETHING BREAKS:
 *   - If a statement is not recognized, verify the uppercase
 *     conversion runs before match_keyword().
 *   - If GOTO loops infinitely, check that program_counter is set
 *     correctly and execute_line does not advance past it.
 * ========================================================================= */

static void execute_line(char *line) {
    char *p = line;
    skip_spaces(&p);
    if (*p == '\0') return;

    /* Input is pre-uppercased by uppercase_line() in the REPL */

    /* --- PRINT --- */
    if (match_keyword(&p, "PRINT") || match_keyword(&p, "PR")) {
        int need_newline = 1;
        skip_spaces(&p);
        if (*p == '\0') { printf("\n"); return; }

        while (*p != '\0') {
            skip_spaces(&p);
            if (*p == '\0') break;

            if (*p == '"') {
                char *end;
                p++;
                end = strchr(p, '"');
                if (end) {
                    *end = '\0';
                    printf("%s", p);
                    *end = '"';
                    p = end + 1;
                } else {
                    printf("%s", p);
                    p += strlen(p);
                }
                need_newline = 1;
            } else if (*p == ',') {
                printf("\t");
                p++;
                need_newline = 0;
            } else if (*p == ';') {
                p++;
                need_newline = 0;
            } else {
                short val = parse_expression(&p);
                if (!running) return;
                if (val >= 0) printf(" ");
                printf("%d ", (int)val);
                need_newline = 1;
            }
        }
        if (need_newline) printf("\n");

    /* --- LET (explicit keyword) --- */
    } else if (match_keyword(&p, "LET")) {
        int var_idx;
        skip_spaces(&p);

        /* @() array assignment */
        if (*p == '@') {
            short idx;
            p++;
            skip_spaces(&p);
            if (*p == '(') p++;
            idx = parse_expression(&p);
            skip_spaces(&p);
            if (*p == ')') p++;
            skip_spaces(&p);
            if (*p == '=') p++;
            if (idx < 0 || idx >= MAX_VARS) {
                printf("ARRAY BOUNDS ERROR\n");
                running = 0;
                return;
            }
            array_mem[idx] = parse_expression(&p);
            return;
        }

        if (!isalpha((unsigned char)*p)) {
            printf("EXPECTED VARIABLE\n");
            running = 0;
            return;
        }
        var_idx = toupper((unsigned char)*p) - 'A';
        p++;
        skip_spaces(&p);
        if (*p == '=') p++;
        variables[var_idx] = parse_expression(&p);

    /* --- INPUT --- */
    } else if (match_keyword(&p, "INPUT")) {
        skip_spaces(&p);
        while (isalpha((unsigned char)*p)) {
            int var_idx = toupper((unsigned char)*p) - 'A';
            char buf[32];
            p++;
            printf("? ");
            fflush(stdout);
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                variables[var_idx] = (short)atoi(buf);
            }
            skip_spaces(&p);
            if (*p == ',') { p++; skip_spaces(&p); }
        }

    /* --- GOTO --- */
    } else if (match_keyword(&p, "GOTO")) {
        int target = (int)parse_expression(&p);
        int idx = find_line(target);
        if (idx < 0) {
            printf("LINE NOT FOUND\n");
            running = 0;
        } else {
            program_counter = idx;
        }

    /* --- GOSUB --- */
    } else if (match_keyword(&p, "GOSUB")) {
        int target, idx;
        if (stack_ptr >= STACK_SIZE) {
            printf("GOSUB STACK OVERFLOW\n");
            running = 0;
            return;
        }
        target = (int)parse_expression(&p);
        idx = find_line(target);
        if (idx < 0) {
            printf("LINE NOT FOUND\n");
            running = 0;
        } else {
            gosub_stack[stack_ptr++] = program_counter + 1;
            program_counter = idx;
        }

    /* --- RETURN --- */
    } else if (match_keyword(&p, "RETURN")) {
        if (stack_ptr <= 0) {
            printf("RETURN WITHOUT GOSUB\n");
            running = 0;
        } else {
            program_counter = gosub_stack[--stack_ptr];
        }

    /* --- IF/THEN --- */
    } else if (match_keyword(&p, "IF")) {
        int cond;
        skip_spaces(&p);
        cond = eval_condition(&p);
        if (!running) return;
        skip_spaces(&p);

        /* THEN keyword is already uppercased by uppercase_line() */
        if (strncmp(p, "THEN", 4) == 0 && !isalpha((unsigned char)p[4])) {
            p += 4;
        }
        skip_spaces(&p);

        if (cond) {
            if (isdigit((unsigned char)*p)) {
                int target = atoi(p);
                int idx = find_line(target);
                if (idx < 0) {
                    printf("LINE NOT FOUND\n");
                    running = 0;
                } else {
                    program_counter = idx;
                }
            } else {
                execute_line(p);
            }
        }

    /* --- REM --- */
    } else if (match_keyword(&p, "REM")) {
        /* Comment: do nothing */

    /* --- RUN (no filename allowed) --- */
    } else if (match_keyword(&p, "RUN")) {
        skip_spaces(&p);
        if (*p != '\0') {
            printf("SYNTAX ERROR\n");
        } else {
            run_program();
        }

    /* --- LIST --- */
    } else if (match_keyword(&p, "LIST")) {
        list_program();

    /* --- NEW --- */
    } else if (match_keyword(&p, "NEW")) {
        clear_all();
        printf("Memory Cleared.\n");

    /* --- CLEAR --- */
    } else if (match_keyword(&p, "CLEAR")) {
        clear_variables();
        printf("Variables Cleared.\n");

    /* --- SAVE --- */
    } else if (match_keyword(&p, "SAVE")) {
        skip_spaces(&p);
        save_program(p);

    /* --- LOAD --- */
    } else if (match_keyword(&p, "LOAD")) {
        skip_spaces(&p);
        load_program(p);

    /* --- HELP --- */
    } else if (match_keyword(&p, "HELP")) {
        cmd_help();

    /* --- BYE --- */
    } else if (match_keyword(&p, "BYE")) {
        exit(0);

    /* --- STOP / END --- */
    } else if (match_keyword(&p, "STOP") || match_keyword(&p, "END")) {
        running = 0;

    /* --- BEEP --- */
    } else if (match_keyword(&p, "BEEP")) {
        trigger_beep();

    } else {
        printf("SYNTAX ERROR\n");
    }
}

/* =========================================================================
 * PROGRAM EXECUTION
 *
 * WHAT CAN BE CHANGED:
 *   - The clear_variables() call at RUN start could be made optional
 *     (some BASIC dialects preserve variables across RUN).
 *
 * WHAT CANNOT BE CHANGED:
 *   - GOTO/GOSUB set program_counter; the runner must check for
 *     this and NOT auto-advance when the counter has changed.
 *
 * WHAT TO EXPECT:
 *   - Execution begins at the first stored line.
 *   - Setting running=0 stops execution after the current line.
 *
 * IF SOMETHING BREAKS:
 *   - If the program runs infinitely, check that END/STOP set
 *     running=0.
 *   - If GOTO skips lines, check that find_line() returns the
 *     correct index.
 * ========================================================================= */

static void run_program(void) {
    char line_buf[LINE_LEN + 1];
    int prev_pc;

    if (num_lines == 0) return;

    clear_variables();
    program_counter = 0;
    running = 1;

    while (running && program_counter < num_lines) {
        prev_pc = program_counter;
        strncpy(line_buf, program[program_counter].text, LINE_LEN);
        line_buf[LINE_LEN] = '\0';
        execute_line(line_buf);
        if (running && program_counter == prev_pc) program_counter++;
    }
    running = 0;
}

static void list_program(void) {
    int i;
    for (i = 0; i < num_lines; i++) {
        printf("%d %s\n", program[i].line_number, program[i].text);
    }
}

/* =========================================================================
 * HELP COMMAND
 *
 * Lightweight built-in help that lists all implemented keywords.
 *
 * WHAT CAN BE CHANGED:
 *   - Update the listing whenever a keyword is added or removed.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Must always reflect the actual implemented feature set.
 *
 * WHAT TO EXPECT:
 *   - Prints a compact reference card to stdout.
 *
 * IF SOMETHING BREAKS:
 *   - This is pure output; it cannot cause logic errors.
 * ========================================================================= */

static void cmd_help(void) {
    printf("\n");
    printf("Palo Alto Tiny BASIC (C89 Port) v1.2\n");
    printf("=====================================\n");
    printf("Statements:  PRINT  LET  INPUT  GOTO  GOSUB  RETURN\n");
    printf("             IF/THEN  REM  END  STOP  CLEAR  BEEP\n");
    printf("Commands:    RUN  LIST  NEW  SAVE  LOAD  HELP  BYE\n");
    printf("Functions:   ABS()  RND()  SIZE\n");
    printf("Operators:   +  -  *  /  =  <  >  <=  >=  <>  #\n");
    printf("\n");
}

/* =========================================================================
 * FILE I/O
 *
 * Programs are saved as plain text files.  Each line is written as:
 *   <line_number> <text>\n
 *
 * WHAT CAN BE CHANGED:
 *   - The default extension (.BAS) can be changed to another.
 *   - The maximum filename length follows LINE_LEN.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Filenames can be given with or without quotes.
 *   - The .BAS extension is auto-appended if not already present.
 *   - LOAD clears the existing program before loading.
 *
 * WHAT TO EXPECT:
 *   - SAVE test   -> writes test.BAS
 *   - SAVE "test" -> writes test.BAS
 *   - SAVE test.BAS -> writes test.BAS (no double extension)
 *   - LOAD test   -> reads test.BAS
 *
 * IF SOMETHING BREAKS:
 *   - If SAVE creates a 0-byte file, check that num_lines > 0.
 *   - If LOAD fails, verify the filename is correct and the file
 *     exists in the current working directory.
 *   - If filenames have trailing quotes, check resolve_filename().
 * ========================================================================= */

/*
 * resolve_filename
 * Strips optional quotes, trims whitespace, and auto-appends .BAS
 * if the filename does not already end in .BAS or .bas.
 */
static void resolve_filename(const char *raw, char *out, int out_size) {
    const char *src;
    int len;

    src = raw;
    while (*src == ' ' || *src == '\t') src++;

    /* Strip leading quote */
    if (*src == '"') src++;

    strncpy(out, src, out_size - 5);
    out[out_size - 5] = '\0';
    trim_newline(out);

    /* Strip trailing quote */
    len = (int)strlen(out);
    if (len > 0 && out[len - 1] == '"') { out[len - 1] = '\0'; len--; }

    /* Strip trailing whitespace */
    while (len > 0 && (out[len - 1] == ' ' || out[len - 1] == '\t')) {
        out[len - 1] = '\0';
        len--;
    }

    if (len == 0) return;

    /* Check if already ends in .BAS or .bas (case-insensitive) */
    if (len >= 4) {
        char e0 = out[len - 4];
        char e1 = (char)toupper((unsigned char)out[len - 3]);
        char e2 = (char)toupper((unsigned char)out[len - 2]);
        char e3 = (char)toupper((unsigned char)out[len - 1]);
        if (e0 == '.' && e1 == 'B' && e2 == 'A' && e3 == 'S') {
            return;  /* Already has .BAS extension */
        }
    }

    /* Auto-append .BAS */
    if (len + 4 < out_size) {
        strcat(out, ".BAS");
    }
}

static void save_program(const char *filename) {
    FILE *fp;
    char fname[LINE_LEN + 1];
    int i;

    if (filename == NULL || *filename == '\0') {
        printf("FILENAME REQUIRED\n");
        return;
    }

    resolve_filename(filename, fname, sizeof(fname));
    if (fname[0] == '\0') { printf("FILENAME REQUIRED\n"); return; }

    fp = fopen(fname, "w");
    if (fp) {
        for (i = 0; i < num_lines; i++) {
            fprintf(fp, "%d %s\n", program[i].line_number, program[i].text);
        }
        fclose(fp);
        printf("OK\n");
    } else {
        printf("FILE ERROR\n");
    }
}

static void load_program(const char *filename) {
    FILE *fp;
    char fname[LINE_LEN + 1];
    char line_buf[LINE_LEN + 32];

    if (filename == NULL || *filename == '\0') {
        printf("FILENAME REQUIRED\n");
        return;
    }

    resolve_filename(filename, fname, sizeof(fname));
    if (fname[0] == '\0') { printf("FILENAME REQUIRED\n"); return; }

    fp = fopen(fname, "r");
    if (fp) {
        clear_all();
        while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
            trim_newline(line_buf);
            if (line_buf[0] != '\0') store_line(line_buf);
        }
        fclose(fp);
        printf("OK\n");
    } else {
        printf("FILE NOT FOUND\n");
    }
}

/* =========================================================================
 * MAIN / REPL
 *
 * The read-eval-print loop.
 *
 * WHAT CAN BE CHANGED:
 *   - The prompt string "> " can be changed to any format.
 *   - The banner text and version number.
 *
 * WHAT CANNOT BE CHANGED:
 *   - srand() must be called before any RND() usage.
 *   - Lines with a leading digit must be stored, not executed.
 *   - Lines without a leading digit must be executed immediately.
 *   - EOF on stdin must exit cleanly.
 *
 * WHAT TO EXPECT:
 *   - The interpreter prints the banner at startup.
 *   - Empty lines are silently ignored.
 *   - The loop runs until BYE is typed or EOF is received.
 *
 * IF SOMETHING BREAKS:
 *   - If the prompt does not appear, check fflush(stdout).
 *   - If input is garbled, check the newline stripping code.
 * ========================================================================= */

int main(void) {
    char input_buffer[LINE_LEN + 1];

    srand((unsigned int)time(NULL));
    clear_all();
    printf("Palo Alto Tiny BASIC (C89 Port) v1.2\n");
    printf("Ready.\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        trim_newline(input_buffer);
        uppercase_line(input_buffer);
        if (input_buffer[0] == '\0') continue;

        if (isdigit((unsigned char)input_buffer[0])) {
            store_line(input_buffer);
        } else {
            running = 1;
            execute_line(input_buffer);
            running = 0;
        }
    }
    return 0;
}

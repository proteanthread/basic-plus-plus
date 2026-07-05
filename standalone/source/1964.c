/*
 * PROJECT:  Dartmouth BASIC 1964 Interpreter (C89 Port)
 * FILENAME: 1964.c
 * VERSION:  1.0.0
 *
 * Prototype: based on the original Dartmouth BASIC by Kemeny and Kurtz (1964)
 *
 * DESCRIPTION:
 *   A strictly compliant ANSI C89 interpreter for the original 1964 dialect
 *   of Dartmouth BASIC. Features strict floating point arithmetic, historic
 *   unary minus precedence, A-Z and A0-Z9 variables, 1D and 2D arrays, 
 *   single-line DEF functions, and historic interactive commands.
 *
 * DIALECT NOTES (Dartmouth BASIC 1964):
 *   - Variables: A-Z and A0-Z9 (all strictly floating-point)
 *   - Arrays: A-Z (1D and 2D arrays via DIM)
 *   - Statements: LET, PRINT, END, READ, DATA, GOTO, IF/THEN, FOR/NEXT,
 *                 GOSUB, RETURN, DEF, DIM, REM, STOP
 *   - Functions: SIN, COS, TAN, ATN, EXP, LOG, ABS, SQR, RND, INT
 *   - DEF functions: FNA through FNZ (single-line definition)
 *   - Operators: + - * / ^ (Exponentiation)
 *   - Relational: = < > <= >= <>
 *   - Historic Precedence: Unary minus binds tighter than ^ (-X^2 is (-X)^2)
 *   - LET keyword is mandatory
 *   - Strings are ONLY allowed as literals in PRINT statements
 *   - Built-in HELP command added for project parity
 *   - Interactive Commands: HELLO, NEW, OLD, SAVE, REPLACE, UNSAVE,
 *                           LIST, RUN, CATALOG/CAT, SCRATCH, GOODBYE/BYE
 *
 * HOW TO COMPILE:
 *   MSVC:  cl /TC /W4 /WX /Za /O2 /D_CRT_SECURE_NO_WARNINGS 1964.c
 *   GCC:   gcc -ansi -Wall -Wextra -Werror -pedantic -O2 -o 1964 1964.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
  #include <io.h>
  #define F_OK 0
  #define access _access
#else
  /* Strict C89 lacks access/remove declarations in stdio sometimes, so we declare them if missing or use fopen */
#endif

/* =========================================================================
 * STANDARD LIBRARY INCLUDES
 *
 * WHAT CAN BE CHANGED:
 *   Nothing here. All headers are required for C89 math and I/O.
 *
 * WHAT CANNOT BE CHANGED:
 *   Do not add C99 or POSIX headers (stdint.h, stdbool.h, unistd.h).
 *   The file must remain strict C89 for maximum portability.
 *
 * WHAT TO EXPECT:
 *   <time.h> is used solely for srand(time(NULL)) at startup.
 *   <math.h> provides the historic math functions.
 *
 * IF SOMETHING BREAKS:
 *   If you get "implicit declaration" warnings, verify all
 *   includes are present and the compiler is in C89 mode.
 * ========================================================================= */

#define MAX_LINES        2000
#define LINE_LEN         255
#define MAX_VARS         286    /* 26 (A-Z) + 26*10 (A0-Z9) */
#define ARRAY_MEM_SIZE   65536
#define STACK_SIZE       128
#define MAX_DATA         4000
#define MAX_FUNCS        26

/* =========================================================================
 * DATA STRUCTURES
 *
 * WHAT CAN BE CHANGED:
 *   - ProgramLine, ArrayDesc, ForLoop, StackFrame, and DefFunc can be
 *     extended to track additional features if the dialect expands.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Line numbers must remain integers to support ranges up to 99999.
 *   - The union in StackFrame must properly manage memory for
 *     both FOR loops and GOSUB returns.
 *
 * WHAT TO EXPECT:
 *   The stack handles both historic subroutine jumps and nested
 *   FOR/NEXT iterations seamlessly.
 *
 * IF SOMETHING BREAKS:
 *   If struct alignment padding causes memory bloat on embedded targets,
 *   consider using compiler pragmas to pack the structs.
 * ========================================================================= */

typedef struct {
    int line_number;
    char text[LINE_LEN + 1];
} ProgramLine;

typedef struct {
    int dim1;
    int dim2;
    int offset;
} ArrayDesc;

typedef struct {
    int var_idx;       /* Variable being iterated */
    double limit;      /* Target limit */
    double step;       /* Step value */
    int loop_line;     /* Line number of FOR statement */
    char *loop_ptr;    /* Pointer to text after FOR statement (if needed) */
} ForLoop;

typedef struct {
    int is_for;
    union {
        int ret_line;  /* For GOSUB */
        ForLoop for_loop;
    } u;
} StackFrame;

typedef struct {
    int param_var_idx; /* Parameter variable index (e.g. X for FNA(X)) */
    char expr[LINE_LEN + 1];
} DefFunc;

/* =========================================================================
 * STATIC MEMORY
 *
 * WHAT CAN BE CHANGED:
 *   - Array sizes can be adjusted based on the constants above.
 *
 * WHAT CANNOT BE CHANGED:
 *   - These must remain static/global (BSS sector) to avoid stack 
 *     overflow on memory-limited platforms and guarantee zero initialization.
 *
 * WHAT TO EXPECT:
 *   All interpreter state, program code, and dynamic variable data
 *   reside completely in these pre-allocated BSS blocks.
 *
 * IF SOMETHING BREAKS:
 *   If the program crashes at startup on small embedded systems, the
 *   static arrays may exceed available RAM. Reduce MAX_LINES or MAX_DATA.
 * ========================================================================= */

static ProgramLine program[MAX_LINES];
static int num_lines = 0;

static double variables[MAX_VARS];
static ArrayDesc arrays[26];
static double array_data[ARRAY_MEM_SIZE];
static int array_free = 0;

static StackFrame control_stack[STACK_SIZE];
static int stack_ptr = 0;

static double data_block[MAX_DATA];
static int data_count = 0;
static int data_ptr = 0;

static DefFunc user_funcs[MAX_FUNCS];
static int def_active = 0; /* Flag to detect recursion if needed */

static int program_counter = 0;
static int running = 0;
static char current_filename[256] = "";

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

static double parse_expression(char **p);
static double parse_term(char **p);
static double parse_power(char **p);
static double parse_factor(char **p);
static int    eval_condition(char **p);

static int   parse_var_index(char **p);
static double* get_array_element(char **p);

static void  execute_line(char *line);
static void  run_program(void);
static void  list_program(void);
static void  cmd_help(void);
static void  resolve_filename(const char *raw, char *out, int out_size);
static void  save_program(const char *filename);
static void  load_program(const char *filename);
static void  build_data_block(void);

/* =========================================================================
 * UTILITY FUNCTIONS
 *
 * WHAT CAN BE CHANGED:
 *   - Whitespace stripping and case conversion can be augmented
 *     to handle other control characters.
 *
 * WHAT CANNOT BE CHANGED:
 *   - match_keyword must correctly enforce word boundaries so that
 *     "FORMAT" does not trigger the "FOR" handler.
 *
 * WHAT TO EXPECT:
 *   These are foundational helpers for parsing keywords safely and
 *   sanitizing incoming terminal streams.
 *
 * IF SOMETHING BREAKS:
 *   If file IO breaks, check file_exists logic on non-Windows platforms.
 * ========================================================================= */

static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

static void skip_spaces(char **p) {
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

static int match_keyword(char **p, const char *kw) {
    size_t len = strlen(kw);
    if (strncmp(*p, kw, len) == 0) {
        if (!isalpha((unsigned char)((*p)[len])) && (*p)[len] != '$') {
            *p += len;
            skip_spaces(p);
            return 1;
        }
    }
    return 0;
}

static void uppercase_input(char *str) {
    int in_quotes = 0;
    while (*str) {
        if (*str == '"') {
            in_quotes = !in_quotes;
        }
        if (!in_quotes) {
            *str = (char)toupper((unsigned char)*str);
        }
        str++;
    }
}

static int file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

static void remove_file(const char *filename) {
    remove(filename);
}

/* =========================================================================
 * CORE SYSTEM MANAGEMENT
 *
 * WHAT CAN BE CHANGED:
 *   - Line search algorithm (binary search) is fast, but could be
 *     replaced with a linked list or hash map if memory permits.
 *
 * WHAT CANNOT BE CHANGED:
 *   - clear_all must zero out the variables array completely.
 *
 * WHAT TO EXPECT:
 *   Handles clearing the machine state, maintaining the ordered
 *   program line buffer, and inserting/deleting statements.
 *
 * IF SOMETHING BREAKS:
 *   If lines are out of order, verify that the insertion shift loop
 *   (store_line) is operating correctly over the ProgramLine buffer.
 * ========================================================================= */

static void clear_variables(void) {
    int i;
    for (i = 0; i < MAX_VARS; i++) {
        variables[i] = 0.0;
    }
    for (i = 0; i < 26; i++) {
        arrays[i].dim1 = 10;
        arrays[i].dim2 = 0;
        arrays[i].offset = -1;
    }
    array_free = 0;
    stack_ptr = 0;
    data_count = 0;
    data_ptr = 0;
    for (i = 0; i < MAX_FUNCS; i++) {
        user_funcs[i].param_var_idx = -1;
        user_funcs[i].expr[0] = '\0';
    }
}

static void clear_all(void) {
    num_lines = 0;
    clear_variables();
}

static int find_line(int line_num) {
    int low = 0;
    int high = num_lines - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (program[mid].line_number == line_num) {
            return mid;
        } else if (program[mid].line_number < line_num) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

static void delete_line(int line_num) {
    int idx = find_line(line_num);
    if (idx != -1) {
        int i;
        for (i = idx; i < num_lines - 1; i++) {
            program[i] = program[i + 1];
        }
        num_lines--;
    }
}

static void store_line(const char *raw) {
    char *p = (char *)raw;
    int line_num;
    int insert_idx = 0;
    int i;

    skip_spaces(&p);
    if (!isdigit((unsigned char)*p)) {
        return;
    }

    line_num = atoi(p);
    if (line_num < 1 || line_num > 99999) {
        printf("LINE NUMBER OUT OF RANGE\n");
        return;
    }

    while (isdigit((unsigned char)*p)) {
        p++;
    }
    skip_spaces(&p);

    delete_line(line_num);

    if (*p == '\0') {
        return;
    }

    if (num_lines >= MAX_LINES) {
        printf("MEMORY FULL\n");
        return;
    }

    while (insert_idx < num_lines && program[insert_idx].line_number < line_num) {
        insert_idx++;
    }

    for (i = num_lines; i > insert_idx; i--) {
        program[i] = program[i - 1];
    }

    program[insert_idx].line_number = line_num;
    strncpy(program[insert_idx].text, p, LINE_LEN);
    program[insert_idx].text[LINE_LEN] = '\0';
    num_lines++;
}

/* =========================================================================
 * EXPRESSION PARSER
 *
 * WHAT CAN BE CHANGED:
 *   - New built-in functions can be added into parse_factor.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Historic Kemeny & Kurtz Dartmouth precedence MUST be preserved
 *     (unary minus binds tighter than exponentiation).
 *   - All values return type 'double'.
 *
 * WHAT TO EXPECT:
 *   A standard recursive descent parser modified for the historical
 *   quirks of 1964 BASIC, supporting 1D/2D arrays and FN functions.
 *
 * IF SOMETHING BREAKS:
 *   If math evaluates incorrectly, ensure parse_power vs parse_factor
 *   hierarchy accurately reflects unary minus priority.
 * ========================================================================= */

static int parse_var_index(char **p) {
    int idx = -1;
    if (isalpha((unsigned char)**p)) {
        int letter = toupper((unsigned char)**p) - 'A';
        (*p)++;
        if (isdigit((unsigned char)**p)) {
            int digit = **p - '0';
            idx = 26 + letter * 10 + digit;
            (*p)++;
        } else {
            idx = letter;
        }
    }
    return idx;
}

static double* get_array_element(char **p) {
    int letter;
    double idx1, idx2;
    int i1, i2;
    ArrayDesc *arr;
    
    if (!isalpha((unsigned char)**p)) return NULL;
    letter = toupper((unsigned char)**p) - 'A';
    (*p)++;
    
    skip_spaces(p);
    if (**p != '(') {
        printf("SYNTAX ERROR: EXPECTED (\n");
        running = 0;
        return NULL;
    }
    (*p)++;
    idx1 = parse_expression(p);
    i1 = (int)idx1;
    i2 = 0;
    
    skip_spaces(p);
    if (**p == ',') {
        (*p)++;
        idx2 = parse_expression(p);
        i2 = (int)idx2;
    }
    
    skip_spaces(p);
    if (**p != ')') {
        printf("SYNTAX ERROR: EXPECTED )\n");
        running = 0;
        return NULL;
    }
    (*p)++;
    
    arr = &arrays[letter];
    if (i1 < 0 || i1 > arr->dim1 || (arr->dim2 > 0 && (i2 < 0 || i2 > arr->dim2))) {
        printf("SUBSCRIPT OUT OF RANGE\n");
        running = 0;
        return NULL;
    }
    
    if (arr->offset == -1) {
        int size = (arr->dim1 + 1) * (arr->dim2 == 0 ? 1 : (arr->dim2 + 1));
        if (array_free + size > ARRAY_MEM_SIZE) {
            printf("MEMORY FULL\n");
            running = 0;
            return NULL;
        }
        arr->offset = array_free;
        array_free += size;
    }
    
    if (arr->dim2 == 0) {
        return &array_data[arr->offset + i1];
    } else {
        return &array_data[arr->offset + i1 * (arr->dim2 + 1) + i2];
    }
}

static double parse_factor(char **p) {
    double val = 0.0;
    skip_spaces(p);

    if (**p == '-') {
        /* Historic Dartmouth unary minus: binds tighter than exponentiation */
        (*p)++;
        return -parse_factor(p);
    } else if (**p == '+') {
        (*p)++;
        return parse_factor(p);
    }

    if (**p == '(') {
        (*p)++;
        val = parse_expression(p);
        skip_spaces(p);
        if (**p == ')') {
            (*p)++;
        } else {
            printf("SYNTAX ERROR: MISSING PARENTHESIS\n");
            running = 0;
        }
    } else if (isdigit((unsigned char)**p) || **p == '.') {
        char *end;
        val = strtod(*p, &end);
        *p = end;
    } else if (isalpha((unsigned char)**p)) {
        /* Check for Math Functions or DEF FNA-FNZ */
        if (match_keyword(p, "SIN(")) { val = sin(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "COS(")) { val = cos(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "TAN(")) { val = tan(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "ATN(")) { val = atan(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "EXP(")) { val = exp(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "LOG(")) { val = log(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "ABS(")) { val = fabs(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "SQR(")) { 
            double arg = parse_expression(p);
            if (arg < 0) { printf("SQR OF NEGATIVE NUMBER\n"); running = 0; }
            else val = sqrt(arg);
            if (**p == ')') (*p)++;
        }
        else if (match_keyword(p, "INT(")) { val = floor(parse_expression(p)); if (**p == ')') (*p)++; }
        else if (match_keyword(p, "RND(")) {
            parse_expression(p); /* Dartmouth RND takes a dummy argument */
            if (**p == ')') (*p)++;
            val = (double)rand() / (double)RAND_MAX;
        }
        else if (strncmp(*p, "FN", 2) == 0 && isalpha((unsigned char)((*p)[2]))) {
            int fn_idx = toupper((unsigned char)((*p)[2])) - 'A';
            *p += 3;
            skip_spaces(p);
            if (**p != '(') {
                printf("SYNTAX ERROR: EXPECTED ( AFTER FN\n");
                running = 0;
            } else {
                (*p)++;
                val = parse_expression(p);
                if (**p == ')') (*p)++;
                else { printf("SYNTAX ERROR: MISSING )\n"); running = 0; }
                
                if (user_funcs[fn_idx].param_var_idx == -1) {
                    printf("UNDEFINED FUNCTION\n");
                    running = 0;
                } else if (def_active) {
                    printf("RECURSIVE FUNCTION CALL NOT SUPPORTED\n");
                    running = 0;
                } else {
                    double old_val = variables[user_funcs[fn_idx].param_var_idx];
                    char *expr_ptr = user_funcs[fn_idx].expr;
                    variables[user_funcs[fn_idx].param_var_idx] = val;
                    def_active = 1;
                    val = parse_expression(&expr_ptr);
                    def_active = 0;
                    variables[user_funcs[fn_idx].param_var_idx] = old_val;
                }
            }
        }
        else {
            char *tmp = *p;
            int is_array = 0;
            
            if (isalpha((unsigned char)*tmp)) {
                tmp++;
                skip_spaces(&tmp);
                if (*tmp == '(') is_array = 1;
            }
            
            if (is_array) {
                double *el = get_array_element(p);
                if (el) val = *el;
            } else {
                int idx = parse_var_index(p);
                if (idx >= 0 && idx < MAX_VARS) {
                    val = variables[idx];
                } else {
                    printf("SYNTAX ERROR: INVALID VARIABLE\n");
                    running = 0;
                }
            }
        }
    } else {
        printf("SYNTAX ERROR: EXPRESSION\n");
        running = 0;
    }
    
    skip_spaces(p);
    return val;
}

static double parse_power(char **p) {
    double val = parse_factor(p);
    skip_spaces(p);
    while (**p == '^') {
        (*p)++;
        val = pow(val, parse_factor(p));
        skip_spaces(p);
    }
    return val;
}

static double parse_term(char **p) {
    double val = parse_power(p);
    skip_spaces(p);
    while (**p == '*' || **p == '/') {
        char op = **p;
        (*p)++;
        if (op == '*') {
            val *= parse_power(p);
        } else {
            double divisor = parse_power(p);
            if (divisor == 0.0) {
                printf("DIVISION BY ZERO\n");
                running = 0;
            } else {
                val /= divisor;
            }
        }
        skip_spaces(p);
    }
    return val;
}

static double parse_expression(char **p) {
    double val = parse_term(p);
    skip_spaces(p);
    while (**p == '+' || **p == '-') {
        char op = **p;
        (*p)++;
        if (op == '+') {
            val += parse_term(p);
        } else {
            val -= parse_term(p);
        }
        skip_spaces(p);
    }
    return val;
}

static int eval_condition(char **p) {
    double left, right;
    char op[3] = {0};
    
    left = parse_expression(p);
    skip_spaces(p);
    
    if (**p == '<' || **p == '>' || **p == '=') {
        op[0] = **p;
        (*p)++;
        if ((op[0] == '<' && (**p == '>' || **p == '=')) || 
            (op[0] == '>' && **p == '=')) {
            op[1] = **p;
            (*p)++;
        }
    } else {
        printf("SYNTAX ERROR: EXPECTED RELATIONAL OPERATOR\n");
        running = 0;
        return 0;
    }
    
    right = parse_expression(p);
    
    if (strcmp(op, "=") == 0) return left == right;
    if (strcmp(op, "<") == 0) return left < right;
    if (strcmp(op, ">") == 0) return left > right;
    if (strcmp(op, "<=") == 0) return left <= right;
    if (strcmp(op, ">=") == 0) return left >= right;
    if (strcmp(op, "<>") == 0) return left != right;
    
    return 0;
}

/* =========================================================================
 * DATA BLOCK BUILDER
 *
 * WHAT CAN BE CHANGED:
 *   - The logic could be extended to allow string DATA if the dialect
 *     gains string variables.
 *
 * WHAT CANNOT BE CHANGED:
 *   - DATA statements must be extracted before execution begins.
 *   - The data pointer must be reset so READ statements start from the beginning.
 *
 * WHAT TO EXPECT:
 *   Scans the entire program for DATA keywords, evaluates the 
 *   values immediately, and loads them into a flat double array.
 *
 * IF SOMETHING BREAKS:
 *   If READ fails with OUT OF DATA, check the comma parsing in this
 *   routine, as it may be failing on trailing commas or bad numbers.
 * ========================================================================= */

static void build_data_block(void) {
    int i;
    data_count = 0;
    data_ptr = 0;
    
    for (i = 0; i < num_lines; i++) {
        char *p = program[i].text;
        skip_spaces(&p);
        if (match_keyword(&p, "DATA")) {
            while (*p && running) {
                double val;
                int sign = 1;
                skip_spaces(&p);
                if (*p == '-') { sign = -1; p++; }
                else if (*p == '+') { p++; }
                
                if (!isdigit((unsigned char)*p) && *p != '.') {
                    printf("SYNTAX ERROR IN DATA LINE %d\n", program[i].line_number);
                    running = 0;
                    return;
                }
                
                val = sign * parse_expression(&p);
                if (data_count < MAX_DATA) {
                    data_block[data_count++] = val;
                } else {
                    printf("TOO MUCH DATA\n");
                    running = 0;
                    return;
                }
                
                skip_spaces(&p);
                if (*p == ',') {
                    p++;
                } else if (*p != '\0') {
                    printf("SYNTAX ERROR IN DATA LINE %d\n", program[i].line_number);
                    running = 0;
                    return;
                }
            }
        }
    }
}

/* =========================================================================
 * EXECUTION ENGINE
 *
 * WHAT CAN BE CHANGED:
 *   - The execution loop can be throttled or hooked into a GUI pump
 *     for SDL environments if required in the future.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Commands like LET, PRINT, GOTO must remain modular to avoid
 *     C89 strict variable declaration scoping issues inside large switch blocks.
 *
 * WHAT TO EXPECT:
 *   The core dispatcher. Parses the first keyword and routes execution.
 *
 * IF SOMETHING BREAKS:
 *   If the program runs into an infinite loop, verify that GOTO, FOR,
 *   and NEXT are properly updating program_counter.
 * ========================================================================= */

static void execute_line(char *line) {
    char *p = line;
    skip_spaces(&p);

    if (*p == '\0') return;

    if (match_keyword(&p, "REM")) {
        return;
    } 
    else if (match_keyword(&p, "LET")) {
        int is_array = 0;
        char *tmp = p;
        if (isalpha((unsigned char)*tmp)) {
            tmp++;
            skip_spaces(&tmp);
            if (*tmp == '(') is_array = 1;
        }
        
        if (is_array) {
            double *el = get_array_element(&p);
            if (!el) return;
            skip_spaces(&p);
            if (*p == '=') {
                p++;
                *el = parse_expression(&p);
            } else {
                printf("SYNTAX ERROR: EXPECTED = IN LET\n");
                running = 0;
            }
        } else {
            int idx = parse_var_index(&p);
            if (idx != -1) {
                skip_spaces(&p);
                if (*p == '=') {
                    p++;
                    variables[idx] = parse_expression(&p);
                } else {
                    printf("SYNTAX ERROR: EXPECTED = IN LET\n");
                    running = 0;
                }
            } else {
                printf("SYNTAX ERROR: INVALID VARIABLE\n");
                running = 0;
            }
        }
    }
    else if (match_keyword(&p, "PRINT")) {
        int suppress_nl = 0;
        skip_spaces(&p);
        if (*p == '\0') {
            printf("\n");
            return;
        }
        while (*p) {
            suppress_nl = 0;
            skip_spaces(&p);
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    putchar(*p);
                    p++;
                }
                if (*p == '"') p++;
            } else {
                double val = parse_expression(&p);
                if (val == floor(val)) {
                    printf("%g ", val);
                } else {
                    printf("%f ", val);
                }
            }
            skip_spaces(&p);
            if (*p == ';') {
                suppress_nl = 1;
                p++;
            } else if (*p == ',') {
                suppress_nl = 1;
                printf("\t");
                p++;
            } else if (*p != '\0') {
                printf("SYNTAX ERROR: EXPECTED , OR ;\n");
                running = 0;
                return;
            }
        }
        if (!suppress_nl) {
            printf("\n");
        }
    }
    else if (match_keyword(&p, "GOTO")) {
        int target = (int)parse_expression(&p);
        int idx = find_line(target);
        if (idx != -1) {
            program_counter = idx;
        } else {
            printf("UNDEFINED LINE NUMBER %d\n", target);
            running = 0;
        }
    }
    else if (match_keyword(&p, "IF")) {
        int cond = eval_condition(&p);
        if (!running) return;
        skip_spaces(&p);
        if (match_keyword(&p, "THEN")) {
            if (cond) {
                int target = (int)parse_expression(&p);
                int idx = find_line(target);
                if (idx != -1) {
                    program_counter = idx;
                } else {
                    printf("UNDEFINED LINE NUMBER %d\n", target);
                    running = 0;
                }
            }
        } else {
            printf("SYNTAX ERROR: EXPECTED THEN\n");
            running = 0;
        }
    }
    else if (match_keyword(&p, "FOR")) {
        int var_idx = parse_var_index(&p);
        if (var_idx != -1) {
            skip_spaces(&p);
            if (*p == '=') {
                double start, limit, step = 1.0;
                p++;
                start = parse_expression(&p);
                variables[var_idx] = start;
                
                skip_spaces(&p);
                if (match_keyword(&p, "TO")) {
                    limit = parse_expression(&p);
                    skip_spaces(&p);
                    if (match_keyword(&p, "STEP")) {
                        step = parse_expression(&p);
                    }
                    
                    if (stack_ptr < STACK_SIZE) {
                        control_stack[stack_ptr].is_for = 1;
                        control_stack[stack_ptr].u.for_loop.var_idx = var_idx;
                        control_stack[stack_ptr].u.for_loop.limit = limit;
                        control_stack[stack_ptr].u.for_loop.step = step;
                        control_stack[stack_ptr].u.for_loop.loop_line = program[program_counter].line_number;
                        control_stack[stack_ptr].u.for_loop.loop_ptr = NULL;
                        stack_ptr++;
                    } else {
                        printf("STACK OVERFLOW\n");
                        running = 0;
                    }
                } else {
                    printf("SYNTAX ERROR: EXPECTED TO\n");
                    running = 0;
                }
            } else {
                printf("SYNTAX ERROR: EXPECTED =\n");
                running = 0;
            }
        }
    }
    else if (match_keyword(&p, "NEXT")) {
        int var_idx = parse_var_index(&p);
        if (var_idx != -1) {
            int i;
            int found = -1;
            for (i = stack_ptr - 1; i >= 0; i--) {
                if (control_stack[i].is_for && control_stack[i].u.for_loop.var_idx == var_idx) {
                    found = i;
                    break;
                }
            }
            if (found != -1) {
                ForLoop *fl = &control_stack[found].u.for_loop;
                variables[var_idx] += fl->step;
                
                if ((fl->step >= 0 && variables[var_idx] <= fl->limit) ||
                    (fl->step < 0 && variables[var_idx] >= fl->limit)) {
                    int loop_idx = find_line(fl->loop_line);
                    if (loop_idx != -1) {
                        program_counter = loop_idx + 1;
                        stack_ptr = found + 1; /* Pop any inner loops */
                    } else {
                        printf("UNDEFINED LINE NUMBER FOR NEXT\n");
                        running = 0;
                    }
                } else {
                    stack_ptr = found; /* Loop finished, pop it */
                }
            } else {
                printf("NEXT WITHOUT FOR\n");
                running = 0;
            }
        }
    }
    else if (match_keyword(&p, "GOSUB")) {
        int target = (int)parse_expression(&p);
        if (stack_ptr < STACK_SIZE) {
            int idx;
            control_stack[stack_ptr].is_for = 0;
            control_stack[stack_ptr].u.ret_line = program_counter;
            stack_ptr++;
            
            idx = find_line(target);
            if (idx != -1) {
                program_counter = idx;
            } else {
                printf("UNDEFINED LINE NUMBER %d\n", target);
                running = 0;
            }
        } else {
            printf("STACK OVERFLOW\n");
            running = 0;
        }
    }
    else if (match_keyword(&p, "RETURN")) {
        int i;
        int found = -1;
        for (i = stack_ptr - 1; i >= 0; i--) {
            if (!control_stack[i].is_for) {
                found = i;
                break;
            }
        }
        if (found != -1) {
            program_counter = control_stack[found].u.ret_line;
            stack_ptr = found;
        } else {
            printf("RETURN WITHOUT GOSUB\n");
            running = 0;
        }
    }
    else if (match_keyword(&p, "DIM")) {
        while (*p && running) {
            int letter;
            int dim1, dim2 = 0;
            
            skip_spaces(&p);
            if (!isalpha((unsigned char)*p)) {
                printf("SYNTAX ERROR: EXPECTED ARRAY NAME IN DIM\n");
                running = 0;
                return;
            }
            letter = toupper((unsigned char)*p) - 'A';
            p++;
            
            skip_spaces(&p);
            if (*p == '(') {
                p++;
                dim1 = (int)parse_expression(&p);
                skip_spaces(&p);
                if (*p == ',') {
                    p++;
                    dim2 = (int)parse_expression(&p);
                }
                skip_spaces(&p);
                if (*p == ')') {
                    p++;
                } else {
                    printf("SYNTAX ERROR: MISSING ) IN DIM\n");
                    running = 0;
                    return;
                }
                
                if (dim1 < 1 || dim2 < 0) {
                    printf("INVALID DIMENSION\n");
                    running = 0;
                    return;
                }
                
                arrays[letter].dim1 = dim1;
                arrays[letter].dim2 = dim2;
                arrays[letter].offset = -1; /* Will allocate on first use */
            } else {
                printf("SYNTAX ERROR: EXPECTED ( IN DIM\n");
                running = 0;
                return;
            }
            
            skip_spaces(&p);
            if (*p == ',') p++;
        }
    }
    else if (match_keyword(&p, "READ")) {
        while (*p && running) {
            int is_array = 0;
            char *tmp;
            skip_spaces(&p);
            tmp = p;
            if (isalpha((unsigned char)*tmp)) {
                tmp++;
                skip_spaces(&tmp);
                if (*tmp == '(') is_array = 1;
            }
            
            if (data_ptr >= data_count) {
                printf("OUT OF DATA\n");
                running = 0;
                return;
            }
            
            if (is_array) {
                double *el = get_array_element(&p);
                if (!el) return;
                *el = data_block[data_ptr++];
            } else {
                int idx = parse_var_index(&p);
                if (idx != -1) {
                    variables[idx] = data_block[data_ptr++];
                } else {
                    printf("SYNTAX ERROR IN READ\n");
                    running = 0;
                    return;
                }
            }
            
            skip_spaces(&p);
            if (*p == ',') p++;
        }
    }
    else if (match_keyword(&p, "DATA")) {
        /* Ignored during execution, handled by build_data_block */
        return;
    }
    else if (match_keyword(&p, "DEF")) {
        skip_spaces(&p);
        if (strncmp(p, "FN", 2) == 0 && isalpha((unsigned char)(p[2]))) {
            int fn_idx = toupper((unsigned char)(p[2])) - 'A';
            p += 3;
            skip_spaces(&p);
            if (*p == '(') {
                int param_idx;
                p++;
                param_idx = parse_var_index(&p);
                if (param_idx != -1) {
                    skip_spaces(&p);
                    if (*p == ')') {
                        p++;
                        skip_spaces(&p);
                        if (*p == '=') {
                            p++;
                            user_funcs[fn_idx].param_var_idx = param_idx;
                            strncpy(user_funcs[fn_idx].expr, p, LINE_LEN);
                            user_funcs[fn_idx].expr[LINE_LEN] = '\0';
                        } else {
                            printf("SYNTAX ERROR: EXPECTED = IN DEF\n");
                            running = 0;
                        }
                    } else {
                        printf("SYNTAX ERROR: MISSING ) IN DEF\n");
                        running = 0;
                    }
                } else {
                    printf("SYNTAX ERROR: INVALID PARAMETER IN DEF\n");
                    running = 0;
                }
            } else {
                printf("SYNTAX ERROR: EXPECTED ( IN DEF\n");
                running = 0;
            }
        } else {
            printf("SYNTAX ERROR: INVALID FUNCTION NAME\n");
            running = 0;
        }
    }
    else if (match_keyword(&p, "END") || match_keyword(&p, "STOP")) {
        running = 0;
    }
    else {
        printf("SYNTAX ERROR: UNKNOWN STATEMENT\n");
        running = 0;
    }
}

static void run_program(void) {
    if (num_lines == 0) return;
    
    clear_variables();
    running = 1;
    build_data_block();
    if (!running) return; /* Syntax error in data */
    
    program_counter = 0;
    
    while (running && program_counter < num_lines) {
        /* Save current line in case of loop/jump */
        int current_line = program_counter;
        execute_line(program[program_counter].text);
        
        /* If program_counter didn't change (no jump), go to next line */
        if (program_counter == current_line) {
            program_counter++;
        }
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
 * COMMAND PROCESSORS
 *
 * WHAT CAN BE CHANGED:
 *   - OS-specific commands (like system("dir") or system("ls")) can be
 *     updated to use platform-independent dirent logic if added.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The historical Kemeny/Kurtz names (HELLO, SCRATCH, OLD, NEW,
 *     CATALOG, UNSAVE) must remain exactly as they are.
 *
 * WHAT TO EXPECT:
 *   These are interactive terminal commands that do not have line numbers.
 *
 * IF SOMETHING BREAKS:
 *   If SAVE or OLD fails, verify resolve_filename correctly appends .bas.
 * ========================================================================= */

static void cmd_help(void) {
    printf("\n*** DARTMOUTH BASIC 1964 COMMANDS ***\n");
    printf("STATEMENTS:\n");
    printf("  LET var = expr   : Assign value (LET is mandatory)\n");
    printf("  PRINT expr,...   : Output values or \"strings\"\n");
    printf("  GOTO line        : Jump to line\n");
    printf("  IF cond THEN ln  : Conditional jump\n");
    printf("  FOR v=a TO b ... : Start loop (optional STEP)\n");
    printf("  NEXT v           : End loop\n");
    printf("  GOSUB line       : Call subroutine\n");
    printf("  RETURN           : Return from subroutine\n");
    printf("  READ var,...     : Read DATA into variables\n");
    printf("  DATA val,...     : Store numeric data block\n");
    printf("  DIM var(sz,sz)   : Dimension 1D or 2D array\n");
    printf("  DEF FNA(x) = ... : Define single-line function\n");
    printf("  REM text         : Comment\n");
    printf("  STOP / END       : Terminate program\n");
    printf("\nFUNCTIONS:\n");
    printf("  SIN, COS, TAN, ATN, EXP, LOG, ABS, SQR, RND, INT\n");
    printf("\nINTERACTIVE COMMANDS:\n");
    printf("  HELLO            : Start fresh session\n");
    printf("  NEW              : Create a new program\n");
    printf("  OLD              : Load an existing program\n");
    printf("  SAVE / REPLACE   : Save current program\n");
    printf("  LIST             : Show program lines\n");
    printf("  RUN              : Execute program\n");
    printf("  CATALOG / CAT    : List files in current directory\n");
    printf("  SCRATCH          : Clear current program from memory\n");
    printf("  UNSAVE           : Delete the current saved program file\n");
    printf("  BYE / GOODBYE    : Exit interpreter\n");
    printf("  HELP             : Show this message\n\n");
}

static void resolve_filename(const char *raw, char *out, int out_size) {
    char temp[256];
    char *p = temp;
    size_t len;
    int i;
    
    strncpy(temp, raw, 255);
    temp[255] = '\0';
    skip_spaces(&p);
    
    if (*p == '"') {
        char *end;
        p++;
        end = strchr(p, '"');
        if (end) *end = '\0';
    }
    
    trim_newline(p);
    
    len = strlen(p);
    for (i = 0; i < (int)len; i++) {
        p[i] = (char)tolower((unsigned char)p[i]);
    }
    
    if (len > 4 && strcmp(p + len - 4, ".bas") == 0) {
        strncpy(out, p, out_size - 1);
    } else {
        sprintf(out, "%.250s.bas", p);
    }
    out[out_size - 1] = '\0';
}

static void save_program(const char *filename) {
    char resolved[256];
    FILE *f;
    int i;
    
    resolve_filename(filename, resolved, sizeof(resolved));
    f = fopen(resolved, "w");
    if (!f) {
        printf("FILE ERROR\n");
        return;
    }
    
    for (i = 0; i < num_lines; i++) {
        fprintf(f, "%d %s\n", program[i].line_number, program[i].text);
    }
    fclose(f);
    printf("READY\n");
}

static void load_program(const char *filename) {
    char resolved[256];
    FILE *f;
    char line[LINE_LEN + 32];
    
    resolve_filename(filename, resolved, sizeof(resolved));
    f = fopen(resolved, "r");
    if (!f) {
        printf("FILE NOT FOUND\n");
        return;
    }
    
    clear_all();
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        store_line(line);
    }
    fclose(f);
    strncpy(current_filename, resolved, sizeof(current_filename));
    current_filename[sizeof(current_filename) - 1] = '\0';
    printf("READY\n");
}

static void do_catalog(void) {
#ifdef _WIN32
    system("dir /W *.bas");
#else
    system("ls -l *.bas");
#endif
}

/* =========================================================================
 * MAIN REPL LOOP
 *
 * WHAT CAN BE CHANGED:
 *   - Prompt strings can be modified.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Execution block variable declarations must stay at the top of the
 *     do/while scope to pass C89 compilation.
 *
 * WHAT TO EXPECT:
 *   The entry point of the interpreter. Handles raw input, line number
 *   detection, and dispatching direct commands vs stored lines.
 *
 * IF SOMETHING BREAKS:
 *   If input hangs or loops indefinitely, check fgets EOF handling
 *   and ensure input buffers are correctly flushed.
 * ========================================================================= */

int main(void) {
    char line[LINE_LEN + 32];
    
    srand((unsigned int)time(NULL));
    clear_all();
    
    printf("DARTMOUTH BASIC 1964\n");
    printf("READY\n");
    
    while (1) {
        char *p;
        
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        trim_newline(line);
        uppercase_input(line);
        p = line;
        skip_spaces(&p);
        
        if (*p == '\0') {
            continue;
        }
        
        if (isdigit((unsigned char)*p)) {
            store_line(p);
        } 
        else if (match_keyword(&p, "HELLO")) {
            clear_all();
            current_filename[0] = '\0';
            printf("READY\n");
        }
        else if (match_keyword(&p, "NEW")) {
            clear_all();
            skip_spaces(&p);
            if (*p) {
                resolve_filename(p, current_filename, sizeof(current_filename));
            } else {
                current_filename[0] = '\0';
            }
            printf("READY\n");
        }
        else if (match_keyword(&p, "SCRATCH")) {
            clear_all();
            printf("READY\n");
        }
        else if (match_keyword(&p, "OLD")) {
            skip_spaces(&p);
            if (*p == '\0') {
                printf("OLD PROGRAM NAME--\n");
                if (fgets(line, sizeof(line), stdin)) {
                    trim_newline(line);
                    uppercase_input(line);
                    load_program(line);
                }
            } else {
                load_program(p);
            }
        }
        else if (match_keyword(&p, "SAVE") || match_keyword(&p, "REPLACE")) {
            skip_spaces(&p);
            if (*p != '\0') {
                save_program(p);
            } else if (current_filename[0] != '\0') {
                save_program(current_filename);
            } else {
                printf("NEW PROGRAM NAME--\n");
                if (fgets(line, sizeof(line), stdin)) {
                    trim_newline(line);
                    uppercase_input(line);
                    if (line[0] != '\0') save_program(line);
                }
            }
        }
        else if (match_keyword(&p, "UNSAVE")) {
            char file_to_del[256];
            skip_spaces(&p);
            if (*p != '\0') {
                resolve_filename(p, file_to_del, sizeof(file_to_del));
            } else if (current_filename[0] != '\0') {
                strncpy(file_to_del, current_filename, sizeof(file_to_del));
            } else {
                printf("PROGRAM NAME--\n");
                if (fgets(line, sizeof(line), stdin)) {
                    trim_newline(line);
                    uppercase_input(line);
                    resolve_filename(line, file_to_del, sizeof(file_to_del));
                } else {
                    continue;
                }
            }
            if (file_to_del[0] != '\0') {
                if (file_exists(file_to_del)) {
                    remove_file(file_to_del);
                    printf("READY\n");
                } else {
                    printf("FILE NOT FOUND\n");
                }
            }
        }
        else if (match_keyword(&p, "CATALOG") || match_keyword(&p, "CAT")) {
            do_catalog();
            printf("READY\n");
        }
        else if (match_keyword(&p, "LIST")) {
            list_program();
            printf("READY\n");
        }
        else if (match_keyword(&p, "RUN")) {
            run_program();
            printf("READY\n");
        }
        else if (match_keyword(&p, "HELP")) {
            cmd_help();
        }
        else if (match_keyword(&p, "GOODBYE") || match_keyword(&p, "BYE")) {
            break;
        }
        else {
            /* Immediate execution of statements not supported in pure Dartmouth 1964,
               but we can provide a syntax error message */
            printf("SYNTAX ERROR: IMMEDIATE MODE NOT SUPPORTED OR UNKNOWN COMMAND\n");
        }
    }
    
    return 0;
}

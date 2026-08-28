// FILENAME: apple2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// DESCRIPTION: Provides core logic and implementation for apple2.c within BASIC++.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* =========================================================================
 * CONSTANTS
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_LINES: increase for larger programs
 *   - MAX_VARS: increase for more named variables
 *   - MAX_ARRAYS / MAX_STRINGS: increase for more DIM'd arrays/strings
 *   - STRING_POOL_SIZE: increase total string memory
 *   - ARRAY_POOL_SIZE: increase total array element count
 *
 * WHAT CANNOT BE CHANGED:
 *   - VAR_NAME_SIG must be 2 (dialect: first 2 chars significant)
 *   - Line numbers must be in 0..32767 range
 *
 * WHAT TO EXPECT:
 *   Total static memory is dominated by program[] and string_pool[].
 *
 * IF SOMETHING BREAKS:
 *   If the program crashes at startup, reduce MAX_LINES or pool sizes.
 * ========================================================================= */

#define MAX_LINES       500
#define LINE_LEN        255
#define MAX_VARS        100
#define VAR_NAME_SIG      2
#define MAX_ARRAYS       26
#define ARRAY_POOL_SIZE 26000
#define MAX_STRINGS      26
#define STRING_POOL_SIZE 16384

/* =========================================================================
 * DATA STRUCTURES
 *
 * WHAT CAN BE CHANGED:
 *   - ProgramLine text size tracks LINE_LEN.
 *   - Variable/Array/StringVar struct fields can be widened.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Variable name is always VAR_NAME_SIG+1 chars.
 *   - String variables require DIM before use.
 *
 * WHAT TO EXPECT:
 *   Variables are looked up by linear search on the first 2 chars.
 *   Arrays and strings are separate namespaces.
 *
 * IF SOMETHING BREAKS:
 *   If variable lookup fails, check that the name normalization
 *   code uppercases and truncates to 2 characters.
 * ========================================================================= */

typedef struct {
    int line_number;
    char text[LINE_LEN + 1];
} ProgramLine;

typedef struct {
    char name[VAR_NAME_SIG + 1];
    short value;
    int used;
} Variable;

typedef struct {
    char name[VAR_NAME_SIG + 1];
    int offset;
    int size;
} ArrayVar;

typedef struct {
    char name[VAR_NAME_SIG + 1];
    int offset;
    int length;
    int max_length;
} StringVar;

/* =========================================================================
 * STATIC MEMORY
 *
 * WHAT CAN BE CHANGED:
 *   - Pool sizes and array counts (see CONSTANTS).
 *
 * WHAT CANNOT BE CHANGED:
 *   - These must remain static/global for C89 portability.
 *   - auto_mode and auto_* variables control AUTO/MAN behavior.
 *
 * WHAT TO EXPECT:
 *   All memory is zero-initialized at program start.
 *
 * IF SOMETHING BREAKS:
 *   If variables have garbage, call clear_variables().
 * ========================================================================= */

static ProgramLine pgm[MAX_LINES];
static int num_lines = 0;

static Variable vars[MAX_VARS];
static int num_vars = 0;

static ArrayVar arrays[MAX_ARRAYS];
static short array_pool[ARRAY_POOL_SIZE];
static int num_arrays = 0;
static int array_pool_used = 0;

static StringVar strings[MAX_STRINGS];
static char string_pool[STRING_POOL_SIZE];
static int num_strings = 0;
static int string_pool_used = 0;

static int pc = 0;
static int running = 0;
static int stopped_pc = -1;

static int auto_mode = 0;
static int auto_line = 10;
static int auto_step = 10;

static char *parser_ptr;

/* =========================================================================
 * FUNCTION PROTOTYPES
 * ========================================================================= */

static void  clear_variables(void);
static void  clear_program(void);
static void  skip_spaces(void);
static int   match_keyword(const char *kw);
static void  parse_varname(char *out);
static int   find_var(const char *name);
static int   find_or_create_var(const char *name);
static int   find_array(const char *name);
static int   find_string(const char *name);
static int   find_line(int ln);
static void  store_line(const char *raw);
static void  delete_line(int ln);
static void  delete_range(int lo, int hi);
static void  resolve_filename(const char *raw, char *out, int sz);

static short parse_expression(void);
static short parse_or_expr(void);
static short parse_and_expr(void);
static short parse_not_expr(void);
static short parse_comparison(void);
static short parse_add_sub(void);
static short parse_mul_div(void);
static short parse_power(void);
static short parse_unary(void);
static short parse_primary(void);

static void  exec_statement(void);
static void  cmd_print(void);
static void  cmd_let(void);
static void  cmd_input(void);
static void  cmd_goto(void);
static void  cmd_if(void);
static void  cmd_dim(void);
static void  cmd_on_goto(void);
static void  cmd_help(void);

static void  run_program(void);
static void  list_program(const char *args);
static void  save_program(const char *fn);
static void  load_program(const char *fn);

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

/* =========================================================================
 * UTILITY FUNCTIONS
 *
 * WHAT CAN BE CHANGED:
 *   - Error message text can be customized.
 *
 * WHAT CANNOT BE CHANGED:
 *   - skip_spaces() must only skip space and tab.
 *   - match_keyword() must check for non-alpha after the keyword.
 *   - parse_varname() must normalize to 2 uppercase characters.
 *
 * WHAT TO EXPECT:
 *   These functions operate on the global parser_ptr.
 *
 * IF SOMETHING BREAKS:
 *   - If keywords are not matched, check that the input was
 *     uppercased before the call.
 *   - If variable names collide unexpectedly, check the 2-char
 *     truncation in parse_varname().
 * ========================================================================= */

static void report_error(const char *msg) {
    printf("\aERR: %s\n", msg);
    fflush(stdout);
    running = 0;
}

static void clear_variables(void) {
    num_vars = 0;
    num_arrays = 0;
    array_pool_used = 0;
    num_strings = 0;
    string_pool_used = 0;
}

static void clear_program(void) {
    num_lines = 0;
    clear_variables();
}

static void skip_spaces(void) {
    while (*parser_ptr == ' ' || *parser_ptr == '\t') parser_ptr++;
}

static int match_keyword(const char *kw) {
    size_t len = strlen(kw);
    if (strncmp(parser_ptr, kw, len) == 0
        && !isalpha((unsigned char)parser_ptr[len])) {
        parser_ptr += len;
        return 1;
    }
    return 0;
}

/*
 * parse_varname
 * Reads a variable name from parser_ptr, normalizes it to
 * 2 uppercase characters, and stores it in out[3].
 * Advances parser_ptr past the name.
 */
static void parse_varname(char *out) {
    int i = 0;
    out[0] = out[1] = out[2] = '\0';
    while (isalnum((unsigned char)*parser_ptr)) {
        if (i < VAR_NAME_SIG) {
            out[i] = (char)toupper((unsigned char)*parser_ptr);
        }
        i++;
        parser_ptr++;
    }
    out[VAR_NAME_SIG] = '\0';
}

static int find_var(const char *name) {
    int i;
    for (i = 0; i < num_vars; i++) {
        if (vars[i].used && strcmp(vars[i].name, name) == 0) return i;
    }
    return -1;
}

static int find_or_create_var(const char *name) {
    int idx = find_var(name);
    if (idx >= 0) return idx;
    if (num_vars >= MAX_VARS) { report_error("TOO MANY VARIABLES"); return -1; }
    strncpy(vars[num_vars].name, name, VAR_NAME_SIG);
    vars[num_vars].name[VAR_NAME_SIG] = '\0';
    vars[num_vars].value = 0;
    vars[num_vars].used = 1;
    return num_vars++;
}

static int find_array(const char *name) {
    int i;
    for (i = 0; i < num_arrays; i++) {
        if (strcmp(arrays[i].name, name) == 0) return i;
    }
    return -1;
}

static int find_string(const char *name) {
    int i;
    for (i = 0; i < num_strings; i++) {
        if (strcmp(strings[i].name, name) == 0) return i;
    }
    return -1;
}

/* =========================================================================
 * PROGRAM STORAGE
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_LINES controls maximum program size.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Lines must always be sorted by line number.
 *   - A line number with no text deletes that line.
 *
 * WHAT TO EXPECT:
 *   - store_line() inserts sorted or replaces existing.
 *   - delete_range() is used by the DEL command.
 *
 * IF SOMETHING BREAKS:
 *   - If LIST shows lines out of order, check the insertion sort.
 *   - If GOTO fails, verify find_line() returns the correct index.
 * ========================================================================= */

static int find_line(int ln) {
    int i;
    for (i = 0; i < num_lines; i++) {
        if (pgm[i].line_number == ln) return i;
        if (pgm[i].line_number > ln) return -1;
    }
    return -1;
}

static void store_line(const char *raw) {
    int ln, idx, i, j;
    const char *text;

    ln = atoi(raw);
    if (ln < 0 || ln > 32767) { report_error("BAD LINE NUMBER"); return; }

    text = raw;
    while (isdigit((unsigned char)*text)) text++;
    while (*text == ' ' || *text == '\t') text++;

    if (*text == '\0') { delete_line(ln); return; }

    idx = find_line(ln);
    if (idx >= 0) {
        memset(pgm[idx].text, 0, LINE_LEN + 1);
        strncpy(pgm[idx].text, text, LINE_LEN);
        return;
    }

    if (num_lines >= MAX_LINES) { report_error("PROGRAM MEMORY FULL"); return; }

    for (i = 0; i < num_lines; i++) {
        if (pgm[i].line_number > ln) break;
    }
    for (j = num_lines; j > i; j--) pgm[j] = pgm[j - 1];

    pgm[i].line_number = ln;
    memset(pgm[i].text, 0, LINE_LEN + 1);
    strncpy(pgm[i].text, text, LINE_LEN);
    num_lines++;
}

static void delete_line(int ln) {
    int idx, i;
    idx = find_line(ln);
    if (idx < 0) return;
    for (i = idx; i < num_lines - 1; i++) pgm[i] = pgm[i + 1];
    num_lines--;
}

static void delete_range(int lo, int hi) {
    int i;
    for (i = num_lines - 1; i >= 0; i--) {
        if (pgm[i].line_number >= lo && pgm[i].line_number <= hi) {
            delete_line(pgm[i].line_number);
        }
    }
}

/* =========================================================================
 * FILENAME RESOLUTION
 *
 * WHAT CAN BE CHANGED:
 *   - The default extension (.BAS) can be changed.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Filenames can be with or without quotes.
 *   - .BAS is auto-appended if not already present.
 *
 * WHAT TO EXPECT:
 *   SAVE test -> test.BAS, SAVE "test.BAS" -> test.BAS
 *
 * IF SOMETHING BREAKS:
 *   If filenames have trailing quotes, check the strip logic.
 * ========================================================================= */

static void resolve_filename(const char *raw, char *out, int sz) {
    const char *src = raw;
    int len;
    while (*src == ' ' || *src == '\t') src++;
    if (*src == '"') src++;
    strncpy(out, src, sz - 5);
    out[sz - 5] = '\0';
    len = (int)strlen(out);
    while (len > 0 && (out[len-1]=='\n' || out[len-1]=='\r')) { out[--len]='\0'; }
    if (len > 0 && out[len - 1] == '"') { out[--len] = '\0'; }
    while (len > 0 && (out[len-1]==' ' || out[len-1]=='\t')) { out[--len]='\0'; }
    if (len == 0) return;
    if (len >= 4) {
        char e1 = (char)toupper((unsigned char)out[len-3]);
        char e2 = (char)toupper((unsigned char)out[len-2]);
        char e3 = (char)toupper((unsigned char)out[len-1]);
        if (out[len-4]=='.' && e1=='B' && e2=='A' && e3=='S') return;
    }
    if (len + 4 < sz) strcat(out, ".BAS");
}

/* =========================================================================
 * FILE I/O
 *
 * WHAT CAN BE CHANGED:
 *   - Output messages ("OK", "FILE NOT FOUND").
 *
 * WHAT CANNOT BE CHANGED:
 *   - LOAD clears the existing program before loading.
 *
 * WHAT TO EXPECT:
 *   Programs are saved as plain text: <linenum> <text>\n
 *
 * IF SOMETHING BREAKS:
 *   If SAVE creates a 0-byte file, check num_lines > 0.
 * ========================================================================= */

static void save_program(const char *fn) {
    FILE *fp;
    char fname[LINE_LEN + 1];
    int i;
    if (fn == NULL || *fn == '\0') { report_error("FILENAME REQUIRED"); return; }
    resolve_filename(fn, fname, sizeof(fname));
    if (fname[0] == '\0') { report_error("FILENAME REQUIRED"); return; }
    fp = fopen(fname, "w");
    if (!fp) { report_error("CANNOT OPEN FILE"); return; }
    for (i = 0; i < num_lines; i++)
        fprintf(fp, "%d %s\n", pgm[i].line_number, pgm[i].text);
    fclose(fp);
    printf("OK\n");
}

static void load_program(const char *fn) {
    FILE *fp;
    char fname[LINE_LEN + 1];
    char buf[LINE_LEN + 32];
    int len;
    if (fn == NULL || *fn == '\0') { report_error("FILENAME REQUIRED"); return; }
    resolve_filename(fn, fname, sizeof(fname));
    if (fname[0] == '\0') { report_error("FILENAME REQUIRED"); return; }
    fp = fopen(fname, "r");
    if (!fp) { report_error("FILE NOT FOUND"); return; }
    clear_program();
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        len = (int)strlen(buf);
        while (len > 0 && (buf[len-1]=='\n'||buf[len-1]=='\r')) buf[--len]='\0';
        if (buf[0] != '\0') store_line(buf);
    }
    fclose(fp);
    printf("OK\n");
}

/* =========================================================================
 * EXPRESSION PARSER (Standard Operator Precedence)
 *
 * Precedence (lowest to highest):
 *   1. OR
 *   2. AND
 *   3. NOT (unary)
 *   4. Relational: = < > <= >= # <>
 *   5. Addition/Subtraction: + -
 *   6. Multiplication/Division/Modulus: * / MOD
 *   7. Exponentiation: ^
 *   8. Unary minus: -
 *   9. Primary: number, variable, array, string func, (expr)
 *
 * WHAT CAN BE CHANGED:
 *   - Add new functions in parse_primary().
 *   - Add new operators at the appropriate precedence level.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Precedence order (dialect requirement).
 *   - Recursive descent structure.
 *   - Division by zero must halt execution.
 *
 * WHAT TO EXPECT:
 *   - 2+3*4 = 14 (not 20).
 *   - 2^3 = 8 (integer exponentiation).
 *   - 10 MOD 3 = 1.
 *   - AND/OR/NOT operate on 0/non-zero boolean values.
 *   - RND(X) returns 0 to X-1 (Apple II convention).
 *
 * IF SOMETHING BREAKS:
 *   - If precedence seems wrong, check the call chain.
 *   - If MOD or ^ are not recognized, check that the keyword
 *     matching in parse_mul_div / parse_power works.
 * ========================================================================= */

static short parse_primary(void) {
    short val = 0;

    if (!running) return 0;
    skip_spaces();

    /* Parenthesized expression */
    if (*parser_ptr == '(') {
        parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return val;
    }

    /* ABS(X) */
    if (strncmp(parser_ptr, "ABS", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return (short)(val < 0 ? -val : val);
    }

    /* SGN(X) */
    if (strncmp(parser_ptr, "SGN", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (val > 0) return 1;
        if (val < 0) return (short)-1;
        return 0;
    }

    /* RND(X) - returns 0 to X-1 (Apple II convention) */
    if (strncmp(parser_ptr, "RND", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        int range;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        range = (int)val;
        if (range <= 0) return 0;
        return (short)(rand() % range);
    }

    /* LEN(S$) */
    if (strncmp(parser_ptr, "LEN", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        char vn[VAR_NAME_SIG + 1];
        int si;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        skip_spaces();
        parse_varname(vn);
        if (*parser_ptr == '$') parser_ptr++;
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return 0; }
        return (short)strings[si].length;
    }

    /* ASC(S$) */
    if (strncmp(parser_ptr, "ASC", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        char vn[VAR_NAME_SIG + 1];
        int si;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        skip_spaces();
        parse_varname(vn);
        if (*parser_ptr == '$') parser_ptr++;
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return 0; }
        if (strings[si].length == 0) return 0;
        return (short)(unsigned char)string_pool[strings[si].offset];
    }

    /* TAB(X) - outputs spaces, returns 0 */
    if (strncmp(parser_ptr, "TAB", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        int col, k;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        col = (int)parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (col < 0) col = 0;
        for (k = 0; k < col; k++) printf(" ");
        return 0;
    }

    /* Variable, array element, or string variable reference */
    if (isalpha((unsigned char)*parser_ptr)) {
        char vn[VAR_NAME_SIG + 1];
        parse_varname(vn);

        /* Check for string variable: NAME$ */
        if (*parser_ptr == '$') {
            parser_ptr++;
            /* String function context: return length or 0 */
            return 0;
        }

        /* Check for array: NAME(expr) */
        skip_spaces();
        if (*parser_ptr == '(') {
            int ai = find_array(vn);
            short idx;
            parser_ptr++;
            idx = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            if (ai < 0) { report_error("ARRAY NOT DIM'D"); return 0; }
            if (idx < 0 || idx >= arrays[ai].size) {
                report_error("SUBSCRIPT OUT OF RANGE");
                return 0;
            }
            return array_pool[arrays[ai].offset + idx];
        }

        /* Simple variable */
        {
            int vi = find_var(vn);
            if (vi >= 0) return vars[vi].value;
            /* Auto-create with 0 */
            vi = find_or_create_var(vn);
            if (vi >= 0) return vars[vi].value;
            return 0;
        }
    }

    /* Numeric literal */
    if (isdigit((unsigned char)*parser_ptr)) {
        return (short)strtol(parser_ptr, &parser_ptr, 10);
    }

    return 0;
}

static short parse_unary(void) {
    skip_spaces();
    if (*parser_ptr == '-') { parser_ptr++; return (short)-parse_unary(); }
    if (*parser_ptr == '+') { parser_ptr++; return parse_unary(); }
    return parse_primary();
}

/* ^ exponentiation (right-to-left associative) */
static short parse_power(void) {
    short base = parse_unary();
    skip_spaces();
    if (*parser_ptr == '^') {
        short exp_val, result = 1;
        int i;
        parser_ptr++;
        exp_val = parse_power();
        if (exp_val < 0) return 0;
        for (i = 0; i < exp_val; i++) result = (short)(result * base);
        return result;
    }
    return base;
}

/* * / MOD */
static short parse_mul_div(void) {
    short result = parse_power();
    while (running) {
        char op;
        short rhs;
        skip_spaces();
        op = *parser_ptr;
        if (op == '*' || op == '/') {
            parser_ptr++;
            rhs = parse_power();
            if (op == '*') {
                result = (short)(result * rhs);
            } else {
                if (rhs == 0) { report_error("DIVISION BY ZERO"); return 0; }
                result = (short)(result / rhs);
            }
        } else if (strncmp(parser_ptr, "MOD", 3) == 0
                   && !isalpha((unsigned char)parser_ptr[3])) {
            parser_ptr += 3;
            rhs = parse_power();
            if (rhs == 0) { report_error("DIVISION BY ZERO"); return 0; }
            result = (short)(result % rhs);
        } else {
            break;
        }
    }
    return result;
}

/* + - */
static short parse_add_sub(void) {
    short result = parse_mul_div();
    while (running) {
        char op;
        short rhs;
        skip_spaces();
        op = *parser_ptr;
        if (op != '+' && op != '-') break;
        parser_ptr++;
        rhs = parse_mul_div();
        if (op == '+') result = (short)(result + rhs);
        else           result = (short)(result - rhs);
    }
    return result;
}

/* Relational: = < > <= >= # <> */
static short parse_comparison(void) {
    short lhs = parse_add_sub();
    char op1, op2;
    short rhs;

    if (!running) return 0;
    skip_spaces();

    op1 = *parser_ptr;
    if (op1 == '#') {
        parser_ptr++;
        rhs = parse_add_sub();
        return (short)(lhs != rhs ? 1 : 0);
    }
    if (op1 != '=' && op1 != '<' && op1 != '>') return lhs;

    parser_ptr++;
    op2 = '\0';
    if (*parser_ptr == '>' || *parser_ptr == '=') { op2 = *parser_ptr; parser_ptr++; }

    rhs = parse_add_sub();

    if (op1 == '=' && op2 == '\0') return (short)(lhs == rhs ? 1 : 0);
    if (op1 == '<' && op2 == '>')  return (short)(lhs != rhs ? 1 : 0);
    if (op1 == '<' && op2 == '=')  return (short)(lhs <= rhs ? 1 : 0);
    if (op1 == '<' && op2 == '\0') return (short)(lhs < rhs ? 1 : 0);
    if (op1 == '>' && op2 == '=')  return (short)(lhs >= rhs ? 1 : 0);
    if (op1 == '>' && op2 == '\0') return (short)(lhs > rhs ? 1 : 0);
    return lhs;
}

/* NOT (unary logical) */
static short parse_not_expr(void) {
    skip_spaces();
    if (strncmp(parser_ptr, "NOT", 3) == 0
        && !isalpha((unsigned char)parser_ptr[3])) {
        parser_ptr += 3;
        return (short)(parse_not_expr() ? 0 : 1);
    }
    return parse_comparison();
}

/* AND */
static short parse_and_expr(void) {
    short result = parse_not_expr();
    while (running) {
        skip_spaces();
        if (strncmp(parser_ptr, "AND", 3) == 0
            && !isalpha((unsigned char)parser_ptr[3])) {
            short rhs;
            parser_ptr += 3;
            rhs = parse_not_expr();
            result = (short)(result && rhs ? 1 : 0);
        } else {
            break;
        }
    }
    return result;
}

/* OR (lowest precedence) */
static short parse_or_expr(void) {
    short result = parse_and_expr();
    while (running) {
        skip_spaces();
        if (strncmp(parser_ptr, "OR", 2) == 0
            && !isalpha((unsigned char)parser_ptr[2])) {
            short rhs;
            parser_ptr += 2;
            rhs = parse_and_expr();
            result = (short)(result || rhs ? 1 : 0);
        } else {
            break;
        }
    }
    return result;
}

static short parse_expression(void) {
    if (!running) return 0;
    return parse_or_expr();
}

/* =========================================================================
 * STATEMENT EXECUTION
 *
 * WHAT CAN BE CHANGED:
 *   - Add new statements by adding match_keyword() cases.
 *   - Error messages can be customized.
 *
 * WHAT CANNOT BE CHANGED:
 *   - LET is mandatory (bare assignment must produce error).
 *   - RUN with a filename must produce SYNTAX ERROR.
 *   - Keywords must be uppercased before matching.
 *
 * WHAT TO EXPECT:
 *   - PRINT uses classic BASIC numeric formatting (space before
 *     positive numbers, trailing space after all numbers).
 *   - DIM allocates from static pools.
 *   - DEL deletes a range of lines.
 *
 * IF SOMETHING BREAKS:
 *   - If a statement is not recognized, check the uppercase
 *     conversion.
 *   - If DIM fails, check pool space.
 * ========================================================================= */

static void cmd_print(void) {
    int need_nl = 1;
    skip_spaces();
    if (*parser_ptr == '\0') { printf("\n"); return; }

    while (*parser_ptr != '\0' && running) {
        skip_spaces();
        if (*parser_ptr == '\0') break;

        if (*parser_ptr == '"') {
            char *end;
            parser_ptr++;
            end = strchr(parser_ptr, '"');
            if (end) {
                *end = '\0';
                printf("%s", parser_ptr);
                *end = '"';
                parser_ptr = end + 1;
            } else {
                printf("%s", parser_ptr);
                parser_ptr += strlen(parser_ptr);
            }
            need_nl = 1;
        } else if (*parser_ptr == ';') {
            parser_ptr++;
            need_nl = 0;
        } else if (*parser_ptr == ',') {
            printf("\t");
            parser_ptr++;
            need_nl = 0;
        } else if (strncmp(parser_ptr, "TAB", 3) == 0
                   && !isalpha((unsigned char)parser_ptr[3])) {
            parse_primary();
            need_nl = 1;
        } else {
            /* Check if this is a string variable to print */
            char vn[VAR_NAME_SIG + 1];
            int need_str = 0;
            if (isalpha((unsigned char)*parser_ptr)) {
                char *look = parser_ptr;
                while (isalnum((unsigned char)*look)) look++;
                if (*look == '$') need_str = 1;
            }
            if (need_str) {
                int si;
                parse_varname(vn);
                if (*parser_ptr == '$') parser_ptr++;
                skip_spaces();
                si = find_string(vn);
                if (si < 0) { report_error("STRING NOT DIM'D"); return; }
                /* Check for slice: A$(start, end) */
                if (*parser_ptr == '(') {
                    int s, e;
                    parser_ptr++;
                    s = (int)parse_expression();
                    skip_spaces();
                    if (*parser_ptr == ',') parser_ptr++;
                    e = (int)parse_expression();
                    skip_spaces();
                    if (*parser_ptr == ')') parser_ptr++;
                    if (s < 0) s = 0;
                    if (e >= strings[si].length) e = strings[si].length - 1;
                    while (s <= e) {
                        putchar(string_pool[strings[si].offset + s]);
                        s++;
                    }
                } else {
                    /* Print whole string */
                    int k;
                    for (k = 0; k < strings[si].length; k++) {
                        putchar(string_pool[strings[si].offset + k]);
                    }
                }
                need_nl = 1;
            } else {
                short val = parse_expression();
                if (!running) return;
                if (val >= 0) printf(" ");
                printf("%d ", (int)val);
                need_nl = 1;
            }
        }
    }
    if (need_nl && running) printf("\n");
}

static void cmd_let(void) {
    char vn[VAR_NAME_SIG + 1];
    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) {
        report_error("EXPECTED VARIABLE"); return;
    }
    parse_varname(vn);

    /* String variable: LET A$ = "value" or LET A$(pos) = "c" */
    if (*parser_ptr == '$') {
        int si;
        parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return; }
        skip_spaces();

        if (*parser_ptr == '(') {
            /* Positional assignment: LET A$(pos) = "c" */
            int pos;
            parser_ptr++;
            pos = (int)parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            skip_spaces();
            if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
            parser_ptr++;
            skip_spaces();
            if (*parser_ptr == '"') {
                parser_ptr++;
                if (pos >= 0 && pos < strings[si].max_length) {
                    string_pool[strings[si].offset + pos] = *parser_ptr;
                    if (pos >= strings[si].length)
                        strings[si].length = pos + 1;
                }
                while (*parser_ptr && *parser_ptr != '"') parser_ptr++;
                if (*parser_ptr == '"') parser_ptr++;
            }
            return;
        }

        if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
        parser_ptr++;
        skip_spaces();
        if (*parser_ptr == '"') {
            int k = 0;
            parser_ptr++;
            while (*parser_ptr && *parser_ptr != '"'
                   && k < strings[si].max_length) {
                string_pool[strings[si].offset + k] = *parser_ptr;
                k++;
                parser_ptr++;
            }
            strings[si].length = k;
            if (*parser_ptr == '"') parser_ptr++;
        } else {
            report_error("EXPECTED STRING");
        }
        return;
    }

    /* Array element: LET A(idx) = expr */
    skip_spaces();
    if (*parser_ptr == '(') {
        int ai = find_array(vn);
        short idx;
        parser_ptr++;
        idx = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        skip_spaces();
        if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
        parser_ptr++;
        if (ai < 0) { report_error("ARRAY NOT DIM'D"); return; }
        if (idx < 0 || idx >= arrays[ai].size) {
            report_error("SUBSCRIPT OUT OF RANGE"); return;
        }
        array_pool[arrays[ai].offset + idx] = parse_expression();
        return;
    }

    /* Simple variable: LET A = expr */
    if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
    parser_ptr++;
    {
        int vi = find_or_create_var(vn);
        if (vi >= 0) vars[vi].value = parse_expression();
    }
}

static void cmd_input(void) {
    char vn[VAR_NAME_SIG + 1];
    char buf[LINE_LEN + 1];
    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) {
        report_error("EXPECTED VARIABLE"); return;
    }
    parse_varname(vn);

    /* String input: INPUT A$ */
    if (*parser_ptr == '$') {
        int si;
        parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return; }
        printf("? "); fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            int k = 0;
            int blen = (int)strlen(buf);
            while (blen > 0 && (buf[blen-1]=='\n'||buf[blen-1]=='\r'))
                buf[--blen] = '\0';
            while (k < blen && k < strings[si].max_length) {
                string_pool[strings[si].offset + k] = buf[k];
                k++;
            }
            strings[si].length = k;
        }
        return;
    }

    /* Numeric input */
    printf("? "); fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin)) {
        int vi = find_or_create_var(vn);
        if (vi >= 0) vars[vi].value = (short)strtol(buf, NULL, 10);
    }
}

static void cmd_goto(void) {
    int target, idx;
    skip_spaces();
    target = (int)strtol(parser_ptr, &parser_ptr, 10);
    idx = find_line(target);
    if (idx < 0) { report_error("LINE NOT FOUND"); }
    else { pc = idx; }
}

static void cmd_if(void) {
    int cond;
    skip_spaces();
    cond = (int)parse_expression();
    if (!running) return;
    skip_spaces();
    if (strncmp(parser_ptr, "THEN", 4) == 0
        && !isalpha((unsigned char)parser_ptr[4])) {
        parser_ptr += 4;
    }
    skip_spaces();
    if (cond) {
        if (isdigit((unsigned char)*parser_ptr)) { cmd_goto(); }
        else { exec_statement(); }
    }
}

static void cmd_dim(void) {
    while (running) {
        char vn[VAR_NAME_SIG + 1];
        int size;
        skip_spaces();
        if (!isalpha((unsigned char)*parser_ptr)) {
            report_error("EXPECTED VARIABLE"); return;
        }
        parse_varname(vn);

        /* String DIM: DIM A$(20) */
        if (*parser_ptr == '$') {
            parser_ptr++;
            skip_spaces();
            if (*parser_ptr != '(') { report_error("EXPECTED '('"); return; }
            parser_ptr++;
            size = (int)parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            if (size <= 0) { report_error("BAD STRING SIZE"); return; }
            if (find_string(vn) >= 0) { report_error("ALREADY DIM'D"); return; }
            if (num_strings >= MAX_STRINGS) { report_error("TOO MANY STRINGS"); return; }
            if (string_pool_used + size > STRING_POOL_SIZE) {
                report_error("OUT OF STRING MEMORY"); return;
            }
            memcpy(strings[num_strings].name, vn, VAR_NAME_SIG);
            strings[num_strings].name[VAR_NAME_SIG] = '\0';
            strings[num_strings].offset = string_pool_used;
            strings[num_strings].length = 0;
            strings[num_strings].max_length = size;
            memset(&string_pool[string_pool_used], 0, (size_t)size);
            string_pool_used += size;
            num_strings++;
        } else {
            /* Integer array DIM: DIM A(100) */
            skip_spaces();
            if (*parser_ptr != '(') { report_error("EXPECTED '('"); return; }
            parser_ptr++;
            size = (int)parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            if (size <= 0) { report_error("BAD ARRAY SIZE"); return; }
            if (find_array(vn) >= 0) { report_error("ALREADY DIM'D"); return; }
            if (num_arrays >= MAX_ARRAYS) { report_error("TOO MANY ARRAYS"); return; }
            if (array_pool_used + size > ARRAY_POOL_SIZE) {
                report_error("OUT OF ARRAY MEMORY"); return;
            }
            memcpy(arrays[num_arrays].name, vn, VAR_NAME_SIG);
            arrays[num_arrays].name[VAR_NAME_SIG] = '\0';
            arrays[num_arrays].offset = array_pool_used;
            arrays[num_arrays].size = size;
            array_pool_used += size;
            num_arrays++;
        }

        skip_spaces();
        if (*parser_ptr == ',') { parser_ptr++; }
        else { break; }
    }
}

static void cmd_on_goto(void) {
    int idx, count;
    short val;

    skip_spaces();
    val = parse_expression();
    if (!running) return;
    skip_spaces();

    if (strncmp(parser_ptr, "GOTO", 4) == 0
        && !isalpha((unsigned char)parser_ptr[4])) {
        parser_ptr += 4;
    } else {
        report_error("EXPECTED GOTO"); return;
    }

    idx = (int)val;
    count = 0;

    while (running) {
        int target, li;
        skip_spaces();
        if (!isdigit((unsigned char)*parser_ptr)) break;
        target = (int)strtol(parser_ptr, &parser_ptr, 10);
        count++;
        if (count == idx) {
            li = find_line(target);
            if (li < 0) { report_error("LINE NOT FOUND"); }
            else { pc = li; }
            return;
        }
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
    }
    report_error("ON INDEX OUT OF RANGE");
}

/* =========================================================================
 * HELP COMMAND
 *
 * WHAT CAN BE CHANGED:
 *   - Update the listing when keywords are added or removed.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Must reflect the actual implemented feature set.
 *
 * WHAT TO EXPECT:
 *   Prints a compact reference card to stdout.
 *
 * IF SOMETHING BREAKS:
 *   This is pure output; it cannot cause logic errors.
 * ========================================================================= */

static void cmd_help(void) {
    printf("\n");
    printf("Apple II Integer BASIC v2.1\n");
    printf("=============================\n");
    printf("Statements:  PRINT  LET  INPUT  GOTO  IF/THEN  REM\n");
    printf("             END  DIM  ON...GOTO  BEEP\n");
    printf("Commands:    RUN  LIST  NEW  SAVE  LOAD  HELP  BYE\n");
    printf("             DEL  CLR  CON  AUTO  MAN\n");
    printf("Functions:   ABS()  SGN()  RND()  ASC()  LEN()  TAB()\n");
    printf("Operators:   +  -  *  /  MOD  ^\n");
    printf("             =  <  >  <=  >=  #  <>\n");
    printf("             AND  OR  NOT\n");
    printf("\n");
}

/* =========================================================================
 * STATEMENT DISPATCHER
 *
 * WHAT CAN BE CHANGED:
 *   - Add new commands here.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The uppercase conversion must happen before keyword matching.
 *   - RUN with filename must produce SYNTAX ERROR.
 *
 * WHAT TO EXPECT:
 *   Unrecognized input produces SYNTAX ERROR.
 *
 * IF SOMETHING BREAKS:
 *   If a keyword is not matched, check the uppercase conversion code.
 * ========================================================================= */

static void exec_statement(void) {
    skip_spaces();
    if (*parser_ptr == '\0') return;

    /* Input is pre-uppercased by uppercase_line() in the REPL */

    if (match_keyword("PRINT")) { cmd_print(); return; }
    if (match_keyword("LET"))   { cmd_let(); return; }
    if (match_keyword("INPUT")) { cmd_input(); return; }
    if (match_keyword("GOTO"))  { cmd_goto(); return; }
    if (match_keyword("IF"))    { cmd_if(); return; }
    if (match_keyword("DIM"))   { cmd_dim(); return; }
    if (match_keyword("ON"))    { cmd_on_goto(); return; }
    if (match_keyword("REM"))   { return; }
    if (match_keyword("END"))   { running = 0; return; }
    if (match_keyword("BEEP"))  { trigger_beep(); return; }

    if (match_keyword("RUN")) {
        skip_spaces();
        if (*parser_ptr != '\0') { report_error("SYNTAX ERROR"); }
        else { run_program(); }
        return;
    }
    if (match_keyword("LIST")) { list_program(parser_ptr); return; }
    if (match_keyword("NEW"))  { clear_program(); return; }
    if (match_keyword("SAVE")) { save_program(parser_ptr); return; }
    if (match_keyword("LOAD")) { load_program(parser_ptr); return; }
    if (match_keyword("HELP")) { cmd_help(); return; }

    if (match_keyword("DEL")) {
        int lo, hi;
        skip_spaces();
        lo = (int)strtol(parser_ptr, &parser_ptr, 10);
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        skip_spaces();
        hi = (int)strtol(parser_ptr, &parser_ptr, 10);
        if (hi < lo) hi = lo;
        delete_range(lo, hi);
        return;
    }

    if (match_keyword("CLR")) { clear_variables(); return; }

    if (match_keyword("CON")) {
        if (stopped_pc < 0) { report_error("CAN'T CONTINUE"); }
        else { pc = stopped_pc; stopped_pc = -1; running = 1; }
        return;
    }

    if (match_keyword("AUTO")) {
        skip_spaces();
        if (isdigit((unsigned char)*parser_ptr)) {
            auto_line = (int)strtol(parser_ptr, &parser_ptr, 10);
            skip_spaces();
            if (*parser_ptr == ',') {
                parser_ptr++;
                skip_spaces();
                auto_step = (int)strtol(parser_ptr, &parser_ptr, 10);
            }
        } else {
            auto_line = 10;
            auto_step = 10;
        }
        auto_mode = 1;
        return;
    }

    if (match_keyword("MAN")) { auto_mode = 0; return; }

    if (match_keyword("BYE")) { exit(0); }

    report_error("SYNTAX ERROR");
}

/* =========================================================================
 * PROGRAM EXECUTION
 *
 * WHAT CAN BE CHANGED:
 *   - Whether CLR is called at RUN start (some dialects preserve vars).
 *
 * WHAT CANNOT BE CHANGED:
 *   - GOTO sets pc; the runner must not auto-advance when pc changed.
 *
 * WHAT TO EXPECT:
 *   - Execution starts at the first stored line.
 *   - running=0 stops after the current line.
 *
 * IF SOMETHING BREAKS:
 *   - Infinite loop: check END sets running=0.
 *   - GOTO skips: check find_line() and pc assignment.
 * ========================================================================= */

static void run_program(void) {
    char buf[LINE_LEN + 1];
    int prev;

    if (num_lines == 0) return;
    clear_variables();
    pc = 0;
    running = 1;

    while (running && pc < num_lines) {
        prev = pc;
        strncpy(buf, pgm[pc].text, LINE_LEN);
        buf[LINE_LEN] = '\0';
        parser_ptr = buf;
        exec_statement();
        if (running && pc == prev) pc++;
    }
    running = 0;
}

static void list_program(const char *args) {
    int i;
    (void)args;
    for (i = 0; i < num_lines; i++) {
        printf("%d %s\n", pgm[i].line_number, pgm[i].text);
    }
}

/* =========================================================================
 * MAIN / REPL
 *
 * WHAT CAN BE CHANGED:
 *   - The prompt string "> ".
 *   - The banner text and version number.
 *   - AUTO mode prompt format.
 *
 * WHAT CANNOT BE CHANGED:
 *   - srand() must be called before any RND() usage.
 *   - Lines with a leading digit must be stored, not executed.
 *   - LET is mandatory: bare variable assignment is SYNTAX ERROR.
 *
 * WHAT TO EXPECT:
 *   - In AUTO mode, the interpreter prints the next line number and
 *     waits for the user to type the statement portion.
 *   - MAN cancels AUTO mode.
 *   - EOF on stdin exits cleanly.
 *
 * IF SOMETHING BREAKS:
 *   - If the prompt does not appear, check fflush(stdout).
 *   - If AUTO mode does not advance, check auto_line/auto_step.
 * ========================================================================= */

int main(void) {
    char input_buf[LINE_LEN + 20];
    char line_buf[LINE_LEN + 20];

    srand((unsigned int)time(NULL));
    clear_program();
    printf("Apple II Integer BASIC v2.1\n");
    printf("%d BYTES FREE\n", (int)(sizeof(pgm) + sizeof(array_pool)
                                   + sizeof(string_pool)));

    while (1) {
        if (auto_mode) {
            printf("%d ", auto_line);
            fflush(stdout);
            if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
                printf("\n");
                break;
            }
            {
                int len = (int)strlen(input_buf);
                while (len > 0 && (input_buf[len-1]=='\n'
                       || input_buf[len-1]=='\r'))
                    input_buf[--len] = '\0';
            }
            uppercase_line(input_buf);
            if (input_buf[0] == '\0') {
                auto_mode = 0;
                continue;
            }
            sprintf(line_buf, "%d %s", auto_line, input_buf);
            store_line(line_buf);
            auto_line += auto_step;
            continue;
        }

        printf("> ");
        fflush(stdout);

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            printf("\n");
            break;
        }

        {
            int len = (int)strlen(input_buf);
            while (len > 0 && (input_buf[len-1]=='\n'
                   || input_buf[len-1]=='\r'))
                input_buf[--len] = '\0';
        }

        uppercase_line(input_buf);
        if (input_buf[0] == '\0') continue;

        if (isdigit((unsigned char)input_buf[0])) {
            store_line(input_buf);
        } else {
            strncpy(line_buf, input_buf, sizeof(line_buf) - 1);
            line_buf[sizeof(line_buf) - 1] = '\0';
            parser_ptr = line_buf;
            running = 1;
            exec_statement();
            running = 0;
        }
    }

    return 0;
}

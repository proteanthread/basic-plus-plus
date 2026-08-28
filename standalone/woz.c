// FILENAME: woz.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// DESCRIPTION: Apple II Integer BASIC interpreter (Steve Wozniak, 1977) in freestanding C89/C90.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef BASIC_RAM_SIZE
#define BASIC_RAM_SIZE    65536L
#endif

#define MAX_LINES         2000
#define LINE_LEN          255
#define MAX_VARS          128
#define VAR_NAME_SIG      2
#define MAX_ARRAYS        26
#define ARRAY_POOL_SIZE   16384
#define MAX_STRINGS       26
#define STRING_POOL_SIZE  16384
#define GOSUB_STACK_SIZE  64
#define FOR_STACK_SIZE    32

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

typedef struct {
    int var_idx;
    short target;
    short step;
    int line_idx;
} ForFrame;

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

static int gosub_stack[GOSUB_STACK_SIZE];
static int gosub_sp = 0;

static ForFrame for_stack[FOR_STACK_SIZE];
static int for_sp = 0;

static int trace_on = 0;
static char dsp_var[VAR_NAME_SIG + 1];

static int pc = 0;
static int running = 0;
static int stopped_pc = -1;

static int auto_mode = 0;
static int auto_line = 10;
static int auto_step = 10;

static char *parser_ptr = NULL;
static char curr_line_buf[LINE_LEN + 1];

/* Function Prototypes */
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
static void  delete_range(int lo, int hi);
static void  resolve_filename(const char *raw, char *out, int sz);
static void  save_program(const char *filename);
static void  load_program(const char *filename);
static void  list_program(const char *args);
static void  run_program(void);
static void  exec_statement(void);
static void  exec_line_statements(char *line);
static short parse_expression(void);

static void trigger_beep(void) {
    printf("\a");
    fflush(stdout);
}

static void report_error(const char *msg) {
    if (running && pc >= 0 && pc < num_lines) {
        printf("*** %s IN %d\n", msg, pgm[pc].line_number);
    } else {
        printf("*** %s\n", msg);
    }
    running = 0;
}

static void clear_variables(void) {
    int i;
    for (i = 0; i < MAX_VARS; i++) {
        vars[i].name[0] = '\0';
        vars[i].value = 0;
        vars[i].used = 0;
    }
    num_vars = 0;
    num_arrays = 0;
    array_pool_used = 0;
    num_strings = 0;
    string_pool_used = 0;
    gosub_sp = 0;
    for_sp = 0;
    dsp_var[0] = '\0';
}

static void clear_program(void) {
    num_lines = 0;
    clear_variables();
    stopped_pc = -1;
    pc = 0;
    running = 0;
}

static void skip_spaces(void) {
    while (*parser_ptr == ' ' || *parser_ptr == '\t') parser_ptr++;
}

static int my_strncasecmp(const char *s1, const char *s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        int c1 = toupper((unsigned char)s1[i]);
        int c2 = toupper((unsigned char)s2[i]);
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
    return 0;
}

static void uppercase_line(char *s) {
    int in_quote = 0;
    while (*s) {
        if (*s == '"') in_quote = !in_quote;
        else if (!in_quote) *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

static int match_keyword(const char *kw) {
    int len;
    skip_spaces();
    len = (int)strlen(kw);
    if (my_strncasecmp(parser_ptr, kw, len) == 0) {
        char next = parser_ptr[len];
        if (!isalnum((unsigned char)next) && next != '$' && next != '(' && next != '=') {
            parser_ptr += len;
            return 1;
        }
        if (kw[len - 1] == '=' || kw[len - 1] == '#' || kw[len - 1] == ':') {
            parser_ptr += len;
            return 1;
        }
    }
    return 0;
}

static void parse_varname(char *out) {
    int i = 0;
    skip_spaces();
    while (isalnum((unsigned char)*parser_ptr) && i < VAR_NAME_SIG) {
        out[i++] = (char)toupper((unsigned char)*parser_ptr++);
    }
    while (isalnum((unsigned char)*parser_ptr)) parser_ptr++;
    out[i] = '\0';
}

static int find_var(const char *name) {
    int i;
    for (i = 0; i < num_vars; i++) {
        if (vars[i].used && strcmp(vars[i].name, name) == 0) return i;
    }
    return -1;
}

static int find_or_create_var(const char *name) {
    int i = find_var(name);
    if (i >= 0) return i;
    if (num_vars >= MAX_VARS) {
        report_error("TOO MANY VARIABLES");
        return -1;
    }
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

static int find_line(int ln) {
    int i;
    for (i = 0; i < num_lines; i++) {
        if (pgm[i].line_number == ln) return i;
    }
    return -1;
}

static void store_line(const char *raw) {
    int ln;
    char *endp;
    int idx, i;
    const char *text;

    while (*raw == ' ' || *raw == '\t') raw++;
    if (!isdigit((unsigned char)*raw)) return;

    ln = (int)strtol(raw, &endp, 10);
    text = endp;
    while (*text == ' ' || *text == '\t') text++;

    idx = find_line(ln);

    if (*text == '\0') {
        if (idx >= 0) {
            for (i = idx; i < num_lines - 1; i++) pgm[i] = pgm[i + 1];
            num_lines--;
        }
        return;
    }

    if (idx >= 0) {
        strncpy(pgm[idx].text, text, LINE_LEN);
        pgm[idx].text[LINE_LEN] = '\0';
        return;
    }

    if (num_lines >= MAX_LINES) {
        printf("*** PROGRAM FULL\n");
        return;
    }

    idx = 0;
    while (idx < num_lines && pgm[idx].line_number < ln) idx++;

    for (i = num_lines; i > idx; i--) pgm[i] = pgm[i - 1];

    pgm[idx].line_number = ln;
    strncpy(pgm[idx].text, text, LINE_LEN);
    pgm[idx].text[LINE_LEN] = '\0';
    num_lines++;
}

static void delete_range(int lo, int hi) {
    int i = 0;
    while (i < num_lines) {
        if (pgm[i].line_number >= lo && pgm[i].line_number <= hi) {
            int j;
            for (j = i; j < num_lines - 1; j++) pgm[j] = pgm[j + 1];
            num_lines--;
        } else {
            i++;
        }
    }
}

static void resolve_filename(const char *raw, char *out, int sz) {
    const char *src = raw;
    int len;
    while (*src == ' ' || *src == '\t') src++;
    if (*src == '"') src++;
    strncpy(out, src, sz - 5);
    out[sz - 5] = '\0';
    len = (int)strlen(out);
    while (len > 0 && (out[len - 1] == ' ' || out[len - 1] == '\t' || out[len - 1] == '\r' || out[len - 1] == '\n' || out[len - 1] == '"')) {
        out[--len] = '\0';
    }
    if (len == 0) return;
    if (len >= 4) {
        char e0 = out[len - 4];
        char e1 = (char)toupper((unsigned char)out[len - 3]);
        char e2 = (char)toupper((unsigned char)out[len - 2]);
        char e3 = (char)toupper((unsigned char)out[len - 1]);
        if (e0 == '.' && e1 == 'B' && e2 == 'A' && e3 == 'S') return;
    }
    if (len + 4 < sz) strcat(out, ".BAS");
}

static void save_program(const char *filename) {
    FILE *fp;
    char fname[256];
    int i;
    if (filename == NULL || *filename == '\0') {
        printf("*** FILENAME REQUIRED\n");
        return;
    }
    resolve_filename(filename, fname, sizeof(fname));
    fp = fopen(fname, "w");
    if (!fp) {
        printf("*** FILE ERROR\n");
        return;
    }
    for (i = 0; i < num_lines; i++) {
        fprintf(fp, "%d %s\n", pgm[i].line_number, pgm[i].text);
    }
    fclose(fp);
    printf("OK\n");
}

static void load_program(const char *filename) {
    FILE *fp;
    char fname[256];
    char buf[LINE_LEN + 20];
    if (filename == NULL || *filename == '\0') {
        printf("*** FILENAME REQUIRED\n");
        return;
    }
    resolve_filename(filename, fname, sizeof(fname));
    fp = fopen(fname, "r");
    if (!fp) {
        printf("*** FILE NOT FOUND\n");
        return;
    }
    clear_program();
    while (fgets(buf, sizeof(buf), fp)) {
        int blen = (int)strlen(buf);
        while (blen > 0 && (buf[blen - 1] == '\n' || buf[blen - 1] == '\r')) buf[--blen] = '\0';
        if (buf[0] != '\0') store_line(buf);
    }
    fclose(fp);
    printf("OK\n");
}

static void list_program(const char *args) {
    int lo = 0, hi = 32767, i;
    const char *p = args;
    while (*p == ' ' || *p == '\t') p++;
    if (isdigit((unsigned char)*p)) {
        lo = (int)strtol(p, (char **)&p, 10);
        hi = lo;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',' || *p == '-') {
            p++;
            while (*p == ' ' || *p == '\t') p++;
            if (isdigit((unsigned char)*p)) {
                hi = (int)strtol(p, NULL, 10);
            } else {
                hi = 32767;
            }
        }
    }
    for (i = 0; i < num_lines; i++) {
        if (pgm[i].line_number >= lo && pgm[i].line_number <= hi) {
            printf("%d %s\n", pgm[i].line_number, pgm[i].text);
        }
    }
}

/* =========================================================================
 * EXPRESSION EVALUATOR (APPLE II INTEGER BASIC)
 * ========================================================================= */

static short parse_unary(void);
static short parse_power(void);
static short parse_mul_div(void);
static short parse_add_sub(void);
static short parse_comparison(void);
static short parse_not_expr(void);
static short parse_and_expr(void);
static short parse_or_expr(void);

static short parse_primary(void) {
    short val = 0;
    if (!running) return 0;
    skip_spaces();

    if (*parser_ptr == '(') {
        parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return val;
    }

    /* ABS(X) */
    if (strncmp(parser_ptr, "ABS", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return (short)(val < 0 ? -val : val);
    }

    /* SGN(X) */
    if (strncmp(parser_ptr, "SGN", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (val > 0) return 1;
        if (val < 0) return -1;
        return 0;
    }

    /* RND(X) - 0 to X-1 */
    if (strncmp(parser_ptr, "RND", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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

    /* LEN(A$) */
    if (strncmp(parser_ptr, "LEN", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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

    /* ASC(A$) */
    if (strncmp(parser_ptr, "ASC", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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

    /* VAL(A$) */
    if (strncmp(parser_ptr, "VAL", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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
        return (short)strtol(&string_pool[strings[si].offset], NULL, 10);
    }

    /* PEEK(X) / PDL(X) / PTR / SCRN(X,Y) */
    if (strncmp(parser_ptr, "PEEK", 4) == 0 && !isalnum((unsigned char)parser_ptr[4])) {
        parser_ptr += 4; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return 0;
    }
    if (strncmp(parser_ptr, "PDL", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return 0;
    }
    if (strncmp(parser_ptr, "SCRN", 4) == 0 && !isalnum((unsigned char)parser_ptr[4])) {
        parser_ptr += 4; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        (void)parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return 0;
    }

    /* TAB(X) */
    if (strncmp(parser_ptr, "TAB", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        int col, k;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        col = (int)parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (col < 0) col = 0;
        for (k = 0; k < col; k++) putchar(' ');
        return 0;
    }

    /* SPC(X) */
    if (strncmp(parser_ptr, "SPC", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        int col, k;
        parser_ptr += 3; skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        col = (int)parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (col < 0) col = 0;
        for (k = 0; k < col; k++) putchar(' ');
        return 0;
    }

    /* Variables and Arrays */
    if (isalpha((unsigned char)*parser_ptr)) {
        char vn[VAR_NAME_SIG + 1];
        parse_varname(vn);
        if (*parser_ptr == '$') {
            parser_ptr++;
            return 0;
        }
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
        {
            int vi = find_var(vn);
            if (vi >= 0) return vars[vi].value;
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
        } else if (strncmp(parser_ptr, "MOD", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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

static short parse_comparison(void) {
    short lhs = parse_add_sub();
    char op1, op2;
    short rhs;

    if (!running) return 0;
    skip_spaces();

    op1 = *parser_ptr;
    if (op1 == '=' || op1 == '<' || op1 == '>' || op1 == '#') {
        parser_ptr++;
        op2 = *parser_ptr;
        if (op2 == '=' || op2 == '>') parser_ptr++;
        else op2 = '\0';

        rhs = parse_add_sub();

        if (op1 == '=' && op2 == '\0') return (short)(lhs == rhs ? 1 : 0);
        if (op1 == '#' || (op1 == '<' && op2 == '>')) return (short)(lhs != rhs ? 1 : 0);
        if (op1 == '<' && op2 == '\0') return (short)(lhs < rhs ? 1 : 0);
        if (op1 == '>' && op2 == '\0') return (short)(lhs > rhs ? 1 : 0);
        if (op1 == '<' && op2 == '=') return (short)(lhs <= rhs ? 1 : 0);
        if (op1 == '>' && op2 == '=') return (short)(lhs >= rhs ? 1 : 0);
    }
    return lhs;
}

static short parse_not_expr(void) {
    skip_spaces();
    if (strncmp(parser_ptr, "NOT", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
        parser_ptr += 3;
        return (short)(parse_not_expr() ? 0 : 1);
    }
    return parse_comparison();
}

static short parse_and_expr(void) {
    short result = parse_not_expr();
    while (running) {
        skip_spaces();
        if (strncmp(parser_ptr, "AND", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
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

static short parse_or_expr(void) {
    short result = parse_and_expr();
    while (running) {
        skip_spaces();
        if (strncmp(parser_ptr, "OR", 2) == 0 && !isalnum((unsigned char)parser_ptr[2])) {
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
 * STATEMENT HANDLERS
 * ========================================================================= */

static void cmd_print(void) {
    int need_nl = 1;
    skip_spaces();
    if (*parser_ptr == '\0' || *parser_ptr == ':') { printf("\n"); return; }

    while (*parser_ptr != '\0' && *parser_ptr != ':' && running) {
        char *look;
        int is_str = 0;

        skip_spaces();
        if (*parser_ptr == '\0' || *parser_ptr == ':') break;

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
        } else if (strncmp(parser_ptr, "CHR$", 4) == 0 && !isalnum((unsigned char)parser_ptr[4])) {
            short code;
            parser_ptr += 4; skip_spaces();
            if (*parser_ptr == '(') parser_ptr++;
            code = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            putchar((char)code);
            need_nl = 1;
        } else if (strncmp(parser_ptr, "STR$", 4) == 0 && !isalnum((unsigned char)parser_ptr[4])) {
            short num;
            parser_ptr += 4; skip_spaces();
            if (*parser_ptr == '(') parser_ptr++;
            num = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            printf("%d", (int)num);
            need_nl = 1;
        } else if (strncmp(parser_ptr, "TAB", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
            parse_primary();
            need_nl = 1;
        } else if (strncmp(parser_ptr, "SPC", 3) == 0 && !isalnum((unsigned char)parser_ptr[3])) {
            parse_primary();
            need_nl = 1;
        } else {
            /* Check if this is a string variable reference A$ */
            look = parser_ptr;
            while (isalnum((unsigned char)*look)) look++;
            if (*look == '$') is_str = 1;

            if (is_str) {
                char vn[VAR_NAME_SIG + 1];
                int si;
                parse_varname(vn);
                if (*parser_ptr == '$') parser_ptr++;
                skip_spaces();
                si = find_string(vn);
                if (si < 0) { report_error("STRING NOT DIM'D"); return; }
                if (*parser_ptr == '(') {
                    int s = 1, e = strings[si].length;
                    parser_ptr++;
                    s = (int)parse_expression();
                    skip_spaces();
                    if (*parser_ptr == ',') {
                        parser_ptr++;
                        e = (int)parse_expression();
                    } else {
                        e = strings[si].length;
                    }
                    skip_spaces();
                    if (*parser_ptr == ')') parser_ptr++;
                    if (s < 1) s = 1;
                    if (e > strings[si].length) e = strings[si].length;
                    s--; e--;
                    while (s <= e && s >= 0 && s < strings[si].length) {
                        putchar(string_pool[strings[si].offset + s]);
                        s++;
                    }
                } else {
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
    char *look;
    int is_str = 0;

    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) {
        report_error("EXPECTED VARIABLE"); return;
    }

    look = parser_ptr;
    while (isalnum((unsigned char)*look)) look++;
    if (*look == '$') is_str = 1;

    if (is_str) {
        int si;
        parse_varname(vn);
        if (*parser_ptr == '$') parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return; }
        skip_spaces();

        if (*parser_ptr == '(') {
            int s = 1, e = 1, k = 0;
            parser_ptr++;
            s = (int)parse_expression();
            skip_spaces();
            if (*parser_ptr == ',') {
                parser_ptr++;
                e = (int)parse_expression();
            } else {
                e = s;
            }
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            skip_spaces();
            if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
            parser_ptr++;
            skip_spaces();
            if (*parser_ptr == '"') {
                parser_ptr++;
                s--; e--;
                if (s < 0) s = 0;
                while (*parser_ptr && *parser_ptr != '"' && (s + k) <= e && (s + k) < strings[si].max_length) {
                    string_pool[strings[si].offset + s + k] = *parser_ptr++;
                    k++;
                }
                if (s + k > strings[si].length) strings[si].length = s + k;
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
            while (*parser_ptr && *parser_ptr != '"' && k < strings[si].max_length) {
                string_pool[strings[si].offset + k++] = *parser_ptr++;
            }
            strings[si].length = k;
            if (*parser_ptr == '"') parser_ptr++;
        } else {
            report_error("EXPECTED STRING");
        }
        return;
    }

    parse_varname(vn);
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

    if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
    parser_ptr++;
    {
        int vi = find_or_create_var(vn);
        if (vi >= 0) {
            vars[vi].value = parse_expression();
            if (dsp_var[0] != '\0' && strcmp(dsp_var, vn) == 0) {
                printf("# %s = %d\n", vn, (int)vars[vi].value);
            }
        }
    }
}

static void cmd_input(void) {
    char vn[VAR_NAME_SIG + 1];
    char buf[LINE_LEN + 1];
    char *look;
    int is_str = 0;

    skip_spaces();

    if (*parser_ptr == '"') {
        char *end;
        parser_ptr++;
        end = strchr(parser_ptr, '"');
        if (end) {
            *end = '\0';
            printf("%s", parser_ptr);
            *end = '"';
            parser_ptr = end + 1;
        }
        skip_spaces();
        if (*parser_ptr == ';' || *parser_ptr == ',') parser_ptr++;
        skip_spaces();
    }

    if (!isalpha((unsigned char)*parser_ptr)) {
        report_error("EXPECTED VARIABLE"); return;
    }

    look = parser_ptr;
    while (isalnum((unsigned char)*look)) look++;
    if (*look == '$') is_str = 1;

    if (is_str) {
        int si;
        parse_varname(vn);
        if (*parser_ptr == '$') parser_ptr++;
        si = find_string(vn);
        if (si < 0) { report_error("STRING NOT DIM'D"); return; }
        printf("? "); fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            int k = 0;
            int blen = (int)strlen(buf);
            while (blen > 0 && (buf[blen - 1] == '\n' || buf[blen - 1] == '\r')) buf[--blen] = '\0';
            while (k < blen && k < strings[si].max_length) {
                string_pool[strings[si].offset + k] = buf[k];
                k++;
            }
            strings[si].length = k;
        }
        return;
    }

    parse_varname(vn);
    printf("? "); fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin)) {
        int vi = find_or_create_var(vn);
        if (vi >= 0) vars[vi].value = (short)strtol(buf, NULL, 10);
    }
}

static void cmd_goto(void) {
    int target, idx;
    skip_spaces();
    target = (int)parse_expression();
    idx = find_line(target);
    if (idx < 0) { report_error("LINE NOT FOUND"); }
    else { pc = idx; }
}

static void cmd_gosub(void) {
    int target, idx;
    skip_spaces();
    target = (int)parse_expression();
    idx = find_line(target);
    if (idx < 0) { report_error("LINE NOT FOUND"); return; }
    if (gosub_sp >= GOSUB_STACK_SIZE) { report_error("GOSUB STACK OVERFLOW"); return; }
    gosub_stack[gosub_sp++] = pc + 1;
    pc = idx;
}

static void cmd_return(void) {
    if (gosub_sp <= 0) { report_error("RETURN WITHOUT GOSUB"); return; }
    pc = gosub_stack[--gosub_sp];
}

static void cmd_for(void) {
    char vn[VAR_NAME_SIG + 1];
    int vi;
    short start_val, limit_val, step_val = 1;

    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) { report_error("EXPECTED VARIABLE"); return; }
    parse_varname(vn);
    vi = find_or_create_var(vn);
    if (vi < 0) return;

    skip_spaces();
    if (*parser_ptr != '=') { report_error("EXPECTED '='"); return; }
    parser_ptr++;

    start_val = parse_expression();
    vars[vi].value = start_val;

    skip_spaces();
    if (!match_keyword("TO")) { report_error("EXPECTED TO"); return; }

    limit_val = parse_expression();

    skip_spaces();
    if (match_keyword("STEP")) {
        step_val = parse_expression();
    }

    /* If this variable is already at top of FOR stack, reuse it */
    if (for_sp > 0 && for_stack[for_sp - 1].var_idx == vi) {
        for_stack[for_sp - 1].target = limit_val;
        for_stack[for_sp - 1].step = step_val;
        for_stack[for_sp - 1].line_idx = pc + 1;
        return;
    }

    if (for_sp >= FOR_STACK_SIZE) { report_error("FOR STACK OVERFLOW"); return; }
    for_stack[for_sp].var_idx = vi;
    for_stack[for_sp].target = limit_val;
    for_stack[for_sp].step = step_val;
    for_stack[for_sp].line_idx = pc + 1;
    for_sp++;
}

static void cmd_next(void) {
    char vn[VAR_NAME_SIG + 1];
    int vi = -1;
    skip_spaces();
    if (isalpha((unsigned char)*parser_ptr)) {
        parse_varname(vn);
        vi = find_var(vn);
    }
    if (for_sp <= 0) { report_error("NEXT WITHOUT FOR"); return; }

    if (vi >= 0 && for_stack[for_sp - 1].var_idx != vi) {
        int found = -1, f;
        for (f = for_sp - 1; f >= 0; f--) {
            if (for_stack[f].var_idx == vi) { found = f; break; }
        }
        if (found >= 0) for_sp = found + 1;
        else { report_error("NEXT VARIABLE MISMATCH"); return; }
    }

    {
        ForFrame *ff = &for_stack[for_sp - 1];
        int var_i = ff->var_idx;
        vars[var_i].value = (short)(vars[var_i].value + ff->step);

        if ((ff->step > 0 && vars[var_i].value <= ff->target) ||
            (ff->step < 0 && vars[var_i].value >= ff->target)) {
            pc = ff->line_idx;
        } else {
            for_sp--;
        }
    }
}

static void cmd_if(void) {
    int cond;
    skip_spaces();
    cond = (int)parse_expression();
    if (!running) return;
    skip_spaces();
    if (match_keyword("THEN") || match_keyword("GOTO")) {
        /* matched */
    }
    skip_spaces();
    if (cond) {
        if (isdigit((unsigned char)*parser_ptr)) { cmd_goto(); }
        else { exec_statement(); }
    } else {
        parser_ptr += strlen(parser_ptr);
    }
}

static void cmd_dim(void) {
    while (running) {
        char vn[VAR_NAME_SIG + 1];
        char *look;
        int size, is_str = 0;

        skip_spaces();
        if (!isalpha((unsigned char)*parser_ptr)) {
            report_error("EXPECTED VARIABLE"); return;
        }

        look = parser_ptr;
        while (isalnum((unsigned char)*look)) look++;
        if (*look == '$') is_str = 1;

        if (is_str) {
            parse_varname(vn);
            if (*parser_ptr == '$') parser_ptr++;
            skip_spaces();
            if (*parser_ptr == '(') {
                parser_ptr++;
                size = (int)parse_expression();
                skip_spaces();
                if (*parser_ptr == ')') parser_ptr++;
                if (size <= 0) size = 255;
                if (num_strings >= MAX_STRINGS || string_pool_used + size >= STRING_POOL_SIZE) {
                    report_error("STRING MEMORY FULL"); return;
                }
                strncpy(strings[num_strings].name, vn, VAR_NAME_SIG);
                strings[num_strings].name[VAR_NAME_SIG] = '\0';
                strings[num_strings].offset = string_pool_used;
                strings[num_strings].length = 0;
                strings[num_strings].max_length = size;
                string_pool_used += size;
                num_strings++;
            }
        } else {
            parse_varname(vn);
            skip_spaces();
            if (*parser_ptr == '(') {
                parser_ptr++;
                size = (int)parse_expression();
                skip_spaces();
                if (*parser_ptr == ')') parser_ptr++;
                if (size <= 0) size = 10;
                size++;
                if (num_arrays >= MAX_ARRAYS || array_pool_used + size >= ARRAY_POOL_SIZE) {
                    report_error("ARRAY MEMORY FULL"); return;
                }
                strncpy(arrays[num_arrays].name, vn, VAR_NAME_SIG);
                arrays[num_arrays].name[VAR_NAME_SIG] = '\0';
                arrays[num_arrays].offset = array_pool_used;
                arrays[num_arrays].size = size;
                memset(&array_pool[array_pool_used], 0, size * sizeof(short));
                array_pool_used += size;
                num_arrays++;
            }
        }
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        else break;
    }
}

static void cmd_on_goto(void) {
    int idx, count = 0;
    int is_gosub = 0;
    skip_spaces();
    idx = (int)parse_expression();
    skip_spaces();
    if (match_keyword("GOSUB")) is_gosub = 1;
    else match_keyword("GOTO");

    while (running && *parser_ptr != '\0' && *parser_ptr != ':') {
        int target;
        skip_spaces();
        target = (int)parse_expression();
        count++;
        if (count == idx) {
            int line_i = find_line(target);
            if (line_i < 0) { report_error("LINE NOT FOUND"); return; }
            if (is_gosub) {
                if (gosub_sp >= GOSUB_STACK_SIZE) { report_error("GOSUB OVERFLOW"); return; }
                gosub_stack[gosub_sp++] = pc + 1;
            }
            pc = line_i;
            return;
        }
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        else break;
    }
}

static void cmd_help(void) {
    printf("=== APPLE II INTEGER BASIC HELP ===\n");
    printf("COMMANDS:   RUN [file], LIST [range], LOAD [file], SAVE [file], NEW, DEL, AUTO, MAN, CLR, CON, BYE/GOODBYE, HELP\n");
    printf("STATEMENTS: PRINT/?, INPUT, LET, GOTO, GOSUB, RETURN, IF..THEN, FOR..TO..STEP, NEXT,\n");
    printf("            DIM, REM, STOP, END, POKE, CALL, PR#, IN#, TEXT, GR, COLOR=, PLOT, HLIN, VLIN, HOME,\n");
    printf("            NORMAL, INVERSE, FLASH, SPEED=, TRACE, NOTRACE, DSP, NODSP, HIMEM:, LOMEM:, BEEP\n");
    printf("FUNCTIONS:  PEEK, SCRN, SGN, ABS, RND, PDL, PTR, LEN, ASC, VAL, CHR$, STR$, TAB, SPC\n");
    printf("VARIABLES:  A-Z, A0-Z9, DIM A$(len) (strings with slicing A$(s,e)), DIM A(dim) (1D arrays)\n");
}

static void exec_statement(void) {
    skip_spaces();
    if (*parser_ptr == '\0' || *parser_ptr == ':') return;

    if (match_keyword("PRINT") || *parser_ptr == '?') {
        if (*parser_ptr == '?') parser_ptr++;
        cmd_print();
        return;
    }
    if (match_keyword("LET"))   { cmd_let(); return; }
    if (match_keyword("INPUT")) { cmd_input(); return; }
    if (match_keyword("GOTO"))  { cmd_goto(); return; }
    if (match_keyword("GOSUB")) { cmd_gosub(); return; }
    if (match_keyword("RETURN")){ cmd_return(); return; }
    if (match_keyword("FOR"))   { cmd_for(); return; }
    if (match_keyword("NEXT"))  { cmd_next(); return; }
    if (match_keyword("IF"))    { cmd_if(); return; }
    if (match_keyword("DIM"))   { cmd_dim(); return; }
    if (match_keyword("ON"))    { cmd_on_goto(); return; }
    if (match_keyword("REM"))   { parser_ptr += strlen(parser_ptr); return; }
    if (match_keyword("END"))   { running = 0; return; }
    if (match_keyword("STOP"))  { stopped_pc = pc; running = 0; printf("STOPPED AT LINE %d\n", pgm[pc].line_number); return; }
    if (match_keyword("BEEP"))  { trigger_beep(); return; }

    /* Display & Graphics Safe Simulation */
    if (match_keyword("HOME") || match_keyword("CLS")) {
        printf("\033[2J\033[H"); fflush(stdout); return;
    }
    if (match_keyword("TEXT") || match_keyword("GR") || match_keyword("NORMAL") ||
        match_keyword("INVERSE") || match_keyword("FLASH") || match_keyword("NOTRACE")) {
        trace_on = 0; return;
    }
    if (match_keyword("TRACE")) { trace_on = 1; return; }
    if (match_keyword("COLOR=")) { (void)parse_expression(); return; }
    if (match_keyword("PLOT"))  { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); return; }
    if (match_keyword("HLIN"))  { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); match_keyword("AT"); (void)parse_expression(); return; }
    if (match_keyword("VLIN"))  { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); match_keyword("AT"); (void)parse_expression(); return; }
    if (match_keyword("SPEED=")) { (void)parse_expression(); return; }

    /* Memory / System Safe Simulation */
    if (match_keyword("POKE")) { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); return; }
    if (match_keyword("CALL")) { (void)parse_expression(); return; }
    if (match_keyword("PR#"))  { (void)parse_expression(); return; }
    if (match_keyword("IN#"))  { (void)parse_expression(); return; }
    if (match_keyword("HIMEM:")) { (void)parse_expression(); return; }
    if (match_keyword("LOMEM:")) { (void)parse_expression(); return; }
    if (match_keyword("DSP"))  { parse_varname(dsp_var); return; }
    if (match_keyword("NODSP")){ dsp_var[0] = '\0'; return; }

    /* Commands */
    if (match_keyword("RUN")) {
        skip_spaces();
        if (*parser_ptr != '\0') load_program(parser_ptr);
        run_program();
        return;
    }
    if (match_keyword("LIST")) { list_program(parser_ptr); return; }
    if (match_keyword("NEW"))  { clear_program(); return; }
    if (match_keyword("SAVE")) { save_program(parser_ptr); return; }
    if (match_keyword("LOAD")) { load_program(parser_ptr); return; }
    if (match_keyword("HELP")) { cmd_help(); return; }

    if (match_keyword("DEL")) {
        int lo = 0, hi = 32767;
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

    if (match_keyword("BYE") || match_keyword("GOODBYE") || match_keyword("SYSTEM") || match_keyword("QUIT")) {
        exit(0);
    }

    /* Bare assignment fallback (e.g. A = 5) */
    if (isalpha((unsigned char)*parser_ptr)) {
        cmd_let();
        return;
    }

    report_error("SYNTAX ERROR");
}

static void exec_line_statements(char *line) {
    int start_pc = pc;
    strncpy(curr_line_buf, line, LINE_LEN);
    curr_line_buf[LINE_LEN] = '\0';
    parser_ptr = curr_line_buf;

    while (*parser_ptr != '\0' && running) {
        skip_spaces();
        if (*parser_ptr == '\0') break;
        exec_statement();
        if (pc != start_pc || !running) break;
        skip_spaces();
        if (*parser_ptr == ':') parser_ptr++;
    }
}

static void run_program(void) {
    if (num_lines == 0) return;
    clear_variables();
    pc = 0;
    running = 1;

    while (running && pc >= 0 && pc < num_lines) {
        int prev = pc;
        if (trace_on) printf("# %d\n", pgm[pc].line_number);
        exec_line_statements(pgm[pc].text);
        if (running && pc == prev) pc++;
    }
    running = 0;
}

int main(int argc, char **argv) {
    char input_buf[LINE_LEN + 20];
    int batch_mode = 0;
    int file_arg_idx = 0;
    int i;

    srand((unsigned int)time(NULL));
    clear_program();

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--batch") == 0 || strcmp(argv[i], "-q") == 0) {
            batch_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: apple2 [options] [filename.bas]\n");
            printf("Options:\n");
            printf("  -b, --batch   Run in batch mode and exit after program completes\n");
            printf("  -h, --help    Show this help message\n");
            return 0;
        } else if (argv[i][0] != '-' && file_arg_idx == 0) {
            file_arg_idx = i;
        }
    }

    printf("APPLE II INTEGER BASIC v1.2\n\n");

    if (file_arg_idx > 0) {
        load_program(argv[file_arg_idx]);
        run_program();
        if (batch_mode) {
            return 0;
        }
    }

    while (1) {
        char *p;
        if (auto_mode) {
            printf("%d ", auto_line);
            fflush(stdout);
        } else {
            printf("] ");
            fflush(stdout);
        }

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) break;

        {
            int slen = (int)strlen(input_buf);
            while (slen > 0 && (input_buf[slen - 1] == '\n' || input_buf[slen - 1] == '\r'))
                input_buf[--slen] = '\0';
        }

        p = input_buf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') continue;

        if (auto_mode && !isdigit((unsigned char)*p)) {
            char combined[LINE_LEN + 20];
            sprintf(combined, "%d %s", auto_line, p);
            store_line(combined);
            auto_line += auto_step;
            continue;
        }

        if (isdigit((unsigned char)*p)) {
            store_line(p);
            if (auto_mode) auto_line += auto_step;
        } else {
            running = 1;
            exec_line_statements(p);
            running = 0;
        }
    }
    return 0;
}

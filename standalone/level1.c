// FILENAME: level1.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// DESCRIPTION: Complete TRS-80 Level I / Level II BASIC interpreter in freestanding C89.

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
#define NUM_VARS          26
#define MAX_ARRAY_SIZE    2048
#define MAX_STR_LEN       255
#define MAX_DATA          512
#define TAB_WIDTH         14
#define GOSUB_STACK_SIZE  64
#define FOR_STACK_SIZE    32
#define SCREEN_W          128
#define SCREEN_H          48

typedef struct {
    int line_number;
    char text[LINE_LEN + 1];
} ProgramLine;

typedef struct {
    int var_idx;
    short target;
    short step;
    int line_idx;
} ForFrame;

/* Program Storage */
static ProgramLine pgm[MAX_LINES];
static int num_lines = 0;

/* Variables */
static short vars[NUM_VARS];
static short array_data[MAX_ARRAY_SIZE];
static int   array_size = 100;
static char  string_vars[NUM_VARS][MAX_STR_LEN + 1];

/* Data Pool */
static short data_pool[MAX_DATA];
static int   data_count = 0;
static int   data_ptr = 0;

/* Virtual Screen for SET/RESET/POINT */
static unsigned char vscreen[SCREEN_H][SCREEN_W / 8];

/* Execution Stacks & State */
static int   gosub_stack[GOSUB_STACK_SIZE];
static int   gosub_sp = 0;

static ForFrame for_stack[FOR_STACK_SIZE];
static int   for_sp = 0;

static int   pc = 0;
static int   running = 0;
static int   stopped_pc = -1;
static int   print_col = 0;

static char *parser_ptr = NULL;
static char  curr_line_buf[LINE_LEN + 1];

/* Forward Declarations */
static void  clear_variables(void);
static void  clear_program(void);
static void  skip_spaces(void);
static int   match_keyword(const char *kw);
static int   peek_keyword(const char *kw);
static int   find_line(int ln);
static void  store_line(const char *raw);
static void  build_data_pool(void);
static void  save_program(const char *filename);
static void  load_program(const char *filename);
static void  list_program(const char *args);
static void  run_program(void);
static void  exec_statement(void);
static void  exec_line_statements(char *line);
static short parse_expression(void);
static short parse_relational(void);

static void trigger_beep(void) {
    printf("\a");
    fflush(stdout);
}

static void report_error(const char *msg) {
    if (running && pc >= 0 && pc < num_lines) {
        printf("%s IN LINE %d\n", msg, pgm[pc].line_number);
    } else {
        printf("%s\n", msg);
    }
    running = 0;
}

static void clear_variables(void) {
    int i;
    for (i = 0; i < NUM_VARS; i++) {
        vars[i] = 0;
        string_vars[i][0] = '\0';
    }
    memset(array_data, 0, sizeof(array_data));
    memset(vscreen, 0, sizeof(vscreen));
    gosub_sp = 0;
    for_sp = 0;
    data_ptr = 0;
    print_col = 0;
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
        if (kw[len - 1] == '.') {
            parser_ptr += len;
            return 1;
        }
        if (!isalnum((unsigned char)next) && next != '$') {
            parser_ptr += len;
            return 1;
        }
    }
    return 0;
}

static int peek_keyword(const char *kw) {
    int len;
    skip_spaces();
    len = (int)strlen(kw);
    if (my_strncasecmp(parser_ptr, kw, len) == 0) {
        char next = parser_ptr[len];
        if (kw[len - 1] == '.') return 1;
        if (!isalnum((unsigned char)next) && next != '$') {
            return 1;
        }
    }
    return 0;
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
        printf("OUT OF MEMORY\n");
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

static void build_data_pool(void) {
    int i;
    data_count = 0;
    data_ptr = 0;

    for (i = 0; i < num_lines; i++) {
        char buf[LINE_LEN + 1];
        char *p = buf;
        strncpy(buf, pgm[i].text, LINE_LEN);
        buf[LINE_LEN] = '\0';

        while (*p) {
            while (*p == ' ' || *p == '\t') p++;
            if (strncmp(p, "DATA", 4) == 0 && !isalnum((unsigned char)p[4])) {
                p += 4;
                while (*p && *p != ':') {
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == '+' || *p == '-' || isdigit((unsigned char)*p)) {
                        short val = (short)strtol(p, &p, 10);
                        if (data_count < MAX_DATA) data_pool[data_count++] = val;
                    }
                    while (*p == ' ' || *p == '\t') p++;
                    if (*p == ',') p++;
                    else if (*p == ':') break;
                    else if (*p) p++;
                }
            } else {
                while (*p && *p != ':') p++;
                if (*p == ':') p++;
            }
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
        printf("SYNTAX ERROR\n");
        return;
    }
    resolve_filename(filename, fname, sizeof(fname));
    fp = fopen(fname, "w");
    if (!fp) {
        printf("FILE ERROR\n");
        return;
    }
    for (i = 0; i < num_lines; i++) {
        fprintf(fp, "%d %s\n", pgm[i].line_number, pgm[i].text);
    }
    fclose(fp);
    printf("READY\n");
}

static void load_program(const char *filename) {
    FILE *fp;
    char fname[256];
    char buf[LINE_LEN + 20];
    if (filename == NULL || *filename == '\0') {
        printf("SYNTAX ERROR\n");
        return;
    }
    resolve_filename(filename, fname, sizeof(fname));
    fp = fopen(fname, "r");
    if (!fp) {
        printf("FILE NOT FOUND\n");
        return;
    }
    clear_program();
    while (fgets(buf, sizeof(buf), fp)) {
        int blen = (int)strlen(buf);
        while (blen > 0 && (buf[blen - 1] == '\n' || buf[blen - 1] == '\r')) buf[--blen] = '\0';
        if (buf[0] != '\0') store_line(buf);
    }
    fclose(fp);
    printf("READY\n");
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
 * EXPRESSION EVALUATION (TRS-80 LEVEL I FLAT LEFT-TO-RIGHT)
 * ========================================================================= */

static short parse_atom(void) {
    short val = 0;
    if (!running) return 0;
    skip_spaces();

    if (*parser_ptr == '-') {
        parser_ptr++;
        return (short)-parse_atom();
    }
    if (*parser_ptr == '+') {
        parser_ptr++;
        return parse_atom();
    }

    if (*parser_ptr == '(') {
        parser_ptr++;
        val = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        return val;
    }

    if (isdigit((unsigned char)*parser_ptr)) {
        return (short)strtol(parser_ptr, &parser_ptr, 10);
    }

    /* ABS(X) */
    if (match_keyword("ABS")) {
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            val = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            val = parse_atom();
        }
        return (val < 0 ? (short)-val : val);
    }

    /* INT(X) */
    if (match_keyword("INT")) {
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            val = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            val = parse_atom();
        }
        return val;
    }

    /* SGN(X) */
    if (match_keyword("SGN")) {
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            val = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            val = parse_atom();
        }
        if (val > 0) return 1;
        if (val < 0) return -1;
        return 0;
    }

    /* RND(X) - 1 to X */
    if (match_keyword("RND")) {
        short upper;
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            upper = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            upper = parse_atom();
        }
        if (upper <= 0) upper = 1;
        return (short)((rand() % upper) + 1);
    }

    /* MEM / M. */
    if (match_keyword("MEM") || match_keyword("M.")) {
        long used = (long)num_lines * (long)(LINE_LEN + 2) + (long)data_count * 2L;
        long free_mem = (long)BASIC_RAM_SIZE - used;
        if (free_mem > 32767) free_mem = 32767;
        if (free_mem < 0) free_mem = 0;
        return (short)free_mem;
    }

    /* TAB(X) / T.(X) */
    if (match_keyword("TAB") || match_keyword("T.")) {
        short col;
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            col = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            col = parse_atom();
        }
        if (col < 0) col = 0;
        while (print_col < col) {
            putchar(' ');
            print_col++;
        }
        return 0;
    }

    /* POINT(X, Y) */
    if (match_keyword("POINT")) {
        short x, y;
        skip_spaces();
        if (*parser_ptr == '(') parser_ptr++;
        x = parse_expression();
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        y = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return 0;
        return (vscreen[y][x / 8] & (1 << (x % 8))) ? 1 : 0;
    }

    /* PEEK(addr) */
    if (match_keyword("PEEK")) {
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            (void)parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            (void)parse_atom();
        }
        return 0;
    }

    /* INP(port) */
    if (match_keyword("INP")) {
        skip_spaces();
        if (*parser_ptr == '(') {
            parser_ptr++;
            (void)parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
        } else {
            (void)parse_atom();
        }
        return 0;
    }

    /* Variable A-Z or Array A(X) or String A$ */
    if (isalpha((unsigned char)*parser_ptr)) {
        char v = (char)toupper((unsigned char)*parser_ptr++);
        int idx = v - 'A';
        skip_spaces();
        if (*parser_ptr == '$') {
            parser_ptr++;
            return 0;
        }
        if (v == 'A' && *parser_ptr == '(') {
            short arr_idx;
            parser_ptr++;
            arr_idx = parse_expression();
            skip_spaces();
            if (*parser_ptr == ')') parser_ptr++;
            if (arr_idx < 0 || arr_idx >= MAX_ARRAY_SIZE) {
                report_error("SUBSCRIPT OUT OF RANGE");
                return 0;
            }
            return array_data[arr_idx];
        }
        if (idx >= 0 && idx < NUM_VARS) return vars[idx];
    }

    return 0;
}

static short parse_expression(void) {
    short val;
    if (!running) return 0;
    val = parse_atom();

    while (running) {
        char op;
        short rhs;
        skip_spaces();
        op = *parser_ptr;
        if (op != '+' && op != '-' && op != '*' && op != '/') break;
        parser_ptr++;
        rhs = parse_atom();
        if (op == '+') val = (short)(val + rhs);
        else if (op == '-') val = (short)(val - rhs);
        else if (op == '*') val = (short)(val * rhs);
        else if (op == '/') {
            if (rhs == 0) { report_error("DIVISION BY ZERO"); return 0; }
            val = (short)(val / rhs);
        }
    }
    return val;
}

static short parse_relational(void) {
    short lhs;
    if (!running) return 0;
    skip_spaces();

    /* String comparison check */
    if (isalpha((unsigned char)*parser_ptr)) {
        char *look = parser_ptr;
        while (isalnum((unsigned char)*look)) look++;
        if (*look == '$') {
            char v = (char)toupper((unsigned char)*parser_ptr++);
            int idx = v - 'A';
            char op1, op2;
            char rhs_str[MAX_STR_LEN + 1];
            if (*parser_ptr == '$') parser_ptr++;
            skip_spaces();
            op1 = *parser_ptr;
            if (op1 == '=' || op1 == '<' || op1 == '>' || op1 == '#') {
                parser_ptr++;
                op2 = *parser_ptr;
                if (op2 == '=' || op2 == '>') parser_ptr++;
                else op2 = '\0';
                skip_spaces();
                rhs_str[0] = '\0';
                if (*parser_ptr == '"') {
                    int k = 0;
                    parser_ptr++;
                    while (*parser_ptr && *parser_ptr != '"' && k < MAX_STR_LEN) rhs_str[k++] = *parser_ptr++;
                    rhs_str[k] = '\0';
                    if (*parser_ptr == '"') parser_ptr++;
                } else if (isalpha((unsigned char)*parser_ptr)) {
                    char rv = (char)toupper((unsigned char)*parser_ptr++);
                    int ri = rv - 'A';
                    if (*parser_ptr == '$') parser_ptr++;
                    if (ri >= 0 && ri < NUM_VARS) strcpy(rhs_str, string_vars[ri]);
                }
                if (idx >= 0 && idx < NUM_VARS) {
                    int cmp = strcmp(string_vars[idx], rhs_str);
                    if (op1 == '=' && op2 == '\0') return (short)(cmp == 0 ? 1 : 0);
                    if (op1 == '#' || (op1 == '<' && op2 == '>')) return (short)(cmp != 0 ? 1 : 0);
                    if (op1 == '<' && op2 == '\0') return (short)(cmp < 0 ? 1 : 0);
                    if (op1 == '>' && op2 == '\0') return (short)(cmp > 0 ? 1 : 0);
                    if (op1 == '<' && op2 == '=') return (short)(cmp <= 0 ? 1 : 0);
                    if (op1 == '>' && op2 == '=') return (short)(cmp >= 0 ? 1 : 0);
                }
            }
            return 0;
        }
    }

    lhs = parse_expression();
    skip_spaces();

    if (*parser_ptr == '<' || *parser_ptr == '>' || *parser_ptr == '=' || *parser_ptr == '#') {
        char op1 = *parser_ptr++;
        char op2 = '\0';
        short rhs;
        if (*parser_ptr == '=' || *parser_ptr == '>') op2 = *parser_ptr++;
        rhs = parse_expression();

        if (op1 == '=' && op2 == '\0') return (short)(lhs == rhs ? 1 : 0);
        if (op1 == '#' || (op1 == '<' && op2 == '>')) return (short)(lhs != rhs ? 1 : 0);
        if (op1 == '<' && op2 == '\0') return (short)(lhs < rhs ? 1 : 0);
        if (op1 == '>' && op2 == '\0') return (short)(lhs > rhs ? 1 : 0);
        if (op1 == '<' && op2 == '=') return (short)(lhs <= rhs ? 1 : 0);
        if (op1 == '>' && op2 == '=') return (short)(lhs >= rhs ? 1 : 0);
    }
    return lhs;
}

/* =========================================================================
 * STATEMENT HANDLERS
 * ========================================================================= */

static void cmd_print(void) {
    int need_nl = 1;
    skip_spaces();
    if (*parser_ptr == '\0' || *parser_ptr == ':') { putchar('\n'); print_col = 0; return; }

    while (*parser_ptr != '\0' && *parser_ptr != ':' && running) {
        skip_spaces();
        if (*parser_ptr == '\0' || *parser_ptr == ':') break;

        if (*parser_ptr == '"') {
            parser_ptr++;
            while (*parser_ptr && *parser_ptr != '"') {
                putchar(*parser_ptr++);
                print_col++;
            }
            if (*parser_ptr == '"') parser_ptr++;
            need_nl = 1;
        } else if (*parser_ptr == ';') {
            parser_ptr++;
            need_nl = 0;
        } else if (*parser_ptr == ',') {
            int next_tab;
            parser_ptr++;
            next_tab = ((print_col / TAB_WIDTH) + 1) * TAB_WIDTH;
            while (print_col < next_tab) { putchar(' '); print_col++; }
            need_nl = 0;
        } else if (peek_keyword("TAB") || peek_keyword("T.")) {
            parse_atom();
            need_nl = 1;
        } else {
            /* Check for string variable A$ */
            char *look = parser_ptr;
            while (isalnum((unsigned char)*look)) look++;
            if (*look == '$') {
                char v = (char)toupper((unsigned char)*parser_ptr++);
                int idx = v - 'A';
                if (*parser_ptr == '$') parser_ptr++;
                if (idx >= 0 && idx < NUM_VARS) {
                    printf("%s", string_vars[idx]);
                    print_col += (int)strlen(string_vars[idx]);
                }
                need_nl = 1;
            } else {
                short val = parse_expression();
                if (!running) return;
                if (val >= 0) { printf(" "); print_col++; }
                printf("%d ", (int)val);
                {
                    int n = val, w = 2;
                    if (n < 0) { w++; n = -n; } else { w++; }
                    while (n >= 10) { w++; n /= 10; }
                    print_col += w;
                }
                need_nl = 1;
            }
        }
    }
    if (need_nl && running) { putchar('\n'); print_col = 0; }
}

static void cmd_let(void) {
    char v;
    int idx;
    char *look;

    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) { report_error("SYNTAX ERROR"); return; }

    look = parser_ptr;
    while (isalnum((unsigned char)*look)) look++;

    if (*look == '$') {
        v = (char)toupper((unsigned char)*parser_ptr++);
        idx = v - 'A';
        if (*parser_ptr == '$') parser_ptr++;
        skip_spaces();
        if (*parser_ptr != '=') { report_error("SYNTAX ERROR"); return; }
        parser_ptr++;
        skip_spaces();
        if (*parser_ptr == '"') {
            int k = 0;
            parser_ptr++;
            while (*parser_ptr && *parser_ptr != '"' && k < MAX_STR_LEN) {
                if (idx >= 0 && idx < NUM_VARS) string_vars[idx][k++] = *parser_ptr;
                parser_ptr++;
            }
            if (idx >= 0 && idx < NUM_VARS) string_vars[idx][k] = '\0';
            if (*parser_ptr == '"') parser_ptr++;
        } else if (isalpha((unsigned char)*parser_ptr)) {
            char rv = (char)toupper((unsigned char)*parser_ptr++);
            int ri = rv - 'A';
            if (*parser_ptr == '$') parser_ptr++;
            if (idx >= 0 && idx < NUM_VARS && ri >= 0 && ri < NUM_VARS) {
                strcpy(string_vars[idx], string_vars[ri]);
            }
        }
        return;
    }

    v = (char)toupper((unsigned char)*parser_ptr++);
    idx = v - 'A';
    skip_spaces();

    if (v == 'A' && *parser_ptr == '(') {
        short arr_idx;
        parser_ptr++;
        arr_idx = parse_expression();
        skip_spaces();
        if (*parser_ptr == ')') parser_ptr++;
        skip_spaces();
        if (*parser_ptr != '=') { report_error("SYNTAX ERROR"); return; }
        parser_ptr++;
        if (arr_idx < 0 || arr_idx >= MAX_ARRAY_SIZE) {
            report_error("SUBSCRIPT OUT OF RANGE"); return;
        }
        array_data[arr_idx] = parse_expression();
        return;
    }

    if (*parser_ptr != '=') { report_error("SYNTAX ERROR"); return; }
    parser_ptr++;
    if (idx >= 0 && idx < NUM_VARS) {
        vars[idx] = parse_expression();
    }
}

static void cmd_input(void) {
    char buf[LINE_LEN + 1];
    char *look;

    skip_spaces();
    if (*parser_ptr == '"') {
        parser_ptr++;
        while (*parser_ptr && *parser_ptr != '"') putchar(*parser_ptr++);
        if (*parser_ptr == '"') parser_ptr++;
        skip_spaces();
        if (*parser_ptr == ';' || *parser_ptr == ',') parser_ptr++;
    }

    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) { report_error("SYNTAX ERROR"); return; }

    look = parser_ptr;
    while (isalnum((unsigned char)*look)) look++;

    if (*look == '$') {
        char v = (char)toupper((unsigned char)*parser_ptr++);
        int idx = v - 'A';
        if (*parser_ptr == '$') parser_ptr++;
        printf("? "); fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            int blen = (int)strlen(buf);
            while (blen > 0 && (buf[blen - 1] == '\n' || buf[blen - 1] == '\r')) buf[--blen] = '\0';
            if (idx >= 0 && idx < NUM_VARS) {
                strncpy(string_vars[idx], buf, MAX_STR_LEN);
                string_vars[idx][MAX_STR_LEN] = '\0';
            }
        }
        return;
    }

    {
        char v = (char)toupper((unsigned char)*parser_ptr++);
        int idx = v - 'A';
        printf("? "); fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            if (idx >= 0 && idx < NUM_VARS) vars[idx] = (short)strtol(buf, NULL, 10);
        }
    }
}

static void cmd_goto(void) {
    short target;
    int idx;
    skip_spaces();
    target = parse_expression();
    idx = find_line(target);
    if (idx < 0) { report_error("UNDEFINED LINE"); return; }
    pc = idx;
}

static void cmd_gosub(void) {
    short target;
    int idx;
    skip_spaces();
    target = parse_expression();
    idx = find_line(target);
    if (idx < 0) { report_error("UNDEFINED LINE"); return; }
    if (gosub_sp >= GOSUB_STACK_SIZE) { report_error("GOSUB STACK OVERFLOW"); return; }
    gosub_stack[gosub_sp++] = pc + 1;
    pc = idx;
}

static void cmd_return(void) {
    if (gosub_sp <= 0) { report_error("RETURN WITHOUT GOSUB"); return; }
    pc = gosub_stack[--gosub_sp];
}

static void cmd_for(void) {
    char v;
    int vi;
    short start_val, limit_val, step_val = 1;

    skip_spaces();
    if (!isalpha((unsigned char)*parser_ptr)) { report_error("SYNTAX ERROR"); return; }
    v = (char)toupper((unsigned char)*parser_ptr++);
    vi = v - 'A';
    if (vi < 0 || vi >= NUM_VARS) return;

    skip_spaces();
    if (*parser_ptr != '=') { report_error("SYNTAX ERROR"); return; }
    parser_ptr++;

    start_val = parse_expression();
    vars[vi] = start_val;

    skip_spaces();
    if (!match_keyword("TO")) { report_error("SYNTAX ERROR"); return; }

    limit_val = parse_expression();

    skip_spaces();
    if (match_keyword("STEP")) {
        step_val = parse_expression();
    }

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
    int vi = -1;
    skip_spaces();
    if (isalpha((unsigned char)*parser_ptr)) {
        char v = (char)toupper((unsigned char)*parser_ptr++);
        vi = v - 'A';
    }
    if (for_sp <= 0) { report_error("NEXT WITHOUT FOR"); return; }

    if (vi >= 0 && for_stack[for_sp - 1].var_idx != vi) {
        int found = -1, f;
        for (f = for_sp - 1; f >= 0; f--) {
            if (for_stack[f].var_idx == vi) { found = f; break; }
        }
        if (found >= 0) for_sp = found + 1;
        else { report_error("NEXT WITHOUT FOR"); return; }
    }

    {
        ForFrame *ff = &for_stack[for_sp - 1];
        int var_i = ff->var_idx;
        vars[var_i] = (short)(vars[var_i] + ff->step);

        if ((ff->step > 0 && vars[var_i] <= ff->target) ||
            (ff->step < 0 && vars[var_i] >= ff->target)) {
            pc = ff->line_idx;
        } else {
            for_sp--;
        }
    }
}

static void cmd_if(void) {
    short cond;
    skip_spaces();
    cond = parse_relational();
    if (!running) return;
    skip_spaces();
    if (!match_keyword("THEN") && !match_keyword("GOTO")) {
        report_error("SYNTAX ERROR"); return;
    }
    skip_spaces();
    if (cond) {
        if (isdigit((unsigned char)*parser_ptr)) cmd_goto();
        else exec_statement();
    } else {
        parser_ptr += strlen(parser_ptr);
    }
}

static void cmd_on_goto(void) {
    short selector;
    int count = 0;
    int is_gosub = 0;
    skip_spaces();
    selector = parse_expression();
    skip_spaces();
    if (match_keyword("GOSUB") || match_keyword("GS.")) is_gosub = 1;
    else if (!match_keyword("GOTO") && !match_keyword("G.")) {
        report_error("SYNTAX ERROR"); return;
    }

    while (running && *parser_ptr != '\0' && *parser_ptr != ':') {
        short target;
        skip_spaces();
        target = parse_expression();
        count++;
        if (count == selector) {
            int idx = find_line(target);
            if (idx < 0) { report_error("UNDEFINED LINE"); return; }
            if (is_gosub) {
                if (gosub_sp >= GOSUB_STACK_SIZE) { report_error("GOSUB STACK OVERFLOW"); return; }
                gosub_stack[gosub_sp++] = pc + 1;
            }
            pc = idx;
            return;
        }
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        else break;
    }
}

static void cmd_read(void) {
    while (running) {
        char v;
        int idx;
        skip_spaces();
        if (!isalpha((unsigned char)*parser_ptr)) { report_error("SYNTAX ERROR"); return; }
        v = (char)toupper((unsigned char)*parser_ptr++);
        idx = v - 'A';
        if (data_ptr >= data_count) { report_error("OUT OF DATA"); return; }
        if (idx >= 0 && idx < NUM_VARS) vars[idx] = data_pool[data_ptr++];
        skip_spaces();
        if (*parser_ptr == ',') parser_ptr++;
        else break;
    }
}

static void cmd_set(void) {
    short x, y;
    skip_spaces();
    if (*parser_ptr == '(') parser_ptr++;
    x = parse_expression();
    skip_spaces();
    if (*parser_ptr == ',') parser_ptr++;
    y = parse_expression();
    skip_spaces();
    if (*parser_ptr == ')') parser_ptr++;
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        vscreen[y][x / 8] |= (1 << (x % 8));
    }
}

static void cmd_reset(void) {
    short x, y;
    skip_spaces();
    if (*parser_ptr == '(') parser_ptr++;
    x = parse_expression();
    skip_spaces();
    if (*parser_ptr == ',') parser_ptr++;
    y = parse_expression();
    skip_spaces();
    if (*parser_ptr == ')') parser_ptr++;
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        vscreen[y][x / 8] &= ~(1 << (x % 8));
    }
}

static void cmd_help(void) {
    printf("=== TRS-80 LEVEL I BASIC HELP ===\n");
    printf("COMMANDS:   RUN [file], LIST [range], LOAD/CLOAD [file], SAVE/CSAVE [file], NEW, CONT/C., CLEAR, BYE/GOODBYE, HELP\n");
    printf("STATEMENTS: PRINT/P./?, INPUT/I., LET/L., GOTO/G., GOSUB/GS., RETURN/R., IF..THEN/F.,\n");
    printf("            FOR..TO..STEP/F., NEXT/N., READ, DATA, RESTORE, REM, STOP/ST., END, POKE, CLS/CL., SET, RESET, POINT\n");
    printf("FUNCTIONS:  PEEK, MEM/M., TAB/T., RND, ABS, INT, SGN, POINT\n");
    printf("VARIABLES:  A-Z (16-bit integers), A(expr) (1D array), A$, B$ (strings A$-Z$)\n");
}

static void exec_statement(void) {
    skip_spaces();
    if (*parser_ptr == '\0' || *parser_ptr == ':') return;

    if (match_keyword("PRINT") || match_keyword("P.") || *parser_ptr == '?') {
        if (*parser_ptr == '?') parser_ptr++;
        cmd_print();
        return;
    }
    if (match_keyword("LET") || match_keyword("L."))     { cmd_let(); return; }
    if (match_keyword("INPUT") || match_keyword("I."))   { cmd_input(); return; }
    if (match_keyword("GOTO") || match_keyword("G."))    { cmd_goto(); return; }
    if (match_keyword("GOSUB") || match_keyword("GS."))  { cmd_gosub(); return; }
    if (match_keyword("RETURN") || match_keyword("R."))  { cmd_return(); return; }
    if (match_keyword("FOR") || match_keyword("F."))     { cmd_for(); return; }
    if (match_keyword("NEXT") || match_keyword("N."))    { cmd_next(); return; }
    if (match_keyword("IF"))                             { cmd_if(); return; }
    if (match_keyword("ON"))                             { cmd_on_goto(); return; }
    if (match_keyword("READ"))                           { cmd_read(); return; }
    if (match_keyword("RESTORE"))                        { data_ptr = 0; return; }
    if (match_keyword("DATA"))                           { while (*parser_ptr && *parser_ptr != ':') parser_ptr++; return; }
    if (match_keyword("REM"))                            { parser_ptr += strlen(parser_ptr); return; }
    if (match_keyword("END"))                            { running = 0; return; }
    if (match_keyword("STOP") || match_keyword("ST."))   {
        stopped_pc = pc; running = 0;
        printf("BREAK");
        if (pc >= 0 && pc < num_lines) printf(" IN LINE %d", pgm[pc].line_number);
        printf("\n");
        return;
    }
    if (match_keyword("CLEAR"))                          { clear_variables(); return; }
    if (match_keyword("BEEP"))                           { trigger_beep(); return; }

    /* Screen & Graphics */
    if (match_keyword("CLS") || match_keyword("CL."))    { printf("\033[2J\033[H"); fflush(stdout); return; }
    if (match_keyword("SET"))                            { cmd_set(); return; }
    if (match_keyword("RESET"))                          { cmd_reset(); return; }

    /* Memory / System Emulation */
    if (match_keyword("POKE")) { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); return; }
    if (match_keyword("OUT"))  { (void)parse_expression(); skip_spaces(); if (*parser_ptr==',') parser_ptr++; (void)parse_expression(); return; }

    /* Commands */
    if (match_keyword("RUN")) {
        skip_spaces();
        if (*parser_ptr != '\0') load_program(parser_ptr);
        run_program();
        return;
    }
    if (match_keyword("LIST"))                           { list_program(parser_ptr); return; }
    if (match_keyword("NEW"))                            { clear_program(); return; }
    if (match_keyword("SAVE") || match_keyword("CSAVE") || match_keyword("CS.")) { save_program(parser_ptr); return; }
    if (match_keyword("LOAD") || match_keyword("CLOAD")) { load_program(parser_ptr); return; }
    if (match_keyword("CONT") || match_keyword("C.")) {
        if (stopped_pc < 0) { printf("CAN'T CONTINUE\n"); }
        else { pc = stopped_pc; stopped_pc = -1; running = 1; }
        return;
    }
    if (match_keyword("HELP"))                           { cmd_help(); return; }

    if (match_keyword("BYE") || match_keyword("GOODBYE") || match_keyword("SYSTEM") || match_keyword("QUIT")) {
        printf("GOODBYE\n");
        exit(0);
    }

    /* Bare assignment fallback (e.g. A = 5 or A$ = "HELLO") */
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
    build_data_pool();
    pc = 0;
    running = 1;

    while (running && pc >= 0 && pc < num_lines) {
        int prev = pc;
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
            printf("Usage: level1 [options] [filename.bas]\n");
            printf("Options:\n");
            printf("  -b, --batch   Run in batch mode and exit after program completes\n");
            printf("  -h, --help    Show this help message\n");
            return 0;
        } else if (argv[i][0] != '-' && file_arg_idx == 0) {
            file_arg_idx = i;
        }
    }

    printf("TRS-80 LEVEL I BASIC v1.2\n\n");
    printf("READY\n");

    if (file_arg_idx > 0) {
        load_program(argv[file_arg_idx]);
        run_program();
        if (batch_mode) return 0;
    }

    while (1) {
        char *p;
        printf("> ");
        fflush(stdout);

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) break;

        {
            int slen = (int)strlen(input_buf);
            while (slen > 0 && (input_buf[slen - 1] == '\n' || input_buf[slen - 1] == '\r'))
                input_buf[--slen] = '\0';
        }

        p = input_buf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') continue;

        if (isdigit((unsigned char)*p)) {
            store_line(p);
        } else {
            running = 1;
            exec_line_statements(p);
            running = 0;
        }
    }
    return 0;
}

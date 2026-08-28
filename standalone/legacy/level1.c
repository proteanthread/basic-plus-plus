// FILENAME: level1.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// DESCRIPTION: Provides core logic and implementation for level1.c within BASIC++.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* =========================================================================
 * CONSTANTS AND TUNABLE LIMITS
 *
 * These control the memory budget and behaviour boundaries of the
 * interpreter.  All limits are intentionally conservative to keep
 * the interpreter small.
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_LINES: increase to allow more stored program lines
 *   - LINE_LEN:  increase for longer source lines (keep < 250)
 *   - MAX_DATA:  increase if programs use very large DATA blocks
 *   - TAB_WIDTH: change comma-tab spacing (original TRS-80 used 14)
 *
 * WHAT CANNOT BE CHANGED:
 *   - NUM_VARS must remain 26 (single-letter A-Z variables are
 *     baked into the parser and expression evaluator)
 *
 * WHAT TO EXPECT:
 *   Total static memory is roughly MAX_LINES * (LINE_LEN+2) plus
 *   small overhead for variables and the data pool.  With the
 *   defaults below, total is approximately 67 KB.
 *
 * IF SOMETHING BREAKS:
 *   If OUT OF MEMORY appears during line entry, increase MAX_LINES.
 *   If input is silently truncated, increase LINE_LEN.
 *   If OUT OF DATA appears unexpectedly, increase MAX_DATA.
 * ========================================================================= */

#define MAX_LINES   500
#define LINE_LEN    130
#define NUM_VARS    26
#define MAX_DATA    256
#define TAB_WIDTH   14

/* Total memory budget reported by the MEM keyword. */
#define TOTAL_MEM   ((long)MAX_LINES * (long)(LINE_LEN + 2) + \
                     (long)NUM_VARS * 2L + (long)MAX_DATA * 2L)

/* =========================================================================
 * PROGRAM STORAGE
 *
 * The stored program is an array of lines sorted in ascending order
 * by line number.  Each entry has a 16-bit line number and a fixed-
 * length character buffer holding the source text (everything after
 * the line number).
 *
 * WHAT CAN BE CHANGED:
 *   - Storage could be converted to a linked list or dynamic array
 *     if the fixed-size array is too limiting.
 *
 * WHAT CANNOT BE CHANGED:
 *   - Lines must remain sorted by line number; the binary-search
 *     lookup in find_line() and the sequential scan in the RUN
 *     loop both depend on sorted order.
 *
 * WHAT TO EXPECT:
 *   Inserting a line in the middle shifts all subsequent lines down
 *   by one slot (O(n) cost).  For a 500-line program this is fast
 *   enough to be imperceptible.
 *
 * IF SOMETHING BREAKS:
 *   If lines appear out of order in LIST output, check find_insert_pos().
 *   If store_line() silently drops lines, check the MAX_LINES limit.
 * ========================================================================= */

static short  line_nums[MAX_LINES];
static char   line_text[MAX_LINES][LINE_LEN + 2];
static int    program_count = 0;

/* =========================================================================
 * INTERPRETER STATE (variables, flags, counters)
 *
 * All mutable interpreter state lives here.  Variables are 16-bit
 * signed shorts (matching the TRS-80 integer model).  The running
 * flag, program counter (pc), and print column are updated during
 * program execution.
 *
 * WHAT CAN BE CHANGED:
 *   - Variable type could be widened to int or long for extended
 *     arithmetic, but that would break the 16-bit dialect contract.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The running flag must be cleared to halt execution (this is
 *     how errors, END, and STOP terminate the run loop).
 *   - pc is an index into the sorted line_nums[]/line_text[] arrays;
 *     it is NOT a line number.
 *
 * WHAT TO EXPECT:
 *   After RUN, all variables are zeroed.  After CLEAR, all variables
 *   are zeroed but the program is preserved.
 *
 * IF SOMETHING BREAKS:
 *   If GOTO jumps to the wrong line, verify that pc is being set to
 *   the array index (from find_line()), not the line number itself.
 * ========================================================================= */

static short  vars[NUM_VARS];
static int    running    = 0;
static int    pc         = 0;
static int    print_col  = 0;
static int    error_flag = 0;

/* =========================================================================
 * CONT / STOP SUBSYSTEM
 *
 * When a STOP statement executes, the interpreter halts and saves
 * the program counter so that CONT can resume execution from the
 * line following the STOP.
 *
 * WHAT CAN BE CHANGED:
 *   - cont_pc could be extended to also save/restore the data_ptr
 *     or variable state for a richer "continue" experience.
 *
 * WHAT CANNOT BE CHANGED:
 *   - CONT must print CAN'T CONTINUE if no STOP has occurred.
 *   - STOP must print BREAK IN LINE <n>.
 *
 * WHAT TO EXPECT:
 *   CONT resumes at the line AFTER the STOP.  Variables and the
 *   DATA pointer are NOT reset by CONT.
 *
 * IF SOMETHING BREAKS:
 *   If CONT resumes at the wrong line, check that cont_pc is set
 *   to (pc + 1), not pc.  If CAN'T CONTINUE fires incorrectly,
 *   check the stopped flag.
 * ========================================================================= */

static int    stopped    = 0;
static int    cont_pc    = 0;

/* =========================================================================
 * DATA / READ / RESTORE SUBSYSTEM
 *
 * DATA statements are declarative: they are skipped during normal
 * execution.  At the start of RUN, build_data_pool() scans every
 * stored line for DATA keywords and collects all comma-separated
 * integer values into a flat pool.  READ consumes values from the
 * pool sequentially.  RESTORE resets the read pointer to the start.
 *
 * WHAT CAN BE CHANGED:
 *   - MAX_DATA to increase/decrease the pool size
 *   - Value type could be extended to support strings or floats
 *
 * WHAT CANNOT BE CHANGED:
 *   - DATA must be scanned before the first READ (this happens
 *     automatically inside cmd_run())
 *   - RESTORE always resets to the beginning of the pool
 *   - DATA values are collected in line-number order
 *
 * WHAT TO EXPECT:
 *   - READ past the end of the pool produces OUT OF DATA error
 *   - DATA items are comma-separated signed integers
 *   - Multiple DATA statements across different lines are merged
 *     into a single sequential pool
 *
 * IF SOMETHING BREAKS:
 *   - Verify build_data_pool() is called at the start of cmd_run()
 *   - Check data_ptr bounds in exec_read()
 *   - Confirm parse_number() handles negative values via the sign
 *     check in build_data_pool()
 * ========================================================================= */

static short  data_pool[MAX_DATA];
static int    data_count = 0;
static int    data_ptr   = 0;

/* Reusable input buffer for the REPL. */
static char   input_buf[LINE_LEN + 2];

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
 * FORWARD DECLARATIONS
 * ========================================================================= */

static void   exec_line(char *line);
static short  parse_expr(char **p);
static short  parse_relational(char **p);

/*
 * uppercase_line
 * Converts an entire input line to uppercase, preserving string
 * literals inside double quotes.  These early machines only had
 * uppercase characters.
 */
static void uppercase_line(char *str)
{
    int in_quotes = 0;
    while (*str) {
        if (*str == '"') in_quotes = !in_quotes;
        else if (!in_quotes) *str = (char)toupper((unsigned char)*str);
        str++;
    }
}

/* =========================================================================
 * UTILITY: WHITESPACE AND KEYWORD MATCHING
 *
 * These small helpers are used throughout the parser and executor.
 * skip_spaces() advances a pointer past spaces and tabs.
 * match_keyword() checks for a case-insensitive keyword at the
 * current position and advances past it if found.  peek_keyword()
 * does the same check without advancing.
 *
 * WHAT CAN BE CHANGED:
 *   - skip_spaces() could be extended to skip other whitespace
 *     characters if needed (form feed, etc.)
 *
 * WHAT CANNOT BE CHANGED:
 *   - match_keyword() and peek_keyword() must enforce a word
 *     boundary after the keyword (prevents "PRINTERS" from
 *     matching "PRINT")
 *   - All toupper() calls must cast through (unsigned char) first
 *
 * WHAT TO EXPECT:
 *   Keywords are matched case-insensitively: "print", "PRINT",
 *   and "Print" all work.
 *
 * IF SOMETHING BREAKS:
 *   If a keyword is not recognized, check the word-boundary test
 *   at the end of match_keyword().  If the wrong keyword matches,
 *   check that the keyword table in exec_line() tests longer
 *   keywords before shorter prefixes.
 * ========================================================================= */

static void skip_spaces(char **p)
{
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

static int match_keyword(char **p, const char *kw)
{
    char *s = *p;
    while (*kw) {
        if ((char)toupper((unsigned char)*s) != *kw) {
            return 0;
        }
        s++;
        kw++;
    }
    if (isalpha((unsigned char)*s) || isdigit((unsigned char)*s)) {
        return 0;
    }
    *p = s;
    return 1;
}

static int peek_keyword(char *p, const char *kw)
{
    while (*kw) {
        if ((char)toupper((unsigned char)*p) != *kw) {
            return 0;
        }
        p++;
        kw++;
    }
    if (isalpha((unsigned char)*p) || isdigit((unsigned char)*p)) {
        return 0;
    }
    return 1;
}

/* =========================================================================
 * UTILITY: NUMERIC LITERAL PARSER
 *
 * Reads an unsigned decimal integer from the text, clamping at 32767
 * to stay within 16-bit signed range.
 *
 * WHAT CAN BE CHANGED:
 *   - The clamp value could be raised if arithmetic is widened
 *
 * WHAT CANNOT BE CHANGED:
 *   - The function only reads unsigned values; sign handling is
 *     done by callers (parse_atom for unary minus, build_data_pool
 *     for signed DATA values)
 *
 * WHAT TO EXPECT:
 *   Numbers larger than 32767 are silently clamped.  Leading zeros
 *   are consumed without error.
 *
 * IF SOMETHING BREAKS:
 *   If numbers parse incorrectly, check that the input pointer is
 *   positioned on a digit character before calling this function.
 * ========================================================================= */

static short parse_number(char **p)
{
    long val = 0;
    while (isdigit((unsigned char)**p)) {
        val = val * 10 + (**p - '0');
        if (val > 32767) {
            val = 32767;
        }
        (*p)++;
    }
    return (short)val;
}

/* =========================================================================
 * PROGRAM STORAGE: FIND, INSERT, DELETE
 *
 * Lines are stored in a sorted array.  Binary search locates lines
 * for GOTO/IF-THEN lookups and for replacement/deletion during
 * editing.  Insertion shifts elements to maintain sorted order.
 *
 * WHAT CAN BE CHANGED:
 *   - The storage could be replaced with a hash table for O(1)
 *     lookup, but sequential RUN iteration would then require
 *     a separate sorted index.
 *
 * WHAT CANNOT BE CHANGED:
 *   - The array must remain sorted by line number at all times.
 *   - Entering a line number with no text must delete that line.
 *
 * WHAT TO EXPECT:
 *   Entering "10 PRINT A" stores line 10.  Entering "10" alone
 *   (no text) deletes line 10.  Replacing line 10 with new text
 *   overwrites the existing line in place.
 *
 * IF SOMETHING BREAKS:
 *   If find_line() returns -1 for a line that should exist, the
 *   array may have become unsorted.  Check store_line() for off-by-
 *   one errors in the shift loop.
 * ========================================================================= */

static int find_line(short num)
{
    int lo = 0;
    int hi = program_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (line_nums[mid] == num) return mid;
        if (line_nums[mid] < num) lo = mid + 1;
        else                      hi = mid - 1;
    }
    return -1;
}

static int find_insert_pos(short num)
{
    int lo = 0;
    int hi = program_count;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (line_nums[mid] < num) lo = mid + 1;
        else                      hi = mid;
    }
    return lo;
}

static void store_line(short num, const char *text)
{
    int idx;
    int i;

    while (*text == ' ' || *text == '\t') {
        text++;
    }

    idx = find_line(num);

    /* Empty text after the line number means delete the line. */
    if (*text == '\0') {
        if (idx >= 0) {
            for (i = idx; i < program_count - 1; i++) {
                line_nums[i] = line_nums[i + 1];
                strcpy(line_text[i], line_text[i + 1]);
            }
            program_count--;
        }
        return;
    }

    /* Replace existing line in place. */
    if (idx >= 0) {
        strncpy(line_text[idx], text, LINE_LEN);
        line_text[idx][LINE_LEN] = '\0';
        return;
    }

    /* Insert new line at the correct sorted position. */
    if (program_count >= MAX_LINES) {
        printf("OUT OF MEMORY\n");
        return;
    }

    idx = find_insert_pos(num);
    for (i = program_count; i > idx; i--) {
        line_nums[i] = line_nums[i - 1];
        strcpy(line_text[i], line_text[i - 1]);
    }
    line_nums[idx] = num;
    strncpy(line_text[idx], text, LINE_LEN);
    line_text[idx][LINE_LEN] = '\0';
    program_count++;
}

/* =========================================================================
 * DATA POOL BUILDER
 *
 * Called once at the start of RUN.  Scans all stored program lines
 * in line-number order, looking for DATA statements.  Each DATA
 * line's comma-separated integer values are appended to the flat
 * data_pool[] array.
 *
 * WHAT CAN BE CHANGED:
 *   - Could be extended to support string DATA items
 *   - Could parse floating-point values if arithmetic is extended
 *
 * WHAT CANNOT BE CHANGED:
 *   - Must be called before the first READ executes
 *   - Must scan lines in ascending line-number order
 *   - Must reset data_count and data_ptr to zero
 *
 * WHAT TO EXPECT:
 *   DATA 10, -5, 30 produces three entries in the pool.
 *   Malformed DATA items (non-numeric) stop parsing for that line.
 *
 * IF SOMETHING BREAKS:
 *   - If READ returns wrong values, print data_pool[] contents
 *     after build_data_pool() to verify the scan
 *   - If negative DATA values fail, check the sign-handling code
 * ========================================================================= */

static void build_data_pool(void)
{
    int i;
    data_count = 0;
    data_ptr   = 0;

    for (i = 0; i < program_count; i++) {
        char *p = line_text[i];
        skip_spaces(&p);

        if (!peek_keyword(p, "DATA")) {
            continue;
        }
        p += 4;
        skip_spaces(&p);

        while (*p != '\0' && data_count < MAX_DATA) {
            short val;
            int   neg = 0;
            skip_spaces(&p);
            if (*p == '-') {
                neg = 1;
                p++;
                skip_spaces(&p);
            } else if (*p == '+') {
                p++;
                skip_spaces(&p);
            }
            if (!isdigit((unsigned char)*p)) {
                break;
            }
            val = parse_number(&p);
            if (neg) {
                val = (short)(-val);
            }
            data_pool[data_count++] = val;
            skip_spaces(&p);
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        }
    }
}

/* =========================================================================
 * EXPRESSION EVALUATOR
 *
 * The TRS-80 Level I BASIC evaluates expressions strictly left-to-right
 * with NO operator precedence.  2+3*4 yields 20, not 14.
 *
 * Grammar (left-to-right, flat):
 *   expr   := atom ( ('+' | '-' | '*' | '/') atom )*
 *   atom   := number | variable | '(' expr ')' | '-' atom | '+' atom
 *           | ABS '(' expr ')' | INT '(' expr ')'
 *           | RND '(' expr ')' | TAB '(' expr ')' | MEM
 *
 * WHAT CAN BE CHANGED:
 *   - New functions can be added to parse_atom() following the
 *     existing pattern (peek_keyword, parse arguments, return value)
 *   - To add operator precedence, replace parse_expr() with a
 *     recursive-descent parser using separate term/factor rules
 *     (but that would break TRS-80 Level I compatibility)
 *
 * WHAT CANNOT BE CHANGED:
 *   - Left-to-right evaluation order (no precedence) is fundamental
 *     to the Level I dialect; changing it alters program behavior
 *   - parse_atom() must handle unary minus/plus recursively
 *   - Division by zero must halt execution and print an error
 *
 * WHAT TO EXPECT:
 *   - 2+3*4 evaluates to 20 (not 14)
 *   - Parentheses override left-to-right order: 2+(3*4) = 14
 *   - ABS(-5) = 5, INT(7) = 7 (no-op), RND(6) = 1..6
 *   - MEM returns free memory as a 16-bit value (capped at 32767)
 *   - TAB(X) emits spaces to reach column X and returns 0
 *   - Unknown tokens in atom position return 0 silently
 *
 * IF SOMETHING BREAKS:
 *   - If expressions give wrong results, trace through parse_atom()
 *     and parse_expr() call by call with sample input
 *   - If division-by-zero does not halt, check that running is
 *     set to 0 in the '/' case of parse_expr()
 *   - If RND always returns the same sequence, verify srand() is
 *     called in main()
 * ========================================================================= */

static short parse_atom(char **p)
{
    short val;
    skip_spaces(p);

    /* Unary minus. */
    if (**p == '-') {
        (*p)++;
        val = parse_atom(p);
        return (short)(-val);
    }

    /* Unary plus (consumed, no effect). */
    if (**p == '+') {
        (*p)++;
        return parse_atom(p);
    }

    /* Parenthesised sub-expression. */
    if (**p == '(') {
        (*p)++;
        val = parse_expr(p);
        skip_spaces(p);
        if (**p == ')') (*p)++;
        return val;
    }

    /* Numeric literal. */
    if (isdigit((unsigned char)**p)) {
        return parse_number(p);
    }

    /* ABS(X): absolute value. */
    if (peek_keyword(*p, "ABS")) {
        *p += 3;
        skip_spaces(p);
        if (**p == '(') {
            (*p)++;
            val = parse_expr(p);
            skip_spaces(p);
            if (**p == ')') (*p)++;
        } else {
            val = parse_atom(p);
        }
        return (val < 0) ? (short)(-val) : val;
    }

    /* INT(X): integer truncation (no-op for integer arithmetic). */
    if (peek_keyword(*p, "INT")) {
        *p += 3;
        skip_spaces(p);
        if (**p == '(') {
            (*p)++;
            val = parse_expr(p);
            skip_spaces(p);
            if (**p == ')') (*p)++;
        } else {
            val = parse_atom(p);
        }
        return val;
    }

    /* RND(X): random integer from 1 to X inclusive. */
    if (peek_keyword(*p, "RND")) {
        short upper;
        *p += 3;
        skip_spaces(p);
        if (**p == '(') {
            (*p)++;
            upper = parse_expr(p);
            skip_spaces(p);
            if (**p == ')') (*p)++;
        } else {
            upper = parse_atom(p);
        }
        if (upper <= 0) upper = 1;
        return (short)(rand() % upper + 1);
    }

    /* TAB(X): emit spaces to reach column X, return 0. */
    if (peek_keyword(*p, "TAB")) {
        short col;
        *p += 3;
        skip_spaces(p);
        if (**p == '(') {
            (*p)++;
            col = parse_expr(p);
            skip_spaces(p);
            if (**p == ')') (*p)++;
        } else {
            col = parse_atom(p);
        }
        if (col < 0) col = 0;
        while (print_col < col) {
            putchar(' ');
            print_col++;
        }
        return 0;
    }

    /* MEM: free memory keyword (no parentheses). */
    if (peek_keyword(*p, "MEM")) {
        long used;
        *p += 3;
        used = (long)program_count * (long)(LINE_LEN + 2)
             + (long)data_count * 2L;
        val = (short)((TOTAL_MEM - used) > 32767 ? 32767 : (TOTAL_MEM - used));
        return val;
    }

    /* Single-letter variable A-Z. */
    if (isalpha((unsigned char)**p)) {
        int idx = (char)toupper((unsigned char)**p) - 'A';
        if (idx >= 0 && idx < NUM_VARS) {
            (*p)++;
            return vars[idx];
        }
    }

    return 0;
}

/* Left-to-right expression: atom op atom op atom ... */
static short parse_expr(char **p)
{
    short val = parse_atom(p);
    for (;;) {
        char op;
        short rhs;
        skip_spaces(p);
        op = **p;
        if (op != '+' && op != '-' && op != '*' && op != '/') {
            break;
        }
        (*p)++;
        rhs = parse_atom(p);
        switch (op) {
            case '+': val = (short)(val + rhs); break;
            case '-': val = (short)(val - rhs); break;
            case '*': val = (short)(val * rhs); break;
            case '/':
                if (rhs == 0) {
                    printf("DIVISION BY ZERO");
                    if (running) {
                        printf(" IN LINE %d", (int)line_nums[pc - 1]);
                    }
                    printf("\n");
                    running = 0;
                    error_flag = 1;
                    return 0;
                }
                val = (short)(val / rhs);
                break;
        }
    }
    return val;
}

/* =========================================================================
 * RELATIONAL (CONDITION) EVALUATOR
 *
 * Wraps parse_expr() and checks for an optional trailing relational
 * operator.  If one is found, evaluates both sides and returns 1
 * (true) or 0 (false).  If no relational operator follows, returns
 * the raw expression value.
 *
 * WHAT CAN BE CHANGED:
 *   - New relational operators can be added (though the standard
 *     set covers all common cases)
 *
 * WHAT CANNOT BE CHANGED:
 *   - The # operator must remain an alias for <> (TRS-80 convention)
 *   - Relational operators must bind looser than arithmetic
 *     (handled by calling parse_expr() for each side)
 *
 * WHAT TO EXPECT:
 *   - IF A=5 THEN ... evaluates the = as equality, not assignment
 *   - Relational operators return exactly 1 or 0
 *
 * IF SOMETHING BREAKS:
 *   - If <= is parsed as < followed by =, check the two-character
 *     lookahead logic
 *   - If # does not work, verify it is handled after the > branch
 * ========================================================================= */

static short parse_relational(char **p)
{
    short lhs = parse_expr(p);
    short rhs;
    skip_spaces(p);

    if (**p == '<') {
        (*p)++;
        if (**p == '>') {
            (*p)++;
            rhs = parse_expr(p);
            return (short)(lhs != rhs ? 1 : 0);
        }
        if (**p == '=') {
            (*p)++;
            rhs = parse_expr(p);
            return (short)(lhs <= rhs ? 1 : 0);
        }
        rhs = parse_expr(p);
        return (short)(lhs < rhs ? 1 : 0);
    }

    if (**p == '>') {
        (*p)++;
        if (**p == '=') {
            (*p)++;
            rhs = parse_expr(p);
            return (short)(lhs >= rhs ? 1 : 0);
        }
        rhs = parse_expr(p);
        return (short)(lhs > rhs ? 1 : 0);
    }

    if (**p == '=') {
        (*p)++;
        rhs = parse_expr(p);
        return (short)(lhs == rhs ? 1 : 0);
    }

    if (**p == '#') {
        (*p)++;
        rhs = parse_expr(p);
        return (short)(lhs != rhs ? 1 : 0);
    }

    return lhs;
}

/* =========================================================================
 * STATEMENT EXECUTION HANDLERS
 *
 * Each exec_*() function implements one BASIC statement.  They
 * receive a pointer-to-pointer into the source line, positioned
 * just after the keyword that triggered the dispatch.  They consume
 * their arguments and leave the pointer at the end of the statement.
 *
 * WHAT CAN BE CHANGED:
 *   - New statements can be added by writing a new exec_*() handler
 *     and adding a match_keyword() check in exec_line()
 *   - PRINT formatting (tab width, newline rules) can be adjusted
 *
 * WHAT CANNOT BE CHANGED:
 *   - LET must remain mandatory (bare A=5 must produce SYNTAX ERROR)
 *   - INPUT must prompt with "? " (TRS-80 convention)
 *   - GOTO must halt with UNDEFINED LINE if the target does not exist
 *   - IF/THEN must support both "THEN linenum" (implicit GOTO) and
 *     "THEN statement" (inline execution)
 *   - STOP must print BREAK IN LINE and set state for CONT
 *
 * WHAT TO EXPECT:
 *   - PRINT separators: semicolon keeps cursor at current column,
 *     comma advances to next TAB_WIDTH tab stop
 *   - Trailing semicolon suppresses the newline after PRINT
 *   - ON expr GOTO with out-of-range index silently falls through
 *   - READ past end of data pool produces OUT OF DATA error
 *
 * IF SOMETHING BREAKS:
 *   - If PRINT output is misaligned, check print_col tracking
 *   - If IF/THEN fails to branch, trace parse_relational() output
 *   - If ON GOTO jumps to the wrong line, verify the 1-based index
 *     counting in the comma-separated target list
 * ========================================================================= */

/* ---- PRINT statement ---- */
static void exec_print(char **p)
{
    int need_newline = 1;

    skip_spaces(p);

    /* Bare PRINT: emit a blank line. */
    if (**p == '\0' || **p == ':') {
        putchar('\n');
        print_col = 0;
        return;
    }

    while (**p != '\0' && **p != ':') {
        skip_spaces(p);

        if (**p == '\0' || **p == ':') {
            break;
        }

        /* String literal. */
        if (**p == '"') {
            (*p)++;
            while (**p != '\0' && **p != '"') {
                putchar(**p);
                print_col++;
                (*p)++;
            }
            if (**p == '"') (*p)++;
            need_newline = 1;
        }
        /* TAB function inside PRINT. */
        else if (peek_keyword(*p, "TAB")) {
            parse_atom(p);
            need_newline = 1;
        }
        /* Numeric expression. */
        else {
            short val;
            char *before = *p;
            error_flag = 0;
            val = parse_expr(p);
            if (error_flag) {
                return;
            }
            /* If parse_expr consumed nothing, the remaining text is
               not a valid expression.  Break to avoid an infinite loop. */
            if (*p == before) {
                break;
            }
            if (val >= 0) printf(" ");
            printf("%d ", (int)val);
            {
                int n = val;
                int w = 2;
                if (n < 0) { w++; n = -n; } else { w++; }
                while (n >= 10) { w++; n /= 10; }
                print_col += w;
            }
            need_newline = 1;
        }

        skip_spaces(p);

        /* Semicolon: suppress newline, stay at current column. */
        if (**p == ';') {
            (*p)++;
            need_newline = 0;
        }
        /* Comma: advance to next tab stop. */
        else if (**p == ',') {
            int next_tab;
            (*p)++;
            next_tab = ((print_col / TAB_WIDTH) + 1) * TAB_WIDTH;
            while (print_col < next_tab) {
                putchar(' ');
                print_col++;
            }
            need_newline = 0;
        }
    }

    if (need_newline) {
        putchar('\n');
        print_col = 0;
    }
}

/* ---- LET statement (mandatory keyword) ---- */
static void exec_let(char **p)
{
    int idx;
    skip_spaces(p);
    if (!isalpha((unsigned char)**p)) {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }
    idx = (char)toupper((unsigned char)**p) - 'A';
    (*p)++;
    skip_spaces(p);
    if (**p != '=') {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }
    (*p)++;
    vars[idx] = parse_expr(p);
}

/* ---- INPUT statement ---- */
static void exec_input(char **p)
{
    int idx;
    char buf[64];
    skip_spaces(p);
    if (!isalpha((unsigned char)**p)) {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }
    idx = (char)toupper((unsigned char)**p) - 'A';
    (*p)++;
    printf("? ");
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        running = 0;
        return;
    }
    vars[idx] = (short)atoi(buf);
}

/* ---- GOTO statement ---- */
static void exec_goto(char **p)
{
    short target;
    int idx;
    skip_spaces(p);
    if (!isdigit((unsigned char)**p)) {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }
    target = parse_number(p);
    idx = find_line(target);
    if (idx < 0) {
        printf("UNDEFINED LINE %d", (int)target);
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }
    pc = idx;
}

/* ---- IF/THEN statement ---- */
static void exec_if(char **p)
{
    short cond;
    skip_spaces(p);
    cond = parse_relational(p);
    skip_spaces(p);

    if (!match_keyword(p, "THEN")) {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }

    if (cond == 0) {
        while (**p != '\0') (*p)++;
        return;
    }

    skip_spaces(p);
    if (isdigit((unsigned char)**p)) {
        exec_goto(p);
    } else {
        exec_line(*p);
        while (**p != '\0') (*p)++;
    }
}

/* ---- ON expr GOTO line1, line2, ... ---- */
static void exec_on_goto(char **p)
{
    short selector;
    int   count;
    short target;

    skip_spaces(p);
    selector = parse_expr(p);
    skip_spaces(p);

    if (!match_keyword(p, "GOTO")) {
        printf("SYNTAX ERROR");
        if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
        printf("\n");
        running = 0;
        return;
    }

    count = 0;
    target = 0;
    for (;;) {
        int idx;
        skip_spaces(p);
        if (!isdigit((unsigned char)**p)) break;
        count++;
        target = parse_number(p);
        if (count == selector) {
            idx = find_line(target);
            if (idx < 0) {
                printf("UNDEFINED LINE %d", (int)target);
                if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
                printf("\n");
                running = 0;
            } else {
                pc = idx;
            }
            return;
        }
        skip_spaces(p);
        if (**p == ',') {
            (*p)++;
        } else {
            break;
        }
    }
    (void)target;
}

/* ---- READ statement ---- */
static void exec_read(char **p)
{
    for (;;) {
        int idx;
        skip_spaces(p);
        if (!isalpha((unsigned char)**p)) {
            printf("SYNTAX ERROR");
            if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
            printf("\n");
            running = 0;
            return;
        }
        idx = (char)toupper((unsigned char)**p) - 'A';
        (*p)++;
        if (data_ptr >= data_count) {
            printf("OUT OF DATA");
            if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
            printf("\n");
            running = 0;
            return;
        }
        vars[idx] = data_pool[data_ptr++];
        skip_spaces(p);
        if (**p == ',') {
            (*p)++;
        } else {
            break;
        }
    }
}

/* ---- STOP statement ---- */
static void exec_stop(void)
{
    printf("BREAK");
    if (running) {
        printf(" IN LINE %d", (int)line_nums[pc - 1]);
    }
    printf("\n");
    stopped = 1;
    cont_pc = pc;
    running = 0;
}

/* =========================================================================
 * COMMAND HANDLERS (direct mode)
 *
 * Commands are entered without a line number and execute immediately.
 * They manage the program lifecycle: running, listing, clearing,
 * saving, loading, and continuing after a STOP.
 *
 * WHAT CAN BE CHANGED:
 *   - LIST could be extended to support LIST -n (from start to n)
 *   - RUN could accept a starting line number (but the spec says
 *     RUN with arguments produces SYNTAX ERROR)
 *   - SAVE/LOAD filename handling can be adjusted
 *
 * WHAT CANNOT BE CHANGED:
 *   - RUN must clear variables and rebuild the data pool
 *   - NEW must clear everything (program, variables, data, CONT state)
 *   - CONT must resume at the saved position without resetting state
 *   - CONT with no prior STOP must print CAN'T CONTINUE
 *   - BYE must print GOODBYE and exit
 *
 * WHAT TO EXPECT:
 *   - RUN with any argument produces SYNTAX ERROR
 *   - LIST with no arguments lists the entire program
 *   - LIST n lists only line n; LIST n-m lists lines n through m
 *   - SAVE/LOAD auto-append .BAS if the filename lacks it
 *   - LOAD clears the existing program before loading
 *
 * IF SOMETHING BREAKS:
 *   - If RUN skips lines, check that pc starts at 0 and the while
 *     loop increments correctly
 *   - If CONT jumps to the wrong line, check that cont_pc is set
 *     to pc (which is already pre-incremented in the run loop).
 *   - If SAVE produces garbled output, check fprintf format strings
 *   - If LOAD skips lines, verify the line-number parsing in the
 *     load loop
 * ========================================================================= */

/* ---- HELP command ---- */
static void cmd_help(void)
{
    printf("\nTRS-80 Level I BASIC v2.1\n");
    printf("===========================\n");
    printf("Statements:  PRINT  LET  INPUT  GOTO  IF/THEN  REM\n");
    printf("             END  STOP  CLEAR  BEEP\n");
    printf("             DATA  READ  RESTORE  ON...GOTO\n");
    printf("Commands:    RUN  LIST  NEW  SAVE  LOAD  CONT  HELP  BYE\n");
    printf("Functions:   ABS()  INT()  RND()  MEM  TAB()\n");
    printf("Operators:   +  -  *  /  =  <  >  <=  >=  <>  #\n\n");
}

/* ---- RUN command ---- */
static void cmd_run(char *p)
{
    int i;

    skip_spaces(&p);
    if (*p != '\0') {
        printf("SYNTAX ERROR\n");
        return;
    }

    if (program_count == 0) return;

    for (i = 0; i < NUM_VARS; i++) vars[i] = 0;

    build_data_pool();

    stopped = 0;
    cont_pc = 0;

    running = 1;
    pc = 0;
    print_col = 0;

    while (running && pc < program_count) {
        char *lp = line_text[pc];
        pc++;
        exec_line(lp);
    }

    if (running) {
        running = 0;
    }
}

/* ---- LIST command ---- */
static void cmd_list(char *p)
{
    int i;
    short start_ln = 0;
    short end_ln   = 32767;

    skip_spaces(&p);

    if (isdigit((unsigned char)*p)) {
        start_ln = parse_number(&p);
        end_ln = start_ln;
        skip_spaces(&p);
        if (*p == '-') {
            p++;
            skip_spaces(&p);
            if (isdigit((unsigned char)*p)) {
                end_ln = parse_number(&p);
            } else {
                end_ln = 32767;
            }
        }
    }

    for (i = 0; i < program_count; i++) {
        if (line_nums[i] >= start_ln && line_nums[i] <= end_ln) {
            printf("%d %s\n", (int)line_nums[i], line_text[i]);
        }
    }
}

/* ---- NEW command ---- */
static void cmd_new(void)
{
    int i;
    program_count = 0;
    for (i = 0; i < NUM_VARS; i++) vars[i] = 0;
    data_count = 0;
    data_ptr   = 0;
    stopped    = 0;
    cont_pc    = 0;
}

/* ---- CONT command ---- */
static void cmd_cont(void)
{
    if (!stopped) {
        printf("CAN'T CONTINUE\n");
        return;
    }

    stopped = 0;
    running = 1;
    pc = cont_pc;

    while (running && pc < program_count) {
        char *lp = line_text[pc];
        pc++;
        exec_line(lp);
    }

    if (running) {
        running = 0;
    }
}

/* =========================================================================
 * FILE I/O: SAVE AND LOAD
 *
 * Programs are saved and loaded as plain-text files.  Each line in
 * the file is formatted as "linenum source_text\n".  Filenames may
 * be given with or without quotes.  If the filename does not already
 * end in .BAS (case-insensitive), ".BAS" is appended automatically.
 *
 * WHAT CAN BE CHANGED:
 *   - The auto-extension could be changed to .bas (lowercase)
 *   - The filename buffer size (256) can be increased
 *   - A tokenized binary format could be added alongside text
 *
 * WHAT CANNOT BE CHANGED:
 *   - SAVE with no filename must produce SYNTAX ERROR
 *   - LOAD must clear the existing program before loading
 *   - LOAD with a missing file must print FILE NOT FOUND
 *
 * WHAT TO EXPECT:
 *   - SAVE "test" and SAVE test both save to test.BAS
 *   - SAVE "test.BAS" does not double-append the extension
 *   - LOAD replaces the current program entirely
 *
 * IF SOMETHING BREAKS:
 *   - If filenames are garbled, check build_filename() quote handling
 *   - If LOAD does not find the file, verify the auto-.BAS logic
 *   - If loaded programs are corrupt, check the line-parsing loop
 *     in cmd_load() (stripping newlines, parsing line numbers)
 * ========================================================================= */

static void build_filename(char *dst, int dst_size, char **p)
{
    int  len = 0;
    int  has_ext = 0;
    char *dot;

    skip_spaces(p);

    if (**p == '"') {
        (*p)++;
        while (**p != '\0' && **p != '"' && len < dst_size - 5) {
            dst[len++] = **p;
            (*p)++;
        }
        if (**p == '"') (*p)++;
    } else {
        while (**p != '\0' && **p != ' ' && **p != '\t' && len < dst_size - 5) {
            dst[len++] = **p;
            (*p)++;
        }
    }
    dst[len] = '\0';

    dot = strrchr(dst, '.');
    if (dot != NULL) {
        if (((char)toupper((unsigned char)dot[1]) == 'B') &&
            ((char)toupper((unsigned char)dot[2]) == 'A') &&
            ((char)toupper((unsigned char)dot[3]) == 'S') &&
            dot[4] == '\0') {
            has_ext = 1;
        }
    }

    if (!has_ext) {
        strcat(dst, ".BAS");
    }
}

static void cmd_save(char *p)
{
    char fname[256];
    FILE *fp;
    int   i;

    skip_spaces(&p);
    if (*p == '\0') {
        printf("SYNTAX ERROR\n");
        return;
    }
    build_filename(fname, sizeof(fname), &p);

    fp = fopen(fname, "w");
    if (fp == NULL) {
        printf("FILE ERROR\n");
        return;
    }
    for (i = 0; i < program_count; i++) {
        fprintf(fp, "%d %s\n", (int)line_nums[i], line_text[i]);
    }
    fclose(fp);
    printf("OK\n");
}

static void cmd_load(char *p)
{
    char fname[256];
    FILE *fp;
    char buf[LINE_LEN + 20];

    skip_spaces(&p);
    if (*p == '\0') {
        printf("SYNTAX ERROR\n");
        return;
    }
    build_filename(fname, sizeof(fname), &p);

    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("FILE NOT FOUND\n");
        return;
    }

    cmd_new();

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *bp = buf;
        short lnum;
        {
            size_t slen = strlen(buf);
            if (slen > 0 && buf[slen - 1] == '\n') buf[slen - 1] = '\0';
            slen = strlen(buf);
            if (slen > 0 && buf[slen - 1] == '\r') buf[slen - 1] = '\0';
        }
        skip_spaces(&bp);
        if (!isdigit((unsigned char)*bp)) continue;
        lnum = parse_number(&bp);
        skip_spaces(&bp);
        store_line(lnum, bp);
    }
    fclose(fp);
    printf("OK\n");
}

/* =========================================================================
 * MAIN LINE DISPATCHER
 *
 * Parses the keyword at the start of a line and dispatches to the
 * appropriate statement handler or command handler.  This is the
 * central routing function called for both direct-mode input and
 * stored-program lines during RUN.
 *
 * WHAT CAN BE CHANGED:
 *   - New keywords can be added by inserting a match_keyword() check
 *     before the fallback error at the bottom.  Order matters: longer
 *     keywords that share prefixes with shorter ones must be tested
 *     first (e.g. "RESTORE" before "REM" is not an issue because
 *     match_keyword enforces word boundaries, but be cautious).
 *
 * WHAT CANNOT BE CHANGED:
 *   - LET must be mandatory.  The bare-variable-assignment detector
 *     near the bottom of this function must remain to enforce this.
 *   - The ? shorthand for PRINT must be supported.
 *   - Unknown keywords must produce SYNTAX ERROR.
 *
 * WHAT TO EXPECT:
 *   - Commands like RUN, LIST, NEW, etc. work in both direct mode
 *     and inside a program (though using them inside a program is
 *     unusual and may have odd side effects).
 *   - DATA lines are skipped during execution (they are declarative).
 *   - REM lines are ignored entirely.
 *
 * IF SOMETHING BREAKS:
 *   - If a valid keyword is not recognized, check that match_keyword()
 *     is testing for the exact uppercase spelling.
 *   - If the wrong handler fires, check keyword ordering for prefix
 *     collisions.
 *   - If SYNTAX ERROR appears for valid input, check that the keyword
 *     is followed by a word boundary in the input.
 * ========================================================================= */

static void exec_line(char *line)
{
    char *p = line;
    skip_spaces(&p);

    if (*p == '\0') return;

    /* ---- Statements ---- */

    if (match_keyword(&p, "PRINT")) {
        exec_print(&p);
        return;
    }
    if (*p == '?') {
        p++;
        exec_print(&p);
        return;
    }
    if (match_keyword(&p, "LET")) {
        exec_let(&p);
        return;
    }
    if (match_keyword(&p, "INPUT")) {
        exec_input(&p);
        return;
    }
    if (match_keyword(&p, "GOTO")) {
        exec_goto(&p);
        return;
    }
    if (match_keyword(&p, "IF")) {
        exec_if(&p);
        return;
    }
    if (match_keyword(&p, "ON")) {
        exec_on_goto(&p);
        return;
    }
    if (match_keyword(&p, "READ")) {
        exec_read(&p);
        return;
    }
    if (match_keyword(&p, "RESTORE")) {
        data_ptr = 0;
        return;
    }
    if (match_keyword(&p, "DATA")) {
        return;
    }
    if (match_keyword(&p, "REM")) {
        return;
    }
    if (match_keyword(&p, "END")) {
        running = 0;
        return;
    }
    if (match_keyword(&p, "STOP")) {
        exec_stop();
        return;
    }
    if (match_keyword(&p, "CLEAR")) {
        int i;
        for (i = 0; i < NUM_VARS; i++) vars[i] = 0;
        return;
    }
    if (match_keyword(&p, "BEEP")) {
        trigger_beep();
        return;
    }

    /* ---- Commands (direct mode) ---- */

    if (match_keyword(&p, "RUN")) {
        cmd_run(p);
        return;
    }
    if (match_keyword(&p, "LIST")) {
        cmd_list(p);
        return;
    }
    if (match_keyword(&p, "NEW")) {
        cmd_new();
        return;
    }
    if (match_keyword(&p, "SAVE")) {
        cmd_save(p);
        return;
    }
    if (match_keyword(&p, "LOAD")) {
        cmd_load(p);
        return;
    }
    if (match_keyword(&p, "CONT")) {
        cmd_cont();
        return;
    }
    if (match_keyword(&p, "HELP")) {
        cmd_help();
        return;
    }
    if (match_keyword(&p, "BYE")) {
        printf("GOODBYE\n");
        exit(0);
    }

    /* ---- Bare variable assignment detection ----
     * Level I BASIC requires the LET keyword.  If we see a letter
     * followed by '=', the user tried to assign without LET.
     */
    if (isalpha((unsigned char)*p)) {
        char *look = p + 1;
        skip_spaces(&look);
        if (*look == '=') {
            printf("SYNTAX ERROR");
            if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
            printf("\n");
            running = 0;
            return;
        }
    }

    /* ---- Unknown statement ---- */
    printf("SYNTAX ERROR");
    if (running) printf(" IN LINE %d", (int)line_nums[pc - 1]);
    printf("\n");
    running = 0;
}

/* =========================================================================
 * MAIN: READ-EVALUATE-PRINT LOOP (REPL)
 *
 * The main loop reads a line of input, determines whether it is a
 * numbered program line (to be stored) or a direct-mode command
 * (to be executed immediately), and dispatches accordingly.
 *
 * WHAT CAN BE CHANGED:
 *   - The prompt string "> " can be changed to "READY\n> " or
 *     any other format
 *   - The banner text and version number
 *   - Ctrl+C handling could be added with signal(SIGINT, ...)
 *
 * WHAT CANNOT BE CHANGED:
 *   - srand() must be called before any RND() usage
 *   - Lines with a leading number must be stored, not executed
 *   - Lines without a leading number must be executed immediately
 *   - EOF on stdin must exit cleanly
 *
 * WHAT TO EXPECT:
 *   - The interpreter prints the banner, free memory, and READY
 *     at startup
 *   - Empty lines are silently ignored
 *   - Line number 0 or negative is rejected
 *   - The loop runs until BYE is entered or EOF is received
 *
 * IF SOMETHING BREAKS:
 *   - If the prompt does not appear, check that fflush(stdout) is
 *     called after printing the prompt
 *   - If input is garbled, check the newline/carriage-return
 *     stripping code
 *   - If stored lines have wrong content, check that parse_number()
 *     correctly advances the pointer past the line number
 * ========================================================================= */

int main(void)
{
    srand((unsigned int)time(NULL));

    printf("TRS-80 Level I BASIC v2.1\n");
    printf("%d BYTES FREE\n", (int)TOTAL_MEM);
    printf("READY\n");

    for (;;) {
        char *p;
        short lnum;

        printf("> ");
        fflush(stdout);

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            printf("\n");
            break;
        }

        /* Strip trailing newline and carriage return. */
        {
            size_t slen = strlen(input_buf);
            if (slen > 0 && input_buf[slen - 1] == '\n') input_buf[slen - 1] = '\0';
            slen = strlen(input_buf);
            if (slen > 0 && input_buf[slen - 1] == '\r') input_buf[slen - 1] = '\0';
        }

        /* Convert entire line to uppercase (preserve quoted strings) */
        uppercase_line(input_buf);

        p = input_buf;
        skip_spaces(&p);

        if (*p == '\0') continue;

        /* Numbered line: store it in the program. */
        if (isdigit((unsigned char)*p)) {
            lnum = parse_number(&p);
            if (lnum <= 0) {
                printf("ILLEGAL LINE NUMBER\n");
                continue;
            }
            store_line(lnum, p);
            continue;
        }

        /* Direct-mode command or statement. */
        exec_line(p);
    }

    return 0;
}

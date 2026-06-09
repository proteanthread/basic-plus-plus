/*
 * =====================================================================
 * BASIC++ Interpreter - builtins.c
 * =====================================================================
 *
 * Built-in function handler implementations and registration.
 *
 * PURPOSE:
 *   Contains all built-in BASIC++ function implementations, extracted
 *   from parser.c into uniform FuncHandler-signature functions. Each
 *   handler receives pre-evaluated arguments as BValue arrays and
 *   returns a BValue result.
 *
 * WHY THIS EXISTS:
 *   Previously, function evaluation was interleaved with parsing
 *   logic in parser.c - argument parsing, evaluation, and function
 *   dispatch were all in one giant switch/if chain. This made it
 *   impossible to override, extend, or introspect functions.
 *
 *   Now:
 *     - The parser parses arguments into a BValue array
 *     - The registry looks up the handler
 *     - The handler receives clean, pre-evaluated arguments
 *     - Function logic is completely separate from parsing logic
 *
 * HANDLER PATTERN:
 *   Every handler follows the same pattern:
 *     1. Access args[0], args[1], etc. (argc already validated)
 *     2. Perform the operation using bval_xxx() helpers from value.h
 *     3. Return the BValue result
 *     4. On error, call error_raise() and return bval_int(0)
 *
 *   The 'rt' parameter is cast to (RuntimeState*) when runtime
 *   services are needed (RND seed, string pool allocation).
 *
 * HOW TO WRITE EXTERNAL FUNCTIONS:
 *   Follow the same pattern as any handler here. Example:
 *
 *     BValue my_square(BValue *args, int argc, void *rt)
 *     {
 *         double x;
 *         (void)argc; (void)rt;
 *         x = bval_to_float(&args[0]);
 *         return bval_float(x * x);
 *     }
 *
 *   Then register with funcreg_register(). See funcreg.h for
 *   the full registration example.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* PI constant for degree/radian conversion */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)
#include "builtins.h"
#include "funcreg.h"
#include "runtime.h"
#include "errors.h"
#include "fileio.h"
#include "stringpool.h"
#include "dialect.h"
#include "gfxbuf.h"

/* =====================================================================
 * Math Function Handlers
 * =====================================================================
 * All math handlers delegate to the bval_xxx() functions in value.c
 * which handle type promotion (int->float) and error checking.
 */

/*
 * ABS(x) - Absolute value.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 * Works on integers and floats.
 */
BValue builtin_abs(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_abs(&args[0], 0);
}

/*
 * SGN(x) - Sign function.
 * Returns -1, 0, or 1.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_sgn(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_sgn(&args[0], 0);
}

/*
 * INT(x) - Truncate to integer.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_int_func(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_int_func(&args[0], 0);
}

/*
 * SQR(x) - Square root.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_sqr(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_sqr(&args[0], 0);
}

/*
 * SIN(x) - Sine.
 * If OPTION ANGLE DEGREES is active, x is in degrees.
 * Otherwise x is in radians (default).
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_sin(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    if (state->angle_degrees) {
        double x = bval_to_float(&args[0]) * DEG_TO_RAD;
        return bval_float(sin(x));
    }
    return bval_sin(&args[0], 0);
}

/*
 * COS(x) - Cosine.
 * Degree/radian mode controlled by OPTION ANGLE.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_cos(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    if (state->angle_degrees) {
        double x = bval_to_float(&args[0]) * DEG_TO_RAD;
        return bval_float(cos(x));
    }
    return bval_cos(&args[0], 0);
}

/*
 * TAN(x) - Tangent.
 * Degree/radian mode controlled by OPTION ANGLE.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_tan(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    if (state->angle_degrees) {
        double x = bval_to_float(&args[0]) * DEG_TO_RAD;
        return bval_float(tan(x));
    }
    return bval_tan(&args[0], 0);
}

/*
 * ATN(x) - Arctangent.
 * If OPTION ANGLE DEGREES, returns result in degrees.
 * Otherwise returns radians (default).
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_atn(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    if (state->angle_degrees) {
        double x = bval_to_float(&args[0]);
        return bval_float(atan(x) * RAD_TO_DEG);
    }
    return bval_atn(&args[0], 0);
}

/*
 * LOG(x) - Natural logarithm.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_log(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_log(&args[0], 0);
}

/*
 * EXP(x) - e raised to x.
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_exp(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_exp(&args[0], 0);
}

/* =====================================================================
 * String Function Handlers
 * =====================================================================
 */

/*
 * LEN(s$) - String length.
 * Category: FCAT_STRING | Safety: FSAFE_PURE
 */
BValue builtin_len(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_len(&args[0], 0);
}

/*
 * ASC(s$) - ASCII value of first character.
 * Category: FCAT_STRING | Safety: FSAFE_PURE
 */
BValue builtin_asc(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_asc(&args[0], 0);
}

/*
 * VAL(s$) - Convert string to number.
 * Category: FCAT_STRING | Safety: FSAFE_PURE
 */
BValue builtin_val(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_val(&args[0], 0);
}

/*
 * CHR$(n) - Convert number to single-character string.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
 */
BValue builtin_chr(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_chr(&args[0], 0, &state->strpool);
}

/*
 * STR$(n) - Convert number to string representation.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
 */
BValue builtin_str(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_str(&args[0], 0, &state->strpool);
}

/*
 * LEFT$(s$, n) - Left substring.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
 */
BValue builtin_left(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_left(&args[0], &args[1], 0, &state->strpool);
}

/*
 * RIGHT$(s$, n) - Right substring.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
 */
BValue builtin_right(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_right(&args[0], &args[1], 0, &state->strpool);
}

/*
 * MID$(s$, start, len) - Middle substring.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
 */
BValue builtin_mid(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    BValue len_val;

    /*
     * MID$(s$, start) - 2-arg form: return from start to end.
     * MID$(s$, start, len) - 3-arg form: return len chars.
     */
    if (argc < 3) {
        /* No length specified: use rest of string */
        len_val = bval_int((long)MAX_STRING_LENGTH);
        return bval_mid(&args[0], &args[1], &len_val, 0,
                        &state->strpool);
    }
    return bval_mid(&args[0], &args[1], &args[2], 0,
                    &state->strpool);
}

/* =====================================================================
 * Utility Function Handlers
 * =====================================================================
 */

/*
 * RND(n) - Random number generation.
 *
 * Dialect-aware behavior:
 *
 *   PATB (Palo Alto Tiny BASIC):
 *     RND(n) returns a random integer in [1, n].
 *
 *   GWBS (GW-BASIC / Dartmouth):
 *     RND(n) where n>0: return float in [0, 1)
 *     RND(0): repeat last random value
 *     RND(n) where n<0: seed with |n|, return float
 *     This is the standard used by all vintage BASIC
 *     games (BASIC Computer Games, etc.)
 *
 * Category: FCAT_UTIL | Safety: FSAFE_STATE (modifies RNG seed)
 */
BValue builtin_rnd(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    long n;
    (void)argc;
    n = bval_to_int(&args[0]);

    if (dialect_get_config()->has_float) {
        /*
         * GW-BASIC mode: return float 0..1
         *
         * RND(n>0) = advance seed, return 0..1
         * RND(0)   = return last value (don't advance)
         * RND(n<0) = seed with |n|, advance, return 0..1
         */
        if (n < 0) {
            state->rnd_seed = (unsigned long)(-n);
        }
        if (n != 0) {
            state->rnd_seed =
                (state->rnd_seed * 1103515245UL +
                 12345UL) & 0x7FFFFFFFUL;
            /*
             * Avoid seed=0 which produces RND=0.
             * ECMA-55: 0 < RND < 1 (exclusive).
             */
            if (state->rnd_seed == 0)
                state->rnd_seed = 1;
        }
        return bval_float(
            (double)state->rnd_seed / 2147483648.0);
    }

    /* PATB mode: return integer 1..n */
    return bval_int(runtime_rnd(state, n));
}

/*
 * SIZE - Available memory.
 *
 * Returns the number of available bytes in the program memory pool.
 * Takes no arguments (argc=0).
 *
 * Category: FCAT_UTIL | Safety: FSAFE_PURE
 */
BValue builtin_size(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)args; (void)argc;
    return bval_int(runtime_size(state));
}

/*
 * EOF(n) - End-of-file check.
 *
 * Returns -1 if file channel n is at EOF, 0 otherwise.
 * This matches GW-BASIC convention where EOF returns -1 (true)
 * or 0 (false), compatible with IF EOF(1) THEN ...
 *
 * Category: FCAT_IO | Safety: FSAFE_STATE
 */
BValue builtin_eof(BValue *args, int argc, void *rt)
{
    int chan;
    int result;
    (void)argc; (void)rt;
    chan = (int)bval_to_int(&args[0]);
    result = fileio_eof(chan, 0);
    if (result < 0) return bval_int(0);  /* error */
    return bval_int(result ? -1 : 0);
}

/*
 * INSTR(haystack$, needle$) - Find substring.
 * Returns 1-based position of needle in haystack, or 0 if not found.
 * Category: FCAT_STRING | Safety: FSAFE_PURE
 */
BValue builtin_instr(BValue *args, int argc, void *rt)
{
    const char *h, *n;
    int hl, nl, i;
    (void)rt;

    if (argc < 2 || !bval_is_string(&args[0]) ||
        !bval_is_string(&args[1])) {
        return bval_int(0);
    }
    h = args[0].v.sval.data;
    hl = args[0].v.sval.length;
    n = args[1].v.sval.data;
    nl = args[1].v.sval.length;

    if (h == NULL || n == NULL || nl > hl || nl == 0)
        return bval_int(0);

    for (i = 0; i <= hl - nl; i++) {
        if (memcmp(h + i, n, (size_t)nl) == 0)
            return bval_int(i + 1);  /* 1-based */
    }
    return bval_int(0);
}

/*
 * SPACE$(n) - Return a string of N spaces.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
 */
BValue builtin_space(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    int n, i;
    char *buf;
    (void)argc;

    n = (int)bval_to_int(&args[0]);
    if (n < 0) n = 0;
    if (n > 255) n = 255;

    buf = strpool_alloc(&state->strpool, n);
    if (buf == NULL) return bval_string(NULL, 0);
    for (i = 0; i < n; i++) buf[i] = ' ';
    return bval_string(buf, n);
}

/*
 * STRING$(n, char) - Return N copies of a character.
 * char can be a string (first char used) or an ASCII code.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
 */
BValue builtin_string_func(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    int n, i;
    char ch;
    char *buf;
    (void)argc;

    n = (int)bval_to_int(&args[0]);
    if (n < 0) n = 0;
    if (n > 255) n = 255;

    if (bval_is_string(&args[1])) {
        if (args[1].v.sval.length > 0 &&
            args[1].v.sval.data != NULL)
            ch = args[1].v.sval.data[0];
        else
            ch = ' ';
    } else {
        ch = (char)bval_to_int(&args[1]);
    }

    buf = strpool_alloc(&state->strpool, n);
    if (buf == NULL) return bval_string(NULL, 0);
    for (i = 0; i < n; i++) buf[i] = ch;
    return bval_string(buf, n);
}

/*
 * HEX$(n) - Convert number to hexadecimal string.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
 */
BValue builtin_hex(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    long val;
    char tmp[20];
    int len, i;
    char *buf;
    unsigned long uv;
    (void)argc;

    val = bval_to_int(&args[0]);
    uv = (unsigned long)val;

    /* Manual hex conversion (portable, no sprintf %X) */
    i = (int)(sizeof(tmp) - 1);
    tmp[i] = '\0';
    if (uv == 0) {
        tmp[--i] = '0';
    } else {
        while (uv > 0 && i > 0) {
            int d = (int)(uv & 0xF);
            tmp[--i] = (char)(d < 10 ?
                '0' + d : 'A' + d - 10);
            uv >>= 4;
        }
    }
    len = (int)(sizeof(tmp) - 1 - (size_t)i);
    buf = strpool_alloc(&state->strpool, len);
    if (buf == NULL) return bval_string(NULL, 0);
    memcpy(buf, tmp + i, (size_t)len);
    return bval_string(buf, len);
}

/*
 * OCT$(n) - Convert number to octal string.
 * Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
 */
BValue builtin_oct(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    long val;
    char tmp[24];
    int len, i;
    char *buf;
    unsigned long uv;
    (void)argc;

    val = bval_to_int(&args[0]);
    uv = (unsigned long)val;

    i = (int)(sizeof(tmp) - 1);
    tmp[i] = '\0';
    if (uv == 0) {
        tmp[--i] = '0';
    } else {
        while (uv > 0 && i > 0) {
            tmp[--i] = (char)('0' + (int)(uv & 7));
            uv >>= 3;
        }
    }
    len = (int)(sizeof(tmp) - 1 - (size_t)i);
    buf = strpool_alloc(&state->strpool, len);
    if (buf == NULL) return bval_string(NULL, 0);
    memcpy(buf, tmp + i, (size_t)len);
    return bval_string(buf, len);
}

/*
 * FIX(x) - Truncate toward zero.
 * Unlike INT (floor), FIX truncates:
 *   FIX(3.7) = 3, FIX(-3.7) = -3
 *   INT(3.7) = 3, INT(-3.7) = -4
 * Category: FCAT_MATH | Safety: FSAFE_PURE
 */
BValue builtin_fix(BValue *args, int argc, void *rt)
{
    double x;
    (void)argc; (void)rt;
    x = bval_to_float(&args[0]);
    if (x >= 0.0)
        return bval_int((long)x);
    else
        return bval_int(-((long)(-x)));
}

/*
 * LOF(n) - File length.
 * Returns the length in bytes of file channel n.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_lof(BValue *args, int argc, void *rt)
{
    int chan;
    long size;
    FILE *fp;
    long cur;
    (void)argc; (void)rt;
    chan = (int)bval_to_int(&args[0]);
    fp = fileio_get_fp(chan);
    if (fp == NULL) return bval_int(0);
    cur = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return bval_int(size);
}

/*
 * ENVIRON$(name$) - Get environment variable.
 * Returns the value of the named env var, or empty
 * string if not found.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_environ(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    char namebuf[256];
    const char *val;
    char *buf;
    int len;
    (void)argc;

    if (!bval_is_string(&args[0]))
        return bval_string(NULL, 0);
    len = args[0].v.sval.length;
    if (len > 254) len = 254;
    if (args[0].v.sval.data != NULL)
        memcpy(namebuf, args[0].v.sval.data, (size_t)len);
    namebuf[len] = '\0';

    val = getenv(namebuf);
    if (val == NULL)
        return bval_string(NULL, 0);

    len = (int)strlen(val);
    buf = strpool_alloc(&state->strpool, len);
    if (buf == NULL) return bval_string(NULL, 0);
    memcpy(buf, val, (size_t)len);
    return bval_string(buf, len);
}

/*
 * builtin_peek - PEEK(address)
 * Read a byte from the virtual memory segment.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_peek(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    long addr;
    int offset;
    (void)argc;

    addr = bval_to_int(&args[0]);
    offset = state->mem_seg_base + (int)addr;
    if (offset < 0 || offset >= MAX_MEM_SEGMENT)
        return bval_int(0);
    return bval_int((long)state->mem_segment[offset]);
}

/*
 * builtin_point - POINT(x, y)
 * Read the color of a pixel from the graphics framebuffer.
 * With 1 arg: POINT(x) returns current cursor position.
 * With 2 args: POINT(x, y) returns pixel color.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_point(BValue *args, int argc, void *rt)
{
    (void)rt;
    if (argc >= 2) {
        int x = (int)bval_to_int(&args[0]);
        int y = (int)bval_to_int(&args[1]);
        return bval_int((long)gfxbuf_point(x, y));
    }
    return bval_int(0);
}

/*
 * builtin_csrlin - CSRLIN
 * Returns the current cursor row (1-based).
 * GW-BASIC: returns the vertical cursor position.
 * We track this via rt->cursor_row which is updated
 * by PRINT and LOCATE.
 * Category: FCAT_UTIL | Safety: FSAFE_STATE
 */
BValue builtin_csrlin(BValue *args, int argc, void *rt_ptr)
{
    RuntimeState *rs = (RuntimeState *)rt_ptr;
    (void)args; (void)argc;
    return bval_int((long)rs->cursor_row);
}

/*
 * builtin_cvi - CVI(string$)
 * Convert a 2-byte string to a 16-bit signed integer.
 * GW-BASIC: interprets the first 2 bytes of the string
 * as a little-endian signed integer.
 * Category: FCAT_UTIL | Safety: FSAFE_PURE
 */
BValue builtin_cvi(BValue *args, int argc, void *rt)
{
    const char *s;
    int len;
    int val;
    unsigned char lo, hi;
    (void)argc; (void)rt;
    if (!bval_is_string(&args[0])) {
        return bval_int(0);
    }
    s = args[0].v.sval.data;
    len = args[0].v.sval.length;
    if (s == NULL || len < 2) {
        return bval_int(0);
    }
    lo = (unsigned char)s[0];
    hi = (unsigned char)s[1];
    val = (int)(lo | (hi << 8));
    /* Sign-extend 16-bit */
    if (val > 32767) val -= 65536;
    return bval_int((long)val);
}

/*
 * builtin_cvs - CVS(string$)
 * Convert a 4-byte string to a single-precision float.
 * GW-BASIC: interprets the first 4 bytes of the string
 * as an IEEE 754 single-precision float (little-endian).
 * Category: FCAT_UTIL | Safety: FSAFE_PURE
 */
BValue builtin_cvs(BValue *args, int argc, void *rt)
{
    const char *s;
    int len;
    float f;
    (void)argc; (void)rt;
    if (!bval_is_string(&args[0])) {
        return bval_int(0);
    }
    s = args[0].v.sval.data;
    len = args[0].v.sval.length;
    if (s == NULL || len < 4) {
        return bval_int(0);
    }
    memcpy(&f, s, sizeof(float));
    return bval_float((double)f);
}

/*
 * builtin_cvd - CVD(string$)
 * Convert an 8-byte string to a double-precision float.
 * GW-BASIC: interprets the first 8 bytes of the string
 * as an IEEE 754 double-precision float (little-endian).
 * Category: FCAT_UTIL | Safety: FSAFE_PURE
 */
BValue builtin_cvd(BValue *args, int argc, void *rt)
{
    const char *s;
    int len;
    double d;
    (void)argc; (void)rt;
    if (!bval_is_string(&args[0])) {
        return bval_int(0);
    }
    s = args[0].v.sval.data;
    len = args[0].v.sval.length;
    if (s == NULL || len < 8) {
        return bval_int(0);
    }
    memcpy(&d, s, sizeof(double));
    return bval_float(d);
}

/* =====================================================================
 * builtins_register - Register all built-in functions.
 * =====================================================================
 *
 * This is the MASTER REGISTRATION TABLE. Every built-in function
 * is listed here with its complete metadata. To add a new function:
 * 1. Write the handler above.
 * 2. Add a row to this table.
 *
 * Table columns:
 *   name, keyword, category, ret_type, min_args, max_args,
 *   safety, overridable, handler
 *
 * DIALECT OVERRIDE POLICY:
 *   overridable=0 -> Core Immutable API (cannot be changed)
 *   overridable=1 -> Dialect-Overridable API (swappable)
 *
 *   Most math functions are core (ABS, SIN, etc.) because their
 *   mathematical definitions don't vary between dialects.
 *
 *   String functions like CHR$ are overridable because different
 *   BASICs may have different character set mappings.
 */
void builtins_register(void)
{
    /* --- Math functions (FCAT_MATH) --- */
    static const FunctionEntry math_funcs[] = {
        { "ABS",  KW_ABS,      FCAT_MATH, FRET_ANY,  1, 1,
          FSAFE_PURE, 0, builtin_abs,
          "Return absolute value: ABS(-7)=7" },
        { "SGN",  KW_SGN,      FCAT_MATH, FRET_INT,  1, 1,
          FSAFE_PURE, 0, builtin_sgn,
          "Return sign: SGN(-3)=-1, SGN(0)=0, SGN(5)=1" },
        { "INT",  KW_INT_FUNC, FCAT_MATH, FRET_INT,  1, 1,
          FSAFE_PURE, 0, builtin_int_func,
          "Truncate to integer: INT(3.7)=3" },
        { "SQR",  KW_SQR,      FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_sqr,
          "Square root: SQR(144)=12" },
        { "SIN",  KW_SIN,      FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_sin,
          "Sine (radians): SIN(3.14159)=0" },
        { "COS",  KW_COS,      FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_cos,
          "Cosine (radians): COS(0)=1" },
        { "TAN",  KW_TAN,      FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_tan,
          "Tangent (radians): TAN(0)=0" },
        { "ATN",  KW_ATN,      FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_atn,
          "Arctangent: ATN(1)=0.7854" },
        { "LOG",  KW_LOG_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
          FSAFE_PURE, 0, builtin_log,
          "Natural logarithm: LOG(2.718)=1" },
        { "EXP",  KW_EXP,       FCAT_MATH, FRET_FLOAT, 1, 1,
          FSAFE_PURE,  0, builtin_exp,
          "e^x: EXP(1)=2.718" },
        { "FIX",  KW_FIX,       FCAT_MATH, FRET_INT,   1, 1,
          FSAFE_PURE,  0, builtin_fix,
          "Truncate toward zero: FIX(-3.7)=-3" }
    };

    /* --- String functions (FCAT_STRING) --- */
    static const FunctionEntry str_funcs[] = {
        { "LEN",   KW_LEN,      FCAT_STRING, FRET_INT,   1, 1,
          FSAFE_PURE,  0, builtin_len,
          "String length: LEN(\"HELLO\")=5" },
        { "ASC",   KW_ASC,      FCAT_STRING, FRET_INT,   1, 1,
          FSAFE_PURE,  0, builtin_asc,
          "ASCII code of first char: ASC(\"A\")=65" },
        { "VAL",   KW_VAL_FUNC, FCAT_STRING, FRET_ANY,   1, 1,
          FSAFE_PURE,  0, builtin_val,
          "Convert string to number: VAL(\"42\")=42" },
        { "CHR$",  KW_CHR,      FCAT_STRING, FRET_STRING,1, 1,
          FSAFE_STATE, 1, builtin_chr,
          "Character from code: CHR$(65)=\"A\"" },
        { "STR$",  KW_STR_FUNC, FCAT_STRING, FRET_STRING,1, 1,
          FSAFE_STATE, 1, builtin_str,
          "Number to string: STR$(42)=\"42\"" },
        { "LEFT$", KW_LEFT,     FCAT_STRING, FRET_STRING,2, 2,
          FSAFE_STATE, 1, builtin_left,
          "Left substring: LEFT$(\"HELLO\",3)=\"HEL\"" },
        { "RIGHT$",KW_RIGHT,    FCAT_STRING, FRET_STRING,2, 2,
          FSAFE_STATE, 1, builtin_right,
          "Right substring: RIGHT$(\"HELLO\",3)=\"LLO\"" },
        { "MID$",  KW_MID,      FCAT_STRING, FRET_STRING,2, 3,
          FSAFE_STATE, 1, builtin_mid,
          "Middle substring: MID$(\"HELLO\",2,3)=\"ELL\"" },
        { "INSTR", KW_INSTR,    FCAT_STRING, FRET_INT,   2, 2,
          FSAFE_PURE,  0, builtin_instr,
          "Find substring: INSTR(\"HELLO\",\"LL\")=3" },
        { "SPACE$",KW_SPACE_FUNC, FCAT_STRING, FRET_STRING,1, 1,
          FSAFE_STATE, 1, builtin_space,
          "Return N spaces: SPACE$(10)" },
        { "STRING$",KW_STRING_FUNC,FCAT_STRING,FRET_STRING,2, 2,
          FSAFE_STATE, 1, builtin_string_func,
          "N copies of char: STRING$(5,\"*\")" },
        { "HEX$", KW_HEX_FUNC,  FCAT_STRING, FRET_STRING,1, 1,
          FSAFE_STATE, 1, builtin_hex,
          "Hex conversion: HEX$(255)=\"FF\"" },
        { "OCT$", KW_OCT_FUNC,  FCAT_STRING, FRET_STRING,1, 1,
          FSAFE_STATE, 1, builtin_oct,
          "Octal conversion: OCT$(8)=\"10\"" }
    };

    /* --- Utility functions (FCAT_UTIL) --- */
    static const FunctionEntry util_funcs[] = {
        { "RND",  KW_RND,  FCAT_UTIL, FRET_INT,  1, 1,
          FSAFE_STATE, 1, builtin_rnd,
          "Random number: RND(N) returns 1..N" },
        { "SIZE", KW_SIZE, FCAT_UTIL, FRET_INT,  0, 0,
          FSAFE_PURE,  0, builtin_size,
          "Free memory bytes: PRINT SIZE" },
        { "EOF",  KW_EOF,  FCAT_UTIL, FRET_INT,  1, 1,
          FSAFE_STATE, 0, builtin_eof,
          "End of file: EOF(1) returns 1 at EOF" },
        { "LOF",  KW_LOF,  FCAT_UTIL, FRET_INT,  1, 1,
          FSAFE_STATE, 0, builtin_lof,
          "File length: LOF(1) returns bytes" },
        { "ENVIRON$", KW_ENVIRON, FCAT_UTIL, FRET_STRING,
          1, 1, FSAFE_STATE, 0, builtin_environ,
          "Environment var: ENVIRON$(\"PATH\")" },
        { "PEEK",  KW_PEEK,  FCAT_UTIL, FRET_INT,  1, 1,
          FSAFE_STATE, 0, builtin_peek,
          "Virtual memory read: PEEK(addr)" },
        { "POINT", KW_POINT, FCAT_UTIL, FRET_INT,  1, 2,
          FSAFE_STATE, 0, builtin_point,
          "Pixel color: POINT(x, y)" }
    };

    int i;
    int math_count = (int)(sizeof(math_funcs) / sizeof(math_funcs[0]));
    int str_count  = (int)(sizeof(str_funcs) / sizeof(str_funcs[0]));
    int util_count = (int)(sizeof(util_funcs) / sizeof(util_funcs[0]));

    /* Register all categories */
    for (i = 0; i < math_count; i++) {
        funcreg_register(&math_funcs[i]);
    }
    for (i = 0; i < str_count; i++) {
        funcreg_register(&str_funcs[i]);
    }
    for (i = 0; i < util_count; i++) {
        funcreg_register(&util_funcs[i]);
    }
}

 // ---
 // BASIC++ Interpreter - builtins_string.c
 // ---
 //
 // String function handlers for the built-in function registry.
 //
 // Contains all FCAT_STRING handlers: substring extraction,
 // conversion, search, repetition, and base-encoding functions.
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include <string.h>
#include "builtins.h"
#include "runtime.h"
#include "errors.h"
#include "stringpool.h"
#include "config.h"
#include "value.h"

 // LEN(s$) - String length.
 // Category: FCAT_STRING | Safety: FSAFE_PURE
BValue builtin_len(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_len(&args[0], 0);
}

 // ASC(s$) - ASCII value of first character.
 // Category: FCAT_STRING | Safety: FSAFE_PURE
BValue builtin_asc(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_asc(&args[0], 0);
}

 // VAL(s$) - Convert string to number.
 // Category: FCAT_STRING | Safety: FSAFE_PURE
BValue builtin_val(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_val(&args[0], 0);
}

 // CHR$(n) - Convert number to single-character string.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
BValue builtin_chr(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 return bval_chr(&args[0], 0, &state->strpool);
}

 // STR$(n) - Convert number to string representation.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
BValue builtin_str(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 return bval_str(&args[0], 0, &state->strpool);
}

 // LEFT$(s$, n) - Left substring.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
BValue builtin_left(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 return bval_left(&args[0], &args[1], 0, &state->strpool);
}

 // RIGHT$(s$, n) - Right substring.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
BValue builtin_right(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 return bval_right(&args[0], &args[1], 0, &state->strpool);
}

 // MID$(s$, start, len) - Middle substring.
 // 2-arg form: MID$(s$, start) returns from start to end.
 // 3-arg form: MID$(s$, start, len) returns len chars.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (uses string pool)
BValue builtin_mid(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 BValue len_val;

 if (argc < 3) {
 // No length specified: use rest of string
 len_val = bval_int((long)MAX_STRING_LENGTH);
 return bval_mid(&args[0], &args[1], &len_val, 0,
 &state->strpool);
 }
 return bval_mid(&args[0], &args[1], &args[2], 0,
 &state->strpool);
}

 // INSTR(haystack$, needle$) - Find substring.
 // Returns 1-based position of needle in haystack, or 0 if not found.
 // Category: FCAT_STRING | Safety: FSAFE_PURE
BValue builtin_instr(BValue *args, int argc, void *rt)
{
 const char *h, *n;
 int hl, nl, i, start_off;
 (void)rt;

 // 2-arg: INSTR(haystack$, needle$)
 // 3-arg: INSTR(start%, haystack$, needle$)
 if (argc >= 3) {
 // 3-arg form: first arg is start position
 start_off = (int)bval_to_int(&args[0]) - 1;
 if (!bval_is_string(&args[1]) ||
 !bval_is_string(&args[2]))
 return bval_int(0);
 h = args[1].v.sval.data;
 hl = args[1].v.sval.length;
 n = args[2].v.sval.data;
 nl = args[2].v.sval.length;
 } else {
 // 2-arg form
 start_off = 0;
 if (argc < 2 || !bval_is_string(&args[0]) ||
 !bval_is_string(&args[1]))
 return bval_int(0);
 h = args[0].v.sval.data;
 hl = args[0].v.sval.length;
 n = args[1].v.sval.data;
 nl = args[1].v.sval.length;
 }

 if (h == NULL || n == NULL || nl > hl || nl == 0)
 return bval_int(0);
 if (start_off < 0) start_off = 0;
 if (start_off > hl - nl) return bval_int(0);

 for (i = start_off; i <= hl - nl; i++) {
 if (memcmp(h + i, n, (size_t)nl) == 0)
 return bval_int(i + 1); // 1-based
 }
 return bval_int(0);
}

 // SPACE$(n) - Return a string of N spaces.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
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

 // STRING$(n, char) - Return N copies of a character.
 // char can be a string (first char used) or an ASCII code.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
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

 // HEX$(n) - Convert number to hexadecimal string.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
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

 // OCT$(n) - Convert number to octal string.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
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

 // BIN$(n) - Convert number to binary string.
 // Output is grouped in 8-bit bytes separated by spaces.
 // Category: FCAT_STRING | Safety: FSAFE_STATE (string pool)
BValue builtin_bin(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 long val;
 char raw[68]; // up to 64 bits LSB-first
 char out[80]; // formatted output with spaces
 int raw_bits, num_bytes, total_bits;
 int i, o;
 char *buf;
 unsigned long uv;
 (void)argc;

 val = bval_to_int(&args[0]);
 uv = (unsigned long)val;

 // Step 1: Generate raw binary digits (LSB first)
 if (uv == 0) {
 raw_bits = 1;
 raw[0] = '0';
 } else {
 raw_bits = 0;
 while (uv > 0 && raw_bits < 64) {
 raw[raw_bits++] = (char)('0' + (int)(uv & 1));
 uv >>= 1;
 }
 }

 // Step 2: Pad to next full 8-bit byte boundary
 num_bytes = (raw_bits + 7) / 8;
 total_bits = num_bytes * 8;
 while (raw_bits < total_bits) {
 raw[raw_bits++] = '0'; // pad high bits with 0
 }

 // Step 3: Build output MSB-first with space between bytes
 o = 0;
 for (i = total_bits - 1; i >= 0; i--) {
 out[o++] = raw[i];
 if (i > 0 && (i % 8) == 0) {
 out[o++] = ' ';
 }
 }
 out[o] = '\0';

 buf = strpool_alloc(&state->strpool, o);
 if (buf == NULL) return bval_string(NULL, 0);
 memcpy(buf, out, (size_t)o);
 return bval_string(buf, o);
}

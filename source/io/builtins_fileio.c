/*
 * ---
 * BASIC++ Interpreter - builtins_fileio.c
 * ---
 *
 * File I/O function handlers for the built-in function registry.
 *
 * Contains handlers for file channel queries and binary
 * data conversion functions (CVI, CVS, CVD).
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include "builtins.h"
#include "fileio.h"
#include "value.h"

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
 if (result < 0) return bval_int(0); /* error */
 return bval_int(result ? -1 : 0);
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
 * CVI(string$) - Convert a 2-byte string to a 16-bit signed integer.
 * Interprets the first 2 bytes as a little-endian signed integer.
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
 * CVS(string$) - Convert a 4-byte string to a single-precision float.
 * Interprets the first 4 bytes as IEEE 754 single (little-endian).
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
 * CVD(string$) - Convert an 8-byte string to a double-precision float.
 * Interprets the first 8 bytes as IEEE 754 double (little-endian).
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

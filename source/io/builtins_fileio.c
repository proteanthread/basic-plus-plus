/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: builtins_fileio.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - builtins_fileio.c
 // ---
 //
 // File I/O function handlers for the built-in function registry.
 //
 // Contains handlers for file channel queries and binary
 // data conversion functions (CVI, CVS, CVD).
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "builtins.h"
#include "fileio.h"
#include "value.h"
#include "runtime.h"
#include "stringpool.h"
#include "mbf_math.h"

 // EOF(n) - End-of-file check.
 //
 // Returns -1 if file channel n is at EOF, 0 otherwise.
 // This matches GW-BASIC convention where EOF returns -1 (true)
 // or 0 (false), compatible with IF EOF(1) THEN ...
 //
 // Category: FCAT_IO | Safety: FSAFE_STATE
BValue builtin_eof(BValue *args, int argc, void *rt)
{
 int chan;
 int result;
 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 result = fileio_eof(chan, 0);
 if (result < 0) return bval_int(0); // error
 return bval_int(result ? -1 : 0);
}

 // LOF(n) - File length.
 // Returns the length in bytes of file channel n.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
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

 // CVI(string$) - Convert a 2-byte string to a 16-bit signed integer.
 // Interprets the first 2 bytes as a little-endian signed integer.
 // Category: FCAT_UTIL | Safety: FSAFE_PURE
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
 // Sign-extend 16-bit
 if (val > 32767) val -= 65536;
 return bval_int((long)val);
}

 // CVS(string$) - Convert a 4-byte string to a single-precision float.
 // Interprets the first 4 bytes as IEEE 754 single (little-endian).
 // Category: FCAT_UTIL | Safety: FSAFE_PURE
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
 if (0) {
     double val = gw_mbf32_to_double((const uint8_t *)s);
     return bval_float(val);
 }
 memcpy(&f, s, sizeof(float));
 return bval_float((double)f);
}

 // CVD(string$) - Convert an 8-byte string to a double-precision float.
 // Interprets the first 8 bytes as IEEE 754 double (little-endian).
 // Category: FCAT_UTIL | Safety: FSAFE_PURE
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
 if (0) {
     double val = gw_mbf64_to_double((const uint8_t *)s);
     return bval_float(val);
 }
 memcpy(&d, s, sizeof(double));
 return bval_float(d);
}

 // EXISTS(filename$) - File existence check.
 //
 // Returns 1 if the file exists, 0 if not.
 // Auto-appends .BAS if no extension is present.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_exists(BValue *args, int argc, void *rt)
{
 char fname[260];
 int len;
 FILE *fp;
 (void)argc; (void)rt;
 if (!bval_is_string(&args[0]))
 return bval_int(0);
 len = args[0].v.sval.length;
 if (len > 255) len = 255;
 if (args[0].v.sval.data != NULL)
 memcpy(fname, args[0].v.sval.data,
 (size_t)len);
 fname[len] = '\0';

 fp = fopen(fname, "r");
 if (fp != NULL) {
 fclose(fp);
 return bval_int(1);
 }
 return bval_int(0);
}

 // FILESIZE(filename$) - File size in bytes.
 //
 // Returns the file size, or -1 if file not found.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_filesize(BValue *args, int argc, void *rt)
{
 char fname[260];
 int len;
 FILE *fp;
 long size;
 (void)argc; (void)rt;
 if (!bval_is_string(&args[0]))
 return bval_int(-1);
 len = args[0].v.sval.length;
 if (len > 255) len = 255;
 if (args[0].v.sval.data != NULL)
 memcpy(fname, args[0].v.sval.data,
 (size_t)len);
 fname[len] = '\0';

 fp = fopen(fname, "rb");
 if (fp == NULL)
 return bval_int(-1);
 fseek(fp, 0L, SEEK_END);
 size = ftell(fp);
 fclose(fp);
 return bval_int(size);
}

 // FILEMOD$(filename$) - File modified date/time.
 //
 // Returns the modification timestamp as a string
 // in "YYYY-MM-DD HH:MM:SS" format, or empty string
 // if file not found.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_filemod(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 char fname[260];
 int len;
 struct stat st;
 char buf[64];
 char *poolbuf;
 int blen;
 struct tm *tm;
 (void)argc;
 if (!bval_is_string(&args[0]))
 return bval_string(NULL, 0);
 len = args[0].v.sval.length;
 if (len > 255) len = 255;
 if (args[0].v.sval.data != NULL)
 memcpy(fname, args[0].v.sval.data,
 (size_t)len);
 fname[len] = '\0';

 if (stat(fname, &st) != 0) {
 return bval_string(NULL, 0);
 }
 time_t t = st.st_mtime;
 tm = localtime(&t);
 if (tm == NULL) {
 return bval_string(NULL, 0);
 }
 sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
 tm->tm_year + 1900, tm->tm_mon + 1,
 tm->tm_mday, tm->tm_hour,
 tm->tm_min, tm->tm_sec);
 blen = (int)strlen(buf);
 poolbuf = strpool_alloc(&state->strpool, blen);
 if (poolbuf == NULL)
 return bval_string(NULL, 0);
 memcpy(poolbuf, buf, (size_t)blen);
 return bval_string(poolbuf, blen);
}

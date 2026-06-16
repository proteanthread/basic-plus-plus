/*
 * ---
 * BASIC++ Interpreter - builtins.c
 * ---
 *
 * Built-in function registration table.
 *
 * This file contains ONLY the master registration table that
 * populates the function registry with all standard BASIC++
 * built-in functions.
 *
 * Handler implementations are in separate category files:
 *
 *   builtins_math.c     - ABS, SGN, INT, SQR, SIN, COS, TAN, ATN,
 *                          LOG, EXP, FIX, COMPLEX, REAL, IMAG,
 *                          MIN, MAX, AVG, MED, ROUND, ASIN, ACOS,
 *                          SINH, COSH, TANH, LOG10, LOG2, COMP,
 *                          PDIF, PI, RND
 *   builtins_string.c   - LEN, ASC, VAL, CHR$, STR$, LEFT$, RIGHT$,
 *                          MID$, INSTR, SPACE$, STRING$, HEX$, OCT$,
 *                          BIN$
 *   builtins_fileio.c   - EOF, LOF, CVI, CVS, CVD
 *   builtins_io.c       - CSRLIN
 *   builtins_memory.c   - PEEK, SIZE
 *   builtins_system.c   - ENVIRON$
 *   builtins_graphics.c - POINT
 *
 * DIALECT OVERRIDE POLICY:
 * overridable=0 -> Core Immutable API (cannot be changed)
 * overridable=1 -> Dialect-Overridable API (swappable)
 *
 * ---
 */

#include "builtins.h"
#include "funcreg.h"
#include "lexer.h"

/* --- builtins_register - Register all built-in functions. ---
 *
 * This is the MASTER REGISTRATION TABLE. Every built-in function
 * is listed here with its complete metadata. To add a new function:
 * 1. Write the handler in the appropriate category file.
 * 2. Declare it in builtins.h.
 * 3. Add a row to the appropriate table below.
 *
 * Table columns:
 * name, keyword, category, ret_type, min_args, max_args,
 * safety, overridable, handler, help_text
 */
void builtins_register(void)
{
 /* --- Arithmetic / Math (builtins_math.c) --- */
 static const FunctionEntry math_funcs[] = {
 { "ABS", KW_ABS, FCAT_MATH, FRET_ANY, 1, 1,
 FSAFE_PURE, 0, builtin_abs,
 "Return absolute value: ABS(-7)=7" },
 { "SGN", KW_SGN, FCAT_MATH, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_sgn,
 "Return sign: SGN(-3)=-1, SGN(0)=0, SGN(5)=1" },
 { "INT", KW_INT_FUNC, FCAT_MATH, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_int_func,
 "Truncate to integer: INT(3.7)=3" },
 { "SQR", KW_SQR, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_sqr,
 "Square root: SQR(144)=12" },
 { "SIN", KW_SIN, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_sin,
 "Sine (radians): SIN(3.14159)=0" },
 { "COS", KW_COS, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_cos,
 "Cosine (radians): COS(0)=1" },
 { "TAN", KW_TAN, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_tan,
 "Tangent (radians): TAN(0)=0" },
 { "ATN", KW_ATN, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_atn,
 "Arctangent: ATN(1)=0.7854" },
 { "LOG", KW_LOG_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
 FSAFE_PURE, 0, builtin_log,
 "Natural logarithm: LOG(2.718)=1" },
 { "EXP", KW_EXP, FCAT_MATH, FRET_FLOAT, 1, 1,
 FSAFE_PURE, 0, builtin_exp,
 "e^x: EXP(1)=2.718" },
  { "FIX", KW_FIX, FCAT_MATH, FRET_INT, 1, 1,
  FSAFE_PURE, 0, builtin_fix,
  "Truncate toward zero: FIX(-3.7)=-3" },
  { "COMPLEX", KW_COMPLEX, FCAT_MATH, FRET_ANY, 2, 2,
  FSAFE_PURE, 0, builtin_complex,
  "Create complex number: COMPLEX(3,4)" },
  { "REAL", KW_REAL_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_real,
  "Real part: REAL(COMPLEX(3,4))=3" },
  { "IMAG", KW_IMAG_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_imag,
  "Imaginary part: IMAG(COMPLEX(3,4))=4" },
  { "CONJ", KW_CONJ_FUNC, FCAT_MATH, FRET_ANY, 1, 1,
  FSAFE_PURE, 0, builtin_conj,
  "Complex conjugate: CONJ(COMPLEX(3,4))=(3-4i)" },
  { "CABS", KW_CABS_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_cabs,
  "Complex magnitude: CABS((3+4i))=5" },
  { "CSQR", KW_CSQR_FUNC, FCAT_MATH, FRET_ANY, 1, 1,
  FSAFE_PURE, 0, builtin_csqr,
  "Complex square root: CSQR((-1+0i))" },
  { "CEXP", KW_CEXP_FUNC, FCAT_MATH, FRET_ANY, 1, 1,
  FSAFE_PURE, 0, builtin_cexp,
  "Complex exponential: CEXP((0+3.14i))" },
  { "CLOG", KW_CLOG_FUNC, FCAT_MATH, FRET_ANY, 1, 1,
  FSAFE_PURE, 0, builtin_clog,
  "Complex natural log: CLOG((-1+0i))" },
  { "CARG", KW_CARG_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_carg,
  "Complex argument (phase): CARG((0+1i))" },
  { "CPOW", KW_CPOW_FUNC, FCAT_MATH, FRET_ANY, 2, 2,
  FSAFE_PURE, 0, builtin_cpow,
  "Complex power: CPOW((3+4i), 2)" },
  { "MIN", KW_MIN_FUNC, FCAT_MATH, FRET_FLOAT,2, 16,
  FSAFE_PURE, 0, builtin_min,
  "Minimum value: MIN(5,3)=3" },
  { "MAX", KW_MAX_FUNC, FCAT_MATH, FRET_FLOAT,2, 16,
  FSAFE_PURE, 0, builtin_max,
  "Maximum value: MAX(5,3)=5" },
  { "AVG", KW_AVG_FUNC, FCAT_MATH, FRET_FLOAT,1, 16,
  FSAFE_PURE, 0, builtin_avg,
  "Average: AVG(10,20,30)=20" },
  { "MED", KW_MED_FUNC, FCAT_MATH, FRET_FLOAT,1, 16,
  FSAFE_PURE, 0, builtin_med,
  "Median: MED(3,1,2)=2" },
  { "ROUND", KW_ROUND_FUNC, FCAT_MATH, FRET_FLOAT,1, 2,
  FSAFE_PURE, 0, builtin_round,
  "Round: ROUND(3.14159,2)=3.14" },
  /* SUPER BASIC (Tymshare) extended math */
  { "ASIN", KW_ASIN_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_asin,
  "Arcsine: ASIN(1)=1.5708" },
  { "ACOS", KW_ACOS_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_acos,
  "Arccosine: ACOS(0)=1.5708" },
  { "SINH", KW_SINH_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_sinh,
  "Hyperbolic sine: SINH(1)=1.1752" },
  { "COSH", KW_COSH_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_cosh,
  "Hyperbolic cosine: COSH(0)=1" },
  { "TANH", KW_TANH_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_tanh,
  "Hyperbolic tangent: TANH(0)=0" },
  { "LOG10", KW_LOG10_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_log10,
  "Log base 10: LOG10(100)=2" },
  { "LOG2", KW_LOG2_FUNC, FCAT_MATH, FRET_FLOAT,1, 1,
  FSAFE_PURE, 0, builtin_log2,
  "Log base 2: LOG2(8)=3" },
  { "COMP", KW_COMP_FUNC, FCAT_MATH, FRET_FLOAT,2, 2,
  FSAFE_PURE, 0, builtin_comp,
  "Compare: COMP(3,5)=-1" },
  { "PDIF", KW_PDIF_FUNC, FCAT_MATH, FRET_FLOAT,2, 2,
  FSAFE_PURE, 0, builtin_pdif,
  "Positive diff: PDIF(5,3)=2" },
  { "PI", KW_PI_FUNC, FCAT_MATH, FRET_FLOAT,0, 0,
  FSAFE_PURE, 0, builtin_pi,
  "Pi constant: PI=3.14159" },
 { "RND", KW_RND, FCAT_MATH, FRET_ANY, 1, 1,
 FSAFE_STATE, 1, builtin_rnd,
 "Random number: RND(N) returns 1..N" }
 };

 /* --- String Functions (builtins_string.c) --- */
 static const FunctionEntry str_funcs[] = {
 { "LEN", KW_LEN, FCAT_STRING, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_len,
 "String length: LEN(\"HELLO\")=5" },
 { "ASC", KW_ASC, FCAT_STRING, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_asc,
 "ASCII code of first char: ASC(\"A\")=65" },
 { "VAL", KW_VAL_FUNC, FCAT_STRING, FRET_ANY, 1, 1,
 FSAFE_PURE, 0, builtin_val,
 "Convert string to number: VAL(\"42\")=42" },
 { "CHR$", KW_CHR, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_chr,
 "Character from code: CHR$(65)=\"A\"" },
 { "STR$", KW_STR_FUNC, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_str,
 "Number to string: STR$(42)=\"42\"" },
 { "LEFT$", KW_LEFT, FCAT_STRING, FRET_STRING,2, 2,
 FSAFE_STATE, 1, builtin_left,
 "Left substring: LEFT$(\"HELLO\",3)=\"HEL\"" },
 { "RIGHT$",KW_RIGHT, FCAT_STRING, FRET_STRING,2, 2,
 FSAFE_STATE, 1, builtin_right,
 "Right substring: RIGHT$(\"HELLO\",3)=\"LLO\"" },
 { "MID$", KW_MID, FCAT_STRING, FRET_STRING,2, 3,
 FSAFE_STATE, 1, builtin_mid,
 "Middle substring: MID$(\"HELLO\",2,3)=\"ELL\"" },
 { "INSTR", KW_INSTR, FCAT_STRING, FRET_INT, 2, 3,
 FSAFE_PURE, 0, builtin_instr,
 "Find substring: INSTR(\"HELLO\",\"LL\")=3" },
 { "SPACE$",KW_SPACE_FUNC, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_space,
 "Return N spaces: SPACE$(10)" },
 { "STRING$",KW_STRING_FUNC,FCAT_STRING,FRET_STRING,2, 2,
 FSAFE_STATE, 1, builtin_string_func,
 "N copies of char: STRING$(5,\"*\")" },
 { "HEX$", KW_HEX_FUNC, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_hex,
 "Hex conversion: HEX$(255)=\"FF\"" },
 { "OCT$", KW_OCT_FUNC, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_oct,
 "Octal conversion: OCT$(8)=\"10\"" },
 { "BIN$", KW_BIN_FUNC, FCAT_STRING, FRET_STRING,1, 1,
 FSAFE_STATE, 1, builtin_bin,
 "Binary conversion: BIN$(10)=\"1010\"" }
 };

 /* --- File I/O (builtins_fileio.c) --- */
 static const FunctionEntry fileio_funcs[] = {
 { "EOF", KW_EOF, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_STATE, 0, builtin_eof,
 "End of file: EOF(1) returns -1 at EOF" },
 { "LOF", KW_LOF, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_STATE, 0, builtin_lof,
 "File length: LOF(1) returns bytes" },
 { "CVI", KW_CVI, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_cvi,
 "Unpack 2-byte string to integer: CVI(A$)" },
 { "CVS", KW_CVS, FCAT_UTIL, FRET_FLOAT, 1, 1,
 FSAFE_PURE, 0, builtin_cvs,
 "Unpack 4-byte string to single: CVS(A$)" },
 { "CVD", KW_CVD, FCAT_UTIL, FRET_FLOAT, 1, 1,
 FSAFE_PURE, 0, builtin_cvd,
 "Unpack 8-byte string to double: CVD(A$)" },
 { "EXISTS", KW_EXISTS, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_STATE, 0, builtin_exists,
 "File exists: EXISTS(\"file.BAS\") returns 1 or 0" },
 { "FILESIZE", KW_FILESIZE, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_STATE, 0, builtin_filesize,
 "File size: FILESIZE(\"file.BAS\") in bytes" },
 { "FILEMOD$", KW_FILEMOD, FCAT_UTIL, FRET_STRING,
 1, 1, FSAFE_STATE, 0, builtin_filemod,
 "Modified date: FILEMOD$(\"file\")=\"2026-01-15 10:30:00\"" }
 };

 /* --- Input / Output (builtins_io.c) --- */
 static const FunctionEntry io_funcs[] = {
 { "CSRLIN", KW_CSRLIN, FCAT_UTIL, FRET_INT, 0, 0,
 FSAFE_STATE, 0, builtin_csrlin,
 "Current cursor row: PRINT CSRLIN" }
 };

 /* --- Memory (builtins_memory.c) --- */
 static const FunctionEntry mem_funcs[] = {
 { "PEEK", KW_PEEK, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_STATE, 0, builtin_peek,
 "Virtual memory read: PEEK(addr)" },
 { "SIZE", KW_SIZE, FCAT_UTIL, FRET_INT, 0, 0,
 FSAFE_PURE, 0, builtin_size,
 "Free memory bytes: PRINT SIZE" }
 };

 /* --- System / Environment (builtins_system.c) --- */
 static const FunctionEntry sys_funcs[] = {
 { "ENVIRON$", KW_ENVIRON, FCAT_UTIL, FRET_STRING,
 1, 1, FSAFE_STATE, 0, builtin_environ,
 "Environment var: ENVIRON$(\"PATH\")" }
 };

 /* --- Graphics (builtins_graphics.c) --- */
 static const FunctionEntry gfx_funcs[] = {
 { "POINT", KW_POINT, FCAT_UTIL, FRET_INT, 1, 2,
 FSAFE_STATE, 0, builtin_point,
 "Pixel color: POINT(x, y)" }
 };

 /* --- Stream I/O (builtins_sio.c) --- */
 static const FunctionEntry sio_funcs[] = {
 { "SIOREAD$", KW_SIOREAD, FCAT_IO, FRET_STRING, 2, 2,
 FSAFE_IO, 0, builtin_sioread,
 "Read bytes: SIOREAD$(chan, n)" },
 { "SIOREADLN$", KW_SIOREADLN, FCAT_IO, FRET_STRING, 1, 1,
 FSAFE_IO, 0, builtin_sioreadln,
 "Read line: SIOREADLN$(chan)" },
 { "SIOWRITE", KW_SIOWRITE, FCAT_IO, FRET_INT, 2, 2,
 FSAFE_IO, 0, builtin_siowrite,
 "Write data: SIOWRITE(chan, data$)" },
 { "SIOSEEK", KW_SIOSEEK, FCAT_IO, FRET_INT, 2, 2,
 FSAFE_IO, 0, builtin_sioseek,
 "Seek: SIOSEEK(chan, pos)" },
 { "SIOFLUSH", KW_SIOFLUSH, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_sioflush,
 "Flush: SIOFLUSH(chan)" },
 { "SIOSTATUS", KW_SIOSTATUS, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_siostatus,
 "Status: SIOSTATUS(chan) bitmask" },
 { "SIOAVAIL", KW_SIOAVAIL, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_sioavail,
 "Available: SIOAVAIL(chan) bytes" }
 };

 /* --- Block I/O (builtins_bio.c) --- */
 static const FunctionEntry bio_funcs[] = {
 { "BIOREAD$", KW_BIOREAD, FCAT_IO, FRET_STRING, 3, 3,
 FSAFE_IO, 0, builtin_bioread,
 "Read block: BIOREAD$(chan, pos, len)" },
 { "BIOWRITE", KW_BIOWRITE, FCAT_IO, FRET_INT, 3, 3,
 FSAFE_IO, 0, builtin_biowrite,
 "Write block: BIOWRITE(chan, pos, data$)" },
 { "BIOSTATUS", KW_BIOSTATUS, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_biostatus,
 "Block status: BIOSTATUS(chan) bitmask" },
 { "BIOSIZE", KW_BIOSIZE, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_biosize,
 "File/dev size: BIOSIZE(chan)" },
 { "BIOCHECKSUM", KW_BIOCHECKSUM, FCAT_UTIL, FRET_INT, 1, 1,
 FSAFE_PURE, 0, builtin_biochecksum,
 "CRC-16: BIOCHECKSUM(data$)" },
 { "BIOCOMPARE", KW_BIOCOMPARE, FCAT_IO, FRET_INT, 3, 3,
 FSAFE_IO, 0, builtin_biocompare,
 "Compare: BIOCOMPARE(chan, pos, data$)" },
 { "BIOFILL", KW_BIOFILL, FCAT_IO, FRET_INT, 4, 4,
 FSAFE_IO, 0, builtin_biofill,
 "Fill block: BIOFILL(chan, pos, len, val)" },
 { "BIOCOPY", KW_BIOCOPY, FCAT_IO, FRET_INT, 4, 4,
 FSAFE_IO, 0, builtin_biocopy,
 "Copy block: BIOCOPY(chan, src, dst, len)" }
 };

 /* Transaction query functions */
 static const FunctionEntry txn_funcs[] = {
 { "TXNSTATUS", KW_TXNSTATUS, FCAT_IO, FRET_INT, 0, 0,
 FSAFE_PURE, 0, builtin_txnstatus,
 "Transaction state: TXNSTATUS()" }
 };

 /* Network query functions */
 static const FunctionEntry net_funcs[] = {
 { "NSTATUS", KW_NSTATUS, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_nstatus,
 "Network channel status: NSTATUS(ch)" },
 { "NHTTPSTATUS", KW_NHTTPSTATUS, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_nhttpstatus,
 "HTTP response code: NHTTPSTATUS(ch)" },
 { "NEOF", KW_NEOF, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_neof,
 "Network EOF flag: NEOF(ch)" },
 { "NBYTESWAITING", KW_NBYTESWAITING, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_nbyteswaiting,
 "Bytes waiting: NBYTESWAITING(ch)" },
 { "NCONNECTED", KW_NCONNECTED, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_nconnected,
 "Connection state: NCONNECTED(ch)" },
 { "NERROR", KW_NERROR, FCAT_IO, FRET_INT, 1, 1,
 FSAFE_IO, 0, builtin_nerror,
 "Last error code: NERROR(ch)" },
 { "NJSONQUERY$", KW_NJSONQUERY, FCAT_IO, FRET_STRING, 2, 2,
 FSAFE_IO, 0, builtin_njsonquery,
 "JSON query: NJSONQUERY$(ch, path$)" },
 { "NINFO$", KW_NINFO, FCAT_IO, FRET_STRING, 1, 1,
 FSAFE_PURE, 0, builtin_ninfo,
 "Adapter info: NINFO$(key$)" }
 };

 int i;
 int math_count   = (int)(sizeof(math_funcs)   / sizeof(math_funcs[0]));
 int str_count    = (int)(sizeof(str_funcs)    / sizeof(str_funcs[0]));
 int fileio_count = (int)(sizeof(fileio_funcs) / sizeof(fileio_funcs[0]));
 int io_count     = (int)(sizeof(io_funcs)     / sizeof(io_funcs[0]));
 int mem_count    = (int)(sizeof(mem_funcs)    / sizeof(mem_funcs[0]));
 int sys_count    = (int)(sizeof(sys_funcs)    / sizeof(sys_funcs[0]));
 int gfx_count    = (int)(sizeof(gfx_funcs)    / sizeof(gfx_funcs[0]));
 int sio_count    = (int)(sizeof(sio_funcs)    / sizeof(sio_funcs[0]));
 int bio_count    = (int)(sizeof(bio_funcs)    / sizeof(bio_funcs[0]));
 int txn_count    = (int)(sizeof(txn_funcs)    / sizeof(txn_funcs[0]));
 int net_count    = (int)(sizeof(net_funcs)    / sizeof(net_funcs[0]));

 /* Register all categories */
 for (i = 0; i < math_count; i++)
 funcreg_register(&math_funcs[i]);
 for (i = 0; i < str_count; i++)
 funcreg_register(&str_funcs[i]);
 for (i = 0; i < fileio_count; i++)
 funcreg_register(&fileio_funcs[i]);
 for (i = 0; i < io_count; i++)
 funcreg_register(&io_funcs[i]);
 for (i = 0; i < mem_count; i++)
 funcreg_register(&mem_funcs[i]);
 for (i = 0; i < sys_count; i++)
 funcreg_register(&sys_funcs[i]);
 for (i = 0; i < gfx_count; i++)
 funcreg_register(&gfx_funcs[i]);
 for (i = 0; i < sio_count; i++)
 funcreg_register(&sio_funcs[i]);
 for (i = 0; i < bio_count; i++)
 funcreg_register(&bio_funcs[i]);
 for (i = 0; i < txn_count; i++)
 funcreg_register(&txn_funcs[i]);
 for (i = 0; i < net_count; i++)
 funcreg_register(&net_funcs[i]);
}


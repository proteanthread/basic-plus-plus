/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lexer.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Lexical analysis, zero-copy tokenization scanner, keyword property tables, and natural language keyword alias packs.
 *
 * 2. WHAT TO EXPECT:
 *    Single-pass zero-copy tokenization. Keywords are filtered based on the active dialect configuration at parse time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    New keywords definitions, keyword property modifiers, and multilingual alias mappings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Core lexer scan logic, token type enumeration values, zero-copy string indexing.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify the keyword list ordering (it must be sorted/indexed correctly). Run self-tests for lexical parsing to locate tokenization boundaries.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - lexer.c
// ---
//
// Lexer (tokenizer) implementation.
//
// PURPOSE:
//   Converts source text into a stream of Token value types.
//   This is the first stage of the interpretation pipeline:
//     Source text -> [LEXER] -> Token stream -> [PARSER]
//
// HOW IT WORKS:
//   Single-pass scanner with these design choices:
//   1. No heap allocation -- tokens are value types in the Lexer struct.
//   2. Zero-copy strings -- TOK_STRING points into the source buffer.
//   3. Case-insensitive keywords -- uppercase comparison, no mutation.
//   4. Greedy operator matching -- <= and >= by one-char lookahead.
//   5. # is context-sensitive -- emitted as TOK_HASH; parser decides.
//
// KEYWORD TABLE ARCHITECTURE:
//   core_keyword_init_table[] -- static table of all keywords with
//   their KeywordId and dialect_flags. Copied into the dynamic
//   table at init so custom keywords can be added at runtime.
//
//   dialect_flags determine which keywords are available when
//   OPTION STRICT is enabled. DFLAG_ALL = always available.
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new keyword:
//   1. Add KW_YOURNAME to the KeywordId enum in lexer.h.
//   2. Add { "YOURNAME", KW_YOURNAME, DFLAG_xxx } to the table below.
//   3. Handle KW_YOURNAME in the parser (parser.c dispatch table).
//
//   Adding a keyword alias (e.g., IMPRIME -> PRINT):
//   Use the ALIAS command at runtime, or call lexer_add_alias()
//   from C code. Aliases are checked before the keyword table.
//
//   Adding a new token type (e.g., a new operator):
//   1. Add TOK_YOURTYPE to the TokenType enum in lexer.h.
//   2. Add recognition logic in lexer_next_token() below.
//   3. Handle it in the parser expression evaluator.
//
// FINE-TUNING:
//   MAX_DYNAMIC_KEYWORDS (512): max keywords after runtime additions.
//   MAX_ALIASES (128): max keyword aliases. Increase for heavily
//   internationalized builds.
//
// TROUBLESHOOTING:
//   - Keyword not recognized:
//     Check dialect_flags. With OPTION STRICT, only keywords
//     flagged for the active dialect are available.
//   - Alias conflict:
//     Aliases cannot shadow existing keywords. Returns -2.
//   - "WHAT?" on valid-looking syntax:
//     Check if the keyword has a $ suffix requirement
//     (e.g., LEFT$ vs LEFT). See lexer_keyword_needs_dollar().
//
// MINIMALIZATION:
//   This file is CORE tier -- always required.
//   To reduce keyword table size for embedded builds, remove
//   entries from core_keyword_init_table[] for keywords you
//   don't need (e.g., remove all DFLAG_SINC entries if you
//   don't support Sinclair BASIC).
//
// C17 COMPLIANCE:
//   - Uses strdup() which is POSIX; MSVC warns (use _strdup).
//     Suppressed by _CRT_SECURE_NO_WARNINGS.
//
// ---

#ifndef _WIN32
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200809L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "errors.h"
#include "../console.h"
#include "../platform.h"

// --- Keyword Table ---
 // Static table mapping keyword strings to KeywordId values.
 // Each entry includes dialect_flags for OPTION STRICT support.
typedef struct KeywordEntry {
 const char *name;
 KeywordId id;
 unsigned int dialect_flags;
} KeywordEntry;

 // Dialect flag legend:
 // DFLAG_ALL = available in all dialects / BASIC++ native
 // DFLAG_MSALL = all Microsoft BASIC family + TRS-80
 // DFLAG_MSBASIC= TRS2+GWBS+QBAS+ASFT+C64B+COCO
 // DFLAG_GWQB = GW-BASIC + QBasic
 // DFLAG_STRUCT = GWBS + QBAS + E116 (structured flow)
#ifdef BPP_LITE_BUILD
static const KeywordEntry core_keyword_init_table[] = {
    { "PRINT", KW_PRINT },
    { "LET", KW_LET },
    { "INPUT", KW_INPUT },
    { "IF", KW_IF },
    { "GOTO", KW_GOTO },
    { "END", KW_END },
    { "REM", KW_REM },
    { "BRUN", KW_BRUN },
    { "MODULE", KW_MODULE },
    { "CSAVE", KW_CSAVE },
    { "CLOAD", KW_CLOAD },
    { "CRUN", KW_CRUN },
    { "MOTOR", KW_MOTOR },
    { "HELP", KW_HELP },
    { "INFO", KW_INFO },
    { "THEN", KW_THEN },
    { "FOR", KW_FOR },
    { "NEXT", KW_NEXT },
    { "TO", KW_TO },
    { "STEP", KW_STEP },
    { "ON", KW_ON },
    { "DATA", KW_DATA },
    { "READ", KW_READ },
    { "RESTORE", KW_RESTORE },
    { "DIM", KW_DIM },
    { "BEEP", KW_BEEP },
    { "VARS", KW_VARS },
    { "FRE", KW_FRE },
    { "TIMER", KW_TIMER },
    { "RND", KW_RND },
    { "VER", KW_VER },
    { "PWD", KW_PWD },
    { "CWD", KW_CWD },
    { "HOSTNAME", KW_HOSTNAME },
    { "USERNAME", KW_USERNAME },
    { "DAY", KW_DAY },
    { "MONTH", KW_MONTH },
    { "YEAR", KW_YEAR },
    { "HOURS", KW_HOURS },
    { "MINUTES", KW_MINUTES },
    { "SECONDS", KW_SECONDS },
    { "JIFFIES", KW_JIFFIES },
    { NULL, 0, 0 } // sentinel
};
#else
static const KeywordEntry core_keyword_init_table[] = {
 // Universal BASIC keywords
 { "PRINT", KW_PRINT },
 { "LET", KW_LET },
 { "INPUT", KW_INPUT },
 { "IF", KW_IF },
 { "GOTO", KW_GOTO },
 { "GOSUB", KW_GOSUB },
 { "RETURN", KW_RETURN },
 { "END", KW_END },
 { "REM", KW_REM },
 { "RUN", KW_RUN },
 { "LIST", KW_LIST },
 { "NEW", KW_NEW },
 { "SAVE", KW_SAVE },
 { "LOAD", KW_LOAD },
 { "UNLOAD", KW_UNLOAD },
 { "STOP", KW_STOP },
 { "THEN", KW_THEN },
 { "FOR", KW_FOR },
 { "NEXT", KW_NEXT },
 { "TO", KW_TO },
 { "STEP", KW_STEP },
 { "ON", KW_ON },
 // Math functions
 { "ABS", KW_ABS },
 { "RND", KW_RND },
 { "SGN", KW_SGN },
 { "SIN", KW_SIN },
 { "COS", KW_COS },
 { "TAN", KW_TAN },
 { "ATN", KW_ATN },
 { "SQR", KW_SQR },
 { "LOG", KW_LOG_FUNC },
 { "EXP", KW_EXP },
 { "INT", KW_INT_FUNC },
 // String functions
 { "LEN", KW_LEN },
 { "LEFT", KW_LEFT },
 { "RIGHT", KW_RIGHT },
 { "MID", KW_MID },
 { "CHR", KW_CHR },
 { "ASC", KW_ASC },
 { "STR", KW_STR_FUNC },
 { "VAL", KW_VAL_FUNC },
 { "NUM", KW_NUM },
 { "PSTORE", KW_PSTORE },
 { "PRETRIEVE", KW_PRETRIEVE },
 // DATA/READ/RESTORE
 { "DATA", KW_DATA },
 { "READ", KW_READ },
 { "RESTORE", KW_RESTORE },
 // DIM
 { "DIM", KW_DIM },
 // SIZE (Tiny BASIC)
 { "SIZE", KW_SIZE },
 // WHILE/WEND
 { "WHILE", KW_WHILE },
 { "WEND", KW_WEND },
 // DO/LOOP/UNTIL
 { "DO", KW_DO },
 { "UNTIL", KW_UNTIL },
 { "LOOP", KW_LOOP },
 // MERGE/CHAIN
 { "MERGE", KW_MERGE },
 { "CHAIN", KW_CHAIN },
 // CLS/CLEAR/CLR
 { "CLS", KW_CLS },
 { "CLEAR", KW_CLEAR },
 { "CLR", KW_CLR },
 // TRON/TROFF
 { "TRON", KW_TRON },
 { "TROFF", KW_TROFF },
 // Error handling
 { "ERROR", KW_ERROR },
 // DEF FN / FN
 { "DEF", KW_DEF },
 { "FN", KW_FN },
 // MAT
 { "MAT", KW_MAT },
 // Array functions
 { "LBOUND", KW_LBOUND },
 { "UBOUND", KW_UBOUND },
 { "DET", KW_DET },
 // File I/O
 { "OPEN", KW_OPEN },
 { "CLOSE", KW_CLOSE },
 { "AS", KW_AS },
 { "EOF", KW_EOF },
 // BASIC++ native (always available)
 { "COMPILE", KW_COMPILE },
 { "BSAVE", KW_BSAVE },
 { "BLOAD", KW_BLOAD },
 { "BRUN", KW_BRUN },
 { "MODULE", KW_MODULE },
 { "SECURITY", KW_SECURITY },
 { "SYSTEM", KW_SYSTEM },
 { "BREAK", KW_BREAK },
 { "CONT", KW_CONT },
 { "REFORMAT", KW_REFORMAT },
 { "VARS", KW_VARS },
 { "ASSERT", KW_ASSERT },
 { "TEST", KW_TEST },
 { "ENDTEST", KW_ENDTEST },
 { "SELFTEST", KW_SELFTEST },
 { "CSAVE", KW_CSAVE },
 { "CLOAD", KW_CLOAD },
 { "CRUN", KW_CRUN },
 { "MOTOR", KW_MOTOR },
 { "HELP", KW_HELP },
 { "INFO", KW_INFO },
 { "CATALOG", KW_CATALOG },
 { "RENUM", KW_RENUM },
 { "DELETE", KW_DELETE },
 { "VER", KW_VER },
 // GW-BASIC / QBasic compatibility
 { "ELSEIF", KW_ELSEIF },
 { "ENDIF", KW_ENDIF },
 { "CAUSE", KW_CAUSE },
 { "WHEN", KW_WHEN },
 { "USE", KW_USE },
 { "RETRY", KW_RETRY },
 { "CONTINUE", KW_CONTINUE },
 { "DECLARE", KW_DECLARE },
 { "ELSE", KW_ELSE },
 { "SWAP", KW_SWAP },
 { "RANDOMIZE", KW_RANDOMIZE },
 { "INSTR", KW_INSTR },
 { "SPACE", KW_SPACE_FUNC },
 { "STRING", KW_STRING_FUNC },
 { "HEX", KW_HEX_FUNC },
 { "OCT", KW_OCT_FUNC },
 { "FIX", KW_FIX },
 { "FILES", KW_FILES },
 { "BEEP", KW_BEEP },
 { "COLOR", KW_COLOR },
 { "DIR", KW_DIR },
 { "AUTO", KW_AUTO },
 { "TAB", KW_TAB_FUNC },
 { "SPC", KW_SPC_FUNC },
 { "SOUND", KW_SOUND },
 { "PLAY", KW_PLAY },
 { "SCREEN", KW_SCREEN },
 { "GRAPHICS", KW_GRAPHICS },
 { "DRAW", KW_DRAW },
 { "WIDTH", KW_WIDTH },
 // Logical/bitwise operators
 { "MOD", KW_MOD },
 { "AND", KW_AND },
 { "OR", KW_OR },
 { "NOT", KW_NOT },
 { "XOR", KW_XOR },
 { "EQV", KW_EQV },
 { "IMP", KW_IMP },
 // String utilities
 { "LCASE", KW_LCASE },
 { "UCASE", KW_UCASE },
 { "TCASE", KW_TCASE },
 { "LTRIM", KW_LTRIM },
 { "RTRIM", KW_RTRIM },
 { "TRIM", KW_TRIM },
 { "REPLACE", KW_REPLACE },
 { "REVERSE", KW_REVERSE },
 { "MCASE", KW_MCASE },
 { "ICASE", KW_ICASE },
 { "ONKEY", KW_ONKEY },
 { "LIKE", KW_LIKE },
 { "HASH", KW_HASH },
 { "CINT", KW_CINT },
 { "TIMER", KW_TIMER },
 { "SLEEP", KW_SLEEP },
 { "LOCATE", KW_LOCATE },
 { "LINE", KW_LINE },
 { "DATE", KW_DATE },
 { "TIME", KW_TIME },
 // QBasic structured features
 { "SELECT", KW_SELECT },
 { "CASE", KW_CASE },
 { "IS", KW_IS },
 { "CSNG", KW_CSNG },
 { "CDBL", KW_CDBL },
 { "EXIT", KW_EXIT },
 { "CONST", KW_CONST_KW },
 { "INKEY", KW_INKEY },
 { "ERASE", KW_ERASE },
 { "USING", KW_USING },
 { "LPRINT", KW_LPRINT },
 // SUB/FUNCTION
 { "SUB", KW_SUB },
 { "CALL", KW_CALL },
 { "FUNCTION", KW_FUNCTION },
 // Additional features
 { "SHELL", KW_SHELL },
 { "REDIM", KW_REDIM },
 { "SHARED", KW_SHARED },
 { "PUBLIC", KW_PUBLIC },
 { "BANK", KW_BANK },
 { "STATIC", KW_STATIC },
 { "RESUME", KW_RESUME },
 { "OPTION", KW_OPTION },
 { "ENVIRON", KW_ENVIRON },
 { "LOF", KW_LOF },
 { "COMMON", KW_COMMON },
 { "SEEK", KW_SEEK },
 { "CHDIR", KW_CHDIR },
 // Memory / graphics
 { "PEEK", KW_PEEK },
 { "POKE", KW_POKE },
 { "SEG", KW_SEG },
 { "PSET", KW_PSET },
 { "CIRCLE", KW_CIRCLE },
 { "PALETTE", KW_PALETTE },
 { "POINT", KW_POINT },
 { "TYPE", KW_TYPE },
 { "PAINT", KW_PAINT },
 { "ACCESS", KW_ACCESS },
 // GW-BASIC specific
 { "CSRLIN", KW_CSRLIN },
 { "CVI", KW_CVI },
 { "CVS", KW_CVS },
 { "CVD", KW_CVD },
 { "DEFINT", KW_DEFINT },
 { "DEFDBL", KW_DEFDBL },
 { "DEFSNG", KW_DEFSNG },
 { "DEFSTR", KW_DEFSTR },
 { "ERL", KW_ERL },
 { "ERR", KW_ERR_VAR },
 { "ERR$", KW_ERR_STR },
 { "EDIT", KW_EDIT },
 { "EXTERR", KW_EXTERR },
 { "ERDEV", KW_ERDEV },
 { "FIELD", KW_FIELD },
 { "FRE", KW_FRE },
 { "TASK", KW_TASK },
 { "PEEKB", KW_PEEKB },
 { "POKEB", KW_POKEB },
 { "GET", KW_GET },
 { "PUT", KW_PUT },
 { "INP", KW_INP },
 { "IOCTL", KW_IOCTL },
 { "KEY", KW_KEY },
 { "KILL", KW_KILL },
 { "LLIST", KW_LLIST },
 { "LOC", KW_LOC },
 { "LOCK", KW_LOCK },
 { "LPOS", KW_LPOS },
 { "LSET", KW_LSET },
 { "MKDIR", KW_MKDIR },
 { "MKD", KW_MKD_FUNC },
 { "MKI", KW_MKI_FUNC },
 { "MKS", KW_MKS_FUNC },
 { "ALIAS", KW_ALIAS },
 { "SCOPE", KW_SCOPE },
 { "KEYWORD", KW_KEYWORD },
 { "OVERRIDE", KW_OVERRIDE },
 { "NAME", KW_NAME },
 { "RENAME", KW_RENAME },
 { "OUT", KW_OUT },
 { "COM", KW_COM },
 { "PEN", KW_PEN },
 { "STRIG", KW_STRIG },
 { "PCOPY", KW_PCOPY },
 { "PMAP", KW_PMAP },
 { "POS", KW_POS_FUNC },
 { "PRESET", KW_PRESET },
 { "HOME", KW_HOME },
 { "AT", KW_AT },
 { "RESET", KW_RESET },
 { "RMDIR", KW_RMDIR },
 { "RSET", KW_RSET },
 { "STICK", KW_STICK },
 { "UNLOCK", KW_UNLOCK },
 { "USR", KW_USR },
 { "VARPTR", KW_VARPTR },
 { "VIEW", KW_VIEW },
 { "WAIT", KW_WAIT },
 { "WINDOW", KW_WINDOW },
 { "WRITE", KW_WRITE },
 { "MEMMAP", KW_MEMMAP },
 { "SYS", KW_SYS },
 { "EXEC", KW_EXEC },
 { "ERRORLEVEL", KW_ERRORLEVEL },
 { "BYE", KW_BYE },
 // ECMA-116 Enhanced Files Module
 { "SET", KW_SET },
 { "ASK", KW_ASK },
 { "REWRITE", KW_REWRITE },
 { "POINTER", KW_POINTER },
 // Virtual subsystem introspection
 { "VDEV", KW_VDEV },
 { "VMEM", KW_VMEM },
 { "VNET", KW_VNET },
 { "VCON", KW_VCON },
 { "VTERM", KW_VTERM },
 { "VMACH", KW_VMACH },
 { "DEVMAP", KW_DEVMAP },
 { "BIN", KW_BIN_FUNC },
 { "CLOCK", KW_CLOCK },
 { "ALARM", KW_ALARM },
 // Sinclair BASIC specific keywords
 { "LN", KW_LOG_FUNC }, // Sinclair uses LN for LOG
 { "BORDER", KW_BORDER }, // Border color 0-7
 { "PAPER", KW_PAPER }, // Paper (background) 0-7
 { "INK", KW_INK }, // Ink (foreground) 0-7
 { "BRIGHT", KW_BRIGHT }, // Bright mode 0/1
 { "FLASH", KW_FLASH }, // Flash/blink mode 0/1
 { "INVERSE", KW_INVERSE }, // Reverse video 0/1
 { "OVER", KW_OVER }, // Overprint mode 0/1
 { "PLOT", KW_PSET }, // PLOT x,y -> PSET
 { "PAUSE", KW_PAUSE }, // PAUSE n (timer ticks)
 // Sinclair COPY (screen dump) handled via LPRINT only in SINC
 { "COPY", KW_COPY }, // COPY file TO file
 { "IN", KW_INP }, // IN addr -> INP
 { "DELAY", KW_DELAY }, // DELAY ms (NOP busy-wait)
 // SuperBASIC (Sinclair QL) structured keywords
 { "REPEAT", KW_REPEAT }, // REPeat label
 { "ENDREPEAT", KW_ENDREPEAT }, // END REPeat label
 { "ENDFOR", KW_ENDFOR }, // END FOR var
 { "REMAINDER", KW_REMAINDER }, // SELect default
 { "DEFINE", KW_DEFINE }, // DEFine PROCedure/FN
 { "PROCEDURE", KW_PROCEDURE }, // PROCedure type
 { "LOCAL", KW_LOCAL }, // LOCal var decl
 { "ENDDEFINE", KW_ENDDEFINE }, // END DEFine
 // Complex number functions
 { "COMPLEX", KW_COMPLEX },
 { "REAL", KW_REAL_FUNC },
 { "IMAG", KW_IMAG_FUNC },
 { "CONJ", KW_CONJ_FUNC },
 { "CABS", KW_CABS_FUNC },
 // Static analysis
 { "CHECK", KW_CHECK },
 { "VERIFY", KW_VERIFY },
 // File query functions
 { "EXISTS", KW_EXISTS },
 { "FILESIZE", KW_FILESIZE },
 { "FILEMOD", KW_FILEMOD },
 // Formatted I/O
 { "DISPLAY", KW_DISPLAY },
 // Complex math functions
 { "CSQR", KW_CSQR_FUNC },
 { "CEXP", KW_CEXP_FUNC },
 { "CLOG", KW_CLOG_FUNC },
 { "CARG", KW_CARG_FUNC },
 { "CPOW", KW_CPOW_FUNC },
 // Aggregate / rounding math functions (BASIC++)
 { "MIN", KW_MIN_FUNC },
 { "MAX", KW_MAX_FUNC },
 { "AVG", KW_AVG_FUNC },
 { "MED", KW_MED_FUNC },
 { "ROUND", KW_ROUND_FUNC },
 // SUPER BASIC (Tymshare) extended math functions
 { "ASIN", KW_ASIN_FUNC },
 { "ACOS", KW_ACOS_FUNC },
 { "SINH", KW_SINH_FUNC },
 { "COSH", KW_COSH_FUNC },
 { "TANH", KW_TANH_FUNC },
 { "LOG10", KW_LOG10_FUNC },
 { "LOG2", KW_LOG2_FUNC },
 { "LGT", KW_LGT_FUNC },
 { "TIM", KW_TIM_FUNC },
 { "HI", KW_HI_FUNC },
 { "LO", KW_LO_FUNC },
 { "COMP", KW_COMP_FUNC },
 { "PDIF", KW_PDIF_FUNC },
 { "PI", KW_PI_FUNC },
 // SUPER BASIC statement modifiers / keywords
 { "UNLESS", KW_UNLESS },
 { "BY", KW_BY },
 { "SCRATCH", KW_SCRATCH },
 { "UNSAVE", KW_UNSAVE },
 // File management (native, no SHELL)
 { "MOVE", KW_MOVE },
 { "PWD", KW_PWD },
 { "CWD", KW_CWD },
 { "CURDIR", KW_CWD }, // QBasic alias
 { "HOSTNAME", KW_HOSTNAME },
 { "USERNAME", KW_USERNAME },
 { "EXIST", KW_EXIST_FUNC },
 { "FILELEN", KW_FILELEN_FUNC },
 // Event trapping
 { "TRAP", KW_TRAP },
 // SUPER BASIC formatted output
 { "IMAGE", KW_IMAGE },
 // Enhanced debugger commands
 { "DEBUG", KW_DEBUG },
 { "DUMP", KW_DUMP },
 { "BACKTRACE", KW_BACKTRACE },
 { "TRACE", KW_TRACE },
 // Stream I/O primitives (BASIC++ Milestone 11)
 { "SIOREAD", KW_SIOREAD },
 { "SIOREADLN", KW_SIOREADLN },
 { "SIOWRITE", KW_SIOWRITE },
 { "SIOSEEK", KW_SIOSEEK },
 { "SIOFLUSH", KW_SIOFLUSH },
 { "SIOSTATUS", KW_SIOSTATUS },
 { "SIOAVAIL", KW_SIOAVAIL },
 // Block I/O primitives (BASIC++ Milestone 11)
 { "BIOREAD", KW_BIOREAD },
 { "BIOWRITE", KW_BIOWRITE },
 { "BIOSTATUS", KW_BIOSTATUS },
 { "BIOSIZE", KW_BIOSIZE },
 { "BIOCHECKSUM", KW_BIOCHECKSUM },
 { "BIOCOMPARE", KW_BIOCOMPARE },
 { "BIOFILL", KW_BIOFILL },
 { "BIOCOPY", KW_BIOCOPY },
 // Transaction / ATOMIC keywords
 { "ATOMIC", KW_ATOMIC },
 { "TXN", KW_TXN },
 { "COMMIT", KW_COMMIT },
 { "ROLLBACK", KW_ROLLBACK },
 { "TXNSTATUS", KW_TXNSTATUS },
 // Network builtins
 { "NSTATUS", KW_NSTATUS },
 { "NHTTPSTATUS", KW_NHTTPSTATUS },
 { "NEOF", KW_NEOF },
 { "NBYTESWAITING", KW_NBYTESWAITING },
 { "NCONNECTED", KW_NCONNECTED },
 { "NERROR", KW_NERROR },
 { "NJSONQUERY", KW_NJSONQUERY },
 { "NINFO", KW_NINFO },
 { "CONSOLE", KW_CONSOLE },
 { "MOUNT", KW_MOUNT },
 { "UMOUNT", KW_UMOUNT },
 { "MOUNTS", KW_MOUNTS },
 { "VPATH", KW_VPATH },
 { "BIOS", KW_BIOS },
 { "CURSOR", KW_CURSOR },
 { "TICKS", KW_TICKS },
 { "DEMAND", KW_DEMAND },
 { "DAY", KW_DAY },
 { "MONTH", KW_MONTH },
 { "YEAR", KW_YEAR },
 { "HOURS", KW_HOURS },
 { "MINUTES", KW_MINUTES },
 { "SECONDS", KW_SECONDS },
 { "JIFFIES", KW_JIFFIES },
 { NULL, 0, 0 } // sentinel
};
#endif

// --- Dynamic Keyword Registry ---
#define MAX_DYNAMIC_KEYWORDS 512
static KeywordEntry dynamic_keyword_table[MAX_DYNAMIC_KEYWORDS];
static int dynamic_keyword_count = 0;
static int next_custom_keyword_id = KW_CUSTOM_START;

#define KEYWORD_HASH_SIZE 1024
static int keyword_hash_table[KEYWORD_HASH_SIZE];

static unsigned int hash_string_nocase(const char *str, int len) {
    unsigned int hash = 5381;
    int i;
    for (i = 0; i < len; i++) {
        char c = str[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        hash = ((hash << 5) + hash) + (unsigned char)c;
    }
    return hash % KEYWORD_HASH_SIZE;
}

static void add_to_keyword_hash(int index) {
    const char *name = dynamic_keyword_table[index].name;
    unsigned int h = hash_string_nocase(name, (int)strlen(name));
    while (keyword_hash_table[h] != -1) {
        h = (h + 1) % KEYWORD_HASH_SIZE;
    }
    keyword_hash_table[h] = index;
}

void keyword_registry_init(void) {
    int i = 0;
    if (dynamic_keyword_count > 0) return; // Already initialized
    memset(keyword_hash_table, -1, sizeof(keyword_hash_table));
    while (core_keyword_init_table[i].name != NULL) {
        dynamic_keyword_table[i] = core_keyword_init_table[i];
        add_to_keyword_hash(i);
        i++;
    }
    dynamic_keyword_count = i;
}

KeywordId keyword_register_custom(const char *name) {
    int id;
    if (dynamic_keyword_count >= MAX_DYNAMIC_KEYWORDS) return KW_COUNT;
    id = next_custom_keyword_id++;
    dynamic_keyword_table[dynamic_keyword_count].name = plat_strdup(name);
    dynamic_keyword_table[dynamic_keyword_count].id = id;
    
    add_to_keyword_hash(dynamic_keyword_count);
    dynamic_keyword_count++;
    return id;
}

// --- Internal Helpers ---

 // peek_char - Return the character at the current position.
 //
 // Returns '\0' at end of input (safe for all comparisons).
static char peek_char(Lexer *lex)
{
 if (lex->pos >= lex->length) {
 return '\0';
 }
 return lex->source[lex->pos];
}

 // advance_char - Consume the current character and return it.
 //
 // Returns '\0' if already at end of input.
static char advance_char(Lexer *lex)
{
 if (lex->pos >= lex->length) {
 return '\0';
 }
 return lex->source[lex->pos++];
}

 // skip_whitespace - Advance past spaces and tabs.
 //
 // Newlines and other control characters are NOT skipped - they
 // terminate the current line/statement.
static void skip_whitespace(Lexer *lex)
{
 while (lex->pos < lex->length) {
 char c = lex->source[lex->pos];
 if (c == ' ' || c == '\t') {
 lex->pos++;
 } else {
 break;
 }
 }
}

 // to_upper - Convert a character to uppercase.
 //
 // Uses manual comparison rather than toupper() to avoid locale
 // dependencies on some platforms. Only converts a-z.
static char to_upper(char c)
{
 if (c >= 'a' && c <= 'z') {
 return (char)(c - 'a' + 'A');
 }
 return c;
}

// --- Keyword Alias Table ---
 // Runtime-configurable name remapping with scoping.
 // Each slot maps a user-chosen identifier (e.g. "IMPRE")
 // to a built-in KeywordId (e.g. KW_PRINT).
 // Aliases are checked FIRST in match_keyword().
typedef struct {
 char name[MAX_ALIAS_NAME]; // upper-cased alias
 int name_len;
 KeywordId target;
 int active; // 1 = in use
 AliasScope scope;
 char module_name[16]; // for ASCOPE_MODULE
} AliasEntry;

static AliasEntry alias_table[MAX_ALIASES];
static int alias_count = 0;

// --- Operator Alias Table ---
 // Maps alias strings to token types for operators.
typedef struct {
 char name[MAX_OP_ALIAS_NAME];
 int name_len;
 int token_type; // TOK_GT_EQ, TOK_LT_EQ, etc.
 int active;
} OpAliasEntry;

static OpAliasEntry op_alias_table[MAX_OP_ALIASES];
static int op_alias_count = 0;

 // lexer_add_alias - Register with GLOBAL scope (backward compat).
int lexer_add_alias(const char *name, int name_len,
 KeywordId target)
{
 return lexer_add_alias_scoped(name, name_len, target,
 ASCOPE_GLOBAL, NULL);
}

 // lexer_add_alias_scoped - Register or update an alias.
int lexer_add_alias_scoped(const char *name, int name_len,
 KeywordId target, AliasScope scope,
 const char *module_name)
{
 int i;
 char upper[MAX_ALIAS_NAME];

 if (name_len <= 0 || name_len >= MAX_ALIAS_NAME)
 return -1;

 // Upper-case the alias name
 for (i = 0; i < name_len; i++)
 upper[i] = to_upper(name[i]);
 upper[name_len] = '\0';

 // Reject aliasing protected keywords
 if (target == KW_ALIAS || target == KW_SCOPE ||
  target == KW_KEYWORD || target == KW_SECURITY ||
  target == KW_OVERRIDE)
 return -3; // protected keyword

 // Reject alias names that shadow existing keywords
 for (i = 0; i < dynamic_keyword_count; i++) {
 const char *kn = dynamic_keyword_table[i].name;
 int ki = 0, match = 1;
 while (kn[ki] && upper[ki]) {
  char ca = upper[ki];
  char cb = kn[ki];
  if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
  if (ca != cb) { match = 0; break; }
  ki++;
 }
 if (match && kn[ki] == '\0' &&
  upper[ki] == '\0')
  return -2; // conflicts with keyword
 }

 // Check if alias already exists - update it
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].name_len == name_len &&
 memcmp(alias_table[i].name, upper,
 (size_t)name_len) == 0) {
 alias_table[i].target = target;
 alias_table[i].scope = scope;
 return 0;
 }
 }

 // Find an empty slot
 for (i = 0; i < MAX_ALIASES; i++) {
 if (!alias_table[i].active) {
 memcpy(alias_table[i].name, upper,
 (size_t)name_len);
 alias_table[i].name[name_len] = '\0';
 alias_table[i].name_len = name_len;
 alias_table[i].target = target;
 alias_table[i].active = 1;
 alias_table[i].scope = scope;
 alias_table[i].module_name[0] = '\0';
 if (module_name != NULL && scope == ASCOPE_MODULE) {
 int mlen = (int)strlen(module_name);
 if (mlen > 15) mlen = 15;
 memcpy(alias_table[i].module_name,
 module_name, (size_t)mlen);
 alias_table[i].module_name[mlen] = '\0';
 }
 alias_count++;
 return 0;
 }
 }

 return -1; // table full
}

 // lexer_remove_alias - Remove a single alias by name.
int lexer_remove_alias(const char *name, int name_len)
{
 int i;
 char upper[MAX_ALIAS_NAME];

 if (name_len <= 0 || name_len >= MAX_ALIAS_NAME)
 return -1;

 for (i = 0; i < name_len; i++)
 upper[i] = to_upper(name[i]);
 upper[name_len] = '\0';

 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].name_len == name_len &&
 memcmp(alias_table[i].name, upper,
 (size_t)name_len) == 0) {
 alias_table[i].active = 0;
 alias_count--;
 return 0;
 }
 }
 return -1; // not found
}

 // lexer_clear_aliases - Remove all aliases.
void lexer_clear_aliases(void)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++)
 alias_table[i].active = 0;
 alias_count = 0;
}

 // lexer_clear_scope - Remove aliases of a specific scope.
void lexer_clear_scope(AliasScope scope)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].scope == scope) {
 alias_table[i].active = 0;
 alias_count--;
 }
 }
}

 // lexer_clear_module_aliases - Remove aliases for a module.
void lexer_clear_module_aliases(const char *mod_name)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].scope == ASCOPE_MODULE) {
 // Case-insensitive compare
 const char *a = mod_name;
 const char *b = alias_table[i].module_name;
 int match = 1;
 while (*a && *b) {
 char ca = *a, cb = *b;
 if (ca >= 'a' && ca <= 'z') ca = (char)(ca-32);
 if (cb >= 'a' && cb <= 'z') cb = (char)(cb-32);
 if (ca != cb) { match = 0; break; }
 a++; b++;
 }
 if (*a || *b) match = 0;
 if (match) {
 alias_table[i].active = 0;
 alias_count--;
 }
 }
 }
}

 // lexer_alias_count - Return count of active aliases.
int lexer_alias_count(void)
{
 return alias_count;
}

 // lexer_keyword_name - Get canonical name for a keyword.
const char *lexer_keyword_name(KeywordId kw)
{
 int i;
 for (i = 0; i < dynamic_keyword_count; i++) {
 if (dynamic_keyword_table[i].id == kw)
 return dynamic_keyword_table[i].name;
 }
 return "";
}

 // lexer_keyword_needs_dollar - Check if keyword uses $.
int lexer_keyword_needs_dollar(KeywordId kw)
{
    return (kw == KW_LEFT || kw == KW_RIGHT ||
    kw == KW_MID || kw == KW_CHR ||
    kw == KW_STR_FUNC ||
    kw == KW_EDIT || kw == KW_NUM_FUNC ||
    kw == KW_SPACE_FUNC ||
    kw == KW_STRING_FUNC ||
    kw == KW_HEX_FUNC ||
    kw == KW_OCT_FUNC ||
    kw == KW_LCASE || kw == KW_UCASE ||
    kw == KW_TCASE ||
    kw == KW_LTRIM || kw == KW_RTRIM ||
    kw == KW_TRIM ||
    kw == KW_DATE_FUNC ||
    kw == KW_DAY || kw == KW_MONTH || kw == KW_YEAR ||
    kw == KW_TIME_FUNC ||
    kw == KW_INKEY ||
    kw == KW_ENVIRON ||
    kw == KW_MKD_FUNC ||
    kw == KW_MKI_FUNC ||
    kw == KW_MKS_FUNC ||
    kw == KW_INPUT_FUNC ||
    kw == KW_IOCTL_FUNC ||
    kw == KW_SHELL ||
    kw == KW_BIN_FUNC ||
    kw == KW_CLOCK ||
    kw == KW_ALARM ||
    kw == 0 ||
    kw == KW_MEMMAP_FUNC ||
    kw == KW_ALIAS_FUNC ||
    kw == KW_CWD ||
    kw == KW_HOSTNAME ||
    kw == KW_USERNAME ||
    kw == KW_SIOREAD ||
    kw == KW_SIOREADLN ||
    kw == KW_BIOREAD ||
    kw == KW_NJSONQUERY ||
    kw == KW_NINFO ||
    kw == KW_REPLACE ||
    kw == KW_REVERSE ||
    kw == KW_MCASE ||
    kw == KW_ICASE ||
    kw == KW_ONKEY ||
    kw == KW_VPATH_FUNC ||
    kw == KW_HASH ||
    kw == KW_PWD ||
    kw == KW_ERR_STR);
}

 // lexer_find_alias_by_name - Given an alias name,
 // return the original keyword name. NULL if not found.
const char *lexer_find_alias_by_name(const char *name,
 int name_len)
{
 int i;
 char upper[MAX_ALIAS_NAME];

 if (name_len <= 0 || name_len >= MAX_ALIAS_NAME)
 return NULL;

 for (i = 0; i < name_len; i++)
 upper[i] = to_upper(name[i]);
 upper[name_len] = '\0';

 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].name_len == name_len &&
 memcmp(alias_table[i].name, upper,
 (size_t)name_len) == 0) {
 const char *orig = lexer_keyword_name(
 alias_table[i].target);
 int need_d = lexer_keyword_needs_dollar(
 alias_table[i].target);
 // Return "PRINT" or "LEFT$" etc.
 if (need_d) {
 // Build name with $ in static buffer
 static char buf[MAX_ALIAS_NAME + 2];
 int olen = (int)strlen(orig);
 if (olen >= MAX_ALIAS_NAME) olen = MAX_ALIAS_NAME-2;
 memcpy(buf, orig, (size_t)olen);
 buf[olen] = '$';
 buf[olen + 1] = '\0';
 return buf;
 }
 return orig;
 }
 }
 return NULL;
}

 // lexer_find_alias_for_keyword - Given a keyword ID,
 // return the first alias name. NULL if not found.
const char *lexer_find_alias_for_keyword(KeywordId kw)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].target == kw) {
 return alias_table[i].name;
 }
 }
 return NULL;
}

 // lexer_list_aliases - Print all active aliases.
static const char *scope_names[] = {
 "GLOBAL", "PROGRAM", "MODULE", "LANG"
};

void lexer_list_aliases(void)
{
 int i, found = 0;
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active) {
 const char *orig =
 lexer_keyword_name(
 alias_table[i].target);
 int need_d = lexer_keyword_needs_dollar(
 alias_table[i].target);
 printf(" ALIAS %s%s = \"%s\"",
 orig,
 need_d ? "$" : "",
 alias_table[i].name);
 if (alias_table[i].scope != ASCOPE_GLOBAL) {
 printf("  [%s",
 scope_names[alias_table[i].scope]);
 if (alias_table[i].scope == ASCOPE_MODULE &&
 alias_table[i].module_name[0]) {
 printf(": %s",
 alias_table[i].module_name);
 }
 printf("]");
 }
 printf("\n");
 found = 1;
 }
 }
 if (!found)
 printf(" (no aliases defined)\n");
 printf(" %d / %d slots used.\n",
 alias_count, MAX_ALIASES);
}

// --- Operator Alias Functions ---

int lexer_add_op_alias(const char *name, int name_len,
 int token_type)
{
 int i;
 char upper[MAX_OP_ALIAS_NAME];

 if (name_len <= 0 || name_len >= MAX_OP_ALIAS_NAME)
 return -1;

 for (i = 0; i < name_len; i++)
 upper[i] = to_upper(name[i]);
 upper[name_len] = '\0';

 // Update existing
 for (i = 0; i < MAX_OP_ALIASES; i++) {
 if (op_alias_table[i].active &&
 op_alias_table[i].name_len == name_len &&
 memcmp(op_alias_table[i].name, upper,
 (size_t)name_len) == 0) {
 op_alias_table[i].token_type = token_type;
 return 0;
 }
 }

 // New slot
 for (i = 0; i < MAX_OP_ALIASES; i++) {
 if (!op_alias_table[i].active) {
 memcpy(op_alias_table[i].name, upper,
 (size_t)name_len);
 op_alias_table[i].name[name_len] = '\0';
 op_alias_table[i].name_len = name_len;
 op_alias_table[i].token_type = token_type;
 op_alias_table[i].active = 1;
 op_alias_count++;
 return 0;
 }
 }
 return -1;
}

void lexer_clear_op_aliases(void)
{
 int i;
 for (i = 0; i < MAX_OP_ALIASES; i++)
 op_alias_table[i].active = 0;
 op_alias_count = 0;
}

void lexer_list_op_aliases(void)
{
 int i, found = 0;
 for (i = 0; i < MAX_OP_ALIASES; i++) {
 if (op_alias_table[i].active) {
 const char *tname = "?";
 switch (op_alias_table[i].token_type) {
 case TOK_GT_EQ: tname = ">="; break;
 case TOK_LT_EQ: tname = "<="; break;
 case TOK_NOT_EQ: tname = "<>"; break;
 case TOK_EQUALS: tname = "="; break;
 case TOK_PLUS: tname = "+"; break;
 case TOK_MINUS: tname = "-"; break;
 case TOK_STAR: tname = "*"; break;
 case TOK_SLASH: tname = "/"; break;
 }
 printf(" ALIAS \"%s\" = \"%s\"\n",
 tname,
 op_alias_table[i].name);
 found = 1;
 }
 }
 if (!found)
 printf(" (no operator aliases)\n");
}

int lexer_op_alias_count(void)
{
 return op_alias_count;
}

// --- ALIAS file I/O ---

int lexer_alias_save(const char *filename)
{
 FILE *fp;
 int i, count = 0;

 fp = fopen(filename, "w");
 if (fp == NULL) return -1;

 fprintf(fp, "# BASIC++ Alias File\n");
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active) {
 const char *orig =
 lexer_keyword_name(alias_table[i].target);
 int need_d = lexer_keyword_needs_dollar(
 alias_table[i].target);
 fprintf(fp, "%s%s = %s\n",
 orig,
 need_d ? "$" : "",
 alias_table[i].name);
 count++;
 }
 }
 fclose(fp);
 return count;
}

int lexer_alias_load(const char *filename)
{
 FILE *fp;
 char line[256];
 int count = 0;

 fp = fopen(filename, "r");
 if (fp == NULL) return -1;

 while (fgets(line, (int)sizeof(line), fp) != NULL) {
 char kw_name[64], alias_name[64];
 int ki, kw_len;
 char *eq;
 KeywordId found_kw = KW_COUNT;

 // Skip comments and blanks
 if (line[0] == '#' || line[0] == '\'' ||
 line[0] == '\n' || line[0] == '\r')
 continue;

 // Parse: KEYWORD = ALIASNAME
 eq = strchr(line, '=');
 if (eq == NULL) continue;

 // Extract keyword (left of =)
 kw_len = 0;
 {
 char *p = line;
 while (p < eq && (*p == ' ' || *p == '\t')) p++;
 while (p < eq && kw_len < 63 &&
 *p != ' ' && *p != '\t' && *p != '=') {
 kw_name[kw_len++] = to_upper(*p);
 p++;
 }
 }
 kw_name[kw_len] = '\0';

  // Strip trailing $ from keyword name for lookup
  {
  int lookup_len = kw_len;
  if (lookup_len > 0 && kw_name[lookup_len-1] == '$')
  lookup_len--;
  for (ki = 0; ki < dynamic_keyword_count; ki++) {
  int tlen = (int)strlen(dynamic_keyword_table[ki].name);
  if (tlen == lookup_len) {
  int j, m = 1;
  for (j = 0; j < tlen; j++) {
  if (to_upper(dynamic_keyword_table[ki].name[j]) !=
  kw_name[j]) {
  m = 0; break;
  }
  }
  if (m) {
  found_kw = dynamic_keyword_table[ki].id;
  break;
  }
  }
  }
  }

 if (found_kw == KW_COUNT) continue;

 // Extract alias name (right of =)
 {
 int alen = 0;
 char *p = eq + 1;
 while (*p == ' ' || *p == '\t') p++;
 while (alen < 63 && *p && *p != '\n' &&
 *p != '\r' && *p != ' ' && *p != '\t') {
 alias_name[alen++] = *p;
 p++;
 }
 alias_name[alen] = '\0';

 if (alen > 0 &&
 lexer_add_alias(alias_name, alen,
 found_kw) == 0) {
 count++;
 }
 }
 }

 fclose(fp);
 return count;
}

 // match_keyword - Try to match an identifier against the
 // alias table first, then the keyword table.
 //
 // Compares the identifier string (starting at 'start',
 // length 'len') case-insensitively.
 //
 // Returns the KeywordId if matched, KW_COUNT if not.
static KeywordId match_keyword(const char *start, int len)
{
 int i;

 // Check alias table first (aliases shadow builtins)
 if (alias_count > 0) {
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].name_len == len) {
 int j, matched = 1;
 for (j = 0; j < len; j++) {
 if (to_upper(start[j]) !=
 alias_table[i].name[j]) {
 matched = 0;
 break;
 }
 }
 if (matched)
 return alias_table[i].target;
 }
 }
 }

 // Then check built-in keyword table via hash map
 unsigned int h = hash_string_nocase(start, len);
 while (keyword_hash_table[h] != -1) {
     int idx = keyword_hash_table[h];
     const char *kw = dynamic_keyword_table[idx].name;
     int kw_len = (int)strlen(kw);
     if (kw_len == len) {
         int j, matched = 1;
         for (j = 0; j < len; j++) {
             if (to_upper(start[j]) != kw[j]) {
                 matched = 0;
                 break;
             }
         }
         if (matched) {
             return dynamic_keyword_table[idx].id;
         }
     }
     h = (h + 1) % KEYWORD_HASH_SIZE;
 }

 return KW_COUNT; // not a keyword
}

 // match_keyword_prefix - Greedy keyword extraction (statement keywords only).
 //
 // Try progressively shorter prefixes to find the longest
 // STATEMENT-STARTING keyword. Only keywords that can begin
 // a statement are matched: FOR, NEXT, IF, THEN, GOTO, GOSUB,
 // PRINT, REM, ON, LET, DIM, RETURN, END, STOP, ELSE, etc.
 //
 // Function keywords (LEFT, RIGHT, MID, KEY, TAB, etc.) are
 // NOT matched, so variable names like KEYWORD$ work correctly.
 //
 // Returns KW_COUNT if no statement keyword prefix found.
 // is_embeddable_keyword - Keywords commonly embedded in GW-BASIC.
 //
 // In GW-BASIC, identifiers can contain keywords without spaces:
 //   FORI=1TO10   -> FOR I = 1 TO 10
 //   NEXTI        -> NEXT I
 //   GOTO100      -> GOTO 100
 //   GOSUB2000    -> GOSUB 2000
 //   THENPRINT    -> THEN PRINT
 //
 // Only keywords that commonly appear this way should be
 // extractable as prefixes. Short keywords like GET, PUT, END,
 // DIM, LET etc. cause false positives in identifiers like
 // GetCount, PutData, EndLoop, LetterCount, etc.
static int is_embeddable_keyword(KeywordId kw)
{
 switch (kw) {
 case KW_FOR: case KW_NEXT: case KW_GOTO: case KW_GOSUB:
 case KW_THEN: case KW_ELSE: case KW_PRINT: case KW_REM:
 case KW_RETURN: case KW_STOP: case KW_INPUT: case KW_READ:
 case KW_RESTORE: case KW_DATA: case KW_POKE: case KW_CALL:
 case KW_RANDOMIZE: case KW_LOCATE: case KW_COLOR:
  return 1;
 default:
  return 0;
 }
}

static KeywordId match_keyword_prefix(const char *start, int len,
 int *match_len)
{
 int try_len;

 // Try longest prefix first, down to 3 characters.
  // Minimum of 3 prevents 2-char keywords (DO, IF, ON,
  // TO) from being extracted from identifiers like
  // Doubler, IFflag, etc. FOR/NEXT/GOTO etc. (3+ chars)
  // still work correctly. 
 for (try_len = len - 1; try_len >= 3; try_len--) {
  KeywordId kw = match_keyword(start, try_len);
   if (kw != KW_COUNT && is_embeddable_keyword(kw)) {
   *match_len = try_len;
   return kw;
  }
 }
 return KW_COUNT;
}

// --- Lexer Public API ---

 // lexer_init - Initialize the lexer on a source line.
 //
 // Sets up all fields and primes the first token by calling
 // lexer_next(). After this call, lex->current contains the
  // first token of the source line.
static void scan_next_raw_token_internal(Lexer *lex);

static void scan_next_raw_token(Lexer *lex, Token *tok)
{
 Token old = lex->current;
 scan_next_raw_token_internal(lex);
 *tok = lex->current;
 lex->current = old;
}

void lexer_init(Lexer *lex, const char *source)
{
 lex->source = source;
 lex->pos = 0;
 lex->length = (int)strlen(source);
 lex->lookahead_count = 0;
 lex->lookahead_read_idx = 0;
 lex->lookahead_write_idx = 0;

 // Clear current token
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;

 // Prime the first token
 lexer_next(lex);
}

static int is_in_sub_decl(Lexer *lex, int start) {
    int p = start - 1;
    while (p >= 0 && (lex->source[p] == ' ' || lex->source[p] == '\t')) p--;
    if (p >= 2) {
        if (to_upper(lex->source[p]) == 'B' && to_upper(lex->source[p-1]) == 'U' && to_upper(lex->source[p-2]) == 'S') {
            if (p-3 < 0 || lex->source[p-3] == ' ' || lex->source[p-3] == '\t' || lex->source[p-3] == ':') return 1;
        }
    }
    if (p >= 7) {
        if (to_upper(lex->source[p]) == 'N' && to_upper(lex->source[p-1]) == 'O' && to_upper(lex->source[p-2]) == 'I' &&
            to_upper(lex->source[p-3]) == 'T' && to_upper(lex->source[p-4]) == 'C' && to_upper(lex->source[p-5]) == 'N' &&
            to_upper(lex->source[p-6]) == 'U' && to_upper(lex->source[p-7]) == 'F') {
            if (p-8 < 0 || lex->source[p-8] == ' ' || lex->source[p-8] == '\t' || lex->source[p-8] == ':') return 1;
        }
    }
    return 0;
}

static void scan_next_raw_token_internal(Lexer *lex)
{
 char c;
 int start_pos;

 skip_whitespace(lex);
 start_pos = lex->pos;
 lex->current.pos = start_pos;

 // Check for end of input
 if (lex->pos >= lex->length) {
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 return;
 }

 c = peek_char(lex);

 // ----- &H hex, &O octal, &B binary literals -----
 //
 // GW-BASIC/QBasic syntax:
 // &Hffff - hexadecimal (case insensitive)
 // &O377 - octal
 // &377 - octal (& without O)
 // &B1010 - binary (BASIC++ extension)
 if (c == '&') {
 long hval = 0;
 int found = 0;
 lex->pos++; // consume '&'

 if (lex->pos < lex->length &&
 (lex->source[lex->pos] == 'H' ||
 lex->source[lex->pos] == 'h')) {
 // &H hex literal
 lex->pos++; // consume 'H'
 while (lex->pos < lex->length) {
 char hc = lex->source[lex->pos];
 if (hc >= '0' && hc <= '9') {
 hval = hval * 16 + (hc - '0');
 } else if (hc >= 'A' && hc <= 'F') {
 hval = hval * 16 + (hc - 'A' + 10);
 } else if (hc >= 'a' && hc <= 'f') {
 hval = hval * 16 + (hc - 'a' + 10);
 } else {
 break;
 }
 lex->pos++;
 found = 1;
 }
 } else if (lex->pos < lex->length &&
 (lex->source[lex->pos] == 'O' ||
 lex->source[lex->pos] == 'o')) {
 // &O octal literal
 lex->pos++; // consume 'O'
 while (lex->pos < lex->length &&
 lex->source[lex->pos] >= '0' &&
 lex->source[lex->pos] <= '7') {
 hval = hval * 8 +
 (lex->source[lex->pos] - '0');
 lex->pos++;
 found = 1;
 }
 } else if (lex->pos < lex->length &&
 (lex->source[lex->pos] == 'B' ||
 lex->source[lex->pos] == 'b')) {
 // &B binary literal
 lex->pos++; // consume 'B'
 while (lex->pos < lex->length &&
 (lex->source[lex->pos] == '0' ||
 lex->source[lex->pos] == '1')) {
 hval = hval * 2 +
 (lex->source[lex->pos] - '0');
 lex->pos++;
 found = 1;
 }
 } else if (lex->pos < lex->length &&
 lex->source[lex->pos] >= '0' &&
 lex->source[lex->pos] <= '7') {
 // &nnn - bare octal (no O prefix)
 while (lex->pos < lex->length &&
 lex->source[lex->pos] >= '0' &&
 lex->source[lex->pos] <= '7') {
 hval = hval * 8 +
 (lex->source[lex->pos] - '0');
 lex->pos++;
 found = 1;
 }
 }

 if (!found) hval = 0;

 lex->current.type = TOK_NUMBER;
 lex->current.value.num_value = hval;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 return;
  }

  // ----- C-style Hex/Octal/Binary literals (0x, 0o, 0b) -----
  if (c == '0' && lex->pos + 1 < lex->length) {
      char next = lex->source[lex->pos + 1];
      if (next == 'x' || next == 'X' || next == 'o' || next == 'O' || next == 'b' || next == 'B') {
          int base = 10;
          if (next == 'x' || next == 'X') base = 16;
          else if (next == 'o' || next == 'O') base = 8;
          else if (next == 'b' || next == 'B') base = 2;

          lex->pos += 2; // consume '0' and the prefix character
          long hval = 0;
          int found = 0;

          while (lex->pos < lex->length) {
              char hc = lex->source[lex->pos];
              int val = -1;
              if (hc >= '0' && hc <= '9') val = hc - '0';
              else if (hc >= 'A' && hc <= 'F') val = hc - 'A' + 10;
              else if (hc >= 'a' && hc <= 'f') val = hc - 'a' + 10;

              if (val < 0 || val >= base) {
                  break;
              }
              hval = hval * base + val;
              lex->pos++;
              found = 1;
          }

          if (!found) hval = 0;

          lex->current.type = TOK_NUMBER;
          lex->current.value.num_value = hval;
          lex->current.str_start = NULL;
          lex->current.str_length = 0;
          return;
      }
  }

  // ----- Numeric literals (integer or float) -----
  if (isdigit((unsigned char)c) || (c == '.' &&
 lex->pos + 1 < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos + 1]))) {
 long ivalue = 0;
 int has_dot = 0;
 int int_start = lex->pos;

 // Scan integer part
 while (lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 ivalue = ivalue * 10 + (lex->source[lex->pos] - '0');
 lex->pos++;
 }

 // Check for decimal point -> float literal
 if (lex->pos < lex->length && lex->source[lex->pos] == '.') {
 has_dot = 1;
 }

 // Check for E/e exponent suffix (scientific
 // notation without decimal point: 1E10, 5E-3).
 // ECMA-55 s5.2 requires E-notation support.
 if (!has_dot && lex->pos < lex->length) {
 char ec = lex->source[lex->pos];
 if (ec == 'E' || ec == 'e') {
 has_dot = 1; // force float path
 }
 }

 if (has_dot) {
 // Re-parse entire number as double using strtod
 char *endptr;
 double fval = strtod(lex->source + int_start, &endptr);
 lex->pos = (int)(endptr - lex->source);

 lex->current.type = TOK_FLOAT_LIT;
 lex->current.value.fval = fval;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 } else {
 lex->current.type = TOK_NUMBER;
 lex->current.value.num_value = ivalue;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 }

 // Check for imaginary suffix (i/I)
 if (lex->pos < lex->length &&
  (lex->source[lex->pos] == 'i' ||
  lex->source[lex->pos] == 'I') &&
  // Must NOT be followed by alnum (not a var)
  (lex->pos + 1 >= lex->length ||
  !isalpha((unsigned char)
  lex->source[lex->pos + 1]))) {
  lex->pos++; // consume 'i'
  // Convert to imaginary token
  if (lex->current.type == TOK_NUMBER) {
  lex->current.value.fval =
   (double)lex->current.value.num_value;
  }
  lex->current.type = TOK_IMAGINARY;
 }

  // GW-BASIC type suffixes on numeric literals:
  //  ! = single-precision, # = double-precision,
  //  % = integer, & = long integer.
  // We consume and discard them since our type
  // system handles precision internally.
 if (lex->pos < lex->length) {
  char ts = lex->source[lex->pos];
  if (ts == '!' || ts == '#' || ts == '%' ||
  ts == '&') {
  lex->pos++; // consume type suffix
  }
 }
 return;
 }

 // ----- String literals -----
 if (c == '"') {
 int start;
 lex->pos++; // skip opening quote
 start = lex->pos;

 while (lex->pos < lex->length && lex->source[lex->pos] != '"') {
 lex->pos++;
 }

 lex->current.type = TOK_STRING;
 lex->current.value.num_value = 0;
 lex->current.str_start = lex->source + start;
 lex->current.str_length = lex->pos - start;

 if (lex->pos < lex->length) {
 lex->pos++; // skip closing quote
 }
 // If no closing quote, we still produce the token with
 // whatever text we found. PATB behavior: unterminated
 // strings are not a fatal error. 
 return;
 }

 // ----- Identifiers and keywords -----
 if (isalpha((unsigned char)c) || c == '_') {
  int start = lex->pos;
  int len;
  KeywordId kw;

  while (lex->pos < lex->length &&
  (isalpha((unsigned char)lex->source[lex->pos]) ||
  isdigit((unsigned char)lex->source[lex->pos]) ||
  lex->source[lex->pos] == '_')) {
  lex->pos++;
  }
 len = lex->pos - start;

  // If the identifier doesn't match a keyword, and it
  // ends with digits (e.g. "X1"), back off the digits
  // so the lexer can parse them as a separate number.
  // This preserves traditional BASIC variable naming
  // (A1 = variable A followed by 1) while allowing
  // keywords like LOG10 and LOG2 to work.

  // Try keyword match with full alphanumeric token
  kw = match_keyword(lex->source + start, len);

  if (kw == KW_COUNT) {
      int alpha_end = lex->pos;
      while (alpha_end > start && isdigit((unsigned char)lex->source[alpha_end - 1])) {
          alpha_end--;
      }
      if (alpha_end > start && alpha_end < lex->pos) {
          int try_len = alpha_end - start;
          KeywordId stripped_kw = match_keyword(lex->source + start, try_len);
          if (stripped_kw != KW_COUNT) {
              lex->pos = alpha_end;
              len = try_len;
              kw = stripped_kw;
          }
      }
  }

  // GW-BASIC greedy keyword extraction:
  // If still no keyword match, try to find the longest
  // keyword prefix. E.g., FORI -> FOR, NEXTI -> NEXT.
  // This is handled in the multi-char fallback below.

 if (kw != KW_COUNT) {
 lex->current.type = TOK_KEYWORD;
 lex->current.value.keyword = kw;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;

   // GW-BASIC variable name collision check:
   // If a keyword is followed by '$' (type suffix) and it's
   // NOT a known string function keyword, then in extended-var
   // mode this is actually a variable name (e.g., KEYWORD$,
   // WORDIN$, WORDOUT$). Reinterpret as TOK_NAMED_VAR.
  if (lex->pos < lex->length &&
  lex->source[lex->pos] == '$' &&
  1) {
  // Check if this is NOT a string function keyword
  if (!(kw == KW_LEFT || kw == KW_RIGHT ||
  kw == KW_MID || kw == KW_CHR ||
  kw == KW_STR_FUNC || kw == KW_SPACE_FUNC ||
  kw == KW_STRING_FUNC || kw == KW_HEX_FUNC ||
  kw == KW_OCT_FUNC || kw == KW_LCASE ||
  kw == KW_UCASE || kw == KW_TCASE ||
  kw == KW_LTRIM || kw == KW_RTRIM ||
  kw == KW_TRIM || kw == KW_DATE_FUNC || kw == KW_DATE || kw == KW_DAY || kw == KW_MONTH || kw == KW_YEAR ||
  kw == KW_TIME_FUNC || kw == KW_TIME || kw == KW_INKEY ||
  kw == KW_ENVIRON || kw == KW_MKD_FUNC ||
  kw == KW_MKI_FUNC || kw == KW_MKS_FUNC ||
   kw == KW_SHELL || kw == KW_BIN_FUNC ||
    kw == KW_INPUT || kw == KW_IOCTL ||
    kw == KW_EDIT || kw == KW_NUM ||
    kw == KW_VARPTR || kw == KW_ERR_VAR || kw == KW_VM ||
    kw == KW_MEMMAP || kw == KW_ALIAS ||
   kw == KW_CLOCK || kw == KW_ALARM ||
   kw == KW_CWD ||
  kw == KW_HOSTNAME ||
  kw == KW_USERNAME ||
  kw == KW_PWD ||
  kw == KW_SIOREAD ||
  kw == KW_SIOREADLN || kw == KW_BIOREAD ||
  kw == KW_NJSONQUERY || kw == KW_NINFO ||
  kw == KW_REPLACE || kw == KW_REVERSE ||
  kw == KW_MCASE || kw == KW_ICASE ||
  kw == KW_ONKEY || kw == KW_VPATH || kw == KW_VPATH_FUNC ||
  kw == KW_HASH || kw == KW_PRETRIEVE)) {
   // Not a string function -- this is a variable name
   lex->pos++; // consume $
   lex->current.type = TOK_NAMED_VAR;
   lex->current.value.num_value = 0;
   lex->current.str_start = lex->source + start;
   lex->current.str_length = len + 1; // include $
   return;
  }
  }

 // String function keywords (LEFT, RIGHT,
 // MID, CHR, STR) may be followed by '$'.
 // Consume it as part of the keyword so
 // LEFT$( tokenizes as KW_LEFT + TOK_LPAREN
 // instead of KW_LEFT + unknown + TOK_LPAREN.
 if (lex->pos < lex->length &&
 lex->source[lex->pos] == '$') {
 if (kw == KW_LEFT || kw == KW_RIGHT ||
 kw == KW_MID || kw == KW_CHR ||
 kw == KW_STR_FUNC ||
 kw == KW_SPACE_FUNC ||
 kw == KW_STRING_FUNC ||
 kw == KW_HEX_FUNC ||
 kw == KW_OCT_FUNC ||
 kw == KW_LCASE ||
 kw == KW_UCASE ||
 kw == KW_TCASE ||
 kw == KW_LTRIM ||
 kw == KW_RTRIM ||
 kw == KW_TRIM ||
 kw == KW_DATE_FUNC || kw == KW_DATE || kw == KW_DAY || kw == KW_MONTH || kw == KW_YEAR ||
 kw == KW_TIME_FUNC || kw == KW_TIME ||
 kw == KW_INKEY ||
 kw == KW_ENVIRON ||
 kw == KW_MKD_FUNC ||
 kw == KW_MKI_FUNC ||
  kw == KW_MKS_FUNC ||
  kw == KW_SHELL ||
  kw == KW_BIN_FUNC ||
  kw == KW_ERR_VAR ||
  kw == KW_CLOCK ||
  kw == KW_ALARM ||
 kw == KW_CWD ||
 kw == KW_HOSTNAME ||
 kw == KW_USERNAME ||
 kw == KW_PWD ||
 kw == KW_SIOREAD ||
 kw == KW_SIOREADLN ||
 kw == KW_BIOREAD ||
 kw == KW_EDIT ||
 kw == KW_NUM ||
 kw == KW_PRETRIEVE ||
 kw == KW_NJSONQUERY ||
 kw == KW_NINFO ||
 kw == KW_REPLACE ||
 kw == KW_REVERSE ||
 kw == KW_MCASE ||
 kw == KW_ICASE ||
 kw == KW_ONKEY ||
 kw == KW_VPATH ||
 kw == KW_VPATH_FUNC ||
 kw == KW_HASH) {
 lex->pos++; // consume '$'
 }
  // DATE$ -> KW_DATE_FUNC
  if (kw == KW_DATE) {
      kw = KW_DATE_FUNC;
      lex->current.value.keyword = kw;
  }
  // DAY$ -> KW_DAY_FUNC
  if (kw == KW_DAY) {
      kw = KW_DAY_FUNC;
      lex->current.value.keyword = kw;
  }
  // MONTH$ -> KW_MONTH_FUNC
  if (kw == KW_MONTH) {
      kw = KW_MONTH_FUNC;
      lex->current.value.keyword = kw;
  }
  // YEAR$ -> KW_YEAR_FUNC
  if (kw == KW_YEAR) {
      kw = KW_YEAR_FUNC;
      lex->current.value.keyword = kw;
  }
  // CLOCK$ -> KW_CLOCK_FUNC
  if (kw == KW_CLOCK) { kw = KW_CLOCK_FUNC; lex->current.value.keyword = kw; }
  // ALARM$ -> KW_ALARM_FUNC
  if (kw == KW_ALARM) { kw = KW_ALARM_FUNC; lex->current.value.keyword = kw; }
  // CWD$ -> KW_CWD_FUNC
  if (kw == KW_CWD) { kw = KW_CWD_FUNC; lex->current.value.keyword = kw; }
  // HOSTNAME$ -> KW_HOSTNAME_FUNC
  if (kw == KW_HOSTNAME) { kw = KW_HOSTNAME_FUNC; lex->current.value.keyword = kw; }
  // USERNAME$ -> KW_USERNAME_FUNC
  if (kw == KW_USERNAME) { kw = KW_USERNAME_FUNC; lex->current.value.keyword = kw; }
  // TIME$ -> KW_TIME_FUNC
  if (kw == KW_TIME) {
      kw = KW_TIME_FUNC;
      lex->current.value.keyword = kw;
  }
  // EDIT$ -> KW_EDIT_FUNC
  if (kw == KW_EDIT) {
      kw = KW_EDIT_FUNC;
      lex->current.value.keyword = kw;
  }
  // NUM$ -> KW_NUM_FUNC
  if (kw == KW_NUM) {
      kw = KW_NUM_FUNC;
      lex->current.value.keyword = kw;
  }
  // PRETRIEVE$ -> KW_PRETRIEVE_STR
  if (kw == KW_PRETRIEVE) {
      kw = KW_PRETRIEVE_STR;
      lex->current.value.keyword = kw;
  }
 // INPUT$ -> KW_INPUT_FUNC
 if (kw == KW_INPUT) {
 lex->pos++;
 kw = KW_INPUT_FUNC;
 lex->current.value.keyword = kw;
 }
 // IOCTL$ -> KW_IOCTL_FUNC
 if (kw == KW_IOCTL) {
 lex->pos++;
 kw = KW_IOCTL_FUNC;
 lex->current.value.keyword = kw;
 }
 // VARPTR$ -> KW_VARPTR_STR
 if (kw == KW_VARPTR) {
 lex->pos++;
 kw = KW_VARPTR_STR;
 lex->current.value.keyword = kw;
 }
 // DIALECT$ -> 0
 if (kw == KW_VM) {
 lex->pos++;
 kw = 0;
 lex->current.value.keyword = kw;
 }
 // MEMMAP$ -> KW_MEMMAP_FUNC
 if (kw == KW_MEMMAP) {
 lex->pos++;
 kw = KW_MEMMAP_FUNC;
 lex->current.value.keyword = kw;
 }
 // ALIAS$ -> KW_ALIAS_FUNC
 if (kw == KW_ALIAS) {
 lex->pos++;
 kw = KW_ALIAS_FUNC;
 lex->current.value.keyword = kw;
 }
 // VPATH$ -> KW_VPATH_FUNC
 if (kw == KW_VPATH) {
 lex->pos++;
 kw = KW_VPATH_FUNC;
 lex->current.value.keyword = kw;
 }
  // ERR$ -> KW_ERR_STR
  if (kw == KW_ERR_VAR) {
  kw = KW_ERR_STR;
  lex->current.value.keyword = kw;
  }
  }
 } else if (len == 1) {
 // Single letter that's not a keyword.
 // In extended-vars mode, check if digits follow (e.g., X1).
 // If so, scan them and emit TOK_NAMED_VAR.
 if (1 &&
 lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 while (lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 lex->pos++;
 }
 len = lex->pos - start;
    // Check for trailing type suffix (e.g. X1$, A%, B!)
   if (lex->pos < lex->length) {
    char t = lex->source[lex->pos];
    if (t == '$' || t == '%' || t == '!' || t == '#' || t == '&' || t == '~') {
     lex->pos++; // consume suffix
     len++;
    }
   }
 lex->current.type = TOK_NAMED_VAR;
 lex->current.value.num_value = 0;
 lex->current.str_start = lex->source + start;
 lex->current.str_length = len;
   } else if (lex->pos < lex->length &&
    lex->source[lex->pos] == '$') {
    // String variable: A$ through Z$
    lex->pos++; // consume $
    lex->current.type = TOK_STRING_VAR;
    lex->current.value.var_name = to_upper(lex->source[start]);
    lex->current.str_start = lex->source + start;
    lex->current.str_length = 2;
  } else if (lex->pos < lex->length &&
    (lex->source[lex->pos] == '%' ||
     lex->source[lex->pos] == '!' || lex->source[lex->pos] == '#' ||
     lex->source[lex->pos] == '&' || lex->source[lex->pos] == '~')) {
    // Typed single-char var: A%, B!, C# etc.
    lex->pos++; // consume suffix
    lex->current.type = TOK_NAMED_VAR;
    lex->current.value.num_value = 0;
    lex->current.str_start = lex->source + start;
    lex->current.str_length = 2;
 } else {
  // Standard single-letter variable A-Z
  lex->current.type = TOK_VARIABLE;
  lex->current.value.var_name = to_upper(lex->source[start]);
  lex->current.str_start = lex->source + start;
  lex->current.str_length = 1;
 }
 } else {
   // Multi-character identifier that's not a keyword.
   //
   // GW-BASIC greedy extraction: try to find the longest
   // keyword PREFIX within this identifier. For example:
   //   FORI     -> FOR (keyword) + I (variable)
   //   NEXTI    -> NEXT (keyword) + I (variable)
   //   REMARKABLE -> REM (keyword) + rest (comment)
   //   THENK3   -> THEN (keyword) + K3 (variable)
   //
   // If no keyword prefix is found and the dialect supports
   // extended variables, treat as a named variable.
   // Otherwise, rewind to single letter and treat as A-Z.
   {
    int prefix_len = 0;
    KeywordId prefix_kw = KW_COUNT;
    if (3 != 3 &&
        3 != 2 &&
        !is_in_sub_decl(lex, start)) {
        prefix_kw = match_keyword_prefix(lex->source + start, len, &prefix_len);
    }
   if (prefix_kw != KW_COUNT) {
   // Found a keyword prefix - rewind to just after it
   lex->pos = start + prefix_len;
   lex->current.type = TOK_KEYWORD;
   lex->current.value.keyword = prefix_kw;
   lex->current.str_start = NULL;
   lex->current.str_length = 0;

   // Handle $ suffix for string function keywords
   if (lex->pos < lex->length &&
   lex->source[lex->pos] == '$') {
    if (prefix_kw == KW_LEFT ||
     prefix_kw == KW_RIGHT ||
     prefix_kw == KW_MID ||
     prefix_kw == KW_CHR ||
     prefix_kw == KW_STR_FUNC ||
     prefix_kw == KW_SPACE_FUNC ||
     prefix_kw == KW_STRING_FUNC ||
     prefix_kw == KW_HEX_FUNC ||
     prefix_kw == KW_OCT_FUNC ||
     prefix_kw == KW_INKEY ||
     prefix_kw == KW_HOSTNAME ||
     prefix_kw == KW_USERNAME ||
     prefix_kw == KW_PWD ||
     prefix_kw == KW_CWD ||
     prefix_kw == KW_INPUT ||
     prefix_kw == KW_PRETRIEVE) {
     lex->pos++;
     if (prefix_kw == KW_INPUT) {
     lex->current.value.keyword =
      KW_INPUT_FUNC;
     } else if (prefix_kw == KW_PRETRIEVE) {
     lex->current.value.keyword =
      KW_PRETRIEVE_STR;
     }
    }
   }
  } else if (1) {
    // No keyword prefix - treat as named variable.
     // len is already correct from the initial scan +
     // back-off. Do NOT re-extend with trailing digits
     // since the initial scan already collected them
     // and the back-off already stripped them.

      // GW-BASIC keyword suffix check:
      // If the identifier ends with a statement keyword
      // (e.g., Z4GOTO, Z5THEN), split: named var + keyword.
      // Try suffixes from longest to shortest.
     {
     int sfx;
     for (sfx = len - 2; sfx >= 1; sfx--) {
      KeywordId skw = match_keyword(
       lex->source + start + sfx, len - sfx);
      if (skw != KW_COUNT && is_embeddable_keyword(skw)) {
       len = sfx;
       lex->pos = start + sfx;
       break;
      }
     }
     }

     // Check for trailing type suffix
     if (lex->pos < lex->length) {
     char t = lex->source[lex->pos];
     if (t == '$' || t == '%' || t == '!' ||
      t == '#' || t == '&' || t == '~') {
      lex->pos++;
      len++;
     }
     }

     lex->current.type = TOK_NAMED_VAR;
     lex->current.value.num_value = 0;
     lex->current.str_start = lex->source + start;
     lex->current.str_length = len;
   } else {
    lex->pos = start + 1;
    lex->current.type = TOK_VARIABLE;
    lex->current.value.var_name =
    to_upper(lex->source[start]);
    lex->current.str_start = lex->source + start;
    lex->current.str_length = 1;
   }
  }
 }
 return;
 }

 // ----- Operators and delimiters -----
 advance_char(lex); // consume the character

 switch (c) {
 case '+':
 lex->current.type = TOK_PLUS;
 break;
 case '-':
 lex->current.type = TOK_MINUS;
 break;
 case '*':
 lex->current.type = TOK_STAR;
 break;
 case '/':
 lex->current.type = TOK_SLASH;
 break;
 case '=':
 lex->current.type = TOK_EQUALS;
 break;
 case '<':
  // Check for <= or <> (allow spaces: < = , < >)
  {
  int save_pos = lex->pos;
  // Skip spaces after <
  while (lex->pos < lex->length &&
   lex->source[lex->pos] == ' ')
   lex->pos++;
  if (lex->pos < lex->length &&
   lex->source[lex->pos] == '=') {
   lex->pos++;
   lex->current.type = TOK_LT_EQ;
  } else if (lex->pos < lex->length &&
   lex->source[lex->pos] == '>') {
   lex->pos++;
   lex->current.type = TOK_NOT_EQ;
  } else {
   lex->pos = save_pos; // restore
   lex->current.type = TOK_LT;
  }
  }
 break;
  case '>':
  // Check for >= then >> (allow spaces: > = , > >)
  {
  int save_pos = lex->pos;
  // Skip spaces after >
  while (lex->pos < lex->length &&
   lex->source[lex->pos] == ' ')
   lex->pos++;
  if (lex->pos < lex->length &&
   lex->source[lex->pos] == '=') {
   lex->pos++;
   lex->current.type = TOK_GT_EQ;
  } else if (lex->pos < lex->length &&
   lex->source[lex->pos] == '>') {
   lex->pos++;
   lex->current.type = TOK_APPEND;
  } else {
   lex->pos = save_pos; // restore
   lex->current.type = TOK_GT;
  }
  }
  break;
 case '|':
 lex->current.type = TOK_PIPE;
 break;
 case '(':
 lex->current.type = TOK_LPAREN;
 break;
 case ')':
 lex->current.type = TOK_RPAREN;
 break;
 case ',':
 lex->current.type = TOK_COMMA;
 break;
 case ';':
 lex->current.type = TOK_SEMICOLON;
 break;
 case ':':
 lex->current.type = TOK_COLON;
 break;
 case '#':
 lex->current.type = TOK_HASH;
 break;
 case '@':
 lex->current.type = TOK_AT;
 break;
 case '^':
 lex->current.type = TOK_CARET;
 break;
 case '\\':
 lex->current.type = TOK_BACKSLASH;
 break;
 case '[':
 lex->current.type = TOK_LBRACKET;
 break;
 case ']':
 lex->current.type = TOK_RBRACKET;
 break;
 case '.':
 lex->current.type = TOK_DOT;
 break;
 case '\'':
 // Apostrophe = REM shorthand.
 // Skip rest of line and produce EOL.
 lex->pos = lex->length;
 lex->current.type = TOK_CR;
 break;
 case '\r':
 case '\n':
 lex->current.type = TOK_CR;
 break;
 default:
 // Unknown character. In Tiny BASIC, this is a
 // syntax error. We produce TOK_EOF to terminate
 // scanning and let the parser raise WHAT?.
 lex->current.type = TOK_EOF;
 break;
 }

  lex->current.value.num_value = 0;
  lex->current.str_start = NULL;
  lex->current.str_length = 0;

  if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_REM) {
      lex->pos = lex->length;
  }

}

void lexer_next(Lexer *lex)
{
 if (lex->lookahead_count > 0) {
  lex->current = lex->lookahead[lex->lookahead_read_idx];
  lex->lookahead_read_idx = (lex->lookahead_read_idx + 1) % LEXER_LOOKAHEAD_SIZE;
  lex->lookahead_count--;
 } else {
  scan_next_raw_token_internal(lex);
 }

 while (lex->lookahead_count < LEXER_LOOKAHEAD_SIZE - 1) {
  Token next_tok;
  scan_next_raw_token(lex, &next_tok);
  lex->lookahead[lex->lookahead_write_idx] = next_tok;
  lex->lookahead_write_idx = (lex->lookahead_write_idx + 1) % LEXER_LOOKAHEAD_SIZE;
  lex->lookahead_count++;
 }
}

Token *lexer_peek(Lexer *lex, int distance)
{
 if (distance == 0) {
  return &lex->current;
 }
 if (distance > 0 && distance <= lex->lookahead_count) {
  int idx = (lex->lookahead_read_idx + distance - 1) % LEXER_LOOKAHEAD_SIZE;
  return &lex->lookahead[idx];
 }
  static Token eof_token = { TOK_EOF, {0}, NULL, 0, 0 };
  return &eof_token;
}

void lexer_rewind_to(Lexer *lex, int pos)
{
 lex->pos = pos;
 lex->lookahead_count = 0;
 lex->lookahead_read_idx = 0;
 lex->lookahead_write_idx = 0;
 lexer_next(lex);
}

 // lexer_peek_type - Return the type of the current token.
TokenType lexer_peek_type(Lexer *lex)
{
 return lex->current.type;
}

 // lexer_expect - Consume a specific token type or raise WHAT?.
 //
 // If the current token matches 'expected', advances the lexer
 // and returns 1 (success). Otherwise raises ERR_WHAT with no
 // line number (immediate mode; the caller can add context)
 // and returns 0 (failure).
int lexer_expect(Lexer *lex, TokenType expected)
{
 if (lex->current.type == expected) {
 lexer_next(lex);
 return 1;
 }
 error_raise(ERR_WHAT, 0);
 return 0;
}

 // lexer_match_keyword - Check for a specific keyword.
 //
 // Returns 1 if the current token is TOK_KEYWORD with the given
 // KeywordId. Does NOT advance the lexer.
int lexer_match_keyword(Lexer *lex, KeywordId kw)
{
 return (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == kw) ? 1 : 0;
}

// lexer_skip_to_end - Skip all remaining tokens.
 //
 // Advances the position to the end of the source string,
 // setting the current token to TOK_EOF. Used by REM to
 // skip comment text and by error recovery.
void lexer_skip_to_end(Lexer *lex)
{
 lex->pos = lex->length;
 lex->lookahead_count = 0;
 lex->lookahead_read_idx = 0;
 lex->lookahead_write_idx = 0;
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
}

 // lexer_get_keyword_flags - Look up dialect bitmask for a keyword.
 //
 // Searches the keyword table for the given KeywordId and returns
 // its dialect_flags. Returns DFLAG_ALL if not found (safe default
 // so unknown keywords are never blocked).
unsigned int lexer_get_keyword_flags(KeywordId kw) { return 1; }

// --- Keyword Case Mode ---

static KeywordCaseMode kw_case_mode = KWCASE_MIXED;

void lexer_set_keyword_case(KeywordCaseMode mode)
{
 kw_case_mode = mode;
}

KeywordCaseMode lexer_get_keyword_case(void)
{
 return kw_case_mode;
}

 // lexer_normalize_line - Apply keyword case transformation.
 //
 // Re-lexes the line to find keyword boundaries, then rewrites
 // each keyword span in the requested case. Skips line numbers,
 // string literals, REM comments, and variable names.
void lexer_normalize_line(char *line)
{
 int pos = 0;
 int len;

 if (kw_case_mode == KWCASE_MIXED) return;
 if (line == NULL) return;

 len = (int)strlen(line);
 if (len == 0) return;

 // Skip leading whitespace
 while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
 pos++;

 // Skip line number
 while (pos < len && line[pos] >= '0' && line[pos] <= '9')
 pos++;

 // Skip whitespace after line number
 while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
 pos++;

 // Scan the rest of the line
 while (pos < len) {
 char c = line[pos];

 // Skip string literals (preserve case inside quotes)
 if (c == '"') {
  pos++;
  while (pos < len && line[pos] != '"')
  pos++;
  if (pos < len) pos++; // skip closing quote
  continue;
 }

 // Skip whitespace
 if (c == ' ' || c == '\t') {
  pos++;
  continue;
 }

 // Skip non-alpha characters (operators, digits, etc.)
 if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
  pos++;
  continue;
 }

 // Found an identifier - extract it
 {
  int start = pos;
  int ident_len;
  KeywordId kw;

  while (pos < len &&
   ((line[pos] >= 'A' && line[pos] <= 'Z') ||
    (line[pos] >= 'a' && line[pos] <= 'z') ||
    (line[pos] >= '0' && line[pos] <= '9') ||
    line[pos] == '_'))
  pos++;

  ident_len = pos - start;

  // Check for $ suffix (string function/variable)
  // Don't include $ in the identifier for matching

  // Try to match as keyword
  kw = match_keyword(line + start, ident_len);

  if (kw != KW_COUNT) {
  // It's a keyword - apply case transformation
  const char *canon = lexer_keyword_name(kw);
  int canon_len = (int)strlen(canon);
  int j;

  // Only transform if lengths match (they should)
  if (canon_len == ident_len) {
   switch (kw_case_mode) {
   case KWCASE_UPPER:
    for (j = 0; j < ident_len; j++)
    line[start + j] = canon[j];
    break;
   case KWCASE_LOWER:
    for (j = 0; j < ident_len; j++) {
    char ch = canon[j];
    if (ch >= 'A' && ch <= 'Z')
     ch = (char)(ch + 32);
    line[start + j] = ch;
    }
    break;
   case KWCASE_TITLE:
    for (j = 0; j < ident_len; j++) {
    char ch = canon[j];
    if (j == 0) {
     // First char uppercase
     line[start + j] = ch;
    } else {
     // Rest lowercase
     if (ch >= 'A' && ch <= 'Z')
     ch = (char)(ch + 32);
     line[start + j] = ch;
    }
    }
    break;
   default:
    break;
   }
  }

  // If keyword is REM, skip rest of line
  if (kw == KW_REM) return;
  }
  // If not a keyword, it's a variable - leave as-is
 }
 }
}

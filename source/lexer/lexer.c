/*
 * ---
 * BASIC++ Interpreter - lexer.c
 * ---
 *
 * Lexer (tokenizer) implementation.
 *
 * DESIGN RATIONALE:
 * The lexer is a simple single-pass scanner that converts source
 * text into a stream of Token value types. Key design choices:
 *
 * 1. No heap allocation - tokens are value types stored in the
 * Lexer struct. This avoids scratch pool fragmentation.
 *
 * 2. Zero-copy strings - TOK_STRING tokens point directly into
 * the source buffer rather than copying the string content.
 *
 * 3. Case-insensitive keywords - the keyword lookup converts
 * characters to uppercase during comparison, but does not
 * modify the source text.
 *
 * 4. Greedy operator matching - multi-character operators like
 * <= and >= are matched by looking ahead one character.
 *
 * 5. # is context-sensitive - in PRINT statements it's a format
 * specifier (TOK_HASH); as a relational operator it means
 * "not equal" (TOK_NOT_EQ). The lexer always emits TOK_HASH
 * and lets the parser decide context.
 * UPDATE: We emit TOK_NOT_EQ when # appears in relop context.
 * Actually, the lexer cannot know the context, so it always
 * emits TOK_HASH and the parser handles disambiguation.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "dialect.h"
#include "errors.h"
#include "dialect.h"

/* --- Keyword Table ---
 * Static table mapping keyword strings to KeywordId values.
 * Each entry includes dialect_flags for OPTION STRICT support.
 */
typedef struct KeywordEntry {
 const char *name;
 KeywordId id;
 unsigned int dialect_flags;
} KeywordEntry;

/*
 * Dialect flag legend:
 * DFLAG_ALL = available in all dialects / BASIC++ native
 * DFLAG_MSALL = all Microsoft BASIC family + TRS-80
 * DFLAG_MSBASIC= TRS2+GWBS+QBAS+ASFT+C64B+COCO
 * DFLAG_GWQB = GW-BASIC + QBasic
 * DFLAG_STRUCT = GWBS + QBAS + E116 (structured flow)
 */
static const KeywordEntry core_keyword_init_table[] = {
 /* Universal BASIC keywords */
 { "PRINT", KW_PRINT, DFLAG_ALL },
 { "LET", KW_LET, DFLAG_ALL },
 { "INPUT", KW_INPUT, DFLAG_ALL },
 { "IF", KW_IF, DFLAG_ALL },
 { "GOTO", KW_GOTO, DFLAG_ALL },
 { "GOSUB", KW_GOSUB, DFLAG_ALL },
 { "RETURN", KW_RETURN, DFLAG_ALL },
 { "END", KW_END, DFLAG_ALL },
 { "REM", KW_REM, DFLAG_ALL },
 { "RUN", KW_RUN, DFLAG_ALL },
 { "LIST", KW_LIST, DFLAG_ALL },
 { "NEW", KW_NEW, DFLAG_ALL },
 { "SAVE", KW_SAVE, DFLAG_ALL },
 { "LOAD", KW_LOAD, DFLAG_ALL },
 { "STOP", KW_STOP, DFLAG_ALL },
 { "THEN", KW_THEN, DFLAG_ALL },
 { "FOR", KW_FOR, DFLAG_ALL },
 { "NEXT", KW_NEXT, DFLAG_ALL },
 { "TO", KW_TO, DFLAG_ALL },
 { "STEP", KW_STEP, DFLAG_ALL },
 { "ON", KW_ON, DFLAG_ALL },
 /* Math functions */
 { "ABS", KW_ABS, DFLAG_ALL },
 { "RND", KW_RND, DFLAG_ALL },
 { "SGN", KW_SGN, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "SIN", KW_SIN, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "COS", KW_COS, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "TAN", KW_TAN, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "ATN", KW_ATN, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "SQR", KW_SQR, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "LOG", KW_LOG_FUNC, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "EXP", KW_EXP, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "INT", KW_INT_FUNC, DFLAG_ALL },
 /* String functions */
 { "LEN", KW_LEN, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "LEFT", KW_LEFT, DFLAG_MSBASIC | DFLAG_E116 | DFLAG_SBAS },
 { "RIGHT", KW_RIGHT, DFLAG_MSBASIC | DFLAG_E116 | DFLAG_SBAS },
 { "MID", KW_MID, DFLAG_MSBASIC | DFLAG_E116 | DFLAG_SBAS },
 { "CHR", KW_CHR, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "ASC", KW_ASC, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SUPA | DFLAG_SBAS },
 { "STR", KW_STR_FUNC, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "VAL", KW_VAL_FUNC, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 /* DATA/READ/RESTORE */
 { "DATA", KW_DATA, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "READ", KW_READ, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "RESTORE", KW_RESTORE, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 /* DIM */
 { "DIM", KW_DIM, DFLAG_MSALL | DFLAG_EC55 | DFLAG_E116 | DFLAG_ATRI | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 /* SIZE (Tiny BASIC) */
 { "SIZE", KW_SIZE, DFLAG_PATB },
 /* WHILE/WEND */
 { "WHILE", KW_WHILE, DFLAG_STRUCT },
 { "WEND", KW_WEND, DFLAG_STRUCT },
 /* DO/LOOP/UNTIL */
 { "DO", KW_DO, DFLAG_QBAS | DFLAG_E116 },
 { "UNTIL", KW_UNTIL, DFLAG_QBAS | DFLAG_E116 },
 { "LOOP", KW_LOOP, DFLAG_QBAS | DFLAG_E116 },
 /* MERGE/CHAIN */
 { "MERGE", KW_MERGE, DFLAG_GWQB | DFLAG_MBAS | DFLAG_SINC },
 { "CHAIN", KW_CHAIN, DFLAG_GWQB | DFLAG_MBAS },
 /* CLS/CLEAR/CLR */
 { "CLS", KW_CLS, DFLAG_MSALL | DFLAG_E116 | DFLAG_SINC | DFLAG_SUPA },
 { "CLEAR", KW_CLEAR, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_COCO },
 { "CLR", KW_CLR, DFLAG_C64B | DFLAG_ATRI },
 /* TRON/TROFF */
 { "TRON", KW_TRON, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_COCO },
 { "TROFF", KW_TROFF, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_COCO },
 /* Error handling */
 { "ERROR", KW_ERROR, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_ASFT | DFLAG_COCO | DFLAG_E116 },
 /* DEF FN / FN */
 { "DEF", KW_DEF, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 { "FN", KW_FN, DFLAG_MSBASIC | DFLAG_EC55 | DFLAG_E116 | DFLAG_SINC | DFLAG_SUPA | DFLAG_SBAS },
 /* MAT */
 { "MAT", KW_MAT, DFLAG_E116 | DFLAG_PATB | DFLAG_SBAS },
 /* File I/O */
 { "OPEN", KW_OPEN, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 },
 { "CLOSE", KW_CLOSE, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 | DFLAG_C64B },
 { "AS", KW_AS, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 },
 { "EOF", KW_EOF, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 },
 /* BASIC++ native (always available) */
 { "DIALECT", KW_DIALECT, DFLAG_ALL },
 { "COMPILE", KW_COMPILE, DFLAG_ALL },
 { "BSAVE", KW_BSAVE, DFLAG_ALL },
 { "BLOAD", KW_BLOAD, DFLAG_ALL },
 { "MODULE", KW_MODULE, DFLAG_ALL },
 { "SECURITY", KW_SECURITY, DFLAG_ALL },
 { "SYSTEM", KW_SYSTEM, DFLAG_ALL },
 { "BREAK", KW_BREAK, DFLAG_ALL },
 { "CONT", KW_CONT, DFLAG_ALL },
 { "VARS", KW_VARS, DFLAG_ALL },
 { "ASSERT", KW_ASSERT, DFLAG_ALL },
 { "TEST", KW_TEST, DFLAG_ALL },
 { "ENDTEST", KW_ENDTEST, DFLAG_ALL },
 { "SELFTEST", KW_SELFTEST, DFLAG_ALL },
 { "HELP", KW_HELP, DFLAG_ALL },
 { "INFO", KW_INFO, DFLAG_ALL },
 { "CATALOG", KW_CATALOG, DFLAG_ALL },
 { "RENUM", KW_RENUM, DFLAG_ALL },
 { "DELETE", KW_DELETE, DFLAG_ALL },
 { "VER", KW_VER, DFLAG_ALL },
 /* GW-BASIC / QBasic compatibility */
 { "ELSEIF", KW_ELSEIF, DFLAG_QBAS | DFLAG_E116 | DFLAG_SUPA },
 { "ENDIF", KW_ENDIF, DFLAG_QBAS | DFLAG_E116 | DFLAG_SUPA },
 { "CAUSE", KW_CAUSE, DFLAG_E116 },
 { "WHEN", KW_WHEN, DFLAG_E116 },
 { "USE", KW_USE, DFLAG_E116 },
 { "RETRY", KW_RETRY, DFLAG_E116 },
 { "CONTINUE", KW_CONTINUE, DFLAG_E116 },
 { "DECLARE", KW_DECLARE, DFLAG_QBAS | DFLAG_E116 },
 { "ELSE", KW_ELSE, DFLAG_MSBASIC | DFLAG_E116 | DFLAG_SBAS },
 { "SWAP", KW_SWAP, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 | DFLAG_SBAS },
 { "RANDOMIZE", KW_RANDOMIZE, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_EC55 | DFLAG_E116 | DFLAG_SINC | DFLAG_SBAS },
 { "INSTR", KW_INSTR, DFLAG_GWQB | DFLAG_E116 | DFLAG_SBAS },
 { "SPACE", KW_SPACE_FUNC, DFLAG_GWQB },
 { "STRING", KW_STRING_FUNC, DFLAG_GWQB },
 { "HEX", KW_HEX_FUNC, DFLAG_GWQB },
 { "OCT", KW_OCT_FUNC, DFLAG_GWQB },
 { "FIX", KW_FIX, DFLAG_GWQB },
 { "FILES", KW_FILES, DFLAG_GWQB },
 { "BEEP", KW_BEEP, DFLAG_GWQB | DFLAG_SINC },
 { "COLOR", KW_COLOR, DFLAG_GWQB | DFLAG_COCO | DFLAG_AINT | DFLAG_ASFT | DFLAG_ATRI },
 { "DIR", KW_DIR, DFLAG_ALL },
 { "AUTO", KW_AUTO, DFLAG_GWQB | DFLAG_TRS2 },
 { "TAB", KW_TAB_FUNC, DFLAG_ALL },
 { "SPC", KW_SPC_FUNC, DFLAG_GWQB },
 { "SOUND", KW_SOUND, DFLAG_GWQB | DFLAG_COCO },
 { "PLAY", KW_PLAY, DFLAG_GWQB | DFLAG_COCO },
 { "SCREEN", KW_SCREEN, DFLAG_GWQB | DFLAG_COCO },
 { "DRAW", KW_DRAW, DFLAG_GWQB | DFLAG_SINC },
 { "WIDTH", KW_WIDTH, DFLAG_GWQB | DFLAG_TRS2 },
 /* Logical/bitwise operators */
 { "MOD", KW_MOD, DFLAG_GWQB | DFLAG_E116 },
 { "AND", KW_AND, DFLAG_ALL },
 { "OR", KW_OR, DFLAG_ALL },
 { "NOT", KW_NOT, DFLAG_ALL },
 { "XOR", KW_XOR, DFLAG_GWQB | DFLAG_E116 },
 { "EQV", KW_EQV, DFLAG_GWQB },
 { "IMP", KW_IMP, DFLAG_GWQB },
 /* String utilities */
 { "LCASE", KW_LCASE, DFLAG_QBAS | DFLAG_E116 },
 { "UCASE", KW_UCASE, DFLAG_QBAS | DFLAG_E116 },
 { "TCASE", KW_TCASE, DFLAG_ALL },
 { "LTRIM", KW_LTRIM, DFLAG_QBAS | DFLAG_E116 },
 { "RTRIM", KW_RTRIM, DFLAG_QBAS | DFLAG_E116 },
 { "TRIM", KW_TRIM, DFLAG_ALL },
 { "CINT", KW_CINT, DFLAG_GWQB },
 { "TIMER", KW_TIMER, DFLAG_GWQB },
 { "SLEEP", KW_SLEEP, DFLAG_QBAS },
 { "LOCATE", KW_LOCATE, DFLAG_GWQB },
 { "LINE", KW_LINE, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 },
 { "DATE", KW_DATE_FUNC, DFLAG_GWQB },
 { "TIME", KW_TIME_FUNC, DFLAG_GWQB },
 /* QBasic structured features */
 { "SELECT", KW_SELECT, DFLAG_QBAS | DFLAG_E116 },
 { "CASE", KW_CASE, DFLAG_QBAS | DFLAG_E116 },
 { "IS", KW_IS, DFLAG_QBAS | DFLAG_E116 },
 { "CSNG", KW_CSNG, DFLAG_GWQB },
 { "CDBL", KW_CDBL, DFLAG_GWQB },
 { "EXIT", KW_EXIT, DFLAG_QBAS | DFLAG_E116 | DFLAG_SUPA },
 { "CONST", KW_CONST_KW, DFLAG_QBAS | DFLAG_E116 },
 { "INKEY", KW_INKEY, DFLAG_GWQB | DFLAG_COCO | DFLAG_SINC },
 { "ERASE", KW_ERASE, DFLAG_GWQB | DFLAG_E116 },
 { "USING", KW_USING, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_E116 | DFLAG_SBAS },
 { "LPRINT", KW_LPRINT, DFLAG_GWQB | DFLAG_SINC },
 /* SUB/FUNCTION */
 { "SUB", KW_SUB, DFLAG_QBAS | DFLAG_E116 },
 { "CALL", KW_CALL, DFLAG_GWQB | DFLAG_E116 | DFLAG_AINT },
 { "FUNCTION", KW_FUNCTION, DFLAG_QBAS | DFLAG_E116 },
 /* Additional features */
 { "SHELL", KW_SHELL, DFLAG_GWQB },
 { "REDIM", KW_REDIM, DFLAG_QBAS },
 { "SHARED", KW_SHARED, DFLAG_QBAS },
 { "STATIC", KW_STATIC, DFLAG_QBAS },
 { "RESUME", KW_RESUME, DFLAG_GWQB | DFLAG_E116 },
 { "OPTION", KW_OPTION, DFLAG_ALL },
 { "ENVIRON", KW_ENVIRON, DFLAG_GWQB },
 { "LOF", KW_LOF, DFLAG_GWQB },
 { "COMMON", KW_COMMON, DFLAG_GWQB },
 { "SEEK", KW_SEEK, DFLAG_QBAS },
 { "CHDIR", KW_CHDIR, DFLAG_GWQB },
 /* Memory / graphics */
 { "PEEK", KW_PEEK, DFLAG_MSBASIC | DFLAG_SINC | DFLAG_SUPA | DFLAG_AINT },
 { "POKE", KW_POKE, DFLAG_MSBASIC | DFLAG_SINC | DFLAG_SUPA | DFLAG_AINT },
 { "SEG", KW_SEG, DFLAG_GWQB },
 { "PSET", KW_PSET, DFLAG_GWQB | DFLAG_COCO },
 { "CIRCLE", KW_CIRCLE, DFLAG_GWQB },
 { "PALETTE", KW_PALETTE, DFLAG_GWQB },
 { "POINT", KW_POINT, DFLAG_GWQB | DFLAG_COCO | DFLAG_TRS1 | DFLAG_TRS2 },
 { "TYPE", KW_TYPE, DFLAG_QBAS },
 { "PAINT", KW_PAINT, DFLAG_GWQB | DFLAG_COCO },
 { "ACCESS", KW_ACCESS, DFLAG_QBAS | DFLAG_E116 },
 /* GW-BASIC specific */
 { "CSRLIN", KW_CSRLIN, DFLAG_GWQB },
 { "CVI", KW_CVI, DFLAG_GWQB },
 { "CVS", KW_CVS, DFLAG_GWQB },
 { "CVD", KW_CVD, DFLAG_GWQB },
 { "DEFINT", KW_DEFINT, DFLAG_MSBASIC },
 { "DEFDBL", KW_DEFDBL, DFLAG_MSBASIC },
 { "DEFSNG", KW_DEFSNG, DFLAG_MSBASIC },
 { "DEFSTR", KW_DEFSTR, DFLAG_MSBASIC },
 { "ERL", KW_ERL, DFLAG_GWQB | DFLAG_E116 },
 { "ERR", KW_ERR_VAR, DFLAG_GWQB | DFLAG_E116 },
 { "ERR$", KW_ERR_STR, DFLAG_GWQB | DFLAG_E116 },
 { "EDIT", KW_EDIT, DFLAG_GWQB | DFLAG_TRS2 },
 { "EXTERR", KW_EXTERR, DFLAG_GWBS },
 { "ERDEV", KW_ERDEV, DFLAG_GWBS },
 { "FIELD", KW_FIELD, DFLAG_GWQB },
 { "FRE", KW_FRE, DFLAG_GWQB | DFLAG_C64B | DFLAG_ASFT | DFLAG_COCO },
 { "GET", KW_GET, DFLAG_GWQB | DFLAG_C64B | DFLAG_ASFT },
 { "PUT", KW_PUT, DFLAG_GWQB | DFLAG_C64B | DFLAG_ASFT },
 { "INP", KW_INP, DFLAG_GWQB | DFLAG_TRS2 | DFLAG_SINC },
 { "IOCTL", KW_IOCTL, DFLAG_GWBS | DFLAG_QBAS },
 { "KEY", KW_KEY, DFLAG_GWQB | DFLAG_COCO },
 { "KILL", KW_KILL, DFLAG_GWQB },
 { "LLIST", KW_LLIST, DFLAG_GWQB | DFLAG_SINC },
 { "LOC", KW_LOC, DFLAG_GWQB },
 { "LOCK", KW_LOCK, DFLAG_GWQB },
 { "LPOS", KW_LPOS, DFLAG_GWQB },
 { "LSET", KW_LSET, DFLAG_GWQB },
 { "MKDIR", KW_MKDIR, DFLAG_GWQB },
 { "MKD", KW_MKD_FUNC, DFLAG_GWQB },
 { "MKI", KW_MKI_FUNC, DFLAG_GWQB },
 { "MKS", KW_MKS_FUNC, DFLAG_GWQB },
 { "ALIAS", KW_ALIAS, DFLAG_ALL },
 { "SCOPE", KW_SCOPE, DFLAG_ALL },
 { "KEYWORD", KW_KEYWORD, DFLAG_ALL },
 { "OVERRIDE", KW_OVERRIDE, DFLAG_ALL },
 { "NAME", KW_NAME, DFLAG_GWQB },
 { "RENAME", KW_RENAME, DFLAG_ALL },
 { "OUT", KW_OUT, DFLAG_GWQB | DFLAG_SINC },
 { "COM", KW_COM, DFLAG_GWQB },
 { "PEN", KW_PEN, DFLAG_GWQB },
 { "STRIG", KW_STRIG, DFLAG_GWQB },
 { "PCOPY", KW_PCOPY, DFLAG_GWQB },
 { "PMAP", KW_PMAP, DFLAG_GWQB },
 { "POS", KW_POS_FUNC, DFLAG_GWQB },
 { "PRESET", KW_PRESET, DFLAG_GWQB },
 { "HOME", KW_HOME, DFLAG_ALL },
 { "AT", KW_AT, DFLAG_ALL },
 { "RESET", KW_RESET, DFLAG_GWQB | DFLAG_TRS1 | DFLAG_TRS2 | DFLAG_COCO },
 { "RMDIR", KW_RMDIR, DFLAG_GWQB },
 { "RSET", KW_RSET, DFLAG_GWQB },
 { "STICK", KW_STICK, DFLAG_GWQB },
 { "UNLOCK", KW_UNLOCK, DFLAG_GWQB },
 { "USR", KW_USR, DFLAG_GWQB | DFLAG_SINC },
 { "VARPTR", KW_VARPTR, DFLAG_GWQB },
 { "VIEW", KW_VIEW, DFLAG_GWQB },
 { "WAIT", KW_WAIT, DFLAG_GWQB },
 { "WINDOW", KW_WINDOW, DFLAG_GWQB },
 { "WRITE", KW_WRITE, DFLAG_GWQB },
 { "MEMMAP", KW_MEMMAP, DFLAG_ALL },
 { "SYS", KW_SYS, DFLAG_ALL },
 { "EXEC", KW_EXEC, DFLAG_ALL },
 { "ERRORLEVEL", KW_ERRORLEVEL, DFLAG_ALL },
 { "BYE", KW_BYE, DFLAG_ALL },
 /* ECMA-116 Enhanced Files Module */
 { "SET", KW_SET, DFLAG_E116 | DFLAG_TRS1 | DFLAG_TRS2 | DFLAG_COCO },
 { "ASK", KW_ASK, DFLAG_E116 },
 { "REWRITE", KW_REWRITE, DFLAG_E116 },
 { "POINTER", KW_POINTER, DFLAG_E116 },
 /* Virtual subsystem introspection */
 { "VDEV", KW_VDEV, DFLAG_ALL },
 { "VMEM", KW_VMEM, DFLAG_ALL },
 { "VNET", KW_VNET, DFLAG_ALL },
 { "VCON", KW_VCON, DFLAG_ALL },
 { "VTERM", KW_VTERM, DFLAG_ALL },
 { "VMACH", KW_VMACH, DFLAG_ALL },
 { "DEVMAP", KW_DEVMAP, DFLAG_ALL },
 { "BIN", KW_BIN_FUNC, DFLAG_ALL },
 { "CLOCK", KW_CLOCK_FUNC, DFLAG_ALL },
 { "ALARM", KW_ALARM_FUNC, DFLAG_ALL },
 /* Sinclair BASIC specific keywords */
 { "LN", KW_LOG_FUNC, DFLAG_SINC }, /* Sinclair uses LN for LOG */
 { "BORDER", KW_BORDER, DFLAG_SINC }, /* Border color 0-7 */
 { "PAPER", KW_PAPER, DFLAG_SINC }, /* Paper (background) 0-7 */
 { "INK", KW_INK, DFLAG_SINC }, /* Ink (foreground) 0-7 */
 { "BRIGHT", KW_BRIGHT, DFLAG_SINC }, /* Bright mode 0/1 */
 { "FLASH", KW_FLASH, DFLAG_SINC }, /* Flash/blink mode 0/1 */
 { "INVERSE", KW_INVERSE, DFLAG_SINC }, /* Reverse video 0/1 */
 { "OVER", KW_OVER, DFLAG_SINC }, /* Overprint mode 0/1 */
 { "PLOT", KW_PSET, DFLAG_SINC }, /* PLOT x,y -> PSET */
 { "PAUSE", KW_PAUSE, DFLAG_ALL }, /* PAUSE n (timer ticks) */
 /* Sinclair COPY (screen dump) handled via LPRINT only in SINC */
 { "COPY", KW_COPY, DFLAG_ALL }, /* COPY file TO file */
 { "IN", KW_INP, DFLAG_SINC }, /* IN addr -> INP */
 { "DELAY", KW_DELAY, DFLAG_ALL }, /* DELAY ms (NOP busy-wait) */
 /* SuperBASIC (Sinclair QL) structured keywords */
 { "REPEAT", KW_REPEAT, DFLAG_SUPA }, /* REPeat label */
 { "ENDREPEAT", KW_ENDREPEAT, DFLAG_SUPA }, /* END REPeat label */
 { "ENDFOR", KW_ENDFOR, DFLAG_SUPA }, /* END FOR var */
 { "REMAINDER", KW_REMAINDER, DFLAG_SUPA }, /* SELect default */
 { "DEFINE", KW_DEFINE, DFLAG_SUPA }, /* DEFine PROCedure/FN */
 { "PROCEDURE", KW_PROCEDURE, DFLAG_SUPA }, /* PROCedure type */
 { "LOCAL", KW_LOCAL, DFLAG_SUPA }, /* LOCal var decl */
 { "ENDDEFINE", KW_ENDDEFINE, DFLAG_SUPA }, /* END DEFine */
 /* Complex number functions */
 { "COMPLEX", KW_COMPLEX, DFLAG_ALL },
 { "REAL", KW_REAL_FUNC, DFLAG_ALL },
 { "IMAG", KW_IMAG_FUNC, DFLAG_ALL },
 { "CONJ", KW_CONJ_FUNC, DFLAG_ALL },
 { "CABS", KW_CABS_FUNC, DFLAG_ALL },
 /* Static analysis */
 { "CHECK", KW_CHECK, DFLAG_ALL },
 { "VERIFY", KW_VERIFY, DFLAG_ALL },
 /* File query functions */
 { "EXISTS", KW_EXISTS, DFLAG_ALL },
 { "FILESIZE", KW_FILESIZE, DFLAG_ALL },
 { "FILEMOD", KW_FILEMOD, DFLAG_ALL },
 /* Formatted I/O */
 { "DISPLAY", KW_DISPLAY, DFLAG_ALL },
 /* Complex math functions */
 { "CSQR", KW_CSQR_FUNC, DFLAG_ALL },
 { "CEXP", KW_CEXP_FUNC, DFLAG_ALL },
 { "CLOG", KW_CLOG_FUNC, DFLAG_ALL },
 { "CARG", KW_CARG_FUNC, DFLAG_ALL },
 { "CPOW", KW_CPOW_FUNC, DFLAG_ALL },
 /* Aggregate / rounding math functions (BASIC++) */
 { "MIN", KW_MIN_FUNC, DFLAG_ALL },
 { "MAX", KW_MAX_FUNC, DFLAG_ALL },
 { "AVG", KW_AVG_FUNC, DFLAG_ALL },
 { "MED", KW_MED_FUNC, DFLAG_ALL },
 { "ROUND", KW_ROUND_FUNC, DFLAG_ALL },
 /* SUPER BASIC (Tymshare) extended math functions */
 { "ASIN", KW_ASIN_FUNC, DFLAG_ALL },
 { "ACOS", KW_ACOS_FUNC, DFLAG_ALL },
 { "SINH", KW_SINH_FUNC, DFLAG_ALL },
 { "COSH", KW_COSH_FUNC, DFLAG_ALL },
 { "TANH", KW_TANH_FUNC, DFLAG_ALL },
 { "LOG10", KW_LOG10_FUNC, DFLAG_ALL },
 { "LOG2", KW_LOG2_FUNC, DFLAG_ALL },
 { "COMP", KW_COMP_FUNC, DFLAG_ALL },
 { "PDIF", KW_PDIF_FUNC, DFLAG_ALL },
 { "PI", KW_PI_FUNC, DFLAG_ALL },
 /* SUPER BASIC statement modifiers / keywords */
 { "UNLESS", KW_UNLESS, DFLAG_SBAS | DFLAG_SUPA },
 { "BY", KW_BY, DFLAG_SBAS | DFLAG_SUPA },
 { "SCRATCH", KW_SCRATCH, DFLAG_ALL },
 { "UNSAVE", KW_UNSAVE, DFLAG_ALL },
 /* File management (native, no SHELL) */
 { "MOVE", KW_MOVE, DFLAG_ALL },
 { "PWD", KW_PWD, DFLAG_ALL },
 { "CWD", KW_CWD_FUNC, DFLAG_ALL },
 { "CURDIR", KW_CWD_FUNC, DFLAG_ALL }, /* QBasic alias */
 { "EXIST", KW_EXIST_FUNC, DFLAG_ALL },
 { "FILELEN", KW_FILELEN_FUNC, DFLAG_ALL },
 /* Event trapping */
 { "TRAP", KW_TRAP, DFLAG_ALL },
 /* SUPER BASIC formatted output */
 { "IMAGE", KW_IMAGE, DFLAG_ALL },
 /* Enhanced debugger commands */
 { "DEBUG", KW_DEBUG, DFLAG_ALL },
 { "DUMP", KW_DUMP, DFLAG_ALL },
 { "BACKTRACE", KW_BACKTRACE, DFLAG_ALL },
 { "TRACE", KW_TRACE, DFLAG_ALL },
 /* Stream I/O primitives (BASIC++ Milestone 11) */
 { "SIOREAD", KW_SIOREAD, DFLAG_ALL },
 { "SIOREADLN", KW_SIOREADLN, DFLAG_ALL },
 { "SIOWRITE", KW_SIOWRITE, DFLAG_ALL },
 { "SIOSEEK", KW_SIOSEEK, DFLAG_ALL },
 { "SIOFLUSH", KW_SIOFLUSH, DFLAG_ALL },
 { "SIOSTATUS", KW_SIOSTATUS, DFLAG_ALL },
 { "SIOAVAIL", KW_SIOAVAIL, DFLAG_ALL },
 /* Block I/O primitives (BASIC++ Milestone 11) */
 { "BIOREAD", KW_BIOREAD, DFLAG_ALL },
 { "BIOWRITE", KW_BIOWRITE, DFLAG_ALL },
 { "BIOSTATUS", KW_BIOSTATUS, DFLAG_ALL },
 { "BIOSIZE", KW_BIOSIZE, DFLAG_ALL },
 { "BIOCHECKSUM", KW_BIOCHECKSUM, DFLAG_ALL },
 { "BIOCOMPARE", KW_BIOCOMPARE, DFLAG_ALL },
 { "BIOFILL", KW_BIOFILL, DFLAG_ALL },
 { "BIOCOPY", KW_BIOCOPY, DFLAG_ALL },
 /* Transaction / ATOMIC keywords */
 { "ATOMIC", KW_ATOMIC, DFLAG_ALL },
 { "TXN", KW_TXN, DFLAG_ALL },
 { "COMMIT", KW_COMMIT, DFLAG_ALL },
 { "ROLLBACK", KW_ROLLBACK, DFLAG_ALL },
 { "TXNSTATUS", KW_TXNSTATUS, DFLAG_ALL },
 /* Network builtins */
 { "NSTATUS", KW_NSTATUS, DFLAG_ALL },
 { "NHTTPSTATUS", KW_NHTTPSTATUS, DFLAG_ALL },
 { "NEOF", KW_NEOF, DFLAG_ALL },
 { "NBYTESWAITING", KW_NBYTESWAITING, DFLAG_ALL },
 { "NCONNECTED", KW_NCONNECTED, DFLAG_ALL },
 { "NERROR", KW_NERROR, DFLAG_ALL },
 { "NJSONQUERY", KW_NJSONQUERY, DFLAG_ALL },
 { "NINFO", KW_NINFO, DFLAG_ALL },
 { NULL, 0, 0 } /* sentinel */
};

/* --- Dynamic Keyword Registry --- */
#define MAX_DYNAMIC_KEYWORDS 512
static KeywordEntry dynamic_keyword_table[MAX_DYNAMIC_KEYWORDS];
static int dynamic_keyword_count = 0;
static int next_custom_keyword_id = KW_CUSTOM_START;

void keyword_registry_init(void) {
    int i = 0;
    if (dynamic_keyword_count > 0) return; /* Already initialized */
    while (core_keyword_init_table[i].name != NULL) {
        dynamic_keyword_table[i] = core_keyword_init_table[i];
        i++;
    }
    dynamic_keyword_count = i;
}

KeywordId keyword_register_custom(const char *name, unsigned int dialect_flags) {
    int id;
    if (dynamic_keyword_count >= MAX_DYNAMIC_KEYWORDS) return KW_COUNT;
    id = next_custom_keyword_id++;
    dynamic_keyword_table[dynamic_keyword_count].name = strdup(name);
    dynamic_keyword_table[dynamic_keyword_count].id = id;
    dynamic_keyword_table[dynamic_keyword_count].dialect_flags = dialect_flags;
    dynamic_keyword_count++;
    return id;
}

/* --- Internal Helpers ---
 */

/*
 * peek_char - Return the character at the current position.
 *
 * Returns '\0' at end of input (safe for all comparisons).
 */
static char peek_char(Lexer *lex)
{
 if (lex->pos >= lex->length) {
 return '\0';
 }
 return lex->source[lex->pos];
}

/*
 * advance_char - Consume the current character and return it.
 *
 * Returns '\0' if already at end of input.
 */
static char advance_char(Lexer *lex)
{
 if (lex->pos >= lex->length) {
 return '\0';
 }
 return lex->source[lex->pos++];
}

/*
 * skip_whitespace - Advance past spaces and tabs.
 *
 * Newlines and other control characters are NOT skipped - they
 * terminate the current line/statement.
 */
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

/*
 * to_upper - Convert a character to uppercase.
 *
 * Uses manual comparison rather than toupper() to avoid locale
 * dependencies on some platforms. Only converts a-z.
 */
static char to_upper(char c)
{
 if (c >= 'a' && c <= 'z') {
 return (char)(c - 'a' + 'A');
 }
 return c;
}

/* --- Keyword Alias Table ---
 * Runtime-configurable name remapping with scoping.
 * Each slot maps a user-chosen identifier (e.g. "IMPRE")
 * to a built-in KeywordId (e.g. KW_PRINT).
 * Aliases are checked FIRST in match_keyword().
 */
typedef struct {
 char name[MAX_ALIAS_NAME]; /* upper-cased alias */
 int name_len;
 KeywordId target;
 int active; /* 1 = in use */
 AliasScope scope;
 char module_name[16]; /* for ASCOPE_MODULE */
} AliasEntry;

static AliasEntry alias_table[MAX_ALIASES];
static int alias_count = 0;

/* --- Operator Alias Table ---
 * Maps alias strings to token types for operators.
 */
typedef struct {
 char name[MAX_OP_ALIAS_NAME];
 int name_len;
 int token_type; /* TOK_GT_EQ, TOK_LT_EQ, etc. */
 int active;
} OpAliasEntry;

static OpAliasEntry op_alias_table[MAX_OP_ALIASES];
static int op_alias_count = 0;

/*
 * lexer_add_alias - Register with GLOBAL scope (backward compat).
 */
int lexer_add_alias(const char *name, int name_len,
 KeywordId target)
{
 return lexer_add_alias_scoped(name, name_len, target,
 ASCOPE_GLOBAL, NULL);
}

/*
 * lexer_add_alias_scoped - Register or update an alias.
 */
int lexer_add_alias_scoped(const char *name, int name_len,
 KeywordId target, AliasScope scope,
 const char *module_name)
{
 int i;
 char upper[MAX_ALIAS_NAME];

 if (name_len <= 0 || name_len >= MAX_ALIAS_NAME)
 return -1;

 /* Upper-case the alias name */
 for (i = 0; i < name_len; i++)
 upper[i] = to_upper(name[i]);
 upper[name_len] = '\0';

 /* Reject aliasing protected keywords */
 if (target == KW_ALIAS || target == KW_SCOPE ||
  target == KW_KEYWORD || target == KW_SECURITY ||
  target == KW_OVERRIDE)
 return -3; /* protected keyword */

 /* Reject alias names that shadow existing keywords */
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
  return -2; /* conflicts with keyword */
 }

 /* Check if alias already exists - update it */
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

 /* Find an empty slot */
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

 return -1; /* table full */
}

/*
 * lexer_remove_alias - Remove a single alias by name.
 */
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
 return -1; /* not found */
}

/*
 * lexer_clear_aliases - Remove all aliases.
 */
void lexer_clear_aliases(void)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++)
 alias_table[i].active = 0;
 alias_count = 0;
}

/*
 * lexer_clear_scope - Remove aliases of a specific scope.
 */
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

/*
 * lexer_clear_module_aliases - Remove aliases for a module.
 */
void lexer_clear_module_aliases(const char *mod_name)
{
 int i;
 for (i = 0; i < MAX_ALIASES; i++) {
 if (alias_table[i].active &&
 alias_table[i].scope == ASCOPE_MODULE) {
 /* Case-insensitive compare */
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

/*
 * lexer_alias_count - Return count of active aliases.
 */
int lexer_alias_count(void)
{
 return alias_count;
}

/*
 * lexer_keyword_name - Get canonical name for a keyword.
 */
const char *lexer_keyword_name(KeywordId kw)
{
 int i;
 for (i = 0; i < dynamic_keyword_count; i++) {
 if (dynamic_keyword_table[i].id == kw)
 return dynamic_keyword_table[i].name;
 }
 return "";
}

/*
 * lexer_keyword_needs_dollar - Check if keyword uses $.
 */
int lexer_keyword_needs_dollar(KeywordId kw)
{
    return (kw == KW_LEFT || kw == KW_RIGHT ||
    kw == KW_MID || kw == KW_CHR ||
    kw == KW_STR_FUNC ||
    kw == KW_SPACE_FUNC ||
    kw == KW_STRING_FUNC ||
    kw == KW_HEX_FUNC ||
    kw == KW_OCT_FUNC ||
    kw == KW_LCASE || kw == KW_UCASE ||
    kw == KW_TCASE ||
    kw == KW_LTRIM || kw == KW_RTRIM ||
    kw == KW_TRIM ||
    kw == KW_DATE_FUNC ||
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
    kw == KW_CLOCK_FUNC ||
    kw == KW_ALARM_FUNC ||
    kw == KW_DIALECT_FUNC ||
    kw == KW_MEMMAP_FUNC ||
    kw == KW_ALIAS_FUNC ||
    kw == KW_CWD_FUNC ||
    kw == KW_SIOREAD ||
    kw == KW_SIOREADLN ||
    kw == KW_BIOREAD ||
    kw == KW_NJSONQUERY ||
    kw == KW_NINFO);
}

/*
 * lexer_find_alias_by_name - Given an alias name,
 * return the original keyword name. NULL if not found.
 */
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
 /* Return "PRINT" or "LEFT$" etc. */
 if (need_d) {
 /* Build name with $ in static buffer */
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

/*
 * lexer_find_alias_for_keyword - Given a keyword ID,
 * return the first alias name. NULL if not found.
 */
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

/*
 * lexer_list_aliases - Print all active aliases.
 */
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

/* --- Operator Alias Functions --- */

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

 /* Update existing */
 for (i = 0; i < MAX_OP_ALIASES; i++) {
 if (op_alias_table[i].active &&
 op_alias_table[i].name_len == name_len &&
 memcmp(op_alias_table[i].name, upper,
 (size_t)name_len) == 0) {
 op_alias_table[i].token_type = token_type;
 return 0;
 }
 }

 /* New slot */
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

/* --- ALIAS file I/O --- */

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

 /* Skip comments and blanks */
 if (line[0] == '#' || line[0] == '\'' ||
 line[0] == '\n' || line[0] == '\r')
 continue;

 /* Parse: KEYWORD = ALIASNAME */
 eq = strchr(line, '=');
 if (eq == NULL) continue;

 /* Extract keyword (left of =) */
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

  /* Strip trailing $ from keyword name for lookup */
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

 /* Extract alias name (right of =) */
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

/*
 * match_keyword - Try to match an identifier against the
 * alias table first, then the keyword table.
 *
 * Compares the identifier string (starting at 'start',
 * length 'len') case-insensitively.
 *
 * Returns the KeywordId if matched, KW_COUNT if not.
 */
static KeywordId match_keyword(const char *start, int len)
{
 int i;

 /* Check alias table first (aliases shadow builtins) */
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

 /* Then check built-in keyword table */
 for (i = 0; i < dynamic_keyword_count; i++) {
 const char *kw = dynamic_keyword_table[i].name;
 int kw_len = (int)strlen(kw);
 int j;
 int matched;

 if (kw_len != len) {
 continue;
 }

 matched = 1;
 for (j = 0; j < len; j++) {
 if (to_upper(start[j]) != kw[j]) {
 matched = 0;
 break;
 }
 }

 if (matched) {
 return dynamic_keyword_table[i].id;
 }
 }

 return KW_COUNT; /* not a keyword */
}

/* --- Lexer Public API ---
 */

/*
 * lexer_init - Initialize the lexer on a source line.
 *
 * Sets up all fields and primes the first token by calling
 * lexer_next(). After this call, lex->current contains the
 * first token of the source line.
 */
void lexer_init(Lexer *lex, const char *source)
{
 lex->source = source;
 lex->pos = 0;
 lex->length = (int)strlen(source);

 /* Clear current token */
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;

 /* Prime the first token */
 lexer_next(lex);
}

/*
 * lexer_next - Scan and produce the next token.
 *
 * This is the core of the lexer. It handles:
 * - Whitespace skipping
 * - Integer literals (sequences of digits)
 * - String literals (double-quoted, no escape sequences)
 * - Identifiers and keywords (alphabetic sequences)
 * - Single-character operators (+, -, *, /, =, (, ), etc.)
 * - Multi-character operators (<=, >=, <>)
 * - End of input (TOK_EOF)
 *
 * The function updates lex->current with the new token.
 */
void lexer_next(Lexer *lex)
{
 char c;

 skip_whitespace(lex);

 /* Check for end of input */
 if (lex->pos >= lex->length) {
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 return;
 }

 c = peek_char(lex);

 /*
 * ----- &H hex, &O octal, &B binary literals -----
 *
 * GW-BASIC/QBasic syntax:
 * &Hffff - hexadecimal (case insensitive)
 * &O377 - octal
 * &377 - octal (& without O)
 * &B1010 - binary (BASIC++ extension)
 */
 if (c == '&') {
 long hval = 0;
 int found = 0;
 lex->pos++; /* consume '&' */

 if (lex->pos < lex->length &&
 (lex->source[lex->pos] == 'H' ||
 lex->source[lex->pos] == 'h')) {
 /* &H hex literal */
 lex->pos++; /* consume 'H' */
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
 /* &O octal literal */
 lex->pos++; /* consume 'O' */
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
 /* &B binary literal */
 lex->pos++; /* consume 'B' */
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
 /* &nnn - bare octal (no O prefix) */
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

 /* ----- Numeric literals (integer or float) ----- */
 if (isdigit((unsigned char)c) || (c == '.' &&
 lex->pos + 1 < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos + 1]))) {
 long ivalue = 0;
 int has_dot = 0;
 int int_start = lex->pos;

 /* Scan integer part */
 while (lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 ivalue = ivalue * 10 + (lex->source[lex->pos] - '0');
 lex->pos++;
 }

 /* Check for decimal point -> float literal */
 if (lex->pos < lex->length && lex->source[lex->pos] == '.') {
 has_dot = 1;
 }

 /*
 * Check for E/e exponent suffix (scientific
 * notation without decimal point: 1E10, 5E-3).
 * ECMA-55 s5.2 requires E-notation support.
 */
 if (!has_dot && lex->pos < lex->length) {
 char ec = lex->source[lex->pos];
 if (ec == 'E' || ec == 'e') {
 has_dot = 1; /* force float path */
 }
 }

 if (has_dot) {
 /* Re-parse entire number as double using strtod */
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

 /* Check for imaginary suffix (i/I) */
 if (lex->pos < lex->length &&
 (lex->source[lex->pos] == 'i' ||
 lex->source[lex->pos] == 'I') &&
 /* Must NOT be followed by alnum (not a var) */
 (lex->pos + 1 >= lex->length ||
 !isalpha((unsigned char)
 lex->source[lex->pos + 1]))) {
 lex->pos++; /* consume 'i' */
 /* Convert to imaginary token */
 if (lex->current.type == TOK_NUMBER) {
 lex->current.value.fval =
 (double)lex->current.value.num_value;
 }
 lex->current.type = TOK_IMAGINARY;
 }
 return;
 }

 /* ----- String literals ----- */
 if (c == '"') {
 int start;
 lex->pos++; /* skip opening quote */
 start = lex->pos;

 while (lex->pos < lex->length && lex->source[lex->pos] != '"') {
 lex->pos++;
 }

 lex->current.type = TOK_STRING;
 lex->current.value.num_value = 0;
 lex->current.str_start = lex->source + start;
 lex->current.str_length = lex->pos - start;

 if (lex->pos < lex->length) {
 lex->pos++; /* skip closing quote */
 }
 /* If no closing quote, we still produce the token with
 * whatever text we found. PATB behavior: unterminated
 * strings are not a fatal error. */
 return;
 }

 /* ----- Identifiers and keywords ----- */
 if (isalpha((unsigned char)c)) {
 int start = lex->pos;
 int len;
 KeywordId kw;

 while (lex->pos < lex->length &&
 (isalpha((unsigned char)lex->source[lex->pos]) ||
 isdigit((unsigned char)lex->source[lex->pos]))) {
 lex->pos++;
 }
 len = lex->pos - start;

 /*
  * If the identifier doesn't match a keyword, and it
  * ends with digits (e.g. "X1"), back off the digits
  * so the lexer can parse them as a separate number.
  * This preserves traditional BASIC variable naming
  * (A1 = variable A followed by 1) while allowing
  * keywords like LOG10 and LOG2 to work.
  */

 /* Try keyword match with full alphanumeric token */
 kw = match_keyword(lex->source + start, len);

 if (kw == KW_COUNT) {
 /* No keyword match — back off trailing digits so
  * they parse as a separate number token.
  * e.g. "X1" -> identifier "X" + number "1" */
 int alpha_end = lex->pos;
 while (alpha_end > start &&
 isdigit((unsigned char)
 lex->source[alpha_end - 1])) {
 alpha_end--;
 }
 if (alpha_end > start && alpha_end < lex->pos) {
 lex->pos = alpha_end;
 len = lex->pos - start;
 }
 }

 /* NOTE: GW-BASIC greedy keyword extraction is not implemented.
 * Programs with no-space patterns (e.g. IFR1>.98THEN) need
 * source preprocessing to add spaces around keywords. */

 if (kw != KW_COUNT) {
 lex->current.type = TOK_KEYWORD;
 lex->current.value.keyword = kw;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;

 /*
 * String function keywords (LEFT, RIGHT,
 * MID, CHR, STR) may be followed by '$'.
 * Consume it as part of the keyword so
 * LEFT$( tokenizes as KW_LEFT + TOK_LPAREN
 * instead of KW_LEFT + unknown + TOK_LPAREN.
 */
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
 kw == KW_DATE_FUNC ||
 kw == KW_TIME_FUNC ||
 kw == KW_INKEY ||
 kw == KW_ENVIRON ||
 kw == KW_MKD_FUNC ||
 kw == KW_MKI_FUNC ||
 kw == KW_MKS_FUNC ||
 kw == KW_SHELL ||
 kw == KW_BIN_FUNC ||
 kw == KW_CLOCK_FUNC ||
 kw == KW_ALARM_FUNC ||
 kw == KW_CWD_FUNC ||
 kw == KW_SIOREAD ||
 kw == KW_SIOREADLN ||
 kw == KW_BIOREAD ||
 kw == KW_NJSONQUERY ||
 kw == KW_NINFO) {
 lex->pos++; /* consume '$' */
 }
 /* INPUT$ -> KW_INPUT_FUNC */
 if (kw == KW_INPUT) {
 lex->pos++;
 kw = KW_INPUT_FUNC;
 lex->current.value.keyword = kw;
 }
 /* IOCTL$ -> KW_IOCTL_FUNC */
 if (kw == KW_IOCTL) {
 lex->pos++;
 kw = KW_IOCTL_FUNC;
 lex->current.value.keyword = kw;
 }
 /* VARPTR$ -> KW_VARPTR_STR */
 if (kw == KW_VARPTR) {
 lex->pos++;
 kw = KW_VARPTR_STR;
 lex->current.value.keyword = kw;
 }
 /* DIALECT$ -> KW_DIALECT_FUNC */
 if (kw == KW_DIALECT) {
 lex->pos++;
 kw = KW_DIALECT_FUNC;
 lex->current.value.keyword = kw;
 }
 /* MEMMAP$ -> KW_MEMMAP_FUNC */
 if (kw == KW_MEMMAP) {
 lex->pos++;
 kw = KW_MEMMAP_FUNC;
 lex->current.value.keyword = kw;
 }
 /* ALIAS$ -> KW_ALIAS_FUNC */
 if (kw == KW_ALIAS) {
 lex->pos++;
 kw = KW_ALIAS_FUNC;
 lex->current.value.keyword = kw;
 }
 }
 } else if (len == 1) {
 /*
 * Single letter that's not a keyword.
 * In extended-vars mode, check if digits follow (e.g., X1).
 * If so, scan them and emit TOK_NAMED_VAR.
 */
 if (dialect_get_config()->has_extended_vars &&
 lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 while (lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 lex->pos++;
 }
 len = lex->pos - start;
 			/* Check for trailing type suffix (e.g. X1$, A%, B!) */
			if (lex->pos < lex->length) {
				char t = lex->source[lex->pos];
				if (t == '$' || t == '%' || t == '!' || t == '#' || t == '&' || t == '~') {
					lex->pos++; /* consume suffix */
					len++;
				}
			}
 lex->current.type = TOK_NAMED_VAR;
 lex->current.value.num_value = 0;
 lex->current.str_start = lex->source + start;
 lex->current.str_length = len;
 		} else if (lex->pos < lex->length &&
			(lex->source[lex->pos] == '$' || lex->source[lex->pos] == '%' ||
			 lex->source[lex->pos] == '!' || lex->source[lex->pos] == '#' ||
			 lex->source[lex->pos] == '&' || lex->source[lex->pos] == '~')) {
			/* String or typed variable: A$ through Z$ or A% etc. */
			/* We treat typed single-char vars as TOK_NAMED_VAR for parser consistency */
			lex->pos++; /* consume suffix */
			lex->current.type = TOK_NAMED_VAR;
			lex->current.value.num_value = 0;
			lex->current.str_start = lex->source + start;
			lex->current.str_length = 2;
 } else {
 /* Standard single-letter variable A-Z */
 lex->current.type = TOK_VARIABLE;
 lex->current.value.var_name = to_upper(lex->source[start]);
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 }
 } else {
 /*
 * Multi-character identifier that's not a keyword.
 *
 * If the active dialect supports extended variables,
 * treat this as a named variable (e.g., SCORE, COUNT).
 * The name is stored in str_start/str_length for the
 * parser to pass to the runtime's named variable table.
 *
 * If the dialect does NOT support extended variables
 * (e.g., PATB), rewind to just after the first character
 * and treat it as a single-letter variable. This matches
 * PATB behavior where only A-Z are valid variable names.
 */
 if (dialect_get_config()->has_extended_vars) {
 /* Also allow digits in the name (e.g., X1, SCORE2) */
 while (lex->pos < lex->length &&
 isdigit((unsigned char)lex->source[lex->pos])) {
 lex->pos++;
 }
 len = lex->pos - start;

 			/*
			 * Check for trailing type suffix.
			 * e.g., RM$, FT%, KM! -> TOK_NAMED_VAR
			 * with "RM$", "FT%", "KM!" as the name.
			 */
			if (lex->pos < lex->length) {
				char t = lex->source[lex->pos];
				if (t == '$' || t == '%' || t == '!' || t == '#' || t == '&' || t == '~') {
					lex->pos++; /* consume suffix */
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
 lex->current.value.var_name = to_upper(lex->source[start]);
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
 }
 }
 return;
 }

 /* ----- Operators and delimiters ----- */
 advance_char(lex); /* consume the character */

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
 /* Check for <= or <> */
 if (peek_char(lex) == '=') {
 advance_char(lex);
 lex->current.type = TOK_LT_EQ;
 } else if (peek_char(lex) == '>') {
 advance_char(lex);
 lex->current.type = TOK_NOT_EQ;
 } else {
 lex->current.type = TOK_LT;
 }
 break;
 case '>':
 /* Check for >= then >> */
 if (peek_char(lex) == '=') {
 advance_char(lex);
 lex->current.type = TOK_GT_EQ;
 } else if (peek_char(lex) == '>') {
 advance_char(lex);
 lex->current.type = TOK_APPEND;
 } else {
 lex->current.type = TOK_GT;
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
 case '\'':
 /*
 * Apostrophe = REM shorthand.
 * Skip rest of line and produce EOL.
 */
 lex->pos = lex->length;
 lex->current.type = TOK_CR;
 break;
 case '\r':
 case '\n':
 lex->current.type = TOK_CR;
 break;
 default:
 /*
 * Unknown character. In Tiny BASIC, this is a
 * syntax error. We produce TOK_EOF to terminate
 * scanning and let the parser raise WHAT?.
 */
 error_raise(ERR_WHAT, 0);
 lex->current.type = TOK_EOF;
 break;
 }

 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
}

/*
 * lexer_peek_type - Return the type of the current token.
 */
TokenType lexer_peek_type(Lexer *lex)
{
 return lex->current.type;
}

/*
 * lexer_expect - Consume a specific token type or raise WHAT?.
 *
 * If the current token matches 'expected', advances the lexer
 * and returns 1 (success). Otherwise raises ERR_WHAT with no
 * line number (immediate mode; the caller can add context)
 * and returns 0 (failure).
 */
int lexer_expect(Lexer *lex, TokenType expected)
{
 if (lex->current.type == expected) {
 lexer_next(lex);
 return 1;
 }
 error_raise(ERR_WHAT, 0);
 return 0;
}

/*
 * lexer_match_keyword - Check for a specific keyword.
 *
 * Returns 1 if the current token is TOK_KEYWORD with the given
 * KeywordId. Does NOT advance the lexer.
 */
int lexer_match_keyword(Lexer *lex, KeywordId kw)
{
 return (lex->current.type == TOK_KEYWORD &&
 lex->current.value.keyword == kw) ? 1 : 0;
}

/*
 * lexer_skip_to_end - Skip all remaining tokens.
 *
 * Advances the position to the end of the source string,
 * setting the current token to TOK_EOF. Used by REM to
 * skip comment text and by error recovery.
 */
void lexer_skip_to_end(Lexer *lex)
{
 lex->pos = lex->length;
 lex->current.type = TOK_EOF;
 lex->current.value.num_value = 0;
 lex->current.str_start = NULL;
 lex->current.str_length = 0;
}

/*
 * lexer_get_keyword_flags - Look up dialect bitmask for a keyword.
 *
 * Searches the keyword table for the given KeywordId and returns
 * its dialect_flags. Returns DFLAG_ALL if not found (safe default
 * so unknown keywords are never blocked).
 */
unsigned int lexer_get_keyword_flags(KeywordId kw)
{
 int i;
 for (i = 0; i < dynamic_keyword_count; i++) {
 if (dynamic_keyword_table[i].id == kw) {
 return dynamic_keyword_table[i].dialect_flags;
 }
 }
 return DFLAG_ALL; /* unknown = always allowed */
}

/* --- Keyword Case Mode --- */

static KeywordCaseMode kw_case_mode = KWCASE_MIXED;

void lexer_set_keyword_case(KeywordCaseMode mode)
{
 kw_case_mode = mode;
}

KeywordCaseMode lexer_get_keyword_case(void)
{
 return kw_case_mode;
}

/*
 * lexer_normalize_line - Apply keyword case transformation.
 *
 * Re-lexes the line to find keyword boundaries, then rewrites
 * each keyword span in the requested case. Skips line numbers,
 * string literals, REM comments, and variable names.
 */
void lexer_normalize_line(char *line)
{
 int pos = 0;
 int len;

 if (kw_case_mode == KWCASE_MIXED) return;
 if (line == NULL) return;

 len = (int)strlen(line);
 if (len == 0) return;

 /* Skip leading whitespace */
 while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
 pos++;

 /* Skip line number */
 while (pos < len && line[pos] >= '0' && line[pos] <= '9')
 pos++;

 /* Skip whitespace after line number */
 while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
 pos++;

 /* Scan the rest of the line */
 while (pos < len) {
 char c = line[pos];

 /* Skip string literals (preserve case inside quotes) */
 if (c == '"') {
  pos++;
  while (pos < len && line[pos] != '"')
  pos++;
  if (pos < len) pos++; /* skip closing quote */
  continue;
 }

 /* Skip whitespace */
 if (c == ' ' || c == '\t') {
  pos++;
  continue;
 }

 /* Skip non-alpha characters (operators, digits, etc.) */
 if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
  pos++;
  continue;
 }

 /* Found an identifier - extract it */
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

  /* Check for $ suffix (string function/variable) */
  /* Don't include $ in the identifier for matching */

  /* Try to match as keyword */
  kw = match_keyword(line + start, ident_len);

  if (kw != KW_COUNT) {
  /* It's a keyword - apply case transformation */
  const char *canon = lexer_keyword_name(kw);
  int canon_len = (int)strlen(canon);
  int j;

  /* Only transform if lengths match (they should) */
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
     /* First char uppercase */
     line[start + j] = ch;
    } else {
     /* Rest lowercase */
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

  /* If keyword is REM, skip rest of line */
  if (kw == KW_REM) return;
  }
  /* If not a keyword, it's a variable - leave as-is */
 }
 }
}

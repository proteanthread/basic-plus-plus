 // ---
 // BASIC++ Interpreter - detok.c
 // ---
 //
 // Detokenizer implementation.
 //
 // DESIGN RATIONALE:
 // The detokenizer is intentionally independent of the parser and
 // executor. It only depends on the lexer (for token types and
 // the Lexer struct). This keeps it usable as a standalone tool.
 //
 // Each token type has a clear textual representation. Keywords
 // are looked up from a static table (shared concept with the
 // lexer, but the detokenizer has its own copy to remain
 // decoupled and standalone).
 //
//
// HOW TO EXTEND:
//   Adding support for a new statement in code generation:
//   1. Add the AST node type in ast.h.
//   2. Add the emit case in this file's switch statement.
//   3. Generate the corresponding C code output.
 // ---

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "detok.h"

// --- Keyword Name Table ---
 // Maps KeywordId values to their string representations.
 // Must be kept in sync with the KeywordId enum in lexer.h.
static const char *keyword_names[] = {
 "PRINT", // KW_PRINT
 "LET", // KW_LET
 "INPUT", // KW_INPUT
 "IF", // KW_IF
 "GOTO", // KW_GOTO
 "GOSUB", // KW_GOSUB
 "RETURN", // KW_RETURN
 "END", // KW_END
 "REM", // KW_REM
 "RUN", // KW_RUN
 "LIST", // KW_LIST
 "NEW", // KW_NEW
 "SAVE", // KW_SAVE
 "LOAD", // KW_LOAD
 "MERGE", // KW_MERGE
 "CHAIN", // KW_CHAIN
 "FOR", // KW_FOR
 "NEXT", // KW_NEXT
 "TO", // KW_TO
 "STEP", // KW_STEP
 "STOP", // KW_STOP
 "THEN", // KW_THEN
 "WHILE", // KW_WHILE
 "WEND", // KW_WEND
 "DO", // KW_DO
 "UNTIL", // KW_UNTIL
 "LOOP", // KW_LOOP
 "DATA", // KW_DATA
 "READ", // KW_READ
 "RESTORE", // KW_RESTORE
 "DIALECT", // KW_DIALECT
 "ABS", // KW_ABS
 "RND", // KW_RND
 "SIZE", // KW_SIZE
 "DIM", // KW_DIM
 "SIN", // KW_SIN
 "COS", // KW_COS
 "TAN", // KW_TAN
 "ATN", // KW_ATN
 "SQR", // KW_SQR
 "LOG", // KW_LOG_FUNC
 "EXP", // KW_EXP
 "INT", // KW_INT_FUNC
 "SGN", // KW_SGN
 "LEN", // KW_LEN
 "LEFT$", // KW_LEFT
 "RIGHT$", // KW_RIGHT
 "MID$", // KW_MID
 "CHR$", // KW_CHR
 "ASC", // KW_ASC
 "STR$", // KW_STR_FUNC
 "VAL", // KW_VAL_FUNC
 "COMPILE", // KW_COMPILE
 "CLS", // KW_CLS
 "CLEAR", // KW_CLEAR
 "CLR", // KW_CLR
 "TRON", // KW_TRON
 "TROFF", // KW_TROFF
 "ON", // KW_ON
 "ERROR", // KW_ERROR
 
 "DEF", // KW_DEF
 "FN", // KW_FN
 
 "MAT", // KW_MAT
 "LBOUND", // KW_LBOUND
 "UBOUND", // KW_UBOUND
 "DET", // KW_DET
 
 "OPEN", // KW_OPEN
 "CLOSE", // KW_CLOSE
 "AS", // KW_AS
 "EOF", // KW_EOF
 
 "BSAVE", // KW_BSAVE
 "BLOAD", // KW_BLOAD
 
 "MODULE", // KW_MODULE
 
 "SECURITY", // KW_SECURITY
 
 "SYSTEM", // KW_SYSTEM
 
 "BREAK", // KW_BREAK
 "CONT", // KW_CONT
 "VARS", // KW_VARS
 
 "ASSERT", // KW_ASSERT
 "TEST", // KW_TEST
 "ENDTEST", // KW_ENDTEST
 "SELFTEST", // KW_SELFTEST
 
 "HELP", // KW_HELP
 "INFO", // KW_INFO
 "CATALOG" /* KW_CATALOG */,
 
 "RENUM", // KW_RENUM
 "DELETE", // KW_DELETE
 "VER", // KW_VER
 // GW-BASIC compatibility
 "ELSE", // KW_ELSE
 "SWAP", // KW_SWAP
 "RANDOMIZE", // KW_RANDOMIZE
 "INSTR", // KW_INSTR
 "SPACE$", // KW_SPACE_FUNC
 "STRING$", // KW_STRING_FUNC
 "HEX$", // KW_HEX_FUNC
 "OCT$", // KW_OCT_FUNC
 "FIX", // KW_FIX
 "FILES", // KW_FILES
 "BEEP", // KW_BEEP
 "COLOR", // KW_COLOR
 "DIR", // KW_DIR
 "AUTO", // KW_AUTO
 "TAB", // KW_TAB_FUNC
 "SPC", // KW_SPC_FUNC
 "SOUND", // KW_SOUND
 "PLAY", // KW_PLAY
 "SCREEN", // KW_SCREEN
 "DRAW", // KW_DRAW
 "WIDTH", // KW_WIDTH
 "MOD", // KW_MOD
 "AND", // KW_AND
 "OR", // KW_OR
 "NOT", // KW_NOT
 "XOR", // KW_XOR
 "EQV", // KW_EQV
 "IMP", // KW_IMP
 "LCASE$", // KW_LCASE
 "UCASE$", // KW_UCASE
 "LTRIM$", // KW_LTRIM
 "RTRIM$", // KW_RTRIM
 "CINT", // KW_CINT
 "TIMER", // KW_TIMER
 "SLEEP", // KW_SLEEP
 "LOCATE", // KW_LOCATE
 "LINE", // KW_LINE
 "DATE$", // KW_DATE_FUNC
 "TIME$", // KW_TIME_FUNC
 // QBasic compatibility - Tier 1
 "SELECT", // KW_SELECT
 "CASE", // KW_CASE
 "END SELECT", // KW_ENDSELECT
 "IS", // KW_IS
 "CSNG", // KW_CSNG
 "CDBL", // KW_CDBL
 "EXIT", // KW_EXIT
 "CONST", // KW_CONST_KW
 // QBasic compatibility - Tier 2
 "INKEY$", // KW_INKEY
 "ERASE", // KW_ERASE
 "USING", // KW_USING
 "LPRINT", // KW_LPRINT
 // QBasic compatibility - Tier 3: SUB/FUNCTION
 "SUB", // KW_SUB
 "CALL", // KW_CALL
 "FUNCTION", // KW_FUNCTION
 // QBasic compatibility - Tier 4
 "SHELL", // KW_SHELL
 "REDIM", // KW_REDIM
 "SHARED", // KW_SHARED
 "STATIC", // KW_STATIC
 "RESUME", // KW_RESUME
 "OPTION", // KW_OPTION
 "ENVIRON$", // KW_ENVIRON
 "LOF", // KW_LOF
 "COMMON", // KW_COMMON
 "SEEK", // KW_SEEK
 "CHDIR", // KW_CHDIR
 // QBasic compatibility - Tier 5
 "PEEK", // KW_PEEK
 "POKE", // KW_POKE
 "SEG", // KW_SEG
 "PSET", // KW_PSET
 "CIRCLE", // KW_CIRCLE
 "PAINT", // KW_PAINT
 "PALETTE", // KW_PALETTE
 "POINT", // KW_POINT
 "TYPE", // KW_TYPE
 "ACCESS", // KW_ACCESS
 // GW-BASIC compatibility - additional
 "CSRLIN", // KW_CSRLIN
 "CVI", // KW_CVI
 "CVS", // KW_CVS
 "CVD", // KW_CVD
 "DEFINT", // KW_DEFINT
 "DEFDBL", // KW_DEFDBL
 "DEFSNG", // KW_DEFSNG
 "DEFSTR", // KW_DEFSTR
 "DEF USR", // KW_DEFUSR
 "ERL", // KW_ERL
 "ERR", // KW_ERR_VAR
 "EDIT", // KW_EDIT
 "EXTERR", // KW_EXTERR
 "ERDEV", // KW_ERDEV
 "FIELD", // KW_FIELD
 "FRE", // KW_FRE
 "GET", // KW_GET
 "PUT", // KW_PUT
 "INP", // KW_INP
 "INPUT$", // KW_INPUT_FUNC
 "IOCTL", // KW_IOCTL
 "IOCTL$", // KW_IOCTL_FUNC
 "KEY", // KW_KEY
 "KILL", // KW_KILL
 "LLIST", // KW_LLIST
 "LOC", // KW_LOC
 "LOCK", // KW_LOCK
 "LPOS", // KW_LPOS
 "LSET", // KW_LSET
 "MKDIR", // KW_MKDIR
 "MKD$", // KW_MKD_FUNC
 "MKI$", // KW_MKI_FUNC
 "MKS$", // KW_MKS_FUNC
 "ALIAS", // KW_ALIAS
 "NAME", // KW_NAME
 "RENAME", // KW_RENAME
 "OUT", // KW_OUT
 "COM", // KW_COM
 "PEN", // KW_PEN
 "STRIG", // KW_STRIG
 "PCOPY", // KW_PCOPY
 "PMAP", // KW_PMAP
 "POS", // KW_POS_FUNC
 "PRESET", // KW_PRESET
 "HOME", // KW_HOME
 "AT", // KW_AT
 "RESET", // KW_RESET
 "RMDIR", // KW_RMDIR
 "RSET", // KW_RSET
 "STICK", // KW_STICK
 "UNLOCK", // KW_UNLOCK
 "USR", // KW_USR
 "VARPTR", // KW_VARPTR
 "VARPTR$", // KW_VARPTR_STR
 "VIEW", // KW_VIEW
 "WAIT", // KW_WAIT
 "WINDOW", // KW_WINDOW
 "WRITE", // KW_WRITE
 "MEMMAP", // KW_MEMMAP
 "SYS", // KW_SYS
 "EXEC", // KW_EXEC
 "ERRORLEVEL", // KW_ERRORLEVEL
 "BYE", // KW_BYE
 // ECMA-116 Enhanced Files
 "SET", // KW_SET
 "ASK", // KW_ASK
 "REWRITE", // KW_REWRITE
 "POINTER", // KW_POINTER
 "FILESIZE", // KW_FILESIZE
 "TCASE$", // KW_TCASE
 "TRIM$", // KW_TRIM
 // Virtual subsystem introspection
 "VDEV", // KW_VDEV
 "VMEM", // KW_VMEM
 "VNET", // KW_VNET
 "VCON", // KW_VCON
 "VTERM", // KW_VTERM
 "VMACH", // KW_VMACH
 "DEVMAP", // KW_DEVMAP
 "BIN$", // KW_BIN_FUNC
 "CLOCK$", // KW_CLOCK_FUNC
 "ALARM$", // KW_ALARM_FUNC
 "DIALECT$", // KW_DIALECT_FUNC
 "MEMMAP$", // KW_MEMMAP_FUNC
 "ALIAS$", // KW_ALIAS_FUNC
 "SCOPE", // KW_SCOPE
 "KEYWORD", // KW_KEYWORD
 "OVERRIDE", // KW_OVERRIDE
 // Sinclair BASIC
 "BORDER", // KW_BORDER
 "PAPER", // KW_PAPER
 "INK", // KW_INK
 "BRIGHT", // KW_BRIGHT
 "FLASH", // KW_FLASH
 "INVERSE", // KW_INVERSE
 "OVER", // KW_OVER
 "PAUSE", // KW_PAUSE
 "DELAY", // KW_DELAY
 // SuperBASIC
 "REPEAT", // KW_REPEAT
 "ENDREPEAT", // KW_ENDREPEAT
 "ENDFOR", // KW_ENDFOR
 "REMAINDER", // KW_REMAINDER
 "DEFINE", // KW_DEFINE
 "PROCEDURE", // KW_PROCEDURE
 "LOCAL", // KW_LOCAL
 "ENDDEFINE", // KW_ENDDEFINE
 // Complex numbers
 "COMPLEX", // KW_COMPLEX
 "REAL", // KW_REAL_FUNC
 "IMAG", // KW_IMAG_FUNC
 // Aggregate / rounding math
 "MIN", // KW_MIN_FUNC
 "MAX", // KW_MAX_FUNC
 "AVG", // KW_AVG_FUNC
 "MED", // KW_MED_FUNC
 "ROUND", // KW_ROUND_FUNC
 // Extended math
 "ASIN", // KW_ASIN_FUNC
 "ACOS", // KW_ACOS_FUNC
 "SINH", // KW_SINH_FUNC
 "COSH", // KW_COSH_FUNC
 "TANH", // KW_TANH_FUNC
 "LOG10", // KW_LOG10_FUNC
 "LOG2", // KW_LOG2_FUNC
 "COMP", // KW_COMP_FUNC
 "PDIF", // KW_PDIF_FUNC
 "PI", // KW_PI_FUNC
 // SUPER BASIC modifiers
 "UNLESS", // KW_UNLESS
 "BY", // KW_BY
 "SCRATCH", // KW_SCRATCH
 "UNSAVE", // KW_UNSAVE
 // File management
 "COPY", // KW_COPY
 "MOVE", // KW_MOVE
 "PWD", // KW_PWD
 "CWD$", // KW_CWD_FUNC
 "EXIST", // KW_EXIST_FUNC
 "FILELEN", // KW_FILELEN_FUNC
 "TRAP", // KW_TRAP
 "IMAGE", // KW_IMAGE
 // Enhanced debugger commands
 "DEBUG", // KW_DEBUG
 "DUMP", // KW_DUMP
 "BACKTRACE", // KW_BACKTRACE
 "TRACE", // KW_TRACE
 // Complex number functions
 "CONJ", // KW_CONJ_FUNC
 "CABS", // KW_CABS_FUNC
 // Static analysis
 "CHECK", // KW_CHECK
 "VERIFY", // KW_VERIFY
 // File query functions
 "EXISTS", // KW_EXISTS
 "FILEMOD$", // KW_FILEMOD
 // Formatted I/O
 "DISPLAY", // KW_DISPLAY
 // Complex math functions
 "CSQR", // KW_CSQR_FUNC
 "CEXP", // KW_CEXP_FUNC
 "CLOG", // KW_CLOG_FUNC
 "CARG", // KW_CARG_FUNC
 "CPOW", // KW_CPOW_FUNC
 // Stream I/O primitives (Milestone 11)
 "SIOREAD$", // KW_SIOREAD
 "SIOREADLN$", // KW_SIOREADLN
 "SIOWRITE", // KW_SIOWRITE
 "SIOSEEK", // KW_SIOSEEK
 "SIOFLUSH", // KW_SIOFLUSH
 "SIOSTATUS", // KW_SIOSTATUS
 "SIOAVAIL", // KW_SIOAVAIL
 // Block I/O primitives (Milestone 11)
 "BIOREAD$", // KW_BIOREAD
 "BIOWRITE", // KW_BIOWRITE
 "BIOSTATUS", // KW_BIOSTATUS
 "BIOSIZE", // KW_BIOSIZE
 "BIOCHECKSUM", // KW_BIOCHECKSUM
 "BIOCOMPARE", // KW_BIOCOMPARE
 "BIOFILL", // KW_BIOFILL
 "BIOCOPY", // KW_BIOCOPY
 // Transaction / ATOMIC
 "ATOMIC", // KW_ATOMIC
 "TXN", // KW_TXN
 "COMMIT", // KW_COMMIT
 "ROLLBACK", // KW_ROLLBACK
 "TXNSTATUS", // KW_TXNSTATUS
 // Network builtins
 "NSTATUS", // KW_NSTATUS
 "NHTTPSTATUS", // KW_NHTTPSTATUS
 "NEOF", // KW_NEOF
 "NBYTESWAITING", // KW_NBYTESWAITING
 "NCONNECTED", // KW_NCONNECTED
 "NERROR", // KW_NERROR
 "NJSONQUERY$", // KW_NJSONQUERY
 "NINFO$", // KW_NINFO
 // Advanced string functions (Milestone 19)
 "REPLACE$", // KW_REPLACE
 "REVERSE$", // KW_REVERSE
 "MCASE$", // KW_MCASE
 "ICASE$", // KW_ICASE
 "ONKEY$", // KW_ONKEY
 "LIKE", // KW_LIKE
 "HASH$" // KW_HASH
};

// --- Token Type Names (for diagnostic output) ---
static const char *token_type_names[] = {
 "EOF", // TOK_EOF
 "NUMBER", // TOK_NUMBER
 "FLOAT_LIT", // TOK_FLOAT_LIT
 "STRING", // TOK_STRING
 "VARIABLE", // TOK_VARIABLE
 "STRING_VAR", // TOK_STRING_VAR
 "NAMED_VAR", // TOK_NAMED_VAR
 "KEYWORD", // TOK_KEYWORD
 "PLUS", // TOK_PLUS
 "MINUS", // TOK_MINUS
 "STAR", // TOK_STAR
 "SLASH", // TOK_SLASH
 "EQUALS", // TOK_EQUALS
 "LT", // TOK_LT
 "GT", // TOK_GT
 "LT_EQ", // TOK_LT_EQ
 "GT_EQ", // TOK_GT_EQ
 "NOT_EQ", // TOK_NOT_EQ
 "LPAREN", // TOK_LPAREN
 "RPAREN", // TOK_RPAREN
 "COMMA", // TOK_COMMA
 "SEMICOLON", // TOK_SEMICOLON
 "COLON", // TOK_COLON
 "HASH", // TOK_HASH
 "AT", // TOK_AT
 "CARET", // TOK_CARET
 "BACKSLASH", // TOK_BACKSLASH
 "PIPE", // TOK_PIPE
 "APPEND", // TOK_APPEND
 "IMAGINARY", // TOK_IMAGINARY
 "COMPLEX_VAR", // TOK_COMPLEX_VAR
 "CR" // TOK_CR
};

 // detok_token_to_string - Convert a single token to text.
 //
 // Writes the textual representation into buf. Examples:
 // TOK_NUMBER (42) -> "42"
 // TOK_KEYWORD (PRINT) -> "PRINT"
 // TOK_VARIABLE (A) -> "A"
 // TOK_STRING ("HELLO")-> "\"HELLO\""
 // TOK_PLUS -> "+"
 //
 // Returns characters written, or -1 if buffer too small.
int detok_token_to_string(const Token *tok, char *buf, int bufsize)
{
 int len = 0;

 if (buf == NULL || bufsize <= 0) {
 return -1;
 }

 switch (tok->type) {
 case TOK_NUMBER:
 len = sprintf(buf, "%ld", tok->value.num_value);
 break;

 case TOK_STRING:
 // Reconstruct quoted string
 if (tok->str_length + 3 > bufsize) return -1;
 buf[0] = '"';
 if (tok->str_start != NULL && tok->str_length > 0) {
 memcpy(buf + 1, tok->str_start,
 (size_t)tok->str_length);
 }
 buf[tok->str_length + 1] = '"';
 buf[tok->str_length + 2] = '\0';
 len = tok->str_length + 2;
 break;

 case TOK_VARIABLE:
 if (bufsize < 2) return -1;
 buf[0] = tok->value.var_name;
 buf[1] = '\0';
 len = 1;
 break;

 case TOK_FLOAT_LIT:
 len = sprintf(buf, "%g", tok->value.fval);
 break;

 case TOK_STRING_VAR:
 if (bufsize < 3) return -1;
 buf[0] = tok->value.var_name;
 buf[1] = '$';
 buf[2] = '\0';
 len = 2;
 break;

 case TOK_KEYWORD:
 if (tok->value.keyword >= 0 &&
 tok->value.keyword < KW_COUNT) {
 len = (int)strlen(keyword_names[tok->value.keyword]);
 if (len + 1 > bufsize) return -1;
 strcpy(buf, keyword_names[tok->value.keyword]);
 } else {
 if (bufsize < 2) return -1;
 buf[0] = '?';
 buf[1] = '\0';
 len = 1;
 }
 break;

 case TOK_NAMED_VAR:
 // Named variable - copy name from str_start/str_length
 {
 int nlen = tok->str_length;
 if (nlen > (int)sizeof(buf) - 1) nlen = (int)sizeof(buf) - 1;
 memcpy(buf, tok->str_start, (size_t)nlen);
 buf[nlen] = '\0';
 len = nlen;
 }
 break;

 case TOK_PLUS: len = sprintf(buf, "+"); break;
 case TOK_MINUS: len = sprintf(buf, "-"); break;
 case TOK_STAR: len = sprintf(buf, "*"); break;
 case TOK_SLASH: len = sprintf(buf, "/"); break;
 case TOK_EQUALS: len = sprintf(buf, "="); break;
 case TOK_LT: len = sprintf(buf, "<"); break;
 case TOK_GT: len = sprintf(buf, ">"); break;
 case TOK_LT_EQ: len = sprintf(buf, "<="); break;
 case TOK_GT_EQ: len = sprintf(buf, ">="); break;
 case TOK_NOT_EQ: len = sprintf(buf, "<>"); break;
 case TOK_LPAREN: len = sprintf(buf, "("); break;
 case TOK_RPAREN: len = sprintf(buf, ")"); break;
 case TOK_COMMA: len = sprintf(buf, ","); break;
 case TOK_SEMICOLON: len = sprintf(buf, ";"); break;
 case TOK_COLON: len = sprintf(buf, ":"); break;
 case TOK_HASH: len = sprintf(buf, "#"); break;
 case TOK_AT: len = sprintf(buf, "@"); break;
 case TOK_CR: len = sprintf(buf, "\\n"); break;
 case TOK_EOF: len = sprintf(buf, "<EOF>"); break;
 default: len = sprintf(buf, "???"); break;
 }

 return len;
}

 // detok_dump_line - Tokenize and dump all tokens in a line.
 //
 // This is a diagnostic function that shows exactly what the lexer
 // produces for a given input line. Output format:
 //
 // [TYPE] value
 //
 // Example output for "10 PRINT A+3":
 // [NUMBER] 10
 // [KEYWORD] PRINT
 // [VARIABLE] A
 // [PLUS] +
 // [NUMBER] 3
 // [EOF]
void detok_dump_line(const char *source)
{
 Lexer lex;
 char buf[MAX_LINE_LENGTH + 4];
 int type_idx;

 printf("DETOK: \"%s\"\n", source);

 lexer_init(&lex, source);

 while (lex.current.type != TOK_EOF) {
 type_idx = (int)lex.current.type;
 if (type_idx >= 0 &&
 type_idx < (int)(sizeof(token_type_names) /
 sizeof(token_type_names[0]))) {
 detok_token_to_string(&lex.current, buf, sizeof(buf));
 printf(" [%s] %s\n", token_type_names[type_idx], buf);
 } else {
 printf(" [UNKNOWN:%d]\n", type_idx);
 }
 lexer_next(&lex);
 }
 printf(" [EOF]\n");
}

// --- detok_keyword_name - Return keyword name string for a KeywordId. ---
 // Used by HELP to convert a bare keyword token back to a string
 // for topic lookup. Returns NULL if out of range.
const char *detok_keyword_name(KeywordId kw)
{
 int idx = (int)kw;
 int count = (int)(sizeof(keyword_names) /
 sizeof(keyword_names[0]));
 if (idx < 0 || idx >= count) return NULL;
 return keyword_names[idx];
}

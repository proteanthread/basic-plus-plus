/**
 * GW-BASIC / BASIC++ Core Engine (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Logic inside statement handlers to optimize behaviors.
 *    - Diagnostic logs and specific error message phrasing.
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables and execution loop structure.
 *    - Memory pool layouts.
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Core interpreter execution honoring C17 standards.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * -----------------------------------------------------------------------------
 */
#include "tokenizer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>



typedef struct {
    const char *str;
    uint16_t tok;
} GW_Keyword;


#define EXT_TOK(prefix, code) (((prefix) << 8) | (code))

static const GW_Keyword KEYWORDS[] = {
    { "END", TOK_END },
    { "FOR", TOK_FOR },
    { "NEXT", TOK_NEXT },
    { "DATA", TOK_DATA },
    { "INPUT", TOK_INPUT },
    { "DIM", TOK_DIM },
    { "READ", TOK_READ },
    { "LET", TOK_LET },
    { "GOTO", TOK_GOTO },
    { "RUN", TOK_RUN },
    { "IF", TOK_IF },
    { "RESTORE", TOK_RESTORE },
    { "GOSUB", TOK_GOSUB },
    { "RETURN", TOK_RETURN },
    { "REM", TOK_REM },
    { "STOP", TOK_STOP },
    { "PRINT", TOK_PRINT },
    { "DEF", TOK_DEF },
    { "DEFINT", TOK_DEFINT },
    { "DEFSNG", TOK_DEFSNG },
    { "DEFDBL", TOK_DEFDBL },
    { "DEFSTR", TOK_DEFSTR },
    { "POKE", TOK_POKE },
    { "WHILE", TOK_WHILE },
    { "WEND", TOK_WEND },
    { "SCREEN", TOK_SCREEN },
    { "COLOR", TOK_COLOR },
    { "LINE", TOK_LINE },
    { "CIRCLE", TOK_CIRCLE },
    { "PAINT", TOK_PAINT },
    { "PLAY", TOK_PLAY },
    { "SOUND", TOK_SOUND },
    { "BEEP", TOK_BEEP },
    { "SEG", TOK_SEG },
    { "THEN", TOK_THEN },
    { "ELSE", TOK_ELSE },
    { "TO", TOK_TO },
    { "STEP", TOK_STEP },
    { "USR", TOK_USR },
    { "CALL", TOK_CALL },
    { "NEW", TOK_NEW },
    { "LIST", TOK_LIST },
    { "SAVE", TOK_SAVE },
    { "LOAD", TOK_LOAD },
    { "CHAIN", TOK_CHAIN },
    { "MERGE", TOK_MERGE },
    { "PEEK", TOK_PEEK },
    { "SYSTEM", TOK_SYSTEM },
    { "CLS", TOK_CLS },
    { "CLEAR", TOK_CLEAR },
    { "WIDTH", TOK_WIDTH },
    { "CONSOLE", TOK_CONSOLE },
    { "FILES", TOK_FILES },
    { "KEY", TOK_KEY },
    { "LOCATE", TOK_LOCATE },
    { "ABS", TOK_ABS },
    { "INT", TOK_INT },
    { "RND", TOK_RND },
    { "SQR", TOK_SQR },
    { "CHR$", TOK_CHR },
    { "STRING$", TOK_STRING_FN },
    { "TAB", TOK_TAB },
    { "INKEY$", TOK_INKEY },
    { "ON", TOK_ON },
    { "ERROR", TOK_ERROR },
    { "RESUME", TOK_RESUME },
    { "ERR", TOK_ERR },
    { "ERL", TOK_ERL },
    { "AND", TOK_AND },
    { "OR", TOK_OR },
    { "NOT", TOK_NOT },
    { "XOR", TOK_XOR },
    { "MID$", TOK_MID },
    { "LEFT$", TOK_LEFT },
    { "RIGHT$", TOK_RIGHT },
    { "LEN", TOK_LEN },
    { "VAL", TOK_VAL },
    { "STR$", TOK_STR },
    { "ASC", TOK_ASC },
    { "INSTR", TOK_INSTR },
    { "SGN", TOK_SGN },
    { "SIN", TOK_SIN },
    { "COS", TOK_COS },
    { "TAN", TOK_TAN },
    { "ATN", TOK_ATN },
    { "EXP", TOK_EXP },
    { "LOG", TOK_LOG },
    { "POS", TOK_POS },
    { "GET", TOK_GET },
    { "PUT", TOK_PUT },
    { "DRAW", TOK_DRAW },
    { "PSET", TOK_PSET },
    { "PRESET", TOK_PRESET },
    { "MOD", TOK_MOD },
    { "CINT", TOK_CINT },
    { "CSNG", TOK_CSNG },
    { "CDBL", TOK_CDBL },
        { "FIX", TOK_FIX },
    // Extended BASIC++ Keywords
    { "AVG", TOK_AVG },
    { "SWAP", TOK_SWAP },
    { "MED", TOK_MED },
    { "ROUND", TOK_ROUND },
    { "CSQR", TOK_CSQR },
    { "CEXP", TOK_CEXP },
    { "CLOG", TOK_CLOG },
    { "CARG", TOK_CARG },
    { "CPOW", TOK_CPOW },
    { "SINH", TOK_SINH },
    { "COSH", TOK_COSH },
    { "TANH", TOK_TANH },
    { "LOG10", TOK_LOG10 },
    { "LOG2", TOK_LOG2 },
    { "COMP", TOK_COMP },
    { "PDIF", TOK_PDIF },
    { "PI", TOK_PI },
    { "COMPLEX", TOK_COMPLEX },
    { "REAL", TOK_REAL },
    { "IMAG", TOK_IMAG },
    { "CONJ", TOK_CONJ },
    { "CABS", TOK_CABS },
    { "ASIN", TOK_ASIN },
    { "ACOS", TOK_ACOS },
    { "LCASE", TOK_LCASE },
    { "UCASE", TOK_UCASE },
    { "TCASE", TOK_TCASE },
    { "TRIM", TOK_TRIM },
    { "REPLACE", TOK_REPLACE },
    { "REVERSE", TOK_REVERSE },
    { "MCASE", TOK_MCASE },
    { "ICASE", TOK_ICASE },
    { "ONKEY", TOK_ONKEY },
    { "LIKE", TOK_LIKE },
    { "LTRIM", TOK_LTRIM },
    { "RTRIM", TOK_RTRIM },
    { "ALARM", TOK_ALARM },
    { "SCOPE", TOK_SCOPE },
    { "VARS", TOK_VARS },
    { "OVERRIDE", TOK_OVERRIDE },
    { "ALIAS", TOK_ALIAS },
    { "KEYWORD", TOK_KEYWORD_BPP },
    { "LBOUND", TOK_LBOUND },
    { "UBOUND", TOK_UBOUND },
    { "ERASE", TOK_ERASE },
    { "REDIM", TOK_REDIM },
    { "MAT", TOK_MAT },
    { "DET", TOK_DET },
    { "BORDER", TOK_BORDER },
    { "PAPER", TOK_PAPER },
    { "INK", TOK_INK },
    { "BRIGHT", TOK_BRIGHT },
    { "FLASH", TOK_FLASH },
    { "INVERSE", TOK_INVERSE },
    { "OVER", TOK_OVER },
    { "COPY", TOK_COPY },
    { "BIOREAD", TOK_BIOREAD },
    { "BIOWRITE", TOK_BIOWRITE },
    { "BIOSTATUS", TOK_BIOSTATUS },
    { "BIOSIZE", TOK_BIOSIZE },
    { "BIOCHECKSUM", TOK_BIOCHECKSUM },
    { "BIOCOMPARE", TOK_BIOCOMPARE },
    { "BIOFILL", TOK_BIOFILL },
    { "BIOCOPY", TOK_BIOCOPY },
    { "SIOREAD", TOK_SIOREAD },
    { "SIOREADLN", TOK_SIOREADLN },
    { "SIOWRITE", TOK_SIOWRITE },
    { "SIOSEEK", TOK_SIOSEEK },
    { "SIOFLUSH", TOK_SIOFLUSH },
    { "SIOSTATUS", TOK_SIOSTATUS },
    { "SIOAVAIL", TOK_SIOAVAIL },
    { "VDEV", TOK_VDEV },
    { "VMEM", TOK_VMEM },
    { "DEVMAP", TOK_DEVMAP },
    { "CLOCK", TOK_CLOCK_FUNC },
    { "VNET", TOK_VNET },
    { "NSTATUS", TOK_NSTATUS },
    { "NHTTPSTATUS", TOK_NHTTPSTATUS },
    { "NEOF", TOK_NEOF },
    { "NBYTESWAITING", TOK_NBYTESWAITING },
    { "NCONNECTED", TOK_NCONNECTED },
    { "NERROR", TOK_NERROR },
    { "NJSONQUERY", TOK_NJSONQUERY },
    { "NINFO", TOK_NINFO },
    { "BIN", TOK_BIN },
    { "MOUNT", TOK_MOUNT },
    { "UMOUNT", TOK_UMOUNT },
    { "MOUNTS", TOK_MOUNTS },
    { "VPATH", TOK_VPATH },
    { "VCON", TOK_VCON },
    { "VTERM", TOK_VTERM },
    { "VMACH", TOK_VMACH },
    { "SYS", TOK_SYS },
    { "EXEC", TOK_EXEC },
    { "ERRORLEVEL", TOK_ERRORLEVEL },
    { "BYE", TOK_BYE },
    { "MODULE", TOK_MODULE },
    { "COMPILE", TOK_COMPILE },
    { "SECURITY", TOK_SECURITY },
    { "DIALECT", TOK_DIALECT },
    { "OPTION", TOK_OPTION },
    { "ENVIRON", TOK_ENVIRON },
    { "DECLARE", TOK_DECLARE },
    { "INFO", TOK_INFO },
    { "VER", TOK_VER },
    { "CATALOG", TOK_CATALOG },
    { "RENUM", TOK_RENUM },
    { "DELETE", TOK_DELETE },
    { "HELP", TOK_HELP },
    { "ATOMIC", TOK_ATOMIC },
    { "TXN", TOK_TXN },
    { "COMMIT", TOK_COMMIT },
    { "ROLLBACK", TOK_ROLLBACK },
    { "TXNSTATUS", TOK_TXNSTATUS },
    { "EXISTS", TOK_EXISTS },
    { "FILESIZE", TOK_FILESIZE },
    { "FILEMOD", TOK_FILEMOD },
    { "PWD", TOK_PWD },
    { "CWD", TOK_CWD },
    { "EXIST", TOK_EXIST },
    { "FILELEN", TOK_FILELEN },
    { "MOVE", TOK_MOVE },
    { "CHDIR", TOK_CHDIR },
    { "MKDIR", TOK_MKDIR },
    { "RMDIR", TOK_RMDIR },
    { "DEBUG", TOK_DEBUG },
    { "DUMP", TOK_DUMP },
    { "TRACE", TOK_TRACE },
    { "CHECK", TOK_CHECK },
    { "VERIFY", TOK_VERIFY },
    { "BACKTRACE", TOK_BACKTRACE },
    { "BSAVE", TOK_BSAVE },
    { "BLOAD", TOK_BLOAD },
    { "BRUN", TOK_BRUN },
    { "BREAK", TOK_BREAK },
    { "CONT", TOK_CONT },
    { "ASSERT", TOK_ASSERT },
    { "TEST", TOK_TEST },
    { "ENDTEST", TOK_ENDTEST },
    { "SELFTEST", TOK_SELFTEST },
    { "CAUSE", TOK_CAUSE },
    { "WHEN", TOK_WHEN },
    { "USE", TOK_USE },
    { "RETRY", TOK_RETRY },
    { "CONTINUE", TOK_CONTINUE },
    { "SET", TOK_SET },
    { "ASK", TOK_ASK },
    { "REWRITE", TOK_REWRITE },
    { "POINTER", TOK_POINTER },
    { "DELAY", TOK_DELAY },
    { "PAUSE", TOK_PAUSE },
    { "UNLESS", TOK_UNLESS },
    { "BY", TOK_BY },
    { "SCRATCH", TOK_SCRATCH },
    { "UNSAVE", TOK_UNSAVE },
    { "TRAP", TOK_TRAP },
    { "IMAGE", TOK_IMAGE },
    { "DO", TOK_DO },
    { "UNTIL", TOK_UNTIL },
    { "LOOP", TOK_LOOP },
    { "SELECT", TOK_SELECT },
    { "CASE", TOK_CASE },
    { "IS", TOK_IS },
    { "EXIT", TOK_EXIT },
    { "CONST", TOK_CONST },
    { "USING", TOK_USING },
    { "LPRINT", TOK_LPRINT },
    { "SUB", TOK_SUB },
    { "FUNCTION", TOK_FUNCTION },
    { "SHELL", TOK_SHELL },
    { "SHARED", TOK_SHARED },
    { "STATIC", TOK_STATIC },
    { "LOF", TOK_LOF },
    { "COMMON", TOK_COMMON },
    { "SEEK", TOK_SEEK },
    { "TYPE", TOK_TYPE },
    { "ACCESS", TOK_ACCESS },
    { "REPEAT", TOK_REPEAT },
    { "ENDREPEAT", TOK_ENDREPEAT },
    { "ENDFOR", TOK_ENDFOR },
    { "REMAINDER", TOK_REMAINDER },
    { "EXEC", TOK_EXEC },
    { "ENVIRON", TOK_ENVIRON },
    { "SANDBOX", TOK_SANDBOX },
    { "MODULE", TOK_MODULE },
    { "IMPORT", TOK_IMPORT },
    { "EXPORT", TOK_EXPORT },
    { "PLUGIN", TOK_PLUGIN },
    { "OPEN", TOK_OPEN_BPP },
    { "PROCEDURE", TOK_PROCEDURE },
    { "LOCAL", TOK_LOCAL },
    { "ENDDEFINE", TOK_ENDDEFINE },
    { "DISPLAY", TOK_DISPLAY },
    { "MEMMAP", TOK_MEMMAP },
    { "OPEN", TOK_OPEN_BPP },
    { "CLOSE", TOK_CLOSE_BPP },
    { "AS", TOK_AS_BPP },
    { "EOF", TOK_EOF_BPP }
};

static const int NUM_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

size_t gw_crunch(const char *input_text, uint8_t *output_tokens, size_t max_len) {
    size_t out_idx = 0;
    const char *p = input_text;
    
    while (*p && out_idx < max_len - 1) {
        // Handle whitespace
        if (isspace((unsigned char)*p)) {
            output_tokens[out_idx++] = *p++;
            continue;
        }
        
        // Handle String literals
        if (*p == '"') {
            output_tokens[out_idx++] = *p++;
            while (*p && *p != '"' && out_idx < max_len - 1) {
                output_tokens[out_idx++] = *p++;
            }
            if (*p == '"' && out_idx < max_len - 1) {
                output_tokens[out_idx++] = *p++;
            }
            continue;
        }
        
        // Handle single quote comment (')
        if (*p == '\'') {
            output_tokens[out_idx++] = TOK_REM;
            p++;
            while (*p && out_idx < max_len - 1) {
                output_tokens[out_idx++] = *p++;
            }
            continue;
        }
        
        // Check keywords (case-insensitive)
        int matched = 0;
        for (int i = 0; i < NUM_KEYWORDS; i++) {
            size_t kw_len = strlen(KEYWORDS[i].str);
            // Check if matches starting from p
            size_t j;
            for (j = 0; j < kw_len; j++) {
                if (toupper((unsigned char)p[j]) != KEYWORDS[i].str[j]) {
                    break;
                }
            }
            if (j == kw_len) {
                // Check if word boundary (so "PRINTING" doesn't match "PRINT")
                char next_c = p[kw_len];
                if (isalnum((unsigned char)next_c) || next_c == '_') {
                    continue;
                }
                
                // Also check previous character (so "MYSUB" doesn't match "SUB")
                if (p > input_text) {
                    char prev_c = *(p - 1);
                    if (isalnum((unsigned char)prev_c) || prev_c == '_') {
                        continue;
                    }
                }
                
                // Found match
                if (KEYWORDS[i].tok > 0xFF) {
                    output_tokens[out_idx++] = (KEYWORDS[i].tok >> 8) & 0xFF;
                    output_tokens[out_idx++] = KEYWORDS[i].tok & 0xFF;
                } else {
                    output_tokens[out_idx++] = KEYWORDS[i].tok & 0xFF;
                }
                p += kw_len;
                matched = 1;
                
                // Special: REM comment consumes the rest of the line
                if (KEYWORDS[i].tok == TOK_REM) {
                    while (*p && out_idx < max_len - 1) {
                        output_tokens[out_idx++] = *p++;
                    }
                }
                break;
            }
        }
        
        if (!matched) {
            output_tokens[out_idx++] = *p++;
        }
    }
    
    output_tokens[out_idx] = '\0';
    return out_idx;
}

void gw_list(const uint8_t *tokens, size_t len, char *output_text, size_t max_len) {
    size_t out_idx = 0;
    
    for (size_t i = 0; i < len && out_idx < max_len - 1; i++) {
        uint8_t t = tokens[i];
        if (t >= 0x80) {
            // Find keyword string
            const char *kw_str = NULL;
            for (int k = 0; k < NUM_KEYWORDS; k++) {
                if (KEYWORDS[k].tok == t) {
                    kw_str = KEYWORDS[k].str;
                    break;
                }
            }
            
            if (kw_str) {
                size_t kw_len = strlen(kw_str);
                if (out_idx + kw_len < max_len - 1) {
                    strcpy(output_text + out_idx, kw_str);
                    out_idx += kw_len;
                }
            } else {
                if (out_idx < max_len - 1) {
                    output_text[out_idx++] = '?';
                }
            }
        } else {
            output_text[out_idx++] = (char)t;
        }
    }
    
    output_text[out_idx] = '\0';
}

static const char *TOKENS_80[] = {
    /* 0x81 */ "END",
    /* 0x82 */ "FOR",
    /* 0x83 */ "NEXT",
    /* 0x84 */ "DATA",
    /* 0x85 */ "INPUT",
    /* 0x86 */ "DIM",
    /* 0x87 */ "READ",
    /* 0x88 */ "LET",
    /* 0x89 */ "GOTO",
    /* 0x8A */ "RUN",
    /* 0x8B */ "IF",
    /* 0x8C */ "RESTORE",
    /* 0x8D */ "GOSUB",
    /* 0x8E */ "RETURN",
    /* 0x8F */ "REM",
    /* 0x90 */ "STOP",
    /* 0x91 */ "PRINT",
    /* 0x92 */ "CLEAR",
    /* 0x93 */ "LIST",
    /* 0x94 */ "NEW",
    /* 0x95 */ "ON",
    /* 0x96 */ "WAIT",
    /* 0x97 */ "DEF",
    /* 0x98 */ "POKE",
    /* 0x99 */ "CONT",
    /* 0x9A */ NULL,
    /* 0x9B */ NULL,
    /* 0x9C */ "OUT",
    /* 0x9D */ "LPRINT",
    /* 0x9E */ "LLIST",
    /* 0x9F */ NULL,
    /* 0xA0 */ "WIDTH",
    /* 0xA1 */ "ELSE",
    /* 0xA2 */ "TRON",
    /* 0xA3 */ "TROFF",
    /* 0xA4 */ "SWAP",
    /* 0xA5 */ "ERASE",
    /* 0xA6 */ "EDIT",
    /* 0xA7 */ "ERROR",
    /* 0xA8 */ "RESUME",
    /* 0xA9 */ "DELETE",
    /* 0xAA */ "AUTO",
    /* 0xAB */ "RENUM",
    /* 0xAC */ "DEFSTR",
    /* 0xAD */ "DEFINT",
    /* 0xAE */ "DEFSNG",
    /* 0xAF */ "DEFDBL",
    /* 0xB0 */ "LINE",
    /* 0xB1 */ "WHILE",
    /* 0xB2 */ "WEND",
    /* 0xB3 */ "CALL",
    /* 0xB4 */ NULL,
    /* 0xB5 */ NULL,
    /* 0xB6 */ NULL,
    /* 0xB7 */ "WRITE",
    /* 0xB8 */ "OPTION",
    /* 0xB9 */ "RANDOMIZE",
    /* 0xBA */ "OPEN",
    /* 0xBB */ "CLOSE",
    /* 0xBC */ "LOAD",
    /* 0xBD */ "MERGE",
    /* 0xBE */ "SAVE",
    /* 0xBF */ "COLOR",
    /* 0xC0 */ "CLS",
    /* 0xC1 */ "MOTOR",
    /* 0xC2 */ "BSAVE",
    /* 0xC3 */ "BLOAD",
    /* 0xC4 */ "SOUND",
    /* 0xC5 */ "BEEP",
    /* 0xC6 */ "PSET",
    /* 0xC7 */ "PRESET",
    /* 0xC8 */ "SCREEN",
    /* 0xC9 */ "KEY",
    /* 0xCA */ "LOCATE",
    /* 0xCB */ NULL,
    /* 0xCC */ "TO",
    /* 0xCD */ "THEN",
    /* 0xCE */ "TAB(",
    /* 0xCF */ "STEP",
    /* 0xD0 */ "USR",
    /* 0xD1 */ "FN",
    /* 0xD2 */ "SPC(",
    /* 0xD3 */ "NOT",
    /* 0xD4 */ "ERL",
    /* 0xD5 */ "ERR",
    /* 0xD6 */ "STRING$",
    /* 0xD7 */ "USING",
    /* 0xD8 */ "INSTR",
    /* 0xD9 */ "'",
    /* 0xDA */ "VARPTR",
    /* 0xDB */ "CSRLIN",
    /* 0xDC */ "POINT",
    /* 0xDD */ "OFF",
    /* 0xDE */ "INKEY$",
    /* 0xDF */ NULL,
    /* 0xE0 */ NULL,
    /* 0xE1 */ NULL,
    /* 0xE2 */ NULL,
    /* 0xE3 */ NULL,
    /* 0xE4 */ NULL,
    /* 0xE5 */ NULL,
    /* 0xE6 */ ">",
    /* 0xE7 */ "=",
    /* 0xE8 */ "<",
    /* 0xE9 */ "+",
    /* 0xEA */ "-",
    /* 0xEB */ "*",
    /* 0xEC */ "/",
    /* 0xED */ "^",
    /* 0xEE */ "AND",
    /* 0xEF */ "OR",
    /* 0xF0 */ "XOR",
    /* 0xF1 */ "EQV",
    /* 0xF2 */ "IMP",
    /* 0xF3 */ "MOD",
    /* 0xF4 */ "\\",
    /* 0xF5 */ NULL,
    /* 0xF6 */ NULL,
    /* 0xF7 */ NULL,
    /* 0xF8 */ NULL,
    /* 0xF9 */ NULL,
    /* 0xFA */ NULL,
    /* 0xFB */ NULL,
    /* 0xFC */ NULL
};

static const char *TOKENS_FD[] = {
    /* 0x81 */ "CVI",
    /* 0x82 */ "CVS",
    /* 0x83 */ "CVD",
    /* 0x84 */ "MKI$",
    /* 0x85 */ "MKS$",
    /* 0x86 */ "MKD$",
    /* 0x87 */ NULL,
    /* 0x88 */ NULL,
    /* 0x89 */ NULL,
    /* 0x8A */ NULL,
    /* 0x8B */ "EXTERR"
};

static const char *TOKENS_FE[] = {
    /* 0x81 */ "FILES",
    /* 0x82 */ "FIELD",
    /* 0x83 */ "SYSTEM",
    /* 0x84 */ "NAME",
    /* 0x85 */ "LSET",
    /* 0x86 */ "RSET",
    /* 0x87 */ "KILL",
    /* 0x88 */ "PUT",
    /* 0x89 */ "GET",
    /* 0x8A */ "RESET",
    /* 0x8B */ "COMMON",
    /* 0x8C */ "CHAIN",
    /* 0x8D */ "DATE$",
    /* 0x8E */ "TIME$",
    /* 0x8F */ "PAINT",
    /* 0x90 */ "COM",
    /* 0x91 */ "CIRCLE",
    /* 0x92 */ "DRAW",
    /* 0x93 */ "PLAY",
    /* 0x94 */ "TIMER",
    /* 0x95 */ "ERDEV",
    /* 0x96 */ "IOCTL",
    /* 0x97 */ "CHDIR",
    /* 0x98 */ "MKDIR",
    /* 0x99 */ "RMDIR",
    /* 0x9A */ "SHELL",
    /* 0x9B */ "ENVIRON",
    /* 0x9C */ "VIEW",
    /* 0x9D */ "WINDOW",
    /* 0x9E */ "PMAP",
    /* 0x9F */ "PALETTE",
    /* 0xA0 */ "LCOPY",
    /* 0xA1 */ "CALLS",
    /* 0xA2 */ NULL,
    /* 0xA3 */ NULL,
    /* 0xA4 */ "NOISE",
    /* 0xA5 */ "PCOPY",
    /* 0xA6 */ "TERM",
    /* 0xA7 */ "LOCK",
    /* 0xA8 */ "UNLOCK"
};

static const char *TOKENS_FF[] = {
    /* 0x81 */ "LEFT$",
    /* 0x82 */ "RIGHT$",
    /* 0x83 */ "MID$",
    /* 0x84 */ "SGN",
    /* 0x85 */ "INT",
    /* 0x86 */ "ABS",
    /* 0x87 */ "SQR",
    /* 0x88 */ "RND",
    /* 0x89 */ "SIN",
    /* 0x8A */ "LOG",
    /* 0x8B */ "EXP",
    /* 0x8C */ "COS",
    /* 0x8D */ "TAN",
    /* 0x8E */ "ATN",
    /* 0x8F */ "FRE",
    /* 0x90 */ "INP",
    /* 0x91 */ "POS",
    /* 0x92 */ "LEN",
    /* 0x93 */ "STR$",
    /* 0x94 */ "VAL",
    /* 0x95 */ "ASC",
    /* 0x96 */ "CHR$",
    /* 0x97 */ "PEEK",
    /* 0x98 */ "SPACE$",
    /* 0x99 */ "OCT$",
    /* 0x9A */ "HEX$",
    /* 0x9B */ "LPOS",
    /* 0x9C */ "CINT",
    /* 0x9D */ "CSNG",
    /* 0x9E */ "CDBL",
    /* 0x9F */ "FIX",
    /* 0xA0 */ "PEN",
    /* 0xA1 */ "STICK",
    /* 0xA2 */ "STRIG",
    /* 0xA3 */ "EOF",
    /* 0xA4 */ "LOC",
    /* 0xA5 */ "LOF"
};

static double mbf_to_double(const uint8_t *bytes, bool is_double) {
    uint8_t exp = bytes[is_double ? 7 : 3];
    if (exp == 0) {
        return 0.0;
    }
    
    double sign = 1.0;
    uint8_t sign_byte = bytes[is_double ? 6 : 2];
    if (sign_byte & 0x80) {
        sign = -1.0;
    }
    
    double mantissa = 0.5;
    if (is_double) {
        uint32_t high_mant = ((bytes[6] & 0x7F) << 16) | (bytes[5] << 8) | bytes[4];
        uint32_t low_mant = ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[1] << 8) | bytes[0];
        double frac = (double)high_mant / 16777216.0 + (double)low_mant / 72057594037927936.0;
        mantissa += frac;
    } else {
        uint32_t mant = ((bytes[2] & 0x7F) << 16) | (bytes[1] << 8) | bytes[0];
        double frac = (double)mant / 16777216.0;
        mantissa += frac;
    }
    
    return sign * mantissa * ldexp(1.0, exp - 128);
}

static void format_double(double val, bool is_double, char *out, size_t out_size) {
    char buf[64];
    if (is_double) {
        snprintf(buf, sizeof(buf), "%.16g", val);
        for (int i = 0; buf[i]; i++) {
            if (buf[i] == 'e' || buf[i] == 'E') {
                buf[i] = 'D';
            }
        }
    } else {
        snprintf(buf, sizeof(buf), "%.7g", val);
    }
    snprintf(out, out_size, "%s", buf);
}

size_t gw_detokenize_binary(const uint8_t *bin, size_t bin_len, char *out, size_t max_len) {
    size_t out_idx = 0;
    size_t in_idx = 0;
    
    while (in_idx < bin_len && out_idx < max_len - 1) {
        uint8_t b = bin[in_idx];
        
        if (b == 0x00) {
            break;
        }
        
        if (b == 0x0B) {
            if (in_idx + 2 < bin_len) {
                uint16_t val = bin[in_idx + 1] | (bin[in_idx + 2] << 8);
                char tmp[32];
                sprintf(tmp, "&O%o", val);
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 3;
            } else {
                in_idx++;
            }
        } else if (b == 0x0C) {
            if (in_idx + 2 < bin_len) {
                uint16_t val = bin[in_idx + 1] | (bin[in_idx + 2] << 8);
                char tmp[32];
                sprintf(tmp, "&H%X", val);
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 3;
            } else {
                in_idx++;
            }
        } else if (b == 0x0D || b == 0x0E) {
            if (in_idx + 2 < bin_len) {
                uint16_t val = bin[in_idx + 1] | (bin[in_idx + 2] << 8);
                char tmp[32];
                sprintf(tmp, "%u", val);
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 3;
            } else {
                in_idx++;
            }
        } else if (b == 0x0F) {
            if (in_idx + 1 < bin_len) {
                uint8_t val = bin[in_idx + 1];
                char tmp[32];
                sprintf(tmp, "%u", val);
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 2;
            } else {
                in_idx++;
            }
        } else if (b >= 0x11 && b <= 0x1B) {
            uint8_t val = b - 0x11;
            char tmp[32];
            sprintf(tmp, "%u", val);
            size_t len = strlen(tmp);
            if (out_idx + len < max_len - 1) {
                strcpy(out + out_idx, tmp);
                out_idx += len;
            }
            in_idx++;
        } else if (b == 0x1C) {
            if (in_idx + 2 < bin_len) {
                int16_t val = bin[in_idx + 1] | (bin[in_idx + 2] << 8);
                char tmp[32];
                sprintf(tmp, "%d", val);
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 3;
            } else {
                in_idx++;
            }
        } else if (b == 0x1D) {
            if (in_idx + 4 < bin_len) {
                double val = mbf_to_double(&bin[in_idx + 1], false);
                char tmp[64];
                format_double(val, false, tmp, sizeof(tmp));
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 5;
            } else {
                in_idx++;
            }
        } else if (b == 0x1F) {
            if (in_idx + 8 < bin_len) {
                double val = mbf_to_double(&bin[in_idx + 1], true);
                char tmp[64];
                format_double(val, true, tmp, sizeof(tmp));
                size_t len = strlen(tmp);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, tmp);
                    out_idx += len;
                }
                in_idx += 9;
            } else {
                in_idx++;
            }
        } else if (b >= 0x20 && b <= 0x7E) {
            out[out_idx++] = (char)b;
            in_idx++;
        
        } else if (b == 0xFD || b == 0xFE || b == 0xFF) {
            if (in_idx + 1 < bin_len) {
                uint8_t next_b = bin[in_idx + 1];
                uint16_t full_tok = (b << 8) | next_b;
                
                // Check KEYWORDS table first for our custom extended tokens
                const char *kw = NULL;
                for (int k = 0; k < NUM_KEYWORDS; k++) {
                    if (KEYWORDS[k].tok == full_tok) {
                        kw = KEYWORDS[k].str;
                        break;
                    }
                }
                
                if (!kw) {
                    // Fallback to standard GW-BASIC extended tokens
                    if (b == 0xFD) {
                        if (next_b >= 0x81 && next_b <= 0x8B) {
                            kw = TOKENS_FD[next_b - 0x81];
                        }
                    } else if (b == 0xFE) {
                        if (next_b >= 0x81 && next_b <= 0xA8) {
                            kw = TOKENS_FE[next_b - 0x81];
                        }
                    } else if (b == 0xFF) {
                        if (next_b >= 0x81 && next_b <= 0xA5) {
                            kw = TOKENS_FF[next_b - 0x81];
                        }
                    }
                }
                
                if (kw) {
                    size_t len = strlen(kw);
                    if (out_idx + len < max_len - 1) {
                        strcpy(out + out_idx, kw);
                        out_idx += len;
                    }
                } else {
                    // Unknown extended token
                    if (out_idx < max_len - 1) out[out_idx++] = '?';
                    if (out_idx < max_len - 1) out[out_idx++] = '?';
                }
                in_idx += 2;
            } else {
                in_idx++;
            }
        } else if (b >= 0x81 && b <= 0xFC) {

            const char *kw = TOKENS_80[b - 0x81];
            if (kw) {
                if (b == 0xA1) {
                    if (out_idx > 0 && out[out_idx - 1] == ':') {
                        out_idx--;
                    }
                }
                if (b == 0xD9) {
                    if (out_idx >= 4 && strncmp(out + out_idx - 4, ":REM", 4) == 0) {
                        out_idx -= 4;
                    }
                }
                if (b == 0xB1 && in_idx + 1 < bin_len && bin[in_idx + 1] == 0xE9) {
                    in_idx += 2;
                    size_t len = strlen(kw);
                    if (out_idx + len < max_len - 1) {
                        strcpy(out + out_idx, kw);
                        out_idx += len;
                    }
                    continue;
                }
                
                size_t len = strlen(kw);
                if (out_idx + len < max_len - 1) {
                    strcpy(out + out_idx, kw);
                    out_idx += len;
                }
            }
            in_idx++;
        } else {
            if (out_idx < max_len - 1) {
                out[out_idx++] = b;
            }
            in_idx++;
        }
    }
    
    out[out_idx] = '\0';
    return out_idx;
}

bool gw_read_binary_line(FILE *f, uint8_t *temp, int max_len, int *out_len) {
    int idx = 0;
    // We check idx < max_len - 10 to ensure we have enough room for any constant (up to 8 bytes)
    while (idx < max_len - 10) {
        int c = fgetc(f);
        if (c == EOF) {
            return false;
        }
        if (c == 0x00) {
            temp[idx] = 0;
            *out_len = idx;
            return true;
        }
        temp[idx++] = (uint8_t)c;
        
        if (c == 0x0B || c == 0x0C || c == 0x0D || c == 0x0E || c == 0x1C) {
            for (int k = 0; k < 2; k++) {
                int val = fgetc(f);
                if (val == EOF) return false;
                temp[idx++] = (uint8_t)val;
            }
        } else if (c == 0x0F) {
            int val = fgetc(f);
            if (val == EOF) return false;
            temp[idx++] = (uint8_t)val;
        } else if (c == 0x1D) {
            for (int k = 0; k < 4; k++) {
                int val = fgetc(f);
                if (val == EOF) return false;
                temp[idx++] = (uint8_t)val;
            }
        } else if (c == 0x1F) {
            for (int k = 0; k < 8; k++) {
                int val = fgetc(f);
                if (val == EOF) return false;
                temp[idx++] = (uint8_t)val;
            }
        } else if (c == 0xFD || c == 0xFE || c == 0xFF) {
            int val = fgetc(f);
            if (val == EOF) return false;
            temp[idx++] = (uint8_t)val;
        }
    }
    return false;
}



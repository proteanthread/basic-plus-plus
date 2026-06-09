/*
 * =====================================================================
 * BASIC++ Interpreter - lexer.h
 * =====================================================================
 *
 * Lexer (tokenizer) interface.
 *
 * PURPOSE:
 *   Converts a line of BASIC source text into a stream of tokens.
 *   Tokens are value types (small structs) - they are not heap-
 *   allocated and do not consume scratch pool memory.
 *
 * HOW IT WORKS:
 *   1. Initialize the lexer with lexer_init() on a source line.
 *   2. Call lexer_next() to advance to each successive token.
 *   3. The current token is always available via lexer->current.
 *   4. The lexer is stateless between lines - reinitialize for
 *      each new line or statement.
 *
 * TOKEN LIFETIME:
 *   Tokens are valid only while the source string remains valid.
 *   String tokens (TOK_STRING) point directly into the source
 *   buffer (zero-copy). Do not modify the source while tokens
 *   reference it.
 *
 * CASE SENSITIVITY:
 *   Keywords are matched case-insensitively (PRINT = print = Print).
 *   Variable names are normalized to uppercase (a -> A).
 *
 * HOW TO EXTEND:
 *   To add new token types, add entries to TokenType and update
 *   lexer_next() in lexer.c. To add new keywords, add entries to
 *   KeywordId and the keyword_table[] in lexer.c.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_LEXER_H
#define BASICPP_LEXER_H

/* =====================================================================
 * Token Types
 * =====================================================================
 * Each token has a type that identifies what kind of lexical element
 * it represents. The types are grouped by category:
 *
 *   Atoms:      TOK_NUMBER, TOK_STRING, TOK_VARIABLE
 *   Keywords:   TOK_KEYWORD
 *   Operators:  TOK_PLUS through TOK_NOT_EQ
 *   Delimiters: TOK_LPAREN, TOK_RPAREN, TOK_COMMA, TOK_SEMICOLON
 *   Special:    TOK_HASH, TOK_AT, TOK_CR, TOK_EOF
 */
typedef enum TokenType {
    TOK_EOF = 0,     /* end of input */
    TOK_NUMBER,      /* integer literal (e.g., 42, 0, 32767) */
    TOK_FLOAT_LIT,   /* floating-point literal (e.g., 3.14, .5) */
    TOK_STRING,      /* quoted string literal (e.g., "HELLO") */
    TOK_VARIABLE,    /* single-letter variable (A-Z) */
    TOK_STRING_VAR,  /* string variable (A$-Z$) */
    TOK_NAMED_VAR,   /* multi-character variable (Phase 3) */
    TOK_KEYWORD,     /* reserved word (PRINT, LET, GOTO, etc.) */
    TOK_PLUS,        /* + */
    TOK_MINUS,       /* - */
    TOK_STAR,        /* * */
    TOK_SLASH,       /* / */
    TOK_EQUALS,      /* = */
    TOK_LT,          /* < */
    TOK_GT,          /* > */
    TOK_LT_EQ,      /* <= */
    TOK_GT_EQ,      /* >= */
    TOK_NOT_EQ,      /* <> or # (as relop) */
    TOK_LPAREN,      /* ( */
    TOK_RPAREN,      /* ) */
    TOK_COMMA,       /* , */
    TOK_SEMICOLON,   /* ; (statement separator in PATB) */
    TOK_COLON,       /* : (statement separator in most dialects) */
    TOK_HASH,        /* # (PRINT format specifier) */
    TOK_AT,          /* @ (array prefix) */
    TOK_CARET,       /* ^ (exponentiation) */
    TOK_BACKSLASH,   /* \ (integer division) */
    TOK_PIPE,        /* | (pipe operator) */
    TOK_APPEND,      /* >> (append redirect) */
    TOK_CR           /* end of line / carriage return */
} TokenType;

/* =====================================================================
 * Keyword Identifiers
 * =====================================================================
 * Each recognized keyword has a unique identifier. The lexer looks
 * up identifiers in the keyword table and assigns the matching
 * KeywordId. Unknown identifiers are treated as variables.
 *
 * Keywords listed here include both Phase 1 (implemented now) and
 * keywords reserved for future phases. Reserving them now prevents
 * them from being used as variable names in future expansions.
 */
typedef enum KeywordId {
    KW_PRINT = 0,
    KW_LET,
    KW_INPUT,
    KW_IF,
    KW_GOTO,
    KW_GOSUB,
    KW_RETURN,
    KW_END,
    KW_REM,
    KW_RUN,
    KW_LIST,
    KW_NEW,
    KW_SAVE,
    KW_LOAD,
    KW_MERGE,
    KW_CHAIN,
    KW_FOR,
    KW_NEXT,
    KW_TO,
    KW_STEP,
    KW_STOP,
    KW_THEN,
    KW_WHILE,      /* Phase 2: WHILE condition */
    KW_WEND,       /* Phase 2: WHILE loop end */
    KW_DO,         /* Phase 2: DO loop start */
    KW_UNTIL,      /* Phase 2: UNTIL condition */
    KW_LOOP,       /* Phase 2: DO loop end */
    KW_DATA,       /* Phase 3: DATA constants */
    KW_READ,       /* Phase 3: READ from DATA */
    KW_RESTORE,    /* Phase 3: reset DATA pointer */
    KW_DIALECT,    /* Phase 3: switch dialect at runtime */
    KW_ABS,
    KW_RND,
    KW_SIZE,
    /* Phase 4: DIM arrays */
    KW_DIM,
    /* Phase 4: Float math functions */
    KW_SIN,
    KW_COS,
    KW_TAN,
    KW_ATN,
    KW_SQR,
    KW_LOG_FUNC,   /* LOG (named differently to avoid macro conflict) */
    KW_EXP,
    KW_INT_FUNC,   /* INT (named differently from int type) */
    KW_SGN,
    /* Phase 4: String functions */
    KW_LEN,
    KW_LEFT,
    KW_RIGHT,
    KW_MID,
    KW_CHR,
    KW_ASC,
    KW_STR_FUNC,   /* STR$ */
    KW_VAL_FUNC,   /* VAL */
    /* Phase 5: Compiler */
    KW_COMPILE,    /* COMPILE command */
    /* Phase 6: Virtual devices and core commands */
    KW_CLS,        /* CLS - clear screen */
    KW_CLEAR,      /* CLEAR - clear stack */
    KW_CLR,        /* CLR - clear variables/arrays */
    KW_TRON,       /* TRON - trace on */
    KW_TROFF,      /* TROFF - trace off */
    KW_ON,         /* ON - ON ERROR GOTO */
    KW_ERROR,      /* ERROR - used with ON */
    /* Phase 9: User-defined functions */
    KW_DEF,        /* DEF - define function */
    KW_FN,         /* FN - invoke user function */
    /* Phase 10: Matrix operations */
    KW_MAT,        /* MAT - matrix operations */
    /* Phase 11: File I/O channels */
    KW_OPEN,       /* OPEN - open file channel */
    KW_CLOSE,      /* CLOSE - close file channel */
    KW_AS,         /* AS - channel specifier in OPEN */
    KW_EOF,        /* EOF - end-of-file check function */
    /* Phase 13: Bytecode format */
    KW_BSAVE,      /* BSAVE - save program as bytecode */
    KW_BLOAD,      /* BLOAD - load program from bytecode */
    /* Phase 14: Module system */
    KW_MODULE,     /* MODULE - module management command */
    /* Phase 15: Security */
    KW_SECURITY,   /* SECURITY - security level management */
    /* Phase 16: Cross-platform */
    KW_SYSTEM,     /* SYSTEM - platform info command */
    /* Phase 17: Interactive debugger */
    KW_BREAK,      /* BREAK - set/clear/list breakpoints */
    KW_CONT,       /* CONT - continue from pause */
    KW_VARS,       /* VARS - dump variable state */
    /* Phase 18: Self-test framework */
    KW_ASSERT,     /* ASSERT - test assertion */
    KW_TEST,       /* TEST - start test block */
    KW_ENDTEST,    /* ENDTEST - end test block */
    KW_SELFTEST,   /* SELFTEST - built-in validation */
    /* Phase 19: Help & introspection */
    KW_HELP,       /* HELP - command reference */
    KW_INFO,       /* INFO - system information */
    KW_CATALOG,    /* CATALOG - list functions */
    /* Phase 20: Final polish */
    KW_RENUM,      /* RENUM - renumber program lines */
    KW_DELETE,     /* DELETE - delete line range */
    KW_VER,        /* VER - show version */
    /* GW-BASIC compatibility */
    KW_ELSE,       /* ELSE - IF/THEN/ELSE branch */
    KW_ELSEIF,     /* ELSEIF - block IF alternative */
    KW_ENDIF,      /* ENDIF / END IF - block IF terminator */
    KW_CAUSE,      /* CAUSE - CAUSE EXCEPTION n (ECMA-116) */
    KW_WHEN,       /* WHEN - WHEN EXCEPTION IN (ECMA-116) */
    KW_USE,        /* USE - exception handler block (ECMA-116) */
    KW_RETRY,      /* RETRY - re-enter protected block (ECMA-116) */
    KW_CONTINUE,   /* CONTINUE - resume after error (ECMA-116) */
    KW_DECLARE,    /* DECLARE - DECLARE EXTERNAL SUB/FUNCTION (ECMA-116) */
    KW_SWAP,       /* SWAP - exchange two variables */
    KW_RANDOMIZE,  /* RANDOMIZE - seed RNG */
    KW_INSTR,      /* INSTR - find substring */
    KW_SPACE_FUNC, /* SPACE$ - N spaces */
    KW_STRING_FUNC,/* STRING$ - N copies of char */
    KW_HEX_FUNC,   /* HEX$ - hex conversion */
    KW_OCT_FUNC,   /* OCT$ - octal conversion */
    KW_FIX,        /* FIX - truncate toward zero */
    KW_FILES,      /* FILES - list directory */
    KW_BEEP,       /* BEEP - audible beep */
    KW_COLOR,      /* COLOR - set text color */
    KW_DIR,        /* DIR - list filenames only */
    KW_AUTO,       /* AUTO - auto line numbering */
    KW_TAB_FUNC,   /* TAB - PRINT tab position */
    KW_SPC_FUNC,   /* SPC - PRINT spaces */
    KW_SOUND,      /* SOUND - play tone */
    KW_PLAY,       /* PLAY - music macro language */
    KW_SCREEN,     /* SCREEN - set screen mode */
    KW_DRAW,       /* DRAW - graphics macro language */
    KW_WIDTH,      /* WIDTH - set screen width */
    KW_MOD,        /* MOD - modulo operator */
    KW_AND,        /* AND - logical/bitwise AND */
    KW_OR,         /* OR - logical/bitwise OR */
    KW_NOT,        /* NOT - logical/bitwise NOT */
    KW_XOR,        /* XOR - bitwise XOR */
    KW_EQV,        /* EQV - logical equivalence */
    KW_IMP,        /* IMP - logical implication */
    KW_LCASE,      /* LCASE$ - lowercase string */
    KW_UCASE,      /* UCASE$ - uppercase string */
    KW_LTRIM,      /* LTRIM$ - trim left spaces */
    KW_RTRIM,      /* RTRIM$ - trim right spaces */
    KW_CINT,       /* CINT - convert to integer */
    KW_TIMER,      /* TIMER - seconds since midnight */
    KW_SLEEP,      /* SLEEP - pause execution */
    KW_LOCATE,     /* LOCATE - cursor position */
    KW_LINE,       /* LINE - LINE INPUT */
    KW_DATE_FUNC,  /* DATE$ - current date string */
    KW_TIME_FUNC,  /* TIME$ - current time string */
    /* QBasic compatibility - Tier 1 */
    KW_SELECT,     /* SELECT - SELECT CASE */
    KW_CASE,       /* CASE - case clause */
    KW_ENDSELECT,  /* END SELECT sentinel (internal) */
    KW_IS,         /* IS - CASE IS > x */
    KW_CSNG,       /* CSNG - convert to single */
    KW_CDBL,       /* CDBL - convert to double */
    KW_EXIT,       /* EXIT - EXIT FOR / EXIT DO */
    KW_CONST_KW,   /* CONST - named constants */
    /* QBasic compatibility - Tier 2 */
    KW_INKEY,      /* INKEY$ - non-blocking input */
    KW_ERASE,      /* ERASE - clear arrays */
    KW_USING,      /* USING - PRINT USING */
    KW_LPRINT,     /* LPRINT - print to stderr */
    /* QBasic compatibility - Tier 3: SUB/FUNCTION */
    KW_SUB,        /* SUB - subprogram definition */
    KW_CALL,       /* CALL - invoke subprogram */
    KW_FUNCTION,   /* FUNCTION - function definition */
    /* QBasic compatibility - Tier 4: Missing features */
    KW_SHELL,      /* SHELL - execute OS command */
    KW_REDIM,      /* REDIM - resize dynamic array */
    KW_SHARED,     /* SHARED - share vars with main */
    KW_STATIC,     /* STATIC - preserve vars across calls */
    KW_RESUME,     /* RESUME - return from error handler */
    KW_OPTION,     /* OPTION - OPTION BASE */
    KW_ENVIRON,    /* ENVIRON$ - environment variable */
    KW_LOF,        /* LOF - file length function */
    KW_COMMON,     /* COMMON - share vars with CHAIN */
    KW_SEEK,       /* SEEK - file position */
    KW_CHDIR,      /* CHDIR - change directory */
    /* QBasic compatibility - Tier 5: Final features */
    KW_PEEK,       /* PEEK - read virtual memory byte */
    KW_POKE,       /* POKE - write virtual memory byte */
    KW_SEG,        /* SEG - used in DEF SEG */
    KW_PSET,       /* PSET - set pixel */
    KW_CIRCLE,     /* CIRCLE - draw circle */
    KW_PAINT,      /* PAINT - flood fill */
    KW_PALETTE,    /* PALETTE - remap color */
    KW_POINT,      /* POINT - read pixel color */
    KW_TYPE,       /* TYPE - user-defined type */
    KW_ACCESS,     /* ACCESS - file access mode */

    /* GW-BASIC compatibility - additional */
    KW_CSRLIN,     /* CSRLIN - current cursor line */
    KW_CVI,        /* CVI - convert string to integer */
    KW_CVS,        /* CVS - convert string to single */
    KW_CVD,        /* CVD - convert string to double */
    KW_DEFINT,     /* DEFINT - declare integer range */
    KW_DEFDBL,     /* DEFDBL - declare double range */
    KW_DEFSNG,     /* DEFSNG - declare single range */
    KW_DEFSTR,     /* DEFSTR - declare string range */
    KW_DEFUSR,     /* DEF USR - user function address */
    KW_ERL,        /* ERL - last error line number */
    KW_ERR_VAR,    /* ERR - last error code */
    KW_EDIT,       /* EDIT - interactive line editor */
    KW_EXTERR,     /* EXTERR - extended error (DOS) */
    KW_ERDEV,      /* ERDEV - device error */
    KW_FIELD,      /* FIELD - random file buffer fields */
    KW_FRE,        /* FRE - free memory function */
    KW_GET,        /* GET - file read / graphics capture */
    KW_PUT,        /* PUT - file write / graphics put */
    KW_INP,        /* INP - read I/O port */
    KW_INPUT_FUNC, /* INPUT$ - read n chars from keyboard/file */
    KW_IOCTL,      /* IOCTL - I/O control for device driver */
    KW_IOCTL_FUNC, /* IOCTL$ - read I/O control string */
    KW_KEY,        /* KEY - function key assignment */
    KW_KILL,       /* KILL - delete a file */
    KW_LLIST,      /* LLIST - list program to printer */
    KW_LOC,        /* LOC - file position */
    KW_LOCK,       /* LOCK - lock file records */
    KW_LPOS,       /* LPOS - printer head position */
    KW_LSET,       /* LSET - left-justify in field buffer */
    KW_MKDIR,      /* MKDIR - create directory */
    KW_MKD_FUNC,   /* MKD$ - convert double to string */
    KW_MKI_FUNC,   /* MKI$ - convert integer to string */
    KW_MKS_FUNC,   /* MKS$ - convert single to string */
    KW_ALIAS,      /* ALIAS - remap keyword names */
    KW_NAME,       /* NAME - rename a file */
    KW_RENAME,     /* RENAME - rename .BAS/.BPP file */
    KW_OUT,        /* OUT - write to I/O port */
    KW_COM,        /* COM - serial port event */
    KW_PEN,        /* PEN - light pen event */
    KW_STRIG,      /* STRIG - joystick trigger event */
    KW_PCOPY,      /* PCOPY - copy display page */
    KW_PMAP,       /* PMAP - map physical/view coords */
    KW_POS_FUNC,   /* POS - cursor column position */
    KW_PRESET,     /* PRESET - reset pixel */
    KW_HOME,       /* HOME - cursor to top-left */
    KW_AT,         /* AT - cursor positioning */
    KW_RESET,      /* RESET - close all files */
    KW_RMDIR,      /* RMDIR - remove directory */
    KW_RSET,       /* RSET - right-justify string in field */
    KW_STICK,      /* STICK - joystick position */
    KW_UNLOCK,     /* UNLOCK - release file lock */
    KW_USR,        /* USR - call machine language */
    KW_VARPTR,     /* VARPTR - variable pointer */
    KW_VARPTR_STR, /* VARPTR$ - variable ptr string */
    KW_VIEW,       /* VIEW - set graphics viewport */
    KW_WAIT,       /* WAIT - wait for port status */
    KW_WINDOW,     /* WINDOW - set logical coordinates */
    KW_WRITE,      /* WRITE - output data with delimiters */
    KW_MEMMAP,     /* MEMMAP - select memory map preset */
    KW_SYS,        /* SYS - call machine routine (C64) */
    KW_EXEC,       /* EXEC - fire-and-forget command */
    KW_ERRORLEVEL, /* ERRORLEVEL - last shell exit code */
    KW_BYE,        /* BYE - exit interpreter */
    /* ECMA-116 Phase 5: Enhanced Files Module */
    KW_SET,        /* SET - file pointer control (ECMA-116) */
    KW_ASK,        /* ASK - file info query (ECMA-116) */
    KW_REWRITE,    /* REWRITE - replace record (ECMA-116) */
    KW_POINTER,    /* POINTER - SET/ASK sub-keyword (ECMA-116) */
    KW_FILESIZE,   /* FILESIZE - ASK sub-keyword (ECMA-116) */
    KW_COUNT       /* sentinel - must be last */
} KeywordId;

/* =====================================================================
 * Token - A single lexical element.
 * =====================================================================
 * Tokens are value types, small enough to be passed by value or
 * stored on the C stack. No dynamic allocation required.
 *
 * For TOK_NUMBER: value.num_value contains the integer value.
 * For TOK_VARIABLE: value.var_name contains 'A'-'Z'.
 * For TOK_KEYWORD: value.keyword contains the KeywordId.
 * For TOK_STRING: str_start points into the source buffer and
 *   str_length gives the length (excluding quotes).
 */
typedef struct Token {
    TokenType type;
    union {
        long      num_value;   /* TOK_NUMBER: integer value */
        double    fval;        /* TOK_FLOAT_LIT: float value */
        char      var_name;    /* TOK_VARIABLE / TOK_STRING_VAR */
        KeywordId keyword;     /* TOK_KEYWORD: keyword identifier */
    } value;
    const char *str_start;     /* TOK_STRING: pointer into source */
    int         str_length;    /* TOK_STRING: length of string content */
} Token;

/* =====================================================================
 * Lexer - Tokenizer state.
 * =====================================================================
 * Maintains the current position within a source line and the most
 * recently produced token. The lexer scans left-to-right, one
 * character at a time, producing tokens on demand.
 *
 * Fields:
 *   source  - pointer to the source line (not owned by lexer).
 *   pos     - current scan position (0-based index).
 *   length  - total length of the source line.
 *   current - the most recently lexed token.
 */
typedef struct Lexer {
    const char *source;
    int         pos;
    int         length;
    Token       current;
} Lexer;

/* =====================================================================
 * Lexer Functions
 * =====================================================================
 */

/*
 * lexer_init - Initialize the lexer on a source line.
 *
 * Sets up the lexer to tokenize the given source string. Calls
 * lexer_next() internally to prime the first token.
 *
 * Parameters:
 *   lex    - pointer to the Lexer to initialize.
 *   source - null-terminated source line to tokenize.
 *            The string must remain valid while the lexer is in use.
 */
void lexer_init(Lexer *lex, const char *source);

/*
 * lexer_next - Advance to the next token.
 *
 * Scans the source from the current position, skipping whitespace,
 * and produces the next token in lex->current. At end of input,
 * produces TOK_EOF (and continues returning TOK_EOF on subsequent
 * calls).
 */
void lexer_next(Lexer *lex);

/*
 * lexer_peek_type - Return the type of the current token.
 *
 * Convenience function: returns lex->current.type without
 * advancing the lexer.
 */
TokenType lexer_peek_type(Lexer *lex);

/*
 * lexer_expect - Consume a token of the expected type.
 *
 * If the current token matches 'expected', advances the lexer
 * (calls lexer_next) and returns 1. Otherwise raises ERR_WHAT
 * and returns 0.
 *
 * Used by the parser to enforce syntax rules.
 */
int lexer_expect(Lexer *lex, TokenType expected);

/*
 * lexer_match_keyword - Check if current token is a specific keyword.
 *
 * Returns 1 if the current token is TOK_KEYWORD with the given
 * KeywordId, 0 otherwise. Does not advance the lexer.
 */
int lexer_match_keyword(Lexer *lex, KeywordId kw);

/*
 * lexer_skip_to_end - Skip all remaining tokens to end of line.
 *
 * Used by REM to skip the comment text, and by error recovery
 * to discard the rest of a malformed line.
 */
void lexer_skip_to_end(Lexer *lex);

/*
 * lexer_get_keyword_flags - Return dialect bitmask for a keyword.
 *
 * Returns the DFLAG_* bitmask indicating which dialects support
 * the specified keyword. Returns DFLAG_ALL (0xFFFF) for unknown
 * keywords or BASIC++ native commands.
 */
unsigned int lexer_get_keyword_flags(KeywordId kw);

/* =====================================================================
 * Keyword Alias API
 * =====================================================================
 * Allows runtime remapping of keyword names.  For example:
 *   ALIAS PRINT = "IMPRE"
 * makes the identifier IMPRE behave exactly like PRINT.
 */

#define MAX_ALIASES 64
#define MAX_ALIAS_NAME 32

/*
 * lexer_add_alias - Register an alias for a keyword.
 *
 * 'name' is the new identifier (e.g. "IMPRE"), stored
 * upper-cased.  'target' is the keyword it maps to.
 * Returns 0 on success, -1 if table is full.
 */
int lexer_add_alias(const char *name, int name_len,
                    KeywordId target);

/*
 * lexer_clear_aliases - Remove all aliases.
 */
void lexer_clear_aliases(void);

/*
 * lexer_list_aliases - Print all active aliases to stdout.
 */
void lexer_list_aliases(void);

/*
 * lexer_alias_count - Return number of active aliases.
 */
int lexer_alias_count(void);

/*
 * lexer_keyword_needs_dollar - Check if a keyword uses
 * a trailing $ (e.g. LEFT$, MID$, CHR$).
 * Returns 1 if yes, 0 if no.
 */
int lexer_keyword_needs_dollar(KeywordId kw);

/*
 * lexer_keyword_name - Return the canonical name string
 * for a keyword.  Returns "" if not found.
 */
const char *lexer_keyword_name(KeywordId kw);

#endif /* BASICPP_LEXER_H */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_lexer.h
 * @brief Ephemeral Lexer/Tokenizer API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares types and functions for the scanner, which converts a BASIC source text line
 *   into a stream of tokens ephemerally.
 * - Why it exists: Tokenizes input line-by-line without permanently storing tokens in memory, keeping
 *   memory overhead negligible.
 * - Why it works this way: It reads characters from a source buffer, skipping whitespace (except in strings
 *   or remarks) and returning tokens referencing slices of the original source text.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Token enum values, adding new keyword mappings, changing operator symbols.
 * - What cannot be changed: Opaque LexerContext pointer.
 * - What to expect: Calling lex_next advances the cursor, while lex_peek returns the next token without advancing.
 * - What to do if something breaks: If keywords are misidentified as identifiers, check the uppercase conversion
 *   and keyword table match.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Input source is ASCII encoded. EOF is represented by character '\0'.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional keywords to BppKeywordId and token codes to BppTokenType.
 * - How to write external extensions: Custom dialects register alias keyword strings that map to these base IDs.
 */

#ifndef BPP_LEXER_H
#define BPP_LEXER_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "memory/memory.h"

/* Ephemeral Token Types */
typedef enum {
    TOK_EOF = 0,
    TOK_EOL,            /* Newline or statement separator ':' */
    TOK_NUMBER,         /* Numeric constant */
    TOK_STRING,         /* String literal inside double quotes */
    TOK_RPN_LITERAL,    /* RPN literal inside curly braces */
    TOK_IDENT,          /* Identifier (variable name or label reference) */
    TOK_KEYWORD,        /* Built-in language statement/command */
    /* Operators */
    TOK_PLUS,           /* + */
    TOK_MINUS,          /* - */
    TOK_MUL,            /* * */
    TOK_DIV,            /* / */
    TOK_EQ,             /* = */
    TOK_NE,             /* <> */
    TOK_LT,             /* < */
    TOK_GT,             /* > */
    TOK_LE,             /* <= */
    TOK_GE,             /* >= */
    TOK_AND,            /* AND */
    TOK_OR,             /* OR */
    TOK_NOT,            /* NOT */
    TOK_XOR,            /* XOR */
    /* Symbols */
    TOK_LPAREN,         /* ( */
    TOK_RPAREN,         /* ) */
    TOK_COMMA,          /* , */
    TOK_SEMICOLON,      /* ; */
    TOK_HASH,           /* # */
    TOK_UNARY_MINUS,    /* Internal unary minus */
    TOK_UNARY_PLUS,     /* Internal unary plus */
    TOK_DIRECTIVE,      /* ::DIRECTIVE */
    TOK_GLOBAL_LABEL,   /* ::label: */
    TOK_NAMESPACE_DECL, /* ::[namespace] */
    TOK_DOCSTRING,      /* // docstring */
    TOK_DOUBLE_COLON,   /* :: */
    TOK_PERIOD,         /* . */
    TOK_LBRACKET,       /* [ */
    TOK_RBRACKET,       /* ] */
    TOK_UNKNOWN
} BppTokenType;

/* Keyword Identifiers */
typedef enum {
    KW_NONE = 0,
    KW_PRINT,
    KW_LET,
    KW_INPUT,
    KW_IF,
    KW_THEN,
    KW_ELSE,
    KW_GOTO,
    KW_GOSUB,
    KW_RETURN,
    KW_END,
    KW_REM,
    KW_LIST,
    KW_RUN,
    KW_NEW,
    /* Loop Keywords */
    KW_FOR,
    KW_NEXT,
    KW_TO,
    KW_STEP,
    KW_BY,
    KW_WHILE,
    KW_WEND,
    KW_DO,
    KW_LOOP,
    KW_UNTIL,
    KW_DIM,
    KW_ERASE,
    KW_OPTION,
    KW_DATA,
    KW_READ,
    KW_RESTORE,
    KW_DEF,
    KW_DEFINT,
    KW_DEFSNG,
    KW_DEFDBL,
    KW_DEFSTR,
    KW_USR,
    KW_ON,
    KW_ERROR,
    KW_RESUME,
    KW_LOAD,
    KW_SAVE,
    KW_MERGE,
    KW_COMMON,
    KW_CHAIN,
    KW_SELFTEST,
    /* Phase 3 & 4 Keywords */
    KW_OPEN,
    KW_CLOSE,
    KW_AS,
    KW_FILES,
    KW_KILL,
    KW_SCRATCH,
    KW_CHDIR,
    KW_MKDIR,
    KW_RMDIR,
    KW_DIR,
    KW_SETATTR,
    KW_LOCK,
    KW_UNLOCK,
    KW_ENVIRON,
    KW_NAME,
    KW_GET,
    KW_PUT,
    KW_BGET,
    KW_BPUT,
    KW_SEEK,
    KW_FREEFILE,
    KW_FIELD,
    KW_SELECT,
    KW_CASE,
    KW_DECLARE,
    KW_SUB,
    KW_FUNCTION,
    KW_CALL,
    KW_AUTO,
    KW_BCOLOR,
    KW_FCOLOR,
    KW_CLEAR,
    KW_CURSOR,
    KW_LOCATE,
    KW_SHARED,
    KW_SCREEN,
    KW_COLOR,
    KW_LINE,
    KW_CIRCLE,
    KW_PSET,
    KW_PRESET,
    KW_CLS,
    KW_LSET,
    KW_VARPTR,
    KW_VARSEG,
    KW_SADD,
    KW_DEFSEG,
    KW_CINT,
    KW_CSNG,
    KW_CDBL,
    KW_RSET,
    KW_SWAP,
    KW_LOCAL,
    KW_STATIC,
    KW_REDIM,
    KW_PRESERVE,
    KW_PAINT,
    KW_BEEP,
    KW_SOUND,
    KW_PLAY,
    KW_BLOAD,
    KW_BSAVE,
    KW_BRUN,
    KW_USING,
    KW_EDIT,
    KW_SECURITY,
    KW_MODULE,
    KW_LEVEL,
    KW_RESTRICT,
    KW_RESET,
    KW_INFO,
    KW_UNLOAD,
    KW_TASK,
    KW_WAIT,
    KW_MAT,
    KW_ARRAY,
    KW_MAP,
    KW_COM,
    KW_PEN,
    KW_STRIG,
    KW_FILTER,
    KW_REDUCE,
    KW_RENUM,
    KW_DELETE,
    KW_HELP,
    KW_CATALOG,
    KW_DEVICES,
    KW_IOCTL,
    KW_MOUNT,
    KW_UMOUNT,
    KW_UNSAVE,
    KW_CHVT,
    KW_NET,
    KW_OUT,
    KW_POKE,
    KW_BIOS,
    KW_GEMINI,
    KW_UNLESS,
    KW_DEMAND,
    KW_TRY,
    KW_CATCH,
    KW_THROW,
    KW_ALIAS,
    KW_METADATA,
    KW_DIALECT,
    KW_DEFINE,
    KW_ENUM,
    KW_WITH,
    KW_NOISE,
    KW_SNDPLAY,
    KW_SNDLOOP,
    KW_SNDSTOP,
    KW_SNDPAUSE,
    KW_SNDVOL,
    KW_MOUSEINPUT,
    KW_MOUSEHIDE,
    KW_MOUSESHOW,
    KW_MOUSE,
    KW_HMOUSE,
    KW_VMOUSE,
    KW_TRIG,
    KW_DISPLAY,
    KW_LPRINT,
    KW_RANDOMIZE,
    KW_PWD,
    KW_HOSTNAME,
    KW_USERNAME,
    KW_PATH,
    KW_TITLE,
    KW_SCREENMOVE,
    KW_FULLSCREEN,
    KW_RESIZE,
    KW_ICON,
    KW_NWRITE,
    KW_FREEIMAGE,
    KW_PUTIMAGE,
    KW_STATESAVE,
    KW_STATELOAD,
    KW_TYPE,
    KW_CLASS,
    KW_BORDER,
    KW_INK,
    KW_PAPER,
    KW_PAUSE,
    KW_SYS,
    KW_GR,
    KW_HGR,
    KW_HGR2,
    KW_HCOLOR,
    KW_PLOT,
    KW_HLIN,
    KW_VLIN,
    KW_HPLOT,
    KW_ONERR,
    KW_GRAPHICS,
    KW_DRAWTO,
    KW_SYSTEM,
    KW_SHELL,
    KW_BYE,
    KW_STOP,
    KW_EXIT,
    KW_PROCEDURE,
    KW_ENDFUNC,
    KW_ENDPROC,
    KW_TXN,
    KW_ATOMIC,
    KW_COMMIT,
    KW_ROLLBACK,
    KW_TIMER,
    KW_ALARM,
    KW_ALARM_STR,
    KW_KEY,
    KW_OFF,
    KW_SET,
    KW_SNOOZE,
    KW_UNSET,
    KW_INITGRAPH,
    KW_CLOSEGRAPH,
    KW_PUTPIXEL,
    KW_GETPIXEL,
    KW_BAR,
    KW_ELLIPSE,
    KW_RECTANGLE,
    KW_OUTTEXTXY,
    KW_PALETTE,
    KW_ASSERT,
    KW_TRON,
    KW_TROFF,
    KW_BREAK,
    KW_VARS,
    KW_REMOVE,
    KW_REMOVE_STR,
    KW_AND,
    KW_OR,
    KW_NOT,
    KW_XOR
} BppKeywordId;

/* Ephemeral Token Struct */
typedef struct {
    BppTokenType type;
    union {
        double       number;
        const char  *string;  /* Lexer source slice pointer (not null-terminated) */
        BppKeywordId keyword;
    } as;
    const char *start;        /* Pointer to first character in source */
    size_t      length;       /* Length of token in source */
} BppToken;

/* Opaque Lexer Context */
typedef struct LexerContext LexerContext;

/**
 * @brief Initialize the lexer context for a specific source line.
 */
LexerContext *lex_init(MemoryContext *mem, const char *source);

/**
 * @brief Free the lexer context.
 */
void lex_shutdown(LexerContext *ctx);

/**
 * @brief Retrieve the next token and advance the scanner position.
 */
BppToken lex_next(LexerContext *ctx);

/**
 * @brief Peek at the next token without advancing the scanner position.
 */
BppToken lex_peek(LexerContext *ctx);

/**
 * @brief Get the current pointer position in the source line.
 */
const char *lex_get_pos(LexerContext *ctx);

/**
 * @brief Set the scanner position (used for rollback/rewind).
 */
void lex_set_pos(LexerContext *ctx, const char *pos);

/**
 * @brief Convert a keyword ID back to its standard string name.
 */
typedef struct BppDialect BppDialect;
void lex_set_dialect(LexerContext *ctx, BppDialect *dialect);
const char *lex_keyword_name(BppKeywordId kw);
BppKeywordId lex_find_keyword_by_name(const char *name);
void keyword_clear_custom(void);
BppKeywordId keyword_register_custom(const char *name);

#endif /* BPP_LEXER_H */

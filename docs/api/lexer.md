# C17 API Reference: Lexical Analyzer (`lexer/lexer.h`)

## 1. Subsystem Overview & Responsibilities

The Lexical Analyzer Subsystem (`lexer/lexer.h`, implemented in `engine/src/lexer/lexer.c`) provides ephemeral token scanning, keyword identification, bounded token comparisons, and lookahead parsing for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Ephemeral Tokenization**: The source code is canonical and is never permanently tokenized. Tokens are parsed ephemerally into tagged `BppToken` records and discarded immediately after statement/expression execution.
- **Tagged Union Token Layout**:
  - `TOK_IDENT`: Pointer into source (`.start`) and `.length` ONLY — union fields `.as.string` and `.as.number` are NOT valid.
  - `TOK_STRING`: Refcounted string pointer (`.as.string`).
  - `TOK_NUMBER`: Double-precision float (`.as.number`).
  - `TOK_KEYWORD`: Keyword enum (`.as.keyword`).
- **Bounded Token Comparisons**: Lexer identifier comparisons must NEVER use unbounded `strcmp()` on `.start`; they must check `tok.length == expected_len` first, followed by `strncasecmp(tok.start, "KEYWORD", expected_len) == 0`.
- **Dual Keyword/Identifier Handling**: Statement modifiers can be emitted as `TOK_KEYWORD` or `TOK_IDENT`; parsers must check both forms.

## 2. Header Inclusion & Prerequisites

```c
#include "lexer/lexer.h"
#include "memory/memory.h"
```

## 3. Data Structures & Types

```c
/* Ephemeral Token Types */
typedef enum {
    TOK_EOF = 0,
    TOK_EOL,            /* Newline '\n' or statement separator ':' */
    TOK_NUMBER,         /* Numeric constant */
    TOK_STRING,         /* String literal inside double quotes */
    TOK_RPN_LITERAL,    /* RPN literal inside curly braces */
    TOK_IDENT,          /* Identifier (variable name or label) */
    TOK_KEYWORD,        /* Built-in language statement/keyword */
    TOK_PLUS,           /* + */
    TOK_MINUS,          /* - */
    TOK_MUL,            /* * */
    TOK_DIV,            /* / */
    TOK_POW,            /* ^ or ** */
    TOK_EQ,             /* = */
    TOK_NE,             /* <> */
    TOK_LT,             /* < */
    TOK_GT,             /* > */
    TOK_LE,             /* <= */
    TOK_GE,             /* >= */
    TOK_AND,            /* AND */
    TOK_OR,             /* OR */
    TOK_NOT,            /* NOT */
    TOK_LPAREN,         /* ( */
    TOK_RPAREN,         /* ) */
    TOK_COMMA,          /* , */
    TOK_SEMICOLON,      /* ; */
    TOK_HASH,           /* # */
    TOK_UNKNOWN
} BppTokenType;

/* Ephemeral Token Struct */
typedef struct {
    BppTokenType type;
    const char  *start;         /* Pointer to start of token in source line */
    size_t       length;        /* Length of token text in bytes */
    union {
        double        number;   /* Valid when type == TOK_NUMBER */
        BppString    *string;   /* Valid when type == TOK_STRING */
        BppKeywordId  keyword;  /* Valid when type == TOK_KEYWORD */
    } as;
} BppToken;

/* Opaque Lexer Context */
typedef struct LexerContext LexerContext;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes a LexerContext for scanning a line of source code.
 * @param mem Pointer to active MemoryContext.
 * @param source Null-terminated BASIC source line.
 * @return Allocated LexerContext pointer.
 */
LexerContext *lex_init(MemoryContext *mem, const char *source);

/**
 * @brief Shuts down the LexerContext and releases ephemeral structures.
 */
void lex_shutdown(LexerContext *lex);

/**
 * @brief Consumes and returns the next token in the stream.
 */
BppToken lex_next(LexerContext *lex);

/**
 * @brief Peeks at the next token without advancing the stream position.
 */
BppToken lex_peek(LexerContext *lex);

/**
 * @brief Peeks at the Nth token ahead in the stream.
 */
BppToken lex_peek_n(LexerContext *lex, int n);

/**
 * @brief Returns current character position offset in source line.
 */
size_t lex_get_pos(LexerContext *lex);
```

## 5. Architectural Invariants & Token Union Safety

- **TOK_IDENT Access Rule**: Accessing `.as.string` on a `TOK_IDENT` token is UNDEFINED BEHAVIOR. Use `.start` and `.length`.
- **Lexer Progress Guard**: A statement handler must NEVER return success (`err.code = 0`) without advancing the lexer via `lex_next()`.

## 6. Code Example: Bounded Identifier Matching

```c
#include "lexer/lexer.h"
#include <string.h>

bool match_identifier(BppToken tok, const char *target) {
    size_t len = strlen(target);
    if (tok.type == TOK_IDENT && tok.length == len) {
        return (strncasecmp(tok.start, target, len) == 0);
    }
    return false;
}
```

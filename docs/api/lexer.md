# Lexer Subsystem API Reference

Header File: [`include/lexer.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/lexer.h)

## Overview
Provides ephemeral tokenizer functionality for scanning BASIC source statements into tokens.

## Exposed API Entities
### Structs & Types
- `LexerContext LexerContext`
- `BppDialect BppDialect`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `lex_shutdown` | `void` | `LexerContext *ctx` |
| `lex_next` | `BppToken` | `LexerContext *ctx` |
| `lex_peek` | `BppToken` | `LexerContext *ctx` |
| `lex_set_pos` | `void` | `LexerContext *ctx, const char *pos` |
| `lex_set_dialect` | `void` | `LexerContext *ctx, BppDialect *dialect` |
| `lex_find_keyword_by_name` | `BppKeywordId` | `const char *name` |
| `keyword_clear_custom` | `void` | `void` |
| `keyword_register_custom` | `BppKeywordId` | `const char *name` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "lexer.h"

void tokenize(const char *source) {
    LexerContext *lex = lex_init(NULL, source);
    BppToken tok = lex_next(lex);
    (void)tok;
    lex_shutdown(lex);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.

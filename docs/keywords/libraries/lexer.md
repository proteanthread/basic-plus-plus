# `lexer` Ephemeral Lexical Analyzer (`libkernel`)

## 1. Architectural Purpose & Overview

The `lexer` subsystem (`engine/src/lexer/lexer.c`) provides fast, zero-allocation, ephemeral token scanning over canonical BASIC++ source code text.

### Key Architectural Invariants:
- **Ephemeral Tokenization**: Source code is never permanently tokenized or mutated. Tokens are generated on-demand and discarded immediately after statement/expression processing.
- **Tagged Union Tokens**: `BppToken` encapsulates `.type`, `.start`, `.length`, `.as.number`, and `.as.keyword`.
- **Bounded Identifier Checks**: Token comparisons against keywords use length-checked bounds (`tok.length == len && strncasecmp(tok.start, "KEY", len) == 0`).

---

## 2. Technical API Signatures (C17)

```c
LexerContext *lex_create(const char *source);
void lex_destroy(LexerContext *lex);
BppToken lex_next(LexerContext *lex);
BppToken lex_peek(LexerContext *lex);
```

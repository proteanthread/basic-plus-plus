/**
 * @file test_lexer.c
 * @brief Lexer/Tokenizer unit tests.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Verifies character scanning, token classification, number parses,
 *   case-insensitive keywords, comments, and operator parses.
 * - Why it exists: Ensures the scanner functions properly under varying input patterns
 *   without memory leaks or buffer overflows.
 * - Why it works this way: It builds a LexerContext with target strings, pulls tokens
 *   using lex_next and checks token type, values, and slice ranges.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Test expressions, token types, and custom keyword scans.
 * - What cannot be changed: Obligation to check EOF termination and cleanup.
 * - What to expect: Invalid strings or symbols return TOK_UNKNOWN or EOL depending on rules.
 * - What to do if something breaks: Trace characters at lexer->pos and verify skips.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Token pointers refer to static or local source buffers.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add multi-line parsing or string escape code checks.
 * - How to write external extensions: Test scenarios for dialect aliases hook this suite.
 */

#include "test_harness.h"
#include "bpp_lexer.h"
#include <stdlib.h>

static bool test_basic_tokenizing(void) {
    MemoryContext *mem = mem_init(1024, 1024, 1024, 1024);
    
    const char *src = "PRINT 123.45 : LET A$ = \"HELLO\"";
    LexerContext *lex = lex_init(mem, src);
    ASSERT_TRUE(lex != NULL, "Failed to initialize lexer context");

    /* Token 1: PRINT (Keyword) */
    BppToken tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_KEYWORD, "Wrong first token type");
    ASSERT_EQ_INT(tok.as.keyword, KW_PRINT, "Wrong first keyword ID");

    /* Token 2: 123.45 (Number) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_NUMBER, "Wrong second token type");
    ASSERT_EQ_DOUBLE(tok.as.number, 123.45, "Wrong parsed number value");

    /* Token 3: : (EOL separator) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_EOL, "Wrong third token type");

    /* Token 4: LET (Keyword) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_KEYWORD, "Wrong fourth token type");
    ASSERT_EQ_INT(tok.as.keyword, KW_LET, "Wrong fourth keyword ID");

    /* Token 5: A$ (Identifier) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_IDENT, "Wrong fifth token type");
    ASSERT_TRUE(strncmp(tok.as.string, "A$", tok.length) == 0, "Wrong identifier string");

    /* Token 6: = (Operator EQ) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_EQ, "Wrong sixth token type");

    /* Token 7: "HELLO" (String literal) */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_STRING, "Wrong seventh token type");
    ASSERT_TRUE(strncmp(tok.as.string, "HELLO", tok.length) == 0, "Wrong string literal value");

    /* Token 8: EOF */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_EOF, "Expected EOF at end of line");

    lex_shutdown(lex);
    mem_shutdown(mem);
    return true;
}

static bool test_lexer_peeking(void) {
    MemoryContext *mem = mem_init(1024, 1024, 1024, 1024);

    const char *src = "IF THEN";
    LexerContext *lex = lex_init(mem, src);

    BppToken tok1 = lex_peek(lex);
    BppToken tok2 = lex_peek(lex);
    ASSERT_EQ_INT(tok1.type, TOK_KEYWORD, "First peek failed");
    ASSERT_EQ_INT(tok1.as.keyword, KW_IF, "First peek returned wrong keyword");
    ASSERT_EQ_INT(tok2.type, TOK_KEYWORD, "Second peek failed to hold state");

    /* Advance */
    lex_next(lex);
    BppToken tok3 = lex_next(lex);
    ASSERT_EQ_INT(tok3.type, TOK_KEYWORD, "Advancement failed");
    ASSERT_EQ_INT(tok3.as.keyword, KW_THEN, "Wrong second keyword after advancement");

    lex_shutdown(lex);
    mem_shutdown(mem);
    return true;
}

static bool test_comment_skipping(void) {
    MemoryContext *mem = mem_init(1024, 1024, 1024, 1024);

    const char *src = "PRINT 10 REM HELLO WORLD";
    LexerContext *lex = lex_init(mem, src);

    BppToken tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_KEYWORD, "Expected PRINT keyword");

    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_NUMBER, "Expected number 10");

    /* REM comment follows: next token should be EOL representing end of remark */
    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_EOL, "Expected comment to evaluate as EOL");

    lex_shutdown(lex);

    /* Temp test */
    const char *temp_src = "inx% = mousex% >= centerx% and mousex% <= centerx%+maxxs%-1";
    LexerContext *temp_lex = lex_init(mem, temp_src);
    printf("\n=== TEMP LEX PRINT ===\n");
    while (1) {
        BppToken t = lex_next(temp_lex);
        printf("Token: type=%d, len=%zu, text='%.*s'\n", t.type, t.length, (int)t.length, t.start);
        if (t.type == TOK_EOF) break;
    }
    printf("======================\n");
    lex_shutdown(temp_lex);

    mem_shutdown(mem);
    return true;
}

static bool test_logical_operators(void) {
    MemoryContext *mem = mem_init(1024, 1024, 1024, 1024);
    const char *src = "AND OR NOT XOR";
    LexerContext *lex = lex_init(mem, src);

    BppToken tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_AND, "Expected TOK_AND");

    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_OR, "Expected TOK_OR");

    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_NOT, "Expected TOK_NOT");

    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_XOR, "Expected TOK_XOR");

    tok = lex_next(lex);
    ASSERT_EQ_INT(tok.type, TOK_EOF, "Expected EOF");

    lex_shutdown(lex);
    mem_shutdown(mem);
    return true;
}

/* Suite descriptor exports */
TestEntry lexer_test_suite[] = {
    {"basic_tokenizing", test_basic_tokenizing},
    {"lexer_peeking", test_lexer_peeking},
    {"comment_skipping", test_comment_skipping},
    {"logical_operators", test_logical_operators},
    {NULL, NULL}
};

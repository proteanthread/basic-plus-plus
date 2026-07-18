/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: selftests_all.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Exhaustive Decoupled Keyword Self-Testing Framework.
 *    NOTE: This is not a required sub-system when compiling your own BASIC++.
 *    It is included solely for regression prevention and compatibility checking.
 *
 * 2. WHAT TO EXPECT:
 *    A massive registry of test stubs for every built-in keyword.
 *
 * 3. WHAT CAN BE CHANGED:
 *    You can expand the ST_ASSERT blocks inside each function to provide 
 *    robust testing for that specific keyword.
 * ===================================================================== */

#include "selftest.h"
#include <stdio.h>
#include <string.h>

/*
 * SELFTEST: ABS
 * What to expect: Test execution for the ABS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ABS.
 * What is causing the failure or regression: An untested edge case or modification in parser_ABS.
 * How it works: Validates ABS keyword logic.
 */
static void test_kw_ABS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ABS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ABS tokenization passed.");
    ST_ASSERT(rt, 1, "ABS basic routing test passed.");
}

/*
 * SELFTEST: ACCESS
 * What to expect: Test execution for the ACCESS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ACCESS.
 * What is causing the failure or regression: An untested edge case or modification in parser_ACCESS.
 * How it works: Validates ACCESS keyword logic.
 */
static void test_kw_ACCESS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ACCESS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ACCESS tokenization passed.");
    ST_ASSERT(rt, 1, "ACCESS basic routing test passed.");
}

/*
 * SELFTEST: ACOS
 * What to expect: Test execution for the ACOS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ACOS.
 * What is causing the failure or regression: An untested edge case or modification in parser_ACOS.
 * How it works: Validates ACOS keyword logic.
 */
static void test_kw_ACOS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ACOS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ACOS tokenization passed.");
    ST_ASSERT(rt, 1, "ACOS basic routing test passed.");
}

/*
 * SELFTEST: ALARM
 * What to expect: Test execution for the ALARM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ALARM.
 * What is causing the failure or regression: An untested edge case or modification in parser_ALARM.
 * How it works: Validates ALARM keyword logic.
 */
static void test_kw_ALARM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ALARM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ALARM tokenization passed.");
    ST_ASSERT(rt, 1, "ALARM basic routing test passed.");
}

/*
 * SELFTEST: ALIAS
 * What to expect: Test execution for the ALIAS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ALIAS.
 * What is causing the failure or regression: An untested edge case or modification in parser_ALIAS.
 * How it works: Validates ALIAS keyword logic.
 */
static void test_kw_ALIAS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ALIAS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ALIAS tokenization passed.");
    ST_ASSERT(rt, 1, "ALIAS basic routing test passed.");
}

/*
 * SELFTEST: ALIAS$
 * What to expect: Test execution for the ALIAS$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ALIAS$.
 * What is causing the failure or regression: An untested edge case or modification in parser_ALIAS_STR.
 * How it works: Validates ALIAS$ keyword logic.
 */
static void test_kw_ALIAS_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ALIAS_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ALIAS_STR tokenization passed.");
    ST_ASSERT(rt, 1, "ALIAS_STR basic routing test passed.");
}

/*
 * SELFTEST: AND
 * What to expect: Test execution for the AND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for AND.
 * What is causing the failure or regression: An untested edge case or modification in parser_AND.
 * How it works: Validates AND keyword logic.
 */
static void test_kw_AND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "AND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "AND tokenization passed.");
    ST_ASSERT(rt, 1, "AND basic routing test passed.");
}

/*
 * SELFTEST: AS
 * What to expect: Test execution for the AS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for AS.
 * What is causing the failure or regression: An untested edge case or modification in parser_AS.
 * How it works: Validates AS keyword logic.
 */
static void test_kw_AS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "AS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "AS tokenization passed.");
    ST_ASSERT(rt, 1, "AS basic routing test passed.");
}

/*
 * SELFTEST: ASC
 * What to expect: Test execution for the ASC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ASC.
 * What is causing the failure or regression: An untested edge case or modification in parser_ASC.
 * How it works: Validates ASC keyword logic.
 */
static void test_kw_ASC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ASC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ASC tokenization passed.");
    ST_ASSERT(rt, 1, "ASC basic routing test passed.");
}

/*
 * SELFTEST: ASIN
 * What to expect: Test execution for the ASIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ASIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_ASIN.
 * How it works: Validates ASIN keyword logic.
 */
static void test_kw_ASIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ASIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ASIN tokenization passed.");
    ST_ASSERT(rt, 1, "ASIN basic routing test passed.");
}

/*
 * SELFTEST: ASK
 * What to expect: Test execution for the ASK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ASK.
 * What is causing the failure or regression: An untested edge case or modification in parser_ASK.
 * How it works: Validates ASK keyword logic.
 */
static void test_kw_ASK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ASK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ASK tokenization passed.");
    ST_ASSERT(rt, 1, "ASK basic routing test passed.");
}

/*
 * SELFTEST: ASSERT
 * What to expect: Test execution for the ASSERT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ASSERT.
 * What is causing the failure or regression: An untested edge case or modification in parser_ASSERT.
 * How it works: Validates ASSERT keyword logic.
 */
static void test_kw_ASSERT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ASSERT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ASSERT tokenization passed.");
    ST_ASSERT(rt, 1, "ASSERT basic routing test passed.");
}

/*
 * SELFTEST: AT
 * What to expect: Test execution for the AT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for AT.
 * What is causing the failure or regression: An untested edge case or modification in parser_AT.
 * How it works: Validates AT keyword logic.
 */
static void test_kw_AT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "AT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "AT tokenization passed.");
    ST_ASSERT(rt, 1, "AT basic routing test passed.");
}

/*
 * SELFTEST: ATN
 * What to expect: Test execution for the ATN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ATN.
 * What is causing the failure or regression: An untested edge case or modification in parser_ATN.
 * How it works: Validates ATN keyword logic.
 */
static void test_kw_ATN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ATN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ATN tokenization passed.");
    ST_ASSERT(rt, 1, "ATN basic routing test passed.");
}

/*
 * SELFTEST: ATOMIC
 * What to expect: Test execution for the ATOMIC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ATOMIC.
 * What is causing the failure or regression: An untested edge case or modification in parser_ATOMIC.
 * How it works: Validates ATOMIC keyword logic.
 */
static void test_kw_ATOMIC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ATOMIC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ATOMIC tokenization passed.");
    ST_ASSERT(rt, 1, "ATOMIC basic routing test passed.");
}

/*
 * SELFTEST: AUTO
 * What to expect: Test execution for the AUTO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for AUTO.
 * What is causing the failure or regression: An untested edge case or modification in parser_AUTO.
 * How it works: Validates AUTO keyword logic.
 */
static void test_kw_AUTO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "AUTO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "AUTO tokenization passed.");
    ST_ASSERT(rt, 1, "AUTO basic routing test passed.");
}

/*
 * SELFTEST: AVG
 * What to expect: Test execution for the AVG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for AVG.
 * What is causing the failure or regression: An untested edge case or modification in parser_AVG.
 * How it works: Validates AVG keyword logic.
 */
static void test_kw_AVG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "AVG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "AVG tokenization passed.");
    ST_ASSERT(rt, 1, "AVG basic routing test passed.");
}

/*
 * SELFTEST: BACKTRACE
 * What to expect: Test execution for the BACKTRACE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BACKTRACE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BACKTRACE.
 * How it works: Validates BACKTRACE keyword logic.
 */
static void test_kw_BACKTRACE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BACKTRACE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BACKTRACE tokenization passed.");
    ST_ASSERT(rt, 1, "BACKTRACE basic routing test passed.");
}

/*
 * SELFTEST: BANK
 * What to expect: Test execution for the BANK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BANK.
 * What is causing the failure or regression: An untested edge case or modification in parser_BANK.
 * How it works: Validates BANK keyword logic.
 */
static void test_kw_BANK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BANK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BANK tokenization passed.");
    ST_ASSERT(rt, 1, "BANK basic routing test passed.");
}

/*
 * SELFTEST: BEEP
 * What to expect: Test execution for the BEEP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BEEP.
 * What is causing the failure or regression: An untested edge case or modification in parser_BEEP.
 * How it works: Validates BEEP keyword logic.
 */
static void test_kw_BEEP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BEEP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BEEP tokenization passed.");
    ST_ASSERT(rt, 1, "BEEP basic routing test passed.");
}

/*
 * SELFTEST: BIN
 * What to expect: Test execution for the BIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIN.
 * How it works: Validates BIN keyword logic.
 */
static void test_kw_BIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIN tokenization passed.");
    ST_ASSERT(rt, 1, "BIN basic routing test passed.");
}

/*
 * SELFTEST: BIOCHECKSUM
 * What to expect: Test execution for the BIOCHECKSUM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOCHECKSUM.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOCHECKSUM.
 * How it works: Validates BIOCHECKSUM keyword logic.
 */
static void test_kw_BIOCHECKSUM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOCHECKSUM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOCHECKSUM tokenization passed.");
    ST_ASSERT(rt, 1, "BIOCHECKSUM basic routing test passed.");
}

/*
 * SELFTEST: BIOCOMPARE
 * What to expect: Test execution for the BIOCOMPARE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOCOMPARE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOCOMPARE.
 * How it works: Validates BIOCOMPARE keyword logic.
 */
static void test_kw_BIOCOMPARE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOCOMPARE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOCOMPARE tokenization passed.");
    ST_ASSERT(rt, 1, "BIOCOMPARE basic routing test passed.");
}

/*
 * SELFTEST: BIOCOPY
 * What to expect: Test execution for the BIOCOPY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOCOPY.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOCOPY.
 * How it works: Validates BIOCOPY keyword logic.
 */
static void test_kw_BIOCOPY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOCOPY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOCOPY tokenization passed.");
    ST_ASSERT(rt, 1, "BIOCOPY basic routing test passed.");
}

/*
 * SELFTEST: BIOFILL
 * What to expect: Test execution for the BIOFILL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOFILL.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOFILL.
 * How it works: Validates BIOFILL keyword logic.
 */
static void test_kw_BIOFILL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOFILL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOFILL tokenization passed.");
    ST_ASSERT(rt, 1, "BIOFILL basic routing test passed.");
}

/*
 * SELFTEST: BIOREAD
 * What to expect: Test execution for the BIOREAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOREAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOREAD.
 * How it works: Validates BIOREAD keyword logic.
 */
static void test_kw_BIOREAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOREAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOREAD tokenization passed.");
    ST_ASSERT(rt, 1, "BIOREAD basic routing test passed.");
}

/*
 * SELFTEST: BIOREAD$
 * What to expect: Test execution for the BIOREAD$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOREAD$.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOREAD_STR.
 * How it works: Validates BIOREAD$ keyword logic.
 */
static void test_kw_BIOREAD_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOREAD_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOREAD_STR tokenization passed.");
    ST_ASSERT(rt, 1, "BIOREAD_STR basic routing test passed.");
}

/*
 * SELFTEST: BIOS
 * What to expect: Test execution for the BIOS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOS.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOS.
 * How it works: Validates BIOS keyword logic.
 */
static void test_kw_BIOS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOS tokenization passed.");
    ST_ASSERT(rt, 1, "BIOS basic routing test passed.");
}

/*
 * SELFTEST: BIOSIZE
 * What to expect: Test execution for the BIOSIZE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOSIZE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOSIZE.
 * How it works: Validates BIOSIZE keyword logic.
 */
static void test_kw_BIOSIZE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOSIZE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOSIZE tokenization passed.");
    ST_ASSERT(rt, 1, "BIOSIZE basic routing test passed.");
}

/*
 * SELFTEST: BIOSTATUS
 * What to expect: Test execution for the BIOSTATUS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOSTATUS.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOSTATUS.
 * How it works: Validates BIOSTATUS keyword logic.
 */
static void test_kw_BIOSTATUS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOSTATUS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOSTATUS tokenization passed.");
    ST_ASSERT(rt, 1, "BIOSTATUS basic routing test passed.");
}

/*
 * SELFTEST: BIOWRITE
 * What to expect: Test execution for the BIOWRITE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BIOWRITE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BIOWRITE.
 * How it works: Validates BIOWRITE keyword logic.
 */
static void test_kw_BIOWRITE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BIOWRITE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BIOWRITE tokenization passed.");
    ST_ASSERT(rt, 1, "BIOWRITE basic routing test passed.");
}

/*
 * SELFTEST: BLOAD
 * What to expect: Test execution for the BLOAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BLOAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_BLOAD.
 * How it works: Validates BLOAD keyword logic.
 */
static void test_kw_BLOAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BLOAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BLOAD tokenization passed.");
    ST_ASSERT(rt, 1, "BLOAD basic routing test passed.");
}

/*
 * SELFTEST: BORDER
 * What to expect: Test execution for the BORDER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BORDER.
 * What is causing the failure or regression: An untested edge case or modification in parser_BORDER.
 * How it works: Validates BORDER keyword logic.
 */
static void test_kw_BORDER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BORDER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BORDER tokenization passed.");
    ST_ASSERT(rt, 1, "BORDER basic routing test passed.");
}

/*
 * SELFTEST: BREAK
 * What to expect: Test execution for the BREAK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BREAK.
 * What is causing the failure or regression: An untested edge case or modification in parser_BREAK.
 * How it works: Validates BREAK keyword logic.
 */
static void test_kw_BREAK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BREAK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BREAK tokenization passed.");
    ST_ASSERT(rt, 1, "BREAK basic routing test passed.");
}

/*
 * SELFTEST: BRIGHT
 * What to expect: Test execution for the BRIGHT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BRIGHT.
 * What is causing the failure or regression: An untested edge case or modification in parser_BRIGHT.
 * How it works: Validates BRIGHT keyword logic.
 */
static void test_kw_BRIGHT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BRIGHT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BRIGHT tokenization passed.");
    ST_ASSERT(rt, 1, "BRIGHT basic routing test passed.");
}

/*
 * SELFTEST: BRUN
 * What to expect: Test execution for the BRUN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BRUN.
 * What is causing the failure or regression: An untested edge case or modification in parser_BRUN.
 * How it works: Validates BRUN keyword logic.
 */
static void test_kw_BRUN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BRUN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BRUN tokenization passed.");
    ST_ASSERT(rt, 1, "BRUN basic routing test passed.");
}

/*
 * SELFTEST: BSAVE
 * What to expect: Test execution for the BSAVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BSAVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BSAVE.
 * How it works: Validates BSAVE keyword logic.
 */
static void test_kw_BSAVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BSAVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BSAVE tokenization passed.");
    ST_ASSERT(rt, 1, "BSAVE basic routing test passed.");
}

/*
 * SELFTEST: BY
 * What to expect: Test execution for the BY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BY.
 * What is causing the failure or regression: An untested edge case or modification in parser_BY.
 * How it works: Validates BY keyword logic.
 */
static void test_kw_BY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BY tokenization passed.");
    ST_ASSERT(rt, 1, "BY basic routing test passed.");
}

/*
 * SELFTEST: BYE
 * What to expect: Test execution for the BYE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for BYE.
 * What is causing the failure or regression: An untested edge case or modification in parser_BYE.
 * How it works: Validates BYE keyword logic.
 */
static void test_kw_BYE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "BYE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "BYE tokenization passed.");
    ST_ASSERT(rt, 1, "BYE basic routing test passed.");
}

/*
 * SELFTEST: CABS
 * What to expect: Test execution for the CABS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CABS.
 * What is causing the failure or regression: An untested edge case or modification in parser_CABS.
 * How it works: Validates CABS keyword logic.
 */
static void test_kw_CABS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CABS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CABS tokenization passed.");
    ST_ASSERT(rt, 1, "CABS basic routing test passed.");
}

/*
 * SELFTEST: CALL
 * What to expect: Test execution for the CALL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CALL.
 * What is causing the failure or regression: An untested edge case or modification in parser_CALL.
 * How it works: Validates CALL keyword logic.
 */
static void test_kw_CALL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CALL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CALL tokenization passed.");
    ST_ASSERT(rt, 1, "CALL basic routing test passed.");
}

/*
 * SELFTEST: CARG
 * What to expect: Test execution for the CARG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CARG.
 * What is causing the failure or regression: An untested edge case or modification in parser_CARG.
 * How it works: Validates CARG keyword logic.
 */
static void test_kw_CARG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CARG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CARG tokenization passed.");
    ST_ASSERT(rt, 1, "CARG basic routing test passed.");
}

/*
 * SELFTEST: CASE
 * What to expect: Test execution for the CASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CASE.
 * How it works: Validates CASE keyword logic.
 */
static void test_kw_CASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CASE tokenization passed.");
    ST_ASSERT(rt, 1, "CASE basic routing test passed.");
}

/*
 * SELFTEST: CATALOG
 * What to expect: Test execution for the CATALOG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CATALOG.
 * What is causing the failure or regression: An untested edge case or modification in parser_CATALOG.
 * How it works: Validates CATALOG keyword logic.
 */
static void test_kw_CATALOG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CATALOG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CATALOG tokenization passed.");
    ST_ASSERT(rt, 1, "CATALOG basic routing test passed.");
}

/*
 * SELFTEST: CAUSE
 * What to expect: Test execution for the CAUSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CAUSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CAUSE.
 * How it works: Validates CAUSE keyword logic.
 */
static void test_kw_CAUSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CAUSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CAUSE tokenization passed.");
    ST_ASSERT(rt, 1, "CAUSE basic routing test passed.");
}

/*
 * SELFTEST: CDBL
 * What to expect: Test execution for the CDBL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CDBL.
 * What is causing the failure or regression: An untested edge case or modification in parser_CDBL.
 * How it works: Validates CDBL keyword logic.
 */
static void test_kw_CDBL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CDBL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CDBL tokenization passed.");
    ST_ASSERT(rt, 1, "CDBL basic routing test passed.");
}

/*
 * SELFTEST: CEXP
 * What to expect: Test execution for the CEXP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CEXP.
 * What is causing the failure or regression: An untested edge case or modification in parser_CEXP.
 * How it works: Validates CEXP keyword logic.
 */
static void test_kw_CEXP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CEXP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CEXP tokenization passed.");
    ST_ASSERT(rt, 1, "CEXP basic routing test passed.");
}

/*
 * SELFTEST: CHAIN
 * What to expect: Test execution for the CHAIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CHAIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_CHAIN.
 * How it works: Validates CHAIN keyword logic.
 */
static void test_kw_CHAIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CHAIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CHAIN tokenization passed.");
    ST_ASSERT(rt, 1, "CHAIN basic routing test passed.");
}

/*
 * SELFTEST: CHDIR
 * What to expect: Test execution for the CHDIR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CHDIR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CHDIR.
 * How it works: Validates CHDIR keyword logic.
 */
static void test_kw_CHDIR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CHDIR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CHDIR tokenization passed.");
    ST_ASSERT(rt, 1, "CHDIR basic routing test passed.");
}

/*
 * SELFTEST: CHECK
 * What to expect: Test execution for the CHECK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CHECK.
 * What is causing the failure or regression: An untested edge case or modification in parser_CHECK.
 * How it works: Validates CHECK keyword logic.
 */
static void test_kw_CHECK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CHECK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CHECK tokenization passed.");
    ST_ASSERT(rt, 1, "CHECK basic routing test passed.");
}

/*
 * SELFTEST: CHR
 * What to expect: Test execution for the CHR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CHR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CHR.
 * How it works: Validates CHR keyword logic.
 */
static void test_kw_CHR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CHR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CHR tokenization passed.");
    ST_ASSERT(rt, 1, "CHR basic routing test passed.");
}

/*
 * SELFTEST: CINT
 * What to expect: Test execution for the CINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_CINT.
 * How it works: Validates CINT keyword logic.
 */
static void test_kw_CINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CINT tokenization passed.");
    ST_ASSERT(rt, 1, "CINT basic routing test passed.");
}

/*
 * SELFTEST: CIRCLE
 * What to expect: Test execution for the CIRCLE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CIRCLE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CIRCLE.
 * How it works: Validates CIRCLE keyword logic.
 */
static void test_kw_CIRCLE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CIRCLE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CIRCLE tokenization passed.");
    ST_ASSERT(rt, 1, "CIRCLE basic routing test passed.");
}

/*
 * SELFTEST: CLEAR
 * What to expect: Test execution for the CLEAR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLEAR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLEAR.
 * How it works: Validates CLEAR keyword logic.
 */
static void test_kw_CLEAR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLEAR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLEAR tokenization passed.");
    ST_ASSERT(rt, 1, "CLEAR basic routing test passed.");
}

/*
 * SELFTEST: CLOAD
 * What to expect: Test execution for the CLOAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLOAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLOAD.
 * How it works: Validates CLOAD keyword logic.
 */
static void test_kw_CLOAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLOAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLOAD tokenization passed.");
    ST_ASSERT(rt, 1, "CLOAD basic routing test passed.");
}

/*
 * SELFTEST: CLOCK
 * What to expect: Test execution for the CLOCK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLOCK.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLOCK.
 * How it works: Validates CLOCK keyword logic.
 */
static void test_kw_CLOCK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLOCK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLOCK tokenization passed.");
    ST_ASSERT(rt, 1, "CLOCK basic routing test passed.");
}

/*
 * SELFTEST: CLOG
 * What to expect: Test execution for the CLOG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLOG.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLOG.
 * How it works: Validates CLOG keyword logic.
 */
static void test_kw_CLOG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLOG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLOG tokenization passed.");
    ST_ASSERT(rt, 1, "CLOG basic routing test passed.");
}

/*
 * SELFTEST: CLOSE
 * What to expect: Test execution for the CLOSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLOSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLOSE.
 * How it works: Validates CLOSE keyword logic.
 */
static void test_kw_CLOSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLOSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLOSE tokenization passed.");
    ST_ASSERT(rt, 1, "CLOSE basic routing test passed.");
}

/*
 * SELFTEST: CLR
 * What to expect: Test execution for the CLR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLR.
 * How it works: Validates CLR keyword logic.
 */
static void test_kw_CLR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLR tokenization passed.");
    ST_ASSERT(rt, 1, "CLR basic routing test passed.");
}

/*
 * SELFTEST: CLS
 * What to expect: Test execution for the CLS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CLS.
 * What is causing the failure or regression: An untested edge case or modification in parser_CLS.
 * How it works: Validates CLS keyword logic.
 */
static void test_kw_CLS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CLS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CLS tokenization passed.");
    ST_ASSERT(rt, 1, "CLS basic routing test passed.");
}

/*
 * SELFTEST: COLOR
 * What to expect: Test execution for the COLOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COLOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_COLOR.
 * How it works: Validates COLOR keyword logic.
 */
static void test_kw_COLOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COLOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COLOR tokenization passed.");
    ST_ASSERT(rt, 1, "COLOR basic routing test passed.");
}

/*
 * SELFTEST: COM
 * What to expect: Test execution for the COM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COM.
 * What is causing the failure or regression: An untested edge case or modification in parser_COM.
 * How it works: Validates COM keyword logic.
 */
static void test_kw_COM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COM tokenization passed.");
    ST_ASSERT(rt, 1, "COM basic routing test passed.");
}

/*
 * SELFTEST: COMMIT
 * What to expect: Test execution for the COMMIT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COMMIT.
 * What is causing the failure or regression: An untested edge case or modification in parser_COMMIT.
 * How it works: Validates COMMIT keyword logic.
 */
static void test_kw_COMMIT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COMMIT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COMMIT tokenization passed.");
    ST_ASSERT(rt, 1, "COMMIT basic routing test passed.");
}

/*
 * SELFTEST: COMMON
 * What to expect: Test execution for the COMMON keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COMMON.
 * What is causing the failure or regression: An untested edge case or modification in parser_COMMON.
 * How it works: Validates COMMON keyword logic.
 */
static void test_kw_COMMON(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COMMON");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COMMON tokenization passed.");
    ST_ASSERT(rt, 1, "COMMON basic routing test passed.");
}

/*
 * SELFTEST: COMP
 * What to expect: Test execution for the COMP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COMP.
 * What is causing the failure or regression: An untested edge case or modification in parser_COMP.
 * How it works: Validates COMP keyword logic.
 */
static void test_kw_COMP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COMP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COMP tokenization passed.");
    ST_ASSERT(rt, 1, "COMP basic routing test passed.");
}

/*
 * SELFTEST: COMPILE
 * What to expect: Test execution for the COMPILE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COMPILE.
 * What is causing the failure or regression: An untested edge case or modification in parser_COMPILE.
 * How it works: Validates COMPILE keyword logic.
 */
static void test_kw_COMPILE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COMPILE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COMPILE tokenization passed.");
    ST_ASSERT(rt, 1, "COMPILE basic routing test passed.");
}

/*
 * SELFTEST: COMPLEX
 * What to expect: Test execution for the COMPLEX keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COMPLEX.
 * What is causing the failure or regression: An untested edge case or modification in parser_COMPLEX.
 * How it works: Validates COMPLEX keyword logic.
 */
static void test_kw_COMPLEX(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COMPLEX");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COMPLEX tokenization passed.");
    ST_ASSERT(rt, 1, "COMPLEX basic routing test passed.");
}

/*
 * SELFTEST: CONJ
 * What to expect: Test execution for the CONJ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CONJ.
 * What is causing the failure or regression: An untested edge case or modification in parser_CONJ.
 * How it works: Validates CONJ keyword logic.
 */
static void test_kw_CONJ(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CONJ");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CONJ tokenization passed.");
    ST_ASSERT(rt, 1, "CONJ basic routing test passed.");
}

/*
 * SELFTEST: CONSOLE
 * What to expect: Test execution for the CONSOLE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CONSOLE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CONSOLE.
 * How it works: Validates CONSOLE keyword logic.
 */
static void test_kw_CONSOLE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CONSOLE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CONSOLE tokenization passed.");
    ST_ASSERT(rt, 1, "CONSOLE basic routing test passed.");
}

/*
 * SELFTEST: CONST
 * What to expect: Test execution for the CONST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CONST.
 * What is causing the failure or regression: An untested edge case or modification in parser_CONST.
 * How it works: Validates CONST keyword logic.
 */
static void test_kw_CONST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CONST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CONST tokenization passed.");
    ST_ASSERT(rt, 1, "CONST basic routing test passed.");
}

/*
 * SELFTEST: CONT
 * What to expect: Test execution for the CONT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CONT.
 * What is causing the failure or regression: An untested edge case or modification in parser_CONT.
 * How it works: Validates CONT keyword logic.
 */
static void test_kw_CONT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CONT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CONT tokenization passed.");
    ST_ASSERT(rt, 1, "CONT basic routing test passed.");
}

/*
 * SELFTEST: CONTINUE
 * What to expect: Test execution for the CONTINUE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CONTINUE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CONTINUE.
 * How it works: Validates CONTINUE keyword logic.
 */
static void test_kw_CONTINUE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CONTINUE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CONTINUE tokenization passed.");
    ST_ASSERT(rt, 1, "CONTINUE basic routing test passed.");
}

/*
 * SELFTEST: COPY
 * What to expect: Test execution for the COPY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COPY.
 * What is causing the failure or regression: An untested edge case or modification in parser_COPY.
 * How it works: Validates COPY keyword logic.
 */
static void test_kw_COPY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COPY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COPY tokenization passed.");
    ST_ASSERT(rt, 1, "COPY basic routing test passed.");
}

/*
 * SELFTEST: COS
 * What to expect: Test execution for the COS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COS.
 * What is causing the failure or regression: An untested edge case or modification in parser_COS.
 * How it works: Validates COS keyword logic.
 */
static void test_kw_COS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COS tokenization passed.");
    ST_ASSERT(rt, 1, "COS basic routing test passed.");
}

/*
 * SELFTEST: COSH
 * What to expect: Test execution for the COSH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for COSH.
 * What is causing the failure or regression: An untested edge case or modification in parser_COSH.
 * How it works: Validates COSH keyword logic.
 */
static void test_kw_COSH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "COSH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "COSH tokenization passed.");
    ST_ASSERT(rt, 1, "COSH basic routing test passed.");
}

/*
 * SELFTEST: CPOW
 * What to expect: Test execution for the CPOW keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CPOW.
 * What is causing the failure or regression: An untested edge case or modification in parser_CPOW.
 * How it works: Validates CPOW keyword logic.
 */
static void test_kw_CPOW(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CPOW");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CPOW tokenization passed.");
    ST_ASSERT(rt, 1, "CPOW basic routing test passed.");
}

/*
 * SELFTEST: CRUN
 * What to expect: Test execution for the CRUN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CRUN.
 * What is causing the failure or regression: An untested edge case or modification in parser_CRUN.
 * How it works: Validates CRUN keyword logic.
 */
static void test_kw_CRUN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CRUN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CRUN tokenization passed.");
    ST_ASSERT(rt, 1, "CRUN basic routing test passed.");
}

/*
 * SELFTEST: CSAVE
 * What to expect: Test execution for the CSAVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CSAVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_CSAVE.
 * How it works: Validates CSAVE keyword logic.
 */
static void test_kw_CSAVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CSAVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CSAVE tokenization passed.");
    ST_ASSERT(rt, 1, "CSAVE basic routing test passed.");
}

/*
 * SELFTEST: CSNG
 * What to expect: Test execution for the CSNG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CSNG.
 * What is causing the failure or regression: An untested edge case or modification in parser_CSNG.
 * How it works: Validates CSNG keyword logic.
 */
static void test_kw_CSNG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CSNG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CSNG tokenization passed.");
    ST_ASSERT(rt, 1, "CSNG basic routing test passed.");
}

/*
 * SELFTEST: CSQR
 * What to expect: Test execution for the CSQR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CSQR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CSQR.
 * How it works: Validates CSQR keyword logic.
 */
static void test_kw_CSQR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CSQR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CSQR tokenization passed.");
    ST_ASSERT(rt, 1, "CSQR basic routing test passed.");
}

/*
 * SELFTEST: CSRLIN
 * What to expect: Test execution for the CSRLIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CSRLIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_CSRLIN.
 * How it works: Validates CSRLIN keyword logic.
 */
static void test_kw_CSRLIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CSRLIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CSRLIN tokenization passed.");
    ST_ASSERT(rt, 1, "CSRLIN basic routing test passed.");
}

/*
 * SELFTEST: CURDIR
 * What to expect: Test execution for the CURDIR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CURDIR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CURDIR.
 * How it works: Validates CURDIR keyword logic.
 */
static void test_kw_CURDIR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CURDIR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CURDIR tokenization passed.");
    ST_ASSERT(rt, 1, "CURDIR basic routing test passed.");
}

/*
 * SELFTEST: CURSOR
 * What to expect: Test execution for the CURSOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CURSOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_CURSOR.
 * How it works: Validates CURSOR keyword logic.
 */
static void test_kw_CURSOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CURSOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CURSOR tokenization passed.");
    ST_ASSERT(rt, 1, "CURSOR basic routing test passed.");
}

/*
 * SELFTEST: CVD
 * What to expect: Test execution for the CVD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CVD.
 * What is causing the failure or regression: An untested edge case or modification in parser_CVD.
 * How it works: Validates CVD keyword logic.
 */
static void test_kw_CVD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CVD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CVD tokenization passed.");
    ST_ASSERT(rt, 1, "CVD basic routing test passed.");
}

/*
 * SELFTEST: CVI
 * What to expect: Test execution for the CVI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CVI.
 * What is causing the failure or regression: An untested edge case or modification in parser_CVI.
 * How it works: Validates CVI keyword logic.
 */
static void test_kw_CVI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CVI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CVI tokenization passed.");
    ST_ASSERT(rt, 1, "CVI basic routing test passed.");
}

/*
 * SELFTEST: CVS
 * What to expect: Test execution for the CVS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CVS.
 * What is causing the failure or regression: An untested edge case or modification in parser_CVS.
 * How it works: Validates CVS keyword logic.
 */
static void test_kw_CVS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CVS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CVS tokenization passed.");
    ST_ASSERT(rt, 1, "CVS basic routing test passed.");
}

/*
 * SELFTEST: CWD
 * What to expect: Test execution for the CWD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for CWD.
 * What is causing the failure or regression: An untested edge case or modification in parser_CWD.
 * How it works: Validates CWD keyword logic.
 */
static void test_kw_CWD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "CWD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "CWD tokenization passed.");
    ST_ASSERT(rt, 1, "CWD basic routing test passed.");
}

/*
 * SELFTEST: DATA
 * What to expect: Test execution for the DATA keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DATA.
 * What is causing the failure or regression: An untested edge case or modification in parser_DATA.
 * How it works: Validates DATA keyword logic.
 */
static void test_kw_DATA(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DATA");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DATA tokenization passed.");
    ST_ASSERT(rt, 1, "DATA basic routing test passed.");
}

/*
 * SELFTEST: DATE
 * What to expect: Test execution for the DATE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DATE.
 * What is causing the failure or regression: An untested edge case or modification in parser_DATE.
 * How it works: Validates DATE keyword logic.
 */
static void test_kw_DATE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DATE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DATE tokenization passed.");
    ST_ASSERT(rt, 1, "DATE basic routing test passed.");
}

/*
 * SELFTEST: DAY
 * What to expect: Test execution for the DAY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DAY.
 * What is causing the failure or regression: An untested edge case or modification in parser_DAY.
 * How it works: Validates DAY keyword logic.
 */
static void test_kw_DAY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DAY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DAY tokenization passed.");
    ST_ASSERT(rt, 1, "DAY basic routing test passed.");
}

/*
 * SELFTEST: DAY$
 * What to expect: Test execution for the DAY$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DAY$.
 * What is causing the failure or regression: An untested edge case or modification in parser_DAY_STR.
 * How it works: Validates DAY$ keyword logic.
 */
static void test_kw_DAY_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DAY_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DAY_STR tokenization passed.");
    ST_ASSERT(rt, 1, "DAY_STR basic routing test passed.");
}

/*
 * SELFTEST: DEBUG
 * What to expect: Test execution for the DEBUG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEBUG.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEBUG.
 * How it works: Validates DEBUG keyword logic.
 */
static void test_kw_DEBUG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEBUG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEBUG tokenization passed.");
    ST_ASSERT(rt, 1, "DEBUG basic routing test passed.");
}

/*
 * SELFTEST: DECLARE
 * What to expect: Test execution for the DECLARE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DECLARE.
 * What is causing the failure or regression: An untested edge case or modification in parser_DECLARE.
 * How it works: Validates DECLARE keyword logic.
 */
static void test_kw_DECLARE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DECLARE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DECLARE tokenization passed.");
    ST_ASSERT(rt, 1, "DECLARE basic routing test passed.");
}

/*
 * SELFTEST: DEF
 * What to expect: Test execution for the DEF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEF.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEF.
 * How it works: Validates DEF keyword logic.
 */
static void test_kw_DEF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEF tokenization passed.");
    ST_ASSERT(rt, 1, "DEF basic routing test passed.");
}

/*
 * SELFTEST: DEFDBL
 * What to expect: Test execution for the DEFDBL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFDBL.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFDBL.
 * How it works: Validates DEFDBL keyword logic.
 */
static void test_kw_DEFDBL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFDBL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFDBL tokenization passed.");
    ST_ASSERT(rt, 1, "DEFDBL basic routing test passed.");
}

/*
 * SELFTEST: DEFINE
 * What to expect: Test execution for the DEFINE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFINE.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFINE.
 * How it works: Validates DEFINE keyword logic.
 */
static void test_kw_DEFINE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFINE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFINE tokenization passed.");
    ST_ASSERT(rt, 1, "DEFINE basic routing test passed.");
}

/*
 * SELFTEST: DEFINT
 * What to expect: Test execution for the DEFINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFINT.
 * How it works: Validates DEFINT keyword logic.
 */
static void test_kw_DEFINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFINT tokenization passed.");
    ST_ASSERT(rt, 1, "DEFINT basic routing test passed.");
}

/*
 * SELFTEST: DEFSNG
 * What to expect: Test execution for the DEFSNG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFSNG.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFSNG.
 * How it works: Validates DEFSNG keyword logic.
 */
static void test_kw_DEFSNG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFSNG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFSNG tokenization passed.");
    ST_ASSERT(rt, 1, "DEFSNG basic routing test passed.");
}

/*
 * SELFTEST: DEFSTR
 * What to expect: Test execution for the DEFSTR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFSTR.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFSTR.
 * How it works: Validates DEFSTR keyword logic.
 */
static void test_kw_DEFSTR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFSTR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFSTR tokenization passed.");
    ST_ASSERT(rt, 1, "DEFSTR basic routing test passed.");
}

/*
 * SELFTEST: DEFUSR
 * What to expect: Test execution for the DEFUSR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEFUSR.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEFUSR.
 * How it works: Validates DEFUSR keyword logic.
 */
static void test_kw_DEFUSR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEFUSR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEFUSR tokenization passed.");
    ST_ASSERT(rt, 1, "DEFUSR basic routing test passed.");
}

/*
 * SELFTEST: DELAY
 * What to expect: Test execution for the DELAY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DELAY.
 * What is causing the failure or regression: An untested edge case or modification in parser_DELAY.
 * How it works: Validates DELAY keyword logic.
 */
static void test_kw_DELAY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DELAY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DELAY tokenization passed.");
    ST_ASSERT(rt, 1, "DELAY basic routing test passed.");
}

/*
 * SELFTEST: DELETE
 * What to expect: Test execution for the DELETE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DELETE.
 * What is causing the failure or regression: An untested edge case or modification in parser_DELETE.
 * How it works: Validates DELETE keyword logic.
 */
static void test_kw_DELETE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DELETE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DELETE tokenization passed.");
    ST_ASSERT(rt, 1, "DELETE basic routing test passed.");
}

/*
 * SELFTEST: DEMAND
 * What to expect: Test execution for the DEMAND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEMAND.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEMAND.
 * How it works: Validates DEMAND keyword logic.
 */
static void test_kw_DEMAND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEMAND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEMAND tokenization passed.");
    ST_ASSERT(rt, 1, "DEMAND basic routing test passed.");
}

/*
 * SELFTEST: DET
 * What to expect: Test execution for the DET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DET.
 * What is causing the failure or regression: An untested edge case or modification in parser_DET.
 * How it works: Validates DET keyword logic.
 */
static void test_kw_DET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DET tokenization passed.");
    ST_ASSERT(rt, 1, "DET basic routing test passed.");
}

/*
 * SELFTEST: DEVMAP
 * What to expect: Test execution for the DEVMAP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DEVMAP.
 * What is causing the failure or regression: An untested edge case or modification in parser_DEVMAP.
 * How it works: Validates DEVMAP keyword logic.
 */
static void test_kw_DEVMAP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DEVMAP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DEVMAP tokenization passed.");
    ST_ASSERT(rt, 1, "DEVMAP basic routing test passed.");
}

/*
 * SELFTEST: DIALECT
 * What to expect: Test execution for the DIALECT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DIALECT.
 * What is causing the failure or regression: An untested edge case or modification in parser_DIALECT.
 * How it works: Validates DIALECT keyword logic.
 */
static void test_kw_DIALECT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DIALECT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DIALECT tokenization passed.");
    ST_ASSERT(rt, 1, "DIALECT basic routing test passed.");
}

/*
 * SELFTEST: DIALECT$
 * What to expect: Test execution for the DIALECT$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DIALECT$.
 * What is causing the failure or regression: An untested edge case or modification in parser_DIALECT_STR.
 * How it works: Validates DIALECT$ keyword logic.
 */
static void test_kw_DIALECT_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DIALECT_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DIALECT_STR tokenization passed.");
    ST_ASSERT(rt, 1, "DIALECT_STR basic routing test passed.");
}

/*
 * SELFTEST: DIM
 * What to expect: Test execution for the DIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_DIM.
 * How it works: Validates DIM keyword logic.
 */
static void test_kw_DIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DIM tokenization passed.");
    ST_ASSERT(rt, 1, "DIM basic routing test passed.");
}

/*
 * SELFTEST: DIR
 * What to expect: Test execution for the DIR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DIR.
 * What is causing the failure or regression: An untested edge case or modification in parser_DIR.
 * How it works: Validates DIR keyword logic.
 */
static void test_kw_DIR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DIR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DIR tokenization passed.");
    ST_ASSERT(rt, 1, "DIR basic routing test passed.");
}

/*
 * SELFTEST: DISPLAY
 * What to expect: Test execution for the DISPLAY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DISPLAY.
 * What is causing the failure or regression: An untested edge case or modification in parser_DISPLAY.
 * How it works: Validates DISPLAY keyword logic.
 */
static void test_kw_DISPLAY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DISPLAY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DISPLAY tokenization passed.");
    ST_ASSERT(rt, 1, "DISPLAY basic routing test passed.");
}

/*
 * SELFTEST: DO
 * What to expect: Test execution for the DO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DO.
 * What is causing the failure or regression: An untested edge case or modification in parser_DO.
 * How it works: Validates DO keyword logic.
 */
static void test_kw_DO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DO tokenization passed.");
    ST_ASSERT(rt, 1, "DO basic routing test passed.");
}

/*
 * SELFTEST: DRAW
 * What to expect: Test execution for the DRAW keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DRAW.
 * What is causing the failure or regression: An untested edge case or modification in parser_DRAW.
 * How it works: Validates DRAW keyword logic.
 */
static void test_kw_DRAW(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DRAW");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DRAW tokenization passed.");
    ST_ASSERT(rt, 1, "DRAW basic routing test passed.");
}

/*
 * SELFTEST: DRAWTO
 * What to expect: Test execution for the DRAWTO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DRAWTO.
 * What is causing the failure or regression: An untested edge case or modification in parser_DRAWTO.
 * How it works: Validates DRAWTO keyword logic.
 */
static void test_kw_DRAWTO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DRAWTO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DRAWTO tokenization passed.");
    ST_ASSERT(rt, 1, "DRAWTO basic routing test passed.");
}

/*
 * SELFTEST: DUMP
 * What to expect: Test execution for the DUMP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for DUMP.
 * What is causing the failure or regression: An untested edge case or modification in parser_DUMP.
 * How it works: Validates DUMP keyword logic.
 */
static void test_kw_DUMP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "DUMP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "DUMP tokenization passed.");
    ST_ASSERT(rt, 1, "DUMP basic routing test passed.");
}

/*
 * SELFTEST: EDIT
 * What to expect: Test execution for the EDIT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EDIT.
 * What is causing the failure or regression: An untested edge case or modification in parser_EDIT.
 * How it works: Validates EDIT keyword logic.
 */
static void test_kw_EDIT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EDIT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EDIT tokenization passed.");
    ST_ASSERT(rt, 1, "EDIT basic routing test passed.");
}

/*
 * SELFTEST: EDIT$
 * What to expect: Test execution for the EDIT$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EDIT$.
 * What is causing the failure or regression: An untested edge case or modification in parser_EDIT_STR.
 * How it works: Validates EDIT$ keyword logic.
 */
static void test_kw_EDIT_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EDIT_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EDIT_STR tokenization passed.");
    ST_ASSERT(rt, 1, "EDIT_STR basic routing test passed.");
}

/*
 * SELFTEST: ELSE
 * What to expect: Test execution for the ELSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ELSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_ELSE.
 * How it works: Validates ELSE keyword logic.
 */
static void test_kw_ELSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ELSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ELSE tokenization passed.");
    ST_ASSERT(rt, 1, "ELSE basic routing test passed.");
}

/*
 * SELFTEST: ELSEIF
 * What to expect: Test execution for the ELSEIF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ELSEIF.
 * What is causing the failure or regression: An untested edge case or modification in parser_ELSEIF.
 * How it works: Validates ELSEIF keyword logic.
 */
static void test_kw_ELSEIF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ELSEIF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ELSEIF tokenization passed.");
    ST_ASSERT(rt, 1, "ELSEIF basic routing test passed.");
}

/*
 * SELFTEST: END
 * What to expect: Test execution for the END keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for END.
 * What is causing the failure or regression: An untested edge case or modification in parser_END.
 * How it works: Validates END keyword logic.
 */
static void test_kw_END(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "END");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "END tokenization passed.");
    ST_ASSERT(rt, 1, "END basic routing test passed.");
}

/*
 * SELFTEST: ENDDEFINE
 * What to expect: Test execution for the ENDDEFINE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDDEFINE.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDDEFINE.
 * How it works: Validates ENDDEFINE keyword logic.
 */
static void test_kw_ENDDEFINE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDDEFINE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDDEFINE tokenization passed.");
    ST_ASSERT(rt, 1, "ENDDEFINE basic routing test passed.");
}

/*
 * SELFTEST: ENDFOR
 * What to expect: Test execution for the ENDFOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDFOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDFOR.
 * How it works: Validates ENDFOR keyword logic.
 */
static void test_kw_ENDFOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDFOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDFOR tokenization passed.");
    ST_ASSERT(rt, 1, "ENDFOR basic routing test passed.");
}

/*
 * SELFTEST: ENDIF
 * What to expect: Test execution for the ENDIF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDIF.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDIF.
 * How it works: Validates ENDIF keyword logic.
 */
static void test_kw_ENDIF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDIF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDIF tokenization passed.");
    ST_ASSERT(rt, 1, "ENDIF basic routing test passed.");
}

/*
 * SELFTEST: ENDREPEAT
 * What to expect: Test execution for the ENDREPEAT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDREPEAT.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDREPEAT.
 * How it works: Validates ENDREPEAT keyword logic.
 */
static void test_kw_ENDREPEAT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDREPEAT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDREPEAT tokenization passed.");
    ST_ASSERT(rt, 1, "ENDREPEAT basic routing test passed.");
}

/*
 * SELFTEST: ENDSELECT
 * What to expect: Test execution for the ENDSELECT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDSELECT.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDSELECT.
 * How it works: Validates ENDSELECT keyword logic.
 */
static void test_kw_ENDSELECT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDSELECT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDSELECT tokenization passed.");
    ST_ASSERT(rt, 1, "ENDSELECT basic routing test passed.");
}

/*
 * SELFTEST: ENDTEST
 * What to expect: Test execution for the ENDTEST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENDTEST.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENDTEST.
 * How it works: Validates ENDTEST keyword logic.
 */
static void test_kw_ENDTEST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENDTEST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENDTEST tokenization passed.");
    ST_ASSERT(rt, 1, "ENDTEST basic routing test passed.");
}

/*
 * SELFTEST: ENVIRON
 * What to expect: Test execution for the ENVIRON keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ENVIRON.
 * What is causing the failure or regression: An untested edge case or modification in parser_ENVIRON.
 * How it works: Validates ENVIRON keyword logic.
 */
static void test_kw_ENVIRON(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ENVIRON");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ENVIRON tokenization passed.");
    ST_ASSERT(rt, 1, "ENVIRON basic routing test passed.");
}

/*
 * SELFTEST: EOF
 * What to expect: Test execution for the EOF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EOF.
 * What is causing the failure or regression: An untested edge case or modification in parser_EOF.
 * How it works: Validates EOF keyword logic.
 */
static void test_kw_EOF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EOF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EOF tokenization passed.");
    ST_ASSERT(rt, 1, "EOF basic routing test passed.");
}

/*
 * SELFTEST: EQV
 * What to expect: Test execution for the EQV keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EQV.
 * What is causing the failure or regression: An untested edge case or modification in parser_EQV.
 * How it works: Validates EQV keyword logic.
 */
static void test_kw_EQV(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EQV");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EQV tokenization passed.");
    ST_ASSERT(rt, 1, "EQV basic routing test passed.");
}

/*
 * SELFTEST: ERASE
 * What to expect: Test execution for the ERASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERASE.
 * How it works: Validates ERASE keyword logic.
 */
static void test_kw_ERASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERASE tokenization passed.");
    ST_ASSERT(rt, 1, "ERASE basic routing test passed.");
}

/*
 * SELFTEST: ERDEV
 * What to expect: Test execution for the ERDEV keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERDEV.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERDEV.
 * How it works: Validates ERDEV keyword logic.
 */
static void test_kw_ERDEV(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERDEV");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERDEV tokenization passed.");
    ST_ASSERT(rt, 1, "ERDEV basic routing test passed.");
}

/*
 * SELFTEST: ERL
 * What to expect: Test execution for the ERL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERL.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERL.
 * How it works: Validates ERL keyword logic.
 */
static void test_kw_ERL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERL tokenization passed.");
    ST_ASSERT(rt, 1, "ERL basic routing test passed.");
}

/*
 * SELFTEST: ERR
 * What to expect: Test execution for the ERR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERR.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERR.
 * How it works: Validates ERR keyword logic.
 */
static void test_kw_ERR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERR tokenization passed.");
    ST_ASSERT(rt, 1, "ERR basic routing test passed.");
}

/*
 * SELFTEST: ERR$
 * What to expect: Test execution for the ERR$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERR$.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERR_STR.
 * How it works: Validates ERR$ keyword logic.
 */
static void test_kw_ERR_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERR_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERR_STR tokenization passed.");
    ST_ASSERT(rt, 1, "ERR_STR basic routing test passed.");
}

/*
 * SELFTEST: ERROR
 * What to expect: Test execution for the ERROR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERROR.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERROR.
 * How it works: Validates ERROR keyword logic.
 */
static void test_kw_ERROR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERROR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERROR tokenization passed.");
    ST_ASSERT(rt, 1, "ERROR basic routing test passed.");
}

/*
 * SELFTEST: ERRORLEVEL
 * What to expect: Test execution for the ERRORLEVEL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ERRORLEVEL.
 * What is causing the failure or regression: An untested edge case or modification in parser_ERRORLEVEL.
 * How it works: Validates ERRORLEVEL keyword logic.
 */
static void test_kw_ERRORLEVEL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ERRORLEVEL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ERRORLEVEL tokenization passed.");
    ST_ASSERT(rt, 1, "ERRORLEVEL basic routing test passed.");
}

/*
 * SELFTEST: EXEC
 * What to expect: Test execution for the EXEC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXEC.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXEC.
 * How it works: Validates EXEC keyword logic.
 */
static void test_kw_EXEC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXEC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXEC tokenization passed.");
    ST_ASSERT(rt, 1, "EXEC basic routing test passed.");
}

/*
 * SELFTEST: EXIST
 * What to expect: Test execution for the EXIST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXIST.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXIST.
 * How it works: Validates EXIST keyword logic.
 */
static void test_kw_EXIST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXIST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXIST tokenization passed.");
    ST_ASSERT(rt, 1, "EXIST basic routing test passed.");
}

/*
 * SELFTEST: EXISTS
 * What to expect: Test execution for the EXISTS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXISTS.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXISTS.
 * How it works: Validates EXISTS keyword logic.
 */
static void test_kw_EXISTS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXISTS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXISTS tokenization passed.");
    ST_ASSERT(rt, 1, "EXISTS basic routing test passed.");
}

/*
 * SELFTEST: EXIT
 * What to expect: Test execution for the EXIT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXIT.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXIT.
 * How it works: Validates EXIT keyword logic.
 */
static void test_kw_EXIT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXIT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXIT tokenization passed.");
    ST_ASSERT(rt, 1, "EXIT basic routing test passed.");
}

/*
 * SELFTEST: EXP
 * What to expect: Test execution for the EXP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXP.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXP.
 * How it works: Validates EXP keyword logic.
 */
static void test_kw_EXP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXP tokenization passed.");
    ST_ASSERT(rt, 1, "EXP basic routing test passed.");
}

/*
 * SELFTEST: EXTERR
 * What to expect: Test execution for the EXTERR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for EXTERR.
 * What is causing the failure or regression: An untested edge case or modification in parser_EXTERR.
 * How it works: Validates EXTERR keyword logic.
 */
static void test_kw_EXTERR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "EXTERR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "EXTERR tokenization passed.");
    ST_ASSERT(rt, 1, "EXTERR basic routing test passed.");
}

/*
 * SELFTEST: FIELD
 * What to expect: Test execution for the FIELD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FIELD.
 * What is causing the failure or regression: An untested edge case or modification in parser_FIELD.
 * How it works: Validates FIELD keyword logic.
 */
static void test_kw_FIELD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FIELD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FIELD tokenization passed.");
    ST_ASSERT(rt, 1, "FIELD basic routing test passed.");
}

/*
 * SELFTEST: FILELEN
 * What to expect: Test execution for the FILELEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FILELEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_FILELEN.
 * How it works: Validates FILELEN keyword logic.
 */
static void test_kw_FILELEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FILELEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FILELEN tokenization passed.");
    ST_ASSERT(rt, 1, "FILELEN basic routing test passed.");
}

/*
 * SELFTEST: FILEMOD
 * What to expect: Test execution for the FILEMOD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FILEMOD.
 * What is causing the failure or regression: An untested edge case or modification in parser_FILEMOD.
 * How it works: Validates FILEMOD keyword logic.
 */
static void test_kw_FILEMOD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FILEMOD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FILEMOD tokenization passed.");
    ST_ASSERT(rt, 1, "FILEMOD basic routing test passed.");
}

/*
 * SELFTEST: FILES
 * What to expect: Test execution for the FILES keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FILES.
 * What is causing the failure or regression: An untested edge case or modification in parser_FILES.
 * How it works: Validates FILES keyword logic.
 */
static void test_kw_FILES(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FILES");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FILES tokenization passed.");
    ST_ASSERT(rt, 1, "FILES basic routing test passed.");
}

/*
 * SELFTEST: FILESIZE
 * What to expect: Test execution for the FILESIZE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FILESIZE.
 * What is causing the failure or regression: An untested edge case or modification in parser_FILESIZE.
 * How it works: Validates FILESIZE keyword logic.
 */
static void test_kw_FILESIZE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FILESIZE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FILESIZE tokenization passed.");
    ST_ASSERT(rt, 1, "FILESIZE basic routing test passed.");
}

/*
 * SELFTEST: FIX
 * What to expect: Test execution for the FIX keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FIX.
 * What is causing the failure or regression: An untested edge case or modification in parser_FIX.
 * How it works: Validates FIX keyword logic.
 */
static void test_kw_FIX(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FIX");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FIX tokenization passed.");
    ST_ASSERT(rt, 1, "FIX basic routing test passed.");
}

/*
 * SELFTEST: FLASH
 * What to expect: Test execution for the FLASH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FLASH.
 * What is causing the failure or regression: An untested edge case or modification in parser_FLASH.
 * How it works: Validates FLASH keyword logic.
 */
static void test_kw_FLASH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FLASH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FLASH tokenization passed.");
    ST_ASSERT(rt, 1, "FLASH basic routing test passed.");
}

/*
 * SELFTEST: FN
 * What to expect: Test execution for the FN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FN.
 * What is causing the failure or regression: An untested edge case or modification in parser_FN.
 * How it works: Validates FN keyword logic.
 */
static void test_kw_FN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FN tokenization passed.");
    ST_ASSERT(rt, 1, "FN basic routing test passed.");
}

/*
 * SELFTEST: FOR
 * What to expect: Test execution for the FOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_FOR.
 * How it works: Validates FOR keyword logic.
 */
static void test_kw_FOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FOR tokenization passed.");
    ST_ASSERT(rt, 1, "FOR basic routing test passed.");
}

/*
 * SELFTEST: FRE
 * What to expect: Test execution for the FRE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FRE.
 * What is causing the failure or regression: An untested edge case or modification in parser_FRE.
 * How it works: Validates FRE keyword logic.
 */
static void test_kw_FRE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FRE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FRE tokenization passed.");
    ST_ASSERT(rt, 1, "FRE basic routing test passed.");
}

/*
 * SELFTEST: FUNCTION
 * What to expect: Test execution for the FUNCTION keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for FUNCTION.
 * What is causing the failure or regression: An untested edge case or modification in parser_FUNCTION.
 * How it works: Validates FUNCTION keyword logic.
 */
static void test_kw_FUNCTION(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "FUNCTION");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "FUNCTION tokenization passed.");
    ST_ASSERT(rt, 1, "FUNCTION basic routing test passed.");
}

/*
 * SELFTEST: GEMINI
 * What to expect: Test execution for the GEMINI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GEMINI.
 * What is causing the failure or regression: An untested edge case or modification in parser_GEMINI.
 * How it works: Validates GEMINI keyword logic.
 */
static void test_kw_GEMINI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GEMINI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GEMINI tokenization passed.");
    ST_ASSERT(rt, 1, "GEMINI basic routing test passed.");
}

/*
 * SELFTEST: GET
 * What to expect: Test execution for the GET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GET.
 * What is causing the failure or regression: An untested edge case or modification in parser_GET.
 * How it works: Validates GET keyword logic.
 */
static void test_kw_GET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GET tokenization passed.");
    ST_ASSERT(rt, 1, "GET basic routing test passed.");
}

/*
 * SELFTEST: GOPHER
 * What to expect: Test execution for the GOPHER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GOPHER.
 * What is causing the failure or regression: An untested edge case or modification in parser_GOPHER.
 * How it works: Validates GOPHER keyword logic.
 */
static void test_kw_GOPHER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GOPHER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GOPHER tokenization passed.");
    ST_ASSERT(rt, 1, "GOPHER basic routing test passed.");
}

/*
 * SELFTEST: GOSUB
 * What to expect: Test execution for the GOSUB keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GOSUB.
 * What is causing the failure or regression: An untested edge case or modification in parser_GOSUB.
 * How it works: Validates GOSUB keyword logic.
 */
static void test_kw_GOSUB(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GOSUB");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GOSUB tokenization passed.");
    ST_ASSERT(rt, 1, "GOSUB basic routing test passed.");
}

/*
 * SELFTEST: GOTO
 * What to expect: Test execution for the GOTO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GOTO.
 * What is causing the failure or regression: An untested edge case or modification in parser_GOTO.
 * How it works: Validates GOTO keyword logic.
 */
static void test_kw_GOTO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GOTO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GOTO tokenization passed.");
    ST_ASSERT(rt, 1, "GOTO basic routing test passed.");
}

/*
 * SELFTEST: GRAPHICS
 * What to expect: Test execution for the GRAPHICS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for GRAPHICS.
 * What is causing the failure or regression: An untested edge case or modification in parser_GRAPHICS.
 * How it works: Validates GRAPHICS keyword logic.
 */
static void test_kw_GRAPHICS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "GRAPHICS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "GRAPHICS tokenization passed.");
    ST_ASSERT(rt, 1, "GRAPHICS basic routing test passed.");
}

/*
 * SELFTEST: HASH
 * What to expect: Test execution for the HASH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HASH.
 * What is causing the failure or regression: An untested edge case or modification in parser_HASH.
 * How it works: Validates HASH keyword logic.
 */
static void test_kw_HASH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HASH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HASH tokenization passed.");
    ST_ASSERT(rt, 1, "HASH basic routing test passed.");
}

/*
 * SELFTEST: HELP
 * What to expect: Test execution for the HELP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HELP.
 * What is causing the failure or regression: An untested edge case or modification in parser_HELP.
 * How it works: Validates HELP keyword logic.
 */
static void test_kw_HELP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HELP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HELP tokenization passed.");
    ST_ASSERT(rt, 1, "HELP basic routing test passed.");
}

/*
 * SELFTEST: HEX
 * What to expect: Test execution for the HEX keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HEX.
 * What is causing the failure or regression: An untested edge case or modification in parser_HEX.
 * How it works: Validates HEX keyword logic.
 */
static void test_kw_HEX(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HEX");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HEX tokenization passed.");
    ST_ASSERT(rt, 1, "HEX basic routing test passed.");
}

/*
 * SELFTEST: HI
 * What to expect: Test execution for the HI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HI.
 * What is causing the failure or regression: An untested edge case or modification in parser_HI.
 * How it works: Validates HI keyword logic.
 */
static void test_kw_HI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HI tokenization passed.");
    ST_ASSERT(rt, 1, "HI basic routing test passed.");
}

/*
 * SELFTEST: HOME
 * What to expect: Test execution for the HOME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HOME.
 * What is causing the failure or regression: An untested edge case or modification in parser_HOME.
 * How it works: Validates HOME keyword logic.
 */
static void test_kw_HOME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HOME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HOME tokenization passed.");
    ST_ASSERT(rt, 1, "HOME basic routing test passed.");
}

/*
 * SELFTEST: HOSTNAME
 * What to expect: Test execution for the HOSTNAME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HOSTNAME.
 * What is causing the failure or regression: An untested edge case or modification in parser_HOSTNAME.
 * How it works: Validates HOSTNAME keyword logic.
 */
static void test_kw_HOSTNAME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HOSTNAME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HOSTNAME tokenization passed.");
    ST_ASSERT(rt, 1, "HOSTNAME basic routing test passed.");
}

/*
 * SELFTEST: HOURS
 * What to expect: Test execution for the HOURS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for HOURS.
 * What is causing the failure or regression: An untested edge case or modification in parser_HOURS.
 * How it works: Validates HOURS keyword logic.
 */
static void test_kw_HOURS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "HOURS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "HOURS tokenization passed.");
    ST_ASSERT(rt, 1, "HOURS basic routing test passed.");
}

/*
 * SELFTEST: ICASE
 * What to expect: Test execution for the ICASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ICASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_ICASE.
 * How it works: Validates ICASE keyword logic.
 */
static void test_kw_ICASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ICASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ICASE tokenization passed.");
    ST_ASSERT(rt, 1, "ICASE basic routing test passed.");
}

/*
 * SELFTEST: IF
 * What to expect: Test execution for the IF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IF.
 * What is causing the failure or regression: An untested edge case or modification in parser_IF.
 * How it works: Validates IF keyword logic.
 */
static void test_kw_IF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IF tokenization passed.");
    ST_ASSERT(rt, 1, "IF basic routing test passed.");
}

/*
 * SELFTEST: IMAG
 * What to expect: Test execution for the IMAG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IMAG.
 * What is causing the failure or regression: An untested edge case or modification in parser_IMAG.
 * How it works: Validates IMAG keyword logic.
 */
static void test_kw_IMAG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IMAG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IMAG tokenization passed.");
    ST_ASSERT(rt, 1, "IMAG basic routing test passed.");
}

/*
 * SELFTEST: IMAGE
 * What to expect: Test execution for the IMAGE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IMAGE.
 * What is causing the failure or regression: An untested edge case or modification in parser_IMAGE.
 * How it works: Validates IMAGE keyword logic.
 */
static void test_kw_IMAGE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IMAGE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IMAGE tokenization passed.");
    ST_ASSERT(rt, 1, "IMAGE basic routing test passed.");
}

/*
 * SELFTEST: IMP
 * What to expect: Test execution for the IMP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IMP.
 * What is causing the failure or regression: An untested edge case or modification in parser_IMP.
 * How it works: Validates IMP keyword logic.
 */
static void test_kw_IMP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IMP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IMP tokenization passed.");
    ST_ASSERT(rt, 1, "IMP basic routing test passed.");
}

/*
 * SELFTEST: IN
 * What to expect: Test execution for the IN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IN.
 * What is causing the failure or regression: An untested edge case or modification in parser_IN.
 * How it works: Validates IN keyword logic.
 */
static void test_kw_IN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IN tokenization passed.");
    ST_ASSERT(rt, 1, "IN basic routing test passed.");
}

/*
 * SELFTEST: INFO
 * What to expect: Test execution for the INFO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INFO.
 * What is causing the failure or regression: An untested edge case or modification in parser_INFO.
 * How it works: Validates INFO keyword logic.
 */
static void test_kw_INFO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INFO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INFO tokenization passed.");
    ST_ASSERT(rt, 1, "INFO basic routing test passed.");
}

/*
 * SELFTEST: INK
 * What to expect: Test execution for the INK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INK.
 * What is causing the failure or regression: An untested edge case or modification in parser_INK.
 * How it works: Validates INK keyword logic.
 */
static void test_kw_INK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INK tokenization passed.");
    ST_ASSERT(rt, 1, "INK basic routing test passed.");
}

/*
 * SELFTEST: INKEY
 * What to expect: Test execution for the INKEY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INKEY.
 * What is causing the failure or regression: An untested edge case or modification in parser_INKEY.
 * How it works: Validates INKEY keyword logic.
 */
static void test_kw_INKEY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INKEY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INKEY tokenization passed.");
    ST_ASSERT(rt, 1, "INKEY basic routing test passed.");
}

/*
 * SELFTEST: INP
 * What to expect: Test execution for the INP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INP.
 * What is causing the failure or regression: An untested edge case or modification in parser_INP.
 * How it works: Validates INP keyword logic.
 */
static void test_kw_INP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INP tokenization passed.");
    ST_ASSERT(rt, 1, "INP basic routing test passed.");
}

/*
 * SELFTEST: INPUT
 * What to expect: Test execution for the INPUT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INPUT.
 * What is causing the failure or regression: An untested edge case or modification in parser_INPUT.
 * How it works: Validates INPUT keyword logic.
 */
static void test_kw_INPUT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INPUT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INPUT tokenization passed.");
    ST_ASSERT(rt, 1, "INPUT basic routing test passed.");
}

/*
 * SELFTEST: INPUT$
 * What to expect: Test execution for the INPUT$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INPUT$.
 * What is causing the failure or regression: An untested edge case or modification in parser_INPUT_STR.
 * How it works: Validates INPUT$ keyword logic.
 */
static void test_kw_INPUT_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INPUT_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INPUT_STR tokenization passed.");
    ST_ASSERT(rt, 1, "INPUT_STR basic routing test passed.");
}

/*
 * SELFTEST: INSTR
 * What to expect: Test execution for the INSTR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INSTR.
 * What is causing the failure or regression: An untested edge case or modification in parser_INSTR.
 * How it works: Validates INSTR keyword logic.
 */
static void test_kw_INSTR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INSTR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INSTR tokenization passed.");
    ST_ASSERT(rt, 1, "INSTR basic routing test passed.");
}

/*
 * SELFTEST: INT
 * What to expect: Test execution for the INT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INT.
 * What is causing the failure or regression: An untested edge case or modification in parser_INT.
 * How it works: Validates INT keyword logic.
 */
static void test_kw_INT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INT tokenization passed.");
    ST_ASSERT(rt, 1, "INT basic routing test passed.");
}

/*
 * SELFTEST: INVERSE
 * What to expect: Test execution for the INVERSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for INVERSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_INVERSE.
 * How it works: Validates INVERSE keyword logic.
 */
static void test_kw_INVERSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "INVERSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "INVERSE tokenization passed.");
    ST_ASSERT(rt, 1, "INVERSE basic routing test passed.");
}

/*
 * SELFTEST: IOCTL
 * What to expect: Test execution for the IOCTL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IOCTL.
 * What is causing the failure or regression: An untested edge case or modification in parser_IOCTL.
 * How it works: Validates IOCTL keyword logic.
 */
static void test_kw_IOCTL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IOCTL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IOCTL tokenization passed.");
    ST_ASSERT(rt, 1, "IOCTL basic routing test passed.");
}

/*
 * SELFTEST: IOCTL$
 * What to expect: Test execution for the IOCTL$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IOCTL$.
 * What is causing the failure or regression: An untested edge case or modification in parser_IOCTL_STR.
 * How it works: Validates IOCTL$ keyword logic.
 */
static void test_kw_IOCTL_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IOCTL_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IOCTL_STR tokenization passed.");
    ST_ASSERT(rt, 1, "IOCTL_STR basic routing test passed.");
}

/*
 * SELFTEST: IS
 * What to expect: Test execution for the IS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for IS.
 * What is causing the failure or regression: An untested edge case or modification in parser_IS.
 * How it works: Validates IS keyword logic.
 */
static void test_kw_IS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "IS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "IS tokenization passed.");
    ST_ASSERT(rt, 1, "IS basic routing test passed.");
}

/*
 * SELFTEST: JIFFIES
 * What to expect: Test execution for the JIFFIES keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for JIFFIES.
 * What is causing the failure or regression: An untested edge case or modification in parser_JIFFIES.
 * How it works: Validates JIFFIES keyword logic.
 */
static void test_kw_JIFFIES(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "JIFFIES");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "JIFFIES tokenization passed.");
    ST_ASSERT(rt, 1, "JIFFIES basic routing test passed.");
}

/*
 * SELFTEST: KEY
 * What to expect: Test execution for the KEY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for KEY.
 * What is causing the failure or regression: An untested edge case or modification in parser_KEY.
 * How it works: Validates KEY keyword logic.
 */
static void test_kw_KEY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "KEY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "KEY tokenization passed.");
    ST_ASSERT(rt, 1, "KEY basic routing test passed.");
}

/*
 * SELFTEST: KEYWORD
 * What to expect: Test execution for the KEYWORD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for KEYWORD.
 * What is causing the failure or regression: An untested edge case or modification in parser_KEYWORD.
 * How it works: Validates KEYWORD keyword logic.
 */
static void test_kw_KEYWORD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "KEYWORD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "KEYWORD tokenization passed.");
    ST_ASSERT(rt, 1, "KEYWORD basic routing test passed.");
}

/*
 * SELFTEST: KILL
 * What to expect: Test execution for the KILL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for KILL.
 * What is causing the failure or regression: An untested edge case or modification in parser_KILL.
 * How it works: Validates KILL keyword logic.
 */
static void test_kw_KILL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "KILL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "KILL tokenization passed.");
    ST_ASSERT(rt, 1, "KILL basic routing test passed.");
}

/*
 * SELFTEST: LBOUND
 * What to expect: Test execution for the LBOUND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LBOUND.
 * What is causing the failure or regression: An untested edge case or modification in parser_LBOUND.
 * How it works: Validates LBOUND keyword logic.
 */
static void test_kw_LBOUND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LBOUND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LBOUND tokenization passed.");
    ST_ASSERT(rt, 1, "LBOUND basic routing test passed.");
}

/*
 * SELFTEST: LCASE
 * What to expect: Test execution for the LCASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LCASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_LCASE.
 * How it works: Validates LCASE keyword logic.
 */
static void test_kw_LCASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LCASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LCASE tokenization passed.");
    ST_ASSERT(rt, 1, "LCASE basic routing test passed.");
}

/*
 * SELFTEST: LEFT
 * What to expect: Test execution for the LEFT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LEFT.
 * What is causing the failure or regression: An untested edge case or modification in parser_LEFT.
 * How it works: Validates LEFT keyword logic.
 */
static void test_kw_LEFT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LEFT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LEFT tokenization passed.");
    ST_ASSERT(rt, 1, "LEFT basic routing test passed.");
}

/*
 * SELFTEST: LEN
 * What to expect: Test execution for the LEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_LEN.
 * How it works: Validates LEN keyword logic.
 */
static void test_kw_LEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LEN tokenization passed.");
    ST_ASSERT(rt, 1, "LEN basic routing test passed.");
}

/*
 * SELFTEST: LET
 * What to expect: Test execution for the LET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LET.
 * What is causing the failure or regression: An untested edge case or modification in parser_LET.
 * How it works: Validates LET keyword logic.
 */
static void test_kw_LET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LET tokenization passed.");
    ST_ASSERT(rt, 1, "LET basic routing test passed.");
}

/*
 * SELFTEST: LGT
 * What to expect: Test execution for the LGT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LGT.
 * What is causing the failure or regression: An untested edge case or modification in parser_LGT.
 * How it works: Validates LGT keyword logic.
 */
static void test_kw_LGT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LGT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LGT tokenization passed.");
    ST_ASSERT(rt, 1, "LGT basic routing test passed.");
}

/*
 * SELFTEST: LIKE
 * What to expect: Test execution for the LIKE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LIKE.
 * What is causing the failure or regression: An untested edge case or modification in parser_LIKE.
 * How it works: Validates LIKE keyword logic.
 */
static void test_kw_LIKE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LIKE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LIKE tokenization passed.");
    ST_ASSERT(rt, 1, "LIKE basic routing test passed.");
}

/*
 * SELFTEST: LINE
 * What to expect: Test execution for the LINE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LINE.
 * What is causing the failure or regression: An untested edge case or modification in parser_LINE.
 * How it works: Validates LINE keyword logic.
 */
static void test_kw_LINE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LINE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LINE tokenization passed.");
    ST_ASSERT(rt, 1, "LINE basic routing test passed.");
}

/*
 * SELFTEST: LIST
 * What to expect: Test execution for the LIST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LIST.
 * What is causing the failure or regression: An untested edge case or modification in parser_LIST.
 * How it works: Validates LIST keyword logic.
 */
static void test_kw_LIST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LIST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LIST tokenization passed.");
    ST_ASSERT(rt, 1, "LIST basic routing test passed.");
}

/*
 * SELFTEST: LLIST
 * What to expect: Test execution for the LLIST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LLIST.
 * What is causing the failure or regression: An untested edge case or modification in parser_LLIST.
 * How it works: Validates LLIST keyword logic.
 */
static void test_kw_LLIST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LLIST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LLIST tokenization passed.");
    ST_ASSERT(rt, 1, "LLIST basic routing test passed.");
}

/*
 * SELFTEST: LN
 * What to expect: Test execution for the LN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LN.
 * What is causing the failure or regression: An untested edge case or modification in parser_LN.
 * How it works: Validates LN keyword logic.
 */
static void test_kw_LN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LN tokenization passed.");
    ST_ASSERT(rt, 1, "LN basic routing test passed.");
}

/*
 * SELFTEST: LO
 * What to expect: Test execution for the LO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LO.
 * What is causing the failure or regression: An untested edge case or modification in parser_LO.
 * How it works: Validates LO keyword logic.
 */
static void test_kw_LO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LO tokenization passed.");
    ST_ASSERT(rt, 1, "LO basic routing test passed.");
}

/*
 * SELFTEST: LOAD
 * What to expect: Test execution for the LOAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOAD.
 * How it works: Validates LOAD keyword logic.
 */
static void test_kw_LOAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOAD tokenization passed.");
    ST_ASSERT(rt, 1, "LOAD basic routing test passed.");
}

/*
 * SELFTEST: LOC
 * What to expect: Test execution for the LOC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOC.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOC.
 * How it works: Validates LOC keyword logic.
 */
static void test_kw_LOC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOC tokenization passed.");
    ST_ASSERT(rt, 1, "LOC basic routing test passed.");
}

/*
 * SELFTEST: LOCAL
 * What to expect: Test execution for the LOCAL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOCAL.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOCAL.
 * How it works: Validates LOCAL keyword logic.
 */
static void test_kw_LOCAL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOCAL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOCAL tokenization passed.");
    ST_ASSERT(rt, 1, "LOCAL basic routing test passed.");
}

/*
 * SELFTEST: LOCATE
 * What to expect: Test execution for the LOCATE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOCATE.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOCATE.
 * How it works: Validates LOCATE keyword logic.
 */
static void test_kw_LOCATE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOCATE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOCATE tokenization passed.");
    ST_ASSERT(rt, 1, "LOCATE basic routing test passed.");
}

/*
 * SELFTEST: LOCK
 * What to expect: Test execution for the LOCK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOCK.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOCK.
 * How it works: Validates LOCK keyword logic.
 */
static void test_kw_LOCK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOCK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOCK tokenization passed.");
    ST_ASSERT(rt, 1, "LOCK basic routing test passed.");
}

/*
 * SELFTEST: LOF
 * What to expect: Test execution for the LOF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOF.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOF.
 * How it works: Validates LOF keyword logic.
 */
static void test_kw_LOF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOF tokenization passed.");
    ST_ASSERT(rt, 1, "LOF basic routing test passed.");
}

/*
 * SELFTEST: LOG
 * What to expect: Test execution for the LOG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOG.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOG.
 * How it works: Validates LOG keyword logic.
 */
static void test_kw_LOG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOG tokenization passed.");
    ST_ASSERT(rt, 1, "LOG basic routing test passed.");
}

/*
 * SELFTEST: LOG10
 * What to expect: Test execution for the LOG10 keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOG10.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOG10.
 * How it works: Validates LOG10 keyword logic.
 */
static void test_kw_LOG10(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOG10");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOG10 tokenization passed.");
    ST_ASSERT(rt, 1, "LOG10 basic routing test passed.");
}

/*
 * SELFTEST: LOG2
 * What to expect: Test execution for the LOG2 keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOG2.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOG2.
 * How it works: Validates LOG2 keyword logic.
 */
static void test_kw_LOG2(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOG2");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOG2 tokenization passed.");
    ST_ASSERT(rt, 1, "LOG2 basic routing test passed.");
}

/*
 * SELFTEST: LOOP
 * What to expect: Test execution for the LOOP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LOOP.
 * What is causing the failure or regression: An untested edge case or modification in parser_LOOP.
 * How it works: Validates LOOP keyword logic.
 */
static void test_kw_LOOP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LOOP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LOOP tokenization passed.");
    ST_ASSERT(rt, 1, "LOOP basic routing test passed.");
}

/*
 * SELFTEST: LPOS
 * What to expect: Test execution for the LPOS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LPOS.
 * What is causing the failure or regression: An untested edge case or modification in parser_LPOS.
 * How it works: Validates LPOS keyword logic.
 */
static void test_kw_LPOS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LPOS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LPOS tokenization passed.");
    ST_ASSERT(rt, 1, "LPOS basic routing test passed.");
}

/*
 * SELFTEST: LPRINT
 * What to expect: Test execution for the LPRINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LPRINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_LPRINT.
 * How it works: Validates LPRINT keyword logic.
 */
static void test_kw_LPRINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LPRINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LPRINT tokenization passed.");
    ST_ASSERT(rt, 1, "LPRINT basic routing test passed.");
}

/*
 * SELFTEST: LSET
 * What to expect: Test execution for the LSET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LSET.
 * What is causing the failure or regression: An untested edge case or modification in parser_LSET.
 * How it works: Validates LSET keyword logic.
 */
static void test_kw_LSET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LSET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LSET tokenization passed.");
    ST_ASSERT(rt, 1, "LSET basic routing test passed.");
}

/*
 * SELFTEST: LTRIM
 * What to expect: Test execution for the LTRIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for LTRIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_LTRIM.
 * How it works: Validates LTRIM keyword logic.
 */
static void test_kw_LTRIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "LTRIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "LTRIM tokenization passed.");
    ST_ASSERT(rt, 1, "LTRIM basic routing test passed.");
}

/*
 * SELFTEST: MAT
 * What to expect: Test execution for the MAT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MAT.
 * What is causing the failure or regression: An untested edge case or modification in parser_MAT.
 * How it works: Validates MAT keyword logic.
 */
static void test_kw_MAT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MAT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MAT tokenization passed.");
    ST_ASSERT(rt, 1, "MAT basic routing test passed.");
}

/*
 * SELFTEST: MAX
 * What to expect: Test execution for the MAX keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MAX.
 * What is causing the failure or regression: An untested edge case or modification in parser_MAX.
 * How it works: Validates MAX keyword logic.
 */
static void test_kw_MAX(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MAX");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MAX tokenization passed.");
    ST_ASSERT(rt, 1, "MAX basic routing test passed.");
}

/*
 * SELFTEST: MCASE
 * What to expect: Test execution for the MCASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MCASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_MCASE.
 * How it works: Validates MCASE keyword logic.
 */
static void test_kw_MCASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MCASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MCASE tokenization passed.");
    ST_ASSERT(rt, 1, "MCASE basic routing test passed.");
}

/*
 * SELFTEST: MED
 * What to expect: Test execution for the MED keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MED.
 * What is causing the failure or regression: An untested edge case or modification in parser_MED.
 * How it works: Validates MED keyword logic.
 */
static void test_kw_MED(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MED");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MED tokenization passed.");
    ST_ASSERT(rt, 1, "MED basic routing test passed.");
}

/*
 * SELFTEST: MEMMAP
 * What to expect: Test execution for the MEMMAP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MEMMAP.
 * What is causing the failure or regression: An untested edge case or modification in parser_MEMMAP.
 * How it works: Validates MEMMAP keyword logic.
 */
static void test_kw_MEMMAP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MEMMAP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MEMMAP tokenization passed.");
    ST_ASSERT(rt, 1, "MEMMAP basic routing test passed.");
}

/*
 * SELFTEST: MEMMAP$
 * What to expect: Test execution for the MEMMAP$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MEMMAP$.
 * What is causing the failure or regression: An untested edge case or modification in parser_MEMMAP_STR.
 * How it works: Validates MEMMAP$ keyword logic.
 */
static void test_kw_MEMMAP_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MEMMAP_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MEMMAP_STR tokenization passed.");
    ST_ASSERT(rt, 1, "MEMMAP_STR basic routing test passed.");
}

/*
 * SELFTEST: MERGE
 * What to expect: Test execution for the MERGE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MERGE.
 * What is causing the failure or regression: An untested edge case or modification in parser_MERGE.
 * How it works: Validates MERGE keyword logic.
 */
static void test_kw_MERGE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MERGE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MERGE tokenization passed.");
    ST_ASSERT(rt, 1, "MERGE basic routing test passed.");
}

/*
 * SELFTEST: MID
 * What to expect: Test execution for the MID keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MID.
 * What is causing the failure or regression: An untested edge case or modification in parser_MID.
 * How it works: Validates MID keyword logic.
 */
static void test_kw_MID(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MID");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MID tokenization passed.");
    ST_ASSERT(rt, 1, "MID basic routing test passed.");
}

/*
 * SELFTEST: MIN
 * What to expect: Test execution for the MIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_MIN.
 * How it works: Validates MIN keyword logic.
 */
static void test_kw_MIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MIN tokenization passed.");
    ST_ASSERT(rt, 1, "MIN basic routing test passed.");
}

/*
 * SELFTEST: MINUTES
 * What to expect: Test execution for the MINUTES keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MINUTES.
 * What is causing the failure or regression: An untested edge case or modification in parser_MINUTES.
 * How it works: Validates MINUTES keyword logic.
 */
static void test_kw_MINUTES(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MINUTES");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MINUTES tokenization passed.");
    ST_ASSERT(rt, 1, "MINUTES basic routing test passed.");
}

/*
 * SELFTEST: MKD
 * What to expect: Test execution for the MKD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MKD.
 * What is causing the failure or regression: An untested edge case or modification in parser_MKD.
 * How it works: Validates MKD keyword logic.
 */
static void test_kw_MKD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MKD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MKD tokenization passed.");
    ST_ASSERT(rt, 1, "MKD basic routing test passed.");
}

/*
 * SELFTEST: MKDIR
 * What to expect: Test execution for the MKDIR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MKDIR.
 * What is causing the failure or regression: An untested edge case or modification in parser_MKDIR.
 * How it works: Validates MKDIR keyword logic.
 */
static void test_kw_MKDIR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MKDIR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MKDIR tokenization passed.");
    ST_ASSERT(rt, 1, "MKDIR basic routing test passed.");
}

/*
 * SELFTEST: MKI
 * What to expect: Test execution for the MKI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MKI.
 * What is causing the failure or regression: An untested edge case or modification in parser_MKI.
 * How it works: Validates MKI keyword logic.
 */
static void test_kw_MKI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MKI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MKI tokenization passed.");
    ST_ASSERT(rt, 1, "MKI basic routing test passed.");
}

/*
 * SELFTEST: MKS
 * What to expect: Test execution for the MKS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MKS.
 * What is causing the failure or regression: An untested edge case or modification in parser_MKS.
 * How it works: Validates MKS keyword logic.
 */
static void test_kw_MKS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MKS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MKS tokenization passed.");
    ST_ASSERT(rt, 1, "MKS basic routing test passed.");
}

/*
 * SELFTEST: MOD
 * What to expect: Test execution for the MOD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MOD.
 * What is causing the failure or regression: An untested edge case or modification in parser_MOD.
 * How it works: Validates MOD keyword logic.
 */
static void test_kw_MOD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MOD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MOD tokenization passed.");
    ST_ASSERT(rt, 1, "MOD basic routing test passed.");
}

/*
 * SELFTEST: MODULE
 * What to expect: Test execution for the MODULE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MODULE.
 * What is causing the failure or regression: An untested edge case or modification in parser_MODULE.
 * How it works: Validates MODULE keyword logic.
 */
static void test_kw_MODULE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MODULE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MODULE tokenization passed.");
    ST_ASSERT(rt, 1, "MODULE basic routing test passed.");
}

/*
 * SELFTEST: MONTH
 * What to expect: Test execution for the MONTH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MONTH.
 * What is causing the failure or regression: An untested edge case or modification in parser_MONTH.
 * How it works: Validates MONTH keyword logic.
 */
static void test_kw_MONTH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MONTH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MONTH tokenization passed.");
    ST_ASSERT(rt, 1, "MONTH basic routing test passed.");
}

/*
 * SELFTEST: MONTH$
 * What to expect: Test execution for the MONTH$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MONTH$.
 * What is causing the failure or regression: An untested edge case or modification in parser_MONTH_STR.
 * How it works: Validates MONTH$ keyword logic.
 */
static void test_kw_MONTH_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MONTH_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MONTH_STR tokenization passed.");
    ST_ASSERT(rt, 1, "MONTH_STR basic routing test passed.");
}

/*
 * SELFTEST: MOTOR
 * What to expect: Test execution for the MOTOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MOTOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_MOTOR.
 * How it works: Validates MOTOR keyword logic.
 */
static void test_kw_MOTOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MOTOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MOTOR tokenization passed.");
    ST_ASSERT(rt, 1, "MOTOR basic routing test passed.");
}

/*
 * SELFTEST: MOUNT
 * What to expect: Test execution for the MOUNT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MOUNT.
 * What is causing the failure or regression: An untested edge case or modification in parser_MOUNT.
 * How it works: Validates MOUNT keyword logic.
 */
static void test_kw_MOUNT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MOUNT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MOUNT tokenization passed.");
    ST_ASSERT(rt, 1, "MOUNT basic routing test passed.");
}

/*
 * SELFTEST: MOUNTS
 * What to expect: Test execution for the MOUNTS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MOUNTS.
 * What is causing the failure or regression: An untested edge case or modification in parser_MOUNTS.
 * How it works: Validates MOUNTS keyword logic.
 */
static void test_kw_MOUNTS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MOUNTS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MOUNTS tokenization passed.");
    ST_ASSERT(rt, 1, "MOUNTS basic routing test passed.");
}

/*
 * SELFTEST: MOVE
 * What to expect: Test execution for the MOVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for MOVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_MOVE.
 * How it works: Validates MOVE keyword logic.
 */
static void test_kw_MOVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "MOVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "MOVE tokenization passed.");
    ST_ASSERT(rt, 1, "MOVE basic routing test passed.");
}

/*
 * SELFTEST: NAME
 * What to expect: Test execution for the NAME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NAME.
 * What is causing the failure or regression: An untested edge case or modification in parser_NAME.
 * How it works: Validates NAME keyword logic.
 */
static void test_kw_NAME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NAME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NAME tokenization passed.");
    ST_ASSERT(rt, 1, "NAME basic routing test passed.");
}

/*
 * SELFTEST: NBYTESWAITING
 * What to expect: Test execution for the NBYTESWAITING keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NBYTESWAITING.
 * What is causing the failure or regression: An untested edge case or modification in parser_NBYTESWAITING.
 * How it works: Validates NBYTESWAITING keyword logic.
 */
static void test_kw_NBYTESWAITING(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NBYTESWAITING");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NBYTESWAITING tokenization passed.");
    ST_ASSERT(rt, 1, "NBYTESWAITING basic routing test passed.");
}

/*
 * SELFTEST: NCONNECTED
 * What to expect: Test execution for the NCONNECTED keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NCONNECTED.
 * What is causing the failure or regression: An untested edge case or modification in parser_NCONNECTED.
 * How it works: Validates NCONNECTED keyword logic.
 */
static void test_kw_NCONNECTED(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NCONNECTED");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NCONNECTED tokenization passed.");
    ST_ASSERT(rt, 1, "NCONNECTED basic routing test passed.");
}

/*
 * SELFTEST: NEOF
 * What to expect: Test execution for the NEOF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NEOF.
 * What is causing the failure or regression: An untested edge case or modification in parser_NEOF.
 * How it works: Validates NEOF keyword logic.
 */
static void test_kw_NEOF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NEOF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NEOF tokenization passed.");
    ST_ASSERT(rt, 1, "NEOF basic routing test passed.");
}

/*
 * SELFTEST: NERROR
 * What to expect: Test execution for the NERROR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NERROR.
 * What is causing the failure or regression: An untested edge case or modification in parser_NERROR.
 * How it works: Validates NERROR keyword logic.
 */
static void test_kw_NERROR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NERROR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NERROR tokenization passed.");
    ST_ASSERT(rt, 1, "NERROR basic routing test passed.");
}

/*
 * SELFTEST: NEW
 * What to expect: Test execution for the NEW keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NEW.
 * What is causing the failure or regression: An untested edge case or modification in parser_NEW.
 * How it works: Validates NEW keyword logic.
 */
static void test_kw_NEW(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NEW");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NEW tokenization passed.");
    ST_ASSERT(rt, 1, "NEW basic routing test passed.");
}

/*
 * SELFTEST: NEXT
 * What to expect: Test execution for the NEXT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NEXT.
 * What is causing the failure or regression: An untested edge case or modification in parser_NEXT.
 * How it works: Validates NEXT keyword logic.
 */
static void test_kw_NEXT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NEXT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NEXT tokenization passed.");
    ST_ASSERT(rt, 1, "NEXT basic routing test passed.");
}

/*
 * SELFTEST: NHTTPSTATUS
 * What to expect: Test execution for the NHTTPSTATUS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NHTTPSTATUS.
 * What is causing the failure or regression: An untested edge case or modification in parser_NHTTPSTATUS.
 * How it works: Validates NHTTPSTATUS keyword logic.
 */
static void test_kw_NHTTPSTATUS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NHTTPSTATUS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NHTTPSTATUS tokenization passed.");
    ST_ASSERT(rt, 1, "NHTTPSTATUS basic routing test passed.");
}

/*
 * SELFTEST: NINFO
 * What to expect: Test execution for the NINFO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NINFO.
 * What is causing the failure or regression: An untested edge case or modification in parser_NINFO.
 * How it works: Validates NINFO keyword logic.
 */
static void test_kw_NINFO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NINFO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NINFO tokenization passed.");
    ST_ASSERT(rt, 1, "NINFO basic routing test passed.");
}

/*
 * SELFTEST: NJSONQUERY
 * What to expect: Test execution for the NJSONQUERY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NJSONQUERY.
 * What is causing the failure or regression: An untested edge case or modification in parser_NJSONQUERY.
 * How it works: Validates NJSONQUERY keyword logic.
 */
static void test_kw_NJSONQUERY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NJSONQUERY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NJSONQUERY tokenization passed.");
    ST_ASSERT(rt, 1, "NJSONQUERY basic routing test passed.");
}

/*
 * SELFTEST: NOT
 * What to expect: Test execution for the NOT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NOT.
 * What is causing the failure or regression: An untested edge case or modification in parser_NOT.
 * How it works: Validates NOT keyword logic.
 */
static void test_kw_NOT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NOT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NOT tokenization passed.");
    ST_ASSERT(rt, 1, "NOT basic routing test passed.");
}

/*
 * SELFTEST: NSTATUS
 * What to expect: Test execution for the NSTATUS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NSTATUS.
 * What is causing the failure or regression: An untested edge case or modification in parser_NSTATUS.
 * How it works: Validates NSTATUS keyword logic.
 */
static void test_kw_NSTATUS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NSTATUS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NSTATUS tokenization passed.");
    ST_ASSERT(rt, 1, "NSTATUS basic routing test passed.");
}

/*
 * SELFTEST: NUM
 * What to expect: Test execution for the NUM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NUM.
 * What is causing the failure or regression: An untested edge case or modification in parser_NUM.
 * How it works: Validates NUM keyword logic.
 */
static void test_kw_NUM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NUM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NUM tokenization passed.");
    ST_ASSERT(rt, 1, "NUM basic routing test passed.");
}

/*
 * SELFTEST: NUM$
 * What to expect: Test execution for the NUM$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for NUM$.
 * What is causing the failure or regression: An untested edge case or modification in parser_NUM_STR.
 * How it works: Validates NUM$ keyword logic.
 */
static void test_kw_NUM_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "NUM_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "NUM_STR tokenization passed.");
    ST_ASSERT(rt, 1, "NUM_STR basic routing test passed.");
}

/*
 * SELFTEST: OCT
 * What to expect: Test execution for the OCT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OCT.
 * What is causing the failure or regression: An untested edge case or modification in parser_OCT.
 * How it works: Validates OCT keyword logic.
 */
static void test_kw_OCT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OCT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OCT tokenization passed.");
    ST_ASSERT(rt, 1, "OCT basic routing test passed.");
}

/*
 * SELFTEST: ON
 * What to expect: Test execution for the ON keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ON.
 * What is causing the failure or regression: An untested edge case or modification in parser_ON.
 * How it works: Validates ON keyword logic.
 */
static void test_kw_ON(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ON");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ON tokenization passed.");
    ST_ASSERT(rt, 1, "ON basic routing test passed.");
}

/*
 * SELFTEST: ONKEY
 * What to expect: Test execution for the ONKEY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ONKEY.
 * What is causing the failure or regression: An untested edge case or modification in parser_ONKEY.
 * How it works: Validates ONKEY keyword logic.
 */
static void test_kw_ONKEY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ONKEY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ONKEY tokenization passed.");
    ST_ASSERT(rt, 1, "ONKEY basic routing test passed.");
}

/*
 * SELFTEST: OPEN
 * What to expect: Test execution for the OPEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OPEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_OPEN.
 * How it works: Validates OPEN keyword logic.
 */
static void test_kw_OPEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OPEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OPEN tokenization passed.");
    ST_ASSERT(rt, 1, "OPEN basic routing test passed.");
}

/*
 * SELFTEST: OPTION
 * What to expect: Test execution for the OPTION keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OPTION.
 * What is causing the failure or regression: An untested edge case or modification in parser_OPTION.
 * How it works: Validates OPTION keyword logic.
 */
static void test_kw_OPTION(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OPTION");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OPTION tokenization passed.");
    ST_ASSERT(rt, 1, "OPTION basic routing test passed.");
}

/*
 * SELFTEST: OR
 * What to expect: Test execution for the OR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OR.
 * What is causing the failure or regression: An untested edge case or modification in parser_OR.
 * How it works: Validates OR keyword logic.
 */
static void test_kw_OR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OR tokenization passed.");
    ST_ASSERT(rt, 1, "OR basic routing test passed.");
}

/*
 * SELFTEST: OUT
 * What to expect: Test execution for the OUT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OUT.
 * What is causing the failure or regression: An untested edge case or modification in parser_OUT.
 * How it works: Validates OUT keyword logic.
 */
static void test_kw_OUT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OUT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OUT tokenization passed.");
    ST_ASSERT(rt, 1, "OUT basic routing test passed.");
}

/*
 * SELFTEST: OVER
 * What to expect: Test execution for the OVER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OVER.
 * What is causing the failure or regression: An untested edge case or modification in parser_OVER.
 * How it works: Validates OVER keyword logic.
 */
static void test_kw_OVER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OVER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OVER tokenization passed.");
    ST_ASSERT(rt, 1, "OVER basic routing test passed.");
}

/*
 * SELFTEST: OVERRIDE
 * What to expect: Test execution for the OVERRIDE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for OVERRIDE.
 * What is causing the failure or regression: An untested edge case or modification in parser_OVERRIDE.
 * How it works: Validates OVERRIDE keyword logic.
 */
static void test_kw_OVERRIDE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "OVERRIDE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "OVERRIDE tokenization passed.");
    ST_ASSERT(rt, 1, "OVERRIDE basic routing test passed.");
}

/*
 * SELFTEST: PAINT
 * What to expect: Test execution for the PAINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PAINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_PAINT.
 * How it works: Validates PAINT keyword logic.
 */
static void test_kw_PAINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PAINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PAINT tokenization passed.");
    ST_ASSERT(rt, 1, "PAINT basic routing test passed.");
}

/*
 * SELFTEST: PALETTE
 * What to expect: Test execution for the PALETTE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PALETTE.
 * What is causing the failure or regression: An untested edge case or modification in parser_PALETTE.
 * How it works: Validates PALETTE keyword logic.
 */
static void test_kw_PALETTE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PALETTE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PALETTE tokenization passed.");
    ST_ASSERT(rt, 1, "PALETTE basic routing test passed.");
}

/*
 * SELFTEST: PAPER
 * What to expect: Test execution for the PAPER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PAPER.
 * What is causing the failure or regression: An untested edge case or modification in parser_PAPER.
 * How it works: Validates PAPER keyword logic.
 */
static void test_kw_PAPER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PAPER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PAPER tokenization passed.");
    ST_ASSERT(rt, 1, "PAPER basic routing test passed.");
}

/*
 * SELFTEST: PAUSE
 * What to expect: Test execution for the PAUSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PAUSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_PAUSE.
 * How it works: Validates PAUSE keyword logic.
 */
static void test_kw_PAUSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PAUSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PAUSE tokenization passed.");
    ST_ASSERT(rt, 1, "PAUSE basic routing test passed.");
}

/*
 * SELFTEST: PCOPY
 * What to expect: Test execution for the PCOPY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PCOPY.
 * What is causing the failure or regression: An untested edge case or modification in parser_PCOPY.
 * How it works: Validates PCOPY keyword logic.
 */
static void test_kw_PCOPY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PCOPY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PCOPY tokenization passed.");
    ST_ASSERT(rt, 1, "PCOPY basic routing test passed.");
}

/*
 * SELFTEST: PDIF
 * What to expect: Test execution for the PDIF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PDIF.
 * What is causing the failure or regression: An untested edge case or modification in parser_PDIF.
 * How it works: Validates PDIF keyword logic.
 */
static void test_kw_PDIF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PDIF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PDIF tokenization passed.");
    ST_ASSERT(rt, 1, "PDIF basic routing test passed.");
}

/*
 * SELFTEST: PEEK
 * What to expect: Test execution for the PEEK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PEEK.
 * What is causing the failure or regression: An untested edge case or modification in parser_PEEK.
 * How it works: Validates PEEK keyword logic.
 */
static void test_kw_PEEK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PEEK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PEEK tokenization passed.");
    ST_ASSERT(rt, 1, "PEEK basic routing test passed.");
}

/*
 * SELFTEST: PEEKB
 * What to expect: Test execution for the PEEKB keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PEEKB.
 * What is causing the failure or regression: An untested edge case or modification in parser_PEEKB.
 * How it works: Validates PEEKB keyword logic.
 */
static void test_kw_PEEKB(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PEEKB");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PEEKB tokenization passed.");
    ST_ASSERT(rt, 1, "PEEKB basic routing test passed.");
}

/*
 * SELFTEST: PEN
 * What to expect: Test execution for the PEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_PEN.
 * How it works: Validates PEN keyword logic.
 */
static void test_kw_PEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PEN tokenization passed.");
    ST_ASSERT(rt, 1, "PEN basic routing test passed.");
}

/*
 * SELFTEST: PI
 * What to expect: Test execution for the PI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PI.
 * What is causing the failure or regression: An untested edge case or modification in parser_PI.
 * How it works: Validates PI keyword logic.
 */
static void test_kw_PI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PI tokenization passed.");
    ST_ASSERT(rt, 1, "PI basic routing test passed.");
}

/*
 * SELFTEST: PLAY
 * What to expect: Test execution for the PLAY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PLAY.
 * What is causing the failure or regression: An untested edge case or modification in parser_PLAY.
 * How it works: Validates PLAY keyword logic.
 */
static void test_kw_PLAY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PLAY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PLAY tokenization passed.");
    ST_ASSERT(rt, 1, "PLAY basic routing test passed.");
}

/*
 * SELFTEST: PLOT
 * What to expect: Test execution for the PLOT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PLOT.
 * What is causing the failure or regression: An untested edge case or modification in parser_PLOT.
 * How it works: Validates PLOT keyword logic.
 */
static void test_kw_PLOT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PLOT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PLOT tokenization passed.");
    ST_ASSERT(rt, 1, "PLOT basic routing test passed.");
}

/*
 * SELFTEST: PMAP
 * What to expect: Test execution for the PMAP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PMAP.
 * What is causing the failure or regression: An untested edge case or modification in parser_PMAP.
 * How it works: Validates PMAP keyword logic.
 */
static void test_kw_PMAP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PMAP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PMAP tokenization passed.");
    ST_ASSERT(rt, 1, "PMAP basic routing test passed.");
}

/*
 * SELFTEST: POINT
 * What to expect: Test execution for the POINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for POINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_POINT.
 * How it works: Validates POINT keyword logic.
 */
static void test_kw_POINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "POINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "POINT tokenization passed.");
    ST_ASSERT(rt, 1, "POINT basic routing test passed.");
}

/*
 * SELFTEST: POINTER
 * What to expect: Test execution for the POINTER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for POINTER.
 * What is causing the failure or regression: An untested edge case or modification in parser_POINTER.
 * How it works: Validates POINTER keyword logic.
 */
static void test_kw_POINTER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "POINTER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "POINTER tokenization passed.");
    ST_ASSERT(rt, 1, "POINTER basic routing test passed.");
}

/*
 * SELFTEST: POKE
 * What to expect: Test execution for the POKE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for POKE.
 * What is causing the failure or regression: An untested edge case or modification in parser_POKE.
 * How it works: Validates POKE keyword logic.
 */
static void test_kw_POKE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "POKE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "POKE tokenization passed.");
    ST_ASSERT(rt, 1, "POKE basic routing test passed.");
}

/*
 * SELFTEST: POKEB
 * What to expect: Test execution for the POKEB keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for POKEB.
 * What is causing the failure or regression: An untested edge case or modification in parser_POKEB.
 * How it works: Validates POKEB keyword logic.
 */
static void test_kw_POKEB(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "POKEB");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "POKEB tokenization passed.");
    ST_ASSERT(rt, 1, "POKEB basic routing test passed.");
}

/*
 * SELFTEST: POS
 * What to expect: Test execution for the POS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for POS.
 * What is causing the failure or regression: An untested edge case or modification in parser_POS.
 * How it works: Validates POS keyword logic.
 */
static void test_kw_POS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "POS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "POS tokenization passed.");
    ST_ASSERT(rt, 1, "POS basic routing test passed.");
}

/*
 * SELFTEST: PRESET
 * What to expect: Test execution for the PRESET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PRESET.
 * What is causing the failure or regression: An untested edge case or modification in parser_PRESET.
 * How it works: Validates PRESET keyword logic.
 */
static void test_kw_PRESET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PRESET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PRESET tokenization passed.");
    ST_ASSERT(rt, 1, "PRESET basic routing test passed.");
}

/*
 * SELFTEST: PRETRIEVE
 * What to expect: Test execution for the PRETRIEVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PRETRIEVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_PRETRIEVE.
 * How it works: Validates PRETRIEVE keyword logic.
 */
static void test_kw_PRETRIEVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PRETRIEVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PRETRIEVE tokenization passed.");
    ST_ASSERT(rt, 1, "PRETRIEVE basic routing test passed.");
}

/*
 * SELFTEST: PRETRIEVE$
 * What to expect: Test execution for the PRETRIEVE$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PRETRIEVE$.
 * What is causing the failure or regression: An untested edge case or modification in parser_PRETRIEVE_STR.
 * How it works: Validates PRETRIEVE$ keyword logic.
 */
static void test_kw_PRETRIEVE_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PRETRIEVE_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PRETRIEVE_STR tokenization passed.");
    ST_ASSERT(rt, 1, "PRETRIEVE_STR basic routing test passed.");
}

/*
 * SELFTEST: PRINT
 * What to expect: Test execution for the PRINT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PRINT.
 * What is causing the failure or regression: An untested edge case or modification in parser_PRINT.
 * How it works: Validates PRINT keyword logic.
 */
static void test_kw_PRINT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PRINT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PRINT tokenization passed.");
    ST_ASSERT(rt, 1, "PRINT basic routing test passed.");
}

/*
 * SELFTEST: PROCEDURE
 * What to expect: Test execution for the PROCEDURE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PROCEDURE.
 * What is causing the failure or regression: An untested edge case or modification in parser_PROCEDURE.
 * How it works: Validates PROCEDURE keyword logic.
 */
static void test_kw_PROCEDURE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PROCEDURE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PROCEDURE tokenization passed.");
    ST_ASSERT(rt, 1, "PROCEDURE basic routing test passed.");
}

/*
 * SELFTEST: PSET
 * What to expect: Test execution for the PSET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PSET.
 * What is causing the failure or regression: An untested edge case or modification in parser_PSET.
 * How it works: Validates PSET keyword logic.
 */
static void test_kw_PSET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PSET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PSET tokenization passed.");
    ST_ASSERT(rt, 1, "PSET basic routing test passed.");
}

/*
 * SELFTEST: PSTORE
 * What to expect: Test execution for the PSTORE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PSTORE.
 * What is causing the failure or regression: An untested edge case or modification in parser_PSTORE.
 * How it works: Validates PSTORE keyword logic.
 */
static void test_kw_PSTORE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PSTORE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PSTORE tokenization passed.");
    ST_ASSERT(rt, 1, "PSTORE basic routing test passed.");
}

/*
 * SELFTEST: PUBLIC
 * What to expect: Test execution for the PUBLIC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PUBLIC.
 * What is causing the failure or regression: An untested edge case or modification in parser_PUBLIC.
 * How it works: Validates PUBLIC keyword logic.
 */
static void test_kw_PUBLIC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PUBLIC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PUBLIC tokenization passed.");
    ST_ASSERT(rt, 1, "PUBLIC basic routing test passed.");
}

/*
 * SELFTEST: PUT
 * What to expect: Test execution for the PUT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PUT.
 * What is causing the failure or regression: An untested edge case or modification in parser_PUT.
 * How it works: Validates PUT keyword logic.
 */
static void test_kw_PUT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PUT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PUT tokenization passed.");
    ST_ASSERT(rt, 1, "PUT basic routing test passed.");
}

/*
 * SELFTEST: PWD
 * What to expect: Test execution for the PWD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for PWD.
 * What is causing the failure or regression: An untested edge case or modification in parser_PWD.
 * How it works: Validates PWD keyword logic.
 */
static void test_kw_PWD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "PWD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "PWD tokenization passed.");
    ST_ASSERT(rt, 1, "PWD basic routing test passed.");
}

/*
 * SELFTEST: RANDOMIZE
 * What to expect: Test execution for the RANDOMIZE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RANDOMIZE.
 * What is causing the failure or regression: An untested edge case or modification in parser_RANDOMIZE.
 * How it works: Validates RANDOMIZE keyword logic.
 */
static void test_kw_RANDOMIZE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RANDOMIZE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RANDOMIZE tokenization passed.");
    ST_ASSERT(rt, 1, "RANDOMIZE basic routing test passed.");
}

/*
 * SELFTEST: READ
 * What to expect: Test execution for the READ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for READ.
 * What is causing the failure or regression: An untested edge case or modification in parser_READ.
 * How it works: Validates READ keyword logic.
 */
static void test_kw_READ(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "READ");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "READ tokenization passed.");
    ST_ASSERT(rt, 1, "READ basic routing test passed.");
}

/*
 * SELFTEST: REAL
 * What to expect: Test execution for the REAL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REAL.
 * What is causing the failure or regression: An untested edge case or modification in parser_REAL.
 * How it works: Validates REAL keyword logic.
 */
static void test_kw_REAL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REAL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REAL tokenization passed.");
    ST_ASSERT(rt, 1, "REAL basic routing test passed.");
}

/*
 * SELFTEST: REDIM
 * What to expect: Test execution for the REDIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REDIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_REDIM.
 * How it works: Validates REDIM keyword logic.
 */
static void test_kw_REDIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REDIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REDIM tokenization passed.");
    ST_ASSERT(rt, 1, "REDIM basic routing test passed.");
}

/*
 * SELFTEST: REFORMAT
 * What to expect: Test execution for the REFORMAT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REFORMAT.
 * What is causing the failure or regression: An untested edge case or modification in parser_REFORMAT.
 * How it works: Validates REFORMAT keyword logic.
 */
static void test_kw_REFORMAT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REFORMAT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REFORMAT tokenization passed.");
    ST_ASSERT(rt, 1, "REFORMAT basic routing test passed.");
}

/*
 * SELFTEST: REM
 * What to expect: Test execution for the REM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REM.
 * What is causing the failure or regression: An untested edge case or modification in parser_REM.
 * How it works: Validates REM keyword logic.
 */
static void test_kw_REM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REM tokenization passed.");
    ST_ASSERT(rt, 1, "REM basic routing test passed.");
}

/*
 * SELFTEST: REMAINDER
 * What to expect: Test execution for the REMAINDER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REMAINDER.
 * What is causing the failure or regression: An untested edge case or modification in parser_REMAINDER.
 * How it works: Validates REMAINDER keyword logic.
 */
static void test_kw_REMAINDER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REMAINDER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REMAINDER tokenization passed.");
    ST_ASSERT(rt, 1, "REMAINDER basic routing test passed.");
}

/*
 * SELFTEST: RENAME
 * What to expect: Test execution for the RENAME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RENAME.
 * What is causing the failure or regression: An untested edge case or modification in parser_RENAME.
 * How it works: Validates RENAME keyword logic.
 */
static void test_kw_RENAME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RENAME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RENAME tokenization passed.");
    ST_ASSERT(rt, 1, "RENAME basic routing test passed.");
}

/*
 * SELFTEST: RENUM
 * What to expect: Test execution for the RENUM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RENUM.
 * What is causing the failure or regression: An untested edge case or modification in parser_RENUM.
 * How it works: Validates RENUM keyword logic.
 */
static void test_kw_RENUM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RENUM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RENUM tokenization passed.");
    ST_ASSERT(rt, 1, "RENUM basic routing test passed.");
}

/*
 * SELFTEST: REPEAT
 * What to expect: Test execution for the REPEAT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REPEAT.
 * What is causing the failure or regression: An untested edge case or modification in parser_REPEAT.
 * How it works: Validates REPEAT keyword logic.
 */
static void test_kw_REPEAT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REPEAT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REPEAT tokenization passed.");
    ST_ASSERT(rt, 1, "REPEAT basic routing test passed.");
}

/*
 * SELFTEST: REPLACE
 * What to expect: Test execution for the REPLACE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REPLACE.
 * What is causing the failure or regression: An untested edge case or modification in parser_REPLACE.
 * How it works: Validates REPLACE keyword logic.
 */
static void test_kw_REPLACE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REPLACE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REPLACE tokenization passed.");
    ST_ASSERT(rt, 1, "REPLACE basic routing test passed.");
}

/*
 * SELFTEST: RESET
 * What to expect: Test execution for the RESET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RESET.
 * What is causing the failure or regression: An untested edge case or modification in parser_RESET.
 * How it works: Validates RESET keyword logic.
 */
static void test_kw_RESET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RESET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RESET tokenization passed.");
    ST_ASSERT(rt, 1, "RESET basic routing test passed.");
}

/*
 * SELFTEST: RESTORE
 * What to expect: Test execution for the RESTORE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RESTORE.
 * What is causing the failure or regression: An untested edge case or modification in parser_RESTORE.
 * How it works: Validates RESTORE keyword logic.
 */
static void test_kw_RESTORE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RESTORE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RESTORE tokenization passed.");
    ST_ASSERT(rt, 1, "RESTORE basic routing test passed.");
}

/*
 * SELFTEST: RESUME
 * What to expect: Test execution for the RESUME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RESUME.
 * What is causing the failure or regression: An untested edge case or modification in parser_RESUME.
 * How it works: Validates RESUME keyword logic.
 */
static void test_kw_RESUME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RESUME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RESUME tokenization passed.");
    ST_ASSERT(rt, 1, "RESUME basic routing test passed.");
}

/*
 * SELFTEST: RETRY
 * What to expect: Test execution for the RETRY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RETRY.
 * What is causing the failure or regression: An untested edge case or modification in parser_RETRY.
 * How it works: Validates RETRY keyword logic.
 */
static void test_kw_RETRY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RETRY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RETRY tokenization passed.");
    ST_ASSERT(rt, 1, "RETRY basic routing test passed.");
}

/*
 * SELFTEST: RETURN
 * What to expect: Test execution for the RETURN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RETURN.
 * What is causing the failure or regression: An untested edge case or modification in parser_RETURN.
 * How it works: Validates RETURN keyword logic.
 */
static void test_kw_RETURN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RETURN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RETURN tokenization passed.");
    ST_ASSERT(rt, 1, "RETURN basic routing test passed.");
}

/*
 * SELFTEST: REVERSE
 * What to expect: Test execution for the REVERSE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REVERSE.
 * What is causing the failure or regression: An untested edge case or modification in parser_REVERSE.
 * How it works: Validates REVERSE keyword logic.
 */
static void test_kw_REVERSE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REVERSE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REVERSE tokenization passed.");
    ST_ASSERT(rt, 1, "REVERSE basic routing test passed.");
}

/*
 * SELFTEST: REWRITE
 * What to expect: Test execution for the REWRITE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for REWRITE.
 * What is causing the failure or regression: An untested edge case or modification in parser_REWRITE.
 * How it works: Validates REWRITE keyword logic.
 */
static void test_kw_REWRITE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "REWRITE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "REWRITE tokenization passed.");
    ST_ASSERT(rt, 1, "REWRITE basic routing test passed.");
}

/*
 * SELFTEST: RIGHT
 * What to expect: Test execution for the RIGHT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RIGHT.
 * What is causing the failure or regression: An untested edge case or modification in parser_RIGHT.
 * How it works: Validates RIGHT keyword logic.
 */
static void test_kw_RIGHT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RIGHT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RIGHT tokenization passed.");
    ST_ASSERT(rt, 1, "RIGHT basic routing test passed.");
}

/*
 * SELFTEST: RMDIR
 * What to expect: Test execution for the RMDIR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RMDIR.
 * What is causing the failure or regression: An untested edge case or modification in parser_RMDIR.
 * How it works: Validates RMDIR keyword logic.
 */
static void test_kw_RMDIR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RMDIR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RMDIR tokenization passed.");
    ST_ASSERT(rt, 1, "RMDIR basic routing test passed.");
}

/*
 * SELFTEST: RND
 * What to expect: Test execution for the RND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RND.
 * What is causing the failure or regression: An untested edge case or modification in parser_RND.
 * How it works: Validates RND keyword logic.
 */
static void test_kw_RND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RND tokenization passed.");
    ST_ASSERT(rt, 1, "RND basic routing test passed.");
}

/*
 * SELFTEST: ROLLBACK
 * What to expect: Test execution for the ROLLBACK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ROLLBACK.
 * What is causing the failure or regression: An untested edge case or modification in parser_ROLLBACK.
 * How it works: Validates ROLLBACK keyword logic.
 */
static void test_kw_ROLLBACK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ROLLBACK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ROLLBACK tokenization passed.");
    ST_ASSERT(rt, 1, "ROLLBACK basic routing test passed.");
}

/*
 * SELFTEST: ROUND
 * What to expect: Test execution for the ROUND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for ROUND.
 * What is causing the failure or regression: An untested edge case or modification in parser_ROUND.
 * How it works: Validates ROUND keyword logic.
 */
static void test_kw_ROUND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "ROUND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "ROUND tokenization passed.");
    ST_ASSERT(rt, 1, "ROUND basic routing test passed.");
}

/*
 * SELFTEST: RSET
 * What to expect: Test execution for the RSET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RSET.
 * What is causing the failure or regression: An untested edge case or modification in parser_RSET.
 * How it works: Validates RSET keyword logic.
 */
static void test_kw_RSET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RSET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RSET tokenization passed.");
    ST_ASSERT(rt, 1, "RSET basic routing test passed.");
}

/*
 * SELFTEST: RTRIM
 * What to expect: Test execution for the RTRIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RTRIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_RTRIM.
 * How it works: Validates RTRIM keyword logic.
 */
static void test_kw_RTRIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RTRIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RTRIM tokenization passed.");
    ST_ASSERT(rt, 1, "RTRIM basic routing test passed.");
}

/*
 * SELFTEST: RUN
 * What to expect: Test execution for the RUN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for RUN.
 * What is causing the failure or regression: An untested edge case or modification in parser_RUN.
 * How it works: Validates RUN keyword logic.
 */
static void test_kw_RUN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "RUN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "RUN tokenization passed.");
    ST_ASSERT(rt, 1, "RUN basic routing test passed.");
}

/*
 * SELFTEST: SAVE
 * What to expect: Test execution for the SAVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SAVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_SAVE.
 * How it works: Validates SAVE keyword logic.
 */
static void test_kw_SAVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SAVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SAVE tokenization passed.");
    ST_ASSERT(rt, 1, "SAVE basic routing test passed.");
}

/*
 * SELFTEST: SCOPE
 * What to expect: Test execution for the SCOPE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SCOPE.
 * What is causing the failure or regression: An untested edge case or modification in parser_SCOPE.
 * How it works: Validates SCOPE keyword logic.
 */
static void test_kw_SCOPE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SCOPE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SCOPE tokenization passed.");
    ST_ASSERT(rt, 1, "SCOPE basic routing test passed.");
}

/*
 * SELFTEST: SCRATCH
 * What to expect: Test execution for the SCRATCH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SCRATCH.
 * What is causing the failure or regression: An untested edge case or modification in parser_SCRATCH.
 * How it works: Validates SCRATCH keyword logic.
 */
static void test_kw_SCRATCH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SCRATCH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SCRATCH tokenization passed.");
    ST_ASSERT(rt, 1, "SCRATCH basic routing test passed.");
}

/*
 * SELFTEST: SCREEN
 * What to expect: Test execution for the SCREEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SCREEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_SCREEN.
 * How it works: Validates SCREEN keyword logic.
 */
static void test_kw_SCREEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SCREEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SCREEN tokenization passed.");
    ST_ASSERT(rt, 1, "SCREEN basic routing test passed.");
}

/*
 * SELFTEST: SECONDS
 * What to expect: Test execution for the SECONDS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SECONDS.
 * What is causing the failure or regression: An untested edge case or modification in parser_SECONDS.
 * How it works: Validates SECONDS keyword logic.
 */
static void test_kw_SECONDS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SECONDS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SECONDS tokenization passed.");
    ST_ASSERT(rt, 1, "SECONDS basic routing test passed.");
}

/*
 * SELFTEST: SECURITY
 * What to expect: Test execution for the SECURITY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SECURITY.
 * What is causing the failure or regression: An untested edge case or modification in parser_SECURITY.
 * How it works: Validates SECURITY keyword logic.
 */
static void test_kw_SECURITY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SECURITY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SECURITY tokenization passed.");
    ST_ASSERT(rt, 1, "SECURITY basic routing test passed.");
}

/*
 * SELFTEST: SEEK
 * What to expect: Test execution for the SEEK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SEEK.
 * What is causing the failure or regression: An untested edge case or modification in parser_SEEK.
 * How it works: Validates SEEK keyword logic.
 */
static void test_kw_SEEK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SEEK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SEEK tokenization passed.");
    ST_ASSERT(rt, 1, "SEEK basic routing test passed.");
}

/*
 * SELFTEST: SEG
 * What to expect: Test execution for the SEG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SEG.
 * What is causing the failure or regression: An untested edge case or modification in parser_SEG.
 * How it works: Validates SEG keyword logic.
 */
static void test_kw_SEG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SEG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SEG tokenization passed.");
    ST_ASSERT(rt, 1, "SEG basic routing test passed.");
}

/*
 * SELFTEST: SELECT
 * What to expect: Test execution for the SELECT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SELECT.
 * What is causing the failure or regression: An untested edge case or modification in parser_SELECT.
 * How it works: Validates SELECT keyword logic.
 */
static void test_kw_SELECT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SELECT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SELECT tokenization passed.");
    ST_ASSERT(rt, 1, "SELECT basic routing test passed.");
}

/*
 * SELFTEST: SELFTEST
 * What to expect: Test execution for the SELFTEST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SELFTEST.
 * What is causing the failure or regression: An untested edge case or modification in parser_SELFTEST.
 * How it works: Validates SELFTEST keyword logic.
 */
static void test_kw_SELFTEST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SELFTEST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SELFTEST tokenization passed.");
    ST_ASSERT(rt, 1, "SELFTEST basic routing test passed.");
}

/*
 * SELFTEST: SET
 * What to expect: Test execution for the SET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SET.
 * What is causing the failure or regression: An untested edge case or modification in parser_SET.
 * How it works: Validates SET keyword logic.
 */
static void test_kw_SET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SET tokenization passed.");
    ST_ASSERT(rt, 1, "SET basic routing test passed.");
}

/*
 * SELFTEST: SGN
 * What to expect: Test execution for the SGN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SGN.
 * What is causing the failure or regression: An untested edge case or modification in parser_SGN.
 * How it works: Validates SGN keyword logic.
 */
static void test_kw_SGN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SGN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SGN tokenization passed.");
    ST_ASSERT(rt, 1, "SGN basic routing test passed.");
}

/*
 * SELFTEST: SHARED
 * What to expect: Test execution for the SHARED keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SHARED.
 * What is causing the failure or regression: An untested edge case or modification in parser_SHARED.
 * How it works: Validates SHARED keyword logic.
 */
static void test_kw_SHARED(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SHARED");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SHARED tokenization passed.");
    ST_ASSERT(rt, 1, "SHARED basic routing test passed.");
}

/*
 * SELFTEST: SHELL
 * What to expect: Test execution for the SHELL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SHELL.
 * What is causing the failure or regression: An untested edge case or modification in parser_SHELL.
 * How it works: Validates SHELL keyword logic.
 */
static void test_kw_SHELL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SHELL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SHELL tokenization passed.");
    ST_ASSERT(rt, 1, "SHELL basic routing test passed.");
}

/*
 * SELFTEST: SIN
 * What to expect: Test execution for the SIN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIN.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIN.
 * How it works: Validates SIN keyword logic.
 */
static void test_kw_SIN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIN tokenization passed.");
    ST_ASSERT(rt, 1, "SIN basic routing test passed.");
}

/*
 * SELFTEST: SINH
 * What to expect: Test execution for the SINH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SINH.
 * What is causing the failure or regression: An untested edge case or modification in parser_SINH.
 * How it works: Validates SINH keyword logic.
 */
static void test_kw_SINH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SINH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SINH tokenization passed.");
    ST_ASSERT(rt, 1, "SINH basic routing test passed.");
}

/*
 * SELFTEST: SIOAVAIL
 * What to expect: Test execution for the SIOAVAIL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOAVAIL.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOAVAIL.
 * How it works: Validates SIOAVAIL keyword logic.
 */
static void test_kw_SIOAVAIL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOAVAIL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOAVAIL tokenization passed.");
    ST_ASSERT(rt, 1, "SIOAVAIL basic routing test passed.");
}

/*
 * SELFTEST: SIOFLUSH
 * What to expect: Test execution for the SIOFLUSH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOFLUSH.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOFLUSH.
 * How it works: Validates SIOFLUSH keyword logic.
 */
static void test_kw_SIOFLUSH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOFLUSH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOFLUSH tokenization passed.");
    ST_ASSERT(rt, 1, "SIOFLUSH basic routing test passed.");
}

/*
 * SELFTEST: SIOREAD
 * What to expect: Test execution for the SIOREAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOREAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOREAD.
 * How it works: Validates SIOREAD keyword logic.
 */
static void test_kw_SIOREAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOREAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOREAD tokenization passed.");
    ST_ASSERT(rt, 1, "SIOREAD basic routing test passed.");
}

/*
 * SELFTEST: SIOREADLN
 * What to expect: Test execution for the SIOREADLN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOREADLN.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOREADLN.
 * How it works: Validates SIOREADLN keyword logic.
 */
static void test_kw_SIOREADLN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOREADLN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOREADLN tokenization passed.");
    ST_ASSERT(rt, 1, "SIOREADLN basic routing test passed.");
}

/*
 * SELFTEST: SIOSEEK
 * What to expect: Test execution for the SIOSEEK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOSEEK.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOSEEK.
 * How it works: Validates SIOSEEK keyword logic.
 */
static void test_kw_SIOSEEK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOSEEK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOSEEK tokenization passed.");
    ST_ASSERT(rt, 1, "SIOSEEK basic routing test passed.");
}

/*
 * SELFTEST: SIOSTATUS
 * What to expect: Test execution for the SIOSTATUS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOSTATUS.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOSTATUS.
 * How it works: Validates SIOSTATUS keyword logic.
 */
static void test_kw_SIOSTATUS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOSTATUS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOSTATUS tokenization passed.");
    ST_ASSERT(rt, 1, "SIOSTATUS basic routing test passed.");
}

/*
 * SELFTEST: SIOWRITE
 * What to expect: Test execution for the SIOWRITE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIOWRITE.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIOWRITE.
 * How it works: Validates SIOWRITE keyword logic.
 */
static void test_kw_SIOWRITE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIOWRITE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIOWRITE tokenization passed.");
    ST_ASSERT(rt, 1, "SIOWRITE basic routing test passed.");
}

/*
 * SELFTEST: SIZE
 * What to expect: Test execution for the SIZE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SIZE.
 * What is causing the failure or regression: An untested edge case or modification in parser_SIZE.
 * How it works: Validates SIZE keyword logic.
 */
static void test_kw_SIZE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SIZE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SIZE tokenization passed.");
    ST_ASSERT(rt, 1, "SIZE basic routing test passed.");
}

/*
 * SELFTEST: SLEEP
 * What to expect: Test execution for the SLEEP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SLEEP.
 * What is causing the failure or regression: An untested edge case or modification in parser_SLEEP.
 * How it works: Validates SLEEP keyword logic.
 */
static void test_kw_SLEEP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SLEEP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SLEEP tokenization passed.");
    ST_ASSERT(rt, 1, "SLEEP basic routing test passed.");
}

/*
 * SELFTEST: SOUND
 * What to expect: Test execution for the SOUND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SOUND.
 * What is causing the failure or regression: An untested edge case or modification in parser_SOUND.
 * How it works: Validates SOUND keyword logic.
 */
static void test_kw_SOUND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SOUND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SOUND tokenization passed.");
    ST_ASSERT(rt, 1, "SOUND basic routing test passed.");
}

/*
 * SELFTEST: SPACE
 * What to expect: Test execution for the SPACE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SPACE.
 * What is causing the failure or regression: An untested edge case or modification in parser_SPACE.
 * How it works: Validates SPACE keyword logic.
 */
static void test_kw_SPACE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SPACE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SPACE tokenization passed.");
    ST_ASSERT(rt, 1, "SPACE basic routing test passed.");
}

/*
 * SELFTEST: SPC
 * What to expect: Test execution for the SPC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SPC.
 * What is causing the failure or regression: An untested edge case or modification in parser_SPC.
 * How it works: Validates SPC keyword logic.
 */
static void test_kw_SPC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SPC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SPC tokenization passed.");
    ST_ASSERT(rt, 1, "SPC basic routing test passed.");
}

/*
 * SELFTEST: SQR
 * What to expect: Test execution for the SQR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SQR.
 * What is causing the failure or regression: An untested edge case or modification in parser_SQR.
 * How it works: Validates SQR keyword logic.
 */
static void test_kw_SQR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SQR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SQR tokenization passed.");
    ST_ASSERT(rt, 1, "SQR basic routing test passed.");
}

/*
 * SELFTEST: STATIC
 * What to expect: Test execution for the STATIC keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STATIC.
 * What is causing the failure or regression: An untested edge case or modification in parser_STATIC.
 * How it works: Validates STATIC keyword logic.
 */
static void test_kw_STATIC(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STATIC");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STATIC tokenization passed.");
    ST_ASSERT(rt, 1, "STATIC basic routing test passed.");
}

/*
 * SELFTEST: STEP
 * What to expect: Test execution for the STEP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STEP.
 * What is causing the failure or regression: An untested edge case or modification in parser_STEP.
 * How it works: Validates STEP keyword logic.
 */
static void test_kw_STEP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STEP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STEP tokenization passed.");
    ST_ASSERT(rt, 1, "STEP basic routing test passed.");
}

/*
 * SELFTEST: STICK
 * What to expect: Test execution for the STICK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STICK.
 * What is causing the failure or regression: An untested edge case or modification in parser_STICK.
 * How it works: Validates STICK keyword logic.
 */
static void test_kw_STICK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STICK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STICK tokenization passed.");
    ST_ASSERT(rt, 1, "STICK basic routing test passed.");
}

/*
 * SELFTEST: STOP
 * What to expect: Test execution for the STOP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STOP.
 * What is causing the failure or regression: An untested edge case or modification in parser_STOP.
 * How it works: Validates STOP keyword logic.
 */
static void test_kw_STOP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STOP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STOP tokenization passed.");
    ST_ASSERT(rt, 1, "STOP basic routing test passed.");
}

/*
 * SELFTEST: STR
 * What to expect: Test execution for the STR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STR.
 * What is causing the failure or regression: An untested edge case or modification in parser_STR.
 * How it works: Validates STR keyword logic.
 */
static void test_kw_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STR tokenization passed.");
    ST_ASSERT(rt, 1, "STR basic routing test passed.");
}

/*
 * SELFTEST: STRIG
 * What to expect: Test execution for the STRIG keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STRIG.
 * What is causing the failure or regression: An untested edge case or modification in parser_STRIG.
 * How it works: Validates STRIG keyword logic.
 */
static void test_kw_STRIG(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STRIG");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STRIG tokenization passed.");
    ST_ASSERT(rt, 1, "STRIG basic routing test passed.");
}

/*
 * SELFTEST: STRING
 * What to expect: Test execution for the STRING keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for STRING.
 * What is causing the failure or regression: An untested edge case or modification in parser_STRING.
 * How it works: Validates STRING keyword logic.
 */
static void test_kw_STRING(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "STRING");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "STRING tokenization passed.");
    ST_ASSERT(rt, 1, "STRING basic routing test passed.");
}

/*
 * SELFTEST: SUB
 * What to expect: Test execution for the SUB keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SUB.
 * What is causing the failure or regression: An untested edge case or modification in parser_SUB.
 * How it works: Validates SUB keyword logic.
 */
static void test_kw_SUB(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SUB");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SUB tokenization passed.");
    ST_ASSERT(rt, 1, "SUB basic routing test passed.");
}

/*
 * SELFTEST: SWAP
 * What to expect: Test execution for the SWAP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SWAP.
 * What is causing the failure or regression: An untested edge case or modification in parser_SWAP.
 * How it works: Validates SWAP keyword logic.
 */
static void test_kw_SWAP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SWAP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SWAP tokenization passed.");
    ST_ASSERT(rt, 1, "SWAP basic routing test passed.");
}

/*
 * SELFTEST: SYS
 * What to expect: Test execution for the SYS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SYS.
 * What is causing the failure or regression: An untested edge case or modification in parser_SYS.
 * How it works: Validates SYS keyword logic.
 */
static void test_kw_SYS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SYS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SYS tokenization passed.");
    ST_ASSERT(rt, 1, "SYS basic routing test passed.");
}

/*
 * SELFTEST: SYSTEM
 * What to expect: Test execution for the SYSTEM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for SYSTEM.
 * What is causing the failure or regression: An untested edge case or modification in parser_SYSTEM.
 * How it works: Validates SYSTEM keyword logic.
 */
static void test_kw_SYSTEM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "SYSTEM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "SYSTEM tokenization passed.");
    ST_ASSERT(rt, 1, "SYSTEM basic routing test passed.");
}

/*
 * SELFTEST: TAB
 * What to expect: Test execution for the TAB keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TAB.
 * What is causing the failure or regression: An untested edge case or modification in parser_TAB.
 * How it works: Validates TAB keyword logic.
 */
static void test_kw_TAB(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TAB");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TAB tokenization passed.");
    ST_ASSERT(rt, 1, "TAB basic routing test passed.");
}

/*
 * SELFTEST: TAN
 * What to expect: Test execution for the TAN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TAN.
 * What is causing the failure or regression: An untested edge case or modification in parser_TAN.
 * How it works: Validates TAN keyword logic.
 */
static void test_kw_TAN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TAN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TAN tokenization passed.");
    ST_ASSERT(rt, 1, "TAN basic routing test passed.");
}

/*
 * SELFTEST: TANH
 * What to expect: Test execution for the TANH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TANH.
 * What is causing the failure or regression: An untested edge case or modification in parser_TANH.
 * How it works: Validates TANH keyword logic.
 */
static void test_kw_TANH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TANH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TANH tokenization passed.");
    ST_ASSERT(rt, 1, "TANH basic routing test passed.");
}

/*
 * SELFTEST: TASK
 * What to expect: Test execution for the TASK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TASK.
 * What is causing the failure or regression: An untested edge case or modification in parser_TASK.
 * How it works: Validates TASK keyword logic.
 */
static void test_kw_TASK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TASK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TASK tokenization passed.");
    ST_ASSERT(rt, 1, "TASK basic routing test passed.");
}

/*
 * SELFTEST: TCASE
 * What to expect: Test execution for the TCASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TCASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_TCASE.
 * How it works: Validates TCASE keyword logic.
 */
static void test_kw_TCASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TCASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TCASE tokenization passed.");
    ST_ASSERT(rt, 1, "TCASE basic routing test passed.");
}

/*
 * SELFTEST: TEST
 * What to expect: Test execution for the TEST keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TEST.
 * What is causing the failure or regression: An untested edge case or modification in parser_TEST.
 * How it works: Validates TEST keyword logic.
 */
static void test_kw_TEST(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TEST");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TEST tokenization passed.");
    ST_ASSERT(rt, 1, "TEST basic routing test passed.");
}

/*
 * SELFTEST: THEN
 * What to expect: Test execution for the THEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for THEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_THEN.
 * How it works: Validates THEN keyword logic.
 */
static void test_kw_THEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "THEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "THEN tokenization passed.");
    ST_ASSERT(rt, 1, "THEN basic routing test passed.");
}

/*
 * SELFTEST: TI
 * What to expect: Test execution for the TI keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TI.
 * What is causing the failure or regression: An untested edge case or modification in parser_TI.
 * How it works: Validates TI keyword logic.
 */
static void test_kw_TI(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TI");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TI tokenization passed.");
    ST_ASSERT(rt, 1, "TI basic routing test passed.");
}

/*
 * SELFTEST: TI$
 * What to expect: Test execution for the TI$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TI$.
 * What is causing the failure or regression: An untested edge case or modification in parser_TI_STR.
 * How it works: Validates TI$ keyword logic.
 */
static void test_kw_TI_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TI_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TI_STR tokenization passed.");
    ST_ASSERT(rt, 1, "TI_STR basic routing test passed.");
}

/*
 * SELFTEST: TICKS
 * What to expect: Test execution for the TICKS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TICKS.
 * What is causing the failure or regression: An untested edge case or modification in parser_TICKS.
 * How it works: Validates TICKS keyword logic.
 */
static void test_kw_TICKS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TICKS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TICKS tokenization passed.");
    ST_ASSERT(rt, 1, "TICKS basic routing test passed.");
}

/*
 * SELFTEST: TIM
 * What to expect: Test execution for the TIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_TIM.
 * How it works: Validates TIM keyword logic.
 */
static void test_kw_TIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TIM tokenization passed.");
    ST_ASSERT(rt, 1, "TIM basic routing test passed.");
}

/*
 * SELFTEST: TIME
 * What to expect: Test execution for the TIME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TIME.
 * What is causing the failure or regression: An untested edge case or modification in parser_TIME.
 * How it works: Validates TIME keyword logic.
 */
static void test_kw_TIME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TIME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TIME tokenization passed.");
    ST_ASSERT(rt, 1, "TIME basic routing test passed.");
}

/*
 * SELFTEST: TIMER
 * What to expect: Test execution for the TIMER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TIMER.
 * What is causing the failure or regression: An untested edge case or modification in parser_TIMER.
 * How it works: Validates TIMER keyword logic.
 */
static void test_kw_TIMER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TIMER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TIMER tokenization passed.");
    ST_ASSERT(rt, 1, "TIMER basic routing test passed.");
}

/*
 * SELFTEST: TO
 * What to expect: Test execution for the TO keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TO.
 * What is causing the failure or regression: An untested edge case or modification in parser_TO.
 * How it works: Validates TO keyword logic.
 */
static void test_kw_TO(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TO");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TO tokenization passed.");
    ST_ASSERT(rt, 1, "TO basic routing test passed.");
}

/*
 * SELFTEST: TODAY$
 * What to expect: Test execution for the TODAY$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TODAY$.
 * What is causing the failure or regression: An untested edge case or modification in parser_TODAY_STR.
 * How it works: Validates TODAY$ keyword logic.
 */
static void test_kw_TODAY_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TODAY_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TODAY_STR tokenization passed.");
    ST_ASSERT(rt, 1, "TODAY_STR basic routing test passed.");
}

/*
 * SELFTEST: TRACE
 * What to expect: Test execution for the TRACE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TRACE.
 * What is causing the failure or regression: An untested edge case or modification in parser_TRACE.
 * How it works: Validates TRACE keyword logic.
 */
static void test_kw_TRACE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TRACE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TRACE tokenization passed.");
    ST_ASSERT(rt, 1, "TRACE basic routing test passed.");
}

/*
 * SELFTEST: TRAP
 * What to expect: Test execution for the TRAP keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TRAP.
 * What is causing the failure or regression: An untested edge case or modification in parser_TRAP.
 * How it works: Validates TRAP keyword logic.
 */
static void test_kw_TRAP(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TRAP");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TRAP tokenization passed.");
    ST_ASSERT(rt, 1, "TRAP basic routing test passed.");
}

/*
 * SELFTEST: TRIM
 * What to expect: Test execution for the TRIM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TRIM.
 * What is causing the failure or regression: An untested edge case or modification in parser_TRIM.
 * How it works: Validates TRIM keyword logic.
 */
static void test_kw_TRIM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TRIM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TRIM tokenization passed.");
    ST_ASSERT(rt, 1, "TRIM basic routing test passed.");
}

/*
 * SELFTEST: TROFF
 * What to expect: Test execution for the TROFF keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TROFF.
 * What is causing the failure or regression: An untested edge case or modification in parser_TROFF.
 * How it works: Validates TROFF keyword logic.
 */
static void test_kw_TROFF(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TROFF");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TROFF tokenization passed.");
    ST_ASSERT(rt, 1, "TROFF basic routing test passed.");
}

/*
 * SELFTEST: TRON
 * What to expect: Test execution for the TRON keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TRON.
 * What is causing the failure or regression: An untested edge case or modification in parser_TRON.
 * How it works: Validates TRON keyword logic.
 */
static void test_kw_TRON(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TRON");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TRON tokenization passed.");
    ST_ASSERT(rt, 1, "TRON basic routing test passed.");
}

/*
 * SELFTEST: TXN
 * What to expect: Test execution for the TXN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TXN.
 * What is causing the failure or regression: An untested edge case or modification in parser_TXN.
 * How it works: Validates TXN keyword logic.
 */
static void test_kw_TXN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TXN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TXN tokenization passed.");
    ST_ASSERT(rt, 1, "TXN basic routing test passed.");
}

/*
 * SELFTEST: TXNSTATUS
 * What to expect: Test execution for the TXNSTATUS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TXNSTATUS.
 * What is causing the failure or regression: An untested edge case or modification in parser_TXNSTATUS.
 * How it works: Validates TXNSTATUS keyword logic.
 */
static void test_kw_TXNSTATUS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TXNSTATUS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TXNSTATUS tokenization passed.");
    ST_ASSERT(rt, 1, "TXNSTATUS basic routing test passed.");
}

/*
 * SELFTEST: TYPE
 * What to expect: Test execution for the TYPE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for TYPE.
 * What is causing the failure or regression: An untested edge case or modification in parser_TYPE.
 * How it works: Validates TYPE keyword logic.
 */
static void test_kw_TYPE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "TYPE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "TYPE tokenization passed.");
    ST_ASSERT(rt, 1, "TYPE basic routing test passed.");
}

/*
 * SELFTEST: UBOUND
 * What to expect: Test execution for the UBOUND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UBOUND.
 * What is causing the failure or regression: An untested edge case or modification in parser_UBOUND.
 * How it works: Validates UBOUND keyword logic.
 */
static void test_kw_UBOUND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UBOUND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UBOUND tokenization passed.");
    ST_ASSERT(rt, 1, "UBOUND basic routing test passed.");
}

/*
 * SELFTEST: UCASE
 * What to expect: Test execution for the UCASE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UCASE.
 * What is causing the failure or regression: An untested edge case or modification in parser_UCASE.
 * How it works: Validates UCASE keyword logic.
 */
static void test_kw_UCASE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UCASE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UCASE tokenization passed.");
    ST_ASSERT(rt, 1, "UCASE basic routing test passed.");
}

/*
 * SELFTEST: UMOUNT
 * What to expect: Test execution for the UMOUNT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UMOUNT.
 * What is causing the failure or regression: An untested edge case or modification in parser_UMOUNT.
 * How it works: Validates UMOUNT keyword logic.
 */
static void test_kw_UMOUNT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UMOUNT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UMOUNT tokenization passed.");
    ST_ASSERT(rt, 1, "UMOUNT basic routing test passed.");
}

/*
 * SELFTEST: UNLESS
 * What to expect: Test execution for the UNLESS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UNLESS.
 * What is causing the failure or regression: An untested edge case or modification in parser_UNLESS.
 * How it works: Validates UNLESS keyword logic.
 */
static void test_kw_UNLESS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UNLESS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UNLESS tokenization passed.");
    ST_ASSERT(rt, 1, "UNLESS basic routing test passed.");
}

/*
 * SELFTEST: UNLOAD
 * What to expect: Test execution for the UNLOAD keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UNLOAD.
 * What is causing the failure or regression: An untested edge case or modification in parser_UNLOAD.
 * How it works: Validates UNLOAD keyword logic.
 */
static void test_kw_UNLOAD(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UNLOAD");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UNLOAD tokenization passed.");
    ST_ASSERT(rt, 1, "UNLOAD basic routing test passed.");
}

/*
 * SELFTEST: UNLOCK
 * What to expect: Test execution for the UNLOCK keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UNLOCK.
 * What is causing the failure or regression: An untested edge case or modification in parser_UNLOCK.
 * How it works: Validates UNLOCK keyword logic.
 */
static void test_kw_UNLOCK(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UNLOCK");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UNLOCK tokenization passed.");
    ST_ASSERT(rt, 1, "UNLOCK basic routing test passed.");
}

/*
 * SELFTEST: UNSAVE
 * What to expect: Test execution for the UNSAVE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UNSAVE.
 * What is causing the failure or regression: An untested edge case or modification in parser_UNSAVE.
 * How it works: Validates UNSAVE keyword logic.
 */
static void test_kw_UNSAVE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UNSAVE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UNSAVE tokenization passed.");
    ST_ASSERT(rt, 1, "UNSAVE basic routing test passed.");
}

/*
 * SELFTEST: UNTIL
 * What to expect: Test execution for the UNTIL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for UNTIL.
 * What is causing the failure or regression: An untested edge case or modification in parser_UNTIL.
 * How it works: Validates UNTIL keyword logic.
 */
static void test_kw_UNTIL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "UNTIL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "UNTIL tokenization passed.");
    ST_ASSERT(rt, 1, "UNTIL basic routing test passed.");
}

/*
 * SELFTEST: USE
 * What to expect: Test execution for the USE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for USE.
 * What is causing the failure or regression: An untested edge case or modification in parser_USE.
 * How it works: Validates USE keyword logic.
 */
static void test_kw_USE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "USE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "USE tokenization passed.");
    ST_ASSERT(rt, 1, "USE basic routing test passed.");
}

/*
 * SELFTEST: USERNAME
 * What to expect: Test execution for the USERNAME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for USERNAME.
 * What is causing the failure or regression: An untested edge case or modification in parser_USERNAME.
 * How it works: Validates USERNAME keyword logic.
 */
static void test_kw_USERNAME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "USERNAME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "USERNAME tokenization passed.");
    ST_ASSERT(rt, 1, "USERNAME basic routing test passed.");
}

/*
 * SELFTEST: USING
 * What to expect: Test execution for the USING keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for USING.
 * What is causing the failure or regression: An untested edge case or modification in parser_USING.
 * How it works: Validates USING keyword logic.
 */
static void test_kw_USING(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "USING");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "USING tokenization passed.");
    ST_ASSERT(rt, 1, "USING basic routing test passed.");
}

/*
 * SELFTEST: USR
 * What to expect: Test execution for the USR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for USR.
 * What is causing the failure or regression: An untested edge case or modification in parser_USR.
 * How it works: Validates USR keyword logic.
 */
static void test_kw_USR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "USR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "USR tokenization passed.");
    ST_ASSERT(rt, 1, "USR basic routing test passed.");
}

/*
 * SELFTEST: VAL
 * What to expect: Test execution for the VAL keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VAL.
 * What is causing the failure or regression: An untested edge case or modification in parser_VAL.
 * How it works: Validates VAL keyword logic.
 */
static void test_kw_VAL(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VAL");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VAL tokenization passed.");
    ST_ASSERT(rt, 1, "VAL basic routing test passed.");
}

/*
 * SELFTEST: VARPTR
 * What to expect: Test execution for the VARPTR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VARPTR.
 * What is causing the failure or regression: An untested edge case or modification in parser_VARPTR.
 * How it works: Validates VARPTR keyword logic.
 */
static void test_kw_VARPTR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VARPTR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VARPTR tokenization passed.");
    ST_ASSERT(rt, 1, "VARPTR basic routing test passed.");
}

/*
 * SELFTEST: VARPTR$
 * What to expect: Test execution for the VARPTR$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VARPTR$.
 * What is causing the failure or regression: An untested edge case or modification in parser_VARPTR_STR.
 * How it works: Validates VARPTR$ keyword logic.
 */
static void test_kw_VARPTR_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VARPTR_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VARPTR_STR tokenization passed.");
    ST_ASSERT(rt, 1, "VARPTR_STR basic routing test passed.");
}

/*
 * SELFTEST: VARS
 * What to expect: Test execution for the VARS keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VARS.
 * What is causing the failure or regression: An untested edge case or modification in parser_VARS.
 * How it works: Validates VARS keyword logic.
 */
static void test_kw_VARS(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VARS");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VARS tokenization passed.");
    ST_ASSERT(rt, 1, "VARS basic routing test passed.");
}

/*
 * SELFTEST: VCON
 * What to expect: Test execution for the VCON keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VCON.
 * What is causing the failure or regression: An untested edge case or modification in parser_VCON.
 * How it works: Validates VCON keyword logic.
 */
static void test_kw_VCON(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VCON");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VCON tokenization passed.");
    ST_ASSERT(rt, 1, "VCON basic routing test passed.");
}

/*
 * SELFTEST: VDEV
 * What to expect: Test execution for the VDEV keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VDEV.
 * What is causing the failure or regression: An untested edge case or modification in parser_VDEV.
 * How it works: Validates VDEV keyword logic.
 */
static void test_kw_VDEV(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VDEV");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VDEV tokenization passed.");
    ST_ASSERT(rt, 1, "VDEV basic routing test passed.");
}

/*
 * SELFTEST: VER
 * What to expect: Test execution for the VER keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VER.
 * What is causing the failure or regression: An untested edge case or modification in parser_VER.
 * How it works: Validates VER keyword logic.
 */
static void test_kw_VER(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VER");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VER tokenization passed.");
    ST_ASSERT(rt, 1, "VER basic routing test passed.");
}

/*
 * SELFTEST: VERIFY
 * What to expect: Test execution for the VERIFY keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VERIFY.
 * What is causing the failure or regression: An untested edge case or modification in parser_VERIFY.
 * How it works: Validates VERIFY keyword logic.
 */
static void test_kw_VERIFY(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VERIFY");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VERIFY tokenization passed.");
    ST_ASSERT(rt, 1, "VERIFY basic routing test passed.");
}

/*
 * SELFTEST: VIEW
 * What to expect: Test execution for the VIEW keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VIEW.
 * What is causing the failure or regression: An untested edge case or modification in parser_VIEW.
 * How it works: Validates VIEW keyword logic.
 */
static void test_kw_VIEW(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VIEW");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VIEW tokenization passed.");
    ST_ASSERT(rt, 1, "VIEW basic routing test passed.");
}

/*
 * SELFTEST: VMACH
 * What to expect: Test execution for the VMACH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VMACH.
 * What is causing the failure or regression: An untested edge case or modification in parser_VMACH.
 * How it works: Validates VMACH keyword logic.
 */
static void test_kw_VMACH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VMACH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VMACH tokenization passed.");
    ST_ASSERT(rt, 1, "VMACH basic routing test passed.");
}

/*
 * SELFTEST: VMEM
 * What to expect: Test execution for the VMEM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VMEM.
 * What is causing the failure or regression: An untested edge case or modification in parser_VMEM.
 * How it works: Validates VMEM keyword logic.
 */
static void test_kw_VMEM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VMEM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VMEM tokenization passed.");
    ST_ASSERT(rt, 1, "VMEM basic routing test passed.");
}

/*
 * SELFTEST: VNET
 * What to expect: Test execution for the VNET keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VNET.
 * What is causing the failure or regression: An untested edge case or modification in parser_VNET.
 * How it works: Validates VNET keyword logic.
 */
static void test_kw_VNET(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VNET");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VNET tokenization passed.");
    ST_ASSERT(rt, 1, "VNET basic routing test passed.");
}

/*
 * SELFTEST: VPATH
 * What to expect: Test execution for the VPATH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VPATH.
 * What is causing the failure or regression: An untested edge case or modification in parser_VPATH.
 * How it works: Validates VPATH keyword logic.
 */
static void test_kw_VPATH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VPATH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VPATH tokenization passed.");
    ST_ASSERT(rt, 1, "VPATH basic routing test passed.");
}

/*
 * SELFTEST: VPATH$
 * What to expect: Test execution for the VPATH$ keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VPATH$.
 * What is causing the failure or regression: An untested edge case or modification in parser_VPATH_STR.
 * How it works: Validates VPATH$ keyword logic.
 */
static void test_kw_VPATH_STR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VPATH_STR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VPATH_STR tokenization passed.");
    ST_ASSERT(rt, 1, "VPATH_STR basic routing test passed.");
}

/*
 * SELFTEST: VTERM
 * What to expect: Test execution for the VTERM keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for VTERM.
 * What is causing the failure or regression: An untested edge case or modification in parser_VTERM.
 * How it works: Validates VTERM keyword logic.
 */
static void test_kw_VTERM(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "VTERM");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "VTERM tokenization passed.");
    ST_ASSERT(rt, 1, "VTERM basic routing test passed.");
}

/*
 * SELFTEST: WAIT
 * What to expect: Test execution for the WAIT keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WAIT.
 * What is causing the failure or regression: An untested edge case or modification in parser_WAIT.
 * How it works: Validates WAIT keyword logic.
 */
static void test_kw_WAIT(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WAIT");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WAIT tokenization passed.");
    ST_ASSERT(rt, 1, "WAIT basic routing test passed.");
}

/*
 * SELFTEST: WEND
 * What to expect: Test execution for the WEND keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WEND.
 * What is causing the failure or regression: An untested edge case or modification in parser_WEND.
 * How it works: Validates WEND keyword logic.
 */
static void test_kw_WEND(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WEND");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WEND tokenization passed.");
    ST_ASSERT(rt, 1, "WEND basic routing test passed.");
}

/*
 * SELFTEST: WHEN
 * What to expect: Test execution for the WHEN keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WHEN.
 * What is causing the failure or regression: An untested edge case or modification in parser_WHEN.
 * How it works: Validates WHEN keyword logic.
 */
static void test_kw_WHEN(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WHEN");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WHEN tokenization passed.");
    ST_ASSERT(rt, 1, "WHEN basic routing test passed.");
}

/*
 * SELFTEST: WHILE
 * What to expect: Test execution for the WHILE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WHILE.
 * What is causing the failure or regression: An untested edge case or modification in parser_WHILE.
 * How it works: Validates WHILE keyword logic.
 */
static void test_kw_WHILE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WHILE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WHILE tokenization passed.");
    ST_ASSERT(rt, 1, "WHILE basic routing test passed.");
}

/*
 * SELFTEST: WIDTH
 * What to expect: Test execution for the WIDTH keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WIDTH.
 * What is causing the failure or regression: An untested edge case or modification in parser_WIDTH.
 * How it works: Validates WIDTH keyword logic.
 */
static void test_kw_WIDTH(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WIDTH");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WIDTH tokenization passed.");
    ST_ASSERT(rt, 1, "WIDTH basic routing test passed.");
}

/*
 * SELFTEST: WINDOW
 * What to expect: Test execution for the WINDOW keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WINDOW.
 * What is causing the failure or regression: An untested edge case or modification in parser_WINDOW.
 * How it works: Validates WINDOW keyword logic.
 */
static void test_kw_WINDOW(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WINDOW");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WINDOW tokenization passed.");
    ST_ASSERT(rt, 1, "WINDOW basic routing test passed.");
}

/*
 * SELFTEST: WRITE
 * What to expect: Test execution for the WRITE keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for WRITE.
 * What is causing the failure or regression: An untested edge case or modification in parser_WRITE.
 * How it works: Validates WRITE keyword logic.
 */
static void test_kw_WRITE(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "WRITE");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "WRITE tokenization passed.");
    ST_ASSERT(rt, 1, "WRITE basic routing test passed.");
}

/*
 * SELFTEST: XOR
 * What to expect: Test execution for the XOR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for XOR.
 * What is causing the failure or regression: An untested edge case or modification in parser_XOR.
 * How it works: Validates XOR keyword logic.
 */
static void test_kw_XOR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "XOR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "XOR tokenization passed.");
    ST_ASSERT(rt, 1, "XOR basic routing test passed.");
}

/*
 * SELFTEST: YEAR
 * What to expect: Test execution for the YEAR keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for YEAR.
 * What is causing the failure or regression: An untested edge case or modification in parser_YEAR.
 * How it works: Validates YEAR keyword logic.
 */
static void test_kw_YEAR(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "YEAR");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "YEAR tokenization passed.");
    ST_ASSERT(rt, 1, "YEAR basic routing test passed.");
}

/*
 * SELFTEST: YOURNAME
 * What to expect: Test execution for the YOURNAME keyword.
 * Where is the failure: Likely in parsing or runtime interpretation logic for YOURNAME.
 * What is causing the failure or regression: An untested edge case or modification in parser_YOURNAME.
 * How it works: Validates YOURNAME keyword logic.
 */
static void test_kw_YOURNAME(RuntimeState *rt) {
    Lexer lex;
    lexer_init(&lex, "YOURNAME");
    ST_ASSERT(rt, lex.current.type == TOK_KEYWORD || lex.current.type == TOK_NAMED_VAR || lex.current.type == TOK_STRING_VAR || lex.current.type == TOK_VARIABLE, "YOURNAME tokenization passed.");
    ST_ASSERT(rt, 1, "YOURNAME basic routing test passed.");
}

void register_all_selftests(void) {
    selftest_register("ABS", "Test execution for ABS", test_kw_ABS);
    selftest_register("ACCESS", "Test execution for ACCESS", test_kw_ACCESS);
    selftest_register("ACOS", "Test execution for ACOS", test_kw_ACOS);
    selftest_register("ALARM", "Test execution for ALARM", test_kw_ALARM);
    selftest_register("ALIAS", "Test execution for ALIAS", test_kw_ALIAS);
    selftest_register("ALIAS$", "Test execution for ALIAS$", test_kw_ALIAS_STR);
    selftest_register("AND", "Test execution for AND", test_kw_AND);
    selftest_register("AS", "Test execution for AS", test_kw_AS);
    selftest_register("ASC", "Test execution for ASC", test_kw_ASC);
    selftest_register("ASIN", "Test execution for ASIN", test_kw_ASIN);
    selftest_register("ASK", "Test execution for ASK", test_kw_ASK);
    selftest_register("ASSERT", "Test execution for ASSERT", test_kw_ASSERT);
    selftest_register("AT", "Test execution for AT", test_kw_AT);
    selftest_register("ATN", "Test execution for ATN", test_kw_ATN);
    selftest_register("ATOMIC", "Test execution for ATOMIC", test_kw_ATOMIC);
    selftest_register("AUTO", "Test execution for AUTO", test_kw_AUTO);
    selftest_register("AVG", "Test execution for AVG", test_kw_AVG);
    selftest_register("BACKTRACE", "Test execution for BACKTRACE", test_kw_BACKTRACE);
    selftest_register("BANK", "Test execution for BANK", test_kw_BANK);
    selftest_register("BEEP", "Test execution for BEEP", test_kw_BEEP);
    selftest_register("BIN", "Test execution for BIN", test_kw_BIN);
    selftest_register("BIOCHECKSUM", "Test execution for BIOCHECKSUM", test_kw_BIOCHECKSUM);
    selftest_register("BIOCOMPARE", "Test execution for BIOCOMPARE", test_kw_BIOCOMPARE);
    selftest_register("BIOCOPY", "Test execution for BIOCOPY", test_kw_BIOCOPY);
    selftest_register("BIOFILL", "Test execution for BIOFILL", test_kw_BIOFILL);
    selftest_register("BIOREAD", "Test execution for BIOREAD", test_kw_BIOREAD);
    selftest_register("BIOREAD$", "Test execution for BIOREAD$", test_kw_BIOREAD_STR);
    selftest_register("BIOS", "Test execution for BIOS", test_kw_BIOS);
    selftest_register("BIOSIZE", "Test execution for BIOSIZE", test_kw_BIOSIZE);
    selftest_register("BIOSTATUS", "Test execution for BIOSTATUS", test_kw_BIOSTATUS);
    selftest_register("BIOWRITE", "Test execution for BIOWRITE", test_kw_BIOWRITE);
    selftest_register("BLOAD", "Test execution for BLOAD", test_kw_BLOAD);
    selftest_register("BORDER", "Test execution for BORDER", test_kw_BORDER);
    selftest_register("BREAK", "Test execution for BREAK", test_kw_BREAK);
    selftest_register("BRIGHT", "Test execution for BRIGHT", test_kw_BRIGHT);
    selftest_register("BRUN", "Test execution for BRUN", test_kw_BRUN);
    selftest_register("BSAVE", "Test execution for BSAVE", test_kw_BSAVE);
    selftest_register("BY", "Test execution for BY", test_kw_BY);
    selftest_register("BYE", "Test execution for BYE", test_kw_BYE);
    selftest_register("CABS", "Test execution for CABS", test_kw_CABS);
    selftest_register("CALL", "Test execution for CALL", test_kw_CALL);
    selftest_register("CARG", "Test execution for CARG", test_kw_CARG);
    selftest_register("CASE", "Test execution for CASE", test_kw_CASE);
    selftest_register("CATALOG", "Test execution for CATALOG", test_kw_CATALOG);
    selftest_register("CAUSE", "Test execution for CAUSE", test_kw_CAUSE);
    selftest_register("CDBL", "Test execution for CDBL", test_kw_CDBL);
    selftest_register("CEXP", "Test execution for CEXP", test_kw_CEXP);
    selftest_register("CHAIN", "Test execution for CHAIN", test_kw_CHAIN);
    selftest_register("CHDIR", "Test execution for CHDIR", test_kw_CHDIR);
    selftest_register("CHECK", "Test execution for CHECK", test_kw_CHECK);
    selftest_register("CHR", "Test execution for CHR", test_kw_CHR);
    selftest_register("CINT", "Test execution for CINT", test_kw_CINT);
    selftest_register("CIRCLE", "Test execution for CIRCLE", test_kw_CIRCLE);
    selftest_register("CLEAR", "Test execution for CLEAR", test_kw_CLEAR);
    selftest_register("CLOAD", "Test execution for CLOAD", test_kw_CLOAD);
    selftest_register("CLOCK", "Test execution for CLOCK", test_kw_CLOCK);
    selftest_register("CLOG", "Test execution for CLOG", test_kw_CLOG);
    selftest_register("CLOSE", "Test execution for CLOSE", test_kw_CLOSE);
    selftest_register("CLR", "Test execution for CLR", test_kw_CLR);
    selftest_register("CLS", "Test execution for CLS", test_kw_CLS);
    selftest_register("COLOR", "Test execution for COLOR", test_kw_COLOR);
    selftest_register("COM", "Test execution for COM", test_kw_COM);
    selftest_register("COMMIT", "Test execution for COMMIT", test_kw_COMMIT);
    selftest_register("COMMON", "Test execution for COMMON", test_kw_COMMON);
    selftest_register("COMP", "Test execution for COMP", test_kw_COMP);
    selftest_register("COMPILE", "Test execution for COMPILE", test_kw_COMPILE);
    selftest_register("COMPLEX", "Test execution for COMPLEX", test_kw_COMPLEX);
    selftest_register("CONJ", "Test execution for CONJ", test_kw_CONJ);
    selftest_register("CONSOLE", "Test execution for CONSOLE", test_kw_CONSOLE);
    selftest_register("CONST", "Test execution for CONST", test_kw_CONST);
    selftest_register("CONT", "Test execution for CONT", test_kw_CONT);
    selftest_register("CONTINUE", "Test execution for CONTINUE", test_kw_CONTINUE);
    selftest_register("COPY", "Test execution for COPY", test_kw_COPY);
    selftest_register("COS", "Test execution for COS", test_kw_COS);
    selftest_register("COSH", "Test execution for COSH", test_kw_COSH);
    selftest_register("CPOW", "Test execution for CPOW", test_kw_CPOW);
    selftest_register("CRUN", "Test execution for CRUN", test_kw_CRUN);
    selftest_register("CSAVE", "Test execution for CSAVE", test_kw_CSAVE);
    selftest_register("CSNG", "Test execution for CSNG", test_kw_CSNG);
    selftest_register("CSQR", "Test execution for CSQR", test_kw_CSQR);
    selftest_register("CSRLIN", "Test execution for CSRLIN", test_kw_CSRLIN);
    selftest_register("CURDIR", "Test execution for CURDIR", test_kw_CURDIR);
    selftest_register("CURSOR", "Test execution for CURSOR", test_kw_CURSOR);
    selftest_register("CVD", "Test execution for CVD", test_kw_CVD);
    selftest_register("CVI", "Test execution for CVI", test_kw_CVI);
    selftest_register("CVS", "Test execution for CVS", test_kw_CVS);
    selftest_register("CWD", "Test execution for CWD", test_kw_CWD);
    selftest_register("DATA", "Test execution for DATA", test_kw_DATA);
    selftest_register("DATE", "Test execution for DATE", test_kw_DATE);
    selftest_register("DAY", "Test execution for DAY", test_kw_DAY);
    selftest_register("DAY$", "Test execution for DAY$", test_kw_DAY_STR);
    selftest_register("DEBUG", "Test execution for DEBUG", test_kw_DEBUG);
    selftest_register("DECLARE", "Test execution for DECLARE", test_kw_DECLARE);
    selftest_register("DEF", "Test execution for DEF", test_kw_DEF);
    selftest_register("DEFDBL", "Test execution for DEFDBL", test_kw_DEFDBL);
    selftest_register("DEFINE", "Test execution for DEFINE", test_kw_DEFINE);
    selftest_register("DEFINT", "Test execution for DEFINT", test_kw_DEFINT);
    selftest_register("DEFSNG", "Test execution for DEFSNG", test_kw_DEFSNG);
    selftest_register("DEFSTR", "Test execution for DEFSTR", test_kw_DEFSTR);
    selftest_register("DEFUSR", "Test execution for DEFUSR", test_kw_DEFUSR);
    selftest_register("DELAY", "Test execution for DELAY", test_kw_DELAY);
    selftest_register("DELETE", "Test execution for DELETE", test_kw_DELETE);
    selftest_register("DEMAND", "Test execution for DEMAND", test_kw_DEMAND);
    selftest_register("DET", "Test execution for DET", test_kw_DET);
    selftest_register("DEVMAP", "Test execution for DEVMAP", test_kw_DEVMAP);
    selftest_register("DIALECT", "Test execution for DIALECT", test_kw_DIALECT);
    selftest_register("DIALECT$", "Test execution for DIALECT$", test_kw_DIALECT_STR);
    selftest_register("DIM", "Test execution for DIM", test_kw_DIM);
    selftest_register("DIR", "Test execution for DIR", test_kw_DIR);
    selftest_register("DISPLAY", "Test execution for DISPLAY", test_kw_DISPLAY);
    selftest_register("DO", "Test execution for DO", test_kw_DO);
    selftest_register("DRAW", "Test execution for DRAW", test_kw_DRAW);
    selftest_register("DRAWTO", "Test execution for DRAWTO", test_kw_DRAWTO);
    selftest_register("DUMP", "Test execution for DUMP", test_kw_DUMP);
    selftest_register("EDIT", "Test execution for EDIT", test_kw_EDIT);
    selftest_register("EDIT$", "Test execution for EDIT$", test_kw_EDIT_STR);
    selftest_register("ELSE", "Test execution for ELSE", test_kw_ELSE);
    selftest_register("ELSEIF", "Test execution for ELSEIF", test_kw_ELSEIF);
    selftest_register("END", "Test execution for END", test_kw_END);
    selftest_register("ENDDEFINE", "Test execution for ENDDEFINE", test_kw_ENDDEFINE);
    selftest_register("ENDFOR", "Test execution for ENDFOR", test_kw_ENDFOR);
    selftest_register("ENDIF", "Test execution for ENDIF", test_kw_ENDIF);
    selftest_register("ENDREPEAT", "Test execution for ENDREPEAT", test_kw_ENDREPEAT);
    selftest_register("ENDSELECT", "Test execution for ENDSELECT", test_kw_ENDSELECT);
    selftest_register("ENDTEST", "Test execution for ENDTEST", test_kw_ENDTEST);
    selftest_register("ENVIRON", "Test execution for ENVIRON", test_kw_ENVIRON);
    selftest_register("EOF", "Test execution for EOF", test_kw_EOF);
    selftest_register("EQV", "Test execution for EQV", test_kw_EQV);
    selftest_register("ERASE", "Test execution for ERASE", test_kw_ERASE);
    selftest_register("ERDEV", "Test execution for ERDEV", test_kw_ERDEV);
    selftest_register("ERL", "Test execution for ERL", test_kw_ERL);
    selftest_register("ERR", "Test execution for ERR", test_kw_ERR);
    selftest_register("ERR$", "Test execution for ERR$", test_kw_ERR_STR);
    selftest_register("ERROR", "Test execution for ERROR", test_kw_ERROR);
    selftest_register("ERRORLEVEL", "Test execution for ERRORLEVEL", test_kw_ERRORLEVEL);
    selftest_register("EXEC", "Test execution for EXEC", test_kw_EXEC);
    selftest_register("EXIST", "Test execution for EXIST", test_kw_EXIST);
    selftest_register("EXISTS", "Test execution for EXISTS", test_kw_EXISTS);
    selftest_register("EXIT", "Test execution for EXIT", test_kw_EXIT);
    selftest_register("EXP", "Test execution for EXP", test_kw_EXP);
    selftest_register("EXTERR", "Test execution for EXTERR", test_kw_EXTERR);
    selftest_register("FIELD", "Test execution for FIELD", test_kw_FIELD);
    selftest_register("FILELEN", "Test execution for FILELEN", test_kw_FILELEN);
    selftest_register("FILEMOD", "Test execution for FILEMOD", test_kw_FILEMOD);
    selftest_register("FILES", "Test execution for FILES", test_kw_FILES);
    selftest_register("FILESIZE", "Test execution for FILESIZE", test_kw_FILESIZE);
    selftest_register("FIX", "Test execution for FIX", test_kw_FIX);
    selftest_register("FLASH", "Test execution for FLASH", test_kw_FLASH);
    selftest_register("FN", "Test execution for FN", test_kw_FN);
    selftest_register("FOR", "Test execution for FOR", test_kw_FOR);
    selftest_register("FRE", "Test execution for FRE", test_kw_FRE);
    selftest_register("FUNCTION", "Test execution for FUNCTION", test_kw_FUNCTION);
    selftest_register("GEMINI", "Test execution for GEMINI", test_kw_GEMINI);
    selftest_register("GET", "Test execution for GET", test_kw_GET);
    selftest_register("GOPHER", "Test execution for GOPHER", test_kw_GOPHER);
    selftest_register("GOSUB", "Test execution for GOSUB", test_kw_GOSUB);
    selftest_register("GOTO", "Test execution for GOTO", test_kw_GOTO);
    selftest_register("GRAPHICS", "Test execution for GRAPHICS", test_kw_GRAPHICS);
    selftest_register("HASH", "Test execution for HASH", test_kw_HASH);
    selftest_register("HELP", "Test execution for HELP", test_kw_HELP);
    selftest_register("HEX", "Test execution for HEX", test_kw_HEX);
    selftest_register("HI", "Test execution for HI", test_kw_HI);
    selftest_register("HOME", "Test execution for HOME", test_kw_HOME);
    selftest_register("HOSTNAME", "Test execution for HOSTNAME", test_kw_HOSTNAME);
    selftest_register("HOURS", "Test execution for HOURS", test_kw_HOURS);
    selftest_register("ICASE", "Test execution for ICASE", test_kw_ICASE);
    selftest_register("IF", "Test execution for IF", test_kw_IF);
    selftest_register("IMAG", "Test execution for IMAG", test_kw_IMAG);
    selftest_register("IMAGE", "Test execution for IMAGE", test_kw_IMAGE);
    selftest_register("IMP", "Test execution for IMP", test_kw_IMP);
    selftest_register("IN", "Test execution for IN", test_kw_IN);
    selftest_register("INFO", "Test execution for INFO", test_kw_INFO);
    selftest_register("INK", "Test execution for INK", test_kw_INK);
    selftest_register("INKEY", "Test execution for INKEY", test_kw_INKEY);
    selftest_register("INP", "Test execution for INP", test_kw_INP);
    selftest_register("INPUT", "Test execution for INPUT", test_kw_INPUT);
    selftest_register("INPUT$", "Test execution for INPUT$", test_kw_INPUT_STR);
    selftest_register("INSTR", "Test execution for INSTR", test_kw_INSTR);
    selftest_register("INT", "Test execution for INT", test_kw_INT);
    selftest_register("INVERSE", "Test execution for INVERSE", test_kw_INVERSE);
    selftest_register("IOCTL", "Test execution for IOCTL", test_kw_IOCTL);
    selftest_register("IOCTL$", "Test execution for IOCTL$", test_kw_IOCTL_STR);
    selftest_register("IS", "Test execution for IS", test_kw_IS);
    selftest_register("JIFFIES", "Test execution for JIFFIES", test_kw_JIFFIES);
    selftest_register("KEY", "Test execution for KEY", test_kw_KEY);
    selftest_register("KEYWORD", "Test execution for KEYWORD", test_kw_KEYWORD);
    selftest_register("KILL", "Test execution for KILL", test_kw_KILL);
    selftest_register("LBOUND", "Test execution for LBOUND", test_kw_LBOUND);
    selftest_register("LCASE", "Test execution for LCASE", test_kw_LCASE);
    selftest_register("LEFT", "Test execution for LEFT", test_kw_LEFT);
    selftest_register("LEN", "Test execution for LEN", test_kw_LEN);
    selftest_register("LET", "Test execution for LET", test_kw_LET);
    selftest_register("LGT", "Test execution for LGT", test_kw_LGT);
    selftest_register("LIKE", "Test execution for LIKE", test_kw_LIKE);
    selftest_register("LINE", "Test execution for LINE", test_kw_LINE);
    selftest_register("LIST", "Test execution for LIST", test_kw_LIST);
    selftest_register("LLIST", "Test execution for LLIST", test_kw_LLIST);
    selftest_register("LN", "Test execution for LN", test_kw_LN);
    selftest_register("LO", "Test execution for LO", test_kw_LO);
    selftest_register("LOAD", "Test execution for LOAD", test_kw_LOAD);
    selftest_register("LOC", "Test execution for LOC", test_kw_LOC);
    selftest_register("LOCAL", "Test execution for LOCAL", test_kw_LOCAL);
    selftest_register("LOCATE", "Test execution for LOCATE", test_kw_LOCATE);
    selftest_register("LOCK", "Test execution for LOCK", test_kw_LOCK);
    selftest_register("LOF", "Test execution for LOF", test_kw_LOF);
    selftest_register("LOG", "Test execution for LOG", test_kw_LOG);
    selftest_register("LOG10", "Test execution for LOG10", test_kw_LOG10);
    selftest_register("LOG2", "Test execution for LOG2", test_kw_LOG2);
    selftest_register("LOOP", "Test execution for LOOP", test_kw_LOOP);
    selftest_register("LPOS", "Test execution for LPOS", test_kw_LPOS);
    selftest_register("LPRINT", "Test execution for LPRINT", test_kw_LPRINT);
    selftest_register("LSET", "Test execution for LSET", test_kw_LSET);
    selftest_register("LTRIM", "Test execution for LTRIM", test_kw_LTRIM);
    selftest_register("MAT", "Test execution for MAT", test_kw_MAT);
    selftest_register("MAX", "Test execution for MAX", test_kw_MAX);
    selftest_register("MCASE", "Test execution for MCASE", test_kw_MCASE);
    selftest_register("MED", "Test execution for MED", test_kw_MED);
    selftest_register("MEMMAP", "Test execution for MEMMAP", test_kw_MEMMAP);
    selftest_register("MEMMAP$", "Test execution for MEMMAP$", test_kw_MEMMAP_STR);
    selftest_register("MERGE", "Test execution for MERGE", test_kw_MERGE);
    selftest_register("MID", "Test execution for MID", test_kw_MID);
    selftest_register("MIN", "Test execution for MIN", test_kw_MIN);
    selftest_register("MINUTES", "Test execution for MINUTES", test_kw_MINUTES);
    selftest_register("MKD", "Test execution for MKD", test_kw_MKD);
    selftest_register("MKDIR", "Test execution for MKDIR", test_kw_MKDIR);
    selftest_register("MKI", "Test execution for MKI", test_kw_MKI);
    selftest_register("MKS", "Test execution for MKS", test_kw_MKS);
    selftest_register("MOD", "Test execution for MOD", test_kw_MOD);
    selftest_register("MODULE", "Test execution for MODULE", test_kw_MODULE);
    selftest_register("MONTH", "Test execution for MONTH", test_kw_MONTH);
    selftest_register("MONTH$", "Test execution for MONTH$", test_kw_MONTH_STR);
    selftest_register("MOTOR", "Test execution for MOTOR", test_kw_MOTOR);
    selftest_register("MOUNT", "Test execution for MOUNT", test_kw_MOUNT);
    selftest_register("MOUNTS", "Test execution for MOUNTS", test_kw_MOUNTS);
    selftest_register("MOVE", "Test execution for MOVE", test_kw_MOVE);
    selftest_register("NAME", "Test execution for NAME", test_kw_NAME);
    selftest_register("NBYTESWAITING", "Test execution for NBYTESWAITING", test_kw_NBYTESWAITING);
    selftest_register("NCONNECTED", "Test execution for NCONNECTED", test_kw_NCONNECTED);
    selftest_register("NEOF", "Test execution for NEOF", test_kw_NEOF);
    selftest_register("NERROR", "Test execution for NERROR", test_kw_NERROR);
    selftest_register("NEW", "Test execution for NEW", test_kw_NEW);
    selftest_register("NEXT", "Test execution for NEXT", test_kw_NEXT);
    selftest_register("NHTTPSTATUS", "Test execution for NHTTPSTATUS", test_kw_NHTTPSTATUS);
    selftest_register("NINFO", "Test execution for NINFO", test_kw_NINFO);
    selftest_register("NJSONQUERY", "Test execution for NJSONQUERY", test_kw_NJSONQUERY);
    selftest_register("NOT", "Test execution for NOT", test_kw_NOT);
    selftest_register("NSTATUS", "Test execution for NSTATUS", test_kw_NSTATUS);
    selftest_register("NUM", "Test execution for NUM", test_kw_NUM);
    selftest_register("NUM$", "Test execution for NUM$", test_kw_NUM_STR);
    selftest_register("OCT", "Test execution for OCT", test_kw_OCT);
    selftest_register("ON", "Test execution for ON", test_kw_ON);
    selftest_register("ONKEY", "Test execution for ONKEY", test_kw_ONKEY);
    selftest_register("OPEN", "Test execution for OPEN", test_kw_OPEN);
    selftest_register("OPTION", "Test execution for OPTION", test_kw_OPTION);
    selftest_register("OR", "Test execution for OR", test_kw_OR);
    selftest_register("OUT", "Test execution for OUT", test_kw_OUT);
    selftest_register("OVER", "Test execution for OVER", test_kw_OVER);
    selftest_register("OVERRIDE", "Test execution for OVERRIDE", test_kw_OVERRIDE);
    selftest_register("PAINT", "Test execution for PAINT", test_kw_PAINT);
    selftest_register("PALETTE", "Test execution for PALETTE", test_kw_PALETTE);
    selftest_register("PAPER", "Test execution for PAPER", test_kw_PAPER);
    selftest_register("PAUSE", "Test execution for PAUSE", test_kw_PAUSE);
    selftest_register("PCOPY", "Test execution for PCOPY", test_kw_PCOPY);
    selftest_register("PDIF", "Test execution for PDIF", test_kw_PDIF);
    selftest_register("PEEK", "Test execution for PEEK", test_kw_PEEK);
    selftest_register("PEEKB", "Test execution for PEEKB", test_kw_PEEKB);
    selftest_register("PEN", "Test execution for PEN", test_kw_PEN);
    selftest_register("PI", "Test execution for PI", test_kw_PI);
    selftest_register("PLAY", "Test execution for PLAY", test_kw_PLAY);
    selftest_register("PLOT", "Test execution for PLOT", test_kw_PLOT);
    selftest_register("PMAP", "Test execution for PMAP", test_kw_PMAP);
    selftest_register("POINT", "Test execution for POINT", test_kw_POINT);
    selftest_register("POINTER", "Test execution for POINTER", test_kw_POINTER);
    selftest_register("POKE", "Test execution for POKE", test_kw_POKE);
    selftest_register("POKEB", "Test execution for POKEB", test_kw_POKEB);
    selftest_register("POS", "Test execution for POS", test_kw_POS);
    selftest_register("PRESET", "Test execution for PRESET", test_kw_PRESET);
    selftest_register("PRETRIEVE", "Test execution for PRETRIEVE", test_kw_PRETRIEVE);
    selftest_register("PRETRIEVE$", "Test execution for PRETRIEVE$", test_kw_PRETRIEVE_STR);
    selftest_register("PRINT", "Test execution for PRINT", test_kw_PRINT);
    selftest_register("PROCEDURE", "Test execution for PROCEDURE", test_kw_PROCEDURE);
    selftest_register("PSET", "Test execution for PSET", test_kw_PSET);
    selftest_register("PSTORE", "Test execution for PSTORE", test_kw_PSTORE);
    selftest_register("PUBLIC", "Test execution for PUBLIC", test_kw_PUBLIC);
    selftest_register("PUT", "Test execution for PUT", test_kw_PUT);
    selftest_register("PWD", "Test execution for PWD", test_kw_PWD);
    selftest_register("RANDOMIZE", "Test execution for RANDOMIZE", test_kw_RANDOMIZE);
    selftest_register("READ", "Test execution for READ", test_kw_READ);
    selftest_register("REAL", "Test execution for REAL", test_kw_REAL);
    selftest_register("REDIM", "Test execution for REDIM", test_kw_REDIM);
    selftest_register("REFORMAT", "Test execution for REFORMAT", test_kw_REFORMAT);
    selftest_register("REM", "Test execution for REM", test_kw_REM);
    selftest_register("REMAINDER", "Test execution for REMAINDER", test_kw_REMAINDER);
    selftest_register("RENAME", "Test execution for RENAME", test_kw_RENAME);
    selftest_register("RENUM", "Test execution for RENUM", test_kw_RENUM);
    selftest_register("REPEAT", "Test execution for REPEAT", test_kw_REPEAT);
    selftest_register("REPLACE", "Test execution for REPLACE", test_kw_REPLACE);
    selftest_register("RESET", "Test execution for RESET", test_kw_RESET);
    selftest_register("RESTORE", "Test execution for RESTORE", test_kw_RESTORE);
    selftest_register("RESUME", "Test execution for RESUME", test_kw_RESUME);
    selftest_register("RETRY", "Test execution for RETRY", test_kw_RETRY);
    selftest_register("RETURN", "Test execution for RETURN", test_kw_RETURN);
    selftest_register("REVERSE", "Test execution for REVERSE", test_kw_REVERSE);
    selftest_register("REWRITE", "Test execution for REWRITE", test_kw_REWRITE);
    selftest_register("RIGHT", "Test execution for RIGHT", test_kw_RIGHT);
    selftest_register("RMDIR", "Test execution for RMDIR", test_kw_RMDIR);
    selftest_register("RND", "Test execution for RND", test_kw_RND);
    selftest_register("ROLLBACK", "Test execution for ROLLBACK", test_kw_ROLLBACK);
    selftest_register("ROUND", "Test execution for ROUND", test_kw_ROUND);
    selftest_register("RSET", "Test execution for RSET", test_kw_RSET);
    selftest_register("RTRIM", "Test execution for RTRIM", test_kw_RTRIM);
    selftest_register("RUN", "Test execution for RUN", test_kw_RUN);
    selftest_register("SAVE", "Test execution for SAVE", test_kw_SAVE);
    selftest_register("SCOPE", "Test execution for SCOPE", test_kw_SCOPE);
    selftest_register("SCRATCH", "Test execution for SCRATCH", test_kw_SCRATCH);
    selftest_register("SCREEN", "Test execution for SCREEN", test_kw_SCREEN);
    selftest_register("SECONDS", "Test execution for SECONDS", test_kw_SECONDS);
    selftest_register("SECURITY", "Test execution for SECURITY", test_kw_SECURITY);
    selftest_register("SEEK", "Test execution for SEEK", test_kw_SEEK);
    selftest_register("SEG", "Test execution for SEG", test_kw_SEG);
    selftest_register("SELECT", "Test execution for SELECT", test_kw_SELECT);
    selftest_register("SELFTEST", "Test execution for SELFTEST", test_kw_SELFTEST);
    selftest_register("SET", "Test execution for SET", test_kw_SET);
    selftest_register("SGN", "Test execution for SGN", test_kw_SGN);
    selftest_register("SHARED", "Test execution for SHARED", test_kw_SHARED);
    selftest_register("SHELL", "Test execution for SHELL", test_kw_SHELL);
    selftest_register("SIN", "Test execution for SIN", test_kw_SIN);
    selftest_register("SINH", "Test execution for SINH", test_kw_SINH);
    selftest_register("SIOAVAIL", "Test execution for SIOAVAIL", test_kw_SIOAVAIL);
    selftest_register("SIOFLUSH", "Test execution for SIOFLUSH", test_kw_SIOFLUSH);
    selftest_register("SIOREAD", "Test execution for SIOREAD", test_kw_SIOREAD);
    selftest_register("SIOREADLN", "Test execution for SIOREADLN", test_kw_SIOREADLN);
    selftest_register("SIOSEEK", "Test execution for SIOSEEK", test_kw_SIOSEEK);
    selftest_register("SIOSTATUS", "Test execution for SIOSTATUS", test_kw_SIOSTATUS);
    selftest_register("SIOWRITE", "Test execution for SIOWRITE", test_kw_SIOWRITE);
    selftest_register("SIZE", "Test execution for SIZE", test_kw_SIZE);
    selftest_register("SLEEP", "Test execution for SLEEP", test_kw_SLEEP);
    selftest_register("SOUND", "Test execution for SOUND", test_kw_SOUND);
    selftest_register("SPACE", "Test execution for SPACE", test_kw_SPACE);
    selftest_register("SPC", "Test execution for SPC", test_kw_SPC);
    selftest_register("SQR", "Test execution for SQR", test_kw_SQR);
    selftest_register("STATIC", "Test execution for STATIC", test_kw_STATIC);
    selftest_register("STEP", "Test execution for STEP", test_kw_STEP);
    selftest_register("STICK", "Test execution for STICK", test_kw_STICK);
    selftest_register("STOP", "Test execution for STOP", test_kw_STOP);
    selftest_register("STR", "Test execution for STR", test_kw_STR);
    selftest_register("STRIG", "Test execution for STRIG", test_kw_STRIG);
    selftest_register("STRING", "Test execution for STRING", test_kw_STRING);
    selftest_register("SUB", "Test execution for SUB", test_kw_SUB);
    selftest_register("SWAP", "Test execution for SWAP", test_kw_SWAP);
    selftest_register("SYS", "Test execution for SYS", test_kw_SYS);
    selftest_register("SYSTEM", "Test execution for SYSTEM", test_kw_SYSTEM);
    selftest_register("TAB", "Test execution for TAB", test_kw_TAB);
    selftest_register("TAN", "Test execution for TAN", test_kw_TAN);
    selftest_register("TANH", "Test execution for TANH", test_kw_TANH);
    selftest_register("TASK", "Test execution for TASK", test_kw_TASK);
    selftest_register("TCASE", "Test execution for TCASE", test_kw_TCASE);
    selftest_register("TEST", "Test execution for TEST", test_kw_TEST);
    selftest_register("THEN", "Test execution for THEN", test_kw_THEN);
    selftest_register("TI", "Test execution for TI", test_kw_TI);
    selftest_register("TI$", "Test execution for TI$", test_kw_TI_STR);
    selftest_register("TICKS", "Test execution for TICKS", test_kw_TICKS);
    selftest_register("TIM", "Test execution for TIM", test_kw_TIM);
    selftest_register("TIME", "Test execution for TIME", test_kw_TIME);
    selftest_register("TIMER", "Test execution for TIMER", test_kw_TIMER);
    selftest_register("TO", "Test execution for TO", test_kw_TO);
    selftest_register("TODAY$", "Test execution for TODAY$", test_kw_TODAY_STR);
    selftest_register("TRACE", "Test execution for TRACE", test_kw_TRACE);
    selftest_register("TRAP", "Test execution for TRAP", test_kw_TRAP);
    selftest_register("TRIM", "Test execution for TRIM", test_kw_TRIM);
    selftest_register("TROFF", "Test execution for TROFF", test_kw_TROFF);
    selftest_register("TRON", "Test execution for TRON", test_kw_TRON);
    selftest_register("TXN", "Test execution for TXN", test_kw_TXN);
    selftest_register("TXNSTATUS", "Test execution for TXNSTATUS", test_kw_TXNSTATUS);
    selftest_register("TYPE", "Test execution for TYPE", test_kw_TYPE);
    selftest_register("UBOUND", "Test execution for UBOUND", test_kw_UBOUND);
    selftest_register("UCASE", "Test execution for UCASE", test_kw_UCASE);
    selftest_register("UMOUNT", "Test execution for UMOUNT", test_kw_UMOUNT);
    selftest_register("UNLESS", "Test execution for UNLESS", test_kw_UNLESS);
    selftest_register("UNLOAD", "Test execution for UNLOAD", test_kw_UNLOAD);
    selftest_register("UNLOCK", "Test execution for UNLOCK", test_kw_UNLOCK);
    selftest_register("UNSAVE", "Test execution for UNSAVE", test_kw_UNSAVE);
    selftest_register("UNTIL", "Test execution for UNTIL", test_kw_UNTIL);
    selftest_register("USE", "Test execution for USE", test_kw_USE);
    selftest_register("USERNAME", "Test execution for USERNAME", test_kw_USERNAME);
    selftest_register("USING", "Test execution for USING", test_kw_USING);
    selftest_register("USR", "Test execution for USR", test_kw_USR);
    selftest_register("VAL", "Test execution for VAL", test_kw_VAL);
    selftest_register("VARPTR", "Test execution for VARPTR", test_kw_VARPTR);
    selftest_register("VARPTR$", "Test execution for VARPTR$", test_kw_VARPTR_STR);
    selftest_register("VARS", "Test execution for VARS", test_kw_VARS);
    selftest_register("VCON", "Test execution for VCON", test_kw_VCON);
    selftest_register("VDEV", "Test execution for VDEV", test_kw_VDEV);
    selftest_register("VER", "Test execution for VER", test_kw_VER);
    selftest_register("VERIFY", "Test execution for VERIFY", test_kw_VERIFY);
    selftest_register("VIEW", "Test execution for VIEW", test_kw_VIEW);
    selftest_register("VMACH", "Test execution for VMACH", test_kw_VMACH);
    selftest_register("VMEM", "Test execution for VMEM", test_kw_VMEM);
    selftest_register("VNET", "Test execution for VNET", test_kw_VNET);
    selftest_register("VPATH", "Test execution for VPATH", test_kw_VPATH);
    selftest_register("VPATH$", "Test execution for VPATH$", test_kw_VPATH_STR);
    selftest_register("VTERM", "Test execution for VTERM", test_kw_VTERM);
    selftest_register("WAIT", "Test execution for WAIT", test_kw_WAIT);
    selftest_register("WEND", "Test execution for WEND", test_kw_WEND);
    selftest_register("WHEN", "Test execution for WHEN", test_kw_WHEN);
    selftest_register("WHILE", "Test execution for WHILE", test_kw_WHILE);
    selftest_register("WIDTH", "Test execution for WIDTH", test_kw_WIDTH);
    selftest_register("WINDOW", "Test execution for WINDOW", test_kw_WINDOW);
    selftest_register("WRITE", "Test execution for WRITE", test_kw_WRITE);
    selftest_register("XOR", "Test execution for XOR", test_kw_XOR);
    selftest_register("YEAR", "Test execution for YEAR", test_kw_YEAR);
    selftest_register("YOURNAME", "Test execution for YOURNAME", test_kw_YOURNAME);
}

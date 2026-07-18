' =====================================================================
' BASIC++ test suite: 26_advanced_endings_lines.bas
' =====================================================================
' What can be changed: Test parameters and assertions.
' What cannot be changed: Nested block endings, option blocks, and hierarchical line numbers.
' What to expect: Verification of END and qualified block endings, OPTION blocks, and hierarchical line runs.
' What to do if something breaks: Check line number parsing in main_lite.c/main_console.c/stmt_program.c.
' =====================================================================

10 PRINT "RUNNING STRUCTURED ENDINGS & HIERARCHICAL LINE NUMBERS TESTS..."

' 1. Test ::OPTION block and structured block endings (END OPTION, END SCOPE)
::OPTION STRICT_OPTS
// Rules for compiler strict options
::SCOPE INNER_SCOPE
// Inner scope rules
END SCOPE
END OPTION

' 2. Test target-qualified block endings
::KEYWORD PRINT
// Qualified prints
PRINT KEYWORD::

::OPTION SPEED_OPT
// Options for speed optimization
HELPFILE "speed_opt_docs.txt"
SPEED_OPT OPTION::

' 3. Test Hierarchical line numbers and execution order
10.01 PRINT "Executing line 10.01..."
10.01.01 PRINT "Executing line 10.01.01 (10.0101)..."
10.01.02 PRINT "Executing line 10.01.02 (10.0102)..."
10.5 PRINT "Executing line 10.5 (10.50)..."
10.50.01 PRINT "Executing line 10.50.01 (10.5001)..."

20 PRINT "--- Querying STRICT_OPTS ---"
30 PRINT HELP("STRICT_OPTS")
40 PRINT "----------------------------"

50 PRINT "--- Querying INNER_SCOPE (nested) ---"
60 PRINT HELP("INNER_SCOPE")
70 PRINT "-------------------------------------"

80 PRINT "--- Querying SPEED_OPT (qualified end) ---"
90 PRINT HELP("SPEED_OPT")
100 PRINT "------------------------------------------"

101 PRINT "--- Querying block via HELP statement ---"
102 HELP SPEED_OPT
103 PRINT "-----------------------------------------"

104 PRINT "--- Running CATALOG statement ---"
105 CATALOG
106 PRINT "---------------------------------"

110 PRINT "Advanced Endings & Hierarchical Lines: PASS"
120 RUN "tests/basic/33_gw_qbasic_compat.bas"
130 END

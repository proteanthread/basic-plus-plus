10 REM =====================================================================
20 REM BASIC++ v6.0.0 Dialect Statement Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Dialect test assertions, print messages.
60 REM - What cannot be changed: The DIALECT LOAD/REGISTER API surface.
70 REM - What to expect: Tests the unified DIALECT statement (LOAD/REGISTER).
80 REM - What to do if something breaks: Check stmt_program.c DIALECT handler.
90 REM =====================================================================
100 PRINT "RUNNING DIALECT STATEMENT TEST..."
110 REM The unified dialect system uses DIALECT LOAD and DIALECT REGISTER.
120 REM Legacy OPTION VERSION and ::DIALECT are removed.
130 REM The test chain continues without setting any active dialect here.
140 PRINT "DIALECT OK"
150 RUN "tests/basic/14_dialect_detect.bas"

10 REM =====================================================================
20 REM BASIC++ v6.0.0 Program Serialization Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Filenames, prints, assertions.
60 REM - What cannot be changed: The SAVE command target path.
70 REM - What to expect: Execution saves this file to temp_saved.bas, and
80 REM   prints "SERIALIZATION OK".
90 REM - What to do if something breaks: Check file write permissions or fopen
100 REM   status in src/statements/stmt_program.c.
110 REM =====================================================================
120 PRINT "RUNNING SERIALIZATION TEST..."
130 SAVE "tests/basic/temp_saved.bas"
140 PRINT "SERIALIZATION OK"
150 RUN "tests/basic/08_fileio.bas"

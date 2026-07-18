10 REM =====================================================================
20 REM BASIC++ v6.0.0 Bootstrap Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Numeric equations, string values, and print messages.
60 REM - What cannot be changed: Line numbers 100-140 (which verify correct sequential flow),
70 REM   assertions for arithmetic results, and GOTO targets.
80 REM - What to expect: Execution should print test results, perform an conditional jump,
90 REM   print "BOOTSTRAP OK", and end without errors.
100 REM - What to do if something breaks: If "BOOTSTRAP OK" is not printed, verify that GOTO,
110 REM   relational checks (>), and assignment operations (%) evaluate correctly in the VM.
120 REM =====================================================================
130 PRINT "RUNNING BOOTSTRAP TEST..."
140 LET A% = 5
150 IF A% > 3 THEN GOTO 180
160 PRINT "FAILED: IF CONDITION DID NOT BRANCH"
170 END
180 PRINT "BOOTSTRAP OK"
190 RUN "tests/basic/01_loops.bas"

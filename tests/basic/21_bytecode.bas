10 REM =====================================================================
20 REM BASIC++ v6.0.0 Phase 13: Bytecode Format Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Filenames, prints.
60 REM - What cannot be changed: Test sequences and keyword assertions.
70 REM - What to expect: Execution saves and loads bytecode and verifies BRUN.
80 REM =====================================================================
90 PRINT "BYTECODE REGRESSION TEST START"
100 ON ERROR GOTO 200
110 LET R$ = "1"
120 OPEN "tests/basic/temp_run.yaml" FOR INPUT AS #1
130 INPUT #1, R$
140 CLOSE #1
150 IF R$ = "2" THEN GOTO 300
160 GOTO 250
200 REM Ignore file-not-found error on first run
210 RESUME NEXT
250 REM First run: set state to "2", then BSAVE/BLOAD/BRUN
260 OPEN "tests/basic/temp_run.yaml" FOR OUTPUT AS #1
270 PRINT #1, "2"
280 CLOSE #1
290 BSAVE "tests/basic/temp_test.bpp"
295 BLOAD "tests/basic/temp_test.bpp"
298 BRUN "tests/basic/temp_test.bpp"
299 END
300 REM Second run (BRUN completed!): reset state to "1", print success and chain
310 OPEN "tests/basic/temp_run.yaml" FOR OUTPUT AS #1
320 PRINT #1, "1"
330 CLOSE #1
340 RUN "tests/basic/22_directives_metadata.bas"
350 END

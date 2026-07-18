10 REM =====================================================================
20 REM BASIC++ v6.0.0 Security Sandbox Gating Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Prints, line numbers.
60 REM - What cannot be changed: Test structure, assertions, module loads.
70 REM - What to expect: Execution verifies function safety gates, keyword restrictions,
80 REM   and path validation checks.
90 REM - What to do if something breaks: Check error codes in eval.c and security.c.
100 REM =====================================================================
110 PRINT "RUNNING DETAILED SECURITY GATING TESTS..."
120 MODULE "MATHEXT"
130 ON ERROR GOTO 350
140 PRINT "Verifying functions are callable at OPEN level..."
150 IF TESTSYS() <> 42 THEN PRINT "FAILED: TESTSYS value": END
160 IF TESTIO() <> 84 THEN PRINT "FAILED: TESTIO value": END
170 PRINT "Functions callable: OK"
180 SECURITY "SAFE"
190 PRINT "Verifying FSAFE_SYSTEM is blocked at SAFE level..."
200 V = TESTSYS()
210 PRINT "FAILED: TESTSYS did not block at SAFE level": END
220 REM Handle blocked system function
230 PRINT "FSAFE_SYSTEM blocked: OK"
240 IF TESTIO() <> 84 THEN PRINT "FAILED: TESTIO at SAFE level": END
250 SECURITY "STANDARD"
260 PRINT "Verifying FSAFE_IO is blocked at STANDARD level..."
270 V = TESTIO()
280 PRINT "FAILED: TESTIO did not block at STANDARD level": END
290 REM Handle blocked IO function
300 PRINT "FSAFE_IO blocked: OK"
310 PRINT "Testing path traversal gating..."
320 OPEN "../sandbox_violation.txt" FOR OUTPUT AS #1
330 PRINT "FAILED: Path traversal was not blocked by security level": END
340 END
350 REM Error handler
360 IF ERR <> 70 THEN PRINT "FAILED: Expected error 70, got "; ERR: END
370 IF ERL = 200 THEN RESUME 230
380 IF ERL = 270 THEN RESUME 300
390 IF ERL = 320 THEN RESUME 410
400 PRINT "FAILED: Unexpected error at line "; ERL: END
410 PRINT "Path traversal blocked: OK"
420 PRINT "Testing dynamic keyword restriction..."
430 SECURITY "RESTRICT" "KEYWORD" "BEEP"
440 ON ERROR GOTO 480
450 BEEP
460 PRINT "FAILED: Restricted keyword BEEP was not blocked": END
470 END
480 IF ERR <> 70 THEN PRINT "FAILED: Expected error 70, got "; ERR: END
485 RESUME 490
490 PRINT "Keyword BEEP restricted: OK"
500 PRINT "Security Gating: PASS"
510 RUN "tests/basic/13_security.bas"

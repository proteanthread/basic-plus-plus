10 REM =====================================================================
20 REM BASIC++ v6.0.0 Error Trapping Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Branch targets, print statements, assertions.
60 REM - What cannot be changed: Expected error code 11 and ERL line number.
70 REM - What to expect: Execution triggers division by zero, branches to 200,
80 REM   verifies ERR (11) and ERL (150), RESUME NEXT, and computed ON GOTO/GOSUB.
90 REM - What to do if something breaks: If RESUME NEXT jumps incorrectly, check
100 REM   vm->next_pos in src/vm/vm.c. If computed GOTO fails, check index range
111 REM   in src/statements/stmt_error.c.
120 REM =====================================================================
130 PRINT "RUNNING ERROR TRAPPING TEST..."
140 ON ERROR GOTO 200
150 LET A = 1 / 0
160 GOTO 180
170 PRINT "FAILED: Error trap did not fire"
175 END
180 ON 2 GOTO 300, 310, 320
190 PRINT "FAILED: Computed GOTO fall-through"
195 END
200 IF ERR = 11 THEN GOTO 230
210 PRINT "FAILED: ERR code was "; ERR
220 END
230 IF ERL = 150 THEN GOTO 260
240 PRINT "FAILED: ERL line was "; ERL
250 END
260 RESUME NEXT
300 PRINT "FAILED: Branch target 300"
305 END
310 PRINT "COMPUTED GOTO PASSED"
320 ON 1 GOSUB 400
325 IF H = 99 THEN GOTO 350
330 PRINT "FAILED: Computed GOSUB return"
340 END
350 PRINT "ERROR TRAPPING OK"
360 RUN "tests/basic/07_serialization.bas"
400 LET H = 99
410 RETURN

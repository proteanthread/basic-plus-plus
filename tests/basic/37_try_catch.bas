10 REM =====================================================================
20 REM What can be changed: Error codes, test messages, GOSUB lines.
30 REM What cannot be changed: The exact TRY/CATCH/THROW keyword structure.
40 REM What to expect: Verification of TRY/CATCH/END TRY and THROW.
50 REM What to do if something breaks: Check structured exceptions in src/statements/stmt_try.c.
60 REM =====================================================================
70 PRINT "RUNNING TRY-CATCH EXCEPTION REGRESSION TESTS..."
80 REM Test 1: Simple throw and catch
90 TRY
100   PRINT "Inside TRY block"
110   THROW 42, "My Custom Error"
120   PRINT "This line should NOT print!"
130 CATCH
140   PRINT "Caught error code:"; ERR
150   PRINT "Caught message: "; ERR$
160   IF ERR <> 42 THEN PRINT "FAILED simple throw code check": END
170   IF ERR$ <> "My Custom Error" THEN PRINT "FAILED simple throw message check": END
180 END TRY
190 PRINT "Passed Simple TRY-CATCH"
200 REM Test 2: TRY-CATCH with normal pass-through
210 TRY
220   PRINT "Inside normal TRY block"
230 CATCH
240   PRINT "This CATCH should NOT run!"
250   END
260 END TRY
270 PRINT "Passed normal pass-through"
280 REM Test 3: TRY-CATCH crossing GOSUB boundary
290 TRY
300   GOSUB 400
310 CATCH
320   PRINT "Caught error from GOSUB! Code:"; ERR
330   IF ERR <> 99 THEN PRINT "FAILED GOSUB exception propagation": END
340 END TRY
350 PRINT "Passed GOSUB propagation"
360 PRINT "TRY-CATCH OK"
370 RUN "tests/basic/38_rpn_stack_ops.bas"
400 REM Subroutine called from inside TRY
410 PRINT "Inside subroutine, about to throw..."
420 THROW 99, "GOSUB Exception"
430 RETURN

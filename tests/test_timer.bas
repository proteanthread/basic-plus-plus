10 REM test_timer.bas
20 REM ================================================================
30 REM What can be changed: Loop count limits, print messages.
40 REM What cannot be changed: GOSUB statement targets, event trapping commands.
50 REM What to expect: TIMER function returns seconds since midnight, ON TIMER fires GOSUB.
60 REM What to do if something breaks: Verify platform timer clock speed.
70 REM ================================================================
80 PRINT "--- STARTING TIMER AND EVENT TRAP TEST ---"
90 T1 = TIMER
100 PRINT "Current midnight seconds: "; T1
110 ON TIMER(1) GOSUB 220
120 TIMER ON
130 PRINT "Waiting for timer event..."
140 T2 = TIMER
150 IF TIMER - T2 < 2.5 THEN GOTO 150
160 TIMER OFF
170 PRINT "Timer turned off."
180 PRINT "--- TEST COMPLETED SUCCESSFULLY ---"
190 END
200 REM --- TIMER EVENT HANDLER ---
210 REM This is the GOSUB target
220 PRINT ">>> TIMER EVENT TRIGGERED at "; TIMER; " <<<"
230 RETURN

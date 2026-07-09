1 REM =======================================================================
2 REM Goal: Test standard console printing.
3 REM Expected behavior: Prints lines to the screen. Simulated input cancels LPT1.
4 REM Constraints: Automated regression test.
5 REM Troubleshooting: If this fails, check PRINT statement implementation.
6 REM =======================================================================
10 REM PRINTTEST.BAS  - screen and printer demo
20 PRINT "PRINT TEST — Screen output"
30 PRINT "Press ENTER to send a test line to printer (LPT1)."
35 REM Simulated Input (cancels to avoid PRINT #1 without OPEN error)
40 REM INPUT A$
45 READ A$: PRINT A$
50 IF A$="" THEN PRINT #1,"Printer test from BASIC":PRINT "Sent to LPT1.":GOTO 70
60 PRINT "Cancelled."
70 PRINT "Printing 10 lines to screen..."
80 FOR I=1 TO 10
90 PRINT "Line ";I;"  Hello from BASIC"
100 NEXT I
110 PRINT "DONE."
1000 DATA "C"

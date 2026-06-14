10 REM PRINTTEST.BAS  - screen and printer demo
20 PRINT "PRINT TEST — Screen output"
30 PRINT "Press ENTER to send a test line to printer (LPT1)."
40 INPUT A$
50 IF A$="" THEN PRINT #1,"Printer test from BASIC":PRINT "Sent to LPT1.":GOTO 70
60 PRINT "Cancelled."
70 PRINT "Printing 10 lines to screen..."
80 FOR I=1 TO 10
90 PRINT "Line ";I;"  Hello from BASIC"
100 NEXT I
110 PRINT "DONE."

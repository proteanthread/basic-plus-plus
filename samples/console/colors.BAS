10 REM =======================================================================
20 REM Console Colors & Formatting Demo
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM I, J, BG, FG
60 CLS
70 PRINT "============================================================"
80 PRINT "  ANSI Colors & Text Formatting Demo"
90 PRINT "============================================================"
100 PRINT
110 PRINT "  BASIC++ supports standard 16-color ANSI output via COLOR."
120 PRINT
130 FOR BG = 0 TO 7
140   FOR FG = 0 TO 15
150     COLOR FG, BG
160     PRINT " ["; FG; ","; BG; "] ";
170   NEXT FG
180   COLOR 7, 0
190   PRINT
200 NEXT BG
210 PRINT
220 COLOR 14, 0: PRINT "  Notice how bright backgrounds are supported where terminals allow."
230 COLOR 7, 0: PRINT
240 PRINT "  Press ENTER to return..."
250 INPUT "", I$
260 CHAIN "samples/console/menu.bas"

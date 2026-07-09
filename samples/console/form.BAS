10 REM =======================================================================
20 REM Console Interactive Form Demo
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM N$, A, CITY$
60 CLS
70 PRINT "============================================================"
80 PRINT "  Interactive Console Form"
90 PRINT "============================================================"
100 PRINT
110 COLOR 11, 0: PRINT "  Please fill out the following information:"
120 COLOR 7, 0: PRINT
130 INPUT "  Name: ", N$
140 INPUT "  Age:  ", A
150 INPUT "  City: ", CITY$
160 PRINT
170 COLOR 10, 0: PRINT "  Thank you! Here is what you entered:"
180 COLOR 7, 0: PRINT "  ------------------------------------"
190 PRINT "  Name: "; N$
200 PRINT "  Age:  "; A
210 PRINT "  City: "; CITY$
220 PRINT
230 COLOR 14, 0: PRINT "  Press ENTER to return..."
240 INPUT "", N$
250 CHAIN "samples/console/menu.bas"

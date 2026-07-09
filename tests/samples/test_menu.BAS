1 REM =======================================================================
2 REM Goal: Test simple menu with subroutines and standard control flow.
3 REM Expected behavior: Should present a menu, navigate to selected subroutines,
4 REM handle invalid input, and gracefully exit on Q.
5 REM Constraints: Cannot be changed without breaking regression tests.
6 REM Troubleshooting: If this fails, check IF/THEN, GOSUB/RETURN, and RND behavior.
7 REM =======================================================================
10 REM MENU.BAS - simple menu with subroutines
20 DEFINT A-Z
25 RANDOMIZE 1
30 CLS
40 DO
42   GOSUB 200
45   REM Simulated Input
50   REM INPUT "Choose (1) Hello (2) FileIO (3) Graphics (Q) Quit: ";C$
55   PRINT "Choose (1) Hello (2) FileIO (3) Graphics (Q) Quit: ";: READ C$: PRINT C$
60   SELECT CASE UCASE$(C$)
62     CASE "Q"
63     EXIT DO
65     CASE "1"
66     GOSUB 100
70     CASE "2"
71     GOSUB 110
75     CASE "3"
76     GOSUB 120
80     CASE ELSE
81     PRINT "Invalid choice."
85   END SELECT
90 LOOP
95 END
100 PRINT "Hello! This is a menu demo.":PRINT: RETURN
110 PRINT "This would run file IO demo (not chained).":PRINT: RETURN
120 PRINT "Graphics demo will run...":GOSUB 300:RETURN
200 REM DRAW HEADER
205 CLS
210 PRINT "=== BASIC SAMPLE MENU ===":PRINT
220 RETURN
300 REM SIMPLE GRAPHICS (uses SCREEN coords if supported)
310 CLS
320 FOR Y=1 TO 20
330 FOR X=1 TO 40
340 IF RND(1)<0.05 THEN PRINT "*"; ELSE PRINT " ";
350 NEXT X
360 PRINT
370 NEXT Y
380 PRINT: RETURN
1000 DATA "1", "2", "3", "4", "Q"

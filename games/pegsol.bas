10 REM =============================================
20 REM  PEG SOLITAIRE - BASIC++ Puzzle Game
30 REM  Arrow keys move cursor, SPACE select/jump
40 REM  Remove all pegs but one to win!
50 REM =============================================
60 CURSOR OFF
70 REM --- Board layout ---
80 REM English cross pattern (7x7)
90 REM 0=invalid, 1=peg, 2=empty hole
100 DIM BD(7, 7)
110 REM Row 1:     . . X X X . .
120 REM Row 2:     . . X X X . .
130 REM Row 3:     X X X X X X X
140 REM Row 4:     X X X . X X X
150 REM Row 5:     X X X X X X X
160 REM Row 6:     . . X X X . .
170 REM Row 7:     . . X X X . .
180 FOR R = 1 TO 7
190   FOR C = 1 TO 7
200     BD(R, C) = 0
210   NEXT C
220 NEXT R
230 REM Fill cross pattern
240 FOR R = 1 TO 7
250   FOR C = 1 TO 7
260     VALID = 0
270     IF R >= 3 AND R <= 5 THEN VALID = 1
280     IF C >= 3 AND C <= 5 THEN VALID = 1
290     IF VALID = 1 THEN BD(R, C) = 1
300   NEXT C
310 NEXT R
320 REM Center hole
330 BD(4, 4) = 2
340 PEGS = 32: MOVES = 0
350 CX = 4: CY = 4
360 SEL.R = 0: SEL.C = 0
370 REM
380 REM --- Draw screen ---
390 CLS
400 COLOR 14, 0
410 LOCATE 1, 25: PRINT "P E G   S O L I T A I R E"
420 COLOR 8, 0
430 LOCATE 2, 1: PRINT STRING$(80, CHR$(196));
440 COLOR 15, 0
450 LOCATE 3, 5: PRINT "Jump pegs over each other to remove them."
460 LOCATE 4, 5: PRINT "Goal: Leave only 1 peg remaining!"
470 LOCATE 5, 5: PRINT "Arrow keys=Move  SPACE=Select/Jump  R=Reset  ESC=Quit"
480 COLOR 8, 0
490 LOCATE 6, 1: PRINT STRING$(80, CHR$(196));
500 REM
510 REM --- Draw board ---
520 GOSUB 1000
530 REM
540 REM ========== MAIN GAME LOOP ==========
550 K$ = INKEY$
560 IF K$ = "" THEN 550
570 IF K$ = CHR$(27) THEN 2000
580 IF LEN(K$) = 2 THEN SK = ASC(MID$(K$, 2, 1)) ELSE SK = 0
590 IF SK = 72 AND CY > 1 THEN CY = CY - 1
600 IF SK = 80 AND CY < 7 THEN CY = CY + 1
610 IF SK = 75 AND CX > 1 THEN CX = CX - 1
620 IF SK = 77 AND CX < 7 THEN CX = CX + 1
630 REM Skip invalid squares
640 IF BD(CY, CX) = 0 THEN GOTO 550
650 REM Select or jump
660 IF K$ = " " THEN GOSUB 1200
670 IF K$ = "r" OR K$ = "R" THEN GOTO 180
680 GOSUB 1000
690 REM Check win/lose
700 IF PEGS = 1 THEN 1800
710 REM Check if any moves remain
720 CANMOVE = 0
730 FOR R = 1 TO 7
740   FOR C = 1 TO 7
750     IF BD(R, C) <> 1 THEN 790
760     REM Check 4 directions
770     IF R >= 3 AND BD(R-1,C) = 1 AND BD(R-2,C) = 2 THEN CANMOVE = 1
780     IF R <= 5 AND BD(R+1,C) = 1 AND BD(R+2,C) = 2 THEN CANMOVE = 1
785     IF C >= 3 AND BD(R,C-1) = 1 AND BD(R,C-2) = 2 THEN CANMOVE = 1
786     IF C <= 5 AND BD(R,C+1) = 1 AND BD(R,C+2) = 2 THEN CANMOVE = 1
790   NEXT C
800 NEXT R
810 IF CANMOVE = 0 AND PEGS > 1 THEN 1900
820 GOTO 550
830 REM
1000 REM === Draw board ===
1010 BX0 = 25: BY0 = 8
1020 FOR R = 1 TO 7
1030   FOR C = 1 TO 7
1040     LOCATE BY0 + R * 2, BX0 + C * 4
1050     IF BD(R, C) = 0 THEN PRINT "    ";: GOTO 1090
1060     IF BD(R, C) = 1 THEN
1070       IF R = SEL.R AND C = SEL.C THEN COLOR 14, 0 ELSE COLOR 11, 0
1080       PRINT " "; CHR$(15); " ";
1090     END IF
1100     IF BD(R, C) = 2 THEN COLOR 8, 0: PRINT " "; CHR$(250); " ";
1110   NEXT C
1120 NEXT R
1130 REM Draw cursor
1140 IF BD(CY, CX) > 0 THEN
1150   COLOR 15, 0
1160   LOCATE BY0 + CY * 2, BX0 + CX * 4
1170   PRINT "[";
1180   LOCATE BY0 + CY * 2, BX0 + CX * 4 + 3
1190   PRINT "]";
1200 END IF
1210 REM Status bar
1220 COLOR 15, 0
1230 LOCATE 24, 5: PRINT "Pegs: "; PEGS; "   Moves: "; MOVES; "   ";
1240 RETURN
1250 REM
1200 REM === Select / Jump ===
1210 IF SEL.R = 0 THEN
1220   REM Select a peg
1230   IF BD(CY, CX) = 1 THEN SEL.R = CY: SEL.C = CX
1240   RETURN
1250 END IF
1260 REM Try to jump
1270 DR = CY - SEL.R: DC = CX - SEL.C
1280 REM Must be exactly 2 squares away in one direction
1290 IF ABS(DR) = 2 AND DC = 0 THEN
1300   MR = SEL.R + DR / 2: MC = SEL.C
1310   GOTO 1350
1320 END IF
1330 IF ABS(DC) = 2 AND DR = 0 THEN
1340   MR = SEL.R: MC = SEL.C + DC / 2
1350   REM Check valid jump
1360   IF BD(MR, MC) = 1 AND BD(CY, CX) = 2 THEN
1370     BD(SEL.R, SEL.C) = 2
1380     BD(MR, MC) = 2
1390     BD(CY, CX) = 1
1400     PEGS = PEGS - 1: MOVES = MOVES + 1
1410     SOUND 800, 1
1420   END IF
1430   GOTO 1450
1440 END IF
1450 REM Deselect
1460 SEL.R = 0: SEL.C = 0
1470 RETURN
1800 REM === YOU WIN ===
1810 CLS
1820 COLOR 14, 0
1830 LOCATE 10, 20: PRINT "C O N G R A T U L A T I O N S !"
1840 LOCATE 12, 20: PRINT "You solved it in"; MOVES; "moves!"
1850 IF BD(4, 4) = 1 THEN
1860   LOCATE 14, 20: PRINT "PERFECT! Last peg in the center!"
1870 END IF
1880 LOCATE 18, 20: PRINT "Press any key..."
1890 K$ = "": WHILE K$ = "": K$ = INKEY$: WEND
1895 CLS: END
1900 REM === NO MORE MOVES ===
1910 COLOR 12, 0
1920 LOCATE 24, 5
1930 PRINT "No more moves! "; PEGS; " pegs remain. Press R to reset.";
1940 GOTO 550
2000 REM === QUIT ===
2010 CURSOR ON: CLS
2020 PRINT "Thanks for playing Peg Solitaire!"
2030 PRINT "Final: "; PEGS; " pegs in "; MOVES; " moves"
2040 END

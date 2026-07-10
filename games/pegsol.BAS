10 REM =============================================
REM ==================================================
REM  EXHAUSTIVE DOCUMENTATION
REM ==================================================
REM  This file is a standalone Game Engine for BASIC++.
REM  It operates strictly within the text UI (TUI) and relies on external
REM  mechanics libraries for its game loop. 
REM  Key Components:
REM  - Initialization: Sets up the visual buffer and arrays.
REM  - Main Loop: Uses a non-blocking INKEY$ loop synced with the TICKS timer.
REM  - Collision/Movement: Delegated to the imported library.
REM  - Save/Load: Invoked via GOSUB 8000 and 8100, saving to .sav files.
REM ==================================================
20 REM  PEG SOLITAIRE ENGINE
30 REM  Uses BOARDGAME.LIB for mechanics.
40 REM  Arrow keys move cursor, SPACE select/jump
50 REM  Press S to Save, L to Load
60 REM =============================================
70 LOAD LIBRARY "lib\boardgame.LIB"
80 PRINT CHR$(27);"[?25l";
90 DIM BD(7, 7)
100 GOSUB 3000
110 PEGS = 32: MOVES = 0: CX = 4: CY = 4: SEL_R = 0: SEL_C = 0
120 CLS: COLOR 14, 0: LOCATE 1, 25: PRINT "P E G   S O L I T A I R E"
130 COLOR 8, 0: LOCATE 2, 1: PRINT STRING$(80, CHR$(196));
140 COLOR 15, 0: LOCATE 3, 5: PRINT "Jump pegs over each other to remove them."
150 LOCATE 4, 5: PRINT "Goal: Leave only 1 peg remaining!"
160 LOCATE 5, 5: PRINT "Arrow keys=Move  SPACE=Select/Jump  R=Reset  ESC=Quit"
170 COLOR 8, 0: LOCATE 6, 1: PRINT STRING$(80, CHR$(196));
180 GOSUB 1000
190 REM ========== MAIN GAME LOOP ==========
200 K$ = INKEY$: IF K$ = "" THEN 200
210 IF K$ = CHR$(27) THEN 2000
220 IF UCASE$(K$) = "S" THEN GOSUB 8000: GOTO 200
230 IF UCASE$(K$) = "L" THEN GOSUB 8100: GOTO 200
240 IF UCASE$(K$) = "R" THEN GOSUB 3000: PEGS = 32: MOVES = 0: CX = 4: CY = 4: SEL_R = 0: SEL_C = 0: GOSUB 1000: GOTO 200
250 SK = 0 : IF LEN(K$) = 2 THEN SK = ASC(MID$(K$, 2, 1))
260 CALL BG_MOVE_CURSOR(SK, 1, 7, 1, 7)
270 IF BD(CY, CX) = 0 THEN 200
280 IF K$ = " " THEN GOSUB 1200
290 GOSUB 1000
300 IF PEGS = 1 THEN 1800
310 CANMOVE = 0
320 FOR R = 1 TO 7
330   FOR C = 1 TO 7
340     IF BD(R, C) = 1 THEN
350       IF R >= 3 THEN IF BD(R-1,C) = 1 AND BD(R-2,C) = 2 THEN CANMOVE = 1
360       IF R <= 5 THEN IF BD(R+1,C) = 1 AND BD(R+2,C) = 2 THEN CANMOVE = 1
370       IF C >= 3 THEN IF BD(R,C-1) = 1 AND BD(R,C-2) = 2 THEN CANMOVE = 1
380       IF C <= 5 THEN IF BD(R,C+1) = 1 AND BD(R,C+2) = 2 THEN CANMOVE = 1
390     END IF
400   NEXT C
410 NEXT R
420 IF CANMOVE = 0 AND PEGS > 1 THEN 1900
430 GOTO 200
1000 REM === Draw board ===
1010 BRD_X = 25: BRD_Y = 8
1020 FOR R = 1 TO 7
1030   FOR C = 1 TO 7
1040     LOCATE BRD_Y + R * 2, BRD_X + C * 4
1050     IF BD(R, C) = 0 THEN PRINT "    ";: GOTO 1090
1060     IF BD(R, C) = 1 THEN COLOR 11, 0 : IF R = SEL_R AND C = SEL_C THEN COLOR 14, 0
1070     IF BD(R, C) = 1 THEN PRINT " "; CHR$(15); " ";
1080     IF BD(R, C) = 2 THEN COLOR 8, 0: PRINT " "; CHR$(250); " ";
1090   NEXT C
1100 NEXT R
1110 IF BD(CY, CX) > 0 THEN COLOR 15, 0: LOCATE BRD_Y + CY * 2, BRD_X + CX * 4: PRINT "[";: LOCATE BRD_Y + CY * 2, BRD_X + CX * 4 + 3: PRINT "]";
1120 COLOR 15, 0: LOCATE 24, 5: PRINT "Pegs: "; PEGS; "   Moves: "; MOVES; "   ";
1130 RETURN
1200 REM === Select / Jump ===
1210 IF SEL_R = 0 THEN IF BD(CY, CX) = 1 THEN SEL_R = CY: SEL_C = CX: RETURN
1220 IF SEL_R = 0 THEN RETURN
1230 DR = CY - SEL_R: DC = CX - SEL_C
1240 IF ABS(DR) = 2 AND DC = 0 THEN MR = SEL_R + DR / 2: MC = SEL_C: GOTO 1300
1250 IF ABS(DC) = 2 AND DR = 0 THEN MR = SEL_R: MC = SEL_C + DC / 2: GOTO 1300
1260 GOTO 1340
1300 IF BD(MR, MC) = 1 AND BD(CY, CX) = 2 THEN BD(SEL_R, SEL_C) = 2: BD(MR, MC) = 2: BD(CY, CX) = 1: PEGS = PEGS - 1: MOVES = MOVES + 1: SOUND 800, 1
1340 SEL_R = 0: SEL_C = 0
1350 RETURN
1800 REM === YOU WIN ===
1810 CLS: COLOR 14, 0: LOCATE 10, 20: PRINT "C O N G R A T U L A T I O N S !"
1820 LOCATE 12, 20: PRINT "You solved it in"; MOVES; "moves!"
1830 IF BD(4, 4) = 1 THEN LOCATE 14, 20: PRINT "PERFECT! Last peg in the center!"
1840 LOCATE 18, 20: PRINT "Press any key...": GOSUB 9990
1850 CLS: END
1900 REM === NO MORE MOVES ===
1910 COLOR 12, 0: LOCATE 24, 5: PRINT "No more moves! "; PEGS; " pegs remain. Press R to reset.";: GOTO 200
2000 REM === QUIT ===
2010 CURSOR ON: CLS: PRINT "Thanks for playing Peg Solitaire!": PRINT "Final: "; PEGS; " pegs in "; MOVES; " moves": END
3000 REM === Init Board ===
3010 FOR R = 1 TO 7
3020   FOR C = 1 TO 7
3030     BD(R, C) = 0: IF (R >= 3 AND R <= 5) OR (C >= 3 AND C <= 5) THEN BD(R, C) = 1
3040   NEXT C
3050 NEXT R
3060 BD(4, 4) = 2: RETURN
8000 REM === Save Game ===
8010 OPEN "pegsol.sav" FOR OUTPUT AS #1
8020 PRINT #1, PEGS: PRINT #1, MOVES: FOR R = 1 TO 7: FOR C = 1 TO 7: PRINT #1, BD(R, C): NEXT C: NEXT R
8030 CLOSE #1: COLOR 14, 0: LOCATE 12, 30: PRINT "GAME SAVED!": DELAY 1000: GOSUB 1000: RETURN
8100 REM === Load Game ===
8110 IF EXIST("pegsol.sav") THEN
8120   OPEN "pegsol.sav" FOR INPUT AS #1
8130   INPUT #1, PEGS: INPUT #1, MOVES: FOR R = 1 TO 7: FOR C = 1 TO 7: INPUT #1, BD(R, C): NEXT C: NEXT R
8140   CLOSE #1: GOSUB 1000
8150 END IF
8160 RETURN

9990 K$ = INKEY$: IF K$ = "" THEN 9990 ELSE RETURN
9992 K$ = INKEY$: IF K$ <> " " THEN 9992 ELSE RETURN
9994 K$ = INKEY$: IF K$ <> "r" AND K$ <> "R" THEN 9994 ELSE RETURN

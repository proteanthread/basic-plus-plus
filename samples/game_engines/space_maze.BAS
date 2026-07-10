10 REM ==================================================
20 REM  SPACE MAZE - Sample Mix & Match Game Engine
30 REM  Uses: MAZE.LIB (grid mechanics) + VFX.LIB (stars)
40 REM  Objective: Navigate the maze while stars fly by.
50 REM ==================================================
60 LOAD LIBRARY "lib\maze.LIB"
70 LOAD LIBRARY "lib\vfx.LIB"
80 RANDOMIZE TIMER
90 PRINT CHR$(27);"[?25l";
100 REM --- Init VFX Starfield ---
110 CONST SS_STARS = 40: CONST SS_DEPTH = 32
120 CALL VFX_INIT_STARFIELD(SS_STARS, SS_DEPTH)
130 REM --- Init Maze ---
140 MW = 38: MH = 22
150 CALL MZ_INIT_BOARD(MW, MH)
160 REM Generate random walls
170 FOR I = 1 TO (MW * MH) / 4
180   WX = INT(RND(1) * MW) + 1: WY = INT(RND(1) * MH) + 1
190   CALL MZ_SET_WALL(WX, WY)
200 NEXT I
210 REM Make sure start is open
220 MZ_GRID(2, 2) = 0
230 PLAYER_ID = 1: PX = 2: PY = 2
240 CALL MZ_MOVE_ENT(PLAYER_ID, PX, PY)
250 CLS
260 COLOR 9, 0
270 FOR Y = 1 TO MH
280   FOR X = 1 TO MW
290     IF MZ_GRID(X, Y) = 1 THEN LOCATE Y + 1, X * 2: PRINT CHR$(219); CHR$(219);
300   NEXT X
310 NEXT Y
320 REM --- Main Loop ---
330 GFT = TICKS
340 REM Draw Stars (offset by 40 X to draw on the right half)
350 CALL VFX_UPDATE_STARS(0.5, 60, 12, 80, 24)
360 REM Draw Player
370 LOCATE PY + 1, PX * 2
380 COLOR 14, 0: PRINT CHR$(2); " ";
390 REM Input
400 K$ = INKEY$
410 IF K$ = CHR$(27) THEN 1000
420 SK = 0: IF LEN(K$) = 2 THEN SK = ASC(MID$(K$, 2, 1))
430 NX = PX: NY = PY
440 IF SK = 72 THEN NY = PY - 1
450 IF SK = 80 THEN NY = PY + 1
460 IF SK = 75 THEN NX = PX - 1
470 IF SK = 77 THEN NX = PX + 1
480 IF NX <> PX OR NY <> PY THEN
490   IF NX >= 1 AND NX <= MW AND NY >= 1 AND NY <= MH THEN
500     IF MZ_GRID(NX, NY) = 0 THEN
510       REM Erase old player
520       LOCATE PY + 1, PX * 2: PRINT "  ";
530       PX = NX: PY = NY
540     END IF
550   END IF
560 END IF
570 GT2 = TICKS: GT1 = GT2 - GFT
580 IF GT1 < 50 THEN DELAY 50 - GT1
590 GOTO 330
1000 PRINT CHR$(27);"[?25h";: CLS: PRINT "Exiting Space Maze.": END

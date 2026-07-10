10 REM ==================================================
20 REM  RPG SHOOTER - Sample Mix & Match Game Engine
30 REM  Uses: RPG_PROCGEN.LIB (map) + ARCADE_SHOOTER.LIB (combat)
40 REM  Objective: Shoot enemies inside a procedurally generated cave!
50 REM ==================================================
60 LOAD LIBRARY "lib\rpg_procgen.LIB"
70 LOAD LIBRARY "lib\arcade_shooter.LIB"
80 RANDOMIZE TIMER
90 PRINT CHR$(27);"[?25l";: CLS
100 REM --- Generate Cave Map ---
110 MW = 80: MH = 22
120 CALL RPG_GEN_GRID(MW, MH, 45)
130 REM Draw the cave once
140 COLOR 8, 0
150 FOR Y = 1 TO MH
160   FOR X = 1 TO MW
170     IF RPG_GRID(X, Y) = 1 THEN LOCATE Y + 2, X: PRINT CHR$(178);
180   NEXT X
190 NEXT Y
200 REM --- Init Shooter Mechanics ---
210 MAX_OBJ = 10
220 CALL AS_INIT_VECTORS(MAX_OBJ)
230 REM Player is ID 1
240 PX = 40: PY = 12
250 REM Ensure player starts in open space
260 RPG_GRID(PX, PY) = 0
270 REM Main loop
280 GFT = TICKS
290 LOCATE PY + 2, PX: COLOR 11, 0: PRINT CHR$(2);
300 K$ = INKEY$
310 IF K$ = CHR$(27) THEN 1000
320 SK = 0: IF LEN(K$) = 2 THEN SK = ASC(MID$(K$, 2, 1))
330 NX = PX: NY = PY
340 IF SK = 72 THEN NY = PY - 1
350 IF SK = 80 THEN NY = PY + 1
360 IF SK = 75 THEN NX = PX - 1
370 IF SK = 77 THEN NX = PX + 1
380 IF NX <> PX OR NY <> PY THEN
390   IF NX >= 1 AND NX <= MW AND NY >= 1 AND NY <= MH THEN
400     IF RPG_GRID(NX, NY) = 0 THEN
410       LOCATE PY + 2, PX: PRINT " ";
420       PX = NX: PY = NY
430     END IF
440   END IF
450 END IF
460 REM In a full game, here we would spawn AS_ objects and move them!
470 GT2 = TICKS: GT1 = GT2 - GFT
480 IF GT1 < 50 THEN DELAY 50 - GT1
490 GOTO 280
1000 PRINT CHR$(27);"[?25h";: CLS: PRINT "Exiting RPG Shooter.": END

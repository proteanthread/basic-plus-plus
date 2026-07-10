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
20 REM  STARFIELD SCREENSAVER ENGINE
30 REM  Uses VFX.LIB for effects.
40 REM  Moving 3D starfield effect that integrates
50 REM  with the TUI desktop as an idle-timeout TSR.
60 REM  Press S to Save (dummy), ESC to Quit
70 REM =============================================
80 LOAD LIBRARY "lib\vfx.LIB"
90 GOSUB 10000
100 END
10000 REM  SCREENSAVER ENGINE
10010 CONST SS_STARS = 80: CONST SS_W = 80: CONST SS_H = 25
10020 CONST SS_CX = 40: CONST SS_CY = 12: CONST SS_DEPTH = 32
10030 CONST SS_SPEED = 0.5: CONST SS_FPS = 50
10040 DIM SS_BUF(2100)
10050 SCREENSAVE 1, 1, 80, 25, SS_BUF()
10060 PRINT CHR$(27);"[?25l";: CLS
10070 CALL VFX_INIT_STARFIELD(SS_STARS, SS_DEPTH)
10080 GFT = TICKS
10090 CALL VFX_UPDATE_STARS(SS_SPEED, SS_CX, SS_CY, SS_W, SS_H)
10100 K$ = INKEY$
10110 IF UCASE$(K$) = "S" THEN GOSUB 8000
10120 IF K$ <> "" AND UCASE$(K$) <> "S" THEN GOTO 10160
10130 GT2 = TICKS: GT1 = GT2 - GFT
10140 IF GT1 < SS_FPS THEN DELAY SS_FPS - GT1
10150 GFT = TICKS: GOTO 10090
10160 SCREENRESTORE SS_BUF()
10170 PRINT CHR$(27);"[?25h";
10180 RETURN
8000 REM === Save Game ===
8010 OPEN "starfield.sav" FOR OUTPUT AS #1
8020 PRINT #1, "SAVED"
8030 CLOSE #1
8040 COLOR 14, 0: LOCATE 12, 30: PRINT "STATE SAVED!": DELAY 1000: CLS: RETURN

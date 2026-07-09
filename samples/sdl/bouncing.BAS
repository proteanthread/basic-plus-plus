10 REM =======================================================================
20 REM SDL Bouncing Ball Demo
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM X, Y, DX, DY, R, C, K$
60 SCREEN 12 ' 640x480 Graphics Mode
70 CLS
80 X = 320: Y = 240
90 DX = 4: DY = 3
100 R = 20: C = 9 ' Light blue ball
110 COLOR 15, 0
120 LOCATE 1, 1: PRINT " Bouncing Ball Demo (SCREEN 12)"
130 LOCATE 2, 1: PRINT " Press Q or ESC to quit."
140 DO
150   ' Erase old ball
160   CIRCLE (X, Y), R, 0
170   ' Move ball
180   X = X + DX
190   Y = Y + DY
200   ' Bounce logic
210   IF X - R <= 0 OR X + R >= 639 THEN DX = -DX: C = INT(RND(1) * 14) + 1
220   IF Y - R <= 0 OR Y + R >= 479 THEN DY = -DY: C = INT(RND(1) * 14) + 1
230   ' Draw new ball
240   CIRCLE (X, Y), R, C
250   ' Wait for VSYNC / delay
260   SLEEP 0.016
270   ' Check input
280   K$ = UCASE$(INKEY$)
290   IF K$ = "Q" OR K$ = CHR$(27) THEN EXIT DO
300 LOOP
310 SCREEN 0 ' Return to text mode
320 CHAIN "samples/sdl/menu.bas"

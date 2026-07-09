10 REM =======================================================================
20 REM SDL Drawing & Primitives Demo
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM I, K$
60 SCREEN 12 ' 640x480 Graphics Mode
70 CLS
80 LOCATE 1, 1: COLOR 15, 0: PRINT " SDL Drawing & Primitives Demo (SCREEN 12)"
90 LOCATE 2, 1: PRINT " Press any key to quit."
100 ' Draw some lines
110 FOR I = 1 TO 10
120   LINE (50, 50 + I * 10)-(200, 50 + I * 10), I
130 NEXT I
140 ' Draw some boxes
150 FOR I = 1 TO 5
160   LINE (250 + I * 10, 50 + I * 10)-(400 - I * 10, 200 - I * 10), I + 8, B
170 NEXT I
180 ' Draw some filled boxes
190 FOR I = 1 TO 5
200   LINE (450 + I * 10, 50 + I * 10)-(600 - I * 10, 200 - I * 10), I + 1, BF
210 NEXT I
220 ' Draw some circles
230 FOR I = 1 TO 10
240   CIRCLE (125, 350), I * 10, I + 2
250 NEXT I
260 ' Draw intersecting lines
270 FOR I = 1 TO 100 STEP 5
280   LINE (250, 250 + I)-(400, 400 - I), INT(RND(1) * 15) + 1
290 NEXT I
300 ' Wait for input
310 DO
320   K$ = INKEY$
330   IF K$ <> "" THEN EXIT DO
340   SLEEP 0.1
350 LOOP
360 SCREEN 0 ' Return to text mode
370 CHAIN "samples/sdl/menu.bas"

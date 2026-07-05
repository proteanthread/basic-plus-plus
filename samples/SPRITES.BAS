10 REM ============================================================
11 REM  SPRITES.BAS - Move @ Sprite with Arrow Keys
12 REM ============================================================
13 REM
14 REM  WHAT CAN BE CHANGED:
15 REM   - Sprite character (@), screen dimensions (80x24)
16 REM   - Movement keys (WASD or arrow keys)
17 REM
18 REM  WHAT CANNOT BE CHANGED:
19 REM   - Do NOT use IF/THEN PRINT...;ELSE PRINT...;
20 REM     on a single line (parser limitation)
21 REM   - Use variable-based print instead
22 REM
23 REM  WHAT TO EXPECT:
24 REM   - Full-screen display with movable @ character
25 REM   - WASD keys move, ESC quits
26 REM
27 REM  TROUBLESHOOTING:
28 REM   - WHAT? on PRINT;ELSE: use variable approach
29 REM   - No movement: check INKEY$ key codes
30 REM ============================================================
40 CLS
50 X=20:Y=10
60 PRINT CHR$(27);"[2J";
70 GOSUB 200
80 K$=INKEY$
90 IF K$<>"" THEN GOSUB 300
100 GOTO 80
200 REM DRAW
210 CLS
220 FOR R=1 TO 24
230 FOR C=1 TO 80
240 CH$=" "
250 IF R=Y AND C=X THEN CH$="@"
260 PRINT CH$;
270 NEXT C
280 PRINT
290 NEXT R
295 RETURN
300 REM KEYS (WASD movement)
310 IF K$="w" OR K$="W" THEN Y=Y-1:GOTO 360
320 IF K$="s" OR K$="S" THEN Y=Y+1:GOTO 360
330 IF K$="a" OR K$="A" THEN X=X-1:GOTO 360
340 IF K$="d" OR K$="D" THEN X=X+1:GOTO 360
350 IF K$=CHR$(27) THEN END
360 IF X<1 THEN X=1
370 IF X>80 THEN X=80
380 IF Y<1 THEN Y=1
390 IF Y>24 THEN Y=24
400 RETURN

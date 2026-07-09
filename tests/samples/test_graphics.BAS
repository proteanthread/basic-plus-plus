10 REM ============================================================
11 REM  GRAPHICS.BAS - Simple Text-Mode Graphics Demo
12 REM ============================================================
13 REM
14 REM  WHAT CAN BE CHANGED:
15 REM   - Box dimensions (R loop 1-12, C loop 1-40)
16 REM   - Characters used for drawing (* and space)
17 REM   - Animation delay (DEL loop count)
18 REM
19 REM  WHAT CANNOT BE CHANGED:
20 REM   - Do NOT use IF/THEN PRINT...;ELSE PRINT...;
21 REM     pattern on a single line (parser limitation)
22 REM   - Use GOSUB-based print or IF/GOTO alternatives
23 REM
24 REM  WHAT TO EXPECT:
25 REM   - Draws a box with diagonal, then animates
26 REM   - Waits for keypress between scenes
27 REM
28 REM  TROUBLESHOOTING:
29 REM   - WHAT? on PRINT;ELSE: split IF/THEN/ELSE across
30 REM     multiple lines or use IF/GOTO approach
31 REM ============================================================
40 CLS
50 PRINT "Text-mode box and diagonal (suitable for GW-BASIC/BASICA):"
60 FOR R=1 TO 12
70 FOR C=1 TO 40
80 CH$=" "
90 IF R=1 OR R=12 OR C=1 OR C=40 OR R=C/3 THEN CH$="*"
100 PRINT CH$;
110 NEXT C
120 PRINT
130 NEXT R
140 PRINT
150 PRINT "Press any key to show animated diagonal..."
155 REM Simulated INKEY$
160 REM A$=INKEY$
165 READ A$: PRINT A$
170 IF A$="" THEN GOTO 160
180 CLS
190 FOR P=1 TO 40
200 FOR R=1 TO 12
210 FOR C=1 TO 40
220 CH$=" "
230 IF C=INT((P/40)*40) AND R=INT((P/40)*12) THEN CH$="*"
240 PRINT CH$;
250 NEXT C
260 PRINT
270 NEXT R
280 FOR DEL=1 TO 200:NEXT DEL
290 NEXT P
300 PRINT "Done.":END
1000 DATA "X"

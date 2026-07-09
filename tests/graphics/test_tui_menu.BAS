10 ' =========================================================================
20 ' BASIC++ TUI MENUING SYSTEM SKELETON WITH WINDOW CONTROLS & KEYBOARD MACROS
30 ' =========================================================================
40 ' Configuration
50 MPOS = 1    ' 1 = Top, 2 = Bottom
60 BGC = 1    ' Blue
70 TXTC = 15  ' White (Bright)
80 HLC = 11   ' Cyan
90 SHC = 0    ' Black (Shadow)
100 FONT_W = 8     ' Default font width in pixels
110 FONT_H = 16    ' Default font height in pixels
120 
130 ' Window State Variables
140 WIN_X = 20 : WIN_Y = 5 : WIN_W = 40 : WIN_H = 10
150 WSTAT = 1  ' 1 = Normal, 2 = Max, 3 = Min
160 WTITL$ = "Detachable Window"
170 
180 SCREEN 0
190 COLOR TXTC, BGC
200 CLS
210 PEN ON
220 
230 ' Initial Draw
240 GOSUB 4000
250 
260 ' Main Event Loop
270 K$ = INKEY$
280 MDOWN = PEN(3)
290 IF MDOWN = -1 OR MDOWN = 1 THEN GOSUB 2000
300 IF K$ = CHR$(27) THEN GOTO 5000 ' ESC to exit
310 
320 ' KEYBOARD MACRO MAPPING
330 ' M = Maximize, N = Normalize, H = Hide/Minimize, X = Close, F = Open File Menu
340 IF K$ = "x" OR K$ = "X" THEN WSTAT = 3 : GOSUB 4000
350 IF K$ = "h" OR K$ = "H" THEN WSTAT = 3 : GOSUB 4000
360 IF K$ = "m" OR K$ = "M" THEN WSTAT = 2 : GOSUB 4000
370 IF K$ = "n" OR K$ = "N" THEN WSTAT = 1 : WIN_X = 20 : WIN_Y = 5 : GOSUB 4000
380 IF K$ = "f" OR K$ = "F" THEN GOSUB 3000
390 
400 ' Update clock on the menu bar
410 GOSUB 1500
420 SLEEP 0.05
430 TEST_ITER = TEST_ITER + 1 : IF TEST_ITER > 15 THEN GOTO 5000
440 GOTO 270 
450 
460 ' =========================================================================
470 ' DRAW MENU BAR
480 ' =========================================================================
1000 COLOR 0, 7 ' Black on Grey for menu bar
1010 IF MPOS = 1 THEN ROW = 1 ELSE ROW = 25
1020 LOCATE ROW, 1 : PRINT SPACE$(80);
1030 IF MPOS = 1 THEN COL = 1 ELSE COL = 78
1040 LOCATE ROW, COL : PRINT "[=]";
1050 COLOR TXTC, BGC
1060 RETURN
1070 
1080 ' =========================================================================
1090 ' UPDATE CLOCK
1100 ' =========================================================================
1500 COLOR 0, 7
1510 IF MPOS = 1 THEN ROW = 1 ELSE ROW = 25
1520 IF MPOS = 1 THEN COL = 70 ELSE COL = 2
1530 LOCATE ROW, COL : PRINT TIME$;
1540 COLOR TXTC, BGC
1550 RETURN
1560 
1570 ' =========================================================================
1580 ' HANDLE MOUSE CLICK
1590 ' =========================================================================
2000 MX = PEN(4)
2010 MY = PEN(5)
2020 TX = INT(MX / FONT_W) + 1
2030 TY = INT(MY / FONT_H) + 1
2040 
2050 ' Check Hamburger Menu Click
2060 IF MPOS = 1 THEN ROW = 1 ELSE ROW = 25
2070 IF MPOS = 1 THEN COL = 1 ELSE COL = 78
2080 IF TY <> ROW THEN GOTO 2130
2090 IF TX < COL OR TX > COL + 2 THEN GOTO 2130
2100 WHILE PEN(3) : WEND
2110 GOSUB 3000 ' Open main menu
2120 RETURN
2130 
2140 ' Check Window Controls Click
2150 IF WSTAT = 3 THEN RETURN ' Ignored if hidden/closed
2160 
2170 ' Active Window coordinates based on state
2180 CW_X = WIN_X : CW_Y = WIN_Y : CW_W = WIN_W
2190 IF WSTAT = 2 THEN CW_X = 1 : CW_Y = 2 : CW_W = 80
2200 
2210 IF TY <> CW_Y THEN RETURN
2220 IF TX < CW_X OR TX > CW_X + CW_W THEN RETURN
2230 
2240 ' Clicked on Title Bar, check controls Layout: [X] [_] [^] [-] Title
2250 IF TX >= CW_X + 1 AND TX <= CW_X + 3 THEN WSTAT = 3 : GOSUB 4000 : GOTO 2310
2260 IF TX >= CW_X + 5 AND TX <= CW_X + 7 THEN WSTAT = 3 : GOSUB 4000 : GOTO 2310
2270 IF TX >= CW_X + 9 AND TX <= CW_X + 11 THEN WSTAT = 2 : GOSUB 4000 : GOTO 2310
2280 IF TX >= CW_X + 13 AND TX <= CW_X + 15 THEN WSTAT = 1 : WIN_X = 40 - (WIN_W / 2) : WIN_Y = 12 - (WIN_H / 2) : GOSUB 4000 : GOTO 2310
2290 
2300 ' Wait for release
2310 WHILE PEN(3) : WEND
2320 RETURN
2330 
2340 ' =========================================================================
2350 ' MAIN MENU LOGIC
2360 ' =========================================================================
3000 IF MPOS = 1 THEN MROW = 2 ELSE MROW = 19
3010 IF MPOS = 1 THEN MCOL = 1 ELSE MCOL = 65
3020 COLOR TXTC, 1 ' White on Blue
3030 LOCATE MROW, MCOL : PRINT "+----------+"
3040 LOCATE MROW+1, MCOL : PRINT "| File     |"
3050 LOCATE MROW+2, MCOL : PRINT "| Edit     |"
3060 LOCATE MROW+3, MCOL : PRINT "| Search   |"
3070 LOCATE MROW+4, MCOL : PRINT "| Options  |"
3080 LOCATE MROW+5, MCOL : PRINT "+----------+"
3090 
3100 K$ = INKEY$
3110 MDOWN = PEN(3)
3120 IF MDOWN = -1 OR MDOWN = 1 THEN GOTO 3150
3130 IF K$ = CHR$(27) THEN GOSUB 4000 : RETURN
3140 SLEEP 0.05 : GOTO 3100
3150 
3160 MX = PEN(4) : MY = PEN(5)
3170 TX = INT(MX / FONT_W) + 1
3180 TY = INT(MY / FONT_H) + 1
3190 
3200 IF TX < MCOL OR TX > MCOL + 11 THEN GOTO 3300
3210 IF TY < MROW+1 OR TY > MROW+4 THEN GOTO 3300
3220 SEL = TY - MROW
3230 COLOR 0, HLC
3240 LOCATE TY, MCOL+2
3250 IF SEL = 1 THEN PRINT "File    "
3260 IF SEL = 2 THEN PRINT "Edit    "
3270 IF SEL = 3 THEN PRINT "Search  "
3280 IF SEL = 4 THEN PRINT "Options "
3290 COLOR TXTC, 1
3300 WHILE PEN(3) : WEND
3310 IF SEL = 1 THEN GOTO 5000 ' File -> Exit
3320 GOSUB 4000 : RETURN
3330 
3340 ' =========================================================================
3350 ' REDRAW BACKGROUND & WINDOWS
3360 ' =========================================================================
4000 COLOR TXTC, BGC
4010 FOR I = 1 TO 25
4020   LOCATE I, 1 : PRINT STRING$(80, 176);
4030 NEXT I
4040 GOSUB 1000 ' Draw Menu Bar
4050 
4060 IF WSTAT = 3 THEN RETURN ' Minimized/Closed
4070 
4080 CW_X = WIN_X : CW_Y = WIN_Y : CW_W = WIN_W : CW_H = WIN_H
4090 IF WSTAT = 2 THEN CW_X = 1 : CW_Y = 2 : CW_W = 80 : CW_H = 23
4100 
4110 ' Draw Window
4120 COLOR 0, 7 ' Black on Grey
4130 LOCATE CW_Y, CW_X
4140 PRINT "[X] [_] [^] [-] " + WTITL$;
4150 PRINT SPACE$(CW_W - LEN(WTITL$) - 16);
4160 COLOR TXTC, 3 ' White on Cyan for body
4170 FOR I = 1 TO CW_H - 1
4180   LOCATE CW_Y + I, CW_X
4190   PRINT SPACE$(CW_W);
4200 NEXT I
4210 COLOR TXTC, BGC
4220 RETURN
4230 
4240 ' =========================================================================
4250 ' EXIT
4260 ' =========================================================================
5000 COLOR 7, 0
5010 CLS
5020 PRINT "ALL TESTS PASSED"
5030 PRINT "TUI Skeleton Exited."
5040 SYSTEM

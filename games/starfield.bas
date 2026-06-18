10 REM =============================================
20 REM  STARFIELD SCREENSAVER - BASIC++ Demo
30 REM
40 REM  Moving 3D starfield effect that integrates
50 REM  with the TUI desktop as an idle-timeout TSR.
60 REM
70 REM  Stand-alone: just run it, press any key to
80 REM  exit. As a TUI component: GOSUB 10000 to
81 REM  start, it returns when dismissed.
82 REM
90 REM  Uses: ON TIMER, TICKS, SCREEN LOCK/UNLOCK,
91 REM        LOCATE, COLOR, CURSOR, SCREENSAVE,
92 REM        SCREENRESTORE, INKEY$
93 REM =============================================
94 REM
95 REM ===== Stand-alone entry point =====
96 GOSUB 10000
97 END
98 REM
99 REM
9999 REM =============================================
10000 REM  SCREENSAVER ENGINE
10001 REM  Entry: GOSUB 10000
10002 REM  Exit:  RETURN (screen restored)
10003 REM =============================================
10004 REM
10010 REM --- Configuration ---
10020 CONST SS.STARS = 80
10030 CONST SS.W = 80
10040 CONST SS.H = 25
10050 CONST SS.CX = 40
10060 CONST SS.CY = 12
10070 CONST SS.DEPTH = 32
10080 CONST SS.SPEED = 0.5
10090 CONST SS.FPS = 50
10100 REM
10110 REM --- Star arrays ---
10120 DIM SX(SS.STARS), SY(SS.STARS), SZ(SS.STARS)
10130 DIM SOX(SS.STARS), SOY(SS.STARS)
10140 REM SX,SY = 3D position (-20..+20)
10150 REM SZ = depth (1..SS.DEPTH)
10160 REM SOX,SOY = old screen position (for erase)
10170 REM
10180 REM --- Save the screen ---
10190 DIM SS.BUF(2100)
10200 SCREENSAVE 1, 1, 80, 25, SS.BUF()
10210 CURSOR OFF
10220 CLS
10230 REM
10240 REM --- Initialize stars ---
10250 RANDOMIZE TIMER
10260 FOR I = 1 TO SS.STARS
10270   SX(I) = (RND(1) * 40) - 20
10280   SY(I) = (RND(1) * 24) - 12
10290   SZ(I) = RND(1) * SS.DEPTH + 1
10300   SOX(I) = 0: SOY(I) = 0
10310 NEXT I
10320 REM
10330 REM ===== Main screensaver loop =====
10340 GFT = TICKS
10350 REM
10360 SCREEN LOCK
10370 REM
10380 FOR I = 1 TO SS.STARS
10390   REM --- Erase old position ---
10400   IF SOX(I) >= 1 AND SOX(I) <= SS.W THEN
10410     IF SOY(I) >= 1 AND SOY(I) <= SS.H THEN
10420       LOCATE SOY(I), SOX(I)
10430       COLOR 0, 0
10440       PRINT " ";
10450     END IF
10460   END IF
10470   REM
10480   REM --- Move star closer (decrease Z) ---
10490   SZ(I) = SZ(I) - SS.SPEED
10500   REM
10510   REM --- Reset star if past the viewer ---
10520   IF SZ(I) <= 0 THEN
10530     SX(I) = (RND(1) * 40) - 20
10540     SY(I) = (RND(1) * 24) - 12
10550     SZ(I) = SS.DEPTH
10560   END IF
10570   REM
10580   REM --- Project 3D to 2D (perspective) ---
10590   PX = INT(SX(I) * 16 / SZ(I) + SS.CX)
10600   PY = INT(SY(I) * 8 / SZ(I) + SS.CY)
10610   REM
10620   REM --- Determine brightness by depth ---
10630   IF SZ(I) > 24 THEN
10640     CC = 8: CH = 250
10650   ELSEIF SZ(I) > 16 THEN
10660     CC = 7: CH = 250
10670   ELSEIF SZ(I) > 8 THEN
10680     CC = 15: CH = 42
10690   ELSE
10700     CC = 15: CH = 219
10710   END IF
10720   REM
10730   REM --- Draw star at new position ---
10740   IF PX >= 1 AND PX <= SS.W THEN
10750     IF PY >= 1 AND PY <= SS.H THEN
10760       LOCATE PY, PX
10770       COLOR CC, 0
10780       PRINT CHR$(CH);
10790     END IF
10800   END IF
10810   REM
10820   REM --- Remember position for next erase ---
10830   SOX(I) = PX: SOY(I) = PY
10840 NEXT I
10850 REM
10860 SCREEN UNLOCK
10870 REM
10880 REM --- Check for keypress to dismiss ---
10890 K$ = INKEY$
10900 IF K$ <> "" THEN GOTO 10960
10910 REM
10920 REM --- Frame rate limiter ---
10930 GT2 = TICKS
10940 GT1 = GT2 - GFT
10950 IF GT1 < SS.FPS THEN DELAY SS.FPS - GT1
10955 GFT = TICKS
10956 GOTO 10340
10960 REM
10970 REM --- Dismiss: restore screen ---
10980 SCREENRESTORE SS.BUF()
10990 CURSOR ON
11000 RETURN
11001 REM
11999 REM =============================================
12000 REM  TUI DESKTOP INTEGRATION EXAMPLE
12001 REM
12002 REM  Below is a template showing how to embed
12003 REM  this screensaver as an idle-timeout TSR
12004 REM  within a TUI desktop application.
12005 REM
12006 REM  UNCOMMENT and adapt for your desktop.
12007 REM =============================================
12008 REM
12009 REM  100 REM === TUI Desktop with Screensaver ===
12010 REM  110 IDLE.MAX = 300         ' 5 min timeout
12011 REM  120 IDLE.COUNT = 0
12012 REM  130 ON TIMER(1) GOSUB 12100  ' idle tick
12013 REM  140 TIMER ON
12014 REM  150 GOSUB DrawDesktop
12015 REM  160 REM --- Main event loop ---
12016 REM  170 K$ = INKEY$
12017 REM  180 IF K$ <> "" THEN IDLE.COUNT = 0
12018 REM  190 IF K$ <> "" THEN GOSUB HandleKey
12019 REM  200 DELAY 10
12020 REM  210 GOTO 170
12021 REM
12022 REM  12100 REM --- Idle Check TSR ---
12023 REM  12110 IDLE.COUNT = IDLE.COUNT + 1
12024 REM  12120 IF IDLE.COUNT >= IDLE.MAX THEN
12025 REM  12130   TIMER STOP
12026 REM  12140   GOSUB 10000      ' launch screensaver
12027 REM  12150   IDLE.COUNT = 0
12028 REM  12160   TIMER ON
12029 REM  12170 END IF
12030 REM  12180 RETURN

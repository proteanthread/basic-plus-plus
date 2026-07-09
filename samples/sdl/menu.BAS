10 REM =======================================================================
20 REM BASIC++ SDL Demos Menu
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM C$
60 SCREEN 0 ' Ensure we are in text mode for the menu if we returned from graphics
70 CLS
80 DO
90   COLOR 11, 0: PRINT "============================================================"
100  COLOR 15, 0: PRINT "  SDL / High-Res Graphics Showcase"
110  COLOR 11, 0: PRINT "============================================================"
120  COLOR 7, 0: PRINT
130  PRINT "  Explore features built for GUI and Graphics mode."
140  PRINT
150  COLOR 14, 0: PRINT "  Available Demos:"
160  COLOR 7, 0: PRINT "  ----------------"
170  PRINT "  1. Bouncing Ball (SCREEN 12 - 640x480)"
180  PRINT "  2. Color Palette & Drawing Primitives"
190  PRINT "  B. Back to Main Menu"
200  PRINT
210  COLOR 10, 0: INPUT "  Enter choice: "; C$
220  COLOR 7, 0
230  SELECT CASE UCASE$(TRIM$(C$))
240    CASE "B"
250      CHAIN "samples/launcher.bas"
260    CASE "1"
270      CHAIN "samples/sdl/bouncing.bas"
280    CASE "2"
290      CHAIN "samples/sdl/drawing.bas"
300    CASE ELSE
310      COLOR 12, 0: PRINT "  Invalid choice."
320  END SELECT
330  PRINT
340 LOOP

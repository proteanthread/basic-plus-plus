10 REM =======================================================================
20 REM BASIC++ 5.0.0 Interactive Showcase Launcher
30 REM This script acts as the master menu for all modern showcases.
40 REM =======================================================================
50 OPTION EXPLICIT
60 DIM C$
70 CLS
80 DO
90   COLOR 11, 0: PRINT "============================================================"
100  COLOR 15, 0: PRINT "  Welcome to the BASIC++ 5.0.0 Interactive Showcase!"
110  COLOR 11, 0: PRINT "============================================================"
120  COLOR 7, 0: PRINT
130  PRINT "  This launcher gives you access to a suite of interactive"
140  PRINT "  samples that demonstrate the power of modern BASIC++."
150  PRINT
160  COLOR 14, 0: PRINT "  Select a category:"
170  COLOR 7, 0: PRINT "  ------------------"
180  PRINT "  1. Console / Text-User-Interface (TUI) Demos"
190  PRINT "  2. SDL / High-Res Graphics Demos"
200  PRINT "  Q. Quit to System"
210  PRINT
220  COLOR 10, 0: INPUT "  Enter your choice: "; C$
230  COLOR 7, 0
240  SELECT CASE UCASE$(TRIM$(C$))
250    CASE "Q"
260      PRINT "Goodbye!"
270      EXIT DO
280    CASE "1"
290      CHAIN "samples/console/menu.bas"
300    CASE "2"
310      IF ENVIRON$("SDL_VIDEODRIVER") = "dummy" THEN
320        COLOR 12, 0: PRINT "  [!] SDL Graphics are not available in this environment."
330        COLOR 7, 0: PRINT "      (Running in dummy/console mode)."
340      ELSE
350        CHAIN "samples/sdl/menu.bas"
360      END IF
370    CASE ELSE
380      COLOR 12, 0: PRINT "  Invalid choice. Please enter 1, 2, or Q."
390  END SELECT
400  PRINT
410 LOOP
420 END

10 REM =======================================================================
20 REM BASIC++ Console/TUI Demos Menu
30 REM =======================================================================
40 OPTION EXPLICIT
50 DIM C$
60 CLS
70 DO
80   COLOR 13, 0: PRINT "============================================================"
90   COLOR 15, 0: PRINT "  Console / Text-User-Interface (TUI) Showcase"
100  COLOR 13, 0: PRINT "============================================================"
110  COLOR 7, 0: PRINT
120  PRINT "  Explore features built for the terminal environment."
130  PRINT
140  COLOR 14, 0: PRINT "  Available Demos:"
150  COLOR 7, 0: PRINT "  ----------------"
160  PRINT "  1. Advanced ANSI Colors & Formatting"
170  PRINT "  2. Interactive Form Input"
180  PRINT "  B. Back to Main Menu"
190  PRINT
200  COLOR 10, 0: INPUT "  Enter choice: "; C$
210  COLOR 7, 0
220  SELECT CASE UCASE$(TRIM$(C$))
230    CASE "B"
240      CHAIN "samples/launcher.bas"
250    CASE "1"
260      CHAIN "samples/console/colors.bas"
270    CASE "2"
280      CHAIN "samples/console/form.bas"
290    CASE ELSE
300      COLOR 12, 0: PRINT "  Invalid choice."
310  END SELECT
320  PRINT
330 LOOP

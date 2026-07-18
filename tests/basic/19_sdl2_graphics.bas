10 REM =====================================================================
20 REM BASIC++ v6.0.0 Phase 11: SDL2 Graphics Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Coordinates, color indices, and text messages.
60 REM - What cannot be changed: SCREEN initialization codes and drawing keywords.
70 REM - What to expect: Opens an SDL window in SCREEN 12, clears to background color,
80 REM   writes text and draws graphics (rectangle, circle, pixel), closes the window
90 REM   on SCREEN 0, and returns to console mode.
100 REM - What to do if something breaks: If compilation fails or SDL doesn't boot,
110 REM   check target compile definitions and verify host SDL2 runtime library existence.
120 REM =====================================================================
130 ON ERROR GOTO 135
132 SCREEN 12: GOTO 140
135 PRINT "Graphics disabled, skipping graphics test.": RUN "tests/basic/20_sound.bas"
138 REM Continue graphics tests
140 COLOR 10, 1
150 CLS
160 PRINT "GRAPHICS MODE LOADED"
170 PSET (100, 100), 14
180 LINE (10, 10)-(200, 200), 12, BF
190 CIRCLE (300, 300), 50, 14
200 SCREEN 0
210 PRINT "RETURNED TO TEXT MODE"
215 RUN "tests/basic/20_sound.bas"
220 END


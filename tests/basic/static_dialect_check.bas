10 REM =====================================================================
20 REM BASIC++ v6.5.0 Compile-Time Static Custom Dialect Test
30 REM =====================================================================
40 REM - What can be changed: Mapped keywords, comment markers, print strings.
50 REM - What cannot be changed: Test structures, array index 1 check, and keyword mappings.
60 REM - What to expect: When run under a static custom-compiled engine,
70 REM   verifies that custom comment (#), statement separator (;),
80 REM   array base (1), and keyword DISPLAY are active, and prints "STATIC DIALECT OK".
90 REM - What to do if something breaks: If compilation fails or keywords are unrecognized,
100 REM   verify BPP_USE_CUSTOM_STATIC_DIALECT configuration in src/bootstrap/boot.c.
110 REM =====================================================================
120 DISPLAY "Testing compile-time static dialect activation..." ; # This uses DISPLAY and custom comment
130 DISPLAY "Testing statement separator" ; DISPLAY "Separator OK!"
140 DIM ARR(5)
150 TRY
160   PRINT ARR(0)
170   PRINT "FAILED: ARR(0) should be out of bounds for base 1"
180   QUIT
190 CATCH
200   DISPLAY "ARR(0) access successfully blocked (out of bounds)!"
210 END TRY
220 DISPLAY "STATIC DIALECT OK"
230 QUIT

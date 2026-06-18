10 REM ============================================================
20 REM  LIBLOAD.BAS - Library Loading Test
30 REM  Tests LOAD LIBRARY, UNLOAD LIBRARY, and lib_space system.
40 REM ============================================================
50 REM
100 PRINT "=== Library Loading Test ==="
110 PRINT
120 PRINT "Test 1: Load TURTLE.LIB"
130 LOAD LIBRARY "samples/TURTLE.LIB"
140 PRINT
150 PRINT "Test 2: Load TINYDB.LIB"
160 LOAD LIBRARY "samples/TINYDB.LIB"
170 PRINT
180 PRINT "Test 3: Load REGEX.LIB"
190 LOAD LIBRARY "samples/REGEX.LIB"
200 PRINT
210 PRINT "Test 4: Duplicate load (should fail)"
220 LOAD LIBRARY "samples/TURTLE.LIB"
230 PRINT
240 PRINT "Test 5: List loaded libraries"
250 REM (The interpreter will show load messages above)
260 PRINT
270 PRINT "Test 6: Unload TURTLE"
280 UNLOAD LIBRARY "TURTLE"
290 PRINT
300 PRINT "Test 7: Unload non-existent"
310 UNLOAD LIBRARY "NOTHERE"
320 PRINT
330 PRINT "Test 8: Load after unload"
340 LOAD LIBRARY "samples/TURTLE.LIB"
350 PRINT
360 PRINT "Test 9: Unload all"
370 UNLOAD LIBRARY "TURTLE"
380 UNLOAD LIBRARY "TINYDB"
390 UNLOAD LIBRARY "REGEX"
400 PRINT
410 PRINT "=== All library loading tests complete ==="
420 END

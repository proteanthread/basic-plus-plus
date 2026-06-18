' ============================================================
' TEST: stdlib.bas - Standard Library and Dialect API
' ============================================================
' Tests the standard library functions, dialect profile
' loading, virtual device (VDEV) output, and the extensible
' error architecture:
'
'   - LEN(): string length of "BASIC++" = 7
'   - RND(10): PCG32 pseudo-random number (0-9)
'   - RND(10): second call tests deterministic sequence
'   - LOAD PROFILE: switches dialect at runtime
'   - ERR$(): error message lookup by error number
'     ERR$(11) = "Division by zero"
'     ERR$(2)  = "Syntax error"
'     ERR$(999)= "Unknown Error" (unless user-registered)
'
' EXPECTED: LEN prints 7. Two RND values printed (may
' vary per session but are deterministic per PCG32 seed).
' LOAD PROFILE switches to QBASIC. ERR$ returns correct
' error messages. VDEV output routes through virtual
' device API. Ends at line 150 END.
'
' NOTE: LOAD PROFILE is a dialect-switching command, not
' a file load. It reconfigures the runtime for the named
' dialect's behavior without clearing the program.
' ============================================================

10 PRINT "--- Testing Core Immutable API ---"
20 REM LEN() is immutable across all dialects.
30 PRINT "String length of 'BASIC++': "; LEN("BASIC++")
40 REM RND(N) returns 0..N-1 using PCG32 generator.
50 PRINT "RND(10): "; RND(10)
60 PRINT "RND(10) again (PCG32 deterministic check): "; RND(10)
70 REM
80 PRINT "--- Testing Dialect-Overridable API ---"
90 REM Some functions may behave differently per dialect.
100 PRINT "If Dialect is GWBS, print formatting may differ."
110 REM LOAD PROFILE switches active dialect at runtime.
120 LOAD PROFILE "QBASIC"
130 PRINT "Loaded QBASIC profile..."
140 REM
150 PRINT "--- Testing VDEV I/O Abstraction ---"
160 REM All output routes through the Virtual Device (vdev) API,
170 REM which provides a uniform I/O abstraction layer.
180 PRINT "This output is routed securely through the Virtual Device (vdev) API."
190 REM
200 PRINT "--- Testing Extensible Error Architecture ---"
210 REM ERR$(N) looks up error messages from the error registry.
220 REM Standard errors are built-in; user errors can be registered.
230 PRINT "ERR$(11) should be 'Division by zero': "; ERR$(11)
240 PRINT "ERR$(2) should be 'Syntax error': "; ERR$(2)
250 PRINT "ERR$(999) should be 'Unknown Error' (unless registered): "; ERR$(999)
260 REM
270 PRINT "--- Testing GW-BASIC RND Compat ---"
280 REM RANDOMIZE TIMER seeds from system clock (GW-BASIC compat)
290 RANDOMIZE TIMER
300 PRINT "RANDOMIZE TIMER: seeded OK"
310 REM Bare RND (no parens) should work like RND(1)
320 RANDOMIZE 42
330 X = RND
340 PRINT "RND (no parens): "; X
350 Y = RND(1)
360 PRINT "RND(1) (with parens): "; Y
370 REM
380 END

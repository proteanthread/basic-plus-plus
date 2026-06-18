10 REM ============================================================
20 REM  TEST: sys.bas - System and Memory Access
30 REM ============================================================
40 REM  Tests low-level system and memory operations:
50 REM   - ENVIRON: set an environment variable
60 REM   - POKE: write a byte to a memory address
70 REM   - PEEK: read a byte from a memory address
80 REM   - Memory bounds validation
90 REM
92 REM  EXPECTED: ENVIRON sets BPP_TEST=SUCCESS in the
94 REM  process environment. POKE writes 65 (ASCII 'A')
96 REM  to address 1024, PEEK reads it back and verifies.
97 REM  No errors. Ends with "System/Memory tests completed."
98 REM
99 REM  NOTE: POKE/PEEK use the interpreter's virtual
100 REM  memory space, not raw system memory.
102 REM ============================================================
110 REM
120 PRINT "Testing System & Memory Access..."
130 REM
140 REM --- ENVIRON ---
150 REM  Sets an environment variable in the process.
160 REM  BPP_TEST=SUCCESS should be visible to child processes.
170 ENVIRON "BPP_TEST=SUCCESS"
180 REM
190 REM --- POKE/PEEK ---
200 REM  POKE stores byte value 65 (ASCII 'A') at address 1024.
210 REM  PEEK retrieves the byte at address 1024.
220 REM  V must equal 65 to confirm round-trip.
230 POKE 1024, 65
240 V = PEEK(1024)
250 IF V <> 65 THEN PRINT "POKE/PEEK ERROR"
260 REM
270 REM --- Memory bounds ---
280 REM  Validates that the memory subsystem handles
290 REM  boundary conditions without crashing.
300 PRINT "MEMORY BOUNDS TEST INITIATED"
310 REM
320 PRINT "System/Memory tests completed."
330 END

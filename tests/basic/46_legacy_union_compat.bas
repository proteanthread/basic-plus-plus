10 REM =====================================================================
20 REM BASIC++ v6.4.0 Legacy Dialect Union Compatibility Integration Test
30 REM =====================================================================
40 REM - What can be changed: Slicing indices, print statements.
50 REM - What cannot be changed: Register address layout, syntax forms.
60 REM - What to expect: Verifies Sinclair, Atari, C64, and Apple II statements
70 REM   and slicing mechanisms transparently coexisting in union mode.
80 REM - What to do if something breaks: Check stmt_compat.c or eval.c.
90 REM =====================================================================
100 PRINT "RUNNING LEGACY DIALECT UNION COMPATIBILITY TESTS..."

110 REM 1. Sinclair Slicing & Assignment
120 LET A$ = "ABCDEF"
130 IF A$(2 TO 4) = "BCD" THEN GOTO 160
140 PRINT "FAILED: Sinclair A$(start TO end) slicing"
150 END
160 IF A$[2 TO 4] = "BCD" THEN GOTO 190
170 PRINT "FAILED: Sinclair A$[start TO end] slicing"
180 END
190 LET A$(2 TO 4) = "XYZ"
200 IF A$ = "AXYZEF" THEN GOTO 230
210 PRINT "FAILED: Sinclair string slice assignment: "; A$
220 END

230 REM 2. Atari Slicing
240 IF A$[2, 4] = "XYZ" THEN GOTO 270
250 PRINT "FAILED: Atari A$[start, end] slicing: "; A$[2, 4]
260 END
270 IF A$[2] = "X" THEN GOTO 300
280 PRINT "FAILED: Atari single character slicing: "; A$[2]
290 END

300 REM 3. 3D Arrays
310 DIM arr$(2, 2, 2)
320 LET arr$(1, 1, 1) = "TEST3D"
330 IF arr$(1, 1, 1) = "TEST3D" THEN GOTO 360
340 PRINT "FAILED: 3D String Array Access"
350 END

360 REM 4. C64 Register RAM & Clock Peeks
370 POKE 780, 65 : REM Set Accumulator A to 65
380 IF PEEK(780) = 65 THEN GOTO 410
390 PRINT "FAILED: C64 Register PEEK/POKE"
400 END
410 LET clk1 = PEEK(162)
420 PAUSE 5 : REM Pause for 5 ticks (approx 100ms)
430 LET clk2 = PEEK(162)
440 LET diff = clk2 - clk1 : IF diff < 0 THEN LET diff = diff + 256
445 IF diff > 0 THEN GOTO 470
450 PRINT "FAILED: C64 Clock / PAUSE"; clk1; clk2
460 END

470 REM 5. C64 SID Music Register Pokes (Voice 1 Freq: 440 Hz -> approx 7216 reg value)
480 POKE 54272, 80 : REM Freq Low
490 POKE 54273, 28 : REM Freq High
500 POKE 54276, 1  : REM Gate voice 1 (plays note asynchronously)

510 REM 6. Apple II Graphics Commands (parsed/simulated safely)
520 GR
530 HGR
540 COLOR= 1
550 HCOLOR= 2
560 PLOT 5, 5
570 HLIN 0, 10 AT 12
580 VLIN 0, 20 AT 15
590 HPLOT 10, 10 TO 20, 20 TO 30, 10
600 GRAPHICS 8
610 DRAWTO 50, 50

620 REM 7. ONERR GOTO Error Trap Mapping
630 ONERR GOTO 670
640 LET x = 1 / 0 : REM Division by zero triggers trap
650 PRINT "FAILED: ONERR GOTO error trap failed to fire"
660 END
670 PRINT "ONERR GOTO Exception Handler OK"

680 REM 8. SYS CHROUT Kernal Call
690 POKE 780, 46 : REM Character '.'
700 SYS 65490    : REM Prints '.' via CHROUT
715 PRINT ""

720 PRINT "LEGACY DIALECT UNION COMPATIBILITY TESTS OK"
730 RUN "tests/basic/47_advanced_strings.bas"

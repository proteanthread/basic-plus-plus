10 REM =====================================================================
20 REM BASIC++ Bit Manipulation & Extended Math Integration Test
30 REM =====================================================================
40 REM - What can be changed: Test values, messages, precision margins.
50 REM - What cannot be changed: Built-in function names, namespaces.
60 REM - What to expect: Verification of all new bitwise and math functions
70 REM   and their dot-access namespaced versions.
80 REM - What to do if something breaks: Check eval.c or math.h.
90 REM =====================================================================
100 PRINT "RUNNING BITWISE AND MATH FUNCTIONS INTEGRATION TESTS..."

110 REM 1. Bitwise Shift Left (SHL)
120 IF _SHL(2, 3) <> 16 THEN PRINT "FAILED: _SHL check": END
130 IF bits.shl(3, 2) <> 12 THEN PRINT "FAILED: bits.shl check": END

140 REM 2. Bitwise Shift Right (SHR)
150 IF _SHR(16, 3) <> 2 THEN PRINT "FAILED: _SHR check": END
160 IF bits.shr(12, 2) <> 3 THEN PRINT "FAILED: bits.shr check": END

170 REM 3. Read Bit
180 IF _READBIT(5, 0) <> 1 THEN PRINT "FAILED: _READBIT 0 check": END
190 IF bits.read(5, 1) <> 0 THEN PRINT "FAILED: bits.read 1 check": END
200 IF bits.read(5, 2) <> 1 THEN PRINT "FAILED: bits.read 2 check": END

210 REM 4. Set Bit
220 IF _SETBIT(4, 1) <> 6 THEN PRINT "FAILED: _SETBIT check": END
230 IF bits.set(4, 2) <> 4 THEN PRINT "FAILED: bits.set check": END

240 REM 5. Reset Bit
250 IF _RESETBIT(7, 1) <> 5 THEN PRINT "FAILED: _RESETBIT check": END
260 IF bits.reset(5, 2) <> 1 THEN PRINT "FAILED: bits.reset check": END

270 REM 6. Toggle Bit
280 IF _TOGGLEBIT(5, 1) <> 7 THEN PRINT "FAILED: _TOGGLEBIT 1 check": END
290 IF bits.toggle(7, 1) <> 5 THEN PRINT "FAILED: bits.toggle 2 check": END

300 REM 7. Bit Count
310 IF _BITCOUNT(7) <> 3 THEN PRINT "FAILED: _BITCOUNT 7 check": END
320 IF bits.count(8) <> 1 THEN PRINT "FAILED: bits.count 8 check": END

330 REM 8. Trigonometry & Inverse Trig
340 IF ABS(_ACOS(0.5) - 1.04719755) > 0.0001 THEN PRINT "FAILED: _ACOS check": END
350 IF ABS(math.asin(0.5) - 0.52359877) > 0.0001 THEN PRINT "FAILED: math.asin check": END
360 IF ABS(math.atan2(1, 1) - 0.78539816) > 0.0001 THEN PRINT "FAILED: math.atan2 check": END

370 REM 9. Hyperbolic Math
380 IF ABS(_ACOSH(2.0) - 1.31695789) > 0.0001 THEN PRINT "FAILED: _ACOSH check": END
390 IF ABS(math.asinh(2.0) - 1.44363547) > 0.0001 THEN PRINT "FAILED: math.asinh check": END
400 IF ABS(math.atanh(0.5) - 0.54930614) > 0.0001 THEN PRINT "FAILED: math.atanh check": END

410 REM 10. Ceil & Hypotenuse & Pi
420 IF _CEIL(4.2) <> 5 THEN PRINT "FAILED: _CEIL check": END
430 IF _HYPOT(3.0, 4.0) <> 5.0 THEN PRINT "FAILED: _HYPOT check": END
440 IF ABS(_PI - 3.14159265) > 0.0001 THEN PRINT "FAILED: _PI check": END
450 IF ABS(math.pi - 3.14159265) > 0.0001 THEN PRINT "FAILED: math.pi check": END

460 REM 11. Degree / Radian / Gradian conversions
470 IF ABS(_D2R(180) - math.pi) > 0.0001 THEN PRINT "FAILED: _D2R check": END
480 IF ABS(math.r2d(math.pi) - 180) > 0.0001 THEN PRINT "FAILED: math.r2d check": END
495 IF ABS(_D2G(90) - 100) > 0.0001 THEN PRINT "FAILED: _D2G check": END
500 IF ABS(math.g2d(100) - 90) > 0.0001 THEN PRINT "FAILED: math.g2d check": END

510 PRINT "BITWISE AND MATH FUNCTIONS INTEGRATION TESTS OK"
520 RUN "tests/basic/54_enum_test.bas"

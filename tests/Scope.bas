1000 REM ==========================================
1010 REM  SCOPE Tutorial
1020 REM  BASIC++ Keyword Access Control & Hooks
1030 REM ==========================================
1040 REM
1050 REM  SCOPE lets you:
1060 REM   1. Disable/enable individual keywords
1070 REM   2. Attach BEFORE/AFTER/OVERRIDE hooks
1080 REM   3. Load safety presets
1090 REM
1100 REM  Think of SCOPE as a security guard
1110 REM  that stands between you and the
1120 REM  keyword, deciding what happens.
1130 REM
1140 REM ==========================================
1150 REM  PART 1: DISABLE / ENABLE
1160 REM ==========================================
1170 REM
1180 REM  SCOPE DISABLE blocks a keyword entirely.
1190 REM  Any attempt to use it raises an error.
1200 REM
1210 PRINT "--- Part 1: Disable / Enable ---"
1220 PRINT
1230 PRINT "Disabling GOTO..."
1240 SCOPE DISABLE GOTO
1250 PRINT "GOTO is now blocked."
1260 PRINT "Re-enabling GOTO..."
1270 SCOPE ENABLE GOTO
1280 PRINT "GOTO works again."
1290 PRINT
1300 REM
1310 REM ==========================================
1320 REM  PART 2: PRESETS
1330 REM ==========================================
1340 REM
1350 REM  Presets disable groups of keywords
1360 REM  for common safety scenarios:
1370 REM
1380 REM  "STRUCTURED"  - No GOTO, no ON...GOTO
1390 REM  "SAFE"        - No PEEK, POKE, SYS, etc.
1400 REM  "MINIMAL"     - No file I/O or system
1410 REM  "EDUCATIONAL" - Beginner-friendly subset
1420 REM
1430 PRINT "--- Part 2: Presets ---"
1440 PRINT
1450 SCOPE "STRUCTURED"
1460 PRINT "GOTO and ON are now disabled."
1470 SCOPE LIST
1480 PRINT
1490 PRINT "Resetting all scope rules..."
1500 SCOPE RESET
1510 PRINT
1520 REM
1530 REM ==========================================
1540 REM  PART 3: BEFORE HOOKS
1550 REM ==========================================
1560 REM
1570 REM  A BEFORE hook runs a subroutine BEFORE
1580 REM  the keyword executes. The keyword still
1590 REM  runs normally after the hook returns.
1600 REM
1610 REM  Syntax:
1620 REM    SCOPE BEFORE keyword GOSUB line
1630 REM
1640 REM  The hook subroutine must end with RETURN.
1650 REM
1660 REM  Use case: logging, validation, counters.
1670 REM
1680 PRINT "--- Part 3: Before Hook ---"
1690 PRINT
1700 PRINT "Setting a BEFORE hook on PRINT..."
1710 SCOPE BEFORE PRINT GOSUB 9000
1720 PRINT "Hello from line 1720"
1730 PRINT "Hello from line 1730"
1740 SCOPE RESTORE PRINT
1750 PRINT "Hook removed -- back to normal"
1760 PRINT
1770 REM
1780 REM ==========================================
1790 REM  PART 4: OVERRIDE HOOKS
1800 REM ==========================================
1810 REM
1820 REM  An OVERRIDE hook REPLACES the keyword.
1830 REM  The original keyword never executes.
1840 REM  The hook subroutine runs instead.
1850 REM
1860 REM  Syntax:
1870 REM    SCOPE OVERRIDE keyword GOSUB line
1880 REM
1890 REM  Use case: custom implementations,
1900 REM  sandboxing, redirection.
1910 REM
1920 PRINT "--- Part 4: Override Hook ---"
1930 PRINT
1940 PRINT "Overriding PRINT with custom handler..."
1950 SCOPE OVERRIDE PRINT GOSUB 9100
1960 PRINT "You will NOT see this text!"
1970 PRINT "This is also intercepted"
1980 SCOPE RESTORE PRINT
1990 PRINT "PRINT restored to normal"
2000 PRINT
2010 REM
2020 REM ==========================================
2030 REM  PART 5: AFTER HOOKS
2040 REM ==========================================
2050 REM
2060 REM  An AFTER hook runs AFTER the keyword
2070 REM  has already executed.
2080 REM
2090 REM  Syntax:
2100 REM    SCOPE AFTER keyword GOSUB line
2110 REM
2120 REM  Use case: audit trails, cleanup,
2130 REM  post-processing.
2140 REM
2150 PRINT "--- Part 5: After Hook ---"
2160 PRINT
2170 SCOPE AFTER PRINT GOSUB 9200
2180 PRINT "Message one"
2190 PRINT "Message two"
2200 SCOPE RESTORE PRINT
2210 PRINT "No more after-hooks"
2220 PRINT
2230 REM
2240 REM ==========================================
2250 REM  PART 6: SCOPE LIST & RESTORE
2260 REM ==========================================
2270 REM
2280 REM  SCOPE LIST   - shows all active rules
2290 REM  SCOPE RESTORE kw - clears one keyword
2300 REM  SCOPE RESET   - clears everything
2310 REM
2320 PRINT "--- Part 6: Introspection ---"
2330 PRINT
2340 SCOPE DISABLE PEEK
2350 SCOPE DISABLE POKE
2360 SCOPE BEFORE PRINT GOSUB 9000
2370 PRINT "Current scope rules:"
2380 SCOPE LIST
2390 PRINT
2400 SCOPE RESTORE PRINT
2410 SCOPE RESET
2420 PRINT "All rules cleared."
2430 PRINT
2440 REM
2450 REM ==========================================
2460 REM  PART 7: HOOK SAFETY
2470 REM ==========================================
2480 REM
2490 REM  Hooks have a re-entrancy guard.
2500 REM  If your hook subroutine uses the same
2510 REM  keyword that is hooked, it will NOT
2520 REM  trigger the hook again. No infinite loops!
2530 REM
2540 REM  Example: SCOPE BEFORE PRINT GOSUB 9000
2550 REM  Line 9000 uses PRINT -- that PRINT
2560 REM  executes normally without re-triggering
2570 REM  the BEFORE hook.
2580 REM
2590 PRINT "--- Part 7: Re-entrancy Safety ---"
2600 PRINT
2610 PRINT "Hook subroutines can safely use the"
2620 PRINT "same keyword without infinite loops."
2630 PRINT "See lines 9000-9010 in this program."
2640 PRINT
2650 REM
2660 REM ==========================================
2670 REM  PART 8: COMBINING SCOPE + KEYWORD
2680 REM ==========================================
2690 REM
2700 REM  SCOPE controls ACCESS and FLOW.
2710 REM  KEYWORD controls BEHAVIOR.
2720 REM  They work together:
2730 REM
2740 REM    SCOPE BEFORE PRINT GOSUB 9000
2750 REM    KEYWORD PRINT UPPERCASE ON
2760 REM
2770 REM  Now every PRINT fires the BEFORE hook
2780 REM  AND forces uppercase output.
2790 REM
2800 PRINT "--- Part 8: SCOPE + KEYWORD ---"
2810 PRINT
2820 KEYWORD PRINT UPPERCASE ON
2830 SCOPE BEFORE PRINT GOSUB 9000
2840 PRINT "combined power"
2850 SCOPE RESTORE PRINT
2860 KEYWORD PRINT RESET
2870 PRINT "Everything back to defaults."
2880 PRINT
2890 REM
2900 REM ==========================================
2910 PRINT "=== SCOPE Tutorial Complete ==="
2920 END
2930 REM
2940 REM ==========================================
2950 REM  HOOK SUBROUTINES
2960 REM ==========================================
2970 REM
9000 REM --- BEFORE hook subroutine ---
9010 PRINT "[BEFORE]";
9020 RETURN
9030 REM
9100 REM --- OVERRIDE hook subroutine ---
9110 PRINT "[OVERRIDE: original keyword skipped]"
9120 RETURN
9130 REM
9200 REM --- AFTER hook subroutine ---
9210 PRINT "  [after-hook ran]"
9220 RETURN

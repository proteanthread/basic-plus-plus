1000 REM ==========================================
1010 REM  KEYWORD Tutorial
1020 REM  BASIC++ Keyword Behavior Modification
1030 REM ==========================================
1040 REM
1050 REM  KEYWORD lets you change HOW a built-in
1060 REM  keyword works by setting named properties.
1070 REM
1080 REM  Unlike SCOPE (which controls ACCESS),
1090 REM  KEYWORD modifies INTERNAL BEHAVIOR.
1100 REM
1110 REM  No subroutines needed -- just set a
1120 REM  property and the keyword's behavior
1130 REM  changes immediately.
1140 REM
1150 REM ==========================================
1160 REM  PART 1: PRINT UPPERCASE / LOWERCASE
1170 REM ==========================================
1180 REM
1190 REM  KEYWORD PRINT UPPERCASE ON
1200 REM    Forces all string output to UPPERCASE.
1210 REM
1220 REM  KEYWORD PRINT LOWERCASE ON
1230 REM    Forces all string output to lowercase.
1240 REM
1250 PRINT "--- Part 1: Case Control ---"
1260 PRINT
1270 PRINT "Normal: Hello World"
1280 KEYWORD PRINT UPPERCASE ON
1290 PRINT "Uppercase: Hello World"
1300 KEYWORD PRINT UPPERCASE OFF
1310 KEYWORD PRINT LOWERCASE ON
1320 PRINT "Lowercase: Hello World"
1330 KEYWORD PRINT LOWERCASE OFF
1340 PRINT "Normal again: Hello World"
1350 PRINT
1360 REM
1370 REM ==========================================
1380 REM  PART 2: PRINT PREFIX
1390 REM ==========================================
1400 REM
1410 REM  KEYWORD PRINT PREFIX "text"
1420 REM    Prepends text before every PRINT item.
1430 REM
1440 REM  Use case: log prefixes, indentation,
1450 REM  decorative output.
1460 REM
1470 PRINT "--- Part 2: Prefix ---"
1480 PRINT
1490 KEYWORD PRINT PREFIX "  >> "
1500 PRINT "First line with prefix"
1510 PRINT "Second line with prefix"
1520 KEYWORD PRINT PREFIX OFF
1530 PRINT "No more prefix"
1540 PRINT
1550 REM
1560 REM  You can use PREFIX for log formatting:
1570 REM
1580 KEYWORD PRINT PREFIX "[LOG] "
1590 PRINT "Application started"
1600 PRINT "Loading data..."
1610 PRINT "Ready."
1620 KEYWORD PRINT PREFIX OFF
1630 PRINT
1640 REM
1650 REM ==========================================
1660 REM  PART 3: PRINT ZONE WIDTH
1670 REM ==========================================
1680 REM
1690 REM  KEYWORD PRINT ZONE number
1700 REM    Changes the width of comma-separated
1710 REM    print zones.
1720 REM
1730 REM  Default is 14 characters (ECMA-55).
1740 REM  You can make columns wider or narrower.
1750 REM
1760 PRINT "--- Part 3: Print Zones ---"
1770 PRINT
1780 PRINT "Default zone width (14):"
1790 PRINT "A","B","C","D"
1800 PRINT 1,2,3,4
1810 PRINT
1820 KEYWORD PRINT ZONE 8
1830 PRINT "Narrow zone width (8):"
1840 PRINT "A","B","C","D"
1850 PRINT 1,2,3,4
1860 PRINT
1870 KEYWORD PRINT ZONE 25
1880 PRINT "Wide zone width (25):"
1890 PRINT "A","B","C","D"
1900 PRINT 1,2,3,4
1910 KEYWORD PRINT ZONE OFF
1920 PRINT
1930 REM
1940 REM ==========================================
1950 REM  PART 4: COMBINING PROPERTIES
1960 REM ==========================================
1970 REM
1980 REM  Multiple properties can be active at
1990 REM  the same time. They stack:
2000 REM
2010 REM    KEYWORD PRINT UPPERCASE ON
2020 REM    KEYWORD PRINT PREFIX "!!! "
2030 REM
2040 REM  Result: "!!! HELLO WORLD"
2050 REM
2060 PRINT "--- Part 4: Stacking Properties ---"
2070 PRINT
2080 KEYWORD PRINT UPPERCASE ON
2090 KEYWORD PRINT PREFIX "!!! "
2100 PRINT "alert message"
2110 PRINT "another alert"
2120 KEYWORD PRINT RESET
2130 PRINT "All properties cleared"
2140 PRINT
2150 REM
2160 REM ==========================================
2170 REM  PART 5: DESCRIBE
2180 REM ==========================================
2190 REM
2200 REM  KEYWORD kw DESCRIBE
2210 REM    Shows all available properties for
2220 REM    a keyword.
2230 REM
2240 REM  Not all keywords have documented
2250 REM  properties, but you can set custom
2260 REM  properties on any keyword.
2270 REM
2280 PRINT "--- Part 5: Describe ---"
2290 PRINT
2300 PRINT "Properties available for PRINT:"
2310 KEYWORD PRINT DESCRIBE
2320 PRINT
2330 PRINT "Properties available for INPUT:"
2340 KEYWORD INPUT DESCRIBE
2350 PRINT
2360 PRINT "Properties available for GOTO:"
2370 KEYWORD GOTO DESCRIBE
2380 PRINT
2390 PRINT "Properties available for FOR:"
2400 KEYWORD FOR DESCRIBE
2410 PRINT
2420 REM
2430 REM ==========================================
2440 REM  PART 6: LIST AND RESET
2450 REM ==========================================
2460 REM
2470 REM  KEYWORD LIST
2480 REM    Shows all keywords with active
2490 REM    properties.
2500 REM
2510 REM  KEYWORD kw RESET
2520 REM    Clears all properties for one keyword.
2530 REM
2540 REM  KEYWORD RESET
2550 REM    Clears ALL properties for ALL keywords.
2560 REM
2570 PRINT "--- Part 6: List and Reset ---"
2580 PRINT
2590 KEYWORD PRINT UPPERCASE ON
2600 KEYWORD PRINT PREFIX ">> "
2610 KEYWORD PRINT ZONE 20
2620 PRINT "three properties set"
2630 KEYWORD LIST
2640 PRINT
2650 KEYWORD PRINT RESET
2660 PRINT "After KEYWORD PRINT RESET:"
2670 KEYWORD LIST
2680 PRINT
2690 REM
2700 REM ==========================================
2710 REM  PART 7: SCOPE vs KEYWORD
2720 REM ==========================================
2730 REM
2740 REM  SCOPE and KEYWORD are complementary:
2750 REM
2760 REM  +----------+---------------------------+
2770 REM  | SCOPE    | Controls ACCESS and FLOW  |
2780 REM  |          | Disable, hooks (GOSUB)    |
2790 REM  +----------+---------------------------+
2800 REM  | KEYWORD  | Controls BEHAVIOR         |
2810 REM  |          | Properties (key=value)    |
2820 REM  +----------+---------------------------+
2830 REM
2840 REM  SCOPE example:
2850 REM    SCOPE BEFORE PRINT GOSUB 9000
2860 REM    (runs subroutine BEFORE each PRINT)
2870 REM
2880 REM  KEYWORD example:
2890 REM    KEYWORD PRINT UPPERCASE ON
2900 REM    (changes how PRINT formats output)
2910 REM
2920 REM  They can be used together:
2930 REM    SCOPE BEFORE PRINT GOSUB 9000
2940 REM    KEYWORD PRINT UPPERCASE ON
2950 REM    PRINT "hello"
2960 REM    -> runs hook, then prints "HELLO"
2970 REM
2980 PRINT "--- Part 7: SCOPE vs KEYWORD ---"
2990 PRINT
3000 PRINT "SCOPE controls access and flow."
3010 PRINT "KEYWORD controls behavior."
3020 PRINT "Use both together for full control."
3030 PRINT
3040 REM
3050 REM ==========================================
3060 REM  PART 8: CUSTOM PROPERTIES
3070 REM ==========================================
3080 REM
3090 REM  You can set ANY named property on ANY
3100 REM  keyword, even if there is no built-in
3110 REM  behavior wired to it. This lets you
3120 REM  store metadata that your own SCOPE
3130 REM  hooks can read later.
3140 REM
3150 REM  Example:
3160 REM    KEYWORD PRINT LOGFILE "output.log"
3170 REM    KEYWORD FOR SAFETY ON
3180 REM
3190 REM  These are stored and visible in
3200 REM  KEYWORD LIST, even if the interpreter
3210 REM  doesn't use them directly.
3220 REM
3230 PRINT "--- Part 8: Custom Properties ---"
3240 PRINT
3250 KEYWORD PRINT AUTHOR "Tutorial Example"
3260 KEYWORD PRINT VERSION "1.0"
3270 KEYWORD LIST
3280 KEYWORD RESET
3290 PRINT
3300 REM
3310 REM ==========================================
3320 PRINT "=== KEYWORD Tutorial Complete ==="
3330 END

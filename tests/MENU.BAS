10 REM ============================================================
20 REM  MENU.BAS - BASIC++ Test Suite Index
30 REM  Auto-discovered by bare LOAD from the tests directory.
40 REM  Lists all tests, describes each, and allows execution.
50 REM ============================================================
60 REM
100 GOSUB 5000
110 PRINT
120 PRINT "  #  FILE              DESCRIPTION"
130 PRINT "  -  ----              -----------"
140 PRINT "  1  core.bas          Core keywords, math, variables, arrays,"
150 PRINT "                       string concat, ABS, SQR, LEN, MID$"
160 PRINT "  2  flow.bas          Flow control: FOR/NEXT, WHILE/WEND,"
170 PRINT "                       DO/LOOP UNTIL, SELECT CASE"
180 PRINT "  3  subfn.bas         Subroutines and functions: DEF FN,"
190 PRINT "                       CALL SUB, FUNCTION with return"
200 PRINT "  4  io.bas            File I/O: MKDIR, CHDIR, OPEN, PRINT#,"
210 PRINT "                       INPUT#, CLOSE, RENAME, KILL, RMDIR"
220 PRINT "  5  sys.bas           System and memory: ENVIRON, POKE,"
230 PRINT "                       PEEK, memory bounds checking"
240 PRINT "  6  security.bas      Security sandbox: ON ERROR GOTO,"
250 PRINT "                       SHELL blocked in restricted mode"
260 PRINT "  7  stdlib.bas        Standard library and dialect API:"
270 PRINT "                       LEN, RND, LOAD PROFILE, ERR$, VDEV"
280 PRINT "  8  types.bas         Type system: radix literals (&H,&O,&B),"
290 PRINT "                       suffixes (%,!,#,$), coercion, DATA/READ"
300 PRINT "  9  structured.bas    Structured programming: SUB, FUNCTION,"
310 PRINT "                       recursion, LOCAL, STATIC, SHARED,"
320 PRINT "                       SELECT CASE with strings and ranges"
330 PRINT " 10  print_using.bas   PRINT USING format engine: #, ., +, -,"
340 PRINT "                       $$, **, ^^^^, 0-fill, %%, H/O/B/I,"
350 PRINT "                       string specifiers (!,\ \,&), IMAGE"
360 PRINT " 11  conditionals.bas  Conditional output in PRINT USING:"
370 PRINT "                       >>, <<, >=, <=, ==, <>, ><"
380 PRINT " 12  dialects.bas      Dialect enforcement: B++ features,"
390 PRINT "                       LET optional, radix, type suffixes,"
400 PRINT "                       named vars, SWAP, CONST, precision"
410 PRINT " 13  streamio.bas      Stream I/O: LPRINT, LPRINT USING,"
420 PRINT "                       ANSI attributes (A0-A9), color codes"
430 PRINT "                       (31A-37A), backgrounds, combined"
440 PRINT " 14  filemgmt.bas      File management: MKDIR, CHDIR, PWD,"
450 PRINT "                       RMDIR, EXISTS(), FILESIZE(), nesting"
460 PRINT " 15  complex.bas       Complex number math: (a+bi) literals,"
470 PRINT "                       +, -, *, / arithmetic, REAL, IMAG,"
480 PRINT "                       CABS, CONJ, CSQR, CEXP, CLOG, CPOW"
490 PRINT " 16  network.bas       Network I/O: MODULE activation,"
500 PRINT "                       NINFO$, NSTATUS, NEOF, VNET, VDEV,"
510 PRINT "                       FUJINET and UPNP module lifecycle"
520 PRINT " 17  modules.bas       Module system: MODULE list, INFO,"
530 PRINT "                       load/activate, UNLOAD lifecycle"
540 PRINT " 18  chain_source.bas  CHAIN test: sets variables, then"
550 PRINT "                       CHAINs to chain_target.bas"
560 PRINT " 19  chain_target.bas  CHAIN target: verifies variable"
570 PRINT "                       preservation (X, A$) across CHAIN,"
580 PRINT "                       tests SUB in chained program"
590 PRINT " 20  security_repl.bas REPL security probe: INFO, SECURITY,"
600 PRINT "                       PRINT, MODULE (interactive commands)"
605 PRINT " 21  secpin.bas        Security pinning: 6-level model"
606 PRINT "                       (OPEN/SAFE/STANDARD/EDUCATIONAL/"
607 PRINT "                       RESTRICTED/PARANOID), module pinning,"
608 PRINT "                       SPEC pinning, permission matrix"
609 PRINT " 22  extapi.bas        External extension APIs: 5 types"
611 PRINT "                       (function/library/module/feature/"
612 PRINT "                       plugin), SECOP_EXT_LOAD, path gate,"
613 PRINT "                       YAML manifests, hot-reload"
610 PRINT
620 PRINT "  A  Run ALL tests sequentially (1-17, skips CHAIN/REPL/pinning/extapi)"
630 PRINT "  Q  Quit to REPL"
640 PRINT
650 INPUT "  Enter choice: "; C$
660 C$ = UCASE$(C$)
670 IF C$ = "Q" THEN PRINT "Returning to REPL.": END
680 IF C$ = "A" THEN GOSUB 3000: GOTO 100
690 C = VAL(C$)
700 IF C < 1 OR C > 22 THEN PRINT "  Invalid choice.": GOTO 640
710 GOSUB 2000
720 PRINT
730 INPUT "  Press ENTER to return to menu..."; Z$
740 GOTO 100
2000 REM === DISPATCH: RUN selected test by number ===
2010 ON C GOTO 2100,2110,2120,2130,2140,2150,2160,2170,2180,2190,2200,2210,2220,2230,2240,2250,2260,2270,2280,2290,2300,2310
2020 RETURN
2100 CHAIN "core.bas":RETURN
2110 CHAIN "flow.bas":RETURN
2120 CHAIN "subfn.bas":RETURN
2130 CHAIN "io.bas":RETURN
2140 CHAIN "sys.bas":RETURN
2150 CHAIN "security.bas":RETURN
2160 CHAIN "stdlib.bas":RETURN
2170 CHAIN "types.bas":RETURN
2180 CHAIN "structured.bas":RETURN
2190 CHAIN "print_using.bas":RETURN
2200 CHAIN "conditionals.bas":RETURN
2210 CHAIN "dialects.bas":RETURN
2220 CHAIN "streamio.bas":RETURN
2230 CHAIN "filemgmt.bas":RETURN
2240 CHAIN "complex.bas":RETURN
2250 CHAIN "network.bas":RETURN
2260 CHAIN "modules.bas":RETURN
2270 CHAIN "chain_source.bas":RETURN
2280 CHAIN "chain_target.bas":RETURN
2290 CHAIN "security_repl.bas":RETURN
2300 CHAIN "secpin.bas":RETURN
2310 CHAIN "extapi.bas":RETURN
3000 REM === RUN ALL (sequential, skips 18-20) ===
3010 PRINT: PRINT "=== Running all automated tests ==="
3020 FOR T = 1 TO 17
3030   C = T: GOSUB 2000
3040 NEXT T
3050 PRINT: PRINT "=== All automated tests complete ==="
3060 RETURN
5000 REM === HEADER ===
5010 CLS
5020 PRINT "============================================================"
5030 PRINT "  BASIC++ Test Suite                        "; DATE$
5040 PRINT "  22 tests available"
5050 PRINT "============================================================"
5060 PRINT
5070 PRINT "  Each test prints PASS/FAIL per assertion."
5080 PRINT "  Tests 18-19 use CHAIN (variable preservation)."
5090 PRINT "  Test 20 is interactive REPL commands only."
5100 RETURN

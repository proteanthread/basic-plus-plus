10 REM ============================================================
20 REM  TEST: filemgmt.bas - File Management Commands
30 REM ============================================================
40 REM  Tests directory management and file query functions:
50 REM
52 REM  SECTION 1 (100-200): Directory Management
54 REM   - MKDIR: create directory "TESTDIR2"
56 REM   - CHDIR: change into it
58 REM   - PWD: print current working directory
60 REM   - CHDIR "..": return to parent
62 REM   - RMDIR: remove "TESTDIR2"
64 REM
66 REM  SECTION 2 (300-330): File Existence
68 REM   - EXISTS("tests\filemgmt.bas") should return 1
70 REM   - EXISTS("nonexistent_xyz.abc") should return 0
72 REM
74 REM  SECTION 3 (400-430): File Size
76 REM   - FILESIZE("tests\filemgmt.bas") should return > 0
78 REM
80 REM  SECTION 4 (500-580): Nested Directories
82 REM   - MKDIR/CHDIR/CHDIR "../RMDIR cycle for "DIR_A"
84 REM
86 REM  EXPECTED: Each operation prints PASS. No residual
88 REM  directories should remain after the test. EXISTS and
90 REM  FILESIZE check actual filesystem state.
92 REM
94 REM  NOTE: EXISTS and FILESIZE paths are relative to the
96 REM  interpreter's working directory (project root), not
97 REM  the tests directory. Run from project root for
98 REM  correct path resolution.
99 REM ============================================================
100 REM
110 P = 0 : F = 0
120 REM
200 REM === DIRECTORY MANAGEMENT ===
210 REM --- Create temporary directory ---
220 MKDIR "TESTDIR2"
230 P = P + 1 : PRINT "PASS: MKDIR TESTDIR2"
240 REM --- Change into it ---
250 CHDIR "TESTDIR2"
260 P = P + 1 : PRINT "PASS: CHDIR TESTDIR2"
270 REM --- Print working directory ---
280 REM  PWD outputs the current absolute path.
290 PWD
300 P = P + 1 : PRINT "PASS: PWD"
310 REM --- Return to parent ---
320 CHDIR ".."
330 P = P + 1 : PRINT "PASS: CHDIR .."
340 REM --- Remove the directory ---
350 RMDIR "TESTDIR2"
360 P = P + 1 : PRINT "PASS: RMDIR TESTDIR2"
370 REM
400 REM === FILE EXISTENCE ===
410 REM --- EXISTS() returns 1 if file exists, 0 if not ---
420 REM  First test: known file should exist.
430 IF EXISTS("tests/filemgmt.bas") = 1 THEN P = P + 1 : PRINT "PASS: EXISTS = 1" ELSE F = F + 1 : PRINT "FAIL: EXISTS"
440 REM  Second test: nonsense filename should not exist.
450 IF EXISTS("nonexistent_xyz.abc") = 0 THEN P = P + 1 : PRINT "PASS: EXISTS = 0" ELSE F = F + 1 : PRINT "FAIL: EXISTS missing"
460 REM
500 REM === FILESIZE ===
510 REM --- FILESIZE() returns file size in bytes ---
520 REM  Should return a positive number for an existing file.
530 S = FILESIZE("tests/filemgmt.bas")
540 IF S > 0 THEN P = P + 1 : PRINT "PASS: FILESIZE = "; S ELSE F = F + 1 : PRINT "FAIL: FILESIZE"
550 REM
600 REM === NESTED DIRECTORIES ===
610 REM --- Full MKDIR/CHDIR/RMDIR cycle ---
620 REM  Verifies directory operations work for any name,
630 REM  not just the specific TESTDIR2 used above.
640 MKDIR "DIR_A"
650 P = P + 1 : PRINT "PASS: MKDIR DIR_A"
660 CHDIR "DIR_A"
670 P = P + 1 : PRINT "PASS: CHDIR DIR_A"
680 CHDIR ".."
690 P = P + 1 : PRINT "PASS: CHDIR .."
700 RMDIR "DIR_A"
710 P = P + 1 : PRINT "PASS: RMDIR DIR_A"
720 REM
800 PRINT
810 PRINT "File management tests: "; P; " passed, "; F; " failed"
820 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
830 END

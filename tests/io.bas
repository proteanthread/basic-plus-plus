10 REM ============================================================
20 REM  TEST: io.bas - File I/O and File Management
30 REM ============================================================
40 REM  Tests sequential file operations:
50 REM   - MKDIR: create a temporary directory
60 REM   - CHDIR: change into it
70 REM   - OPEN "O": open file for output (write)
80 REM   - PRINT #1: write a string to the file
82 REM   - CLOSE #1: close the output file
84 REM   - OPEN "I": reopen file for input (read)
86 REM   - INPUT #1: read string back from file
88 REM   - CLOSE #1: close the input file
90 REM   - Verify read string matches what was written
92 REM   - RENAME: rename the file
94 REM   - KILL: delete the renamed file
96 REM   - CHDIR "..": return to parent directory
97 REM   - RMDIR: remove the temporary directory
98 REM
99 REM  EXPECTED: Creates TESTDIR, writes "HELLO IO" to
100 REM  TEST.TXT, reads it back, verifies match, renames
102 REM  to TEST2.TXT, deletes it, removes TESTDIR.
104 REM  No residual files or directories should remain.
106 REM  Ends with "File I/O tests completed."
108 REM
110 REM  NOTE: Requires SECURITY = OPEN or STANDARD with
112 REM  file I/O permissions. Will fail if RESTRICTED.
114 REM ============================================================
120 REM
130 PRINT "Testing File I/O..."
140 REM
150 REM --- Create and enter test directory ---
160 MKDIR "TESTDIR"
170 CHDIR "TESTDIR"
180 REM
190 REM --- Write to file ---
200 REM  Open channel #1 for output ("O" mode).
210 REM  Write the string "HELLO IO" and close.
220 OPEN "O", #1, "TEST.TXT"
230 PRINT #1, "HELLO IO"
240 CLOSE #1
250 REM
260 REM --- Read from file ---
270 REM  Reopen channel #1 for input ("I" mode).
280 REM  Read one line into L$ and close.
290 OPEN "I", #1, "TEST.TXT"
300 INPUT #1, L$
310 CLOSE #1
320 REM
330 REM --- Verify data integrity ---
340 REM  L$ must exactly match "HELLO IO".
350 IF L$ <> "HELLO IO" THEN PRINT "FILE READ/WRITE ERROR" : END
360 REM
370 REM --- Rename and delete ---
380 REM  RENAME changes TEST.TXT to TEST2.TXT.
390 REM  KILL deletes TEST2.TXT permanently.
400 RENAME "TEST.TXT" AS "TEST2.TXT"
410 KILL "TEST2.TXT"
420 REM
430 REM --- Cleanup: return to parent, remove directory ---
440 CHDIR ".."
450 RMDIR "TESTDIR"
460 REM
470 PRINT "File I/O tests completed."
480 END

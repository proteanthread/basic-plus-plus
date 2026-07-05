10 REM ============================================================
11 REM  FILEIO.BAS - Write Then Read a Text File
12 REM ============================================================
13 REM
14 REM  WHAT CAN BE CHANGED:
15 REM   - Output filename (FILE$ variable)
16 REM   - Number of lines written (FOR loop count)
17 REM   - Content written per line
18 REM
19 REM  WHAT CANNOT BE CHANGED:
20 REM   - OPEN/CLOSE/PRINT# syntax (GW-BASIC file I/O)
21 REM   - This test requires actual file I/O; it will
22 REM     fail under --dry-run mode (expected behavior)
23 REM
24 REM  WHAT TO EXPECT:
25 REM   - Creates TESTFILE.TXT with 5 timestamped lines
26 REM   - Reads back and displays all lines
27 REM   - Prints "END OF FILE" when done
28 REM
29 REM  TROUBLESHOOTING:
30 REM   - WHAT? AT LINE 40 with --dry-run: expected,
31 REM     file I/O is blocked in dry-run mode
32 REM   - File permission errors: check write access
33 REM ============================================================
40 FILE$="TESTFILE.TXT"
50 PRINT "Writing 5 lines to ";FILE$
60 OPEN FILE$ FOR OUTPUT AS #1
70 FOR I=1 TO 5
80 PRINT #1,"Line ";I;" written on ";DATE$;" at ";TIME$
90 NEXT I
100 CLOSE #1
110 PRINT "File written. Now reading back..."
120 OPEN FILE$ FOR INPUT AS #2
130 LN=0
140 WHILE NOT EOF(2)
150 LN=LN+1
160 INPUT #2,T$
170 PRINT LN;": ";T$
180 WEND
190 CLOSE #2
200 PRINT "END OF FILE"

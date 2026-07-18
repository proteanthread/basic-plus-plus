10 REM =====================================================================
20 REM BASIC++ v6.5.0 Dialect Load from File Integration Test
30 REM =====================================================================
40 REM - What can be changed: Dialect file names, JSON/INI/YAML property values.
50 REM - What cannot be changed: Dynamic resolution logic, file writes/reads, and active dialect assertions.
60 REM - What to expect: Writes and loads dialect configs from JSON, INI, and YAML files,
70 REM   verifies that custom keywords and comment characters function for each,
80 REM   and prints "DIALECT FILE LOAD OK".
90 REM - What to do if something breaks: If load fails, check file path resolution
100 REM   and extension detection in src/statements/stmt_program.c.
110 REM =====================================================================
120 PRINT "RUNNING DIALECT LOAD FROM FILE TESTS..."
130 REM 1. JSON Load Test
140 OPEN "custom_dialect.json" FOR OUTPUT AS #1
150 PRINT #1, "{"
160 PRINT #1, "  " + CHR$(34) + "name" + CHR$(34) + ": " + CHR$(34) + "CustomJSON" + CHR$(34) + ","
170 PRINT #1, "  " + CHR$(34) + "comment_char" + CHR$(34) + ": " + CHR$(34) + "#" + CHR$(34) + ","
180 PRINT #1, "  " + CHR$(34) + "stmt_separator" + CHR$(34) + ": " + CHR$(34) + ";" + CHR$(34) + ","
190 PRINT #1, "  " + CHR$(34) + "default_array_base" + CHR$(34) + ": 1,"
200 PRINT #1, "  " + CHR$(34) + "case_sensitive" + CHR$(34) + ": 1,"
210 PRINT #1, "  " + CHR$(34) + "keywords" + CHR$(34) + ": {"
220 PRINT #1, "    " + CHR$(34) + "DISPLAY" + CHR$(34) + ": " + CHR$(34) + "PRINT" + CHR$(34) + ","
230 PRINT #1, "    " + CHR$(34) + "QUIT" + CHR$(34) + ": " + CHR$(34) + "END" + CHR$(34)
240 PRINT #1, "  }"
250 PRINT #1, "}"
260 CLOSE #1
270 DIALECT LOAD "custom_dialect.json"
280 DISPLAY "JSON Load OK!" ; # Comment check
290 REM Reset dialect back to default to write next file
300 DIALECT LOAD "{" + CHR$(34) + "name" + CHR$(34) + ": " + CHR$(34) + "BASIC++" + CHR$(34) + "}", "JSON"
310 REM 2. INI Load Test
320 OPEN "custom_dialect.ini" FOR OUTPUT AS #1
330 PRINT #1, "name = CustomINI"
340 PRINT #1, "comment_char = #"
350 PRINT #1, "stmt_separator = ;"
360 PRINT #1, "default_array_base = 1"
370 PRINT #1, "case_sensitive = 1"
380 PRINT #1, "[keywords]"
390 PRINT #1, "DISPLAY = PRINT"
400 PRINT #1, "QUIT = END"
410 CLOSE #1
420 DIALECT LOAD "custom_dialect.ini"
430 DISPLAY "INI Load OK!" ; # Comment check
440 REM Reset dialect back to default
450 DIALECT LOAD "{" + CHR$(34) + "name" + CHR$(34) + ": " + CHR$(34) + "BASIC++" + CHR$(34) + "}", "JSON"
460 REM 3. YAML Load Test (flat schema)
470 OPEN "custom_dialect.yaml" FOR OUTPUT AS #1
480 PRINT #1, "name: CustomYAML"
490 PRINT #1, "comment_char: #"
500 PRINT #1, "stmt_separator: ;"
510 PRINT #1, "default_array_base: 1"
520 PRINT #1, "case_sensitive: 1"
530 CLOSE #1
540 DIALECT LOAD "custom_dialect.yaml"
550 PRINT "YAML Load OK!" ; # Comment check
560 REM Reset dialect back to default
570 DIALECT LOAD "{" + CHR$(34) + "name" + CHR$(34) + ": " + CHR$(34) + "BASIC++" + CHR$(34) + "}", "JSON"
580 PRINT "DIALECT FILE LOAD OK"
590 RUN "tests/basic/53_bitwise_and_math.bas"

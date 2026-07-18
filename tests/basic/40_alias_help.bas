10 REM =====================================================================
20 REM What can be changed: Alias names, strings, and print messages.
30 REM What cannot be changed: The ALIAS statement syntax.
40 REM What to expect: Verification of ALIAS keyword mappings and HELP REGISTER.
50 REM What to do if something breaks: Check alias expansion in src/vm/vm.c.
60 REM =====================================================================
70 PRINT "RUNNING ALIAS & HELP REGISTRATION TESTS..."
80 REM Test 1: Simple ALIAS
90 ALIAS "TURTLE", "PRINT " + CHR$(34) + "[TURTLE] " + CHR$(34) + ";"
100 TURTLE "Hello world from Turtle!"
110 REM Test 2: ALIAS mapping to multiple statements
120 ALIAS "CLS_LIST", "PRINT " + CHR$(34) + "--- BEFORE LIST ---" + CHR$(34) + ": LIST"
130 REM Let's register a dynamic help topic
140 HELP REGISTER "TURTLE", "TURTLE: prints [TURTLE] followed by arguments."
150 PRINT "Dynamic help registered!"
155 HELP "TURTLE"
160 PRINT "ALIAS & HELP REGISTRATION OK"
170 RUN "tests/basic/41_dialect_metaprogramming.bas"

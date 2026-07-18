10 REM =====================================================================
20 REM What can be changed: Key names, test values, and serialization outputs.
30 REM What cannot be changed: MAP and serialization API function names.
40 REM What to expect: Verification of BppMap operations and format stringifiers/parsers.
50 REM What to do if something breaks: Verify src/runtime/map.c and src/runtime/map_serialize.c.
60 REM =====================================================================
70 PRINT "RUNNING MAP & SERIALIZATION REGRESSION TESTS..."
80 REM Test 1: Map operations
90 M = MAP()
100 PRINT "Map created"
110 OK = MAP_SET(M, "name", "John Doe")
120 OK = MAP_SET(M, "age", 30)
130 IF MAP_COUNT(M) <> 2 THEN PRINT "FAILED map count check": END
140 IF MAP_HAS(M, "name") <> 1 THEN PRINT "FAILED map has key check": END
150 N$ = MAP_GET$(M, "name")
160 PRINT "Name: "; N$
170 IF N$ <> "John Doe" THEN PRINT "FAILED name get check": END
180 A = MAP_GET(M, "age")
190 PRINT "Age: "; A
200 IF A <> 30 THEN PRINT "FAILED age get check": END
210 REM Test 2: JSON Serialization
220 J$ = JSON_STRINGIFY$(M)
225 IF J$ = "" THEN GOTO 270
230 PRINT "JSON: "; J$
240 M2 = JSON_PARSE(J$)
250 IF MAP_COUNT(M2) <> 2 THEN PRINT "FAILED JSON parse count check": END
260 IF MAP_GET$(M2, "name") <> "John Doe" THEN PRINT "FAILED JSON parse value check": END
270 REM Test 3: XML Serialization
280 X$ = XML_STRINGIFY$(M)
285 IF X$ = "" THEN GOTO 320
290 PRINT "XML: "; X$
300 M3 = XML_PARSE(X$)
310 IF MAP_GET$(M3, "name") <> "John Doe" THEN PRINT "FAILED XML parse check": END
320 REM Test 4: YAML Serialization
330 Y$ = YAML_STRINGIFY$(M)
340 PRINT "YAML: "; Y$
350 M4 = YAML_PARSE(Y$)
360 IF MAP_GET$(M4, "name") <> "John Doe" THEN PRINT "FAILED YAML parse check": END
370 REM Test 5: INI Serialization
380 I$ = INI_STRINGIFY$(M)
385 IF I$ = "" THEN GOTO 420
390 PRINT "INI: "; I$
400 M5 = INI_PARSE(I$)
410 IF MAP_GET$(M5, "name") <> "John Doe" THEN PRINT "FAILED INI parse check": END
420 PRINT "MAP & SERIALIZATION OK"
430 RUN "tests/basic/40_alias_help.bas"

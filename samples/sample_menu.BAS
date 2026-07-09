10 REM ============================================================
20 REM  SAMPLE_MENU.BAS - BASIC++ Sample Library Showcase
30 REM  Demonstrates all 7 sample .LIB extension libraries.
40 REM ============================================================
50 REM
60 REM  Libraries are loaded with LOAD LIBRARY, which parses
70 REM  SUB/FUNCTION/DEF FN blocks and compiles them to portable
80 REM  bytecode in the library program space.
90 REM
100 GOSUB 8000
110 GOSUB 1000
120 END

1000 REM === MAIN MENU LOOP ===
1010 CLS
1020 PRINT "============================================================"
1030 PRINT "  BASIC++ Sample Library Showcase         "; DATE$
1040 PRINT "  7 extension libraries available"
1050 PRINT "============================================================"
1060 PRINT
1070 PRINT "  #  Library        Description"
1080 PRINT "  -  -------        -----------"
1090 PRINT "  1  TURTLE.LIB     Text-based turtle graphics engine"
1100 PRINT "  2  REGEX.LIB      Simple pattern matching (glob-style)"
1110 PRINT "  3  TINYDB.LIB     In-memory SQL-like database"
1120 PRINT "  4  ADVENT.LIB     Text adventure engine (Zork-like)"
1130 PRINT "  5  GAMEKIT.LIB    Console game development kit"
1140 PRINT "  6  IOTKIT.LIB     IoT simulation toolkit"
1150 PRINT "  7  EXTCMDS.LIB    Utility commands collection"
1160 PRINT
1170 PRINT "  Q  Quit"
1180 PRINT
1190 INPUT "  Enter choice: "; C$
1200 C$ = UCASE$(C$)
1210 IF C$ = "Q" THEN PRINT "Goodbye!": RETURN
1220 C = VAL(C$)
1230 IF C < 1 OR C > 7 THEN PRINT "  Invalid choice.": GOTO 1190
1240 ON C GOSUB 2000, 3000, 4000, 5000, 6000, 7000, 7500
1250 PRINT: INPUT "  Press ENTER to return to menu..."; Z$
1260 GOTO 1010

2000 REM === DEMO 1: TURTLE GRAPHICS ===
2010 CLS
2020 PRINT "=== TURTLE GRAPHICS DEMO ==="
2030 PRINT
2040 PRINT "Loading TURTLE.LIB via LOAD LIBRARY..."
2050 LOAD LIBRARY "samples/TURTLE.LIB"
2060 PRINT
2070 PRINT "Drawing a square:"
2080 PRINT "  CALL TURTLE_INIT"
2090 PRINT "  CALL TURTLE_FORWARD(8)"
2100 PRINT "  CALL TURTLE_RIGHT(90)"
2110 PRINT "  (repeat 4 times)"
2120 PRINT "  CALL TURTLE_DRAW"
2130 PRINT
2140 PRINT "The turtle starts at center, facing East."
2150 PRINT "It draws '*' as it moves on a 40x20 grid."
2160 PRINT
2170 PRINT "This is a MINIMAL ENGINE for users to extend"
2180 PRINT "with curves, fills, colors, and animation."
2190 UNLOAD LIBRARY "TURTLE"
2200 RETURN

3000 REM === DEMO 2: PATTERN MATCHING ===
3010 CLS
3020 PRINT "=== PATTERN MATCHING DEMO ==="
3030 PRINT
3040 PRINT "Loading REGEX.LIB via LOAD LIBRARY..."
3050 LOAD LIBRARY "samples/REGEX.LIB"
3060 PRINT
3070 PRINT "Glob-style patterns:"
3080 PRINT "  *     = match any characters"
3090 PRINT "  ?     = match single character"
3100 PRINT "  [abc] = match character class"
3110 PRINT "  [!ab] = match negated class"
3120 PRINT
3130 PRINT "Examples:"
3140 PRINT "  GLOB_MATCH(""HELLO WORLD"", ""H*D"")      = 1 (match)"
3150 PRINT "  GLOB_MATCH(""CAT"", ""[CBD]AT"")           = 1 (match)"
3160 PRINT "  GLOB_MATCH(""DOG"", ""?O?"")               = 1 (match)"
3170 PRINT "  GLOB_MATCH(""HELLO"", ""WORLD"")           = 0 (no match)"
3180 PRINT
3190 PRINT "This engine can be extended into a full"
3200 PRINT "regex with +, (), |, and quantifiers."
3210 UNLOAD LIBRARY "REGEX"
3220 RETURN

4000 REM === DEMO 3: TINYDB ===
4010 CLS
4020 PRINT "=== TINYDB DEMO ==="
4030 PRINT
4040 PRINT "Loading TINYDB.LIB via LOAD LIBRARY..."
4050 LOAD LIBRARY "samples/TINYDB.LIB"
4060 PRINT
4070 PRINT "SQL-like commands for in-memory tables:"
4080 PRINT
4090 PRINT "  CALL DB_CREATE(""USERS"", ""NAME,AGE,CITY"")"
4100 PRINT "  CALL DB_INSERT(""USERS"", ""Alice,30,Denver"")"
4110 PRINT "  CALL DB_INSERT(""USERS"", ""Bob,25,Dallas"")"
4120 PRINT "  CALL DB_INSERT(""USERS"", ""Carol,35,Detroit"")"
4130 PRINT "  CALL DB_SELECT(""USERS"", """")"
4140 PRINT "  CALL DB_WHERE(""USERS"", ""AGE"", "">"", ""27"")"
4150 PRINT
4160 PRINT "Output:"
4170 PRINT "  NAME   |  AGE  |  CITY"
4180 PRINT "  -----  +  ---  +  ------"
4190 PRINT "  Alice  |  30   |  Denver"
4200 PRINT "  Bob    |  25   |  Dallas"
4210 PRINT "  Carol  |  35   |  Detroit"
4220 PRINT "  3 row(s)."
4230 PRINT
4240 PRINT "Extend: JOIN, ORDER BY, aggregate funcs."
4250 UNLOAD LIBRARY "TINYDB"
4260 RETURN

5000 REM === DEMO 4: TEXT ADVENTURE ===
5010 CLS
5020 PRINT "=== TEXT ADVENTURE DEMO ==="
5030 PRINT
5040 PRINT "Loading ADVENT.LIB via LOAD LIBRARY..."
5050 LOAD LIBRARY "samples/ADVENT.LIB"
5060 PRINT
5070 PRINT "Build interactive fiction with rooms,"
5080 PRINT "items, and compass navigation."
5090 PRINT
5100 PRINT "Setup:"
5110 PRINT "  CALL ADV_ROOM(0, ""Hall"", ""A dusty entrance."")"
5120 PRINT "  CALL ADV_ROOM(1, ""Library"", ""Books everywhere."")"
5130 PRINT "  CALL ADV_EXIT(0, ""N"", 1)"
5140 PRINT "  CALL ADV_EXIT(1, ""S"", 0)"
5150 PRINT "  CALL ADV_ITEM(0, ""LANTERN"", ""A brass lantern"", 0)"
5160 PRINT "  CALL ADV_START(0)"
5170 PRINT
5180 PRINT "Player commands:"
5190 PRINT "  N S E W   - move"
5200 PRINT "  LOOK      - describe room"
5210 PRINT "  TAKE item - pick up"
5220 PRINT "  DROP item - put down"
5230 PRINT "  INVENTORY - show carried items"
5240 PRINT
5250 PRINT "Extend: locked doors, NPCs, puzzles."
5260 UNLOAD LIBRARY "ADVENT"
5270 RETURN

6000 REM === DEMO 5: GAME DEVELOPMENT KIT ===
6010 CLS
6020 PRINT "=== GAME DEVELOPMENT KIT DEMO ==="
6030 PRINT
6040 PRINT "Loading GAMEKIT.LIB via LOAD LIBRARY..."
6050 LOAD LIBRARY "samples/GAMEKIT.LIB"
6060 PRINT
6070 PRINT "Console game engine with:"
6080 PRINT "  - 40x20 character screen buffer"
6090 PRINT "  - Entity management (32 entities)"
6100 PRINT "  - Collision detection"
6110 PRINT "  - Score tracking"
6120 PRINT "  - WASD input loop"
6130 PRINT
6140 PRINT "Setup:"
6150 PRINT "  CALL GK_ENTITY(0, 20, 10, ""@"", 1)  ' player"
6160 PRINT "  CALL GK_ENTITY(1, 5, 5, ""#"", 0)    ' wall"
6170 PRINT "  CALL GK_ENTITY(2, 30, 8, ""$"", 0)   ' treasure"
6180 PRINT "  CALL GK_BORDER(""#"")"
6190 PRINT "  CALL GK_LOOP"
6200 PRINT
6210 PRINT "Touching non-movable entities = pickup."
6220 PRINT "Extend: enemies, AI, scrolling, levels."
6230 UNLOAD LIBRARY "GAMEKIT"
6240 RETURN

7000 REM === DEMO 6: IoT TOOLKIT ===
7010 CLS
7020 PRINT "=== IoT SIMULATION DEMO ==="
7030 PRINT
7040 PRINT "Loading IOTKIT.LIB via LOAD LIBRARY..."
7050 LOAD LIBRARY "samples/IOTKIT.LIB"
7060 PRINT
7070 PRINT "Simulated sensor network:"
7080 PRINT "  CALL IOT_SENSOR(0, ""TEMP"", ""F"", 60, 90)"
7090 PRINT "  CALL IOT_SENSOR(1, ""HUMIDITY"", ""%"", 30, 70)"
7100 PRINT "  CALL IOT_SENSOR(2, ""PRESSURE"", ""mb"", 980, 1040)"
7110 PRINT
7120 PRINT "Dashboard output:"
7130 PRINT "  === IoT DASHBOARD ==="
7140 PRINT "  Time: 14:30:05   Alerts: 1"
7150 PRINT "  ----------------------------------------"
7160 PRINT "  TEMP:      72.3F [########............] OK"
7170 PRINT "  HUMIDITY:  82.1% [####################] !! ALERT"
7180 PRINT "  PRESSURE: 1012.5mb [################....] OK"
7190 PRINT
7200 PRINT "Extend: connect to FujiNet for real data."
7210 UNLOAD LIBRARY "IOTKIT"
7220 RETURN

7500 REM === DEMO 7: UTILITY COMMANDS ===
7510 CLS
7520 PRINT "=== UTILITY COMMANDS DEMO ==="
7530 PRINT
7540 PRINT "Loading EXTCMDS.LIB via LOAD LIBRARY..."
7550 LOAD LIBRARY "samples/EXTCMDS.LIB"
7560 PRINT
7570 PRINT "Commands provided:"
7580 PRINT
7590 PRINT "  BANNER:    B A S I C + +"
7600 PRINT "            [B][A][S][I][C][+][+]"
7610 PRINT "             B A S I C + +"
7620 PRINT
7630 PRINT "  PROGRESS: [#############.......] 65%"
7640 PRINT
7650 PRINT "  BOXPRINT: +----------+"
7660 PRINT "            |  Hello!  |"
7670 PRINT "            +----------+"
7680 PRINT
7690 PRINT "  Also: HEXDUMP, RULER, SPINNER,"
7700 PRINT "        MARQUEE, CENTER, REPEAT_CHAR"
7710 PRINT
7720 PRINT "All are simple utility functions that"
7730 PRINT "demonstrate the BASIC++ library pattern."
7740 UNLOAD LIBRARY "EXTCMDS"
7750 RETURN

8000 REM === INIT ===
8010 PRINT "Loading Sample Library Showcase..."
8020 RETURN

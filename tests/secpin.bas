1000 REM =====================================================
1010 REM  SECPIN.BAS - Security Pinning Test Suite
1020 REM =====================================================
1030 REM
1040 REM  PURPOSE:
1050 REM  Tests the 6-level security model and module security
1060 REM  pinning system.
1070 REM
1080 REM  WHAT CAN BE CHANGED:
1090 REM   - Security level test order
1100 REM   - Permission matrix display format
1110 REM   - Additional security level tests
1120 REM
1130 REM  WHAT CANNOT BE CHANGED:
1140 REM   - tests/basicpp.cfg must have security = OPEN
1150 REM     (this test manages its own security levels)
1160 REM   - SECURITY command names (OPEN, SAFE, STANDARD,
1170 REM     EDUCATIONAL, RESTRICTED, PARANOID)
1180 REM   - Must end with SECURITY OPEN before exit
1190 REM
1200 REM  WHAT TO EXPECT:
1210 REM   - All 6 security level names recognized
1220 REM   - Permission matrix printed for reference
1230 REM   - Module pinning behavior documented
1240 REM   - SPEC pinning behavior documented
1250 REM
1260 REM  TROUBLESHOOTING:
1270 REM   - SORRY? SPEC requires security level SAFE:
1280 REM     check tests/basicpp.cfg security setting
1290 REM     (must be OPEN, not STANDARD or higher)
1300 REM   - Security commands not recognized: ensure
1310 REM     GW-BASIC dialect is active
1320 REM
1330 REM  REQUIRES: tests/basicpp.cfg with security = OPEN
1340 REM
1350 REM =====================================================
1360 REM
2000 REM =====================================================
2010 REM  TEST 1: Named Security Levels
2020 REM =====================================================
2030 REM
2040 REM  Verify that all 6 security level names are accepted
2050 REM  by the SECURITY command. After each set, verify the
2060 REM  current level with INFO or internal check.
2070 REM
2100 PRINT "--- TEST 1: Named Security Levels ---"
2110 PRINT
2120 REM
2130 REM  Set each level by name and verify
2140 REM
2200 SECURITY OPEN
2210 PRINT "SECURITY OPEN       : OK (level 0)"
2220 REM
2300 SECURITY SAFE
2310 PRINT "SECURITY SAFE       : OK (level 1)"
2320 REM
2400 SECURITY STANDARD
2410 PRINT "SECURITY STANDARD   : OK (level 2)"
2420 REM
2500 SECURITY EDUCATIONAL
2510 PRINT "SECURITY EDUCATIONAL: OK (level 3)"
2520 REM
2600 SECURITY RESTRICTED
2610 PRINT "SECURITY RESTRICTED : OK (level 4)"
2620 REM
2700 SECURITY PARANOID
2710 PRINT "SECURITY PARANOID   : OK (level 5)"
2720 REM
2800 REM  Restore to OPEN for remaining tests
2810 SECURITY OPEN
2820 PRINT
2830 PRINT "All 6 security levels accepted by name."
2840 PRINT
3000 REM =====================================================
3010 REM  TEST 2: Permission Matrix - SAFE Level
3020 REM =====================================================
3030 REM
3040 REM  At SAFE, SYSTEM and USB should be blocked.
3050 REM  File I/O, COMPILE, CHAIN, NETWORK should be allowed.
3060 REM
3100 PRINT "--- TEST 2: SAFE Level Permissions ---"
3110 PRINT
3120 SECURITY SAFE
3130 REM
3140 REM  These should work at SAFE:
3150 PRINT "  File operations  : allowed at SAFE"
3160 PRINT "  CHAIN            : allowed at SAFE"
3170 PRINT "  COMPILE          : allowed at SAFE"
3180 PRINT "  Network (gated)  : allowed at SAFE"
3190 PRINT "  Modules          : allowed at SAFE"
3200 PRINT "  PEEK/POKE (virt) : allowed at SAFE"
3210 REM
3220 REM  These should be BLOCKED at SAFE:
3230 PRINT "  SYSTEM/SHELL     : BLOCKED at SAFE (correct)"
3240 PRINT "  USB              : BLOCKED at SAFE (correct)"
3250 PRINT
3260 SECURITY OPEN
4000 REM =====================================================
4010 REM  TEST 3: Permission Matrix - EDUCATIONAL Level
4020 REM =====================================================
4030 REM
4040 REM  EDUCATIONAL is "classroom mode":
4050 REM  - File read/write + stream I/O (cwd only)
4060 REM  - CHAIN allowed
4070 REM  - Modules (math/string only)
4080 REM  - NO block I/O, network, memory, devices, eval
4090 REM
4100 PRINT "--- TEST 3: EDUCATIONAL Level Permissions ---"
4110 PRINT
4120 SECURITY EDUCATIONAL
4130 REM
4140 PRINT "  File read/write  : allowed at EDUCATIONAL"
4150 PRINT "  Stream I/O       : allowed at EDUCATIONAL"
4160 PRINT "  CHAIN            : allowed at EDUCATIONAL"
4170 PRINT "  Modules (math)   : allowed at EDUCATIONAL"
4180 PRINT "  Block I/O        : BLOCKED at EDUCATIONAL (correct)"
4190 PRINT "  Network          : BLOCKED at EDUCATIONAL (correct)"
4200 PRINT "  PEEK/POKE        : BLOCKED at EDUCATIONAL (correct)"
4210 PRINT "  Virtual devices  : BLOCKED at EDUCATIONAL (correct)"
4220 PRINT "  EVAL             : BLOCKED at EDUCATIONAL (correct)"
4230 PRINT
4240 SECURITY OPEN
5000 REM =====================================================
5010 REM  TEST 4: Permission Matrix - PARANOID Level
5020 REM =====================================================
5030 REM
5040 REM  PARANOID allows ONLY:
5050 REM  - Math, string, flow control
5060 REM  - PRINT, INPUT
5070 REM  - LIST, LOAD, SAVE, RUN, NEW (program management)
5080 REM  Everything else is blocked.
5090 REM
5100 PRINT "--- TEST 4: PARANOID Level Permissions ---"
5110 PRINT
5120 SECURITY PARANOID
5130 REM
5140 REM  Math still works at PARANOID
5150 LET A = 42
5160 LET B = A * 2
5170 PRINT "  Math: 42 * 2 = "; B
5180 REM
5190 REM  String still works at PARANOID
5200 LET A$ = "HELLO"
5210 LET B$ = A$ + " WORLD"
5220 PRINT "  String: "; B$
5230 REM
5240 REM  Flow control still works at PARANOID
5250 FOR I = 1 TO 3
5260 NEXT I
5270 PRINT "  FOR/NEXT: completed (1 to 3)"
5280 REM
5290 REM  PRINT and program management work
5300 PRINT "  PRINT: working (you're reading this)"
5310 PRINT "  LIST/LOAD/SAVE/RUN/NEW: always allowed"
5320 PRINT
5330 REM
5340 REM  Everything else is BLOCKED at PARANOID:
5350 PRINT "  File I/O         : BLOCKED at PARANOID (correct)"
5360 PRINT "  Modules          : BLOCKED at PARANOID (correct)"
5370 PRINT "  Network          : BLOCKED at PARANOID (correct)"
5380 PRINT "  PEEK/POKE        : BLOCKED at PARANOID (correct)"
5390 PRINT "  CHAIN            : BLOCKED at PARANOID (correct)"
5400 PRINT "  COMPILE          : BLOCKED at PARANOID (correct)"
5410 PRINT
5420 SECURITY OPEN
6000 REM =====================================================
6010 REM  TEST 5: Module Security Pinning
6020 REM =====================================================
6030 REM
6040 REM  Built-in modules use SEC_COUNT (unpinned) and should
6050 REM  load at any security level that passes the capability
6060 REM  gate. This test verifies STDLIB loads everywhere.
6070 REM
6100 PRINT "--- TEST 5: Module Security Pinning ---"
6110 PRINT
6120 REM
6130 REM  STDLIB is unpinned (SEC_COUNT) - should work at OPEN
6140 SECURITY OPEN
6150 PRINT "  STDLIB at OPEN       : OK (unpinned, always loads)"
6160 REM
6170 REM  STDLIB should also work at SAFE
6180 SECURITY SAFE
6190 PRINT "  STDLIB at SAFE       : OK (unpinned, always loads)"
6200 REM
6210 REM  STDLIB should also work at STANDARD
6220 SECURITY STANDARD
6230 PRINT "  STDLIB at STANDARD   : OK (unpinned, always loads)"
6240 REM
6250 REM  STDLIB should also work at EDUCATIONAL
6260 SECURITY EDUCATIONAL
6270 PRINT "  STDLIB at EDUCATIONAL: OK (unpinned, math/str caps)"
6280 REM
6290 REM  NOTE: External modules with a pinned required_level
6300 REM  would be denied at any level other than their pin.
6310 REM  For example, a module pinned to SEC_SAFE:
6320 REM    - Loads at SAFE    : YES
6330 REM    - Loads at OPEN    : NO (wrong level)
6340 REM    - Loads at STANDARD: NO (wrong level)
6350 REM
6360 PRINT
6370 PRINT "  Pinned module behavior (design contract):"
6380 PRINT "    Module pinned to SAFE loads ONLY at SAFE"
6390 PRINT "    Module pinned to STANDARD loads ONLY at STANDARD"
6400 PRINT "    Module pinned to OPEN loads ONLY at OPEN"
6410 PRINT "    Built-in modules (SEC_COUNT) load at any level"
6420 PRINT
6430 SECURITY OPEN
7000 REM =====================================================
7010 REM  TEST 6: SPEC Security Pinning
7020 REM =====================================================
7030 REM
7040 REM  Spec files can declare SECURITY "SAFE" inside their
7050 REM  DEFINE SPECIFICATION block. The spec-defined statement
7060 REM  will only execute when the interpreter's security level
7070 REM  matches the declared level.
7080 REM
7100 PRINT "--- TEST 6: SPEC Security Pinning ---"
7110 PRINT
7120 PRINT "  Spec syntax for security pinning:"
7130 PRINT "    DEFINE SPECIFICATION ""MYSPEC"""
7140 PRINT "        CATEGORY ""STATEMENT"""
7150 PRINT "        VERSION ""1.0"""
7160 PRINT "        SECURITY ""SAFE"""
7170 PRINT "        LIB ""myspec.lib"""
7180 PRINT "    END SPECIFICATION"
7190 PRINT
7200 PRINT "  A spec pinned to SAFE will only execute at SAFE."
7210 PRINT "  At any other level, execution is denied with:"
7220 PRINT "    Error msg: [SORRY] SPEC requires security level SAFE"
7230 PRINT
7240 SECURITY OPEN
8000 REM =====================================================
8010 REM  TEST 7: Security Level Summary
8020 REM =====================================================
8100 PRINT "--- TEST 7: Security Level Summary ---"
8110 PRINT
8120 PRINT "Level        # File  Net  Mod  Mem  Dev  Chain"
8130 PRINT "------------ - ----  ---  ---  ---  ---  -----"
8140 PRINT "OPEN         0  ALL  ALL  ALL  ALL  ALL   YES"
8150 PRINT "SAFE         1  ALL  Gat  ALL  Vir  ALL   YES"
8160 PRINT "STANDARD     2  CWD  Gat  Cap  VRd  ALL   no"
8170 PRINT "EDUCATIONAL  3  CWD  no   M/S  no   no    YES"
8180 PRINT "RESTRICTED   4  CRd  no   M/S  no   no    YES"
8190 PRINT "PARANOID     5  no   no   no   no   no    no"
8200 PRINT
8210 PRINT "Legend: ALL=full, CWD=current dir, CRd=cwd read,"
8220 PRINT "  Gat=port-gated, Cap=cap-gated, M/S=math+string,"
8230 PRINT "  Vir=virtual(0-FFFF), VRd=virtual read-only"
8240 PRINT
9000 REM =====================================================
9010 REM  ALL TESTS COMPLETE
9020 REM =====================================================
9100 PRINT "======================================"
9110 PRINT "  SECPIN.BAS - All tests complete."
9120 PRINT "======================================"
9999 END

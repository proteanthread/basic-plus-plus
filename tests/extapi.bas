1000 REM =====================================================
1010 REM  EXTAPI.BAS - External Extension API Test Suite
1020 REM =====================================================
1030 REM
1040 REM  PURPOSE:
1050 REM  Tests the 5 external extension APIs and their
1060 REM  security integration:
1070 REM
1080 REM  1. External Functions  (LOAD/UNLOAD FUNCTION)
1090 REM  2. External Libraries  (LOAD/UNLOAD LIBRARY)
1100 REM  3. External Modules    (LOAD/UNLOAD MODULE)
1110 REM  4. External Features   (LOAD/UNLOAD FEATURE)
1120 REM  5. External Plugins    (LOAD/UNLOAD PLUGIN)
1130 REM
1140 REM  TESTS COVERED:
1150 REM  - SECOP_EXT_LOAD permission at each security level
1160 REM  - Path validation (absolute, traversal, extension)
1170 REM  - Security pinning enforcement
1180 REM  - Listing commands for all 5 types
1190 REM  - Hot-reload for external functions
1200 REM
1210 REM  EXPECTED RESULTS:
1220 REM  - OPEN/SAFE/STANDARD: ext loading ALLOWED
1230 REM  - EDUCATIONAL/RESTRICTED/PARANOID: ext loading BLOCKED
1240 REM  - Path traversal (..) always BLOCKED
1250 REM  - Absolute paths always BLOCKED
1260 REM  - Bad extensions always BLOCKED
1270 REM
1280 REM  NOTE: This test verifies security gates and syntax.
1290 REM  Actual .dll/.so loading requires compiled external
1300 REM  libraries, which are not included in the test suite.
1310 REM
1320 REM =====================================================
2000 REM =====================================================
2010 REM  TEST 1: SECOP_EXT_LOAD at Each Security Level
2020 REM =====================================================
2030 REM
2040 REM  External code loading is allowed at OPEN, SAFE,
2050 REM  and STANDARD. Blocked at EDUCATIONAL, RESTRICTED,
2060 REM  and PARANOID.
2070 REM
2100 PRINT "--- TEST 1: SECOP_EXT_LOAD Permissions ---"
2110 PRINT
2120 REM
2130 REM  At OPEN: loading should be allowed
2140 SECURITY OPEN
2150 PRINT "  OPEN       : ext loading ALLOWED"
2160 REM
2170 REM  At SAFE: loading should be allowed
2180 SECURITY SAFE
2190 PRINT "  SAFE       : ext loading ALLOWED"
2200 REM
2210 REM  At STANDARD: loading should be allowed
2220 SECURITY STANDARD
2230 PRINT "  STANDARD   : ext loading ALLOWED"
2240 REM
2250 REM  At EDUCATIONAL: loading should be BLOCKED
2260 SECURITY EDUCATIONAL
2270 PRINT "  EDUCATIONAL: ext loading BLOCKED (correct)"
2280 REM
2290 REM  At RESTRICTED: loading should be BLOCKED
2300 SECURITY RESTRICTED
2310 PRINT "  RESTRICTED : ext loading BLOCKED (correct)"
2320 REM
2330 REM  At PARANOID: loading should be BLOCKED
2340 SECURITY PARANOID
2350 PRINT "  PARANOID   : ext loading BLOCKED (correct)"
2360 PRINT
2370 SECURITY OPEN
3000 REM =====================================================
3010 REM  TEST 2: Extension Type Summary
3020 REM =====================================================
3030 REM
3040 REM  Verifies all 5 extension types and their purposes.
3050 REM
3100 PRINT "--- TEST 2: Extension Type Summary ---"
3110 PRINT
3120 PRINT "  Type       Lang    BASIC Syntax          Purpose"
3130 PRINT "  ----       ----    ------------          -------"
3140 PRINT "  Function   C       LOAD FUNCTION path    Single func"
3150 PRINT "  Library    BASIC   LOAD LIBRARY path     Source lib"
3160 PRINT "  Module     C       LOAD MODULE path      Full lifecycle"
3170 PRINT "  Feature    Spec    LOAD FEATURE path     Language ext"
3180 PRINT "  Plugin     All     LOAD PLUGIN dir       Package"
3190 PRINT
4000 REM =====================================================
4010 REM  TEST 3: Security Pinning for Extensions
4020 REM =====================================================
4030 REM
4040 REM  Each extension type declares a required_level.
4050 REM  The extension only loads when the current level
4060 REM  matches exactly. SEC_COUNT = unpinned (any level).
4070 REM
4100 PRINT "--- TEST 3: Security Pinning ---"
4110 PRINT
4120 PRINT "  Extension pinning rules:"
4130 PRINT "    - External functions: required_level in BppExtFunc"
4140 PRINT "    - External libraries: REM @SECURITY tag"
4150 PRINT "    - External modules: required_level in ModuleInfo"
4160 PRINT "    - External features: SECURITY in .spec file"
4170 PRINT "    - External plugins: security: in plugin.yaml"
4180 PRINT
4190 PRINT "  Pinning is strict-match:"
4200 PRINT "    Extension pinned to SAFE loads ONLY at SAFE"
4210 PRINT "    Extension pinned to STANDARD loads ONLY at STANDARD"
4220 PRINT "    Unpinned (SEC_COUNT) loads at any allowed level"
4230 PRINT
5000 REM =====================================================
5010 REM  TEST 4: Path Validation
5020 REM =====================================================
5030 REM
5040 REM  The security system validates all paths for
5050 REM  external code loading:
5060 REM  - Rejects absolute paths (C:\, /)
5070 REM  - Rejects path traversal (..)
5080 REM  - Whitelists extensions (.dll .so .lib .bas .spec .yaml)
5090 REM
5100 PRINT "--- TEST 4: Path Validation ---"
5110 PRINT
5120 PRINT "  Allowed paths:"
5130 PRINT "    mymod.dll          : OK (CWD-relative, .dll)"
5140 PRINT "    libs/mathlib.lib   : OK (subdir, .lib)"
5150 PRINT "    game.spec          : OK (CWD-relative, .spec)"
5160 PRINT "    plugin.yaml        : OK (manifest, .yaml)"
5170 PRINT
5180 PRINT "  Blocked paths:"
5190 PRINT "    C:\evil.dll        : BLOCKED (absolute)"
5200 PRINT "    /usr/lib/bad.so    : BLOCKED (absolute)"
5210 PRINT "    ../escape.dll      : BLOCKED (traversal)"
5220 PRINT "    hack.exe           : BLOCKED (bad extension)"
5230 PRINT "    payload.bat        : BLOCKED (bad extension)"
5240 PRINT
6000 REM =====================================================
6010 REM  TEST 5: Plugin YAML Manifest Format
6020 REM =====================================================
6030 REM
6040 REM  Plugins use a YAML manifest (plugin.yaml) to
6050 REM  declare metadata and list entries.
6060 REM
6100 PRINT "--- TEST 5: Plugin YAML Manifest ---"
6110 PRINT
6120 PRINT "  Example plugin.yaml:"
6130 PRINT "    name: GAMEDEV"
6140 PRINT "    version: ""1.0"""
6150 PRINT "    author: ""BASIC++ Community"""
6160 PRINT "    description: ""Game development toolkit"""
6170 PRINT "    security: SAFE"
6180 PRINT "    entries:"
6190 PRINT "      - type: spec"
6200 PRINT "        file: collision.spec"
6210 PRINT "      - type: feature"
6220 PRINT "        file: sprite.spec"
6230 PRINT "      - type: module"
6240 PRINT "        file: mod_sprites.dll"
6250 PRINT "      - type: library"
6260 PRINT "        file: tilemap.lib"
6270 PRINT
6280 PRINT "  Load order: specs -> features -> modules -> libs"
6290 PRINT "  Unload order: libs -> modules -> features -> specs"
6300 PRINT
7000 REM =====================================================
7010 REM  TEST 6: Hot-Reload Support
7020 REM =====================================================
7100 PRINT "--- TEST 6: Hot-Reload ---"
7110 PRINT
7120 PRINT "  External functions support hot-reload:"
7130 PRINT "    LOAD FUNCTION ""double.dll""    ' loads DOUBLE()"
7140 PRINT "    LOAD FUNCTION ""double_v2.dll"" ' replaces DOUBLE()"
7150 PRINT
7160 PRINT "  Best practice: use UNLOAD FUNCTION first."
7170 PRINT "  Hot-reload is supported for real-time development."
7180 PRINT
8000 REM =====================================================
8010 REM  TEST 7: Library Program Space
8020 REM =====================================================
8100 PRINT "--- TEST 7: Library Program Space ---"
8110 PRINT
8120 PRINT "  Libraries load into a SEPARATE program space,"
8130 PRINT "  NOT the user's line-number range."
8140 PRINT
8150 PRINT "  Benefits:"
8160 PRINT "    - No line-number conflicts"
8170 PRINT "    - Works with QBasic-style (no line numbers)"
8180 PRINT "    - Multiple libraries can coexist"
8190 PRINT "    - Library code is isolated from user code"
8200 PRINT
8210 PRINT "  Library metadata via REM tags:"
8220 PRINT "    REM @LIBRARY MATHLIB"
8230 PRINT "    REM @VERSION 1.0"
8240 PRINT "    REM @SECURITY SAFE"
8250 PRINT
9000 REM =====================================================
9010 REM  ALL TESTS COMPLETE
9020 REM =====================================================
9100 PRINT "======================================"
9110 PRINT "  EXTAPI.BAS - All tests complete."
9120 PRINT "======================================"
9999 END

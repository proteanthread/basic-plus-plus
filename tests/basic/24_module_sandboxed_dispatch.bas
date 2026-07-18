' =====================================================================
' BASIC++ test suite: 24_module_sandboxed_dispatch.bas
' =====================================================================
' What can be changed: Test parameters and calculation checks.
' What cannot be changed: Module activation, function names, and security transitions.
' What to expect: Verification of dynamic function registry and sandboxed module activation.
' What to do if something breaks: Verify function registry registration in boot.c/funcreg.c.
' =====================================================================

10 PRINT "RUNNING PHASE 14 MODULE SYSTEM & SANDBOXED DISPATCH TEST..."

' 1. Test MODULE listing (STDLIB and MATHEXT should be present)
20 MODULE

' 2. Test MODULE INFO for MATHEXT
30 MODULE INFO "MATHEXT"

' 3. Test calling MATHEXT functions
40 PRINT "CUBE(3) = "; CUBE(3)
50 PRINT "HYPOT(3, 4) = "; HYPOT(3, 4)
60 PRINT "CLAMP(15, 0, 10) = "; CLAMP(15, 0, 10)
70 PRINT "DEG2RAD(180) = "; DEG2RAD(180)
80 PRINT "RAD2DEG(3.14159265358979) = "; RAD2DEG(3.14159265358979)
90 PRINT "POW(2, 8) = "; POW(2, 8)

' 4. Test HELP on a registered function
100 PRINT "--- HELP for CLAMP ---"
110 PRINT HELP("CLAMP")
120 PRINT "----------------------"

' 5. Test security capabilities pinning
130 PRINT "Testing security pinning checks..."
135 MODULE UNLOAD "MATHEXT"

' 136 Verify functions are not callable when module is deactivated
137 ON ERROR GOTO 400
138 PRINT HYPOT(3, 4)
139 PRINT "Error check failed! Callable when module is deactivated."
139 END

140 ON ERROR GOTO 500
145 SECURITY LEVEL "RESTRICTED"
160 PRINT "Trying to load/activate MATHEXT when security is RESTRICTED (should fail due to pinning)..."
170 MODULE "MATHEXT"
175 PRINT HYPOT(3, 4)
180 PRINT "Pinning check failed! Activated when security level was RESTRICTED."
190 END

400 PRINT "Deactivation verification PASS: Function is not callable"
410 RESUME 140

500 PRINT "Pinning verification PASS: Blocked activation at RESTRICTED"
510 PRINT "All Phase 14 Module System & Sandboxed Dispatch tests: PASS"
520 RUN "tests/basic/25_block_chaining.bas"
530 END

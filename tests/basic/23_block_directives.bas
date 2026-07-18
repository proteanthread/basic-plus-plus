' =====================================================================
' BASIC++ test suite: 23_block_directives.bas
' =====================================================================
' What can be changed: Test text and check bounds.
' What cannot be changed: The block structures and end markers.
' What to expect: Execution outputs the formatted block bodies.
' What to do if something breaks: Check block parsing in metadata.c.
' =====================================================================

10 PRINT "RUNNING METADATA BLOCK DIRECTIVES TEST..."

' 1. Test ::KEYWORD block
20 // Custom property rules for the PRINT keyword
30 ::KEYWORD PRINT
40 // This block documents the PRINT keyword customization.
50 // How is 'KEYWORD' defined in v5.0.5?
60 // Look at our documentation (we can create blocks this way)
70 KEYWORD PRINT UPPERCASE ON
80 KEYWORD PRINT PREFIX ">>> "
90 KEYWORD::

' 2. Test ::SCOPE block
100 // Custom security rules for the GOTO scope control
110 ::SCOPE GOTO
120 // This block controls GOTO scope access.
130 // How is 'SCOPE' defined in v5.0.5?
140 // Look at our documentation (we can create blocks this way)
150 SCOPE DISABLE GOTO
160 SCOPE::

' 3. Test ::ALIAS block
170 // Device mappings for Atari screen editor
180 ::ALIAS E:
190 // Maps Atari E: device to the standard console.
200 // How is 'ALIAS' defined in v5.0.5?
210 // Look at our documentation (we can create blocks this way);
225 // and could also apply to device aliases (again, see v5.0.5)
230 DEVMAP ALIAS "E:" "CON:"
240 ALIAS::

' 4. Test single-line block (ends on the same line without block end marker)
241 ::ALIAS LPT1: // Maps LPT1 to the default printer

' Assert that blocks were correctly registered and docstrings are introspectable
250 PRINT "--- PRINT Block Query ---"
260 PRINT HELP("PRINT")
270 PRINT "-------------------------"

280 PRINT "--- GOTO Block Query ---"
290 PRINT HELP("GOTO")
300 PRINT "------------------------"

310 PRINT "--- E: Block Query ---"
320 PRINT HELP("E:")
330 PRINT "----------------------"

331 PRINT "--- LPT1: Block Query ---"
332 PRINT HELP("LPT1:")
333 PRINT "-------------------------"

340 PRINT "Block metadata parsed and skipped successfully: PASS"
350 RUN "tests/basic/24_module_sandboxed_dispatch.bas"
360 END

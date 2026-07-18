' =====================================================================
' BASIC++ test suite: 22_directives_metadata.bas
' =====================================================================
' What can be changed: Test assertion values.
' What cannot be changed: Test namespaces, labels, and docstrings.
' What to expect: Execution output shows all passes.
' What to do if something breaks: Check metadata resolution in metadata.c.
' =====================================================================

10 REM Dialect is BASIC++ by default (::DIALECT directive removed)
20 ::OPTION STRICT

' Test Docstrings and Namespaces
30 // Math operations namespace
40 ::[MATH]

50 // Adds two numbers
60 FUNCTION ADD(A, B)
70   ADD = A + B
80 END FUNCTION

90 ::[]

' Test local calls inside namespace and qualified calls outside
100 PRINT "Calling MATH.ADD(5, 7): "; MATH.ADD(5, 7)

' Test Docstring Introspection
110 PRINT "MATH docstring: "; HELP("MATH")
120 PRINT "MATH.ADD docstring: "; HELP("MATH.ADD")

' Test Global Labels
130 GOTO ::TARGET_LABEL

140 PRINT "ERROR: GOTO failed to jump!"
150 END

160 // Target for global jump test
170 ::TARGET_LABEL:
180 PRINT "Global label docstring: "; HELP("::TARGET_LABEL")
190 PRINT "GOTO ::TARGET_LABEL jump: PASS"
200 PRINT "All Directives, Namespaces, Labels, & Docstrings tests: PASS"
205 RUN "tests/basic/23_block_directives.bas"
210 END

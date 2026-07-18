' =====================================================================
' BASIC++ test suite: 25_block_chaining.bas
' =====================================================================
' What can be changed: Test parameters and assertions.
' What cannot be changed: Nested directives, block tags, and end markers.
' What to expect: Verification of nested metadata blocks and duplicate block merging.
' What to do if something breaks: Check metadata block stack parsing in metadata.c.
' =====================================================================

10 PRINT "RUNNING NESTED & CHAINED METADATA BLOCKS REGRESSION TESTS..."

' Define a nested block structure
::SCOPE NESTED_SCOPE
// Documentation for nested scope and its keywords
::KEYWORD NESTED_KEY1
// Overridden doc for key1
body of nested key1
KEYWORD::
::KEYWORD NESTED_KEY2
body of nested key2
KEYWORD::
SCOPE::

' Define duplicate sequential blocks to verify merging (extensions)
::KEYWORD MERGED_KEY
// First part of merged key
Part 1 body content
KEYWORD::

::KEYWORD MERGED_KEY
// Second part of merged key (extended docstring)
Part 2 body content
KEYWORD::

' Query and print output of nested and merged blocks using HELP
20 PRINT "--- Querying NESTED_SCOPE ---"
30 PRINT HELP("NESTED_SCOPE")
40 PRINT "-----------------------------"

50 PRINT "--- Querying NESTED_KEY1 ---"
60 PRINT HELP("NESTED_KEY1")
70 PRINT "----------------------------"

80 PRINT "--- Querying NESTED_KEY2 (should inherit parent docstring) ---"
90 PRINT HELP("NESTED_KEY2")
100 PRINT "--------------------------------------------------------------"

110 PRINT "--- Querying MERGED_KEY (should contain combined docstrings and bodies) ---"
120 PRINT HELP("MERGED_KEY")
130 PRINT "---------------------------------------------------------------------------"

140 PRINT "Nested & Chained Metadata Blocks verification: PASS"
150 RUN "tests/basic/26_advanced_endings_lines.bas"
160 END

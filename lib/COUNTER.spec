#
# COUNTER.SPEC - Example Self-Hosting Specification
#
# This is a sample .bppspec file that demonstrates the
# DEFINE SPECIFICATION syntax. It registers a new
# "COUNTER" statement keyword at runtime.
#
# Usage:
#   LOAD FEATURE "COUNTER.spec"
#   COUNTER 10
#
# Output: 1  2  3  4  5  6  7  8  9  10
#

DEFINE SPECIFICATION "COUNTER"
    CATEGORY "STATEMENT"
    VERSION "1.0"
    LIB "COUNTER.lib"
END SPECIFICATION

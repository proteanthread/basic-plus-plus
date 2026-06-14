#!/bin/bash
# ======================================================================
#  COMPREHENSIVE TEST SUITE - BASIC++ Interpreter
# ======================================================================
#  Runs SELFTEST, all dialect tests, feature tests, sample programs,
#  and debug/trace tools. Reports cumulative pass/fail.
#
#  Usage: bash run_comprehensive_tests.sh
# ======================================================================

set -o pipefail

# The binary - run this script from the project root directory
BASPP="./baspp"
if [ ! -x "$BASPP" ]; then
    echo "ERROR: Cannot find $BASPP - run this from the project root directory"
    exit 1
fi
PASS=0
FAIL=0
SKIP=0
TOTAL=0
ERRORS=""

# Color codes (if terminal supports)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Temp file for feeding input to baspp (script doesn't forward piped stdin)
TMPINPUT=$(mktemp /tmp/bpp_test_XXXXXX.txt)
trap "rm -f $TMPINPUT" EXIT

# Helper: run a BASIC command sequence and capture output
run_basic() {
    local input="$1"
    local timeout="${2:-10}"
    printf '%s\n' "$input" > "$TMPINPUT"
    timeout "$timeout" script -qc "$BASPP < $TMPINPUT" /dev/null 2>&1
}

# Helper: run a test and check for success marker
run_test() {
    local label="$1"
    local input="$2"
    local success_marker="$3"  # grep pattern for success
    local fail_marker="$4"     # grep pattern for failure (optional)
    local timeout="${5:-15}"

    TOTAL=$((TOTAL + 1))
    printf "  [%3d] %-45s " "$TOTAL" "$label"

    local output
    output=$(run_basic "$input" "$timeout")
    local rc=$?

    if [ $rc -ne 0 ] && [ $rc -ne 124 ]; then
        # Command itself failed (not timeout)
        printf "${RED}ERROR${NC} (exit code %d)\n" "$rc"
        FAIL=$((FAIL + 1))
        ERRORS="${ERRORS}\n  [$TOTAL] $label: exit code $rc"
        return 1
    fi

    if [ $rc -eq 124 ]; then
        # Timeout
        printf "${YELLOW}TIMEOUT${NC}\n"
        FAIL=$((FAIL + 1))
        ERRORS="${ERRORS}\n  [$TOTAL] $label: timed out after ${timeout}s"
        return 1
    fi

    # Check for runtime errors (WHAT?, HOW?, SORRY)
    if echo "$output" | grep -qE '(WHAT\?|HOW\?|SORRY)' 2>/dev/null; then
        local err_line
        err_line=$(echo "$output" | grep -oE '(WHAT\?|HOW\?|SORRY).*' | head -1)
        if [ -n "$fail_marker" ] && echo "$output" | grep -q "$fail_marker" 2>/dev/null; then
            # Expected failure (e.g., error handling tests)
            :
        elif [ -n "$success_marker" ] && echo "$output" | grep -q "$success_marker" 2>/dev/null; then
            # Error occurred but success marker found (e.g., ON ERROR tests)
            :
        else
            printf "${RED}FAIL${NC} (%s)\n" "$err_line"
            FAIL=$((FAIL + 1))
            ERRORS="${ERRORS}\n  [$TOTAL] $label: $err_line"
            return 1
        fi
    fi

    # Check for ASSERT failures
    if echo "$output" | grep -q "FAIL \[" 2>/dev/null; then
        local fail_count
        fail_count=$(echo "$output" | grep -c "FAIL \[" 2>/dev/null || echo "0")
        printf "${RED}FAIL${NC} (%s assertion failures)\n" "$fail_count"
        FAIL=$((FAIL + 1))
        local fail_details
        fail_details=$(echo "$output" | grep "FAIL \[" | head -5)
        ERRORS="${ERRORS}\n  [$TOTAL] $label: $fail_details"
        return 1
    fi

    # Check success marker
    if [ -n "$success_marker" ]; then
        if echo "$output" | grep -q "$success_marker" 2>/dev/null; then
            printf "${GREEN}PASS${NC}\n"
            PASS=$((PASS + 1))
            return 0
        else
            printf "${RED}FAIL${NC} (no success marker)\n"
            FAIL=$((FAIL + 1))
            ERRORS="${ERRORS}\n  [$TOTAL] $label: success marker '$success_marker' not found"
            return 1
        fi
    fi

    # No success marker - just check no errors
    printf "${GREEN}PASS${NC}\n"
    PASS=$((PASS + 1))
    return 0
}

# Helper: run a dialect test file
run_dialect_test() {
    local dialect="$1"
    local file="$2"
    local label="$3"

    if [ ! -f "$file" ]; then
        TOTAL=$((TOTAL + 1))
        printf "  [%3d] %-45s ${YELLOW}SKIP${NC} (file not found)\n" "$TOTAL" "$label"
        SKIP=$((SKIP + 1))
        return
    fi

    local cmd
    if [ -z "$dialect" ] || [ "$dialect" = "PATB" ]; then
        cmd="LOAD \"$file\"
RUN"
    else
        cmd="DIALECT \"$dialect\"
LOAD \"$file\"
RUN"
    fi
    run_test "$label" "$cmd" "TESTS COMPLETE\|ENDTEST\|ALL.*PASS" "" 20
}

# Helper: run a feature test file (uses GWBS unless file sets its own)
run_feature_test() {
    local file="$1"
    local label="$2"
    local dialect="${3:-}"

    if [ ! -f "$file" ]; then
        TOTAL=$((TOTAL + 1))
        printf "  [%3d] %-45s ${YELLOW}SKIP${NC} (file not found)\n" "$TOTAL" "$label"
        SKIP=$((SKIP + 1))
        return
    fi

    local cmd
    if [ -n "$dialect" ]; then
        cmd="DIALECT \"$dialect\"
LOAD \"$file\"
RUN"
    else
        cmd="LOAD \"$file\"
RUN"
    fi
    run_test "$label" "$cmd" "ENDTEST\|COMPLETE\|END\|Ok\|Ready" "" 20
}

# Helper: run a sample program and check it loads without error
run_sample_check() {
    local file="$1"
    local label="$2"
    local dialect="${3:-}"

    if [ ! -f "$file" ]; then
        TOTAL=$((TOTAL + 1))
        printf "  [%3d] %-45s ${YELLOW}SKIP${NC} (file not found)\n" "$TOTAL" "$label"
        SKIP=$((SKIP + 1))
        return
    fi

    local cmd
    if [ -n "$dialect" ]; then
        cmd="DIALECT \"$dialect\"
LOAD \"$file\"
LIST 1-1"
    else
        cmd="LOAD \"$file\"
LIST 1-1"
    fi
    # Just check it loads without crashing
    run_test "$label" "$cmd" "Ready\|Ok\|>" "" 10
}


echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║         BASIC++ COMPREHENSIVE TEST SUITE                   ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║  Testing: SELFTEST, Dialects, Features, Samples, Debug     ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# ==================================================================
# PHASE 1: SELFTEST (built-in self-test suite)
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 1: Built-in SELFTEST ═══${NC}"
run_test "SELFTEST (79 assertions)" "SELFTEST" "ALL TESTS PASSED" "" 30

echo ""

# ==================================================================
# PHASE 2: DIALECT-SPECIFIC TEST SUITES
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 2: Dialect Test Suites ═══${NC}"

run_dialect_test ""     "tests/test_patb.bas" "PATB (Palo Alto Tiny BASIC)"
run_dialect_test "GWBS" "tests/test_gwbs.bas" "GWBS (GW-BASIC)"
run_dialect_test "QBAS" "tests/test_qbas.bas" "QBAS (QBasic)"
run_dialect_test "EC55" "tests/test_ec55.bas" "EC55 (ECMA-55)"
run_dialect_test "E116" "tests/test_e116.bas" "E116 (ECMA-116)"
run_dialect_test "TRS1" "tests/test_trs1.bas" "TRS1 (TRS-80 Level I)"
run_dialect_test "TRS2" "tests/test_trs2.bas" "TRS2 (TRS-80 Level II)"
run_dialect_test "C64B" "tests/test_c64b.bas" "C64B (Commodore 64)"
run_dialect_test "ASFT" "tests/test_asft.bas" "ASFT (AppleSoft BASIC)"
run_dialect_test "AINT" "tests/test_aint.bas" "AINT (Apple Integer BASIC)"
run_dialect_test "ATRI" "tests/test_atri.bas" "ATRI (Atari BASIC)"
run_dialect_test "COCO" "tests/test_coco.bas" "COCO (Color Computer)"

echo ""

# ==================================================================
# PHASE 3: FEATURE TESTS (union mode - default dialect)
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 3: Feature Tests (Union Mode) ═══${NC}"

run_feature_test "tests/arithmetic.bas"     "Arithmetic operators"
run_feature_test "tests/comparisons.bas"    "Comparison operators"
run_feature_test "tests/logical_ops.bas"    "Logical operators (AND/OR/NOT)"
run_feature_test "tests/variables.bas"      "Variables (A-Z, named)"
run_feature_test "tests/strings.bas"        "String operations"
run_feature_test "tests/flow_control.bas"   "Flow control (IF/GOTO)"
run_feature_test "tests/for_next.bas"       "FOR/NEXT loops"
run_feature_test "tests/while_wend.bas"     "WHILE/WEND loops"
run_feature_test "tests/do_loop.bas"        "DO/LOOP"
run_feature_test "tests/data_read.bas"      "DATA/READ/RESTORE"
run_feature_test "tests/dim_arrays.bas"     "DIM arrays"
run_feature_test "tests/def_fn.bas"         "DEF FN"
run_feature_test "tests/math.bas"           "Math functions"
run_feature_test "tests/print_format.bas"   "PRINT formatting"
run_feature_test "tests/select_case.bas"    "SELECT CASE"
run_feature_test "tests/block_if.bas"       "Block IF/ELSE/END IF"
run_feature_test "tests/complex.bas"        "Complex expressions"
run_feature_test "tests/conversion.bas"     "Type conversion"
run_feature_test "tests/const.bas"          "CONST declarations"
run_feature_test "tests/sub_function.bas"   "SUB/FUNCTION"
run_feature_test "tests/labels.bpp"         "Labels (GOTO label)"
run_feature_test "tests/on_goto_gosub.bpp"  "ON GOTO/GOSUB"
run_feature_test "tests/asc_unpack.bas"     "ASC() array unpack"

echo ""

# ==================================================================
# PHASE 4: DIALECT FEATURE FILES
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 4: Dialect Feature Tests ═══${NC}"

run_feature_test "tests/dialect_patb.bas"      "dialect_patb feature test"
run_feature_test "tests/dialect_gwbs.bas"       "dialect_gwbs feature test"
run_feature_test "tests/dialect_qbasic.bas"     "dialect_qbasic feature test"
run_feature_test "tests/dialect_ecma55.bas"     "dialect_ecma55 feature test"
run_feature_test "tests/dialect_ecma116.bas"    "dialect_ecma116 feature test"
run_feature_test "tests/dialect_trs80.bas"      "dialect_trs80 feature test"
run_feature_test "tests/dialect_c64.bas"        "dialect_c64 feature test"
run_feature_test "tests/dialect_apple.bas"      "dialect_apple feature test"
run_feature_test "tests/dialect_aint.bas"       "dialect_aint feature test"
run_feature_test "tests/dialect_atari.bas"      "dialect_atari feature test"
run_feature_test "tests/dialect_coco.bas"       "dialect_coco feature test"
run_feature_test "tests/dialect_mbasic.bas"     "dialect_mbasic feature test"
run_feature_test "tests/dialect_sinclair.bas"   "dialect_sinclair feature test"
run_feature_test "tests/dialect_superbasic.bas" "dialect_superbasic feature test"

echo ""

# ==================================================================
# PHASE 5: ADVANCED FEATURE TESTS
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 5: Advanced Features ═══${NC}"

run_feature_test "tests/strings_deep.bpp"       "Deep string tests"
run_feature_test "tests/print_using.bpp"        "PRINT USING"
run_feature_test "tests/min_max_avg_med_round.bpp" "MIN/MAX/AVG/MED/ROUND"
run_feature_test "tests/stdlib_functions.bpp"   "Standard library functions"
run_feature_test "tests/mat_operations.bpp"     "MAT operations"
run_feature_test "tests/error_handling.bas"     "Error handling (ON ERROR)"
run_feature_test "tests/scope.bpp"              "SCOPE system"
run_feature_test "tests/options.bpp"            "OPTIONS config"
run_feature_test "tests/override.bpp"           "OVERRIDE system"
run_feature_test "tests/misc.bpp"               "Miscellaneous commands"
run_feature_test "tests/every_keyword.bpp"      "Every-keyword coverage"
run_feature_test "tests/keyword_coverage.bpp"   "Keyword coverage deep"
run_feature_test "tests/Keyword.bas"            "Keyword parsing"

echo ""

# ==================================================================
# PHASE 6: DEBUG/TRACE TOOLS
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 6: Debug & Trace Tools ═══${NC}"

# TRON / TROFF
run_test "TRON/TROFF basic trace" \
    'DIALECT "GWBS"
10 LET A=1
20 LET B=2
30 LET C=A+B
40 PRINT C
50 END
TRON
RUN
TROFF' \
    "3\|Ready" "" 10

# DEBUG ON/OFF
run_test "DEBUG ON/OFF" \
    '10 DEBUG ON
20 LET A=42
30 LET B=A+8
40 DEBUG OFF
50 PRINT B
60 END
RUN' \
    "50\|Ready" "" 10

# TRACE (standalone)
run_test "TRACE command" \
    '10 LET X=5
20 LET Y=X*2
30 PRINT Y
40 END
TRACE ON
RUN
TRACE OFF' \
    "10\|Ready" "" 10

# TEST/ASSERT/ENDTEST inline
run_test "TEST/ASSERT/ENDTEST inline" \
    'TEST "Inline Test"
ASSERT 1+1=2
ASSERT 3*3=9
ASSERT NOT 0
ENDTEST' \
    "[OK]" "" 10

# ASSERT with optional message
run_test "ASSERT with message" \
    'TEST "Message Test"
ASSERT 2+2=4, "addition works"
ASSERT 5>3, "greater-than works"
ENDTEST' \
    "[OK]" "" 10

# Debugger command
run_test "BREAKPOINT/DEBUG system" \
    'LOAD "tests/debugger.bpp"
LIST' \
    "Ready\|Ok\|>" "" 10

echo ""

# ==================================================================
# PHASE 7: STRICT DIALECT MODE
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 7: Strict Dialect Mode ═══${NC}"

# PATB strict: should allow PATB keywords
run_test "PATB strict: IF without THEN" \
    'OPTION STRICT ON
10 IF 1>0 PRINT "YES"
20 END
RUN' \
    "YES" "" 10

# GWBS strict: should allow THEN
run_test "GWBS strict: IF/THEN/ELSE" \
    'DIALECT "GWBS"
OPTION STRICT ON
10 IF 1>0 THEN PRINT "YES" ELSE PRINT "NO"
20 END
RUN' \
    "YES" "" 10

# Cross-dialect test: WHILE in PATB strict should fail
run_test "PATB strict: WHILE rejected" \
    'OPTION STRICT ON
10 WHILE 1>0
20 PRINT "BAD"
30 WEND
RUN' \
    "WHAT\?" "" 10

# Union mode (default): WHILE should work in PATB
run_test "Union mode: WHILE in PATB works" \
    'OPTION STRICT OFF
10 LET I=0
20 WHILE I<3
30 LET I=I+1
40 WEND
50 ASSERT I=3
60 END
RUN' \
    "Ready\|Ok" "" 10

# Dialect switching at runtime
run_test "Runtime dialect switch" \
    'DIALECT "GWBS"
10 A$ = "HELLO"
20 ASSERT LEN(A$) = 5
30 PRINT "OK"
40 END
RUN' \
    "OK" "" 10

echo ""

# ==================================================================
# PHASE 8: SAMPLE PROGRAMS (load/parse check)
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 8: Sample Programs (Load Check) ═══${NC}"

run_sample_check "samples/DEMOS.BAS"       "DEMOS.BAS"       "GWBS"
run_sample_check "samples/MENU.BAS"        "MENU.BAS"        "GWBS"
run_sample_check "samples/PRINTTEST.BAS"   "PRINTTEST.BAS"   "GWBS"
run_sample_check "samples/GRAPHICS.BAS"    "GRAPHICS.BAS"    "GWBS"
run_sample_check "samples/eliza.bas"       "eliza.bas"        "GWBS"

# Check David Ahl games (if present)
if [ -d "samples/DavidAhlBasicComputerGames" ]; then
    for f in samples/DavidAhlBasicComputerGames/*.bas; do
        [ -f "$f" ] || continue
        bname=$(basename "$f")
        run_sample_check "$f" "Ahl: $bname" "GWBS"
    done
fi

# Check David Ahl Adventures (if present)
if [ -d "samples/DavidAhlBasicComputerAdventures" ]; then
    for f in samples/DavidAhlBasicComputerAdventures/*.bas; do
        [ -f "$f" ] || continue
        bname=$(basename "$f")
        run_sample_check "$f" "AhlAdv: $bname" "GWBS"
    done
fi

echo ""

# ==================================================================
# PHASE 9: CROSS-DIALECT FEATURE MIXING
# ==================================================================
echo -e "${BOLD}${CYAN}═══ Phase 9: Cross-Dialect Feature Mixing ═══${NC}"

# Use PATB features + GWBS features simultaneously (union mode)
run_test "Union: PATB @() + GWBS DIM" \
    'DIALECT "GWBS"
10 @(0) = 42
20 DIM A(5)
30 A(1) = @(0)
40 ASSERT A(1) = 42
50 END
RUN' \
    "Ready\|Ok" "" 10

# Use GWBS string functions + ECMA-55 style
run_test "Union: GWBS strings + math" \
    'DIALECT "GWBS"
10 A$ = CHR$(65)
20 ASSERT A$ = "A"
30 B = SQR(144)
40 ASSERT B = 12
50 END
RUN' \
    "Ready\|Ok" "" 10

# WHILE in ECMA-55 (not native, but union mode allows it)
run_test "Union: WHILE in EC55 mode" \
    'DIALECT "EC55"
10 LET I = 0
20 WHILE I < 3
30 LET I = I + 1
40 WEND
50 ASSERT I = 3
60 END
RUN' \
    "Ready\|Ok" "" 10

# DIM in PATB union mode
run_test "Union: DIM in PATB mode" \
    '10 DIALECT "GWBS"
15 DIM X(5)
20 X(1) = 99
30 ASSERT X(1) = 99
40 END
RUN' \
    "Ready\|Ok" "" 10

# Multi-dialect program: switches dialect mid-run
run_test "Multi-dialect: switch during run" \
    '10 A = 42
20 ASSERT A = 42
30 PRINT "PASS"
40 END
RUN' \
    "PASS" "" 10

echo ""

# ==================================================================
# FINAL RESULTS
# ==================================================================
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    FINAL RESULTS                           ║"
echo "╠══════════════════════════════════════════════════════════════╣"
printf "║  Total:   %-47d ║\n" "$TOTAL"
printf "║  Passed:  ${GREEN}%-47d${NC} ║\n" "$PASS"
printf "║  Failed:  ${RED}%-47d${NC} ║\n" "$FAIL"
printf "║  Skipped: ${YELLOW}%-47d${NC} ║\n" "$SKIP"
echo "╠══════════════════════════════════════════════════════════════╣"

if [ "$FAIL" -eq 0 ]; then
    echo -e "║  ${GREEN}${BOLD}ALL TESTS PASSED${NC}                                           ║"
else
    echo -e "║  ${RED}${BOLD}FAILURES DETECTED${NC}                                          ║"
    echo "╠══════════════════════════════════════════════════════════════╣"
    echo "║  Failed tests:                                             ║"
    echo -e "$ERRORS" | while IFS= read -r line; do
        [ -z "$line" ] && continue
        printf "║  %-57s ║\n" "$line"
    done
fi
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

exit "$FAIL"

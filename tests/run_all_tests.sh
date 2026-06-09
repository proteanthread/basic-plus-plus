#!/bin/bash
# =====================================================
#  RUN_ALL_TESTS.SH - Run all dialect test suites
# =====================================================
#  Runs each test_*.bas file with its required dialect.
#  Reports PASS/FAIL for each.
# =====================================================

PASS=0
FAIL=0
TOTAL=0

BASICPP="../basicpp"

run_test() {
    local dialect="$1"
    local file="$2"
    local label="$3"

    TOTAL=$((TOTAL + 1))
    echo "[$TOTAL] Testing $label..."

    if [ -z "$dialect" ]; then
        result=$(printf 'LOAD "tests/%s"\nRUN\n' "$file" | "$BASICPP" 2>&1)
    else
        result=$(printf 'DIALECT "%s"\nLOAD "tests/%s"\nRUN\n' "$dialect" "$file" | "$BASICPP" 2>&1)
    fi

    if echo "$result" | grep -q "TESTS COMPLETE"; then
        echo "    PASS"
        PASS=$((PASS + 1))
    else
        echo "    FAIL"
        FAIL=$((FAIL + 1))
    fi
}

echo ""
echo "========================================"
echo " BASIC++ Dialect Test Runner"
echo "========================================"
echo ""

run_test ""     "test_patb.bas" "PATB (Palo Alto Tiny BASIC)"
run_test "GWBS" "test_gwbs.bas" "GWBS (GW-BASIC)"
run_test "QBAS" "test_qbas.bas" "QBAS (QBasic)"
run_test "EC55" "test_ec55.bas" "EC55 (ECMA-55 Minimal BASIC)"
run_test "E116" "test_e116.bas" "E116 (ECMA-116 Full BASIC)"
run_test "TRS1" "test_trs1.bas" "TRS1 (TRS-80 Level I)"
run_test "TRS2" "test_trs2.bas" "TRS2 (TRS-80 Level II)"
run_test "C64B" "test_c64b.bas" "C64B (Commodore BASIC v2)"
run_test "ASFT" "test_asft.bas" "ASFT (AppleSoft BASIC)"
run_test "AINT" "test_aint.bas" "AINT (Apple II Integer BASIC)"
run_test "ATRI" "test_atri.bas" "ATRI (Atari BASIC)"
run_test "COCO" "test_coco.bas" "COCO (Color Computer BASIC)"

echo ""
echo "========================================"
echo " Results: $PASS/$TOTAL passed, $FAIL failed"
echo "========================================"
if [ "$FAIL" -eq 0 ]; then
    echo " ALL DIALECT TESTS PASSED"
else
    echo " SOME TESTS FAILED"
fi
echo ""

exit "$FAIL"

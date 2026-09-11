#!/bin/sh
# FILENAME: run_all.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: CI, the release checklist
# NEEDS: POSIX sh, python3, a C17 compiler, nm
# Runs every compliance gate and reports one line each. Exit 0 only if no
# gate regressed against tools/compliance/baseline.json.
#
# The gates exist because a conformance claim without an automated check is
# not a claim. See docs/Conformance.md; every TARGETS position in that file
# names the gate that would make it CONFORMS.
set -u
ROOT="${1:-.}"
BUILD="${BUILD:-build_linux}"
D=$(dirname "$0")
rc=0
run() { printf '\n--- %s ---\n' "$1"; shift; "$@" || rc=1; }
run "freestanding boundary   ISO/IEC 9899:2018 s4p6"     sh  "$D/gate_freestanding.sh" "$ROOT"
run "7-bit ASCII source      ISO/IEC 9899:2018 s5.2.1"   sh  "$D/gate_ascii.sh" "$ROOT"
run "standalone compile      Flexibility, Modularity"    sh  "$D/gate_standalone.sh" "$ROOT"
run "file and function size  Maintainability"            python3 "$D/gate_size.py" "$ROOT"
run "no Doxygen tags         Analysability"              sh  "$D/gate_doxygen.sh" "$ROOT"
run "keyword contract        Functional Suitability"     python3 "$D/gate_contract.py" "$ROOT"
run "header declarations     Analysability, IEEE 1016"   python3 "$D/gate_headers.py" "$ROOT"
run "honest failure          Safety, PLD 2024/2853"      python3 "$D/gate_stubs.py" "$ROOT"
run "link-time modularity    Modularity, Flexibility"    python3 "$D/gate_modularity.py" "$BUILD"
run "regression suites      ISO/IEC/IEEE 29119"         sh  "$ROOT/tests/run.sh" "$ROOT"
printf '\n=== compliance gates: %s ===\n' "$([ $rc -eq 0 ] && echo PASS || echo REGRESSED)"
exit $rc

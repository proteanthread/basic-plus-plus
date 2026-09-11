#!/bin/sh
# FILENAME: gate_doxygen.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: POSIX sh, grep
# PROJECT_RULES section 5 prohibits Doxygen tags. A prohibition with live
# violations is not a rule, it is a preference; this makes it a rule.
# Standard: ISO/IEC 25010:2023 Maintainability > Analysability
. "$(dirname "$0")/_baseline.sh"
set -eu
ROOT="${1:-.}"
n=$(grep -rlE '@(brief|param|return|returns)\b' "$ROOT/engine" --include='*.c' --include='*.h' 2>/dev/null | wc -l | tr -d ' ')
grep -rnE '@(brief|param|return|returns)\b' "$ROOT/engine" --include='*.c' --include='*.h' 2>/dev/null | head -20 || true
printf 'GATE doxygen: %s file(s) contain prohibited tags\n' "$n"
bl_check doxygen_files "$n" 'files with prohibited tags'

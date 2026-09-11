#!/bin/sh
# FILENAME: gate_standalone.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: POSIX sh, a C17 compiler
# THE PORTABILITY GATE. Every translation unit must compile ON ITS OWN,
# seeing only the public include tree. A file that needs a sibling .c to be
# compiled first is not a micro-library and cannot be lifted onto FreeDOS,
# an ESP32, or any other target one piece at a time.
# Also enforces C17 pedantically: -std=c17 -pedantic-errors.
# Standard: ISO/IEC 9899:2018 | ISO/IEC 25010:2023 Flexibility, Modularity
. "$(dirname "$0")/_baseline.sh"
set -eu
ROOT="${1:-.}"
CC="${CC:-gcc}"
LOG="${LOG:-/tmp/gate_standalone.log}"
: > "$LOG"
ok=0; bad=0
for f in $(find "$ROOT/engine/src" "$ROOT/engine/lib" -name '*.c' 2>/dev/null | sort); do
    if $CC -fsyntax-only -std=c17 -pedantic-errors \
        -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 \
        -I"$ROOT/engine/include" -I"$ROOT/engine/iot/include" \
        -I"$ROOT/engine/src/device/bgi/aalib/include" \
        "$f" >> "$LOG" 2>&1
    then ok=$((ok+1))
    else bad=$((bad+1)); printf '%s\n' "${f#"$ROOT"/}" ; fi
done
printf 'GATE standalone: %s compile alone, %s do NOT  (log: %s)\n' "$ok" "$bad" "$LOG"
bl_check standalone_failures "$bad" 'translation units that cannot compile alone'

#!/bin/sh
# FILENAME: gate_freestanding.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: POSIX sh, grep, sed
# Enforces the freestanding boundary: ISO/IEC 9899:2018 clause 4 paragraph 6
# permits exactly nine system headers. Everything else is hosted, and hosted
# headers are allowed only inside the sanctioned platform directories.
# Standard: ISO/IEC 9899:2018 s4p6 | ISO/IEC 25010:2023 Flexibility
. "$(dirname "$0")/_baseline.sh"
set -eu
ROOT="${1:-.}"
PERMITTED='float\.h|iso646\.h|limits\.h|stdalign\.h|stdarg\.h|stdbool\.h|stddef\.h|stdint\.h|stdnoreturn\.h'
SANCTIONED='engine/lib/platform/|engine/src/hal/|engine/src/hardware/|engine/iot/src/|engine/include/platform/'
fail=0
tmp=$(mktemp)
find "$ROOT/engine" -name '*.c' -o -name '*.h' 2>/dev/null | sort | while read -r f; do
    rel=${f#"$ROOT"/}
    case "$rel" in
        engine/lib/platform/*|engine/src/hal/*|engine/src/hardware/*|engine/iot/src/*|engine/include/platform/*) continue ;;
    esac
    grep -n '^[[:space:]]*#[[:space:]]*include[[:space:]]*<' "$f" 2>/dev/null |
      sed 's/^\([0-9]*\):.*<\([^>]*\)>.*/\1 \2/' |
      while read -r line hdr; do
        case "$hdr" in
            float.h|iso646.h|limits.h|stdalign.h|stdarg.h|stdbool.h|stddef.h|stdint.h|stdnoreturn.h) ;;
            *) printf '%s:%s: hosted header <%s> outside the platform layer\n' "$rel" "$line" "$hdr" ;;
        esac
      done
done > "$tmp"
n=$(wc -l < "$tmp" | tr -d ' ')
if [ "$n" -gt 0 ]; then cat "$tmp"; fi
rm -f "$tmp"
printf 'GATE freestanding: %s violation(s)\n' "$n"
bl_check freestanding_violations "$n" 'hosted headers outside the platform layer' || fail=1
exit $fail

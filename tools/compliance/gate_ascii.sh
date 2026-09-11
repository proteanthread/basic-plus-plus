#!/bin/sh
# FILENAME: gate_ascii.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: POSIX sh, grep
# Source must be pure 7-bit ASCII. C17 clause 5.2.1 leaves the mapping of
# characters outside the basic source character set implementation-defined;
# a compiler in a non-UTF-8 locale, or a 16-bit DOS toolchain, reads them as
# code-page bytes. MSVC emits C4819 for exactly this.
# Standard: ISO/IEC 9899:2018 s5.2.1 | ISO/IEC 25010:2023 Flexibility
. "$(dirname "$0")/_baseline.sh"
set -eu
ROOT="${1:-.}"
n=0
for f in $(find "$ROOT/engine" -name '*.c' -o -name '*.h' 2>/dev/null | sort); do
    if LC_ALL=C grep -qP '[^\x00-\x7F]' "$f" 2>/dev/null; then
        cnt=$(LC_ALL=C grep -cP '[^\x00-\x7F]' "$f")
        printf '%s: %s line(s) contain bytes above 0x7F\n' "${f#"$ROOT"/}" "$cnt"
        n=$((n+1))
    fi
done
printf 'GATE ascii: %s file(s) with non-ASCII bytes\n' "$n"
bl_check ascii_files "$n" 'files with non-ASCII bytes'

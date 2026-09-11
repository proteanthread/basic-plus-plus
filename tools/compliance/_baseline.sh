# FILENAME: _baseline.sh
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: every gate_*.sh in this directory
# NEEDS: POSIX sh, python3
# Shared ceiling check. A gate that can never pass gets switched off by the
# first person it annoys, so every gate compares against a recorded ceiling
# and fails only on a REGRESSION. Lower the ceilings in baseline.json as the
# work lands; that ratchet is the whole mechanism.
bl_check() {  # bl_check <key> <value> <label>
    _k=$1; _v=$2; _l=$3
    _b="${BL_DIR:-$(dirname "$0")}/baseline.json"
    [ -f "$_b" ] || { printf '  no baseline; %s = %s recorded as informational\n' "$_l" "$_v"; return 0; }
    _lim=$(python3 -c "import json,sys;print(json.load(open('$_b')).get('$_k','none'))" 2>/dev/null)
    [ "$_lim" = "none" ] && { printf '  no ceiling for %s\n' "$_k"; return 0; }
    if [ "$_v" -gt "$_lim" ]; then
        printf '  FAIL: %s rose to %s, ceiling is %s\n' "$_l" "$_v" "$_lim"; return 1
    fi
    if [ "$_v" -lt "$_lim" ]; then
        printf '  IMPROVED: %s is %s, ceiling was %s -- lower the ceiling in baseline.json\n' "$_l" "$_v" "$_lim"
    fi
    return 0
}

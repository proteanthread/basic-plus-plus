#!/bin/bash
# ========================================================
# BASIC++ 4.0.0 (Stable) — Linux Build (gcc)
# ========================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/source"

echo "========================================================"
echo "BASIC++ 4.0.0 (Stable) — Linux Build (gcc)"
echo "========================================================"

echo "[INFO] Cleaning previous build..."
make clean 2>/dev/null || true

echo "[INFO] Building baspp..."
make

echo "[OK] baspp built."

echo "[INFO] Cleaning intermediate .o files..."
find . -name '*.o' -delete 2>/dev/null || true
echo ""

# Build legacy prototypes (disabled — build separately if needed)
cd "$SCRIPT_DIR"

# echo "[INFO] Building legacy prototypes..."
#
# gcc -std=c90 -pedantic -Wall -Wextra -O2 -o int-bas int-bas.c
# echo "[OK] int-bas built."
#
# gcc -std=c90 -pedantic -Wall -Wextra -O2 -o core-bas core-bas.c
# echo "[OK] core-bas built."
#
# gcc -std=c90 -pedantic -Wall -Wextra -O2 -o tinybasic tinybasic.c
# echo "[OK] tinybasic built."

echo ""
echo "========================================================"
echo "All builds complete."
echo "  baspp       — BASIC++ interpreter"
# echo "  int-bas     — Integer BASIC v5 prototype"
# echo "  core-bas    — Tiny BASIC v1 prototype"
# echo "  tinybasic   — Palo Alto Tiny BASIC port"
echo "========================================================"

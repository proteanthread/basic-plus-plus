#!/bin/bash
set -e

echo "========================================================"
echo "BASIC++ v6.x.x — Linux Build (CMake)"
echo "========================================================"

mkdir -p build_linux
cd build_linux
cmake ..
make -j$(nproc 2>/dev/null || echo 4)

echo "[OK] Build completed."

// FILENAME: bppc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (bppc_internal.h)
// Implements toolchain and compiler subsystem components for bppc.
//
// ---- Includes ----

#include "tools/bppc_internal.h"

// All compiler logic is decomposed into:
// - tools/bppc/bppc_transpile.c: Translation of BASIC AST/tokens into freestanding C17
// - tools/bppc/bppc_detok.c: GW-BASIC binary decoding to ASCII
// - tools/bppc/bppc_bytecode.c: Bytecode compilation and standalone executable stub packaging
// - tools/bppc/bppc_main.c: CLI parsing and compilation orchestration

int main(int argc, char **argv) {
    return bppc_main_entry(argc, argv);
}

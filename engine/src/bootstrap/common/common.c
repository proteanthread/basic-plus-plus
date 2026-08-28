// FILENAME: common.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: 
// NEEDS: libboot (common_internal.h)
// Provides core logic and interface definitions for common within BASIC++.
//
// ---- Includes ----

#include "bootstrap/common_internal.h"

// All bootstrap logic is decomposed into:
// - bootstrap/common/common_reg_stmts.c: Statement and command handler registration
// - bootstrap/common/common_reg_funcs.c: Builtin and intrinsic function registration
// - bootstrap/common/common_boot.c: Memory init, virtual devices, and boot lifecycle

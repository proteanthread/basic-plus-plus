// FILENAME: exec_control.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (exec_control_internal.h)
// Implements bytecode virtual machine execution and state for exec_control.
//
// ---- Includes ----

#include "vm/exec_control_internal.h"

// All execution control implementation is decomposed into:
// - vm/exec/control/exec_postfix.c: Postfix keyword exemption and loop/conditional evaluation
// - vm/exec/control/exec_stmt.c: Statement lookup, alias expansion, method calls, and execution
// - vm/exec/control/exec_line.c: Full line execution loops and namespace resolution

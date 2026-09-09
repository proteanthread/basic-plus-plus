// FILENAME: reg_udx.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, BASIC++ runtime
// NEEDS: types/types.h, stdbool.h
// Provides definitions for the Universal Data Exchange (UDX / XCHG.*) register subsystem.

#ifndef ENGINE_REG_UDX_H
#define ENGINE_REG_UDX_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// UDX Register Bank Context
typedef struct {
    BValue  poly_r[16];    // XCHG.R0 .. XCHG.RF (polymorphic BValue: numbers, strings)
    double  num_r[16];     // XCHG.0 .. XCHG.F (64-bit IEEE float / integer)
    BValue  acc_a;         // XCHG.ACC.A (polymorphic accumulator)
    double  acc_b;         // XCHG.ACC.B (numeric accumulator)
} RegUdxContext;

// Reset UDX registers and accumulators to clean initial state
void reg_udx_reset(void);

// Retrieve pointer to active UDX register context
RegUdxContext* reg_udx_get_context(void);

// Read UDX register by identifier (e.g. "XCHG.R0", "XCHG.0", "XCHG.ACC.A", "UDX.R0")
bool reg_udx_get(const char *name, BValue *out_val);

// Write UDX register by identifier
bool reg_udx_set(const char *name, BValue val);

// Indexed register access for XCHG[index] (index 0..15)
bool reg_udx_get_by_index(int index, bool numeric_bank, BValue *out_val);
bool reg_udx_set_by_index(int index, bool numeric_bank, BValue val);

// Accumulator getters and setters for banks 'A', 'B', 'C'
BValue reg_udx_get_acc(char bank);
void reg_udx_set_acc(char bank, BValue val);

// Unified builtin identifier lookup for REG.*, UDX.*, XCHG.*, FIFO.*, LIFO.*, etc.
bool reg_lookup_builtin(const char *name, BValue *out_val);

// Domain checkers for architectural boundary enforcement
bool reg_is_hardware_domain(const char *name);
bool reg_is_math_domain(const char *name);
bool reg_is_udx_domain(const char *name);

// Validates domain boundaries and executes register assignment
bool reg_assign_builtin(const char *target_name, BValue val, const char *expr_hint, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_REG_UDX_H

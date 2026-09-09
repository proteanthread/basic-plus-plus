// FILENAME: rpn_mirror.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: types/types.h, reg/reg_hw.h
// Real-time evaluator mirroring and RPN stack telemetry observer definitions.

#ifndef EVAL_RPN_MIRROR_H
#define EVAL_RPN_MIRROR_H

#include "types/types.h"
#include "reg/reg_hw.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Evaluator mirroring configuration (!!EVAL_MIRROR ON/OFF)
void rpn_set_eval_mirror(bool enable);
bool rpn_get_eval_mirror(void);

// Mirror root expression evaluation result to STACK.X / STACK.LASTX
void rpn_mirror_eval_result(BValue val);

// Register real-time observer for RPN/STACK modifications
void rpn_register_observer(RegObserverFn fn, void *user_data);

// Notify RPN/STACK observer of register modification
void rpn_notify_observer(const char *reg_name, double old_val, double new_val);

#ifdef __cplusplus
}
#endif

#endif // EVAL_RPN_MIRROR_H

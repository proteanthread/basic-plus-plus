// FILENAME: rpn_mirror.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: eval/rpn_mirror.h, eval/rpn.h, runtime/string/strops.h
// Implements real-time evaluator mirroring and RPN stack telemetry observer callbacks.

#include "eval/rpn_mirror.h"
#include "eval/rpn.h"
#include "runtime/string/strops.h"

static bool s_eval_mirror_enabled = false;
static RegObserverFn s_rpn_observer_fn = NULL;
static void *s_rpn_observer_user_data = NULL;

void rpn_set_eval_mirror(bool enable) {
    s_eval_mirror_enabled = enable;
}

bool rpn_get_eval_mirror(void) {
    return s_eval_mirror_enabled;
}

void rpn_register_observer(RegObserverFn fn, void *user_data) {
    s_rpn_observer_fn = fn;
    s_rpn_observer_user_data = user_data;
}

void rpn_notify_observer(const char *reg_name, double old_val, double new_val) {
    if (s_rpn_observer_fn) {
        s_rpn_observer_fn("STACK", reg_name, old_val, new_val, s_rpn_observer_user_data);
    }
}

void rpn_mirror_eval_result(BValue val) {
    if (!s_eval_mirror_enabled) return;

    RpnContext *ctx = rpn_get_context();
    if (!ctx) return;

    double old_x = 0.0;
    if (ctx->x.type == VAL_INTEGER || ctx->x.type == VAL_NUMBER) {
        old_x = ctx->x.as.number;
    }

    double new_val = 0.0;
    if (val.type == VAL_INTEGER || val.type == VAL_NUMBER) {
        new_val = val.as.number;
    }

    // Roll previous X into LASTX
    ctx->last_x = ctx->x;

    // Set new X
    ctx->x = val;

    rpn_notify_observer("X", old_x, new_val);
}

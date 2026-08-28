// FILENAME: type.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (type.h)
// Provides core logic and interface definitions for type within BASIC++.
//
// ---- Includes ----

#include "eval/type.h"

bool eval_type_is_numeric(const BValue *val) {
    return val && val->type == VAL_NUMBER;
}

bool eval_type_is_string(const BValue *val) {
    return val && val->type == VAL_STRING;
}

bool eval_type_is_truthy(const BValue *val) {
    if (!val) return false;
    if (val->type == VAL_NUMBER) {
        return val->as.number != 0.0;
    }
    return false;
}

double eval_bool_to_basic(bool condition) {
    return condition ? -1.0 : 0.0;
}

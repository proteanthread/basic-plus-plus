// FILENAME: type.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (type.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for type within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_EVAL_TYPE_H
#define EVAL_EVAL_TYPE_H

#include "types/types.h"
#include <stdbool.h>

bool eval_type_is_numeric(const BValue *val);
bool eval_type_is_string(const BValue *val);
bool eval_type_is_truthy(const BValue *val);
double eval_bool_to_basic(bool condition);

#endif // EVAL_EVAL_TYPE_H

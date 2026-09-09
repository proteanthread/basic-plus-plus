// FILENAME: pick_mv_legacy.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// Shelved legacy Pick MultiValue functions (DINDEX, DINSERT, DREPLACE, DDELETE, DCOUNT).
// Reserved for future specialized Pick compatibility modules.

#ifndef PLANNED_PICK_MV_LEGACY_H
#define PLANNED_PICK_MV_LEGACY_H

#include "types/bvalue.h"
#include "types/errors.h"
#include "vm/vm.h"

BValue planned_dcount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue planned_dindex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue planned_dinsert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue planned_dreplace_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue planned_ddelete_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // PLANNED_PICK_MV_LEGACY_H

// FILENAME: set_unify.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (arrays.h, map.h), libkernel (set.h, vm.h)
// Implements container unification between Sets, Arrays, and Maps in BASIC++.

#include "runtime/set.h"
#include "runtime/arrays.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "vm/vm.h"

// Unified array element getter: adapts Arr{i} to arr_get_element
bool set_unify_array_get(void *vm, const char *name, int index, BValue *out_val, BppError *err) {
    if (!vm || !name || !out_val || !err) return false;
    VMContext *vm_ctx = (VMContext *)vm;
    ArrayContext *arr_ctx = vm_get_arr(vm_ctx);
    if (!arr_ctx || !arr_exists(arr_ctx, name)) return false;

    int idx = index;
    BValue *elem = arr_get_element(arr_ctx, name, 1, &idx, err);
    if (!elem || err->code != 0) return false;

    *out_val = *elem;
    if (out_val->type == VAL_STRING && out_val->as.string) {
        str_add_ref(out_val->as.string);
    } else if (out_val->type == VAL_MAP && out_val->as.map) {
        map_add_ref(out_val->as.map);
    } else if (out_val->type == VAL_SET && out_val->as.set) {
        set_add_ref(out_val->as.set);
    } else if (out_val->type == VAL_GROUP && out_val->as.group) {
        group_add_ref(out_val->as.group);
    }
    return true;
}

// Unified array element setter: adapts Arr{i} = val to arr_get_element
bool set_unify_array_set(void *vm, const char *name, int index, BValue val, BppError *err) {
    if (!vm || !name || !err) return false;
    VMContext *vm_ctx = (VMContext *)vm;
    ArrayContext *arr_ctx = vm_get_arr(vm_ctx);
    if (!arr_ctx || !arr_exists(arr_ctx, name)) return false;

    int idx = index;
    BValue *elem = arr_get_element(arr_ctx, name, 1, &idx, err);
    if (!elem || err->code != 0) return false;

    // Release previous value
    StringContext *sctx = vm_get_str(vm_ctx);
    if (elem->type == VAL_STRING && elem->as.string) {
        str_release(sctx, elem->as.string);
    } else if (elem->type == VAL_MAP && elem->as.map) {
        map_release(sctx, elem->as.map);
    } else if (elem->type == VAL_SET && elem->as.set) {
        set_release(sctx, elem->as.set);
    } else if (elem->type == VAL_GROUP && elem->as.group) {
        group_release(sctx, elem->as.group);
    }

    *elem = val;
    if (elem->type == VAL_STRING && elem->as.string) {
        str_add_ref(elem->as.string);
    } else if (elem->type == VAL_MAP && elem->as.map) {
        map_add_ref(elem->as.map);
    } else if (elem->type == VAL_SET && elem->as.set) {
        set_add_ref(elem->as.set);
    } else if (elem->type == VAL_GROUP && elem->as.group) {
        group_add_ref(elem->as.group);
    }
    return true;
}

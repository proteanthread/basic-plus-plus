/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file array_sort.c
 * @brief Array sorting utilities.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Sorts one-dimensional numeric and string arrays in place.
 * - Why it exists: Provides Phase 2 SORT and SORT$ functionality.
 * - Why it works this way: Retrieves the flat elements from the array subsystem and uses qsort.
 */

#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include <stdlib.h>
#include <string.h>

static int cmp_numeric_asc(const void *a, const void *b) {
    const BValue *va = (const BValue *)a;
    const BValue *vb = (const BValue *)b;
    double da = va->type == VAL_NUMBER ? va->as.number : 0.0;
    double db = vb->type == VAL_NUMBER ? vb->as.number : 0.0;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

static int cmp_numeric_desc(const void *a, const void *b) {
    return cmp_numeric_asc(b, a);
}

static int cmp_string_asc(const void *a, const void *b) {
    const BValue *va = (const BValue *)a;
    const BValue *vb = (const BValue *)b;
    const char *sa = (va->type == VAL_STRING && va->as.string) ? str_data(va->as.string) : "";
    const char *sb = (vb->type == VAL_STRING && vb->as.string) ? str_data(vb->as.string) : "";
    return strcmp(sa, sb);
}

static int cmp_string_desc(const void *a, const void *b) {
    return cmp_string_asc(b, a);
}

bool arr_sort_inplace(ArrayContext *ctx, const char *name, bool is_string, bool ascending) {
    int total_size = 0;
    BValue *elements = arr_get_flat_elements(ctx, name, &total_size);
    if (!elements || total_size <= 1) {
        return false;
    }
    
    if (is_string) {
        qsort(elements, total_size, sizeof(BValue), ascending ? cmp_string_asc : cmp_string_desc);
    } else {
        qsort(elements, total_size, sizeof(BValue), ascending ? cmp_numeric_asc : cmp_numeric_desc);
    }
    
    return true;
}

/* SORT(array_name$) -> Number (returns 1 on success, 0 on failure) */
BValue array_sort_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *name = str_data(args[0].as.string);
    bool success = arr_sort_inplace(vm_get_arr(vm), name, false, true);
    return bval_float(success ? 1.0 : 0.0);
}

/* SORT$(array_name$) -> Number */
BValue array_sort_str_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *name = str_data(args[0].as.string);
    bool success = arr_sort_inplace(vm_get_arr(vm), name, true, true);
    return bval_float(success ? 1.0 : 0.0);
}


void register_array_sort_functions(void) {
    FunctionEntry fe;
    memset(&fe, 0, sizeof(fe));
    fe.module_name = "array_sort";
    fe.overridable = 1;
    fe.category = FCAT_UTIL;
    
    fe.name = "SORT";
    fe.ret_type = FRET_INT;
    fe.min_args = 1;
    fe.max_args = 1;
    fe.safety = FSAFE_STATE;
    fe.handler = array_sort_func;
    funcreg_register(&fe);
    
    fe.name = "SORT$";
    fe.ret_type = FRET_INT;
    fe.min_args = 1;
    fe.max_args = 1;
    fe.safety = FSAFE_STATE;
    fe.handler = array_sort_str_func;
    funcreg_register(&fe);
}

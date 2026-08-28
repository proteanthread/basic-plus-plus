// FILENAME: keyword_props.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, keyword.c)
// NEEDS: libcore (keyword_props.h, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides core logic and interface definitions for keyword_props within BASIC++.
//
// ---- Includes ----

#include "runtime/keyword_props.h"
#include "types/types.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static BppKeywordPropTable g_kw_prop_table;

static BppKeywordPropTable *get_prop_table(VMContext *vm) {
    (void)vm;
    return &g_kw_prop_table;
}

void kw_props_init(VMContext *vm) {
    BppKeywordPropTable *t = get_prop_table(vm);
    if (t) {
        runtime_memset(t, 0, sizeof(BppKeywordPropTable));
    }
}

void kw_props_clear(VMContext *vm) {
    kw_props_init(vm);
}

bool kw_props_set_property(VMContext *vm, const char *kw, const char *prop_name, const char *prop_val) {
    if (!kw || !prop_name || !prop_val) return false;
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return false;

    for (int i = 0; i < t->count; ++i) {
        if (runtime_strcasecmp(t->props[i].keyword, kw) == 0 && runtime_strcasecmp(t->props[i].property_name, prop_name) == 0) {
            runtime_strncpy(t->props[i].property_value, prop_val, sizeof(t->props[i].property_value) - 1);
            t->props[i].property_value[sizeof(t->props[i].property_value) - 1] = '\0';
            return true;
        }
    }

    if (t->count >= MAX_KEYWORD_PROPERTIES) return false;

    runtime_strncpy(t->props[t->count].keyword, kw, sizeof(t->props[t->count].keyword) - 1);
    t->props[t->count].keyword[sizeof(t->props[t->count].keyword) - 1] = '\0';

    runtime_strncpy(t->props[t->count].property_name, prop_name, sizeof(t->props[t->count].property_name) - 1);
    t->props[t->count].property_name[sizeof(t->props[t->count].property_name) - 1] = '\0';

    runtime_strncpy(t->props[t->count].property_value, prop_val, sizeof(t->props[t->count].property_value) - 1);
    t->props[t->count].property_value[sizeof(t->props[t->count].property_value) - 1] = '\0';

    t->count++;
    return true;
}

const char *kw_props_get_property(VMContext *vm, const char *kw, const char *prop_name) {
    if (!kw || !prop_name) return NULL;
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return NULL;

    for (int i = 0; i < t->count; ++i) {
        if (runtime_strcasecmp(t->props[i].keyword, kw) == 0 && runtime_strcasecmp(t->props[i].property_name, prop_name) == 0) {
            return t->props[i].property_value;
        }
    }
    return NULL;
}

void kw_props_list(VMContext *vm, const char *kw) {
    BppKeywordPropTable *t = get_prop_table(vm);
    if (!t) return;

    vdev_printf(vm_get_vdev(vm), "Properties for keyword '%s':\n", kw ? kw : "*");
    int count = 0;
    for (int i = 0; i < t->count; ++i) {
        if (!kw || runtime_strcasecmp(t->props[i].keyword, kw) == 0) {
            vdev_printf(vm_get_vdev(vm), "  [%s] %s = %s\n", t->props[i].keyword, t->props[i].property_name, t->props[i].property_value);
            count++;
        }
    }
    vdev_printf(vm_get_vdev(vm), "Total Properties Found: %d\n", count);
}

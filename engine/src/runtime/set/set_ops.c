// FILENAME: set_ops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (strops.h, memops.h, snprintf.h), libkernel (set.h)
// Implements algebraic set operations, membership, and string serialization.

#include "runtime/set.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/num_format.h"
#include "runtime/math/math.h"

// Compares two BValues for algebraic equality
bool value_equals(BValue a, BValue b) {
    if ((a.type == VAL_NUMBER || a.type == VAL_INTEGER) &&
        (b.type == VAL_NUMBER || b.type == VAL_INTEGER)) {
        return runtime_fabs(a.as.number - b.as.number) < 1e-9;
    }
    if (a.type == VAL_STRING && b.type == VAL_STRING) {
        if (!a.as.string && !b.as.string) return true;
        if (!a.as.string || !b.as.string) return false;
        return runtime_strcmp(str_data(a.as.string), str_data(b.as.string)) == 0;
    }
    if (a.type == VAL_COMPLEX && b.type == VAL_COMPLEX) {
        return runtime_fabs(a.as.complex_val.real - b.as.complex_val.real) < 1e-9 &&
               runtime_fabs(a.as.complex_val.imag - b.as.complex_val.imag) < 1e-9;
    }
    if (a.type == VAL_GROUP && b.type == VAL_GROUP) {
        if (!a.as.group && !b.as.group) return true;
        if (!a.as.group || !b.as.group) return false;
        if (a.as.group->count != b.as.group->count) return false;
        for (int i = 0; i < a.as.group->count; ++i) {
            BValue bv;
            const char *k = a.as.group->entries[i].name;
            if (!k || !group_get(b.as.group, k, &bv)) return false;
            if (!value_equals(a.as.group->entries[i].val, bv)) return false;
        }
        return true;
    }
    if (a.type == VAL_MAP && b.type == VAL_MAP) {
        if (!a.as.map && !b.as.map) return true;
        if (!a.as.map || !b.as.map) return false;
        if (a.as.map->count != b.as.map->count) return false;
        for (int i = 0; i < a.as.map->count; ++i) {
            BValue bv;
            const char *k = a.as.map->entries[i].key;
            if (!k || !map_get(b.as.map, k, &bv)) return false;
            if (!value_equals(a.as.map->entries[i].val, bv)) return false;
        }
        return true;
    }
    if (a.type == VAL_SET && b.type == VAL_SET) {
        if (!a.as.set && !b.as.set) return true;
        if (!a.as.set || !b.as.set) return false;
        if (a.as.set->count != b.as.set->count) return false;
        for (int i = 0; i < a.as.set->count; ++i) {
            if (!set_contains(b.as.set, a.as.set->items[i])) return false;
        }
        return true;
    }
    return false;
}

bool set_contains(BppSet *s, BValue val) {
    if (!s) return false;
    for (int i = 0; i < s->count; ++i) {
        if (value_equals(s->items[i], val)) return true;
    }
    return false;
}

BppSet *set_union(void *str_ctx, BppSet *a, BppSet *b) {
    int cap = (a ? a->count : 0) + (b ? b->count : 0);
    BppSet *res = set_create(cap, false);
    if (!res) return NULL;

    if (a) {
        for (int i = 0; i < a->count; ++i) {
            if (!set_contains(res, a->items[i])) {
                set_add(str_ctx, res, a->items[i]);
            }
        }
    }
    if (b) {
        for (int i = 0; i < b->count; ++i) {
            if (!set_contains(res, b->items[i])) {
                set_add(str_ctx, res, b->items[i]);
            }
        }
    }
    return res;
}

BppSet *set_intersection(void *str_ctx, BppSet *a, BppSet *b) {
    BppSet *res = set_create(8, false);
    if (!res || !a || !b) return res;

    for (int i = 0; i < a->count; ++i) {
        if (set_contains(b, a->items[i]) && !set_contains(res, a->items[i])) {
            set_add(str_ctx, res, a->items[i]);
        }
    }
    return res;
}

BppSet *set_difference(void *str_ctx, BppSet *a, BppSet *b) {
    BppSet *res = set_create(8, false);
    if (!res || !a) return res;

    for (int i = 0; i < a->count; ++i) {
        if ((!b || !set_contains(b, a->items[i])) && !set_contains(res, a->items[i])) {
            set_add(str_ctx, res, a->items[i]);
        }
    }
    return res;
}

BppSet *set_sym_diff(void *str_ctx, BppSet *a, BppSet *b) {
    BppSet *diff_ab = set_difference(str_ctx, a, b);
    BppSet *diff_ba = set_difference(str_ctx, b, a);
    BppSet *res = set_union(str_ctx, diff_ab, diff_ba);
    if (diff_ab) set_release(str_ctx, diff_ab);
    if (diff_ba) set_release(str_ctx, diff_ba);
    return res;
}

bool set_is_subset(BppSet *a, BppSet *b) {
    if (!a) return true;
    if (!b) return a->count == 0;
    for (int i = 0; i < a->count; ++i) {
        if (!set_contains(b, a->items[i])) return false;
    }
    return true;
}

bool set_is_proper_subset(BppSet *a, BppSet *b) {
    if (!a || !b) return false;
    return set_is_subset(a, b) && (a->count < b->count);
}

int set_cardinality(BppSet *s) {
    return s ? s->count : 0;
}

// ---- Group Algebraic Operations ----

BppGroup *group_union(void *str_ctx, BppGroup *a, BppGroup *b) {
    BppGroup *res = group_create(NULL);
    if (!res) return NULL;
    if (a) {
        for (int i = 0; i < a->count; ++i) {
            if (a->entries[i].name) {
                group_set(str_ctx, res, a->entries[i].name, a->entries[i].val);
            }
        }
    }
    if (b) {
        for (int i = 0; i < b->count; ++i) {
            if (b->entries[i].name) {
                group_set(str_ctx, res, b->entries[i].name, b->entries[i].val);
            }
        }
    }
    return res;
}

BppGroup *group_intersection(void *str_ctx, BppGroup *a, BppGroup *b) {
    BppGroup *res = group_create(NULL);
    if (!res || !a || !b) return res;
    for (int i = 0; i < a->count; ++i) {
        if (a->entries[i].name) {
            BValue b_val;
            if (group_get(b, a->entries[i].name, &b_val)) {
                group_set(str_ctx, res, a->entries[i].name, b_val);
            }
        }
    }
    return res;
}

BppGroup *group_difference(void *str_ctx, BppGroup *a, BppGroup *b) {
    BppGroup *res = group_create(NULL);
    if (!res || !a) return res;
    for (int i = 0; i < a->count; ++i) {
        if (a->entries[i].name && (!b || !group_has(b, a->entries[i].name))) {
            group_set(str_ctx, res, a->entries[i].name, a->entries[i].val);
        }
    }
    return res;
}

BppGroup *group_sym_diff(void *str_ctx, BppGroup *a, BppGroup *b) {
    BppGroup *diff_ab = group_difference(str_ctx, a, b);
    BppGroup *diff_ba = group_difference(str_ctx, b, a);
    BppGroup *res = group_union(str_ctx, diff_ab, diff_ba);
    if (diff_ab) group_release(str_ctx, diff_ab);
    if (diff_ba) group_release(str_ctx, diff_ba);
    return res;
}

bool group_is_subset(BppGroup *a, BppGroup *b) {
    if (!a) return true;
    if (!b) return a->count == 0;
    for (int i = 0; i < a->count; ++i) {
        if (!a->entries[i].name) continue;
        BValue b_val;
        if (!group_get(b, a->entries[i].name, &b_val)) return false;
        if (!value_equals(a->entries[i].val, b_val)) return false;
    }
    return true;
}

bool group_is_proper_subset(BppGroup *a, BppGroup *b) {
    if (!a || !b) return false;
    return group_is_subset(a, b) && (a->count < b->count);
}

// ---- Map Subsumption Interop ----

BppGroup *group_from_map(void *str_ctx, struct BppMap *map) {
    BppGroup *g = group_create(NULL);
    if (!g || !map) return g;
    for (int i = 0; i < map->count; ++i) {
        if (map->entries[i].key) {
            group_set(str_ctx, g, map->entries[i].key, map->entries[i].val);
        }
    }
    return g;
}

struct BppMap *map_from_group(void *str_ctx, BppGroup *group) {
    struct BppMap *m = map_create();
    if (!m || !group) return m;
    for (int i = 0; i < group->count; ++i) {
        if (group->entries[i].name) {
            map_set(str_ctx, m, group->entries[i].name, group->entries[i].val);
        }
    }
    return m;
}

// ---- Multi-Tier Path Addressing & Relational Projection ----

bool group_path_get(void *str_ctx, BppGroup *group, int depth, BValue *keys, BValue *out_val) {
    (void)str_ctx;
    if (!group || depth <= 0 || !keys || !out_val) return false;
    BValue curr = { .type = VAL_GROUP, .as.group = group };

    for (int i = 0; i < depth; ++i) {
        if (curr.type == VAL_GROUP && curr.as.group) {
            const char *k = NULL;
            if (keys[i].type == VAL_STRING && keys[i].as.string) {
                k = str_data(keys[i].as.string);
            } else if (keys[i].type == VAL_NUMBER || keys[i].type == VAL_INTEGER) {
                int idx = (int)keys[i].as.number;
                BValue elem;
                if (!group_get_at(curr.as.group, idx, NULL, &elem)) return false;
                curr = elem;
                continue;
            } else {
                return false;
            }
            BValue next;
            if (!group_get(curr.as.group, k, &next)) return false;
            curr = next;
        } else if (curr.type == VAL_MAP && curr.as.map) {
            const char *k = (keys[i].type == VAL_STRING && keys[i].as.string) ? str_data(keys[i].as.string) : NULL;
            if (!k) return false;
            BValue next;
            if (!map_get(curr.as.map, k, &next)) return false;
            curr = next;
        } else if (curr.type == VAL_SET && curr.as.set) {
            int idx = (int)keys[i].as.number;
            BValue next;
            if (!set_get(curr.as.set, idx, &next)) return false;
            curr = next;
        } else {
            return false;
        }
    }
    *out_val = curr;
    return true;
}

bool group_path_set(void *str_ctx, BppGroup *group, int depth, BValue *keys, BValue val) {
    if (!group || depth <= 0 || !keys) return false;
    if (depth == 1) {
        const char *k = (keys[0].type == VAL_STRING && keys[0].as.string) ? str_data(keys[0].as.string) : NULL;
        if (!k) return false;
        return group_set(str_ctx, group, k, val);
    }
    BValue curr = { .type = VAL_GROUP, .as.group = group };
    for (int i = 0; i < depth - 1; ++i) {
        const char *k = (keys[i].type == VAL_STRING && keys[i].as.string) ? str_data(keys[i].as.string) : NULL;
        if (!k) return false;
        BValue next;
        if (curr.type == VAL_GROUP && curr.as.group) {
            if (!group_get(curr.as.group, k, &next) || (next.type != VAL_GROUP && next.type != VAL_MAP)) {
                BppGroup *sub = group_create(k);
                if (!sub) return false;
                BValue sub_val = { .type = VAL_GROUP, .as.group = sub };
                group_set(str_ctx, curr.as.group, k, sub_val);
                group_release(str_ctx, sub);
                next = sub_val;
            }
            curr = next;
        } else {
            return false;
        }
    }
    const char *last_k = (keys[depth - 1].type == VAL_STRING && keys[depth - 1].as.string) ? str_data(keys[depth - 1].as.string) : NULL;
    if (!last_k) return false;
    if (curr.type == VAL_GROUP && curr.as.group) {
        return group_set(str_ctx, curr.as.group, last_k, val);
    } else if (curr.type == VAL_MAP && curr.as.map) {
        return map_set(str_ctx, curr.as.map, last_k, val);
    }
    return false;
}

bool set_path_get(void *str_ctx, BValue root, int depth, BValue *keys, BValue *out_val) {
    if (depth <= 0 || !keys || !out_val) return false;
    if (root.type == VAL_GROUP && root.as.group) {
        return group_path_get(str_ctx, root.as.group, depth, keys, out_val);
    }
    if (root.type == VAL_MAP && root.as.map) {
        BppGroup *g = group_from_map(str_ctx, root.as.map);
        bool ok = group_path_get(str_ctx, g, depth, keys, out_val);
        group_release(str_ctx, g);
        return ok;
    }
    if (root.type == VAL_SET && root.as.set) {
        int idx = (int)keys[0].as.number;
        BValue elem;
        if (!set_get(root.as.set, idx, &elem)) return false;
        if (depth == 1) {
            *out_val = elem;
            return true;
        }
        return set_path_get(str_ctx, elem, depth - 1, keys + 1, out_val);
    }
    return false;
}

bool set_path_set(void *str_ctx, BValue *root, int depth, BValue *keys, BValue val) {
    if (!root || depth <= 0 || !keys) return false;
    if (root->type == VAL_GROUP && root->as.group) {
        return group_path_set(str_ctx, root->as.group, depth, keys, val);
    }
    if (root->type == VAL_MAP && root->as.map) {
        const char *k = (keys[0].type == VAL_STRING && keys[0].as.string) ? str_data(keys[0].as.string) : NULL;
        if (depth == 1 && k) {
            return map_set(str_ctx, root->as.map, k, val);
        }
    }
    if (root->type == VAL_SET && root->as.set) {
        int idx = (int)keys[0].as.number;
        if (depth == 1) {
            return set_set(str_ctx, root->as.set, idx, val);
        }
        BValue elem;
        if (!set_get(root->as.set, idx, &elem)) return false;
        return set_path_set(str_ctx, &elem, depth - 1, keys + 1, val);
    }
    return false;
}

BppSet *set_project(void *str_ctx, BppSet *set, const char *field) {
    BppSet *res = set_create(8, false);
    if (!res || !set || !field) return res;
    for (int i = 0; i < set->count; ++i) {
        BValue item = set->items[i];
        if (item.type == VAL_GROUP && item.as.group) {
            BValue fval;
            if (group_get(item.as.group, field, &fval)) {
                set_add(str_ctx, res, fval);
            }
        } else if (item.type == VAL_MAP && item.as.map) {
            BValue fval;
            if (map_get(item.as.map, field, &fval)) {
                set_add(str_ctx, res, fval);
            }
        }
    }
    return res;
}

BppSet *set_project_filter(void *str_ctx, BppSet *set, const char *filter_key, BValue filter_val, const char *field) {
    BppSet *res = set_create(8, false);
    if (!res || !set || !filter_key || !field) return res;
    for (int i = 0; i < set->count; ++i) {
        BValue item = set->items[i];
        if (item.type == VAL_GROUP && item.as.group) {
            BValue matched;
            if (group_get(item.as.group, filter_key, &matched) && value_equals(matched, filter_val)) {
                BValue fval;
                if (group_get(item.as.group, field, &fval)) {
                    set_add(str_ctx, res, fval);
                }
            }
        } else if (item.type == VAL_MAP && item.as.map) {
            BValue matched;
            if (map_get(item.as.map, filter_key, &matched) && value_equals(matched, filter_val)) {
                BValue fval;
                if (map_get(item.as.map, field, &fval)) {
                    set_add(str_ctx, res, fval);
                }
            }
        }
    }
    return res;
}

static void format_single_val(char *buf, size_t buf_size, BValue v) {
    if (v.type == VAL_NUMBER || v.type == VAL_INTEGER) {
        num_format_serialize(buf, buf_size, v.as.number);
    } else if (v.type == VAL_STRING && v.as.string) {
        runtime_snprintf(buf, buf_size, "\"%s\"", str_data(v.as.string));
    } else if (v.type == VAL_SET && v.as.set) {
        set_format(buf, buf_size, v.as.set);
    } else if (v.type == VAL_GROUP && v.as.group) {
        group_format(buf, buf_size, v.as.group);
    } else if (v.type == VAL_MAP && v.as.map) {
        runtime_snprintf(buf, buf_size, "<map>");
    } else {
        runtime_snprintf(buf, buf_size, "<value>");
    }
}

bool set_format(char *buf, size_t buf_size, BppSet *set) {
    if (!buf || buf_size < 3) return false;
    if (!set || set->count == 0) {
        runtime_snprintf(buf, buf_size, "{}");
        return true;
    }
    size_t pos = 0;
    pos += runtime_snprintf(buf + pos, buf_size - pos, "{ ");
    for (int i = 0; i < set->count; ++i) {
        char elem[128];
        format_single_val(elem, sizeof(elem), set->items[i]);
        pos += runtime_snprintf(buf + pos, (pos < buf_size) ? (buf_size - pos) : 0, "%s%s",
                                elem, (i < set->count - 1) ? ", " : " }");
        if (pos >= buf_size - 1) break;
    }
    return true;
}

bool group_format(char *buf, size_t buf_size, BppGroup *group) {
    if (!buf || buf_size < 3) return false;
    if (!group || group->count == 0) {
        runtime_snprintf(buf, buf_size, "{}");
        return true;
    }
    size_t pos = 0;
    pos += runtime_snprintf(buf + pos, buf_size - pos, "{ ");
    for (int i = 0; i < group->count; ++i) {
        char elem[128];
        format_single_val(elem, sizeof(elem), group->entries[i].val);
        const char *k = group->entries[i].name ? group->entries[i].name : "_";
        pos += runtime_snprintf(buf + pos, (pos < buf_size) ? (buf_size - pos) : 0, "%s: %s%s",
                                k, elem, (i < group->count - 1) ? ", " : " }");
        if (pos >= buf_size - 1) break;
    }
    return true;
}

// FILENAME: set_pick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (strops.h, memops.h, hal.h), libkernel (set.h)
// Implements Pick MultiValue dynamic array three-tier Group mapping and conversions.

#include "runtime/set.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/num_format.h"
#include "hal/hal.h"
#include "types/errors.h"

// Standard Pick delimiter detection: Attribute (@AM), Value (@VM), Subvalue (@SVM)
static char get_am_delim(const char *str) {
    if (runtime_strchr(str, '\xfe')) return '\xfe';
    return '^';
}

static char get_vm_delim(const char *str) {
    if (runtime_strchr(str, '\xfd')) return '\xfd';
    return ']';
}

static char get_svm_delim(const char *str) {
    if (runtime_strchr(str, '\xfc')) return '\xfc';
    return '\\';
}

// Splits string by single character delimiter without bare malloc (using scratch/HAL)
static int split_delim(const char *str, char delim, char parts[64][128], int max_parts) {
    if (!str) return 0;
    int count = 0;
    const char *p = str;
    while (*p && count < max_parts) {
        const char *next = runtime_strchr(p, delim);
        size_t len = next ? (size_t)(next - p) : runtime_strlen(p);
        if (len >= 127) len = 127;
        runtime_memcpy(parts[count], p, len);
        parts[count][len] = '\0';
        count++;
        if (!next) break;
        p = next + 1;
    }
    return count;
}

// Parses Pick dynamic array string into 3-tier Set of Groups
BppSet *set_from_dynarray(void *str_ctx, const char *str) {
    BppSet *root = set_create(8, false);
    if (!root || !str) return root;

    char am = get_am_delim(str);
    char vm = get_vm_delim(str);
    char svm = get_svm_delim(str);

    char attrs[64][128];
    int attr_count = split_delim(str, am, attrs, 64);

    for (int a = 0; a < attr_count; ++a) {
        char vals[32][128];
        int val_count = split_delim(attrs[a], vm, vals, 32);

        char abuf[16];
        runtime_snprintf(abuf, sizeof(abuf), "a%d", a + 1);
        BppGroup *attr_grp = group_create(abuf);
        if (!attr_grp) continue;

        for (int v = 0; v < val_count; ++v) {
            char subvals[16][128];
            int subval_count = split_delim(vals[v], svm, subvals, 16);

            char vbuf[16];
            runtime_snprintf(vbuf, sizeof(vbuf), "v%d", v + 1);

            if (subval_count > 1) {
                BppGroup *val_grp = group_create(vbuf);
                if (val_grp) {
                    for (int s = 0; s < subval_count; ++s) {
                        char sbuf[16];
                        runtime_snprintf(sbuf, sizeof(sbuf), "s%d", s + 1);
                        BValue s_val;
                        s_val.type = VAL_STRING;
                        s_val.as.string = str_create((StringContext *)str_ctx, subvals[s], runtime_strlen(subvals[s]));
                        group_set(str_ctx, val_grp, sbuf, s_val);
                        str_release((StringContext *)str_ctx, s_val.as.string);
                    }
                    BValue vg_val = { .type = VAL_GROUP, .as.group = val_grp };
                    group_set(str_ctx, attr_grp, vbuf, vg_val);
                    group_release(str_ctx, val_grp);
                }
            } else {
                BValue v_val;
                v_val.type = VAL_STRING;
                v_val.as.string = str_create((StringContext *)str_ctx, vals[v], runtime_strlen(vals[v]));
                group_set(str_ctx, attr_grp, vbuf, v_val);
                str_release((StringContext *)str_ctx, v_val.as.string);
            }
        }

        BValue ag_val = { .type = VAL_GROUP, .as.group = attr_grp };
        set_add(str_ctx, root, ag_val);
        group_release(str_ctx, attr_grp);
    }
    return root;
}

// Helper to serialize an individual scalar value into buffer
static void serialize_scalar(char *buf, size_t buf_size, size_t *pos, BValue val) {
    if (*pos >= buf_size - 1) return;
    if (val.type == VAL_STRING && val.as.string) {
        *pos += runtime_snprintf(buf + *pos, buf_size - *pos, "%s", str_data(val.as.string));
    } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
        char nbuf[64];
        num_format_serialize(nbuf, sizeof(nbuf), val.as.number);
        *pos += runtime_snprintf(buf + *pos, buf_size - *pos, "%s", nbuf);
    }
}

// Serializes a 3-tier Set/Group structure back to a Pick dynamic array string
char *set_to_dynarray(void *str_ctx, BValue val) {
    (void)str_ctx;
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;

    size_t buf_size = 4096;
    char *buf = (char *)hal->mem.alloc(buf_size);
    if (!buf) return NULL;
    runtime_memset(buf, 0, buf_size);
    size_t pos = 0;

    if (val.type == VAL_SET && val.as.set) {
        BppSet *s = val.as.set;
        for (int a = 0; a < s->count; ++a) {
            if (a > 0 && pos < buf_size - 1) buf[pos++] = '\xfe';
            BValue a_item = s->items[a];
            if (a_item.type == VAL_GROUP && a_item.as.group) {
                BppGroup *g = a_item.as.group;
                for (int v = 0; v < g->count; ++v) {
                    if (v > 0 && pos < buf_size - 1) buf[pos++] = '\xfd';
                    BValue v_item = g->entries[v].val;
                    if (v_item.type == VAL_GROUP && v_item.as.group) {
                        BppGroup *sg = v_item.as.group;
                        for (int sv = 0; sv < sg->count; ++sv) {
                            if (sv > 0 && pos < buf_size - 1) buf[pos++] = '\xfc';
                            serialize_scalar(buf, buf_size, &pos, sg->entries[sv].val);
                        }
                    } else {
                        serialize_scalar(buf, buf_size, &pos, v_item);
                    }
                }
            } else {
                serialize_scalar(buf, buf_size, &pos, a_item);
            }
        }
    } else if (val.type == VAL_GROUP && val.as.group) {
        BppGroup *g = val.as.group;
        for (int v = 0; v < g->count; ++v) {
            if (v > 0 && pos < buf_size - 1) buf[pos++] = '\xfe';
            serialize_scalar(buf, buf_size, &pos, g->entries[v].val);
        }
    } else if (val.type == VAL_STRING && val.as.string) {
        runtime_strncpy(buf, str_data(val.as.string), buf_size - 1);
    }
    return buf;
}

// Polymorphic Pick extraction for Sets and Groups
BValue set_dyn_extract(void *str_ctx, BValue target, int attr, int val, int subval, BppError *err) {
    (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;
    res.as.string = str_create((StringContext *)str_ctx, "", 0);

    if (attr <= 0) return res;

    if (target.type == VAL_SET && target.as.set) {
        BppSet *s = target.as.set;
        if (attr > s->count) return res;
        BValue a_val = s->items[attr - 1];
        if (val <= 0) return a_val;

        if (a_val.type == VAL_GROUP && a_val.as.group) {
            BppGroup *g = a_val.as.group;
            if (val > g->count) return res;
            BValue v_val = g->entries[val - 1].val;
            if (subval <= 0) return v_val;

            if (v_val.type == VAL_GROUP && v_val.as.group) {
                BppGroup *sg = v_val.as.group;
                if (subval > sg->count) return res;
                return sg->entries[subval - 1].val;
            }
            return (subval == 1) ? v_val : res;
        }
        return (val == 1 && subval <= 1) ? a_val : res;
    }

    if (target.type == VAL_GROUP && target.as.group) {
        BppGroup *g = target.as.group;
        if (attr > g->count) return res;
        BValue a_val = g->entries[attr - 1].val;
        if (val <= 0) return a_val;

        if (a_val.type == VAL_GROUP && a_val.as.group) {
            BppGroup *vg = a_val.as.group;
            if (val > vg->count) return res;
            BValue v_val = vg->entries[val - 1].val;
            if (subval <= 0) return v_val;
            if (v_val.type == VAL_GROUP && v_val.as.group) {
                if (subval > v_val.as.group->count) return res;
                return v_val.as.group->entries[subval - 1].val;
            }
            return (subval == 1) ? v_val : res;
        }
        return (val == 1 && subval <= 1) ? a_val : res;
    }
    return res;
}

// Polymorphic Pick replacement for Sets, Groups, and Dynamic Strings
BValue set_dyn_replace(void *str_ctx, BValue target, int attr, int val, int subval, BValue new_val, BppError *err) {
    (void)err;
    if (attr <= 0) return target;

    if (target.type == VAL_STRING && target.as.string) {
        BppSet *parsed = set_from_dynarray(str_ctx, str_data(target.as.string));
        BValue set_val = { .type = VAL_SET, .as.set = parsed };
        set_dyn_replace(str_ctx, set_val, attr, val, subval, new_val, err);
        char *ser = set_to_dynarray(str_ctx, set_val);
        set_release(str_ctx, parsed);
        if (ser) {
            BValue res;
            res.type = VAL_STRING;
            res.as.string = str_create((StringContext *)str_ctx, ser, runtime_strlen(ser));
            HalContext *hal = hal_get();
            if (hal && hal->mem.free) hal->mem.free(ser);
            return res;
        }
        return target;
    }

    if (target.type == VAL_SET && target.as.set) {
        BppSet *s = target.as.set;
        if (attr > s->count) return target;
        if (val <= 0) {
            set_set(str_ctx, s, attr, new_val);
            return target;
        }
        BValue a_val = s->items[attr - 1];
        if (a_val.type == VAL_GROUP && a_val.as.group) {
            BppGroup *g = a_val.as.group;
            if (val <= g->count) {
                if (subval <= 0) {
                    char vbuf[16];
                    runtime_snprintf(vbuf, sizeof(vbuf), "v%d", val);
                    group_set(str_ctx, g, vbuf, new_val);
                } else {
                    BValue v_val = g->entries[val - 1].val;
                    if (v_val.type == VAL_GROUP && v_val.as.group) {
                        char sbuf[16];
                        runtime_snprintf(sbuf, sizeof(sbuf), "s%d", subval);
                        group_set(str_ctx, v_val.as.group, sbuf, new_val);
                    }
                }
            }
        }
        return target;
    }

    if (target.type == VAL_GROUP && target.as.group) {
        BppGroup *g = target.as.group;
        if (attr <= g->count) {
            if (val <= 0) {
                char abuf[16];
                runtime_snprintf(abuf, sizeof(abuf), "a%d", attr);
                group_set(str_ctx, g, abuf, new_val);
            }
        }
        return target;
    }
    return target;
}

// Polymorphic Pick insertion
BValue set_dyn_insert(void *str_ctx, BValue target, int attr, int val, int subval, BValue new_val, BppError *err) {
    (void)subval; (void)err;
    if (target.type == VAL_SET && target.as.set) {
        if (attr <= 0 || attr > target.as.set->count + 1) {
            set_add(str_ctx, target.as.set, new_val);
        } else {
            // Add and shift
            set_add(str_ctx, target.as.set, new_val);
        }
        return target;
    }
    if (target.type == VAL_GROUP && target.as.group) {
        char kbuf[16];
        runtime_snprintf(kbuf, sizeof(kbuf), "k%d", target.as.group->count + 1);
        group_set(str_ctx, target.as.group, kbuf, new_val);
        return target;
    }
    return target;
}

// Polymorphic Pick deletion
BValue set_dyn_delete(void *str_ctx, BValue target, int attr, int val, int subval, BppError *err) {
    (void)val; (void)subval; (void)err;
    if (target.type == VAL_SET && target.as.set) {
        if (attr > 0 && attr <= target.as.set->count) {
            set_remove_at(str_ctx, target.as.set, attr - 1);
        }
        return target;
    }
    return target;
}

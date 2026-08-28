// FILENAME: struct.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c)
// NEEDED BY: libengine (class.c, context.c, control.c, data.c, dim.c)
// NEEDED BY: libengine (eval_expr_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, isam.c, ops.c, rpn.c, sub_internal.h)
// NEEDED BY: libengine (type.c, vm_internal.h)
// NEEDS: libcore (ctype.h, ctype.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strings.h, strings.c)
// NEEDS: libcore (strops.h, strops.c, struct.h)
// NEEDS: libengine (map.h, map.c, vm.h)
// Provides core logic and interface definitions for struct within BASIC++.
//
// ---- Includes ----

#include "core/struct.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

static void normalize_type_name(char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = (char)runtime_toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

void struct_registry_init(BppTypeRegistry *reg) {
    if (reg) {
        runtime_memset(reg, 0, sizeof(BppTypeRegistry));
    }
}

bool struct_register_type(BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len) {
    if (!reg || !def) return false;
    if (reg->count >= MAX_USER_TYPES) {
        runtime_snprintf(err_buf, err_len, "Maximum user-defined types limit reached (%d)", MAX_USER_TYPES);
        return false;
    }
    
    char norm_name[64];
    normalize_type_name(norm_name, def->name, sizeof(norm_name));
    
    // Check duplicate
    for (int i = 0; i < reg->count; ++i) {
        char existing[64];
        normalize_type_name(existing, reg->types[i].name, sizeof(existing));
        if (runtime_strcmp(existing, norm_name) == 0) {
            runtime_snprintf(err_buf, err_len, "Type or Class '%s' already defined", def->name);
            return false;
        }
    }
    
    BppUserTypeDef *entry = &reg->types[reg->count++];
    runtime_memcpy(entry, def, sizeof(BppUserTypeDef));
    normalize_type_name(entry->name, def->name, sizeof(entry->name));
    return true;
}


const BppUserTypeDef *struct_find_type(const BppTypeRegistry *reg, const char *name) {
    if (!reg || !name) return NULL;
    char norm_name[64];
    normalize_type_name(norm_name, name, sizeof(norm_name));
    
    for (int i = 0; i < reg->count; ++i) {
        if (runtime_strcmp(reg->types[i].name, norm_name) == 0) {
            return &reg->types[i];
        }
    }
    return NULL;
}

BppMap *struct_instantiate(VMContext *vm, const BppTypeRegistry *reg, const char *type_name, char *err_buf, size_t err_len) {
    const BppUserTypeDef *def = struct_find_type(reg, type_name);
    if (!def) {
        runtime_snprintf(err_buf, err_len, "Type '%s' not defined", type_name);
        return NULL;
    }
    
    BppMap *inst = map_create();
    if (!inst) {
        runtime_snprintf(err_buf, err_len, "Out of memory in struct_instantiate");
        return NULL;
    }
    
    // Store type name metadata
    BValue t_val;
    t_val.type = VAL_STRING;
    t_val.as.string = str_create(vm_get_str(vm), def->name, runtime_strlen(def->name));
    map_set(vm_get_str(vm), inst, "__type__", t_val);
    
    // If class inherits from a parent, pre-fill parent fields first
    if (def->parent_name[0] != '\0') {
        const BppUserTypeDef *pdef = struct_find_type(reg, def->parent_name);
        if (pdef) {
            for (int i = 0; i < pdef->field_count; ++i) {
                const BppUserTypeField *pf = &pdef->fields[i];
                BValue pf_val;
                runtime_memset(&pf_val, 0, sizeof(pf_val));
                if (pf->nested_type[0] != '\0') {
                    BppMap *pn = struct_instantiate(vm, reg, pf->nested_type, err_buf, err_len);
                    if (!pn) { map_release(vm_get_str(vm), inst); return NULL; }
                    pf_val.type = VAL_MAP;
                    pf_val.as.map = pn;
                } else if (pf->type == VAL_STRING) {
                    pf_val.type = VAL_STRING;
                    pf_val.as.string = str_create(vm_get_str(vm), "", 0);
                } else if (pf->type == VAL_COMPLEX) {
                    pf_val.type = VAL_COMPLEX;
                } else {
                    pf_val.type = VAL_NUMBER;
                    pf_val.as.number = 0.0;
                }
                map_set(vm_get_str(vm), inst, pf->name, pf_val);
            }
        }
    }

    // Pre-fill default values for each field
    for (int i = 0; i < def->field_count; ++i) {
        const BppUserTypeField *f = &def->fields[i];
        BValue f_val;
        runtime_memset(&f_val, 0, sizeof(f_val));
        
        if (f->nested_type[0] != '\0') {
            // Nested UDT
            BppMap *nested_inst = struct_instantiate(vm, reg, f->nested_type, err_buf, err_len);
            if (!nested_inst) {
                map_release(vm_get_str(vm), inst);
                return NULL;
            }
            f_val.type = VAL_MAP;
            f_val.as.map = nested_inst;
        } else {
            // Primitive types
            if (f->type == VAL_STRING) {
                f_val.type = VAL_STRING;
                f_val.as.string = str_create(vm_get_str(vm), "", 0);
            } else {
                f_val.type = VAL_NUMBER;
                f_val.as.number = 0.0;
            }
        }
        map_set(vm_get_str(vm), inst, f->name, f_val);
    }
    
    return inst;
}

bool struct_copy_instance(VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len) {
    if (!dst || !src) return false;
    
    BValue dst_type, src_type;
    if (!map_get(dst, "__type__", &dst_type) || !map_get(src, "__type__", &src_type)) {
        runtime_snprintf(err_buf, err_len, "Incompatible type copy: missing type metadata");
        return false;
    }
    
    if (dst_type.type != VAL_STRING || src_type.type != VAL_STRING ||
        runtime_strcmp(str_data(dst_type.as.string), str_data(src_type.as.string)) != 0) {
        runtime_snprintf(err_buf, err_len, "Type mismatch in structure assignment (cannot assign %s to %s)",
                 src_type.as.string ? str_data(src_type.as.string) : "Unknown",
                 dst_type.as.string ? str_data(dst_type.as.string) : "Unknown");
        return false;
    }
    
    // Copy all key values
    for (int i = 0; i < map_count(src); ++i) {
        const char *key = map_key(src, i);
        if (runtime_strcmp(key, "__type__") == 0) continue;
        
        BValue val;
        map_get(src, key, &val);
        
        // Add reference counts for strings or nested maps
        if (val.type == VAL_STRING && val.as.string) {
            str_add_ref(val.as.string);
        } else if (val.type == VAL_MAP && val.as.map) {
            map_add_ref(val.as.map);
        }
        
        // Set in destination (this automatically releases previous value inside map_set)
        map_set(vm_get_str(vm), dst, key, val);
    }
    
    return true;
}


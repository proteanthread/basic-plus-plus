/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file struct.c
 * @brief User-Defined Types (UDT / TYPE...END TYPE) and Classes registry implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `struct_registry_init()`, `struct_define()`, `struct_lookup()`, `struct_instantiate()`, and `struct_copy()`.
 *
 * 2. WHY IT EXISTS:
 * Provides QBASIC `TYPE...END TYPE` user-defined records and OOP class layout registry parity.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Registers field names, types, and array dimensions in `BppTypeInfo` structures; instantiates instances as structured `BppMap` instances with default zero-initialization.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "core/struct.h", "runtime/map.h", "runtime/strings.h", "vm/vm.h", <string.h>, <ctype.h>, <stdlib.h>, <stdio.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support class inheritance (`TYPE Extending Base`) or access control modifiers (`PRIVATE`, `PUBLIC`).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory zero-initialization of UDT fields and case-insensitive record type lookup.
 *
 * 8. WHAT TO EXPECT:
 * `struct_instantiate()` returns a `BValue` of type `VAL_MAP` containing pre-allocated UDT fields.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Trace field reference count releases during nested UDT overwrites in `struct_copy()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Active `VMContext` with initialized memory manager.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Case-folding via `toupper((unsigned char)c)`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/map.c
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/core/struct.h
 * - engine/include/runtime/map.h
 * - engine/include/runtime/strings.h
 */

#include "core/struct.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

static void normalize_type_name(char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = (char)toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

void struct_registry_init(BppTypeRegistry *reg) {
    if (reg) {
        memset(reg, 0, sizeof(BppTypeRegistry));
    }
}

bool struct_register_type(BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len) {
    if (!reg || !def) return false;
    if (reg->count >= MAX_USER_TYPES) {
        snprintf(err_buf, err_len, "Maximum user-defined types limit reached (%d)", MAX_USER_TYPES);
        return false;
    }
    
    char norm_name[64];
    normalize_type_name(norm_name, def->name, sizeof(norm_name));
    
    /* Check duplicate */
    for (int i = 0; i < reg->count; ++i) {
        char existing[64];
        normalize_type_name(existing, reg->types[i].name, sizeof(existing));
        if (strcmp(existing, norm_name) == 0) {
            snprintf(err_buf, err_len, "Type or Class '%s' already defined", def->name);
            return false;
        }
    }
    
    BppUserTypeDef *entry = &reg->types[reg->count++];
    memcpy(entry, def, sizeof(BppUserTypeDef));
    normalize_type_name(entry->name, def->name, sizeof(entry->name));
    return true;
}

const BppUserTypeDef *struct_find_type(const BppTypeRegistry *reg, const char *name) {
    if (!reg || !name) return NULL;
    char norm_name[64];
    normalize_type_name(norm_name, name, sizeof(norm_name));
    
    for (int i = 0; i < reg->count; ++i) {
        if (strcmp(reg->types[i].name, norm_name) == 0) {
            return &reg->types[i];
        }
    }
    return NULL;
}

BppMap *struct_instantiate(VMContext *vm, const BppTypeRegistry *reg, const char *type_name, char *err_buf, size_t err_len) {
    const BppUserTypeDef *def = struct_find_type(reg, type_name);
    if (!def) {
        snprintf(err_buf, err_len, "Type '%s' not defined", type_name);
        return NULL;
    }
    
    BppMap *inst = map_create();
    if (!inst) {
        snprintf(err_buf, err_len, "Out of memory in struct_instantiate");
        return NULL;
    }
    
    /* Store type name metadata */
    BValue t_val;
    t_val.type = VAL_STRING;
    t_val.as.string = str_create(vm_get_str(vm), def->name, strlen(def->name));
    map_set(vm_get_str(vm), inst, "__type__", t_val);
    
    /* Pre-fill default values for each field */
    for (int i = 0; i < def->field_count; ++i) {
        const BppUserTypeField *f = &def->fields[i];
        BValue f_val;
        memset(&f_val, 0, sizeof(f_val));
        
        if (f->nested_type[0] != '\0') {
            /* Nested UDT */
            BppMap *nested_inst = struct_instantiate(vm, reg, f->nested_type, err_buf, err_len);
            if (!nested_inst) {
                map_release(vm_get_str(vm), inst);
                return NULL;
            }
            f_val.type = VAL_MAP;
            f_val.as.map = nested_inst;
        } else {
            /* Primitive types */
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
        snprintf(err_buf, err_len, "Incompatible type copy: missing type metadata");
        return false;
    }
    
    if (dst_type.type != VAL_STRING || src_type.type != VAL_STRING ||
        strcmp(str_data(dst_type.as.string), str_data(src_type.as.string)) != 0) {
        snprintf(err_buf, err_len, "Type mismatch in structure assignment (cannot assign %s to %s)",
                 src_type.as.string ? str_data(src_type.as.string) : "Unknown",
                 dst_type.as.string ? str_data(dst_type.as.string) : "Unknown");
        return false;
    }
    
    /* Copy all key values */
    for (int i = 0; i < map_count(src); ++i) {
        const char *key = map_key(src, i);
        if (strcmp(key, "__type__") == 0) continue;
        
        BValue val;
        map_get(src, key, &val);
        
        /* Add reference counts for strings or nested maps */
        if (val.type == VAL_STRING && val.as.string) {
            str_add_ref(val.as.string);
        } else if (val.type == VAL_MAP && val.as.map) {
            map_add_ref(val.as.map);
        }
        
        /* Set in destination (this automatically releases previous value inside map_set) */
        map_set(vm_get_str(vm), dst, key, val);
    }
    
    return true;
}

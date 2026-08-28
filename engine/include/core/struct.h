// FILENAME: struct.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c, struct.c)
// NEEDED BY: libengine (class.c, context.c, control.c, data.c, dim.c)
// NEEDED BY: libengine (eval_expr_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, isam.c, ops.c, rpn.c, sub_internal.h)
// NEEDED BY: libengine (type.c, vm_internal.h)
// NEEDS: libkernel (config.h, types.h)
// Provides core logic and interface definitions for struct within BASIC++.
//
// ---- Includes ----

#ifndef CORE_STRUCT_H
#define CORE_STRUCT_H

#include "types/types.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_TYPE_FIELDS  64
#define MAX_USER_TYPES   64

typedef struct {
    char      name[64];
    ValueType type;
    char      nested_type[64]; // If type is nested UDT
    bool      is_private;
} BppUserTypeField;

typedef struct {
    char             name[64];
    char             parent_name[64]; // Extends parent class/type
    BppUserTypeField fields[MAX_TYPE_FIELDS];
    int              field_count;
    bool             is_class;        // True if CLASS instead of TYPE
    bool             is_record;       // True if RECORD instead of TYPE
} BppUserTypeDef;

typedef struct BppTypeRegistry {
    BppUserTypeDef types[MAX_USER_TYPES];
    int            count;
} BppTypeRegistry;

#include "types/config.h"

struct VMContext;
struct BppMap;
typedef struct VMContext VMContext;
typedef struct BppMap BppMap;

#if SUPPORT_OOP
void struct_registry_init(BppTypeRegistry *reg);
bool struct_register_type(BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len);
const BppUserTypeDef *struct_find_type(const BppTypeRegistry *reg, const char *name);
BppMap *struct_instantiate(VMContext *vm, const BppTypeRegistry *reg, const char *type_name, char *err_buf, size_t err_len);
bool struct_copy_instance(VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len);
#else
static inline void struct_registry_init(BppTypeRegistry *reg) { (void)reg; }
static inline bool struct_register_type(BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len) { (void)reg; (void)def; (void)err_buf; (void)err_len; return false; }
static inline const BppUserTypeDef *struct_find_type(const BppTypeRegistry *reg, const char *name) { (void)reg; (void)name; return NULL; }
static inline BppMap *struct_instantiate(VMContext *vm, const BppTypeRegistry *reg, const char *type_name, char *err_buf, size_t err_len) { (void)vm; (void)reg; (void)type_name; (void)err_buf; (void)err_len; return NULL; }
static inline bool struct_copy_instance(VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len) { (void)vm; (void)dst; (void)src; (void)err_buf; (void)err_len; return false; }
#endif

#endif // CORE_STRUCT_H

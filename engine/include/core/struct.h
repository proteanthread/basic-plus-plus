/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_struct.h
 * @brief User-Defined Types (UDT) and Class structure definitions for BASIC++.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines structures representing custom records (TYPE ... END TYPE)
 *   and class structures (CLASS ... END CLASS) and declares registry management APIs.
 * - Why it exists: Supports object-oriented and structured record programming models in Phase 26.
 * - Why it works this way: Uses a static registry mapping names to member fields. Variable
 *   instances are instantiated as dynamic key-value maps (BppMap) to support fields and nesting.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Maximum fields count (64), maximum types count (64).
 * - What cannot be changed: Opaque map-based runtime instantiation layout.
 * - What to expect: Type lookup returns structured descriptors detailing name, type, and nesting.
 * - What to do if something breaks: Trace registry bounds and check name normalization.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Type names are unique and case-insensitive.
 * - Portability concerns: Standard C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add virtual method tables (VMT) to support polymorphism.
 * - How to write external extensions: External modules query structure schemas via registry lookups.
 */

#ifndef BPP_STRUCT_H
#define BPP_STRUCT_H

#include "types/types.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_TYPE_FIELDS  64
#define MAX_USER_TYPES   64

typedef struct {
    char      name[64];
    ValueType type;
    char      nested_type[64]; /* If type is nested UDT */
} BppUserTypeField;

typedef struct {
    char             name[64];
    BppUserTypeField fields[MAX_TYPE_FIELDS];
    int              field_count;
    bool             is_class;        /* True if CLASS instead of TYPE */
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

#if BPP_SUPPORT_OOP
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

#endif /* BPP_STRUCT_H */

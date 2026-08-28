// FILENAME: nil_bead.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nil_bead.c)
// NEEDED BY: libengine (func_nil_bead.c, stmt_nil_bead.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Implements RFC 51 Bead, Vector, and PLEX serialization tags and formats.
//
// ---- Includes ----

#ifndef RUNTIME_NIL_BEAD_H
#define RUNTIME_NIL_BEAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types/types.h"
#include "vm/vm.h"

// RFC 51 Bead Type Identifiers
typedef enum {
    NIL_BEAD_NONE        = 0x00,
    NIL_BEAD_CHAR        = 0x01, // 8-bit character string
    NIL_BEAD_INT32       = 0x02, // 32-bit signed integer
    NIL_BEAD_FLPNUM      = 0x03, // Double precision float (IEEE 754 / RFC 51)
    NIL_BEAD_VECTOR      = 0x10, // 1D/2D homogenous/heterogenous array
    NIL_BEAD_COMPVECTOR  = 0x11, // Compressed array vector
    NIL_BEAD_PLEX        = 0x20  // Hierarchical directed map / object
} NilBeadType;

// Serialization / Deserialization helpers
size_t nil_bead_serialize_value(VMContext *vm, BValue val, uint8_t *out_buf, size_t max_out);
bool   nil_bead_deserialize_value(VMContext *vm, const uint8_t *buf, size_t len, BValue *out_val);

#endif // RUNTIME_NIL_BEAD_H

// FILENAME: nil_bead.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_nil_bead.c, stmt_nil_bead.c)
// NEEDS: libcore (arrays.h, arrays.c, nil_bead.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (map.h, map.c, string.c)
// Implements RFC 51 Bead, Vector, and PLEX serialization tags and formats.
//
// ---- Includes ----

#include "runtime/nil_bead.h"
#include "runtime/strings.h"
#include "runtime/map.h"
#include "runtime/arrays.h"

#include <string.h>

size_t nil_bead_serialize_value(VMContext *vm, BValue val, uint8_t *out, size_t max_out) {
    (void)vm;
    if (!out || max_out < 8) return 0;

    size_t written = 0;

    switch (val.type) {
        case VAL_NUMBER: {
            out[written++] = (uint8_t)NIL_BEAD_FLPNUM;
            double num = val.as.number;
            if (written + sizeof(double) <= max_out) {
                memcpy(out + written, &num, sizeof(double));
                written += sizeof(double);
            }
            break;
        }
        case VAL_STRING: {
            out[written++] = (uint8_t)NIL_BEAD_CHAR;
            if (val.as.string) {
                const char *data = str_data(val.as.string);
                size_t len = str_len(val.as.string);
                uint16_t u16_len = (uint16_t)(len < 65535 ? len : 65535);
                if (written + 2 + u16_len <= max_out) {
                    memcpy(out + written, &u16_len, 2);
                    written += 2;
                    memcpy(out + written, data, u16_len);
                    written += u16_len;
                }
            } else {
                uint16_t u16_len = 0;
                memcpy(out + written, &u16_len, 2);
                written += 2;
            }
            break;
        }
        default: {
            out[written++] = (uint8_t)NIL_BEAD_NONE;
            break;
        }
    }

    return written;
}

bool nil_bead_deserialize_value(VMContext *vm, const uint8_t *buf, size_t len, BValue *out_val) {
    if (!buf || len == 0 || !out_val) return false;

    uint8_t tag = buf[0];
    switch (tag) {
        case NIL_BEAD_FLPNUM: {
            if (len >= 1 + sizeof(double)) {
                double num;
                memcpy(&num, buf + 1, sizeof(double));
                out_val->type = VAL_NUMBER;
                out_val->as.number = num;
                return true;
            }
            break;
        }
        case NIL_BEAD_CHAR: {
            if (len >= 3) {
                uint16_t u16_len;
                memcpy(&u16_len, buf + 1, 2);
                if (len >= (size_t)(3 + u16_len)) {
                    out_val->type = VAL_STRING;
                    out_val->as.string = str_create(vm_get_str(vm), (const char *)(buf + 3), u16_len);
                    return true;
                }
            }
            break;
        }
        default:
            break;
    }

    out_val->type = VAL_NONE;
    out_val->as.number = 0.0;
    return false;
}

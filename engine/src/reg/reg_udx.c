// FILENAME: reg_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, BASIC++ runtime
// NEEDS: libreg (reg_udx.h), runtime (strops.h, strings.h)
// Implements Universal Data Exchange (UDX / XCHG.*) register subsystem.

#include "reg/reg_udx.h"
#include "reg/reg_hw.h"
#include "reg/reg_buffer.h"
#include "eval/rpn.h"
#include "eval/pn.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"

// Active global UDX register context
static RegUdxContext s_active_udx_context;

// Helper to release string reference if val is a string
static void udx_release_bval(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_release(NULL, val.as.string);
    }
}

// Helper to retain string reference if val is a string
static void udx_retain_bval(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
}

void reg_udx_reset(void) {
    for (int i = 0; i < 16; i++) {
        udx_release_bval(s_active_udx_context.poly_r[i]);
        s_active_udx_context.poly_r[i].type = VAL_NUMBER;
        s_active_udx_context.poly_r[i].as.number = 0.0;
        s_active_udx_context.num_r[i] = 0.0;
    }
    udx_release_bval(s_active_udx_context.acc_a);
    s_active_udx_context.acc_a.type = VAL_NUMBER;
    s_active_udx_context.acc_a.as.number = 0.0;
    s_active_udx_context.acc_b = 0.0;
}

RegUdxContext* reg_udx_get_context(void) {
    return &s_active_udx_context;
}

static int parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static const char* strip_udx_prefix(const char *name) {
    if (!name) return "";
    if (runtime_strncasecmp(name, "XCHG.", 5) == 0) return name + 5;
    if (runtime_strncasecmp(name, "XCHG_", 5) == 0) return name + 5;
    if (runtime_strncasecmp(name, "UDX.", 4) == 0) return name + 4;
    if (runtime_strncasecmp(name, "UDX_", 4) == 0) return name + 4;
    return name;
}

bool reg_udx_get(const char *name, BValue *out_val) {
    if (!out_val) return false;
    const char *r = strip_udx_prefix(name);

    // Check accumulator variants: ACC, ACC.A, ACC.B, ACC.C, ACCA, ACCB, ACCC
    if (runtime_strcasecmp(r, "ACC") == 0 || runtime_strcasecmp(r, "ACC.A") == 0 ||
        runtime_strcasecmp(r, "ACCA") == 0) {
        *out_val = reg_udx_get_acc('A');
        return true;
    }
    if (runtime_strcasecmp(r, "ACC.B") == 0 || runtime_strcasecmp(r, "ACCB") == 0) {
        *out_val = reg_udx_get_acc('B');
        return true;
    }
    if (runtime_strcasecmp(r, "ACC.C") == 0 || runtime_strcasecmp(r, "ACCC") == 0 ||
        runtime_strcasecmp(r, "ACC.ACC.C") == 0) {
        *out_val = reg_udx_get_acc('C');
        return true;
    }

    // Check polymorphic registers R0 .. RF
    if ((r[0] == 'R' || r[0] == 'r') && r[1] != '\0' && r[2] == '\0') {
        int idx = parse_hex_digit(r[1]);
        if (idx >= 0 && idx < 16) {
            *out_val = s_active_udx_context.poly_r[idx];
            udx_retain_bval(*out_val);
            return true;
        }
    }

    // Check numeric registers 0 .. F
    if (r[0] != '\0' && r[1] == '\0') {
        int idx = parse_hex_digit(r[0]);
        if (idx >= 0 && idx < 16) {
            out_val->type = VAL_NUMBER;
            out_val->as.number = s_active_udx_context.num_r[idx];
            return true;
        }
    }

    return false;
}

bool reg_udx_set(const char *name, BValue val) {
    const char *r = strip_udx_prefix(name);

    // Check accumulators
    if (runtime_strcasecmp(r, "ACC") == 0 || runtime_strcasecmp(r, "ACC.A") == 0 ||
        runtime_strcasecmp(r, "ACCA") == 0) {
        reg_udx_set_acc('A', val);
        return true;
    }
    if (runtime_strcasecmp(r, "ACC.B") == 0 || runtime_strcasecmp(r, "ACCB") == 0) {
        reg_udx_set_acc('B', val);
        return true;
    }
    if (runtime_strcasecmp(r, "ACC.C") == 0 || runtime_strcasecmp(r, "ACCC") == 0 ||
        runtime_strcasecmp(r, "ACC.ACC.C") == 0) {
        reg_udx_set_acc('C', val);
        return true;
    }

    // Check polymorphic registers R0 .. RF
    if ((r[0] == 'R' || r[0] == 'r') && r[1] != '\0' && r[2] == '\0') {
        int idx = parse_hex_digit(r[1]);
        if (idx >= 0 && idx < 16) {
            udx_retain_bval(val);
            udx_release_bval(s_active_udx_context.poly_r[idx]);
            s_active_udx_context.poly_r[idx] = val;
            return true;
        }
    }

    // Check numeric registers 0 .. F
    if (r[0] != '\0' && r[1] == '\0') {
        int idx = parse_hex_digit(r[0]);
        if (idx >= 0 && idx < 16) {
            double num = 0.0;
            if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                num = val.as.number;
            }
            s_active_udx_context.num_r[idx] = num;
            return true;
        }
    }

    return false;
}

bool reg_udx_get_by_index(int index, bool numeric_bank, BValue *out_val) {
    if (!out_val || index < 0 || index >= 16) return false;
    if (numeric_bank) {
        out_val->type = VAL_NUMBER;
        out_val->as.number = s_active_udx_context.num_r[index];
    } else {
        *out_val = s_active_udx_context.poly_r[index];
        udx_retain_bval(*out_val);
    }
    return true;
}

bool reg_udx_set_by_index(int index, bool numeric_bank, BValue val) {
    if (index < 0 || index >= 16) return false;
    if (numeric_bank) {
        double num = 0.0;
        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) num = val.as.number;
        s_active_udx_context.num_r[index] = num;
    } else {
        udx_retain_bval(val);
        udx_release_bval(s_active_udx_context.poly_r[index]);
        s_active_udx_context.poly_r[index] = val;
    }
    return true;
}

BValue reg_udx_get_acc(char bank) {
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    if (bank == 'B' || bank == 'b') {
        res.type = VAL_NUMBER;
        res.as.number = s_active_udx_context.acc_b;
        return res;
    }
    if (bank == 'C' || bank == 'c') {
        // Combined accumulator: prefer polymorphic acc_a if string, else numeric if non-zero
        if (s_active_udx_context.acc_a.type == VAL_STRING) {
            res = s_active_udx_context.acc_a;
            udx_retain_bval(res);
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (s_active_udx_context.acc_b != 0.0) ? s_active_udx_context.acc_b : s_active_udx_context.acc_a.as.number;
        return res;
    }

    // Default Bank A
    res = s_active_udx_context.acc_a;
    udx_retain_bval(res);
    return res;
}

void reg_udx_set_acc(char bank, BValue val) {
    if (bank == 'B' || bank == 'b') {
        double num = 0.0;
        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) num = val.as.number;
        s_active_udx_context.acc_b = num;
        return;
    }
    if (bank == 'C' || bank == 'c') {
        // Mirror to both accumulators
        udx_retain_bval(val);
        udx_release_bval(s_active_udx_context.acc_a);
        s_active_udx_context.acc_a = val;
        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            s_active_udx_context.acc_b = val.as.number;
        }
        return;
    }

    // Default Bank A
    udx_retain_bval(val);
    udx_release_bval(s_active_udx_context.acc_a);
    s_active_udx_context.acc_a = val;
}

bool reg_lookup_builtin(const char *name, BValue *out_val) {
    if (!name || !out_val) return false;

    // 1. Hardware registers: REG.*, CPU.*, CPU86.*, BIOS.*
    if (runtime_strncasecmp(name, "REG.", 4) == 0 || runtime_strncasecmp(name, "REG_", 4) == 0 ||
        runtime_strncasecmp(name, "CPU86.", 6) == 0 || runtime_strncasecmp(name, "CPU86_", 6) == 0 ||
        runtime_strncasecmp(name, "CPU.", 4) == 0 || runtime_strncasecmp(name, "CPU_", 4) == 0 ||
        runtime_strncasecmp(name, "BIOS.", 5) == 0 || runtime_strncasecmp(name, "BIOS_", 5) == 0) {
        bool found = false;
        double val = reg_hw_get(name, &found);
        if (found) {
            out_val->type = VAL_INTEGER;
            out_val->as.number = val;
            return true;
        }
    }

    // 2. UDX exchange registers: XCHG.*, UDX.*
    if (runtime_strncasecmp(name, "XCHG.", 5) == 0 || runtime_strncasecmp(name, "XCHG_", 5) == 0 ||
        runtime_strncasecmp(name, "UDX.", 4) == 0 || runtime_strncasecmp(name, "UDX_", 4) == 0) {
        if (reg_udx_get(name, out_val)) return true;
    }

    // 3. First-class separate buffers: FIFO.*, LIFO.*, LILO.*, FILO.*
    if (runtime_strncasecmp(name, "FIFO.", 5) == 0 || runtime_strncasecmp(name, "LIFO.", 5) == 0 ||
        runtime_strncasecmp(name, "LILO.", 5) == 0 || runtime_strncasecmp(name, "FILO.", 5) == 0) {
        const char *prop = name + 5;
        char disc_name[8];
        runtime_memcpy(disc_name, name, 4);
        disc_name[4] = '\0';

        if (runtime_strcasecmp(prop, "COUNT") == 0) {
            out_val->type = VAL_INTEGER;
            out_val->as.number = (double)reg_buffer_count_by_name(disc_name);
            return true;
        }
        if (runtime_strcasecmp(prop, "PEEK") == 0) {
            RegBufferChannel *ch = reg_buffer_channel_get(disc_name, NULL);
            if (reg_buffer_peek(ch, out_val)) return true;
        }
        if (runtime_strcasecmp(prop, "POP") == 0) {
            if (reg_buffer_pop_by_name(disc_name, out_val)) return true;
        }
    }

    return false;
}

bool reg_is_hardware_domain(const char *name) {
    if (!name) return false;
    return (runtime_strncasecmp(name, "REG.", 4) == 0 ||
            runtime_strncasecmp(name, "CPU.", 4) == 0 ||
            runtime_strncasecmp(name, "CPU86.", 6) == 0 ||
            runtime_strncasecmp(name, "CPU8086.", 8) == 0 ||
            runtime_strncasecmp(name, "CPU8088.", 8) == 0 ||
            runtime_strncasecmp(name, "BIOS.", 5) == 0);
}

bool reg_is_math_domain(const char *name) {
    if (!name) return false;
    return (runtime_strncasecmp(name, "STACK.", 6) == 0 ||
            runtime_strncasecmp(name, "RPN.", 4) == 0 ||
            runtime_strncasecmp(name, "PN.", 3) == 0);
}

bool reg_is_udx_domain(const char *name) {
    if (!name) return false;
    return (runtime_strncasecmp(name, "XCHG.", 5) == 0 ||
            runtime_strncasecmp(name, "UDX.", 4) == 0);
}

bool reg_assign_builtin(const char *target_name, BValue val, const char *expr_hint, BppError *err) {
    if (!target_name) return false;

    bool is_hw = reg_is_hardware_domain(target_name);
    bool is_math = reg_is_math_domain(target_name);
    bool is_udx = reg_is_udx_domain(target_name);

    if (!is_hw && !is_math && !is_udx) return false;

    // Strict boundary enforcement: direct cross-domain assignment is prohibited
    if (expr_hint && expr_hint[0] != '\0') {
        const char *hint = expr_hint;
        while (*hint == ' ' || *hint == '\t') hint++;
        bool rhs_is_hw = reg_is_hardware_domain(hint);
        bool rhs_is_math = reg_is_math_domain(hint);

        if (is_hw && rhs_is_math) {
            if (err) {
                err->code = 5;
                err->message = "Illegal function call: Direct cross-domain register assignment prohibited; use UDX or buffers";
            }
            return false;
        }
        if (is_math && rhs_is_hw) {
            if (err) {
                err->code = 5;
                err->message = "Illegal function call: Direct cross-domain register assignment prohibited; use UDX or buffers";
            }
            return false;
        }
    }

    if (is_hw) {
        double num = (val.type == VAL_INTEGER || val.type == VAL_NUMBER) ? val.as.number : 0.0;
        if (!reg_hw_set(target_name, num)) {
            if (err) {
                err->code = 5;
                err->message = "Unknown hardware register";
            }
        }
        return true;
    }

    if (is_math) {
        if (runtime_strncasecmp(target_name, "PN.", 3) == 0 || runtime_strncasecmp(target_name, "PN_", 3) == 0) {
            if (!pn_reg_set(target_name, val)) {
                if (err) {
                    err->code = 5;
                    err->message = "Unknown prefix calculator register";
                }
            }
        } else {
            if (!rpn_reg_set(target_name, val)) {
                if (err) {
                    err->code = 5;
                    err->message = "Unknown stack/RPN register";
                }
            }
        }
        return true;
    }

    if (is_udx) {
        if (!reg_udx_set(target_name, val)) {
            if (err) {
                err->code = 5;
                err->message = "Unknown UDX register";
            }
        }
        return true;
    }

    return false;
}

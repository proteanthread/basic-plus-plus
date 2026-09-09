// FILENAME: reg_hw.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, libhardware, BASIC++ runtime
// NEEDS: libreg (reg_hw.h), runtime (strops.h)
// Implements unified 64-bit universal hardware register architecture, personality slots, and real-time telemetry (REG.*).

#include "reg/reg_hw.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/num_parse.h"
#include "runtime/format/snprintf.h"

// Active universal hardware register context shared by CPU, BIOS, and IoT subsystems
static RegHwContext s_active_hw_context = {
    .r = {0},
    .acc = 0,
    .status = 0x0002,
    .pc = 0x0100,
    .sp = 0xFFFE,
    .arch_slot = ARCH_SLOT_X86,
    .dirty_mask = 0,
    .epoch = 0,
    .halted = false
};

// Real-time observer registration
static RegObserverFn s_observer_fn = NULL;
static void *s_observer_user_data = NULL;

// Hardware HAL & MMIO callbacks
static RegHwReadFn s_hal_read_fn = NULL;
static RegHwWriteFn s_hal_write_fn = NULL;
static void *s_hal_user_data = NULL;

typedef struct {
    uintptr_t addr;
    uint8_t width;
} RegHwMmioBinding;

static RegHwMmioBinding s_mmio_bindings[32] = {{0, 0}};

// Map of canonical legacy register indices for REG[0..13]
static const char *s_reg_index_names[] = {
    "AX", "BX", "CX", "DX", "SI", "DI", "BP", "SP",
    "IP", "CS", "DS", "ES", "SS", "FLAGS"
};

void reg_hw_reset(void) {
    for (int i = 0; i < 32; i++) {
        s_active_hw_context.r[i] = 0;
        s_mmio_bindings[i].addr = 0;
        s_mmio_bindings[i].width = 0;
    }
    s_active_hw_context.acc = 0;
    s_active_hw_context.status = 0x0002;
    s_active_hw_context.pc = 0x0100;
    s_active_hw_context.sp = 0xFFFE;
    s_active_hw_context.arch_slot = ARCH_SLOT_X86;
    s_active_hw_context.dirty_mask = 0;
    s_active_hw_context.epoch = 0;
    s_active_hw_context.halted = false;

    // Synchronize legacy pointers
    s_active_hw_context.r[7] = 0xFFFE;
    s_active_hw_context.r[8] = 0x0100;
}

RegHwContext* reg_hw_get_context(void) {
    return &s_active_hw_context;
}

static const char* strip_reg_prefix(const char *name) {
    if (!name) return "";
    if (runtime_strncasecmp(name, "REG.", 4) == 0) return name + 4;
    if (runtime_strncasecmp(name, "REG_", 4) == 0) return name + 4;
    if (runtime_strncasecmp(name, "CPU86.", 6) == 0) return name + 6;
    if (runtime_strncasecmp(name, "CPU86_", 6) == 0) return name + 6;
    if (runtime_strncasecmp(name, "CPU.", 4) == 0) return name + 4;
    if (runtime_strncasecmp(name, "CPU_", 4) == 0) return name + 4;
    if (runtime_strncasecmp(name, "BIOS.", 5) == 0) return name + 5;
    if (runtime_strncasecmp(name, "BIOS_", 5) == 0) return name + 5;
    return name;
}

static void notify_reg_observer(const char *reg_name, double old_val, double new_val) {
    if (s_observer_fn) {
        s_observer_fn("HARDWARE", reg_name, old_val, new_val, s_observer_user_data);
    }
}

void reg_register_observer(RegObserverFn fn, void *user_data) {
    s_observer_fn = fn;
    s_observer_user_data = user_data;
}

void reg_hw_bind_hal_hooks(RegHwReadFn read_fn, RegHwWriteFn write_fn, void *user_data) {
    s_hal_read_fn = read_fn;
    s_hal_write_fn = write_fn;
    s_hal_user_data = user_data;
}

void reg_hw_bind_mmio(int reg_idx, uintptr_t mmio_addr, uint8_t width) {
    if (reg_idx >= 0 && reg_idx < 32) {
        s_mmio_bindings[reg_idx].addr = mmio_addr;
        s_mmio_bindings[reg_idx].width = width;
    }
}

void reg_hw_set_arch_slot(uint16_t slot_id) {
    s_active_hw_context.arch_slot = slot_id;
}

uint16_t reg_hw_get_arch_slot(void) {
    return s_active_hw_context.arch_slot;
}

uint64_t reg_hw_get_dirty_mask(void) {
    return s_active_hw_context.dirty_mask;
}

void reg_hw_clear_dirty_mask(void) {
    s_active_hw_context.dirty_mask = 0;
}

uint64_t reg_hw_get_epoch(void) {
    return s_active_hw_context.epoch;
}

double reg_hw_get(const char *reg_name, bool *found) {
    if (found) *found = true;
    const char *r = strip_reg_prefix(reg_name);

    // 1. Generic 64-bit registers: R0..R31
    if ((r[0] == 'R' || r[0] == 'r') && runtime_isdigit((unsigned char)r[1])) {
        int idx = runtime_atoi(r + 1);
        if (idx >= 0 && idx < 32) {
            // Check MMIO binding
            if (s_mmio_bindings[idx].addr != 0) {
                volatile void *ptr = (volatile void *)s_mmio_bindings[idx].addr;
                if (s_mmio_bindings[idx].width == 1) return (double)(*(volatile uint8_t*)ptr);
                if (s_mmio_bindings[idx].width == 2) return (double)(*(volatile uint16_t*)ptr);
                if (s_mmio_bindings[idx].width == 4) return (double)(*(volatile uint32_t*)ptr);
                if (s_mmio_bindings[idx].width == 8) return (double)(*(volatile uint64_t*)ptr);
            }
            // Check HAL callback
            if (s_hal_read_fn) {
                return (double)s_hal_read_fn(idx, s_hal_user_data);
            }
            return (double)s_active_hw_context.r[idx];
        }
    }

    // 2. Accumulator: ACC, A
    if (runtime_strcasecmp(r, "ACC") == 0 || runtime_strcasecmp(r, "A") == 0) {
        return (double)s_active_hw_context.acc;
    }

    // 3. Status and Flags: STATUS, FLAGS, FL
    if (runtime_strcasecmp(r, "STATUS") == 0 || runtime_strcasecmp(r, "FLAGS") == 0 || runtime_strcasecmp(r, "FL") == 0) {
        return (double)s_active_hw_context.status;
    }

    // 4. Program Counter: PC, IP
    if (runtime_strcasecmp(r, "PC") == 0 || runtime_strcasecmp(r, "IP") == 0) {
        return (double)s_active_hw_context.pc;
    }

    // 5. Stack Pointer: SP
    if (runtime_strcasecmp(r, "SP") == 0) {
        return (double)s_active_hw_context.sp;
    }

    // 6. Architecture slot and telemetry query
    if (runtime_strcasecmp(r, "SLOT") == 0 || runtime_strcasecmp(r, "ARCH") == 0) {
        return (double)s_active_hw_context.arch_slot;
    }
    if (runtime_strcasecmp(r, "DIRTY") == 0) {
        return (double)s_active_hw_context.dirty_mask;
    }
    if (runtime_strcasecmp(r, "EPOCH") == 0) {
        return (double)s_active_hw_context.epoch;
    }

    // 7. Legacy x86 personality mappings (active in x86 slot or flexible fallback)
    if (runtime_strcasecmp(r, "AX") == 0) return (double)(s_active_hw_context.r[0] & 0xFFFF);
    if (runtime_strcasecmp(r, "BX") == 0) return (double)(s_active_hw_context.r[1] & 0xFFFF);
    if (runtime_strcasecmp(r, "CX") == 0) return (double)(s_active_hw_context.r[2] & 0xFFFF);
    if (runtime_strcasecmp(r, "DX") == 0) return (double)(s_active_hw_context.r[3] & 0xFFFF);

    if (runtime_strcasecmp(r, "AL") == 0) return (double)(s_active_hw_context.r[0] & 0xFF);
    if (runtime_strcasecmp(r, "AH") == 0) return (double)((s_active_hw_context.r[0] >> 8) & 0xFF);
    if (runtime_strcasecmp(r, "BL") == 0) return (double)(s_active_hw_context.r[1] & 0xFF);
    if (runtime_strcasecmp(r, "BH") == 0) return (double)((s_active_hw_context.r[1] >> 8) & 0xFF);
    if (runtime_strcasecmp(r, "CL") == 0) return (double)(s_active_hw_context.r[2] & 0xFF);
    if (runtime_strcasecmp(r, "CH") == 0) return (double)((s_active_hw_context.r[2] >> 8) & 0xFF);
    if (runtime_strcasecmp(r, "DL") == 0) return (double)(s_active_hw_context.r[3] & 0xFF);
    if (runtime_strcasecmp(r, "DH") == 0) return (double)((s_active_hw_context.r[3] >> 8) & 0xFF);

    if (runtime_strcasecmp(r, "SI") == 0) return (double)(s_active_hw_context.r[4] & 0xFFFF);
    if (runtime_strcasecmp(r, "DI") == 0) return (double)(s_active_hw_context.r[5] & 0xFFFF);
    if (runtime_strcasecmp(r, "BP") == 0) return (double)(s_active_hw_context.r[6] & 0xFFFF);
    if (runtime_strcasecmp(r, "CS") == 0) return (double)(s_active_hw_context.r[9] & 0xFFFF);
    if (runtime_strcasecmp(r, "DS") == 0) return (double)(s_active_hw_context.r[10] & 0xFFFF);
    if (runtime_strcasecmp(r, "ES") == 0) return (double)(s_active_hw_context.r[11] & 0xFFFF);
    if (runtime_strcasecmp(r, "SS") == 0) return (double)(s_active_hw_context.r[12] & 0xFFFF);

    // Flag bits access
    if (runtime_strcasecmp(r, "CF") == 0) return (s_active_hw_context.status & 0x0001) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "PF") == 0) return (s_active_hw_context.status & 0x0004) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "AF") == 0) return (s_active_hw_context.status & 0x0010) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "ZF") == 0) return (s_active_hw_context.status & 0x0040) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "SF") == 0) return (s_active_hw_context.status & 0x0080) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "TF") == 0) return (s_active_hw_context.status & 0x0100) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "IF") == 0) return (s_active_hw_context.status & 0x0200) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "DF") == 0) return (s_active_hw_context.status & 0x0400) ? 1.0 : 0.0;
    if (runtime_strcasecmp(r, "OF") == 0) return (s_active_hw_context.status & 0x0800) ? 1.0 : 0.0;

    if (found) *found = false;
    return 0.0;
}

bool reg_hw_set(const char *reg_name, double val) {
    const char *r = strip_reg_prefix(reg_name);
    uint64_t u64 = (uint64_t)val;
    uint16_t u16 = (uint16_t)((uint32_t)val & 0xFFFF);
    uint8_t u8 = (uint8_t)((uint32_t)val & 0xFF);
    double old_val = 0.0;
    bool found = false;

    // 1. Generic 64-bit registers: R0..R31
    if ((r[0] == 'R' || r[0] == 'r') && runtime_isdigit((unsigned char)r[1])) {
        int idx = runtime_atoi(r + 1);
        if (idx >= 0 && idx < 32) {
            old_val = (double)s_active_hw_context.r[idx];
            s_active_hw_context.r[idx] = u64;

            // Handle MMIO binding
            if (s_mmio_bindings[idx].addr != 0) {
                volatile void *ptr = (volatile void *)s_mmio_bindings[idx].addr;
                if (s_mmio_bindings[idx].width == 1) *(volatile uint8_t*)ptr = (uint8_t)u64;
                else if (s_mmio_bindings[idx].width == 2) *(volatile uint16_t*)ptr = (uint16_t)u64;
                else if (s_mmio_bindings[idx].width == 4) *(volatile uint32_t*)ptr = (uint32_t)u64;
                else if (s_mmio_bindings[idx].width == 8) *(volatile uint64_t*)ptr = u64;
            }
            // Handle HAL callback
            if (s_hal_write_fn) {
                s_hal_write_fn(idx, u64, s_hal_user_data);
            }

            s_active_hw_context.dirty_mask |= (1ULL << idx);
            s_active_hw_context.epoch++;
            notify_reg_observer(r, old_val, val);

            // Keep SP/PC synched if R7/R8 modified
            if (idx == 7) s_active_hw_context.sp = u64;
            if (idx == 8) s_active_hw_context.pc = u64;
            return true;
        }
    }

    // 2. Accumulator: ACC, A
    if (runtime_strcasecmp(r, "ACC") == 0 || runtime_strcasecmp(r, "A") == 0) {
        old_val = (double)s_active_hw_context.acc;
        s_active_hw_context.acc = u64;
        s_active_hw_context.epoch++;
        notify_reg_observer("ACC", old_val, val);
        return true;
    }

    // 3. Status and Flags: STATUS, FLAGS, FL
    if (runtime_strcasecmp(r, "STATUS") == 0 || runtime_strcasecmp(r, "FLAGS") == 0 || runtime_strcasecmp(r, "FL") == 0) {
        old_val = (double)s_active_hw_context.status;
        s_active_hw_context.status = u64;
        s_active_hw_context.epoch++;
        notify_reg_observer("STATUS", old_val, val);
        return true;
    }

    // 4. Program Counter: PC, IP
    if (runtime_strcasecmp(r, "PC") == 0 || runtime_strcasecmp(r, "IP") == 0) {
        old_val = (double)s_active_hw_context.pc;
        s_active_hw_context.pc = u64;
        s_active_hw_context.r[8] = u64;
        s_active_hw_context.dirty_mask |= (1ULL << 8);
        s_active_hw_context.epoch++;
        notify_reg_observer("PC", old_val, val);
        return true;
    }

    // 5. Stack Pointer: SP
    if (runtime_strcasecmp(r, "SP") == 0) {
        old_val = (double)s_active_hw_context.sp;
        s_active_hw_context.sp = u64;
        s_active_hw_context.r[7] = u64;
        s_active_hw_context.dirty_mask |= (1ULL << 7);
        s_active_hw_context.epoch++;
        notify_reg_observer("SP", old_val, val);
        return true;
    }

    // 6. Architecture slot
    if (runtime_strcasecmp(r, "SLOT") == 0 || runtime_strcasecmp(r, "ARCH") == 0) {
        s_active_hw_context.arch_slot = (uint16_t)u16;
        s_active_hw_context.epoch++;
        return true;
    }

    // 7. Legacy x86 register mapping
    if (runtime_strcasecmp(r, "AX") == 0) {
        old_val = (double)(s_active_hw_context.r[0] & 0xFFFF);
        s_active_hw_context.r[0] = (s_active_hw_context.r[0] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 0);
        s_active_hw_context.epoch++;
        notify_reg_observer("AX", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "BX") == 0) {
        old_val = (double)(s_active_hw_context.r[1] & 0xFFFF);
        s_active_hw_context.r[1] = (s_active_hw_context.r[1] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 1);
        s_active_hw_context.epoch++;
        notify_reg_observer("BX", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "CX") == 0) {
        old_val = (double)(s_active_hw_context.r[2] & 0xFFFF);
        s_active_hw_context.r[2] = (s_active_hw_context.r[2] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 2);
        s_active_hw_context.epoch++;
        notify_reg_observer("CX", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "DX") == 0) {
        old_val = (double)(s_active_hw_context.r[3] & 0xFFFF);
        s_active_hw_context.r[3] = (s_active_hw_context.r[3] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 3);
        s_active_hw_context.epoch++;
        notify_reg_observer("DX", old_val, val);
        return true;
    }

    if (runtime_strcasecmp(r, "AL") == 0) {
        old_val = (double)(s_active_hw_context.r[0] & 0xFF);
        s_active_hw_context.r[0] = (s_active_hw_context.r[0] & ~0xFFULL) | u8;
        s_active_hw_context.dirty_mask |= (1ULL << 0);
        s_active_hw_context.epoch++;
        notify_reg_observer("AL", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "AH") == 0) {
        old_val = (double)((s_active_hw_context.r[0] >> 8) & 0xFF);
        s_active_hw_context.r[0] = (s_active_hw_context.r[0] & ~0xFF00ULL) | ((uint64_t)u8 << 8);
        s_active_hw_context.dirty_mask |= (1ULL << 0);
        s_active_hw_context.epoch++;
        notify_reg_observer("AH", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "BL") == 0) {
        old_val = (double)(s_active_hw_context.r[1] & 0xFF);
        s_active_hw_context.r[1] = (s_active_hw_context.r[1] & ~0xFFULL) | u8;
        s_active_hw_context.dirty_mask |= (1ULL << 1);
        s_active_hw_context.epoch++;
        notify_reg_observer("BL", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "BH") == 0) {
        old_val = (double)((s_active_hw_context.r[1] >> 8) & 0xFF);
        s_active_hw_context.r[1] = (s_active_hw_context.r[1] & ~0xFF00ULL) | ((uint64_t)u8 << 8);
        s_active_hw_context.dirty_mask |= (1ULL << 1);
        s_active_hw_context.epoch++;
        notify_reg_observer("BH", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "CL") == 0) {
        old_val = (double)(s_active_hw_context.r[2] & 0xFF);
        s_active_hw_context.r[2] = (s_active_hw_context.r[2] & ~0xFFULL) | u8;
        s_active_hw_context.dirty_mask |= (1ULL << 2);
        s_active_hw_context.epoch++;
        notify_reg_observer("CL", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "CH") == 0) {
        old_val = (double)((s_active_hw_context.r[2] >> 8) & 0xFF);
        s_active_hw_context.r[2] = (s_active_hw_context.r[2] & ~0xFF00ULL) | ((uint64_t)u8 << 8);
        s_active_hw_context.dirty_mask |= (1ULL << 2);
        s_active_hw_context.epoch++;
        notify_reg_observer("CH", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "DL") == 0) {
        old_val = (double)(s_active_hw_context.r[3] & 0xFF);
        s_active_hw_context.r[3] = (s_active_hw_context.r[3] & ~0xFFULL) | u8;
        s_active_hw_context.dirty_mask |= (1ULL << 3);
        s_active_hw_context.epoch++;
        notify_reg_observer("DL", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "DH") == 0) {
        old_val = (double)((s_active_hw_context.r[3] >> 8) & 0xFF);
        s_active_hw_context.r[3] = (s_active_hw_context.r[3] & ~0xFF00ULL) | ((uint64_t)u8 << 8);
        s_active_hw_context.dirty_mask |= (1ULL << 3);
        s_active_hw_context.epoch++;
        notify_reg_observer("DH", old_val, val);
        return true;
    }

    if (runtime_strcasecmp(r, "SI") == 0) {
        old_val = (double)(s_active_hw_context.r[4] & 0xFFFF);
        s_active_hw_context.r[4] = (s_active_hw_context.r[4] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 4);
        s_active_hw_context.epoch++;
        notify_reg_observer("SI", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "DI") == 0) {
        old_val = (double)(s_active_hw_context.r[5] & 0xFFFF);
        s_active_hw_context.r[5] = (s_active_hw_context.r[5] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 5);
        s_active_hw_context.epoch++;
        notify_reg_observer("DI", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "BP") == 0) {
        old_val = (double)(s_active_hw_context.r[6] & 0xFFFF);
        s_active_hw_context.r[6] = (s_active_hw_context.r[6] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 6);
        s_active_hw_context.epoch++;
        notify_reg_observer("BP", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "CS") == 0) {
        old_val = (double)(s_active_hw_context.r[9] & 0xFFFF);
        s_active_hw_context.r[9] = (s_active_hw_context.r[9] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 9);
        s_active_hw_context.epoch++;
        notify_reg_observer("CS", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "DS") == 0) {
        old_val = (double)(s_active_hw_context.r[10] & 0xFFFF);
        s_active_hw_context.r[10] = (s_active_hw_context.r[10] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 10);
        s_active_hw_context.epoch++;
        notify_reg_observer("DS", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "ES") == 0) {
        old_val = (double)(s_active_hw_context.r[11] & 0xFFFF);
        s_active_hw_context.r[11] = (s_active_hw_context.r[11] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 11);
        s_active_hw_context.epoch++;
        notify_reg_observer("ES", old_val, val);
        return true;
    }
    if (runtime_strcasecmp(r, "SS") == 0) {
        old_val = (double)(s_active_hw_context.r[12] & 0xFFFF);
        s_active_hw_context.r[12] = (s_active_hw_context.r[12] & ~0xFFFFULL) | u16;
        s_active_hw_context.dirty_mask |= (1ULL << 12);
        s_active_hw_context.epoch++;
        notify_reg_observer("SS", old_val, val);
        return true;
    }

    // Flag bits setter
    if (runtime_strcasecmp(r, "CF") == 0) { reg_hw_set_flag(0x0001, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "PF") == 0) { reg_hw_set_flag(0x0004, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "AF") == 0) { reg_hw_set_flag(0x0010, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "ZF") == 0) { reg_hw_set_flag(0x0040, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "SF") == 0) { reg_hw_set_flag(0x0080, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "TF") == 0) { reg_hw_set_flag(0x0100, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "IF") == 0) { reg_hw_set_flag(0x0200, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "DF") == 0) { reg_hw_set_flag(0x0400, val != 0.0); return true; }
    if (runtime_strcasecmp(r, "OF") == 0) { reg_hw_set_flag(0x0800, val != 0.0); return true; }

    (void)found;
    return false;
}

bool reg_hw_get_by_index(int index, const char **out_name, uint16_t *out_val) {
    if (index < 0 || index >= 14) return false;
    if (out_name) *out_name = s_reg_index_names[index];
    if (out_val) {
        bool found = false;
        *out_val = (uint16_t)reg_hw_get(s_reg_index_names[index], &found);
    }
    return true;
}

bool reg_hw_set_by_index(int index, uint16_t val) {
    if (index < 0 || index >= 14) return false;
    return reg_hw_set(s_reg_index_names[index], (double)val);
}

bool reg_hw_get_by_index64(int index, uint64_t *out_val) {
    if (index < 0 || index >= 32) return false;
    if (out_val) *out_val = s_active_hw_context.r[index];
    return true;
}

bool reg_hw_set_by_index64(int index, uint64_t val) {
    if (index < 0 || index >= 32) return false;
    char buf[16];
    runtime_snprintf(buf, sizeof(buf), "R%d", index);
    return reg_hw_set(buf, (double)val);
}

uint16_t reg_hw_get_width(const char *reg_name, int width, bool *found) {
    double v = reg_hw_get(reg_name, found);
    if (!found || !(*found)) return 0;
    if (width == 8) return (uint16_t)((uint32_t)v & 0xFF);
    return (uint16_t)((uint32_t)v & 0xFFFF);
}

bool reg_hw_get_flag(int bit_mask) {
    return (s_active_hw_context.status & bit_mask) != 0;
}

void reg_hw_set_flag(int bit_mask, bool val) {
    double old_val = (double)s_active_hw_context.status;
    if (val) {
        s_active_hw_context.status |= (uint64_t)bit_mask;
    } else {
        s_active_hw_context.status &= ~(uint64_t)bit_mask;
    }
    s_active_hw_context.epoch++;
    notify_reg_observer("STATUS", old_val, (double)s_active_hw_context.status);
}

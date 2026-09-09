// FILENAME: reg_hw.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, libhardware, BASIC++ runtime
// NEEDS: stdint, stdbool
// Implements the unified 8086/88 CPU and IBM PC BIOS hardware register subsystem (REG.*).

#ifndef ENGINE_REG_HW_H
#define ENGINE_REG_HW_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Unified universal hardware register context (64-bit generic bank + architecture slots)
typedef struct {
    uint64_t r[32];        // Generic 64-bit registers (REG.R0..REG.R31)
    uint64_t acc;          // Hardware accumulator (REG.ACC / REG.A)
    uint64_t status;       // Universal status / flags register (REG.STATUS / REG.FLAGS)
    uint64_t pc;           // Program counter / Instruction pointer (REG.PC / REG.IP)
    uint64_t sp;           // Stack pointer (REG.SP)
    uint16_t arch_slot;    // Active architecture slot (0x0001 x86, 0x0010 IoT HAL, 0x0002..0x000F open)
    uint64_t dirty_mask;   // 64-bit dirty bitmask tracking register mutations
    uint64_t epoch;        // Monotonic mutation generation counter
    bool     halted;       // CPU halted state flag
} RegHwContext;

// Architecture slot IDs
#define ARCH_SLOT_X86       0x0001
#define ARCH_SLOT_IOT_HAL   0x0010

// HAL & MMIO callback definitions
typedef uint64_t (*RegHwReadFn)(int reg_idx, void *user_data);
typedef void (*RegHwWriteFn)(int reg_idx, uint64_t val, void *user_data);

// Real-time observer callback definition
typedef void (*RegObserverFn)(const char *domain, const char *reg_name, double old_val, double new_val, void *user_data);

// Reset hardware registers to initial BIOS/system execution state
void reg_hw_reset(void);

// Retrieve pointer to active hardware register context
RegHwContext* reg_hw_get_context(void);

// Read hardware register value by name (e.g. "REG.R0".."REG.R31", "REG.AX", "AX", "AL", "AH", "ACC", "STATUS", "FLAGS", "CF")
double reg_hw_get(const char *reg_name, bool *found);

// Write hardware register value by name
bool reg_hw_set(const char *reg_name, double val);

// Indexed register access for REG[index] (0..31 generic, 0..13 legacy x86 mapping)
bool reg_hw_get_by_index(int index, const char **out_name, uint16_t *out_val);
bool reg_hw_set_by_index(int index, uint16_t val);

// 64-bit indexed register access for REG[index]
bool reg_hw_get_by_index64(int index, uint64_t *out_val);
bool reg_hw_set_by_index64(int index, uint64_t val);

// Width-modified register access (width = 8 for low byte, 16 for full word, 32 or 64)
uint16_t reg_hw_get_width(const char *reg_name, int width, bool *found);

// Flag helpers
bool reg_hw_get_flag(int bit_mask);
void reg_hw_set_flag(int bit_mask, bool val);

// Architecture slot management
void reg_hw_set_arch_slot(uint16_t slot_id);
uint16_t reg_hw_get_arch_slot(void);

// Hardware HAL & MMIO bindings
void reg_hw_bind_hal_hooks(RegHwReadFn read_fn, RegHwWriteFn write_fn, void *user_data);
void reg_hw_bind_mmio(int reg_idx, uintptr_t mmio_addr, uint8_t width);

// Real-time observer and telemetry helpers
void reg_register_observer(RegObserverFn fn, void *user_data);
uint64_t reg_hw_get_dirty_mask(void);
void reg_hw_clear_dirty_mask(void);
uint64_t reg_hw_get_epoch(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_REG_HW_H

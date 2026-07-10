/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mock_bios_core.h
 * Subsystem: Switchable IBM PC BIOS Emulator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Emulates IBM PC BIOS interrupts (INT 10h, 16h, 21h).
 *
 * 2. WHAT TO EXPECT:
 *    Interfaces with BDA/IVT mapping hardware calls.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Emulated ports, system timer tick rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    IVT/BDA hardware offset rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If system crashes, check BDA values configuration.
 * ===================================================================== */

#ifndef STANDALONE_MOCK_BIOS_CORE_H
#define STANDALONE_MOCK_BIOS_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct MockBiosRegs {
    uint32_t ax;
    uint32_t bx;
    uint32_t cx;
    uint32_t dx;
    uint32_t flags;
} MockBiosRegs;

typedef enum MockBiosModel {
    BIOS_MODEL_NONE = 0,
    BIOS_MODEL_MSDOS,
    BIOS_MODEL_PC,
    BIOS_MODEL_PCJR,
    BIOS_MODEL_XT,
    BIOS_MODEL_AT,
    BIOS_MODEL_ATARI,
    BIOS_MODEL_C64,
    BIOS_MODEL_APPLE2,
    BIOS_MODEL_TRS80,
    BIOS_MODEL_TANDY
} MockBiosModel;

typedef struct MockBiosContext {
    // Callbacks to read/write guest memory
    uint8_t (*read_mem)(void *user_data, uint32_t addr);
    void (*write_mem)(void *user_data, uint32_t addr, uint8_t val);

    // Callbacks to interact with host virtual peripherals
    void (*vdev_sleep)(void *user_data, int ms);
    int (*vdev_ioctl)(void *user_data, int cmd, void *arg);

    // Callbacks to read/write CPU registers
    void (*get_registers)(void *user_data, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags);
    void (*set_registers)(void *user_data, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags);

    void *user_data; // Client-specific context (e.g. RuntimeState* in BASIC++)
    MockBiosModel model;

    // Internal BIOS status states (e.g., keyboard buffer, CMOS state)
    int key_buffer;
    uint8_t cmos_idx;
    uint8_t cmos_regs[64];
    
    // PIT/PPI simulation registers
    uint8_t ppi_port_b;
    uint8_t pit_counter;
    uint8_t crtc_index;
    uint8_t crtc_regs[32];
    
    // DOS File Handles
    FILE *dos_handles[20];
} MockBiosContext;

// Initialize mock BIOS memory structures (IVT, BDA, signatures) in target memory.
void mock_bios_init_mem(MockBiosContext *ctx, uint8_t *mem_segment, size_t mem_size, MockBiosModel model);

// Emulate an INP instruction for a given port
uint8_t mock_bios_in(MockBiosContext *ctx, uint16_t port);

// Emulate an OUT instruction for a given port
void mock_bios_out(MockBiosContext *ctx, uint16_t port, uint8_t val);

// Emulate an INT interrupt instruction
void mock_bios_interrupt(MockBiosContext *ctx, uint8_t int_num);

#endif // STANDALONE_MOCK_BIOS_CORE_H

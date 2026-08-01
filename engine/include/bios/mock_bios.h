/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_mock_bios.h
 * @brief Switchable IBM PC BIOS & DOS Interrupt Emulator interfaces.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures, constants, and function prototypes for
 *   emulating conventional PC BIOS and DOS interrupt services.
 * - Why it exists: Enables low-level x86 interrupt simulation (INT 10h, 16h, 21h, etc.)
 *   for vintage PC, XT, AT, and PCjr targets in a fully portable library.
 * - Why it works this way: It operates on client-defined memory and register callbacks
 *   to remain completely decoupled from the compiler core and host OS.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Adding new emulated models, new port numbers.
 * - What cannot be changed: Memory-mapping offset layout (IVT/BDA offsets).
 * - What to expect: No heap allocations. Extremely fast execution loops.
 * - What to do if something breaks: Verify user_data payload and memory callback alignments.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: CPU register callbacks write back to target register representations.
 * - Portability concerns: None. C17 standard compliant and suitable for bare-metal kernels.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTERNAL HOOKS
 * - How future expansion can occur: Implement INT 14h serial and INT 25h/26h absolute sector calls.
 * - External hooks: Shared libraries (.DLL/.SO) can expose these standard entry points.
 */

#ifndef BPP_MOCK_BIOS_H
#define BPP_MOCK_BIOS_H

#include <stdint.h>
#include <stddef.h>

#define VDEV_IOCTL_INT10 9
#define VDEV_IOCTL_INT15 10
#define VDEV_IOCTL_INT16 11
#define VDEV_IOCTL_INT21 12
#define VDEV_IOCTL_VFS_RESOLVE 13
#define VDEV_IOCTL_DIR_MKDIR 14
#define VDEV_IOCTL_DIR_RMDIR 15
#define VDEV_IOCTL_DIR_CHDIR 16
#define VDEV_IOCTL_DIR_GETCWD 17
#define VDEV_IOCTL_MEM_ALLOC 18
#define VDEV_IOCTL_MEM_FREE 19
#define VDEV_IOCTL_MEM_RESIZE 20
#if defined(_WIN32) && defined(BPP_BIOS_DLL_EXPORT)
#define BPP_BIOS_API __declspec(dllexport)
#elif defined(_WIN32)
#define BPP_BIOS_API
#else
#define BPP_BIOS_API __attribute__((visibility("default")))
#endif

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
    BIOS_MODEL_VIC20,
    BIOS_MODEL_APPLE2,
    BIOS_MODEL_TRS80,
    BIOS_MODEL_TANDY
} MockBiosModel;

typedef struct MockBiosContext {
    /* Guest Memory Access Callbacks */
    uint8_t (*read_mem)(void *user_data, uint32_t addr);
    void (*write_mem)(void *user_data, uint32_t addr, uint8_t val);

    /* Host Peripheral Interface Callbacks */
    void (*vdev_sleep)(void *user_data, int ms);
    int (*vdev_ioctl)(void *user_data, int cmd, void *arg);

    /* CPU Register Access Callbacks */
    void (*get_registers)(void *user_data, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags);
    void (*set_registers)(void *user_data, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags);

    void *user_data; /* Client context payload */
    MockBiosModel model;

    /* Persistent emulation states */
    int key_buffer;
    uint8_t cmos_idx;
    uint8_t cmos_regs[64];
    uint8_t ppi_port_b;
    uint8_t pit_counter;
    uint8_t crtc_index;
    uint8_t crtc_regs[32];
    void *dos_handles[20];
} MockBiosContext;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize conventional memory maps (IVT, BDA, signatures) for the selected PC model.
 */
BPP_BIOS_API void mock_bios_init_mem(MockBiosContext *ctx, uint8_t *mem_segment, size_t mem_size, MockBiosModel model);

/**
 * @brief Emulate an INP instruction from a hardware port.
 */
BPP_BIOS_API uint8_t mock_bios_in(MockBiosContext *ctx, uint16_t port);

/**
 * @brief Emulate an OUT instruction to a hardware port.
 */
BPP_BIOS_API void mock_bios_out(MockBiosContext *ctx, uint16_t port, uint8_t val);

/**
 * @brief Emulate execution of an x86/DOS interrupt.
 */
BPP_BIOS_API void mock_bios_interrupt(MockBiosContext *ctx, uint8_t int_num);
BPP_BIOS_API const char *mock_bios_model_to_string(MockBiosModel model);
BPP_BIOS_API MockBiosModel mock_bios_model_from_string(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* BPP_MOCK_BIOS_H */

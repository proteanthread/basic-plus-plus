// FILENAME: boot.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (common_internal.h, embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (bpp_api.c, iot_main.c)
// NEEDS: libcore (hal.h, memory.h, memory.c, strings.h, strings.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides core logic and interface definitions for boot within BASIC++.
//
// ---- Includes ----

#ifndef CORE_BOOT_H
#define CORE_BOOT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "hal/hal.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "device/vdev.h"
#include "vm/vm.h"

// Aggregated context block populated during the 9-phase boot sequence
typedef struct {
    MemoryContext   *mem;
    StringContext   *str;
    VariableContext *var;
    VDevContext     *vdev;
    VMContext       *vm;
} BootContext;

// Boot Configuration block passed by minimal runner stubs
typedef struct {
    size_t prog_mem;
    size_t var_mem;
    size_t str_mem;
    size_t scratch_mem;
    bool   is_repl;
    bool   sdl_gui;
    bool   sdl_ondemand;
    bool   is_freestanding;
    const HalContext *custom_hal;
} BootConfig;

// @brief Runs the deterministic 9-phase boot sequence.
// @return Populated BootContext on success, or NULL on phase failure.
BootContext *boot_execute(const BootConfig *config);

// @brief Shuts down all boot subsystems in precise reverse order of initialization.
void boot_shutdown(BootContext *ctx);
void boot_shutdown_ex(BootContext *ctx, bool force_exit);

// @brief High-level helper: boots system with default dynamic memory allocations.
VMContext *boot_system(size_t heap_size);

// @brief High-level helper: shuts down system given a VMContext pointer.
void boot_shutdown_vm(VMContext *vm);

#endif // CORE_BOOT_H

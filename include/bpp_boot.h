/**
 * @file bpp_boot.h
 * @brief Deterministic 9-Phase Boot Controller API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the BootContext aggregation block and functions to run the 9-phase boot sequence
 *   and shut down subsystems in reverse order.
 * - Why it exists: Establishes a predictable, deterministic startup sequence across all host systems.
 * - Why it works this way: It sequentially builds up the memory context, string manager, variable registry,
 *   virtual device system, and VM core. If any phase fails, it rewinds and shuts down successfully initialized
 *   subsystems cleanly.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Phase checks, boot status logging targets, boot configuration overrides.
 * - What cannot be changed: Obligation to run all phases sequentially.
 * - What to expect: Calling boot_execute returns a fully populated BootContext containing initialized managers.
 * - What to do if something breaks: If booting halts, check the return code of the failed boot phase
 *   and verify memory limit constraints.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Subsystems are non-dependent on OS features at phase boot times.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add phase logs or pre-boot command line overrides.
 * - How to write external extensions: External plugins compile hooks and register during boot Phase 5.
 */

#ifndef BPP_BOOT_H
#define BPP_BOOT_H

#include "bpp_memory.h"
#include "bpp_strings.h"
#include "bpp_variables.h"
#include "bpp_vdev.h"
#include "bpp_vm.h"

/* Aggregated context block populated during the 9-phase boot sequence */
typedef struct {
    MemoryContext   *mem;
    StringContext   *str;
    VariableContext *var;
    VDevContext     *vdev;
    VMContext       *vm;
} BootContext;

/* Boot Configuration block passed by minimal runner stubs */
typedef struct {
    size_t prog_mem;
    size_t var_mem;
    size_t str_mem;
    size_t scratch_mem;
    bool   is_repl;
    bool   sdl_gui;
    bool   sdl_ondemand;
    const struct BppDialect *dialect_config; /* Optional static dialect config */
} BootConfig;

/**
 * @brief Runs the deterministic 9-phase boot sequence.
 * @return Populated BootContext on success, or NULL on phase failure.
 */
BootContext *boot_execute(const BootConfig *config);

/**
 * @brief Shuts down all boot subsystems in precise reverse order of initialization.
 */
void boot_shutdown(BootContext *ctx);

#endif /* BPP_BOOT_H */

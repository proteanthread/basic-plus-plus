/*
 * ---
 * BASIC++ Interpreter - boot.c
 * ---
 *
 * Boot Controller Implementation.
 *
 * ARCHITECTURE:
 * The boot controller orchestrates all 8 initialization phases
 * in strict order. Each phase is an internal function that returns
 * a BootStatus. The controller propagates failures and tracks
 * diagnostics in the static BootState.
 *
 * PHASE ORDER (immutable):
 *   0: Host Entry       - platform_init(), no OS assumptions
 *   1: Core Memory      - mem_init(), error system
 *   2: VM Core          - vm_init(), scope, keywords, override
 *   3: Virtual Devices  - vdev, network, fileio, graphics
 *   4: Standard Library - funcreg, error_registry, builtins
 *   5: Module System    - security, module table, activation
 *   6: Dialect & Config - dialect selection, overrides, strict
 *   7: Ready State      - interpreter ready for input
 *
 * SHUTDOWN:
 * boot_shutdown() reverses the init order, tearing down each
 * subsystem symmetrically.
 *
 * DIAGNOSTIC OUTPUT:
 * boot_log() writes to stderr at the configured verbosity level.
 * Phase transitions are logged at BOOT_LOG level.
 * Module/extension details are logged at BOOT_DEBUG level.
 * Continuous telemetry is logged at BOOT_VERBOSE level.
 *
 * ---
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "boot.h"
#include "memory.h"
#include "errors.h"
#include "platform.h"
#include "lexer.h"
#include "vm.h"
#include "scope.h"
#include "keyword_props.h"
#include "override.h"
#include "vdev.h"
#include "io/vdev_net.h"
#include "fileio.h"
#include "gfxbuf.h"
#include "funcreg.h"
#include "error_registry.h"
#include "security.h"
#include "module.h"
#include "mod_stdlib.h"
#ifndef BPP_FREEDOS
#include "mod_usb.h"
#include "mod_fujinet.h"
#include "mod_upnp.h"
#endif
#include "dialect.h"
#include "device_alias.h"
#include "txn.h"
#include "runtime.h"
#include "builtins.h"

/* --- Static Boot State ---
 */
static BootState boot_state;

/* --- Phase Names ---
 */
static const char *phase_names[PHASE_COUNT] = {
 "Host Entry",
 "Core Memory",
 "VM Core",
 "Virtual Devices",
 "Standard Library",
 "Module System",
 "Dialect & Config",
 "Ready State"
};

/* --- Status Names ---
 */
static const char *status_names[] = {
 "OK",
 "DEGRADED",
 "CRITICAL"
};

/* ================================================================
 * boot_log - Diagnostic output to stderr
 * ================================================================
 */
void boot_log(BootVerbosity level, const char *fmt, ...)
{
 va_list args;

 if (level > boot_state.verbosity) return;

 va_start(args, fmt);
 fprintf(stderr, "[BOOT] ");
 vfprintf(stderr, fmt, args);
 fprintf(stderr, "\n");
 va_end(args);
}

/* ================================================================
 * Phase 0: Host Entry Point
 * ================================================================
 * Platform detection. No OS-specific logic affects semantics.
 * Only provides: raw execution context, CLI args, file primitives.
 */
static BootStatus boot_phase0_host(const BootConfig *config)
{
 (void)config;

 boot_log(BOOT_LOG, "Phase 0: Host Entry");

 /* Detect host platform - must be first */
 platform_init();

 boot_log(BOOT_LOG, "  Platform: %s (%s)",
  platform_name(), platform_short_name());
 boot_log(BOOT_LOG, "  Word size: %d-bit",
  platform_word_size());

 return BOOT_OK;
}

/* ================================================================
 * Phase 1: Core Memory System Initialization
 * ================================================================
 * Memory pools, allocation tables, error system.
 * CRITICAL - if this fails, nothing else can work.
 */
static BootStatus boot_phase1_memory(MemorySystem *memory)
{
 boot_log(BOOT_LOG, "Phase 1: Core Memory");

 /* Initialize error system first */
 error_clear();

 /* Initialize memory pools */
 if (mem_init(memory) != 0) {
  boot_log(BOOT_LOG,
   "  CRITICAL: Cannot allocate memory pools");
  return BOOT_CRITICAL;
 }

 boot_log(BOOT_LOG,
  "  Variable pool: %ld bytes", memory->variable.size);
 boot_log(BOOT_LOG,
  "  Scratch pool: %ld bytes", memory->scratch.size);
 boot_log(BOOT_LOG,
  "  Program store: %d line capacity",
  memory->program.capacity);

 return BOOT_OK;
}

/* ================================================================
 * Phase 2: VM Core Initialization
 * ================================================================
 * Dispatch table, execution engine, control-flow systems.
 * Must be complete before any parsing or execution.
 */
static BootStatus boot_phase2_vm_core(void)
{
 boot_log(BOOT_LOG, "Phase 2: VM Core");

 /* Build instruction dispatch table */
 vm_init();
 boot_log(BOOT_DEBUG, "  Dispatch table initialized");

 /* Initialize keyword scope system */
 scope_init();
 boot_log(BOOT_DEBUG, "  Scope system initialized");

 /* Initialize keyword property system */
 keyword_props_init();
 boot_log(BOOT_DEBUG, "  Keyword properties initialized");

 /* Initialize keyword override system */
 override_init();
 boot_log(BOOT_DEBUG, "  Override system initialized");

 return BOOT_OK;
}

/* ================================================================
 * Phase 3: Virtual Device System Initialization
 * ================================================================
 * Register CON:, ERR:, FILE:, NET:, GFX: devices.
 * CON: and ERR: are critical. Others are non-critical.
 */
static BootStatus boot_phase3_devices(void)
{
 BootStatus status = BOOT_OK;

 boot_log(BOOT_LOG, "Phase 3: Virtual Devices");

 /* Network socket layer (before vdev for NET: device) */
 vdev_net_init();
 boot_log(BOOT_DEBUG, "  Network layer initialized");

 /* Core device table (CON:, ERR:, FILE:) */
 vdev_init();
 boot_log(BOOT_DEBUG, "  Device table initialized");

 /* File I/O channels */
 fileio_channels_init();
 boot_log(BOOT_DEBUG, "  File channels initialized");

 /* Device alias table (for cross-dialect device mapping) */
 device_alias_init();
 boot_log(BOOT_DEBUG, "  Device alias table initialized");

 /* Transaction journal (ATOMIC/TXN support) */
 txn_init();
 boot_log(BOOT_DEBUG, "  Transaction journal initialized");

 /* Graphics framebuffer (non-critical) */
 gfxbuf_init();
 boot_log(BOOT_DEBUG, "  Graphics framebuffer initialized");

 return status;
}

/* ================================================================
 * Phase 4: Standard Library Initialization
 * ================================================================
 * Register built-in functions, error messages.
 * After this: ABS, SIN, LEN, LEFT$, etc. are available.
 */
static BootStatus boot_phase4_stdlib(void)
{
 boot_log(BOOT_LOG, "Phase 4: Standard Library");

 /* Initialize function registry */
 funcreg_init();
 boot_log(BOOT_DEBUG, "  Function registry initialized");

 /* Initialize error message registry */
 error_registry_init();
 boot_log(BOOT_DEBUG, "  Error registry initialized");

 boot_state.functions_registered = funcreg_count();
 boot_log(BOOT_LOG, "  Functions registered: %d",
  boot_state.functions_registered);

 return BOOT_OK;
}

/* ================================================================
 * Phase 5: Module System Initialization
 * ================================================================
 * Security, module table, built-in modules, CLI modules.
 * Module failures are non-critical.
 */
static BootStatus boot_phase5_modules(
 const BootConfig *config,
 struct RuntimeState *runtime)
{
 BootStatus status = BOOT_OK;

 boot_log(BOOT_LOG, "Phase 5: Module System");

 /* Initialize security system BEFORE modules load */
 security_init(config->security);
 boot_log(BOOT_LOG, "  Security level: %s",
  security_level_name(config->security));

 /* Initialize module table */
 module_system_init();

 /* Register built-in modules */
 mod_stdlib_register();
 boot_log(BOOT_DEBUG, "  Registered: STDLIB");

#ifndef BPP_FREEDOS
 mod_usb_register();
 boot_log(BOOT_DEBUG, "  Registered: USB");

 mod_fujinet_register();
 boot_log(BOOT_DEBUG, "  Registered: FUJINET");

 mod_upnp_register();
 boot_log(BOOT_DEBUG, "  Registered: UPNP");
#endif

 /* Activate STDLIB (always) */
 module_activate("STDLIB", runtime);

 /* Load external modules from CLI */
 if (config->cli_lib != NULL) {
  boot_log(BOOT_DEBUG,
   "  Loading CLI library: %s", config->cli_lib);
  if (module_load_dynamic(config->cli_lib) != 0) {
   boot_log(BOOT_LOG,
    "  WARNING: Failed to load: %s",
    config->cli_lib);
   boot_state.modules_failed++;
   boot_state.warnings++;
   status = BOOT_DEGRADED;
  }
 }
 if (config->cli_mod != NULL) {
  boot_log(BOOT_DEBUG,
   "  Loading CLI module: %s", config->cli_mod);
  if (module_load_dynamic(config->cli_mod) != 0) {
   boot_log(BOOT_LOG,
    "  WARNING: Failed to load: %s",
    config->cli_mod);
   boot_state.modules_failed++;
   boot_state.warnings++;
   status = BOOT_DEGRADED;
  }
 }
 if (config->cli_func != NULL) {
  boot_log(BOOT_DEBUG,
   "  Loading CLI function: %s",
   config->cli_func);
  if (module_load_dynamic(config->cli_func) != 0) {
   boot_log(BOOT_LOG,
    "  WARNING: Failed to load: %s",
    config->cli_func);
   boot_state.modules_failed++;
   boot_state.warnings++;
   status = BOOT_DEGRADED;
  }
 }

 boot_state.modules_registered = module_count();
 boot_log(BOOT_LOG, "  Modules: %d registered, %d failed",
  boot_state.modules_registered,
  boot_state.modules_failed);

 return status;
}

/* ================================================================
 * Phase 6: Dialect Selection and Configuration
 * ================================================================
 * Select dialect, apply overrides, set strict mode.
 * Dialect selection occurs AFTER all functions are registered
 * so overrides can target the complete function table.
 */
static BootStatus boot_phase6_dialect(const BootConfig *config)
{
 boot_log(BOOT_LOG, "Phase 6: Dialect & Config");

 /* Initialize and select dialect */
 dialect_init(config->dialect);
 boot_log(BOOT_LOG, "  Dialect: %s",
  dialect_get_short_name());

 /* Apply dialect-specific function overrides */
 dialect_apply();
 boot_log(BOOT_DEBUG, "  Dialect overrides applied");

 /* Apply strict mode if configured */
 if (config->strict) {
  dialect_set_strict(1);
  boot_log(BOOT_LOG, "  Strict mode: ON");
 }

 /* Report final function count (may have changed) */
 boot_state.functions_registered = funcreg_count();
 boot_log(BOOT_DEBUG,
  "  Functions after dialect: %d",
  boot_state.functions_registered);

 return BOOT_OK;
}

/* ================================================================
 * Phase 7: Interpreter Ready State
 * ================================================================
 * Initialize runtime state, set deterministic RNG seed,
 * enter READY state.
 */
static BootStatus boot_phase7_ready(
 MemorySystem *memory,
 struct RuntimeState *runtime)
{
 boot_log(BOOT_LOG, "Phase 7: Ready State");

 /* Initialize runtime state (variables, stack, etc.) */
 runtime_init(runtime, &memory->program, memory);
 boot_log(BOOT_DEBUG, "  Runtime state initialized");

 /*
  * RNG seed is set to 1 by runtime_init() — this is
  * deterministic by design. Users override with RANDOMIZE.
  * This guarantees: identical configuration produces
  * identical runtime state on all platforms.
  */
 boot_log(BOOT_DEBUG, "  RNG seed: deterministic (1)");

 /* Print boot summary if boot-log mode */
 if (boot_state.verbosity >= BOOT_LOG) {
  boot_print_summary();
 }

 boot_log(BOOT_LOG, "Boot complete.");
 return BOOT_OK;
}

/* ================================================================
 * boot_execute - Run the full boot sequence
 * ================================================================
 */
BootStatus boot_execute(const BootConfig *config,
                        MemorySystem *memory,
                        struct RuntimeState *runtime)
{
 int i;
 BootStatus phase_result;

 /* Initialize boot state */
 memset(&boot_state, 0, sizeof(boot_state));
 boot_state.verbosity = config->verbosity;
 boot_state.overall_status = BOOT_OK;

 /* Phase 0: Host Entry */
 boot_state.current_phase = PHASE_HOST;
 phase_result = boot_phase0_host(config);
 boot_state.phase_status[PHASE_HOST] = phase_result;
 if (phase_result == BOOT_CRITICAL) {
  boot_state.overall_status = BOOT_CRITICAL;
  return BOOT_CRITICAL;
 }

 /* Phase 1: Core Memory */
 boot_state.current_phase = PHASE_MEMORY;
 phase_result = boot_phase1_memory(memory);
 boot_state.phase_status[PHASE_MEMORY] = phase_result;
 if (phase_result == BOOT_CRITICAL) {
  boot_state.overall_status = BOOT_CRITICAL;
  printf("SORRY. Cannot allocate memory.\n");
  return BOOT_CRITICAL;
 }

 /* Phase 2: VM Core */
 boot_state.current_phase = PHASE_VM_CORE;
 phase_result = boot_phase2_vm_core();
 boot_state.phase_status[PHASE_VM_CORE] = phase_result;
 if (phase_result == BOOT_CRITICAL) {
  boot_state.overall_status = BOOT_CRITICAL;
  return BOOT_CRITICAL;
 }

 /* Phase 3: Virtual Devices */
 boot_state.current_phase = PHASE_DEVICES;
 phase_result = boot_phase3_devices();
 boot_state.phase_status[PHASE_DEVICES] = phase_result;
 if (phase_result == BOOT_DEGRADED) {
  boot_state.overall_status = BOOT_DEGRADED;
 }
 if (phase_result == BOOT_CRITICAL) {
  boot_state.overall_status = BOOT_CRITICAL;
  return BOOT_CRITICAL;
 }

 /* Phase 4: Standard Library */
 boot_state.current_phase = PHASE_STDLIB;
 phase_result = boot_phase4_stdlib();
 boot_state.phase_status[PHASE_STDLIB] = phase_result;
 if (phase_result == BOOT_CRITICAL) {
  boot_state.overall_status = BOOT_CRITICAL;
  return BOOT_CRITICAL;
 }

 /* Phase 5: Module System */
 boot_state.current_phase = PHASE_MODULES;
 phase_result = boot_phase5_modules(config, runtime);
 boot_state.phase_status[PHASE_MODULES] = phase_result;
 if (phase_result == BOOT_DEGRADED) {
  boot_state.overall_status = BOOT_DEGRADED;
 }

 /* Phase 6: Dialect & Config */
 boot_state.current_phase = PHASE_DIALECT;
 phase_result = boot_phase6_dialect(config);
 boot_state.phase_status[PHASE_DIALECT] = phase_result;
 if (phase_result == BOOT_DEGRADED) {
  boot_state.overall_status = BOOT_DEGRADED;
 }

 /* Phase 7: Ready State */
 boot_state.current_phase = PHASE_READY;
 phase_result = boot_phase7_ready(memory, runtime);
 boot_state.phase_status[PHASE_READY] = phase_result;

 /* Promote overall status */
 for (i = 0; i < PHASE_COUNT; i++) {
  if (boot_state.phase_status[i] > boot_state.overall_status) {
   boot_state.overall_status = boot_state.phase_status[i];
  }
 }

 return boot_state.overall_status;
}

/* ================================================================
 * boot_shutdown - Symmetric shutdown (reverse phase order)
 * ================================================================
 */
void boot_shutdown(MemorySystem *memory)
{
 boot_log(BOOT_LOG, "Shutdown: reverse phase order");

 /* Phase 7: Leave ready state (no-op) */
 /* Phase 6: Dialect cleanup (no-op) */
 /* Phase 5: Module cleanup - deactivate all */
 boot_log(BOOT_DEBUG, "  Deactivating modules...");
 /* Modules are cleaned up when their cleanup callbacks fire.
  * Currently module_deactivate handles this per-module.
  * A future module_system_shutdown() could iterate all. */

 /* Phase 4: Stdlib cleanup (no-op - static registry) */

 /* Phase 3: Device cleanup */
 boot_log(BOOT_DEBUG, "  Closing network devices...");
 vdev_net_cleanup();

 /* Phase 2: VM cleanup (no-op - static dispatch) */
 /* Phase 1: Memory cleanup */
 boot_log(BOOT_DEBUG, "  Releasing memory pools...");
 mem_shutdown(memory);

 /* Phase 0: Host cleanup (no-op - return from main) */
 boot_log(BOOT_LOG, "Shutdown complete.");
}

/* ================================================================
 * Diagnostic functions
 * ================================================================
 */

const BootState *boot_get_state(void)
{
 return &boot_state;
}

const char *boot_phase_name(BootPhase phase)
{
 if (phase >= 0 && phase < PHASE_COUNT) {
  return phase_names[phase];
 }
 return "Unknown";
}

const char *boot_status_name(BootStatus status)
{
 if (status >= BOOT_OK && status <= BOOT_CRITICAL) {
  return status_names[status];
 }
 return "Unknown";
}

void boot_print_summary(void)
{
 int i;

 fprintf(stderr,
  "\n=== BASIC++ Boot Summary ===\n");

 for (i = 0; i < PHASE_COUNT; i++) {
  fprintf(stderr, "  Phase %d: %-18s [%s]\n",
   i, phase_names[i],
   boot_status_name(boot_state.phase_status[i]));
 }

 fprintf(stderr,
  "  ---\n"
  "  Functions: %d\n"
  "  Devices:   %d\n"
  "  Modules:   %d registered, %d failed\n"
  "  Warnings:  %d\n"
  "  Overall:   %s\n"
  "============================\n\n",
  boot_state.functions_registered,
  boot_state.devices_registered,
  boot_state.modules_registered,
  boot_state.modules_failed,
  boot_state.warnings,
  boot_status_name(boot_state.overall_status));
}

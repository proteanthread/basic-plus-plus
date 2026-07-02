/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: boot.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - boot.h
 // ---
 //
 // Boot Controller Interface.
 //
 // PURPOSE:
 // Formalizes the BASIC++ initialization sequence into a
 // deterministic, ordered, modular boot system. The interpreter
 // transitions through 8 defined phases (0-7) before any user
 // program execution begins.
 //
 // BOOT PHASES:
 //   Phase 0: Host Entry       - platform detection, CLI args, config
 //   Phase 1: Core Memory      - memory pools, allocation tables
 //   Phase 2: VM Core          - dispatch table, execution engine
 //   Phase 3: Virtual Devices  - CON:, ERR:, FILE:, NET:, GFX:
 //   Phase 4: Standard Library - function registry, builtins
 //   Phase 5: Module System    - security, modules, extensions
 //   Phase 6: Dialect & Config - dialect selection, overrides, strict
 //   Phase 7: Ready State      - interpreter enters READY
 //
 // DETERMINISM GUARANTEE:
 //   Boot sequence executes in identical order on all platforms.
 //   Identical configuration produces identical runtime state.
 //   The RNG seed is deterministic (1) unless overridden by
 //   RANDOMIZE at runtime.
 //
 // FAILURE HANDLING:
 //   BOOT_OK       - phase completed successfully
 //   BOOT_DEGRADED - phase completed with reduced functionality
 //   BOOT_CRITICAL - phase failed, boot must halt
 //
 //   Non-critical failures (devices, modules) log a warning and
 //   continue. Critical failures (memory, CON:) halt immediately.
 //   At no point shall boot failure corrupt memory state, execute
 //   partial user programs, or bypass safety mechanisms.
 //
 // DIAGNOSTIC MODES:
 //   --boot-log  Print phase-by-phase boot diagnostics, stop at READY
 //   --debug     Verbose output for modules, extensions, external code
 //   --verbose   Continuous detailed logging to stderr throughout run
 //
 // ---

#ifndef BASICPP_BOOT_H
#define BASICPP_BOOT_H

#include "config.h"
#include "memory.h"
#include "security.h"
#include "dialect.h"

// Forward declaration
struct RuntimeState;

// --- Boot Phases ---
typedef enum BootPhase {
 PHASE_HOST      = 0, // Host entry point
 PHASE_MEMORY    = 1, // Core memory system
 PHASE_VM_CORE   = 2, // Virtual machine core
 PHASE_DEVICES   = 3, // Virtual device system
 PHASE_STDLIB    = 4, // Standard library
 PHASE_MODULES   = 5, // Module system
 PHASE_DIALECT   = 6, // Dialect & configuration
 PHASE_READY     = 7, // Interpreter ready
 PHASE_COUNT     = 8 // sentinel
} BootPhase;

// --- Boot Status Codes ---
typedef enum BootStatus {
 BOOT_OK       = 0, // Phase completed successfully
 BOOT_DEGRADED = 1, // Completed with reduced functionality
 BOOT_CRITICAL = 2 // Failed - boot must halt
} BootStatus;

// --- Diagnostic Verbosity Levels ---
typedef enum BootVerbosity {
 BOOT_SILENT  = 0, // No diagnostic output
 BOOT_LOG     = 1, // --boot-log: phase summaries only
 BOOT_DEBUG   = 2, // --debug: module/extension details
 BOOT_VERBOSE = 3 // --verbose: continuous detailed log
} BootVerbosity;

// --- Boot Configuration ---
 // Effective settings resolved from CLI + config file.
 // Passed into boot_execute() to control behavior.
typedef struct BootConfig {
 // Dialect & security
 DialectId  dialect;
 SecLevel   security;
 int        strict;
 int        quiet;

 // Diagnostic modes
 BootVerbosity verbosity;

 // File execution modes
 const char *run_file;
 const char *load_file;
 const char *command;

 // External module paths from CLI
 const char *cli_lib;
 const char *cli_mod;
 const char *cli_func;
} BootConfig;

// --- Boot State ---
 // Tracks the current phase, cumulative status, and per-phase
 // diagnostic information. Readable after boot for INFO display.
typedef struct BootState {
 BootPhase    current_phase;
 BootStatus   phase_status[PHASE_COUNT];
 BootStatus   overall_status;
 BootVerbosity verbosity;

 // Diagnostic counters
 int functions_registered;
 int devices_registered;
 int modules_registered;
 int modules_active;
 int modules_failed;
 int warnings;
} BootState;

// --- Boot API ---

 // boot_execute - Run the full boot sequence.
 //
 // Executes all 8 phases in strict order. Returns the overall
 // boot status. On BOOT_CRITICAL, the interpreter should not
 // enter the REPL.
 //
 // Parameters:
 //   config   - resolved boot configuration
 //   memory   - pointer to MemorySystem (allocated by caller)
 //   runtime  - pointer to RuntimeState (allocated by caller)
 //
 // Returns BOOT_OK, BOOT_DEGRADED, or BOOT_CRITICAL.
BootStatus boot_execute(const BootConfig *config,
                        MemorySystem *memory,
                        struct RuntimeState *runtime);

void boot_downgrade_status(BootStatus status);

 // boot_shutdown - Symmetric shutdown in reverse phase order.
 //
 // Tears down all subsystems in reverse order of initialization.
 // Must be called before process exit.
void boot_shutdown(MemorySystem *memory);

 // boot_get_state - Get the current boot state.
 //
 // Returns a pointer to the static BootState for diagnostic
 // queries (e.g., INFO command showing boot status).
const BootState *boot_get_state(void);

 // boot_phase_name - Get human-readable name for a phase.
const char *boot_phase_name(BootPhase phase);

 // boot_status_name - Get human-readable name for a status.
const char *boot_status_name(BootStatus status);

 // boot_print_summary - Print boot diagnostic summary.
 //
 // Shows each phase, its status, and any warnings.
 // Called automatically in --boot-log mode, or on demand.
void boot_print_summary(void);

 // boot_log - Write a diagnostic message (if verbosity permits).
 //
 // Messages at or below the current verbosity level are printed
 // to stderr. Higher-level messages are suppressed.
 //
 // Parameters:
 //   level - minimum verbosity required to show this message
 //   fmt   - printf-style format string
 //   ...   - format arguments
void boot_log(BootVerbosity level, const char *fmt, ...);
void boot_log_write(struct RuntimeState *rt, const char *fmt, ...);
void boot_init_runtime(struct RuntimeState *runtime);

#endif // BASICPP_BOOT_H

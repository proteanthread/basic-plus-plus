/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: boot.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - boot.c
// ---
//
// Boot Controller Implementation -- 8-phase initialization sequence.
//
// PURPOSE:
//   Orchestrates all interpreter initialization in strict phase
//   order. Each subsystem is brought up in dependency order,
//   with diagnostic logging and failure propagation. The boot
//   controller also provides symmetric shutdown (reverse order).
//
// HOW IT WORKS:
//   The boot controller runs 8 phases in order:
//
//     Phase 0: Host Entry       -> platform_init()
//     Phase 1: Core Memory      -> mem_init(), error system
//     Phase 2: VM Core          -> vm_init(), scope, keywords
//     Phase 3: Virtual Devices  -> vdev, network, fileio, graphics
//     Phase 4: Standard Library -> funcreg, error_registry, builtins
//     Phase 5: Module System    -> security, modules, activation
//     Phase 6: Dialect & Config -> dialect selection, overrides
//     Phase 7: Ready State      -> runtime_init(), RNG seed
//
//   Each phase returns a BootStatus:
//     BOOT_OK       (0) -- phase completed normally
//     BOOT_DEGRADED (1) -- non-critical failure (continues)
//     BOOT_CRITICAL (2) -- fatal failure (aborts boot)
//
//   Shutdown reverses the order (phase 7 -> phase 0).
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new boot phase:
//   1. Add the phase to the BootPhase enum in boot.h
//      (before PHASE_COUNT).
//   2. Add the phase name to phase_names[] below.
//   3. Create a static boot_phaseN_xxx() function.
//   4. Add the phase call in boot_execute() below.
//   5. Add symmetric cleanup in boot_shutdown() if needed.
//
//   Adding a new built-in module:
//   1. Create the module file (modules/mod_xxx.c).
//   2. Add mod_xxx_register() call in Phase 5 below.
//   3. Wrap in #ifndef BPP_FREEDOS if too large for DOS.
//
//   Changing boot verbosity:
//   - Set BootConfig.verbosity before calling boot_execute():
//       BOOT_SILENT  (0) -- no boot output
//       BOOT_LOG     (1) -- phase transitions only
//       BOOT_DEBUG   (2) -- subsystem details
//       BOOT_VERBOSE (3) -- continuous telemetry
//   - CLI flag: -v (BOOT_LOG), -vv (BOOT_DEBUG), -vvv (BOOT_VERBOSE)
//
// FINE-TUNING:
//   - Boot order is immutable -- phases have hard dependencies.
//   - To skip a non-critical phase on embedded platforms, have
//     it return BOOT_OK immediately without initializing.
//   - On FreeDOS (BPP_FREEDOS), USB/FujiNet/UPnP modules are
//     excluded to save memory (~15 KB per module).
//   - The RNG seed is deterministic (1) -- identical config
//     produces identical state on all platforms. Users override
//     with RANDOMIZE in their programs.
//
// TROUBLESHOOTING:
//   - "SORRY. Cannot allocate memory." at startup:
//     Phase 1 (Core Memory) failed. Not enough RAM for the
//     configured pool sizes. Solutions:
//       1. Reduce MAX_VARIABLE_POOL, MAX_SCRATCH_POOL in config.h.
//       2. Reduce MAX_PROGRAM_LINES in config.h.
//       3. Use the BPP_FREEDOS profile for smaller defaults.
//
//   - "WARNING: Failed to load: <module>":
//     Phase 5 (Module System) couldn't load a CLI-specified module.
//     The module file doesn't exist or has incompatible ABI.
//     Boot continues in DEGRADED mode (module unavailable).
//
//   - Boot hangs at "Phase 3: Virtual Devices":
//     Network initialization (vdev_net_init) may block on Windows
//     if WSAStartup fails. Check network stack health.
//
//   - "Phase 6: Dialect & Config" shows wrong dialect:
//     Check BootConfig.dialect (set from CLI -d flag or config file).
//     Default: BASICPP_DEFAULT_DIALECT from config.h.
//
//   Using boot diagnostics:
//     Run with -v flag to see phase transitions:
//       basicpp -v myprogram.bas
//     Run with -vv for full subsystem detail:
//       basicpp -vv
//
// PERFORMANCE:
//   - Boot is a one-time cost (<10ms on modern hardware).
//   - Each phase is O(1) except Phase 5 (module loading, O(M)).
//   - Shutdown is O(1) (no iteration over dynamic data).
//
// MINIMALIZATION:
//   This file is CORE tier -- required for any build.
//   For the smallest possible boot:
//   - Remove Phase 3 devices (vdev, fileio, graphics) if unused.
//   - Remove Phase 5 modules (keep only STDLIB).
//   - Phase 0, 1, 2, 4, 6, 7 are always required.
//   - On embedded: skip graphics, network, transaction journal.
//
// DEPENDENCIES:
//   - boot.h, memory.h, errors.h, platform.h, lexer.h, vm.h
//   - scope.h, keyword_props.h, override.h, vdev.h, vdev_net.h
//   - fileio.h, gfxbuf.h, funcreg.h, error_registry.h
//   - security.h, module.h, mod_stdlib.h, mod_usb.h, etc.
//   - dialect.h, device_alias.h, txn.h, runtime.h, builtins.h
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (stdio.h, stdarg.h, string.h).
//   - No platform-specific code (platform abstraction in platform.c).
//   - Compiles cleanly on MSVC /std:c17 and gcc -std=c17.
//
// ---

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#endif
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
#include "io/vfs.h"
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
#include "mod_gwbasic.h"
#endif
#ifndef NO_SDL2
#include "gw_sdl2.h"
#endif
#include "dialect.h"
#include "device_alias.h"
#include "txn.h"
#include "runtime.h"
#include "task.h"
#include "builtins.h"
#include "../console.h"

// -----------------------------------------------------------------
// Static Boot State
// -----------------------------------------------------------------
// Tracks the progress and diagnostics of the current boot sequence.
// Accessible via boot_get_state() for post-boot inspection.
// -----------------------------------------------------------------

static BootState boot_state;

// Phase names for diagnostic output
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

static struct RuntimeState *g_boot_runtime = NULL;

// Status names for diagnostic output
static const char *status_names[] = {
    "OK",
    "DEGRADED",
    "CRITICAL"
};

// -----------------------------------------------------------------
// Diagnostic Logging
// -----------------------------------------------------------------

// boot_log - Write diagnostic output to stderr.
//
// Only writes if the message's verbosity level is at or below
// the configured verbosity. Format:
//   [BOOT] <message>
//
// Parameters:
//   level - message verbosity (BOOT_LOG, BOOT_DEBUG, BOOT_VERBOSE)
//   fmt   - printf-style format string
//   ...   - format arguments
//
void boot_log(BootVerbosity level, const char *fmt, ...)
{
    va_list args;
    int max_verbosity = boot_state.verbosity;

    if (g_boot_runtime && g_boot_runtime->log_fp) {
        if (g_boot_runtime->log_level > max_verbosity) {
            max_verbosity = g_boot_runtime->log_level;
        }
    }

    if (level > (BootVerbosity)max_verbosity) return;

    if (level <= boot_state.verbosity || (g_boot_runtime && g_boot_runtime->log_fp)) {
        va_start(args, fmt);
        fprintf(stderr, "[BOOT] ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
    }

    if (g_boot_runtime && g_boot_runtime->log_fp && level <= (BootVerbosity)g_boot_runtime->log_level) {
        va_start(args, fmt);
        fprintf((FILE*)g_boot_runtime->log_fp, "[BOOT] ");
        vfprintf((FILE*)g_boot_runtime->log_fp, fmt, args);
        fprintf((FILE*)g_boot_runtime->log_fp, "\n");
        va_end(args);
        fflush((FILE*)g_boot_runtime->log_fp);
    }
}

void boot_log_write(struct RuntimeState *rt, const char *fmt, ...)
{
    va_list args;
    if (rt && rt->log_fp) {
        va_start(args, fmt);
        vfprintf((FILE*)rt->log_fp, fmt, args);
        va_end(args);
        fflush((FILE*)rt->log_fp);
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            va_start(args, fmt);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fflush(stderr);
        }
#else
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fflush(stderr);
#endif
    }
}

void boot_init_runtime(struct RuntimeState *runtime)
{
    g_boot_runtime = runtime;
}

// =================================================================
// Phase 0: Host Entry Point
// =================================================================
// Platform detection. No OS-specific logic affects semantics.
// Only provides: raw execution context, CLI args, file primitives.
//
// What it initializes:
//   - PlatformInfo struct (OS, compiler, word size)
//
// Failure mode: BOOT_CRITICAL (should never fail).
// =================================================================
static BootStatus boot_phase0_host(const BootConfig *config)
{
    (void)config;

    boot_log(BOOT_LOG, "Phase 0: Host Entry");

    // Detect host platform -- must be the very first init call
    platform_init();

    boot_log(BOOT_LOG, "  Platform: %s (%s)",
        platform_name(), platform_short_name());
    boot_log(BOOT_LOG, "  Word size: %d-bit",
        platform_word_size());

    return BOOT_OK;
}

// =================================================================
// Phase 1: Core Memory System Initialization
// =================================================================
// Memory pools, allocation tables, error system.
// CRITICAL -- if this fails, nothing else can work.
//
// What it initializes:
//   - Error system (error_clear)
//   - Variable memory pool (bump allocator)
//   - Scratch memory pool (temporary computations)
//   - Program line store (sorted array)
//
// Failure mode: BOOT_CRITICAL if malloc fails.
// =================================================================
static BootStatus boot_phase1_memory(MemorySystem *memory, struct RuntimeState *runtime)
{
    boot_log(BOOT_LOG, "Phase 1: Core Memory");

    // Initialize error system first (needed by all other phases)
    error_clear();

    // Initialize memory pools
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
        "  Graphics pool: %ld bytes", memory->graphics.size);
    boot_log(BOOT_LOG,
        "  Program store: %d line capacity",
        memory->program.capacity);

    // Initialize stack, string pools, variables, arrays in Phase 1 (A1)
    runtime_init(runtime, &memory->program, memory);
    boot_log(BOOT_DEBUG, "  Runtime memory (stack, string pool, variables) initialized in Phase 1");

    return BOOT_OK;
}

// =================================================================
// Phase 2: VM Core Initialization
// =================================================================
// Dispatch table, execution engine, control-flow systems.
// Must complete before any parsing or execution.
//
// What it initializes:
//   - VM instruction dispatch table
//   - Keyword scope system (block-level scoping)
//   - Keyword property system (attributes per keyword)
//   - Keyword override system (dialect-specific replacements)
//
// Failure mode: BOOT_CRITICAL (should never fail -- static init).
// =================================================================
static BootStatus boot_phase2_vm_core(void)
{
    boot_log(BOOT_LOG, "Phase 2: VM Core");

    // Build instruction dispatch table
    vm_init();
    boot_log(BOOT_DEBUG, "  Dispatch table initialized");

    // Initialize keyword scope system
    scope_init();
    boot_log(BOOT_DEBUG, "  Scope system initialized");

    // Initialize keyword property system
    keyword_props_init();
    boot_log(BOOT_DEBUG, "  Keyword properties initialized");

    // Initialize keyword override system
    override_init();
    boot_log(BOOT_DEBUG, "  Override system initialized");

    return BOOT_OK;
}

// =================================================================
// Phase 3: Virtual Device System Initialization
// =================================================================
// Register CON:, ERR:, FILE:, NET:, GFX: devices.
// CON: and ERR: are critical. Others are non-critical.
//
// What it initializes:
//   - Core device table (vdev: CON:, ERR:, FILE:)
//   - File I/O channels (fileio)
//   - Device alias table (cross-dialect device mapping)
//   - Transaction journal (ATOMIC/TXN support)
//   - Graphics framebuffer (gfxbuf)
//
// Failure mode: BOOT_DEGRADED if non-critical device fails.
// =================================================================
static BootStatus boot_phase3_devices(MemorySystem *memory)
{
    BootStatus status = BOOT_OK;

    boot_log(BOOT_LOG, "Phase 3: Virtual Devices");

    // Network socket layer (vdev_net_init) is now lazily initialized on demand

    // Core device table (CON:, ERR:, FILE:)
    vdev_init();
    boot_log(BOOT_DEBUG, "  Device table initialized");

    // File I/O channels
#ifndef BPP_LITE_BUILD
    fileio_channels_init();
    boot_log(BOOT_DEBUG, "  File channels initialized");
#endif

    // Virtual Filesystem Layer
    vfs_init();
    boot_log(BOOT_DEBUG, "  Virtual Filesystem initialized");

    // Device alias table (for cross-dialect device mapping)
#ifndef BPP_LITE_BUILD
    device_alias_init();
    boot_log(BOOT_DEBUG, "  Device alias table initialized");
#endif

    // Transaction journal (ATOMIC/TXN support)
#ifndef BPP_LITE_BUILD
    boot_log_write(g_boot_runtime, "[TRACE3] before txn_init\n");
    txn_init();
    boot_log_write(g_boot_runtime, "[TRACE3] after txn_init\n");
    boot_log(BOOT_DEBUG, "  Transaction journal initialized");
#endif

    // Graphics framebuffer (non-critical)
#ifndef BPP_LITE_BUILD
    gfxbuf_init_pool(memory);
    boot_log(BOOT_DEBUG, "  Graphics framebuffer initialized with dynamic pool");
#endif

    return status;
}

// =================================================================
// Phase 4: Standard Library Initialization
// =================================================================
// Register built-in functions and error messages.
// After this: ABS, SIN, LEN, LEFT$, etc. are available.
//
// What it initializes:
//   - Function registry (funcreg: name -> function pointer map)
//   - Error message registry (error code -> message map)
//
// Failure mode: BOOT_CRITICAL (should never fail -- static init).
// =================================================================
static BootStatus boot_phase4_stdlib(void)
{
    boot_log(BOOT_LOG, "Phase 4: Standard Library");

    // Initialize function registry
    funcreg_init();
    boot_log(BOOT_DEBUG, "  Function registry initialized");

    // Initialize error message registry
    error_registry_init();
    boot_log(BOOT_DEBUG, "  Error registry initialized");

    boot_state.functions_registered = funcreg_count();
    boot_log(BOOT_LOG, "  Functions registered: %d",
        boot_state.functions_registered);

    return BOOT_OK;
}

// =================================================================
// Phase 5: Module System Initialization
// =================================================================
// Security, module table, built-in modules, CLI modules.
// Module failures are non-critical (DEGRADED, not CRITICAL).
//
// What it initializes:
//   - Security system (level, permission matrix)
//   - Module table (registration slots)
//   - Built-in modules: STDLIB, USB, FujiNet, UPnP
//   - CLI-specified modules (from -l, -m, -f flags)
//   - STDLIB is always activated
//
// Failure mode: BOOT_DEGRADED if a module fails to load.
// =================================================================
static BootStatus boot_phase5_modules(
    const BootConfig *config,
    struct RuntimeState *runtime)
{
    BootStatus status = BOOT_OK;

    boot_log_write(runtime, "[TRACE] inside boot_phase5_modules start\n");
    boot_log(BOOT_LOG, "Phase 5: Module System");
    boot_log_write(runtime, "[TRACE] after boot_log in boot_phase5_modules\n");

    // Initialize security system BEFORE modules load
    // (modules check their capabilities against security level)
    boot_log_write(runtime, "[TRACE] calling security_init(%d)\n", config->security);
    security_init(config->security);
    boot_log(BOOT_LOG, "  Security level: %s",
        security_level_name(config->security));

    // Initialize module table
    module_system_init();

    // Register built-in modules
    mod_stdlib_register();
    boot_log(BOOT_DEBUG, "  Registered: STDLIB");

#if !defined(BPP_FREEDOS) && !defined(BPP_LITE_BUILD)
    mod_usb_register();
    boot_log(BOOT_DEBUG, "  Registered: USB");

    mod_fujinet_register();
    boot_log(BOOT_DEBUG, "  Registered: FUJINET");

    mod_upnp_register();
    boot_log(BOOT_DEBUG, "  Registered: UPNP");

    mod_gwbasic_register();
    boot_log(BOOT_DEBUG, "  Registered: GWBASIC");
#endif

    // Activate STDLIB (always -- provides core math/string functions)
    module_activate("STDLIB", runtime);

#if !defined(BPP_FREEDOS) && !defined(BPP_LITE_BUILD)
    if (config->dialect == DIALECT_GW_BASIC) {
        module_activate("GWBASIC", runtime);
    }
#endif

    // Load external modules from CLI flags
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

// =================================================================
// Phase 6: Dialect Selection and Configuration
// =================================================================
// Select dialect, apply overrides, set strict mode.
// Dialect selection occurs AFTER all functions are registered
// so overrides can target the complete function table.
//
// What it initializes:
//   - Active dialect selection
//   - Dialect-specific function overrides
//   - Device alias loading for the dialect
//   - Strict mode (if configured)
//
// Failure mode: BOOT_OK (dialect always falls back to default).
// =================================================================
static BootStatus boot_phase6_dialect(const BootConfig *config, MemorySystem *memory)
{
    boot_log(BOOT_LOG, "Phase 6: Dialect & Config");

    DialectId active_dialect = config->dialect;

    // Check program lines for a dialect header (e.g. "'$lang: qbasic" or "10 REM $lang: c64")
    if (memory && memory->program.count > 0) {
        int limit = memory->program.count;
        if (limit > 5) limit = 5;
        int idx;
        for (idx = 0; idx < limit; idx++) {
            const char *text = memory->program.lines[idx].text;
            const char *lang_ptr = strstr(text, "$lang");
            if (lang_ptr) {
                lang_ptr += 5;
                while (*lang_ptr == ':' || *lang_ptr == ' ' || *lang_ptr == '\t') {
                    lang_ptr++;
                }
                char lang_name[32];
                int name_len = 0;
                while (name_len < 31 && (isalnum((unsigned char)lang_ptr[name_len]) || lang_ptr[name_len] == '_' || lang_ptr[name_len] == '-' || lang_ptr[name_len] == '+')) {
                    lang_name[name_len] = lang_ptr[name_len];
                    name_len++;
                }
                lang_name[name_len] = '\0';
                
                int detected = dialect_find_by_name(lang_name);
                if (detected >= 0) {
                    active_dialect = (DialectId)detected;
                    boot_log(BOOT_LOG, "  Detected dialect header: %s", lang_name);
                    break;
                }
            }
        }
    }

    // Initialize and select dialect
    dialect_init(active_dialect);
    boot_log(BOOT_LOG, "  Dialect: %s (%s)",
        dialect_get_name(), dialect_get_short_name());

    // Apply dialect-specific function overrides and device aliases
    dialect_apply();
    boot_log(BOOT_DEBUG, "  Dialect overrides applied");

    // Apply strict mode if configured via CLI or config file, or if dialect header detected
    if (config->strict || active_dialect != config->dialect) {
        dialect_set_strict(1);
        boot_log(BOOT_LOG, "  Strict mode: ON");
    }

    // Report final function count (may have changed via overrides)
    boot_state.functions_registered = funcreg_count();
    boot_log(BOOT_DEBUG,
        "  Functions after dialect: %d",
        boot_state.functions_registered);

    return BOOT_OK;
}

// =================================================================
// Phase 7: Interpreter Ready State
// =================================================================
// Initialize runtime state, set deterministic RNG seed,
// enter READY state. After this, the REPL can accept input.
//
// What it initializes:
//   - TASK multitasking manager
//   - Ready state flags, RNG seed
//
// Failure mode: BOOT_OK (always succeeds).
// =================================================================
static BootStatus boot_phase7_ready(
    MemorySystem *memory,
    struct RuntimeState *runtime)
{
    (void)memory;
    boot_log(BOOT_LOG, "Phase 7: Ready State");

    // Initialize TASK multitasking manager
    task_mgr_init(runtime);

    // RNG seed is set to 1 -- deterministic by design.
    runtime->running = 0;
    runtime->rnd_seed = 1;
    boot_log(BOOT_DEBUG, "  RNG seed: deterministic (1)");

    // Print boot summary if boot-log mode is enabled
    if (boot_state.verbosity >= BOOT_LOG) {
        boot_print_summary();
    }

    boot_log(BOOT_LOG, "Boot complete.");
    return BOOT_OK;
}

// =================================================================
// Boot Execute -- Run the Full Boot Sequence
// =================================================================

// boot_execute - Run all 8 phases in order.
//
// Parameters:
//   config  - boot configuration (dialect, security, verbosity, CLI)
//   memory  - memory system to initialize
//   runtime - runtime state to initialize
//
// Returns:
//   BOOT_OK       -- all phases succeeded
//   BOOT_DEGRADED -- some non-critical phases failed
//   BOOT_CRITICAL -- a critical phase failed (boot aborted)
//
// On BOOT_CRITICAL, the caller should print an error and exit.
// On BOOT_DEGRADED, the interpreter runs with reduced functionality.
//
BootStatus boot_execute(const BootConfig *config,
                        MemorySystem *memory,
                        struct RuntimeState *runtime)
{
    int i;
    BootStatus phase_result;

    // Initialize boot state tracking
    memset(&boot_state, 0, sizeof(boot_state));
    boot_state.verbosity = config->verbosity;
    boot_state.overall_status = BOOT_OK;

    g_boot_runtime = runtime;

    // Phase 0: Host Entry
    boot_state.current_phase = PHASE_HOST;
    phase_result = boot_phase0_host(config);
    boot_state.phase_status[PHASE_HOST] = phase_result;
    if (phase_result == BOOT_CRITICAL) {
        boot_state.overall_status = BOOT_CRITICAL;
        g_boot_runtime = NULL;
        return BOOT_CRITICAL;
    }

    // Phase 1: Core Memory (CRITICAL -- abort on failure)
    boot_state.current_phase = PHASE_MEMORY;
    phase_result = boot_phase1_memory(memory, runtime);
    boot_state.phase_status[PHASE_MEMORY] = phase_result;
    if (phase_result == BOOT_CRITICAL) {
        boot_state.overall_status = BOOT_CRITICAL;
        printf("SORRY. Cannot allocate memory.\n");
        g_boot_runtime = NULL;
        return BOOT_CRITICAL;
    }

    // Phase 2: VM Core
    boot_state.current_phase = PHASE_VM_CORE;
    phase_result = boot_phase2_vm_core();
    boot_state.phase_status[PHASE_VM_CORE] = phase_result;
    if (phase_result == BOOT_CRITICAL) {
        boot_state.overall_status = BOOT_CRITICAL;
        g_boot_runtime = NULL;
        return BOOT_CRITICAL;
    }

    // Phase 3: Virtual Devices
    boot_state.current_phase = PHASE_DEVICES;
    phase_result = boot_phase3_devices(memory);
    boot_state.phase_status[PHASE_DEVICES] = phase_result;
    if (phase_result == BOOT_DEGRADED) {
        boot_state.overall_status = BOOT_DEGRADED;
    }
    if (phase_result == BOOT_CRITICAL) {
        boot_state.overall_status = BOOT_CRITICAL;
        g_boot_runtime = NULL;
        return BOOT_CRITICAL;
    }
    runtime->dev_con = vdev_get(VDEV_CON);
    runtime->dev_err = vdev_get(VDEV_ERR);

    // Phase 4: Standard Library
    boot_state.current_phase = PHASE_STDLIB;
    phase_result = boot_phase4_stdlib();
    boot_state.phase_status[PHASE_STDLIB] = phase_result;
    if (phase_result == BOOT_CRITICAL) {
        boot_state.overall_status = BOOT_CRITICAL;
        g_boot_runtime = NULL;
        return BOOT_CRITICAL;
    }

    // Phase 5: Module System (non-critical failures OK)
    boot_log_write(runtime, "[TRACE] before Phase 5\n");
    boot_state.current_phase = PHASE_MODULES;
    boot_log_write(runtime, "[TRACE] calling boot_phase5_modules (config=%p, runtime=%p)\n", config, runtime);
    phase_result = boot_phase5_modules(config, runtime);
    boot_log_write(runtime, "[TRACE] after boot_phase5_modules\n");
    boot_state.phase_status[PHASE_MODULES] = phase_result;
    if (phase_result == BOOT_DEGRADED) {
        boot_state.overall_status = BOOT_DEGRADED;
    }

    // Phase 6: Dialect & Config
    boot_state.current_phase = PHASE_DIALECT;
    phase_result = boot_phase6_dialect(config, memory);
    boot_state.phase_status[PHASE_DIALECT] = phase_result;
    if (phase_result == BOOT_DEGRADED) {
        boot_state.overall_status = BOOT_DEGRADED;
    }

    // Phase 7: Ready State
    boot_state.current_phase = PHASE_READY;
    phase_result = boot_phase7_ready(memory, runtime);
    boot_state.phase_status[PHASE_READY] = phase_result;

    // Compute overall status (worst of all phases)
    for (i = 0; i < PHASE_COUNT; i++) {
        if (boot_state.phase_status[i] > boot_state.overall_status) {
            boot_state.overall_status = boot_state.phase_status[i];
        }
    }

    // Keep g_boot_runtime active during interpreter runtime for continuous telemetry
    return boot_state.overall_status;
}

// =================================================================
// Boot Shutdown -- Symmetric Reverse-Order Teardown
// =================================================================

// boot_shutdown - Shut down all subsystems in reverse phase order.
//
// Each phase's cleanup is the mirror of its initialization.
// Some phases are no-ops (static data doesn't need cleanup).
//
// Parameters:
//   memory - memory system to release
//
void boot_shutdown(MemorySystem *memory)
{
    boot_log(BOOT_LOG, "Shutdown: reverse phase order");

    // Phase 7: Leave ready state (no-op -- runtime cleared by caller)

    // Phase 6: Dialect cleanup (no-op -- static config)

    // Phase 5: Module cleanup -- deactivate all modules
    boot_log(BOOT_DEBUG, "  Deactivating modules...");
    // Modules clean up via their deactivate callbacks.
    // A future module_system_shutdown() could iterate all.

    // Phase 4: Stdlib cleanup (no-op -- static registry)

    // Phase 3: Device cleanup -- close network sockets
    boot_log(BOOT_DEBUG, "  Closing network devices...");
#ifndef BPP_LITE_BUILD
    vdev_net_cleanup();
#endif
#ifndef NO_SDL2
    gw_sdl2_cleanup();
#endif

    // Phase 2: VM cleanup (no-op -- static dispatch table)

    // Phase 1: Memory cleanup -- release all memory pools
    boot_log(BOOT_DEBUG, "  Releasing memory pools...");
    task_mgr_shutdown();
    mem_shutdown(memory);

    // Phase 0: Host cleanup (no-op -- return from main)
    boot_log(BOOT_LOG, "Shutdown complete.");
}

// =================================================================
// Diagnostic Functions
// =================================================================

// boot_get_state - Return the boot state for inspection.
//
// The returned pointer is valid for the lifetime of the process.
// Useful for post-boot diagnostics (e.g., SYSTEM command).
//
const BootState *boot_get_state(void)
{
    return &boot_state;
}

void boot_downgrade_status(BootStatus status)
{
    if (status > boot_state.overall_status) {
        boot_state.overall_status = status;
    }
}

// boot_phase_name - Return the human-readable name for a phase.
const char *boot_phase_name(BootPhase phase)
{
    if (phase >= 0 && phase < PHASE_COUNT) {
        return phase_names[phase];
    }
    return "Unknown";
}

// boot_status_name - Return the human-readable name for a status.
const char *boot_status_name(BootStatus status)
{
    if (status >= BOOT_OK && status <= BOOT_CRITICAL) {
        return status_names[status];
    }
    return "Unknown";
}

// boot_print_summary - Print the boot diagnostic summary to stderr.
//
// Shows all 8 phases with their status, plus aggregate counts
// for functions, devices, modules, and warnings.
//
// Example output:
//   === BASIC++ Boot Summary ===
//   Phase 0: Host Entry          [OK]
//   Phase 1: Core Memory         [OK]
//   Phase 2: VM Core             [OK]
//   Phase 3: Virtual Devices     [OK]
//   Phase 4: Standard Library    [OK]
//   Phase 5: Module System       [DEGRADED]
//   Phase 6: Dialect & Config    [OK]
//   Phase 7: Ready State         [OK]
//   ---
//   Functions: 87
//   Devices:   5
//   Modules:   4 registered, 1 failed
//   Warnings:  1
//   Overall:   DEGRADED
//   ============================
//
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

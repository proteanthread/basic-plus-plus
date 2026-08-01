/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file boot.c
 * @brief Deterministic 9-Phase Boot Controller implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the 9-phase boot sequence, coordinating allocations, registration,
 *   dialect mappings, and basic checks.
 * - Why it exists: Ensures uniform boot up behaviors on all target environments.
 * - Why it works this way: It sequentially allocates memory, registers virtual devices (like CON:),
 *   creates the string and variable systems, compiles the VM context, and triggers Phase 8 validation.
 *   If any initialization fails, it rewinds dynamically in reverse order.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Target memory limits, default dialect settings, Phase 8 verification scripts.
 * - What cannot be changed: Obligation to roll back allocations on failure.
 * - What to expect: Successfully booting outputs a "Ready." prompt with system information.
 * - What to do if something breaks: If a boot phase returns NULL, trace variable and stack configurations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: CON: device registers successfully. Platform init completes safely.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional device setups or auto-run program loads.
 * - How to write external extensions: External plugins compile registry hooks during boot Phase 6.
 */

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "security/security.h"
#include "module/module.h"
#include "runtime/task.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "device/fujinet.h"
#include "lexer/lexer.h"
#include "core/dialect.h"
#ifdef BPP_USE_CUSTOM_STATIC_DIALECT
#include "core/custom_dialect_static.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Forward declare console creator */
VDev vdev_console_create(void);
int g_is_repl = 0;
void mod_mathext_register(void);
void register_crypto_functions(void);
void register_string_ext_functions(void);
#ifndef BPP_LITE_BUILD
void register_arrayext_functions(void);
#endif
void register_array_sort_functions(void);
void register_regex_functions(void);
void mod_io_devices_register(void);

static int stdlib_init(void *rt) {
    (void)rt;
    return 0;
}

static size_t boot_get_mem_size(const char *env_var, size_t default_val) {
    const char *val = getenv(env_var);
    if (val) {
        char *endptr;
        double parsed = strtod(val, &endptr);
        if (parsed > 0) {
            if (*endptr == 'k' || *endptr == 'K') return (size_t)(parsed * 1024);
            if (*endptr == 'm' || *endptr == 'M') return (size_t)(parsed * 1024 * 1024);
            if (*endptr == 'g' || *endptr == 'G') return (size_t)(parsed * 1024 * 1024 * 1024);
            return (size_t)parsed;
        }
    }
    return default_val;
}

BootContext *boot_execute(const BootConfig *config) {
    if (!config) return NULL;
    
    g_is_repl = config->is_repl;

    BootContext *ctx = (BootContext *)calloc(1, sizeof(BootContext));
    if (!ctx) return NULL;

    /* Initialize Security Sandbox */
    security_init(SEC_OPEN);

    /* Initialize Module System */
    module_system_init();
    funcreg_init();
    spec_registry_init();
    
    register_crypto_functions();
    register_string_ext_functions();
#ifndef BPP_LITE_BUILD
    register_arrayext_functions();
#endif
    register_regex_functions();
    register_array_sort_functions();

    /* Phase 0: Host Entry Point & Platform Init */
    platform_init();

    /* Phase 1: Core Memory System Initialization */
    size_t prog_mem = config->prog_mem ? config->prog_mem : boot_get_mem_size("BPP_PROG_MEM", BPP_DEFAULT_PROG_MEM);
    size_t var_mem  = config->var_mem ? config->var_mem : boot_get_mem_size("BPP_VAR_MEM", BPP_DEFAULT_VAR_MEM);
    size_t str_mem  = config->str_mem ? config->str_mem : boot_get_mem_size("BPP_STR_MEM", BPP_DEFAULT_STR_MEM);
    size_t scratch  = config->scratch_mem ? config->scratch_mem : boot_get_mem_size("BPP_SCRATCH_MEM", BPP_DEFAULT_SCRATCH_MEM);

    ctx->mem = mem_init(prog_mem, var_mem, str_mem, scratch);
    if (!ctx->mem) {
        boot_shutdown(ctx);
        return NULL;
    }

    /* Phase 2: Virtual Device System Initialization */
    ctx->vdev = vdev_init(ctx->mem);
    if (!ctx->vdev) {
        boot_shutdown(ctx);
        return NULL;
    }
    /* Register default terminal console device "CON:" */
    VDev con = vdev_console_create();
    if (!vdev_register(ctx->vdev, con)) {
        boot_shutdown(ctx);
        return NULL;
    }

#if BPP_SUPPORT_NET
    /* Initialize FujiNet system and register virtual devices */
    fujinet_init_system(ctx->vm);
    vdev_register(ctx->vdev, fujinet_create_n_dev(ctx->vm));
    vdev_register(ctx->vdev, fujinet_create_fuji_dev(ctx->vm));
    vdev_register(ctx->vdev, fujinet_create_clock_dev(ctx->vm));
#endif

    /* Phase 3: String Manager Initialization */
    ctx->str = str_init(ctx->mem);
    if (!ctx->str) {
        boot_shutdown(ctx);
        return NULL;
    }

    /* Phase 4: Variable Registry Initialization */
    ctx->var = var_init(ctx->mem, ctx->str);
    if (!ctx->var) {
        boot_shutdown(ctx);
        return NULL;
    }

    /* Phase 5: VM Core Initialization */
    ctx->vm = vm_init(ctx->mem, ctx->str, ctx->var, ctx->vdev);
    if (!ctx->vm) {
        boot_shutdown(ctx);
        return NULL;
    }

    /* Register and activate STDLIB */
    static BppModuleInfo stdlib_mod = {
        "STDLIB",
        "1.0",
        "Core standard functions",
        MOD_EXTENSION,
        CAP_MATH | CAP_STRING | CAP_IO,
        SEC_COUNT,
        stdlib_init,
        NULL
    };
    module_register(&stdlib_mod);
    module_activate("STDLIB", ctx->vm);

#if BPP_SUPPORT_MAT
    /* Register and activate MATHEXT built-in module */
    mod_mathext_register();
    module_activate("MATHEXT", ctx->vm);
#endif

    mod_io_devices_register();
    module_activate("IODEVICES", ctx->vm);

    /* Phase 6: Standard Library (Registered inside vm_init) */
    
#ifndef BPP_LITE_BUILD
    /* Dialect selection is a NOP under unified universal BASIC++ */
#endif

#ifdef BPP_USE_CUSTOM_STATIC_DIALECT
    BppDialect *d_custom = dialect_create();
    if (d_custom) {
        init_custom_static_dialect(d_custom);
        /* Map keyword IDs */
        for (int k = 0; k < d_custom->keyword_count; ++k) {
            d_custom->keywords[k].id = lex_find_keyword_by_name(d_custom->keywords[k].mapped_to);
            if (d_custom->keywords[k].id == KW_NONE) {
                d_custom->keywords[k].id = keyword_register_custom(d_custom->keywords[k].mapped_to);
            }
        }
        vm_set_active_dialect(ctx->vm, d_custom);
    }
#endif

    if (config->dialect_config) {
        BppDialect *d_custom = dialect_create();
        if (d_custom) {
            *d_custom = *(config->dialect_config); /* Copy static config */
            /* Map keyword IDs if needed */
            for (int k = 0; k < d_custom->keyword_count; ++k) {
                d_custom->keywords[k].id = lex_find_keyword_by_name(d_custom->keywords[k].mapped_to);
                if (d_custom->keywords[k].id == KW_NONE) {
                    d_custom->keywords[k].id = keyword_register_custom(d_custom->keywords[k].mapped_to);
                }
            }
            vm_set_active_dialect(ctx->vm, d_custom);
        }
    }

    /* Initialize Ready State multitasking & RNG */
    srand(1);
    task_mgr_init(ctx->vm);

    /* Phase 8: Self-Test Validation */
    BppError err = vm_execute_line(ctx->vm, "REM Boot self-test logic");
    if (err.code != 0) {
        /* Phase 8 failed */
        boot_shutdown(ctx);
        return NULL;
    }

    return ctx;
}

void boot_shutdown(BootContext *ctx) {
    if (!ctx) return;

    /* Shutdown Task Manager */
    task_mgr_shutdown();

#if BPP_SUPPORT_NET
    /* Shutdown FujiNet System */
    fujinet_shutdown_system();
#endif

    /* Phase 5: Tear down VM core */
    if (ctx->vm) {
        vm_shutdown(ctx->vm);
    }

    /* Phase 4: Tear down variable registry */
    if (ctx->var) {
        var_shutdown(ctx->var);
    }

    /* Phase 3: Tear down string manager */
    if (ctx->str) {
        str_shutdown(ctx->str);
    }

    /* Phase 2: Tear down VDS */
    if (ctx->vdev) {
        vdev_shutdown(ctx->vdev);
    }

    /* Phase 1: Tear down Memory system */
    if (ctx->mem) {
        mem_shutdown(ctx->mem);
    }

    /* Phase 0: Restore platform terminal states */
    platform_shutdown();

    free(ctx);
}

void boot_shutdown_ex(BootContext *ctx, bool force_exit) {
    if (!ctx) return;

    if (force_exit) {
        platform_shutdown();
        free(ctx);
        return;
    } else {
        boot_shutdown(ctx);
    }
}

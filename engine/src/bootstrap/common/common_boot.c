// FILENAME: common_boot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: 
// NEEDS: libboot (common_internal.h)
// NEEDS: libcore (hal.h)
// NEEDS: libkernel (vdev_esp32.h, vdev_esp32.c)
// Provides core logic and interface definitions for common_boot within BASIC++.
//
// ---- Includes ----

#include "bootstrap/common_internal.h"
#include "hal/hal.h"
#include "device/vdev_esp32.h"

// Forward declarations
VDev vdev_console_create(void);
VDev vdev_esp32_create(const char *name);
int g_is_repl = 0;
int mod_mathext_init(void *rt);

static int stdlib_init(void *rt) {
    (void)rt;
    return 0;
}

static size_t boot_get_mem_size(const char *env_var, size_t default_val) {
    const char *val = platform_getenv(env_var);
    if (val) {
        char *endptr = NULL;
        double parsed = runtime_strtod(val, &endptr);
        if (parsed > 0) {
            if (endptr && (*endptr == 'k' || *endptr == 'K')) return (size_t)(parsed * 1024);
            if (endptr && (*endptr == 'm' || *endptr == 'M')) return (size_t)(parsed * 1024 * 1024);
            if (endptr && (*endptr == 'g' || *endptr == 'G')) return (size_t)(parsed * 1024 * 1024 * 1024);
            return (size_t)parsed;
        }
    }
    return default_val;
}

//
// ---- Bootstrap Orchestrator ----
//

BootContext *boot_execute(const BootConfig *config) {
    if (!config) return NULL;
    
    g_is_repl = config->is_repl;

    if (config->custom_hal) {
        hal_set(config->custom_hal);
    } else if (config->is_freestanding) {
        hal_init_freestanding();
    } else {
        hal_init_hosted();
    }

    HalContext *hal = hal_get();
    BootContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (BootContext *)hal->mem.alloc(sizeof(BootContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(BootContext));

    security_init(SEC_OPEN);
    feature_reg_init();

    funcreg_init();
    microlib_init();
    spec_registry_init();

    boot_register_all_statements();
    boot_register_all_functions();

    platform_init();

    size_t prog_mem = config->prog_mem ? config->prog_mem : boot_get_mem_size("BASIC_PROG_MEM", BASIC_DEFAULT_PROG_MEM);
    size_t var_mem  = config->var_mem ? config->var_mem : boot_get_mem_size("BASIC_VAR_MEM", BASIC_DEFAULT_VAR_MEM);
    size_t str_mem  = config->str_mem ? config->str_mem : boot_get_mem_size("BASIC_STR_MEM", BASIC_DEFAULT_STR_MEM);
    size_t scratch  = config->scratch_mem ? config->scratch_mem : boot_get_mem_size("BASIC_SCRATCH_MEM", BASIC_DEFAULT_SCRATCH_MEM);

    ctx->mem = mem_init(prog_mem, var_mem, str_mem, scratch);
    if (!ctx->mem) {
        boot_shutdown(ctx);
        return NULL;
    }

    ctx->vdev = vdev_init(ctx->mem);
    if (!ctx->vdev) {
        boot_shutdown(ctx);
        return NULL;
    }

    VDev con = vdev_console_create();
    if (!vdev_register(ctx->vdev, con)) {
        boot_shutdown(ctx);
        return NULL;
    }

    static const char * const k_vprinter_names[] = {
        "LPT:", "LPT1:", "LPT2:", "LPT3:", "LPT4:", "LPT5:", "LPT6:", "LPT7:", "LPT8:",
        "PRN:", "PRN1:", "PRN2:", "PRN3:", "PRN4:", "PRN5:", "PRN6:", "PRN7:", "PRN8:",
        "LST:", "LST1:", "LST2:", "LST3:", "LST4:", "LST5:", "LST6:", "LST7:", "LST8:",
        "PDF:", "PDF1:", "PDF2:", "PDF3:", "PDF4:", "PDF5:", "PDF6:", "PDF7:", "PDF8:"
    };

    vprinter_init();
    for (size_t i = 0; i < sizeof(k_vprinter_names) / sizeof(k_vprinter_names[0]); ++i) {
        vdev_register(ctx->vdev, vprinter_create_vdev(k_vprinter_names[i]));
    }
    vdev_register(ctx->vdev, vdev_esp32_create("ESP32:"));

#if SUPPORT_NET
    fujinet_init_system(ctx->vm);
    vdev_register(ctx->vdev, fujinet_create_n_dev(ctx->vm));
    vdev_register(ctx->vdev, fujinet_create_fuji_dev(ctx->vm));
    vdev_register(ctx->vdev, fujinet_create_clock_dev(ctx->vm));
#endif

    ctx->str = str_init(ctx->mem);
    if (!ctx->str) {
        boot_shutdown(ctx);
        return NULL;
    }

    ctx->var = var_init(ctx->mem, ctx->str);
    if (!ctx->var) {
        boot_shutdown(ctx);
        return NULL;
    }

    ctx->vm = vm_init(ctx->mem, ctx->str, ctx->var, ctx->vdev);
    if (!ctx->vm) {
        boot_shutdown(ctx);
        return NULL;
    }

    stdlib_init(ctx->vm);
#if SUPPORT_MAT
    mod_mathext_init(ctx->vm);
#endif

    BppError err = vm_execute_line(ctx->vm, "REM Boot self-test logic");
    if (err.code != 0) {
        boot_shutdown(ctx);
        return NULL;
    }

    return ctx;
}

void boot_shutdown(BootContext *ctx) {
    if (!ctx) return;

    vprinter_shutdown();
    if (ctx->vm) vm_shutdown(ctx->vm);
    if (ctx->var) var_shutdown(ctx->var);
    if (ctx->str) str_shutdown(ctx->str);
    if (ctx->vdev) vdev_shutdown(ctx->vdev);
    if (ctx->mem) mem_shutdown(ctx->mem);

    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(ctx);
    }
}

VMContext *boot_system(size_t heap_size) {
    BootConfig config;
    runtime_memset(&config, 0, sizeof(config));
    
    if (heap_size > 0) {
        config.prog_mem    = heap_size / 4;
        config.var_mem     = heap_size / 4;
        config.str_mem     = heap_size / 4;
        config.scratch_mem = heap_size / 4;
    }
    
    config.is_repl = 0;
    
    BootContext *boot = boot_execute(&config);
    if (!boot) return NULL;
    
    VMContext *vm = boot->vm;
    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(boot);
    }
    return vm;
}

void boot_shutdown_vm(VMContext *vm) {
    if (!vm) return;
    MemoryContext *mem = vm_get_mem(vm);
    StringContext *str = vm_get_str(vm);
    VariableContext *var = vm_get_var(vm);
    VDevContext *vdev = vm_get_vdev(vm);

    vm_shutdown(vm);
    if (var) var_shutdown(var);
    if (str) str_shutdown(str);
    if (vdev) vdev_shutdown(vdev);
    if (mem) mem_shutdown(mem);
}

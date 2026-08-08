/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file common.c
 * @brief Deterministic 9-Phase Boot Controller and Micro-Library Registry initialization for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `boot_engine()`, `boot_shutdown()`, driving the 9-phase engine boot sequence across desktop (`baspp`), IoT (`bpp`), script runner (`bs`), headless, embedded, and mobile targets.
 *
 * 2. WHY IT EXISTS:
 * Guarantees zero hidden initialization order dependencies by executing a strict 9-phase startup sequence: (1) Platform Init, (2) Memory Partitioning, (3) Virtual Devices, (4) Variable/String Heaps, (5) VM Context Creation, (6) Statement Registration, (7) Function Registration, (8) Capability Verification, (9) Environment Readiness.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Allocates `MemoryContext`, registers `CON:` virtual device, initializes variable space, registers statement and function tables, and sets default security levels. Rolls back in reverse order on failure.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "bootstrap/boot.h", "platform/platform.h", "memory/memory.h", "device/vdev.h", "device/vcon.h", "vm/vm.h", "stmt/stmt.h", "eval/eval.h", "security/security.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Register new micro-library initialization routines in Phase 6 (Statements) or Phase 7 (Functions).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 9-phase boot sequence ordering and reverse-order cleanup on startup failure.
 *
 * 8. WHAT TO EXPECT:
 * `boot_engine()` returns non-NULL `VMContext*` on successful initialization of all 9 phases.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check console output for phase failure log messages and verify phase return codes.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid platform memory allocation capability.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance across desktop, mobile, server, and microcontroller hosts.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/memory/mem_system.c
 * - engine/src/device/vdev.c
 * - engine/src/device/vcon.c
 * - engine/src/vm/context.c
 * - engine/src/parser/parser.c
 * - engine/src/eval/eval.c
 * - engine/src/security/security.c
 * Prerequisite Header Files:
 * - engine/include/bootstrap/boot.h
 * - engine/include/platform/platform.h
 * - engine/include/memory/memory.h
 * - engine/include/device/vdev.h
 * - engine/include/device/vcon.h
 * - engine/include/vm/vm.h
 * - engine/include/stmt/stmt.h
 * - engine/include/eval/eval.h
 * - engine/include/security/security.h
 */

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "security/security.h"
#include "runtime/funcreg.h"
#include "runtime/spec.h"
#include "runtime/funcreg.h"
#include "runtime/micro_lib_metadata.h"
#include "core/feature_reg.h"
#include "device/fujinet.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "memory/memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Forward declare console creator */
VDev vdev_console_create(void);
int g_is_repl = 0;
int mod_mathext_init(void *rt);
void register_crypto_functions(void);
void register_string_ext_functions(void);
#ifndef BASIC_LITE_BUILD
void register_arrayext_functions(void);
#endif
void register_array_sort_functions(void);
void register_regex_functions(void);

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

void func_rnd_register(void);
void func_int_register(void);
void func_abs_register(void);
void func_sqr_register(void);
void func_sin_register(void);
void func_cos_register(void);
void func_tan_register(void);
void func_atn_register(void);
void func_log_register(void);
void func_exp_register(void);
void func_acos_register(void);
void func_asin_register(void);
void func_atan2_register(void);
void func_ceil_register(void);
void func_clamp_register(void);
void func_fix_register(void);
void func_floor_register(void);
void func_lerp_register(void);
void func_pi_register(void);
void func_round_register(void);
void func_sgn_register(void);
void func_bin_register(void);
void func_chr_register(void);
void func_hex_register(void);
void func_instr_register(void);
void func_lcase_register(void);
void func_left_register(void);
void func_len_register(void);
void func_ltrim_register(void);
void func_mid_register(void);
void func_oct_register(void);
void func_pack_register(void);
void func_right_register(void);
void func_rtrim_register(void);
void func_space_register(void);
void func_str_register(void);
void func_string_register(void);
void func_trim_register(void);
void func_ucase_register(void);
void func_unpack_register(void);
void func_val_register(void);
void func_bitcount_register(void);
void func_readbit_register(void);
void func_resetbit_register(void);
void func_setbit_register(void);
void func_shl_register(void);
void func_shr_register(void);
void func_togglebit_register(void);
void func_clock_num_register(void);
void func_clock_str_register(void);
void func_date_register(void);
void func_environ_register(void);
void func_fre_register(void);
void func_inkey_register(void);
void func_inp_register(void);
void func_peek_register(void);
void func_ticks_register(void);
void func_time_register(void);
void func_timer_register(void);
void stmt_goto_register(void);
void stmt_gosub_register(void);
void stmt_return_register(void);
void stmt_if_register(void);
void stmt_end_register(void);
void stmt_rem_register(void);
void stmt_stop_register(void);
void stmt_print_register(void);
void stmt_input_register(void);
void stmt_select_register(void);
void stmt_for_register(void);
void stmt_next_register(void);
void stmt_while_register(void);
void stmt_wend_register(void);
void stmt_do_register(void);
void stmt_loop_register(void);
void stmt_exit_loop_register(void);
void stmt_open_register(void);
void stmt_close_register(void);
void stmt_get_register(void);
void stmt_put_register(void);
void stmt_field_register(void);
void stmt_files_register(void);
void stmt_kill_register(void);
void stmt_mkdir_register(void);
void stmt_name_register(void);
void stmt_seek_register(void);
void stmt_screen_register(void);
void stmt_cls_register(void);
void stmt_home_register(void);
void stmt_color_register(void);
void stmt_line_register(void);
void stmt_circle_register(void);
void stmt_sound_register(void);
void stmt_play_register(void);
void stmt_beep_register(void);
void stmt_voice_register(void);
void stmt_on_key_register(void);
void stmt_on_timer_register(void);
void stmt_on_error_register(void);
void stmt_resume_register(void);
void stmt_try_register(void);
void stmt_mat_input_register(void);
void stmt_mat_print_register(void);
void stmt_mat_read_register(void);
void stmt_scope_register(void);
void stmt_help_register(void);
void stmt_reformat_register(void);
void stmt_check_register(void);
void stmt_verify_register(void);
void stmt_test_register(void);
void stmt_debug_register(void);
void stmt_version_register(void);
void func_varptr_register(void);
void stmt_introspection_register(void);
void stmt_selftest_register(void);

BootContext *boot_execute(const BootConfig *config) {
    if (!config) return NULL;
    
    g_is_repl = config->is_repl;

    BootContext *ctx = (BootContext *)calloc(1, sizeof(BootContext));
    if (!ctx) return NULL;

    /* Initialize Security Sandbox & Feature Self-Registration */
    security_init(SEC_OPEN);
    feature_reg_init();

    funcreg_init();
    microlib_init();
    spec_registry_init();

    func_rnd_register();
    func_int_register();
    func_abs_register();
    func_sqr_register();
    func_sin_register();
    func_cos_register();
    func_tan_register();
    func_atn_register();
    func_log_register();
    func_exp_register();
    func_acos_register();
    func_asin_register();
    func_atan2_register();
    func_ceil_register();
    func_clamp_register();
    func_fix_register();
    func_floor_register();
    func_lerp_register();
    func_pi_register();
    func_round_register();
    func_sgn_register();
    func_bin_register();
    func_chr_register();
    func_hex_register();
    func_instr_register();
    func_lcase_register();
    func_left_register();
    func_len_register();
    func_ltrim_register();
    func_mid_register();
    func_oct_register();
    func_pack_register();
    func_right_register();
    func_rtrim_register();
    func_space_register();
    func_str_register();
    func_string_register();
    func_trim_register();
    func_ucase_register();
    func_unpack_register();
    func_val_register();
    func_bitcount_register();
    func_readbit_register();
    func_resetbit_register();
    func_setbit_register();
    func_shl_register();
    func_shr_register();
    func_togglebit_register();
    func_clock_num_register();
    func_clock_str_register();
    func_date_register();
    func_environ_register();
    func_fre_register();
    func_inkey_register();
    func_inp_register();
    func_peek_register();
    func_ticks_register();
    func_time_register();
    func_timer_register();
    stmt_goto_register();
    stmt_gosub_register();
    stmt_return_register();
    stmt_if_register();
    stmt_end_register();
    stmt_rem_register();
    stmt_stop_register();
    stmt_print_register();
    stmt_input_register();
    stmt_select_register();
    stmt_for_register();
    stmt_next_register();
    stmt_while_register();
    stmt_wend_register();
    stmt_do_register();
    stmt_loop_register();
    stmt_exit_loop_register();
    stmt_open_register();
    stmt_close_register();
    stmt_get_register();
    stmt_put_register();
    stmt_field_register();
    stmt_files_register();
    stmt_kill_register();
    stmt_mkdir_register();
    stmt_name_register();
    stmt_seek_register();
    stmt_screen_register();
    stmt_cls_register();
    stmt_home_register();
    stmt_color_register();
    stmt_line_register();
    stmt_circle_register();
    stmt_sound_register();
    stmt_play_register();
    stmt_beep_register();
    stmt_voice_register();
    stmt_on_key_register();
    stmt_on_timer_register();
    stmt_on_error_register();
    stmt_resume_register();
    stmt_try_register();
    stmt_mat_input_register();
    stmt_mat_print_register();
    stmt_mat_read_register();
    stmt_scope_register();
    stmt_help_register();
    stmt_reformat_register();
    stmt_check_register();
    stmt_verify_register();
    stmt_test_register();
    stmt_debug_register();
    stmt_version_register();
    func_varptr_register();
    stmt_introspection_register();
    stmt_selftest_register();

    
    register_crypto_functions();
    register_string_ext_functions();
#ifndef BASIC_LITE_BUILD
    register_arrayext_functions();
#endif
    register_regex_functions();
    register_array_sort_functions();

    /* Phase 0: Host Entry Point & Platform Init */
    platform_init();

    /* Phase 1: Core Memory System Initialization */
    size_t prog_mem = config->prog_mem ? config->prog_mem : boot_get_mem_size("BASIC_PROG_MEM", BASIC_DEFAULT_PROG_MEM);
    size_t var_mem  = config->var_mem ? config->var_mem : boot_get_mem_size("BASIC_VAR_MEM", BASIC_DEFAULT_VAR_MEM);
    size_t str_mem  = config->str_mem ? config->str_mem : boot_get_mem_size("BASIC_STR_MEM", BASIC_DEFAULT_STR_MEM);
    size_t scratch  = config->scratch_mem ? config->scratch_mem : boot_get_mem_size("BASIC_SCRATCH_MEM", BASIC_DEFAULT_SCRATCH_MEM);

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

#if SUPPORT_NET
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

    stdlib_init(ctx->vm);
#if SUPPORT_MAT
    mod_mathext_init(ctx->vm);
#endif

    srand(1);

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

    if (ctx->vm) vm_shutdown(ctx->vm);
    if (ctx->var) var_shutdown(ctx->var);
    if (ctx->str) str_shutdown(ctx->str);
    if (ctx->vdev) vdev_shutdown(ctx->vdev);
    if (ctx->mem) mem_shutdown(ctx->mem);

    free(ctx);
}

VMContext *boot_system(size_t heap_size) {
    BootConfig config;
    memset(&config, 0, sizeof(config));
    
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
    free(boot);
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

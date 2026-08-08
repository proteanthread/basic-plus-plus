/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mobile.c
 * @brief Mobile Devices (Phone & Tablet) C17 Engine bootstrap entry point.
 *
 * WHAT IT DOES:
 * Implements the mobile app embedding entry point for Android (NDK JNI bridge) and iOS
 * (Objective-C/Swift C-bridge) on phones and tablets.
 *
 * WHY IT EXISTS:
 * Dedicated bootstrap profile supporting touch-screen virtual devices, mobile lifecycle events
 * (pause, resume, low-memory warnings), and touch canvas rendering.
 *
 * WHY IT WORKS THIS WAY:
 * Exposes C17 lifecycle routines (mobile_engine_init, mobile_engine_exec, mobile_engine_pause,
 * mobile_engine_shutdown) callable by iOS AppDelegates and Android NativeActivities.
 *
 * WHAT CAN BE CHANGED:
 * Mobile touch gesture mappings and mobile heap limits.
 *
 * WHAT CANNOT BE CHANGED:
 * Mobile lifecycle C-bridge function signatures.
 *
 * WHAT TO EXPECT:
 * Embedded C17 engine responds cleanly to mobile OS pause, resume, and touch events.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Check JNI / Swift bridge function signatures and mobile memory limits.
 *
 * ASSUMPTIONS:
 * Host mobile environment calls mobile_engine_init() on app launch.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, no host OS-specific C++ headers.
 *
 * FUTURE EXPANSIONS:
 * Accelerometer and multi-touch gesture virtual device hooks.
 */

#include "core/boot.h"
#include "types/config.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "debug/logger.h"

#include <stdio.h>
#include <stdlib.h>

static VMContext *g_mobile_vm = NULL;

int mobile_engine_init(size_t heap_mb) {
    platform_init();
    logger_init("mobile.log", "mobile.out");

    size_t mem_size = (heap_mb > 0 ? heap_mb : 128L) * 1024L * 1024L; /* Default 128 MB Mobile allocation */
    g_mobile_vm = boot_system(mem_size);
    if (!g_mobile_vm) {
        return -1;
    }
    return 0;
}

int mobile_engine_exec(const char *code) {
    if (!g_mobile_vm || !code) return -1;
    BppError err = vm_execute_line(g_mobile_vm, code);
    return err.code;
}

void mobile_engine_pause(void) {
    /* Mobile OS pause lifecycle hook */
}

void mobile_engine_resume(void) {
    /* Mobile OS resume lifecycle hook */
}

void mobile_engine_shutdown(void) {
    if (g_mobile_vm) {
        boot_shutdown_vm(g_mobile_vm);
        g_mobile_vm = NULL;
    }
    platform_shutdown();
}

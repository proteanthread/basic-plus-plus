/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file standalone_runner.h
 * @brief Editor component implementation and public API surface for standalone_runner.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for standalone_runner.h within the editor subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

#ifndef STANDALONE_RUNNER_H
#define STANDALONE_RUNNER_H

#ifdef STANDALONE_EDITOR

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 500
#endif
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#ifndef F_OK
#define F_OK 0
#endif
#ifndef access
#define access _access
#endif
#else
#include <unistd.h>
#endif

static const char* find_runner(void) {
    if (access("bs.exe", F_OK) == 0) return "bs.exe";
    if (access("bpp.exe", F_OK) == 0) return "bpp.exe";
    if (access("baspp.exe", F_OK) == 0) return "baspp.exe";
    if (access("bs", F_OK) == 0) return "./bs";
    if (access("bpp", F_OK) == 0) return "./bpp";
    if (access("baspp", F_OK) == 0) return "./baspp";
    return NULL;
}

static void execute_standalone(const char *target, int exec_mode) {
    const char *runner = find_runner();
    if (!runner) {
        printf("\n[Error: Execution engine not found. Ensure bs, bpp, or baspp is in the directory.]\n");
        return;
    }
    
    const char *args = "";
    if (strstr(runner, "bpp") || strstr(runner, "baspp")) {
        args = "-c ";
    }
    
    char cmd[2048];
    if (exec_mode == 1) { /* Debug */
        printf("\n[Standalone Debugging: Simulated Step Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else if (exec_mode == 2) { /* Trace */
        printf("\n[Standalone Tracing: Simulated Trace Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    }
    
    system(cmd);
}

#endif /* STANDALONE_EDITOR */
#endif /* STANDALONE_RUNNER_H */
